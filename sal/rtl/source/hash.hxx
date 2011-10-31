/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_RTL_SOURCE_HASH_HXX
#define INCLUDED_RTL_SOURCE_HASH_HXX

#include <sal/types.h>
#include <rtl/ustring.h>

/* These functions are not multi-thread safe: */

rtl_uString     *rtl_str_hash_intern (rtl_uString       *pString,
                                      int                can_return);
void             rtl_str_hash_remove (rtl_uString       *pString);

#endif /* INCLUDED_RTL_SOURCE_HASH_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
