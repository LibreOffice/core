/*************************************************************************
 *
 *  $RCSfile: nmspmap.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 18:20:11 $
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
#include <xmltoken.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <nmspmap.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
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
 * All references to Indices are now deprecated and the corresponding
 * 'Key' methods should be used instead
 *
 * Martin 13/06/01
 */

SvXMLNamespaceMap::SvXMLNamespaceMap()
: sXMLNS( GetXMLToken ( XML_XMLNS ) )
{
}

SvXMLNamespaceMap::SvXMLNamespaceMap( const SvXMLNamespaceMap& rMap )
: sXMLNS( GetXMLToken ( XML_XMLNS ) )
{
    aNameHash = rMap.aNameHash;
    aNameMap  = rMap.aNameMap;
}

SvXMLNamespaceMap::~SvXMLNamespaceMap()
{
    QNameCache::iterator aIter = aQNameCache.begin(), aEnd = aQNameCache.end();
    while ( aIter != aEnd )
    {
        const OUString *pString = (*aIter).first.second;
        aIter++;
        delete pString;
    }
}

int SvXMLNamespaceMap::operator ==( const SvXMLNamespaceMap& rCmp ) const
{
    return static_cast < int > (aNameHash == rCmp.aNameHash);
}

sal_uInt16 SvXMLNamespaceMap::_Add( const OUString& rPrefix, const OUString &rName, sal_uInt16 nKey )
{
    if( XML_NAMESPACE_UNKNOWN == nKey )
    {
        // create a new unique key with UNKNOWN flag set
        nKey = XML_NAMESPACE_UNKNOWN_FLAG;
        do
        {
            NameSpaceMap::const_iterator aIter = aNameMap.find ( nKey );
            if( aIter == aNameMap.end() )
                break;
            nKey++;
        }
        while ( sal_True );
    }
    NameSpaceEntry *pEntry = new NameSpaceEntry;
    pEntry->sName   = rName;
    pEntry->nKey    = nKey;
    pEntry->sPrefix = rPrefix;
    aNameHash[ rPrefix ] = pEntry;
    aNameMap [ nKey ]    = pEntry;
    return nKey;
}

sal_uInt16 SvXMLNamespaceMap::Add( const OUString& rPrefix, const OUString& rName,
                               sal_uInt16 nKey )
{
    if( XML_NAMESPACE_UNKNOWN == nKey )
        nKey = GetKeyByName( rName );

    DBG_ASSERT( XML_NAMESPACE_NONE != nKey,
                "SvXMLNamespaceMap::Add: invalid namespace key" );

    if( XML_NAMESPACE_NONE == nKey )
        return USHRT_MAX;

    if ( aNameHash.find ( rPrefix ) == aNameHash.end() )
        nKey = _Add( rPrefix, rName, nKey );

    return nKey;
}

sal_uInt16 SvXMLNamespaceMap::GetKeyByPrefix( const OUString& rPrefix ) const
{
    NameSpaceHash::const_iterator aIter = aNameHash.find(rPrefix);
    return (aIter != aNameHash.end()) ? (*aIter).second->nKey : USHRT_MAX;
}

sal_uInt16 SvXMLNamespaceMap::GetKeyByName( const OUString& rName ) const
{
    sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN;
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

const OUString& SvXMLNamespaceMap::GetPrefixByKey( sal_uInt16 nKey ) const
{
    NameSpaceMap::const_iterator aIter = aNameMap.find (nKey);
    return (aIter != aNameMap.end()) ? (*aIter).second->sPrefix : sEmpty;
}

const OUString& SvXMLNamespaceMap::GetNameByKey( sal_uInt16 nKey ) const
{
    NameSpaceMap::const_iterator aIter = aNameMap.find (nKey);
    return (aIter != aNameMap.end()) ? (*aIter).second->sName : sEmpty;
}

OUString SvXMLNamespaceMap::GetAttrNameByKey( sal_uInt16 nKey ) const
{
    OUStringBuffer sAttrName;
    NameSpaceMap::const_iterator aIter = aNameMap.find ( nKey );
    if (aIter != aNameMap.end())
    {
        sAttrName.append( sXMLNS  );
        sAttrName.append( sal_Unicode(':') );
        sAttrName.append( (*aIter).second->sPrefix);
    }
    return sAttrName.makeStringAndClear();
}

OUString SvXMLNamespaceMap::GetQNameByKey( sal_uInt16 nKey,
                            const OUString& rLocalName ) const
{
    // We always want to return at least the rLocalName...

    switch ( nKey )
    {
        case XML_NAMESPACE_UNKNOWN:
            // ...if it's a completely unknown namespace, assert and return the local name
            DBG_ASSERT( sal_False, "SvXMLNamespaceMap::GetQNameByKey: invalid namespace key" );
        case XML_NAMESPACE_NONE:
            // ...if there isn't one, return the local name
            return rLocalName;
        break;
        case XML_NAMESPACE_XMLNS:
        {
            // ...if it's in the xmlns namespace, make the prefix
            // don't bother caching this, it rarely happens
            OUStringBuffer sQName;
            sQName.append ( sXMLNS );
            sQName.append ( sal_Unicode(':') );
            sQName.append ( rLocalName );
            return sQName.makeStringAndClear();;
        }
        break;
        default:
        {
            QNameCache::const_iterator aQCacheIter = aQNameCache.find ( QNamePair ( nKey, &rLocalName ) );
            if ( aQCacheIter != aQNameCache.end() )
                return (*aQCacheIter).second;
            else
            {
                NameSpaceMap::const_iterator aIter = aNameMap.find ( nKey );
                if ( aIter != aNameMap.end() )
                {
                    OUStringBuffer sQName;
                    // ...if it's in our map, make the prefix
                    sQName.append ( (*aIter).second->sPrefix);
                    sQName.append ( sal_Unicode(':') );
                    sQName.append ( rLocalName );
                    OUString *pString = new OUString ( rLocalName ), &rString = sQName.makeStringAndClear();
                    const_cast < QNameCache * > (&aQNameCache)->operator[] ( QNamePair ( nKey, pString ) ) = rString;
                    return rString;
                }
                else
                {
                    // ... if it isn't, this is a Bad Thing, assert and return the local name
                    DBG_ASSERT( sal_False, "SvXMLNamespaceMap::GetQNameByKey: invalid namespace key" );
                    return rLocalName;
                }
            }
        }
    }
}

sal_uInt16 SvXMLNamespaceMap::_GetKeyByAttrName(
                            const OUString& rAttrName,
                            OUString *pLocalName ) const
{
    return GetKeyByAttrName( rAttrName, 0, pLocalName, 0 );
}

sal_uInt16 SvXMLNamespaceMap::_GetKeyByAttrName( const OUString& rAttrName,
                                            OUString *pPrefix,
                                            OUString *pLocalName,
                                            OUString *pNamespace ) const
{
    sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN;

    NameSpaceHash::const_iterator aIter = aNameCache.find ( rAttrName );
    if ( aIter != aNameCache.end() )
    {
        const NameSpaceEntry &rEntry = (*aIter).second.getBody();
        if ( pPrefix )
            *pPrefix = rEntry.sPrefix;
        if ( pLocalName )
            *pLocalName = rEntry.sName;
        nKey = rEntry.nKey;
        if ( pNamespace )
        {
            NameSpaceMap::const_iterator aMapIter = aNameMap.find (nKey);
            *pNamespace = aMapIter != aNameMap.end() ? (*aMapIter).second->sName : sEmpty;
        }
    }
    else
    {
        sal_Int32 nColonPos = rAttrName.indexOf( sal_Unicode(':') );
        NameSpaceEntry *pEntry = new NameSpaceEntry;
        if( -1L == nColonPos )
        {
            // case: no ':' found -> default namespace
            pEntry->sPrefix = OUString();
            pEntry->sName = rAttrName;
        }
        else
        {
            // normal case: ':' found -> get prefix/suffix
            pEntry->sPrefix = rAttrName.copy( 0L, nColonPos );
            pEntry->sName = rAttrName.copy( nColonPos + 1L );
        }

        if( pPrefix )
            *pPrefix = pEntry->sPrefix;
        if( pLocalName )
            *pLocalName = pEntry->sName;

        NameSpaceHash::const_iterator aIter = aNameHash.find( pEntry->sPrefix );
        if ( aIter != aNameHash.end() )
        {
            // found: retrieve namespace key
            nKey = pEntry->nKey = (*aIter).second->nKey;
            if ( pNamespace )
                *pNamespace = (*aIter).second->sName;
        }
        else if ( pEntry->sPrefix == sXMLNS )
            // not found, but xmlns prefix: return xmlns 'namespace'
            nKey = pEntry->nKey = XML_NAMESPACE_XMLNS;
        else if( nColonPos == -1L )
            // not found, and no namespace: 'namespace' none
            nKey = pEntry->nKey = XML_NAMESPACE_NONE;

        const_cast < NameSpaceHash* > ( &aNameCache )->operator[] ( rAttrName ) = pEntry;
    }

    return nKey;
}

sal_uInt16 SvXMLNamespaceMap::GetFirstKey() const
{
    return aNameMap.empty() ? USHRT_MAX : (*aNameMap.begin()).second->nKey;
}

sal_uInt16 SvXMLNamespaceMap::GetNextKey( sal_uInt16 nLastKey ) const
{
    NameSpaceMap::const_iterator aIter = aNameMap.find ( nLastKey );
    return (++aIter == aNameMap.end()) ? USHRT_MAX : (*aIter).second->nKey;
}


// All methods after this are deprecated...

sal_uInt16 SvXMLNamespaceMap::GetKeyByIndex( sal_uInt16 nIdx ) const
{
    return nIdx;
}

sal_uInt16 SvXMLNamespaceMap::GetIndexByKey( sal_uInt16 nKey ) const
{
    return nKey;
}
sal_uInt16 SvXMLNamespaceMap::GetFirstIndex() const
{
    return aNameMap.empty() ? USHRT_MAX : (*aNameMap.begin()).second->nKey;
}

sal_uInt16 SvXMLNamespaceMap::GetNextIndex( sal_uInt16 nOldIdx ) const
{
    NameSpaceMap::const_iterator aIter = aNameMap.find ( nOldIdx );
    return (++aIter == aNameMap.end()) ? USHRT_MAX : (*aIter).second->nKey;
}

sal_Bool SvXMLNamespaceMap::AddAtIndex( sal_uInt16 nIdx, const OUString& rPrefix,
                                    const OUString& rName, sal_uInt16 nKey )
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

sal_Bool SvXMLNamespaceMap::AddAtIndex( sal_uInt16 nIdx, const sal_Char *pPrefix,
                                     const sal_Char *pName, sal_uInt16 nKey )
{
    OUString sPrefix( OUString::createFromAscii(pPrefix) );
    OUString sName( OUString::createFromAscii(pName) );

    return AddAtIndex( nIdx, sPrefix, sName, nKey );
}

OUString SvXMLNamespaceMap::GetAttrNameByIndex( sal_uInt16 nIdx ) const
{
    return GetAttrNameByKey( nIdx );
}

OUString SvXMLNamespaceMap::GetQNameByIndex( sal_uInt16 nIdx,
                                           const OUString& rLocalName ) const
{
    return GetQNameByKey( nIdx, rLocalName );
}

const OUString& SvXMLNamespaceMap::GetPrefixByIndex( sal_uInt16 nIdx ) const
{
    NameSpaceMap::const_iterator aIter = aNameMap.find (nIdx);
    return (aIter != aNameMap.end()) ? (*aIter).second->sPrefix : sEmpty;
}

const OUString& SvXMLNamespaceMap::GetNameByIndex( sal_uInt16 nIdx ) const
{
    NameSpaceMap::const_iterator aIter = aNameMap.find (nIdx);
    return (aIter != aNameMap.end()) ? (*aIter).second->sName : sEmpty;
}

sal_uInt16 SvXMLNamespaceMap::GetIndexByPrefix( const OUString& rPrefix ) const
{
    NameSpaceHash::const_iterator aIter = aNameHash.find(rPrefix);
    return (aIter != aNameHash.end()) ? (*aIter).second->nKey : USHRT_MAX;
}
sal_uInt16 SvXMLNamespaceMap::GetKeyByAttrName(
                            const OUString& rAttrName,
                            OUString *pLocalName,
                            sal_uInt16 nIdxGuess) const
{
    return _GetKeyByAttrName( rAttrName, 0, pLocalName, 0 );
}

sal_uInt16 SvXMLNamespaceMap::GetKeyByAttrName( const OUString& rAttrName,
                                            OUString *pPrefix,
                                            OUString *pLocalName,
                                            OUString *pNamespace,
                                            USHORT nIdxGuess ) const
{
    return _GetKeyByAttrName ( rAttrName, pPrefix, pLocalName, pNamespace );
}
