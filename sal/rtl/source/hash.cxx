/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hash.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-19 11:56:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#ifndef INCLUDED_RTL_ALLOCATOR_HXX
#include "rtl/allocator.hxx"
#endif

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
