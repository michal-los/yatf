#include "../include/yatf.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yatf_tests
#include <boost/test/unit_test.hpp>
#include <cstdarg>
#include <string>
#include "common.h"

char buffer[4096];
int position = 0;

int print(const char *fmt, ...) {
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf((char *)buffer + position, fmt, args);
    position += i;
    va_end(args);
    return i;
}

std::string get_buffer() {
    position = 0;
    return std::string(buffer);
}

void reset_buffer() {
    buffer[0] = 0;
    position = 0;
}

namespace yatf {
namespace detail {

test_session test_session::_instance;
printf_t _printf = print;

} // namespace detail
} // namespace yatf

void empty_test_case() {}
yatf::detail::test_session::test_case dummy_tc{"suite", "test", empty_test_case};

