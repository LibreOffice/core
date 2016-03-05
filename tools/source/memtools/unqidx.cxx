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

UniqueIndexImpl::Index UniqueIndexImpl::Insert( void* p )
{
    // NULL-Pointer not allowed
    if ( !p )
        return IndexNotFound;

    const Index nTmp = static_cast<Index>(maMap.size()) + 1;

    // Avoid overflow of UniqIndex upon deletion
    nUniqIndex = nUniqIndex % nTmp;

    // Search next empty index
    while ( maMap.find( nUniqIndex ) != maMap.end() )
        nUniqIndex = (nUniqIndex+1) % nTmp;

    maMap[ nUniqIndex ] = p;

    nUniqIndex++;
    return ( nUniqIndex + nStartIndex - 1 );
}

void* UniqueIndexImpl::Remove( Index nIndex )
{
    // Check for valid index
    if ( nIndex >= nStartIndex )
    {
        std::map<Index, void*>::iterator it = maMap.find( nIndex - nStartIndex );
        if( it != maMap.end() )
        {
            void* p = it->second;
            maMap.erase( it );
            return p;
        }
    }
    return nullptr;
}

void* UniqueIndexImpl::Get( Index nIndex ) const
{
    // check for valid index
    if ( nIndex >= nStartIndex )
    {
        std::map<Index, void*>::const_iterator it = maMap.find( nIndex - nStartIndex );
        if( it != maMap.end() )
            return it->second;
    }
    return nullptr;
}

UniqueIndexImpl::Index UniqueIndexImpl::FirstIndex() const
{
    if ( maMap.empty() )
        return IndexNotFound;

    return maMap.begin()->first;
}

UniqueIndexImpl::Index UniqueIndexImpl::LastIndex() const
{
    if ( maMap.empty() )
        return IndexNotFound;

    return maMap.rbegin()->first;
}

UniqueIndexImpl::Index UniqueIndexImpl::NextIndex(Index aIndex) const
{
    std::map<Index, void*>::const_iterator it = maMap.find( aIndex );
    if ( it == maMap.end() )
        return IndexNotFound;
    ++it;
    if ( it == maMap.end() )
        return IndexNotFound;
    return it->first;
}

UniqueIndexImpl::Index UniqueIndexImpl::GetIndexOf(void* p) const
{
    for( std::map<Index, void*>::const_iterator it = maMap.begin(); it != maMap.end(); ++it )
        if( it->second == p )
            return it->first;
    return IndexNotFound;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
