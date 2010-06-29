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

#include <precomp.h>
#include <cosv/dirchain.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/bstream.hxx>




namespace csv
{
namespace ploc
{


DirectoryChain::DirectoryChain()
{
}

DirectoryChain::DirectoryChain( const char *        i_sSubPath,
                                bool                i_bPathIsAlwaysDir,
                                const char *        i_sDelimiter        )
{
    Set( i_sSubPath, i_bPathIsAlwaysDir, i_sDelimiter );
}

DirectoryChain::~DirectoryChain()
{
}

void
DirectoryChain::Set( const char *        i_sSubPath,
                     bool                i_bPathIsAlwaysDir,
                     const char *        i_sDelimiter        )
{
    csv_assert(i_sDelimiter != 0);
    if (i_sSubPath == 0)
        return;

    const char * pRestPath = i_sSubPath;
    if (*pRestPath == *i_sDelimiter)
        ++pRestPath;

    for ( const char * pDirEnd = strchr(pRestPath,*i_sDelimiter);
          pDirEnd != 0;
          pDirEnd = strchr(pRestPath,*i_sDelimiter) )
    {
        aPath.push_back( String(pRestPath, pDirEnd) );
        pRestPath = pDirEnd + 1;
    }
    if (*pRestPath != 0 AND i_bPathIsAlwaysDir)
        aPath.push_back( String(pRestPath) );
}

void
DirectoryChain::PushFront( const String & i_sName )
{
    aPath.insert( aPath.begin(), i_sName );
}

void
DirectoryChain::PushFront( const DirectoryChain & i_sPath )
{
    aPath.insert( aPath.begin(), i_sPath.Begin(), i_sPath.End() );
}

void
DirectoryChain::PushBack( const String & i_sName )
{
    aPath.push_back(i_sName);
}

void
DirectoryChain::PushBack( const DirectoryChain & i_sPath )
{
    aPath.insert( aPath.end(), i_sPath.Begin(), i_sPath.End() );
}

void
DirectoryChain::PopFront( uintt i_nCount )
{
    if (i_nCount <= aPath.size())
        aPath.erase( aPath.begin(), aPath.begin() + i_nCount );
    else
        aPath.erase( aPath.begin(), aPath.end() );
}

void
DirectoryChain::PopBack( uintt i_nCount )
{
    if (i_nCount <= aPath.size())
        aPath.erase( aPath.end() - i_nCount, aPath.end() );
    else
        aPath.erase( aPath.begin(), aPath.end() );
}

void
DirectoryChain::Get( ostream &      o_rPath,
                     const char *   i_sDelimiter ) const
{
    for ( std::vector<String>::const_iterator it = aPath.begin();
          it != aPath.end();
          ++it )
    {
        o_rPath << (*it).c_str() << i_sDelimiter;
    }
}

void
DirectoryChain::Get( bostream &      o_rPath,
                     const char *    i_sDelimiter ) const
{
    uintt deliLen = strlen(i_sDelimiter);

    for ( std::vector<String>::const_iterator it = aPath.begin();
          it != aPath.end();
          ++it )
    {
        o_rPath.write( (*it).c_str() );
        o_rPath.write( i_sDelimiter,  deliLen);
    }
}




} // namespace ploc
} // namespace csv
