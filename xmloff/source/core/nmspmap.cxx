/*************************************************************************
 *
 *  $RCSfile: nmspmap.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dvo $ $Date: 2001-06-29 21:07:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <nmspmap.hxx>
#endif

using namespace rtl;
using ::xmloff::token::GetXMLToken;
using ::xmloff::token::XML_XMLNS;


/* The basic idea of this class is that we have two two ways to search our
 * data...by prefix and by key. We use an STL hash_map for fast prefix
 * searching and an STL map for fast key searching.
 *
 * The references to an 'Index' refer to an earlier implementation of the
 * name space map and remain to support code which uses these interfaces.
 *
 * In this implementation, key and index should always be the same number.
 *
 * Martin 13/06/01
 */

SvXMLNamespaceMap::SvXMLNamespaceMap() :
    sXMLNS( GetXMLToken(XML_XMLNS) )
{
}

SvXMLNamespaceMap::SvXMLNamespaceMap( const SvXMLNamespaceMap& rMap ) :
    sXMLNS( GetXMLToken(XML_XMLNS) )
{
    aNameHash = rMap.aNameHash;
    aNameMap  = rMap.aNameMap;
}

SvXMLNamespaceMap::~SvXMLNamespaceMap()
{
}

int SvXMLNamespaceMap::operator ==( const SvXMLNamespaceMap& rCmp ) const
{
    return static_cast < int > (aNameHash == rCmp.aNameHash);
}

void SvXMLNamespaceMap::_Add( const OUString& rPrefix, const OUString &rName, USHORT nKey )
{
    NameSpaceEntry *pEntry = new NameSpaceEntry;
    pEntry->sName   = rName;
    pEntry->nKey    = nKey;
    pEntry->sPrefix = rPrefix;
    aNameHash[rPrefix] = pEntry;
    aNameMap[nKey]     = pEntry;
}

USHORT SvXMLNamespaceMap::Add( const OUString& rPrefix, const OUString& rName,
                               USHORT nKey )
{
    sal_Bool bRet = sal_False;

    if( XML_NAMESPACE_UNKNOWN == nKey )
        nKey = GetKeyByName( rName );

    DBG_ASSERT( XML_NAMESPACE_NONE != nKey,
                "SvXMLNamespaceMap::Add: invalid namespace key" );
    if( XML_NAMESPACE_NONE==nKey )
        return USHRT_MAX;

    if (!(aNameHash.count ( rPrefix ) ) )
    {
        _Add( rPrefix, rName, nKey );
        bRet = sal_True;
    }
    return bRet;
}

BOOL SvXMLNamespaceMap::AddAtIndex( USHORT nIdx, const OUString& rPrefix,
                                    const OUString& rName, USHORT nKey )
{
    sal_Bool bRet = sal_False;

    if( XML_NAMESPACE_UNKNOWN == nKey )
        nKey = GetKeyByName( rName );

    DBG_ASSERT( XML_NAMESPACE_NONE != nKey,
                "SvXMLNamespaceMap::AddAtIndex: invalid namespace key" );
    if( XML_NAMESPACE_NONE != nKey && ! ( aNameHash.count ( rPrefix ) ) )
    {
        _Add( rPrefix, rName, nKey );
        bRet = sal_True;
    }
    return bRet;
}

BOOL SvXMLNamespaceMap::AddAtIndex( USHORT nIdx, const sal_Char *pPrefix,
                                     const sal_Char *pName, USHORT nKey )
{
    OUString sPrefix( OUString::createFromAscii(pPrefix) );
    OUString sName( OUString::createFromAscii(pName) );

    return AddAtIndex( nIdx, sPrefix, sName, nKey );
}

USHORT SvXMLNamespaceMap::GetIndexByKey( USHORT nKey ) const
{
    return nKey;
}

USHORT SvXMLNamespaceMap::GetIndexByPrefix( const OUString& rPrefix ) const
{
    NameSpaceHash::const_iterator aIter = aNameHash.find(rPrefix);
    return (aIter != aNameHash.end()) ? (*aIter).second->nKey : USHRT_MAX;
}

USHORT SvXMLNamespaceMap::GetKeyByName( const OUString& rName ) const
{
    USHORT nKey = XML_NAMESPACE_UNKNOWN;
    NameSpaceHash::const_iterator aIter = aNameHash.begin(), aEnd = aNameHash.end();
    while (aIter != aEnd )
    {
        if ((*aIter).second->sName == rName)
        {
            nKey = (*aIter).second->nKey;
            break;
        }
        aIter++;
    }
    return nKey;
}

const OUString& SvXMLNamespaceMap::GetPrefixByIndex( USHORT nIdx ) const
{
    NameSpaceMap::const_iterator aIter = aNameMap.find (nIdx);
    return (aIter != aNameMap.end()) ? (*aIter).second->sPrefix : sEmpty;
}

const OUString& SvXMLNamespaceMap::GetNameByIndex( USHORT nIdx ) const
{
    NameSpaceMap::const_iterator aIter = aNameMap.find (nIdx);
    return (aIter != aNameMap.end()) ? (*aIter).second->sName : sEmpty;
}

USHORT SvXMLNamespaceMap::GetKeyByIndex( USHORT nIdx ) const
{
    return nIdx;
}

OUString SvXMLNamespaceMap::GetAttrNameByIndex( USHORT nIdx ) const
{
    OUStringBuffer sAttrName;
    NameSpaceMap::const_iterator aIter = aNameMap.find ( nIdx );
    if (aIter != aNameMap.end())
    {
        sAttrName.append( sXMLNS  );
        sAttrName.append( sal_Unicode(':') );
        sAttrName.append( (*aIter).second->sPrefix);
    }
    return sAttrName.makeStringAndClear();
}

OUString SvXMLNamespaceMap::GetQNameByIndex( USHORT nIdx,
                                           const OUString& rLocalName ) const
{
    // We always want to return at least the rLocalName...
    OUStringBuffer sQName;

    switch ( nIdx )
    {
        case XML_NAMESPACE_UNKNOWN:
            // ...if it's a completely unknown namespace, assert and return the local name
            DBG_ASSERT( sal_False, "SvXMLNamespaceMap::GetQNameByIndex: invalid namespace key" );
        case XML_NAMESPACE_NONE:
            // ...if there isn't one, return the local name
            sQName.append ( rLocalName );
        break;
        case XML_NAMESPACE_XMLNS:
        {
            // ...if it's in the xmlns namespace, make the prefix
            sQName.append ( sXMLNS );
            sQName.append ( sal_Unicode(':') );
            sQName.append ( rLocalName );
        }
        break;
        default:
        {
            NameSpaceMap::const_iterator aIter = aNameMap.find ( nIdx );
            if ( aIter != aNameMap.end() )
            {
                // ...if it's in our map, make the prefix
                sQName.append ( (*aIter).second->sPrefix);
                sQName.append ( sal_Unicode(':') );
                sQName.append ( rLocalName );
            }
            else
            {
                // ... if isn't, this is a Bad Thing, assert and return the local name
                DBG_ASSERT( sal_False, "SvXMLNamespaceMap::GetQNameByIndex: invalid namespace key" );
                sQName.append ( rLocalName );
            }
        }
    }
    return sQName.makeStringAndClear();;
}

USHORT SvXMLNamespaceMap::GetKeyByAttrName( const OUString& rAttrName,
                                            OUString *pPrefix,
                                            OUString *pLocalName,
                                            OUString *pNamespace,
                                            USHORT nIdxGuess ) const
{
    USHORT nKey = XML_NAMESPACE_UNKNOWN;

    sal_Int32 nColonPos = rAttrName.indexOf( sal_Unicode(':') );
    if( -1L != nColonPos )
    {
        OUString aPrefix( rAttrName.copy( 0L, nColonPos ) );
        if( pPrefix )
            *pPrefix = aPrefix;
        if( pLocalName )
            *pLocalName = rAttrName.copy( nColonPos + 1L );

        NameSpaceHash::const_iterator aIter = aNameHash.find( aPrefix );
        if ( aIter != aNameHash.end() )
        {
            nKey = (*aIter).second->nKey;
            if ( pNamespace )
                *pNamespace = (*aIter).second->sName;
        }
        else if ( aPrefix == sXMLNS )
            nKey = XML_NAMESPACE_XMLNS;
    }
    else
    {
        nKey = XML_NAMESPACE_NONE;
        if( pPrefix )
            *pPrefix = sEmpty;
        if( pLocalName )
            *pLocalName = rAttrName;
        if( pNamespace )
            *pNamespace = sEmpty;
    }

    return nKey;
}

USHORT SvXMLNamespaceMap::GetFirstIndex() const
{
    return aNameMap.empty() ? USHRT_MAX : (*aNameMap.begin()).second->nKey;
}

USHORT SvXMLNamespaceMap::GetNextIndex( USHORT nOldIdx ) const
{
    NameSpaceMap::const_iterator aIter = aNameMap.find ( nOldIdx );
    return (++aIter == aNameMap.end()) ? USHRT_MAX : (*aIter).second->nKey;
}
