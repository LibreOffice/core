/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"
#include "rtl/allocator.hxx"

#include "hash.h"
#include "strimp.h"


#include <hash_set>

namespace {
    struct UStringHash
    {
        size_t operator()(rtl_uString * const &rString) const
            { return (size_t)rtl_ustr_hashCode_WithLength( rString->buffer, rString->length ); }
    };
    struct UStringEqual
    {
        sal_Bool operator() ( rtl_uString * const &pStringA,
                              rtl_uString * const &pStringB) const
        {
            if (pStringA == pStringB)
                return true;
            if (pStringA->length != pStringB->length)
                return false;
            return !rtl_ustr_compare_WithLength( pStringA->buffer, pStringA->length,
                                                 pStringB->buffer, pStringB->length);
        }
    };
}

typedef std::hash_set< rtl_uString *, UStringHash, UStringEqual,
                       rtl::Allocator<rtl_uString *> > UniqueHash;

struct StringHashTableImpl : public UniqueHash
{
    StringHashTableImpl(sal_uInt32 nSize) : UniqueHash( nSize ) {}
};

StringHashTable *
rtl_str_hash_new (sal_uInt32 nSize)
{
    return new StringHashTableImpl (nSize);
}

void
rtl_str_hash_free (StringHashTable *pHash)
{
    delete pHash;
}

rtl_uString *
rtl_str_hash_intern (StringHashTable   *pHash,
                     rtl_uString       *pString,
                     int                can_return)
{
    UniqueHash::iterator aIter;
    aIter = pHash->find(pString);
    if (aIter != pHash->end())
    {
        rtl_uString *pHashStr = *aIter;
        rtl_uString_acquire (pHashStr);
        return pHashStr;
    }
    if (!can_return)
    {
        rtl_uString *pCopy = NULL;
        rtl_uString_newFromString( &pCopy, pString );
        pString = pCopy;
        if (!pString)
            return NULL;
    }

    if (!SAL_STRING_IS_STATIC (pString))
        pString->refCount |= SAL_STRING_INTERN_FLAG;
    pHash->insert(pString);

    return pString;
}

void
rtl_str_hash_remove (StringHashTable   *pHash,
                     rtl_uString       *pString)
{
    pHash->erase(pString);
}
