/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "svl/stringpool.hxx"
#include "unotools/charclass.hxx"

namespace svl {

StringPool::StringPool() : mpCharClass(NULL) {}
StringPool::StringPool( const CharClass* pCharClass ) : mpCharClass(pCharClass) {}

rtl_uString* StringPool::intern( const OUString& rStr )
{
    InsertResultType aRes = findOrInsert(maStrPool, rStr);
    if (aRes.first == maStrPool.end())
        // Insertion failed.
        return NULL;

    rtl_uString* pOrig = aRes.first->pData;

    if (!aRes.second)
        // No new string has been inserted. Return the existing string in the pool.
        return pOrig;

    if (!mpCharClass)
        return pOrig;

    // This is a new string insertion. Establish mapping to upper-case variant.

    OUString aUpper = mpCharClass->uppercase(rStr);
    aRes = findOrInsert(maStrPoolUpper, aUpper);
    if (aRes.first == maStrPoolUpper.end())
        // Failed to insert or fetch upper-case variant. Should never happen.
        return pOrig;

    // Set mapping.
    rtl_uString* pUpper = aRes.first->pData;
    maToUpperMap.insert(StrIdMapType::value_type(pOrig, pUpper));

    return pOrig;
}

const rtl_uString* StringPool::getIdentifier( const OUString& rStr ) const
{
    StrHashType::iterator it = maStrPool.find(rStr);
    return (it == maStrPool.end()) ? NULL : it->pData;
}

const rtl_uString* StringPool::getIdentifierIgnoreCase( const OUString& rStr ) const
{
    if (!mpCharClass)
        return NULL;

    OUString aUpper = mpCharClass->uppercase(rStr);
    StrHashType::iterator it = maStrPoolUpper.find(aUpper);
    return (it == maStrPool.end()) ? NULL : it->pData;
}

StringPool::InsertResultType StringPool::findOrInsert( StrHashType& rPool, const OUString& rStr ) const
{
    StrHashType::iterator it = rPool.find(rStr);
    bool bInserted = false;
    if (it == rPool.end())
    {
        // Not yet in the pool.
        std::pair<StrHashType::iterator, bool> r = rPool.insert(rStr.intern());
        if (!r.second)
            // Insertion failed.
            return InsertResultType(rPool.end(), false);

        it = r.first;
        bInserted = true;
    }

    return InsertResultType(it, bInserted);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
