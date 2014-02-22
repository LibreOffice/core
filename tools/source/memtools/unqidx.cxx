/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/unqidx.hxx>

sal_uIntPtr UniqueIndexImpl::Insert( void* p )
{
    
    if ( !p )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

   
    sal_uIntPtr nTmp = size();
    if( nTmp == nCount )
        nTmp++;

    
    nUniqIndex = nUniqIndex % nTmp;

    
    while ( find( nUniqIndex ) != end() )
        nUniqIndex = (nUniqIndex+1) % nTmp;

    
    (*this)[ nUniqIndex ] = p;

    nCount++;
    nUniqIndex++;
    return ( nUniqIndex + nStartIndex - 1 );
}

void UniqueIndexImpl::Insert( sal_uIntPtr nIndex, void* p )
{
    
    if ( !p )
        return;

    sal_uIntPtr nContIndex = nIndex - nStartIndex;

    bool bFound = find( nContIndex ) != end();

    
    (*this)[ nContIndex ] = p;

    if( !bFound )
        nCount++;
}

void* UniqueIndexImpl::Remove( sal_uIntPtr nIndex )
{
    
    if ( (nIndex >= nStartIndex) &&
         (nIndex < (size() + nStartIndex)) )
    {
        
        
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

void* UniqueIndexImpl::Get( sal_uIntPtr nIndex ) const
{
    
    if ( (nIndex >= nStartIndex) &&
         (nIndex < (size() + nStartIndex)) )
    {
        const_iterator it = find( nIndex - nStartIndex );
        if( it != end() )
            return it->second;
    }
    return NULL;
}

sal_uIntPtr UniqueIndexImpl::FirstIndex() const
{
    if ( empty() )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

    return begin()->first;
}

sal_uIntPtr UniqueIndexImpl::LastIndex() const
{
    if ( empty() )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

    return rbegin()->first;
}

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

sal_uIntPtr UniqueIndexImpl::GetIndexOf(void* p) const
{
    for( const_iterator it = begin(); it != end(); ++it )
        if( it->second == p )
            return it->first;
    return UNIQUEINDEX_ENTRY_NOTFOUND;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
