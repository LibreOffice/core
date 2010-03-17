#if defined(GRAPHITEADAPTSTL) && defined(std)
#   include <ostream>
#   include <istream>
#   include <fstream>
#   include <iostream>
#   include <vector>
#   include <algorithm>
#   define std_was_redefined_as_stlport std
#   undef std
#   define _STLP_OUTERMOST_HEADER_ID 0xdeadbeaf
#   pragma GCC visibility push(default)
#   include _STLP_NATIVE_HEADER(exception_defines.h)
#   include _STLP_NATIVE_HEADER(limits)
#   include _STLP_NATIVE_HEADER(memory)
#   include _STLP_NATIVE_HEADER(exception)
#   include _STLP_NATIVE_HEADER(iosfwd)
#   include _STLP_NATIVE_HEADER(algorithm)
#   include _STLP_NATIVE_HEADER(string)
#   include _STLP_NATIVE_HEADER(streambuf)
#   include _STLP_NATIVE_HEADER(ios)
#   include _STLP_NATIVE_HEADER(locale)
#   include _STLP_NATIVE_HEADER(stdexcept)
#   include _STLP_NATIVE_HEADER(ostream)
#   include _STLP_NATIVE_HEADER(istream)
#   include _STLP_NATIVE_HEADER(iostream)
#   include _STLP_NATIVE_HEADER(vector)
#   pragma GCC visibility pop
#endif
//sil_std resolves to the std that Graphite was built with
namespace sil_std = std;
