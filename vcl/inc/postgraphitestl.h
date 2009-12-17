#ifdef std_was_redefined_as_stlport
//  put things back the way they were
#   define std std_was_redefined_as_stlport
#   undef _STLP_DONT_REDEFINE_STD
#   undef _STLP_WHOLE_NATIVE_STD
#   undef _STLP_STRING
#   undef _STLP_IOSTREAM
#   undef _STLP_IOSFWD
#   undef _STLP_IOMANIP
#   undef _STLP_ALGORITHM
#   undef _STLP_VECTOR
#   undef _STLP_SET
#   undef _STLP_MAP
//  force config to be re-read
#   undef _STLP_CONFIG_H
#   include <stddef.h>
#endif
