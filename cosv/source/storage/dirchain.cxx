/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dirchain.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:05:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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
    const char * pRestPath = i_sSubPath;
    if ( pRestPath != 0 ? *pRestPath == *i_sDelimiter : false )
        ++pRestPath;

    for ( const char * pDirEnd = strchr(pRestPath,*i_sDelimiter);
          pDirEnd != 0;
          pDirEnd = strchr(pRestPath,*i_sDelimiter) )
    {
        aPath.push_back( String(pRestPath, pDirEnd - pRestPath) );
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



