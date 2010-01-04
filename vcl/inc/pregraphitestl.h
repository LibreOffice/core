#if defined(GRAPHITEADAPTSTL) && defined(std)
#   define std_was_redefined_as_stlport std
#   undef std
#   undef _STLP_CONFIG_H
#   undef _STLP_STRING
#   undef _STLP_IOSTREAM
#   undef _STLP_IOSFWD
#   undef _STLP_IOMANIP
#   undef _STLP_ALGORITHM
#   undef _STLP_VECTOR
#   undef _STLP_SET
#   undef _STLP_MAP
#   define _STLP_DONT_REDEFINE_STD 1
#   define _STLP_WHOLE_NATIVE_STD 1
#   pragma GCC visibility push(default)
#   include _STLP_NATIVE_HEADER(exception_defines.h)
#   include _STLP_NATIVE_HEADER(limits)
#   include _STLP_NATIVE_HEADER(memory)
#   include _STLP_NATIVE_HEADER(exception)
#   include _STLP_NATIVE_HEADER(iosfwd)
#   include _STLP_NATIVE_HEADER(string)
#   include _STLP_NATIVE_HEADER(streambuf)
#   include _STLP_NATIVE_HEADER(ios)
#   include _STLP_NATIVE_HEADER(locale)
#   include _STLP_NATIVE_HEADER(stdexcept)
#   include _STLP_NATIVE_HEADER(ostream)
#   include _STLP_NATIVE_HEADER(istream)
#   include _STLP_NATIVE_HEADER(iostream)
#   pragma GCC visibility pop
#endif
//sil_std resolves to the std that Graphite was built with
namespace sil_std = std;
