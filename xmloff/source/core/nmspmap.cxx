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
#include "precompiled_xmloff.hxx"
#include <tools/debug.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#ifndef _XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#include <xmloff/nmspmap.hxx>

#include "xmloff/xmlnmspe.hxx"


using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::xmloff::token;

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

void SvXMLNamespaceMap::operator=( const SvXMLNamespaceMap& rMap )
{
    aNameHash = rMap.aNameHash;
    aNameMap = rMap.aNameMap;
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
    ::vos::ORef<NameSpaceEntry> pEntry(new NameSpaceEntry);
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

sal_uInt16 SvXMLNamespaceMap::AddIfKnown( const OUString& rPrefix, const OUString& rName )
{
    sal_uInt16 nKey = GetKeyByName( rName );

    DBG_ASSERT( XML_NAMESPACE_NONE != nKey,
                "SvXMLNamespaceMap::AddIfKnown: invalid namespace key" );

    if( XML_NAMESPACE_NONE == nKey )
        return XML_NAMESPACE_UNKNOWN;

    if( XML_NAMESPACE_UNKNOWN != nKey )
    {
        NameSpaceHash::const_iterator aIter = aNameHash.find( rPrefix );
        if( aIter == aNameHash.end() || (*aIter).second->sName != rName )
            nKey = _Add( rPrefix, rName, nKey );
    }

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
        const ::rtl::OUString & prefix( (*aIter).second->sPrefix );
        if (prefix.getLength()) // not default namespace
        {
            sAttrName.append( sal_Unicode(':') );
            sAttrName.append( prefix );
        }
    }
    return sAttrName.makeStringAndClear();
}

OUString SvXMLNamespaceMap::GetQNameByKey( sal_uInt16 nKey,
                            const OUString& rLocalName,
                            sal_Bool bCache) const
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
        case XML_NAMESPACE_XMLNS:
        {
            // ...if it's in the xmlns namespace, make the prefix
            // don't bother caching this, it rarely happens
            OUStringBuffer sQName;
            sQName.append ( sXMLNS );
            if (rLocalName.getLength()) // not default namespace
            {
                sQName.append ( sal_Unicode(':') );
                sQName.append ( rLocalName );
            }
            return sQName.makeStringAndClear();;
        }
        case XML_NAMESPACE_XML:
        {
            // this namespace is reserved, and needs not to be declared
            OUStringBuffer sQName;
            sQName.append ( GetXMLToken(XML_XML) );
            sQName.append ( sal_Unicode(':') );
            sQName.append ( rLocalName );
            return sQName.makeStringAndClear();;
        }
        default:
        {
            QNameCache::const_iterator aQCacheIter;
            if (bCache)
                aQCacheIter = aQNameCache.find ( QNamePair ( nKey, &rLocalName ) );
            else
                aQCacheIter = aQNameCache.end();
            if ( aQCacheIter != aQNameCache.end() )
                return (*aQCacheIter).second;
            else
            {
                NameSpaceMap::const_iterator aIter = aNameMap.find ( nKey );
                if ( aIter != aNameMap.end() )
                {
                    OUStringBuffer sQName;
                    // ...if it's in our map, make the prefix
                    const OUString & prefix( (*aIter).second->sPrefix );
                    if (prefix.getLength()) // not default namespace
                    {
                        sQName.append( prefix );
                        sQName.append( sal_Unicode(':') );
                    }
                    sQName.append ( rLocalName );
                    if (bCache)
                    {
                        OUString sString(sQName.makeStringAndClear());
                        OUString *pString = new OUString ( rLocalName );
                        const_cast < QNameCache * > (&aQNameCache)->operator[] ( QNamePair ( nKey, pString ) ) = sString;
                        return sString;
                    }
                    else
                        return sQName.makeStringAndClear();
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
                            OUString *pLocalName,
                            sal_Bool bCache) const
{
    return _GetKeyByAttrName( rAttrName, 0, pLocalName, 0, bCache );
}

sal_uInt16 SvXMLNamespaceMap::_GetKeyByAttrName( const OUString& rAttrName,
                                            OUString *pPrefix,
                                            OUString *pLocalName,
                                            OUString *pNamespace,
                                            sal_Bool bCache) const
{
    sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN;

    NameSpaceHash::const_iterator it;
    if (bCache)
        it = aNameCache.find ( rAttrName );
    else
        it = aNameCache.end();
    if ( it != aNameCache.end() )
    {
        const NameSpaceEntry &rEntry = (*it).second.getBody();
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
    vos::ORef<NameSpaceEntry> xEntry(new NameSpaceEntry());

        sal_Int32 nColonPos = rAttrName.indexOf( sal_Unicode(':') );
        if( -1L == nColonPos )
        {
            // case: no ':' found -> default namespace
            xEntry->sPrefix = OUString();
            xEntry->sName = rAttrName;
        }
        else
        {
            // normal case: ':' found -> get prefix/suffix
            xEntry->sPrefix = rAttrName.copy( 0L, nColonPos );
            xEntry->sName = rAttrName.copy( nColonPos + 1L );
        }

        if( pPrefix )
            *pPrefix = xEntry->sPrefix;
        if( pLocalName )
            *pLocalName = xEntry->sName;

        NameSpaceHash::const_iterator aIter = aNameHash.find( xEntry->sPrefix );
        if ( aIter != aNameHash.end() )
        {
            // found: retrieve namespace key
            nKey = xEntry->nKey = (*aIter).second->nKey;
            if ( pNamespace )
                *pNamespace = (*aIter).second->sName;
        }
        else if ( xEntry->sPrefix == sXMLNS )
            // not found, but xmlns prefix: return xmlns 'namespace'
            nKey = xEntry->nKey = XML_NAMESPACE_XMLNS;
        else if( nColonPos == -1L )
            // not found, and no namespace: 'namespace' none
            nKey = xEntry->nKey = XML_NAMESPACE_NONE;

        if (bCache)
    {
        typedef std::pair< const rtl::OUString, vos::ORef<NameSpaceEntry> > value_type;
        (void) const_cast<NameSpaceHash*>(&aNameCache)->insert (value_type (rAttrName, xEntry));
    }
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

sal_Bool SvXMLNamespaceMap::AddAtIndex( sal_uInt16 /*nIdx*/, const OUString& rPrefix,
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
                            sal_uInt16 /*nIdxGuess*/) const
{
    return _GetKeyByAttrName( rAttrName, 0, pLocalName, 0 );
}

sal_uInt16 SvXMLNamespaceMap::GetKeyByAttrName( const OUString& rAttrName,
                                            OUString *pPrefix,
                                            OUString *pLocalName,
                                            OUString *pNamespace,
                                            sal_uInt16 /*nIdxGuess*/ ) const
{
    return _GetKeyByAttrName ( rAttrName, pPrefix, pLocalName, pNamespace );
}

sal_Bool SvXMLNamespaceMap::NormalizeURI( ::rtl::OUString& rName )
{
    // try OASIS + W3 URI normalization
    sal_Bool bSuccess = NormalizeOasisURN( rName );
    if( ! bSuccess )
        bSuccess = NormalizeW3URI( rName );
    return bSuccess;
}

sal_Bool SvXMLNamespaceMap::NormalizeW3URI( ::rtl::OUString& rName )
{
    // check if URI matches:
    // http://www.w3.org/[0-9]*/[:letter:]*
    //                   (year)/(WG name)
    // For the following WG/standards names:
    // - xforms

    sal_Bool bSuccess = sal_False;
    const OUString sURIPrefix = GetXMLToken( XML_URI_W3_PREFIX );
    if( rName.compareTo( sURIPrefix, sURIPrefix.getLength() ) == 0 )
    {
        const OUString sURISuffix = GetXMLToken( XML_URI_XFORMS_SUFFIX );
        sal_Int32 nCompareFrom = rName.getLength() - sURISuffix.getLength();
        if( rName.copy( nCompareFrom ).equals( sURISuffix ) )
        {
            // found W3 prefix, and xforms suffix
            rName = GetXMLToken( XML_N_XFORMS_1_0 );
            bSuccess = sal_True;
        }
    }
    return bSuccess;
}

sal_Bool SvXMLNamespaceMap::NormalizeOasisURN( ::rtl::OUString& rName )
{
    // #i38644#
    // we exported the wrong namespace for smil, so we correct this here on load
    // for older documents
    if( IsXMLToken( rName, ::xmloff::token::XML_N_SVG ) )
    {
        rName = GetXMLToken( ::xmloff::token::XML_N_SVG_COMPAT );
        return sal_True;
    }
    else if( IsXMLToken( rName, ::xmloff::token::XML_N_FO ) )
    {
        rName = GetXMLToken( ::xmloff::token::XML_N_FO_COMPAT );
        return sal_True;
    }
    else if( IsXMLToken( rName, ::xmloff::token::XML_N_SMIL ) ||
               IsXMLToken( rName, ::xmloff::token::XML_N_SMIL_OLD )  )
    {
        rName = GetXMLToken( ::xmloff::token::XML_N_SMIL_COMPAT );
        return sal_True;
    }

    //
    // Check if URN matches
    // :urn:oasis:names:tc:[^:]*:xmlns:[^:]*:1.[^:]*
    //                     |---|       |---| |-----|
    //                     TC-Id      Sub-Id Version

    sal_Int32 nNameLen = rName.getLength();
    // :urn:oasis:names:tc.*
    const OUString& rOasisURN = GetXMLToken( XML_URN_OASIS_NAMES_TC );
    if( 0 != rName.compareTo( rOasisURN, rOasisURN.getLength() ) )
        return sal_False;

    // :urn:oasis:names:tc:.*
    sal_Int32 nPos = rOasisURN.getLength();
    if( nPos >= nNameLen || rName[nPos] != ':' )
        return sal_False;

    // :urn:oasis:names:tc:[^:]:.*
    sal_Int32 nTCIdStart = nPos+1;
    sal_Int32 nTCIdEnd = rName.indexOf( ':', nTCIdStart );
    if( -1 == nTCIdEnd )
        return sal_False;

    // :urn:oasis:names:tc:[^:]:xmlns.*
    nPos = nTCIdEnd + 1;
    OUString sTmp( rName.copy( nPos ) );
    const OUString& rXMLNS = GetXMLToken( XML_XMLNS );
    if( 0!= sTmp.compareTo( rXMLNS, rXMLNS.getLength() ) )
        return sal_False;

    // :urn:oasis:names:tc:[^:]:xmlns:.*
    nPos += rXMLNS.getLength();
    if( nPos >= nNameLen || rName[nPos] != ':' )
        return sal_False;

    // :urn:oasis:names:tc:[^:]:xmlns:[^:]*:.*
    nPos = rName.indexOf( ':', nPos+1 );
    if( -1 == nPos )
        return sal_False;

    // :urn:oasis:names:tc:[^:]:xmlns:[^:]*:[^:][^:][^:][^:]*
    sal_Int32 nVersionStart = nPos+1;
    if( nVersionStart+2 >= nNameLen ||
        -1 != rName.indexOf( ':', nVersionStart ) )
        return sal_False;

    // :urn:oasis:names:tc:[^:]:xmlns:[^:]*:1\.[^:][^:]*
    if( rName[nVersionStart] != '1' || rName[nVersionStart+1] != '.' )
        return sal_False;

    // replace [tcid] with current TCID and version with current version.
    OUStringBuffer aNewName( nNameLen +20 );
    aNewName.append( rName.copy( 0, nTCIdStart ) );
    aNewName.append( GetXMLToken( XML_OPENDOCUMENT ) );
    aNewName.append( rName.copy( nTCIdEnd, nVersionStart-nTCIdEnd ) );
    aNewName.append( GetXMLToken( XML_1_0 ) );

    rName = aNewName.makeStringAndClear();

    return sal_True;
}
