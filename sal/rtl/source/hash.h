#ifndef INCLUDED_RTL_SOURCE_HASH_H
#define INCLUDED_RTL_SOURCE_HASH_H

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct StringHashTableImpl StringHashTable;

StringHashTable *rtl_str_hash_new    (sal_uInt32         nSize);
void             rtl_str_hash_free   (StringHashTable   *pHash);
rtl_uString     *rtl_str_hash_intern (StringHashTable   *pHash,
                                      rtl_uString       *pString,
                                      int                can_return);
void             rtl_str_hash_remove (StringHashTable   *pHash,
                                      rtl_uString       *pString);

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDED_RTL_SOURCE_HASH_H */
