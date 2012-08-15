/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/unqidx.hxx>


/*************************************************************************
|*
|*    UniqueIndex::Insert()
|*
*************************************************************************/

sal_uIntPtr UniqueIndexImpl::Insert( void* p )
{
    // NULL-Pointer ist nicht erlaubt
    if ( !p )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

   // Ist Array voll, dann expandieren
    sal_uIntPtr nTmp = size();
    if( nTmp == nCount )
        nTmp++;

    // Damit UniqIndex nicht ueberlaeuft, wenn Items geloescht wurden
    nUniqIndex = nUniqIndex % nTmp;

    // Leeren Eintrag suchen
    while ( find( nUniqIndex ) != end() )
        nUniqIndex = (nUniqIndex+1) % nTmp;

    // Object im Array speichern
    (*this)[ nUniqIndex ] = p;

    // Anzahl der Eintraege erhoehen und Index zurueckgeben
    nCount++;
    nUniqIndex++;
    return ( nUniqIndex + nStartIndex - 1 );
}

/*************************************************************************
|*
|*    UniqueIndexImpl::Insert()
|*
*************************************************************************/

void UniqueIndexImpl::Insert( sal_uIntPtr nIndex, void* p )
{
    // NULL-Pointer ist nicht erlaubt
    if ( !p )
        return;

    sal_uIntPtr nContIndex = nIndex - nStartIndex;

    bool bFound = find( nContIndex ) != end();

    // Object im Array speichern
    (*this)[ nContIndex ] = p;

    if( !bFound )
        nCount++;
}

/*************************************************************************
|*
|*    UniqueIndexImpl::Remove()
|*
*************************************************************************/

void* UniqueIndexImpl::Remove( sal_uIntPtr nIndex )
{
    // Ist Index zulaessig
    if ( (nIndex >= nStartIndex) &&
         (nIndex < (size() + nStartIndex)) )
    {
        // Index-Eintrag als leeren Eintrag setzen und Anzahl der
        // gespeicherten Indexe erniedriegen, wenn Eintrag belegt war
        iterator it = find( nIndex - nStartIndex );
        if( it != end() )
        {
            void* p = it->second;
            erase( it );
            nCount--;
            return p;
        }
    }
    return NULL;
}

/*************************************************************************
|*
|*    UniqueIndexImpl::Get()
|*
*************************************************************************/

void* UniqueIndexImpl::Get( sal_uIntPtr nIndex ) const
{
    // Ist Index zulaessig
    if ( (nIndex >= nStartIndex) &&
         (nIndex < (size() + nStartIndex)) )
    {
        const_iterator it = find( nIndex - nStartIndex );
        if( it != end() )
            return it->second;
    }
    return NULL;
}

/*************************************************************************
|*
|*    UniqueIndexImpl::FirstIndex()
|*
*************************************************************************/

sal_uIntPtr UniqueIndexImpl::FirstIndex() const
{
    if ( empty() )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

    return begin()->first;
}

/*************************************************************************
|*
|*    UniqueIndexImpl::LastIndex()
|*
*************************************************************************/

sal_uIntPtr UniqueIndexImpl::LastIndex() const
{
    if ( empty() )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

    return rbegin()->first;
}

/*************************************************************************
|*
|*    UniqueIndexImpl::NextIndex()
|*
*************************************************************************/

sal_uIntPtr UniqueIndexImpl::NextIndex(sal_uIntPtr aIndex) const
{
    const_iterator it = find( aIndex );
    if ( it == end() )
        return UNIQUEINDEX_ENTRY_NOTFOUND;
    ++it;
    if ( it == end() )
        return UNIQUEINDEX_ENTRY_NOTFOUND;
    return it->first;
}

/*************************************************************************
|*
|*    UniqueIndexImpl::GetIndexOf()
|*
*************************************************************************/

sal_uIntPtr UniqueIndexImpl::GetIndexOf(void* p) const
{
    for( const_iterator it = begin(); it != end(); ++it )
        if( it->second == p )
            return it->first;
    return UNIQUEINDEX_ENTRY_NOTFOUND;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
