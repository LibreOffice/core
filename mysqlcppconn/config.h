#include <sal/types.h>

#define HAVE_FUNCTION_STRTOL 1
#define HAVE_FUNCTION_STRTOUL 1
#define HAVE_FUNCTION_STRTOL 1
#define HAVE_FUNCTION_STRTOULL 1

#if defined(MACOSX) || defined(SOLARIS) || defined(LINUX)
    #define HAVE_STDINT_H
    #define HAVE_INTTYPES_H
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#if !defined(HAVE_STDINT_H) && !defined(HAVE_INTTYPES_H)

// if we do not have the standard C99 integer types, then define them based on the respective SAL types
#if !defined(_SYS_TYPES_H)
typedef sal_Int8    int8_t;
typedef sal_Int16   int16_t;
typedef sal_Int32   int32_t;
#endif // !defined(_SYS_TYPES_H)
typedef sal_uInt8   uint8_t;
typedef sal_uInt16  uint16_t;
typedef sal_uInt32  uint32_t;
typedef sal_Int64   int64_t;
typedef sal_uInt64  uint64_t;
#endif // !defined(HAVE_STDINT_H) && !defined(HAVE_INTTYPES_H)
