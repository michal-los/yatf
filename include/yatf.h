#pragma once

#include <cstddef>

namespace yatf {

using tests_printer = int (*)(const char *, ...);

namespace detail {

extern tests_printer _print;

struct test_session final {

    // Minimal version of inherited_list
    template <typename Type>
    class tests_list {

        Type *_prev, *_next;

        void add_element(Type &new_element, Type &prev, Type &next) {
            next._prev = &new_element;
            prev._next = &new_element;
            new_element._next = &next;
            new_element._prev = &prev;
        }

        operator Type &() {
            return *reinterpret_cast<Type *>(this);
        }

    public:

        class iterator {

            Type *_ptr = nullptr;

        public:

            iterator(Type *t)
                : _ptr(t) {}

            iterator &operator++() {
                _ptr = _ptr->next();
                return *this;
            }

            Type &operator*() {
                return *_ptr;
            }

            bool operator!=(const iterator &comp) {
                return _ptr != comp._ptr;
            }

        };

        tests_list() {
            _next = _prev = reinterpret_cast<Type *>(this);
        }

        Type &add(Type *new_element) {
            add_element(*new_element, *_prev, *this);
            return *this;
        }

        Type *next() {
            return _next == this ? nullptr : _next;
        }

        auto begin() {
            return iterator(_next);
        }

        auto end() {
            return iterator(reinterpret_cast<Type *>(this));
        }

    };

    class test_case final : public tests_list<test_case> {

        const char *_suite_name;
        const char *_test_name;
        void (*_func)();
        const char *_run_message = "\e[32m[  RUN   ]\e[0m";
        const char *_pass_message = "\e[32m[  PASS  ]\e[0m";
        const char *_fail_message = "\e[31m[  FAIL  ]\e[0m";

        void print_test_start_message() {
            _print("%s %s.%s\n", _run_message, _suite_name, _test_name);
        }

        void print_test_result() {
            if (failed)
                _print("%s ", _fail_message);
            else
                _print("%s ", _pass_message);
            _print("%s.%s (%u assertions)\n\n", _suite_name, _test_name, assertions);
        }

    public:

        int assertions = 0;
        int failed = 0;

        test_case(const char *suite, const char *test, void (*func)())
                : _suite_name(suite), _test_name(test), _func(func) {
            test_session::get().register_test(this);
        }

        bool assert(bool cond) {
            ++assertions;
            if (!cond) ++failed;
            return cond;
        }

        template <typename T1, typename T2>
        bool assert_eq(T1 lhs, T2 rhs) {
            ++assertions;
            bool cond = (lhs == rhs);
            if (!cond) ++failed;
            return cond;
        }

        int call() {
            print_test_start_message();
            _func();
            print_test_result();
            return failed;
        }

    };

private:

    tests_list<test_case> _test_cases;
    test_case *_current_test_case;
    size_t _tests_number = 0;
    static test_session _instance;

public:

    static test_session &get() {
        return _instance;
    }

    void register_test(test_case *t) {
        _tests_number++;
        _test_cases.add(t);
    }

    int run() {
        unsigned failed = 0;
        unsigned test_cases = 0;;
        _print("\e[32m[========]\e[0m Running %u test cases\n\n", _tests_number);
        for (auto &test : _test_cases) {
            _current_test_case = &test;
            if (test.call()) failed++;
            test_cases++;
        }
        _print("\e[32m[========]\e[0m Passed %u test cases\n", test_cases - failed);
        if (failed) _print("\e[31m[========]\e[0m Failed %u test cases\n", failed);
        return failed;
    }

    test_case &current_test_case() {
        return *_current_test_case;
    }

};

inline void print(const char *str) {
    _print(str);
}

inline void print(char c) {
    _print("%c", c);
}

inline void print(unsigned char c) {
    _print("0x%x", c);
}

inline void print(short a) {
    _print("%d", a);
}

inline void print(unsigned short a) {
    _print("0x%x", a);
}

inline void print(int a) {
    _print("%d", a);
}

inline void print(unsigned int a) {
    _print("0x%x", a);
}

inline void print(void *a) {
    _print("0x%x", reinterpret_cast<unsigned long>(a));
}

inline void print(std::nullptr_t) {
    _print("NULL");
}

template <typename T>
inline void print(const T &a) {
    _print("0x%x", reinterpret_cast<unsigned long>(&a));
}

template<typename First, typename... Rest>
inline void print(const First &first, const Rest &... rest) {
    print(first);
    print(rest...);
}

} // namespace detail

#define REQUIRE(cond) \
    { \
        if (!yatf::detail::test_session::get().current_test_case().assert(cond)) \
            yatf::detail::print("assertion failed: ", __FILE__, ':', __LINE__, " \'", #cond, "\' is false\n"); \
    }

#define REQUIRE_FALSE(cond) \
    { \
        if (!yatf::detail::test_session::get().current_test_case().assert(!(cond))) \
            yatf::detail::print("assertion failed: ", __FILE__, ':', __LINE__, " \'", #cond, "\' is true\n"); \
    }

#define REQUIRE_EQ(lhs, rhs) \
    { \
        if (!yatf::detail::test_session::get().current_test_case().assert_eq(lhs, rhs)) { \
            yatf::detail::print("assertion failed: ", __FILE__, ':', __LINE__, " \'", #lhs, "\' isn't \'", #rhs, "\': "); \
            yatf::detail::print(lhs, " != ", rhs, "\n"); \
        } \
    }

// gtest compatible
#define EXPECT_EQ(lhs, rhs) REQUIRE_EQ(lhs, rhs)
#define EXPECT_TRUE(cond) REQUIRE(cond)

#define YATF_UNIQUE_NAME(name) \
    name##__COUNTER__

#define TEST(suite, name) \
    static void suite##_##name(); \
    yatf::detail::test_session::test_case YATF_UNIQUE_NAME(suite##_##name){#suite, #name, suite##_##name}; \
    static void suite##_##name()

#ifdef YATF_MAIN

namespace detail {

test_session test_session::_instance;
tests_printer _print;

} // namespace detail

inline int main(tests_printer printer) {
    detail::_print = printer;
    return detail::test_session::get().run();
}

#endif

} // namespace yatf

