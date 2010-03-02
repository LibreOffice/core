#ifdef std_was_redefined_as_stlport
//  put things back the way they were
#   define std std_was_redefined_as_stlport
#   undef _STLP_OUTERMOST_HEADER_ID
//  force config to be re-read
#   undef _STLP_NOTHROW_INHERENTLY
#   undef _STLP_CONFIG_H
#   include <stddef.h>
#endif
