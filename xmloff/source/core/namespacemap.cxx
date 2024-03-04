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

#include <sal/config.h>

#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/namespacemap.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <o3tl/string_view.hxx>


using namespace ::xmloff::token;

/* The basic idea of this class is that we have two ways to search our
 * data, by prefix and by key. We use an unordered_map for fast prefix
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

const OUString sEmpty;

SvXMLNamespaceMap::SvXMLNamespaceMap()
: m_sXMLNS( GetXMLToken ( XML_XMLNS ) )
{
    // approx worst-case size
    m_aNameHash.reserve(20);
    maKeyToNamespaceMap.reserve(20);
}

SvXMLNamespaceMap::SvXMLNamespaceMap( const SvXMLNamespaceMap& rMap )
: m_sXMLNS( GetXMLToken ( XML_XMLNS ) )
{
    m_aNameHash = rMap.m_aNameHash;
    maKeyToNamespaceMap  = rMap.maKeyToNamespaceMap;
}

SvXMLNamespaceMap& SvXMLNamespaceMap::operator=( const SvXMLNamespaceMap& rMap )
{
    m_aNameHash = rMap.m_aNameHash;
    maKeyToNamespaceMap = rMap.maKeyToNamespaceMap;
    return *this;
}

SvXMLNamespaceMap::~SvXMLNamespaceMap()
{
}

void SvXMLNamespaceMap::Clear()
{
    m_aNameHash.clear();
    m_aNameCache.clear();
    maKeyToNamespaceMap.clear();
    m_aQNameCache.clear();
}


bool SvXMLNamespaceMap::operator ==( const SvXMLNamespaceMap& rCmp ) const
{
    return m_aNameHash == rCmp.m_aNameHash;
}

sal_uInt16 SvXMLNamespaceMap::Add_( const OUString& rPrefix, const OUString &rName, sal_uInt16 nKey )
{
    if( XML_NAMESPACE_UNKNOWN == nKey )
    {
        // create a new unique key with UNKNOWN flag set
        nKey = XML_NAMESPACE_UNKNOWN_FLAG;
        do
        {
            auto aIter = maKeyToNamespaceMap.find ( nKey );
            if( aIter == maKeyToNamespaceMap.end() )
                break;
            nKey++;
        }
        while ( true );
    }
    m_aNameHash.insert_or_assign( rPrefix, NameSpaceEntry{ rName, rPrefix, nKey} );
    maKeyToNamespaceMap.insert_or_assign( nKey, KeyToNameSpaceMapEntry{ rName, rPrefix} );
    return nKey;
}

sal_uInt16 SvXMLNamespaceMap::Add( const OUString& rPrefix, const OUString& rName,
                               sal_uInt16 nKey )
{
    if( XML_NAMESPACE_UNKNOWN == nKey )
        nKey = GetKeyByName( rName );

#ifdef NDEBUG
    if( XML_NAMESPACE_NONE == nKey )
        return USHRT_MAX;
#else
    assert(XML_NAMESPACE_NONE != nKey);
#endif

    if ( m_aNameHash.find ( rPrefix ) == m_aNameHash.end() )
        nKey = Add_( rPrefix, rName, nKey );

    return nKey;
}

sal_uInt16 SvXMLNamespaceMap::AddIfKnown( const OUString& rPrefix, const OUString& rName )
{
    sal_uInt16 nKey = GetKeyByName( rName );

#ifdef NDEBUG
    if( XML_NAMESPACE_NONE == nKey )
        return XML_NAMESPACE_UNKNOWN;
#else
    assert(nKey != XML_NAMESPACE_NONE);
#endif

    if( XML_NAMESPACE_UNKNOWN != nKey )
    {
        NameSpaceHash::const_iterator aIter = m_aNameHash.find( rPrefix );
        if( aIter == m_aNameHash.end() || (*aIter).second.m_sName != rName )
            nKey = Add_( rPrefix, rName, nKey );
    }

    return nKey;
}


sal_uInt16 SvXMLNamespaceMap::GetKeyByPrefix( const OUString& rPrefix ) const
{
    NameSpaceHash::const_iterator aIter = m_aNameHash.find(rPrefix);
    return (aIter != m_aNameHash.end()) ? (*aIter).second.m_nKey : USHRT_MAX;
}

sal_uInt16 SvXMLNamespaceMap::GetKeyByName( const OUString& rName ) const
{
    sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN;
    auto aIter = std::find_if(m_aNameHash.cbegin(), m_aNameHash.cend(),
        [&rName](const NameSpaceHash::value_type& rEntry) { return rEntry.second.m_sName == rName; });

    if (aIter != m_aNameHash.cend())
        nKey = (*aIter).second.m_nKey;

    return nKey;
}

const OUString& SvXMLNamespaceMap::GetPrefixByKey( sal_uInt16 nKey ) const
{
    auto aIter = maKeyToNamespaceMap.find (nKey);
    return (aIter != maKeyToNamespaceMap.end()) ? (*aIter).second.sPrefix : sEmpty;
}

const OUString& SvXMLNamespaceMap::GetNameByKey( sal_uInt16 nKey ) const
{
    auto aIter = maKeyToNamespaceMap.find (nKey);
    return (aIter != maKeyToNamespaceMap.end()) ? (*aIter).second.sName : sEmpty;
}

OUString SvXMLNamespaceMap::GetAttrNameByKey( sal_uInt16 nKey ) const
{
    auto aIter = maKeyToNamespaceMap.find ( nKey );
    if (aIter == maKeyToNamespaceMap.end())
        return OUString();

    const OUString & prefix( (*aIter).second.sPrefix );
    if (prefix.isEmpty()) // default namespace
        return m_sXMLNS;

    return m_sXMLNS + ":" + prefix;
}

OUString SvXMLNamespaceMap::GetQNameByKey( sal_uInt16 nKey,
                            const OUString& rLocalName,
                            bool bCache) const
{
    // We always want to return at least the rLocalName...

    switch ( nKey )
    {
        case XML_NAMESPACE_UNKNOWN:
            // ...if it's a completely unknown namespace, assert and return the local name
            SAL_WARN("xmloff.core", "unknown namespace, probable missing xmlns: declaration");
            [[fallthrough]];
        case XML_NAMESPACE_NONE:
            // ...if there isn't one, return the local name
            return rLocalName;
        case XML_NAMESPACE_XMLNS:
        {
            // ...if it's in the xmlns namespace, make the prefix
            // don't bother caching this, it rarely happens
            if (!rLocalName.isEmpty()) // not default namespace
                return m_sXMLNS + ":" + rLocalName;
            else
                return m_sXMLNS;
        }
        case XML_NAMESPACE_XML:
        {
            // this namespace is reserved, and needs not to be declared
            return GetXMLToken(XML_XML) + ":" + rLocalName;
        }
        default:
        {
            QNameCache::const_iterator aQCacheIter;
            if (bCache)
                aQCacheIter = m_aQNameCache.find ( QNamePair ( nKey, rLocalName ) );
            else
                aQCacheIter = m_aQNameCache.end();
            if ( aQCacheIter != m_aQNameCache.end() )
                return (*aQCacheIter).second;
            else
            {
                auto aIter = maKeyToNamespaceMap.find ( nKey );
                if ( aIter != maKeyToNamespaceMap.end() )
                {
                    // ...if it's in our map, make the prefix
                    const OUString & prefix( (*aIter).second.sPrefix );
                    OUString sQName;
                    if (!prefix.isEmpty()) // not default namespace
                        sQName = prefix + ":" + rLocalName;
                    else
                        sQName = rLocalName;
                    if (bCache)
                        m_aQNameCache.emplace(QNamePair(nKey, rLocalName), sQName);
                    return sQName;
                }
                else
                {
                    // ... if it isn't, this is a Bad Thing, assert and return the local name
                    assert(false);
                    return rLocalName;
                }
            }
        }
    }
}

sal_uInt16 SvXMLNamespaceMap::GetKeyByAttrValueQName(
                            const OUString& rAttrValue,
                            OUString *pLocalName) const
{
    return GetKeyByQName(rAttrValue, nullptr, pLocalName, nullptr, QNameMode::AttrValue);
}

/**
 @param rQName           either attribute name or qualified/namespaced attribute value
 @param bCacheAttrName   true: rQName is element or attribute name, cache it
                         false: rQName is attribute value, may contain extra ':', don't cache it
 */
sal_uInt16 SvXMLNamespaceMap::GetKeyByQName(const OUString& rQName,
                                            OUString *pPrefix,
                                            OUString *pLocalName,
                                            OUString *pNamespace,
                                            QNameMode const eMode) const
{
    sal_uInt16 nKey;

    NameSpaceHash::const_iterator it;
    if (eMode == QNameMode::AttrNameCached)
        it = m_aNameCache.find ( rQName );
    else
        it = m_aNameCache.end();
    if ( it != m_aNameCache.end() )
    {
        const NameSpaceEntry &rEntry = (*it).second;
        if ( pPrefix )
            *pPrefix = rEntry.m_sPrefix;
        if ( pLocalName )
            *pLocalName = rEntry.m_sName;
        nKey = rEntry.m_nKey;
        if ( pNamespace )
        {
            auto aMapIter = maKeyToNamespaceMap.find (nKey);
            *pNamespace = aMapIter != maKeyToNamespaceMap.end() ? (*aMapIter).second.sName : OUString();
        }
    }
    else
    {
        OUString sEntryPrefix, sEntryName;

        sal_Int32 nColonPos = rQName.indexOf( ':' );
        if( -1 == nColonPos )
        {
            // case: no ':' found -> default namespace
            sEntryName = rQName;
        }
        else
        {
            // normal case: ':' found -> get prefix/suffix
            sEntryPrefix = rQName.copy( 0, nColonPos );
            sEntryName = rQName.copy( nColonPos + 1 );
        }

        if (eMode == QNameMode::AttrNameCached && sEntryName.indexOf(':') != -1)
        {
            SAL_INFO("xmloff", "invalid attribute name with multiple ':'");
            assert(false);
            return XML_NAMESPACE_UNKNOWN;
        }

        if( pPrefix )
            *pPrefix = sEntryPrefix;
        if( pLocalName )
            *pLocalName = sEntryName;

        NameSpaceHash::const_iterator aIter = m_aNameHash.find( sEntryPrefix );
        if ( aIter != m_aNameHash.end() )
        {
            // found: retrieve namespace key
            nKey = (*aIter).second.m_nKey;
            if ( pNamespace )
                *pNamespace = (*aIter).second.m_sName;
        }
        else if ( sEntryPrefix == m_sXMLNS )
            // not found, but xmlns prefix: return xmlns 'namespace'
            nKey = XML_NAMESPACE_XMLNS;
        else if( nColonPos == -1 )
            // not found, and no namespace: 'namespace' none
            nKey = XML_NAMESPACE_NONE;
        else
            nKey = XML_NAMESPACE_UNKNOWN;

        if (eMode == QNameMode::AttrNameCached)
        {
            m_aNameCache.insert_or_assign(rQName, NameSpaceEntry{std::move(sEntryName), std::move(sEntryPrefix), nKey});
        }
    }

    return nKey;
}

sal_uInt16 SvXMLNamespaceMap::GetFirstKey() const
{
    return maKeyToNamespaceMap.empty() ? USHRT_MAX : (*maKeyToNamespaceMap.begin()).first;
}

sal_uInt16 SvXMLNamespaceMap::GetNextKey( sal_uInt16 nLastKey ) const
{
    auto aIter = maKeyToNamespaceMap.find ( nLastKey );
    assert(aIter != maKeyToNamespaceMap.end());
    return (++aIter == maKeyToNamespaceMap.end()) ? USHRT_MAX : (*aIter).first;
}


// All methods after this are deprecated...

sal_uInt16 SvXMLNamespaceMap::GetIndexByKey( sal_uInt16 nKey )
{
    return nKey;
}
sal_uInt16 SvXMLNamespaceMap::GetFirstIndex() const
{
    return maKeyToNamespaceMap.empty() ? USHRT_MAX : (*maKeyToNamespaceMap.begin()).first;
}

sal_uInt16 SvXMLNamespaceMap::GetNextIndex( sal_uInt16 nOldIdx ) const
{
    auto aIter = maKeyToNamespaceMap.find ( nOldIdx );
    assert(aIter != maKeyToNamespaceMap.end());
    return (++aIter == maKeyToNamespaceMap.end()) ? USHRT_MAX : (*aIter).first;
}

void SvXMLNamespaceMap::AddAtIndex( const OUString& rPrefix,
                                    const OUString& rName, sal_uInt16 nKey )
{
    if( XML_NAMESPACE_UNKNOWN == nKey )
        nKey = GetKeyByName( rName );

    assert(XML_NAMESPACE_NONE != nKey);
    if( XML_NAMESPACE_NONE != nKey && ! ( m_aNameHash.count ( rPrefix ) ) )
    {
        Add_( rPrefix, rName, nKey );
    }
}

OUString SvXMLNamespaceMap::GetAttrNameByIndex( sal_uInt16 nIdx ) const
{
    return GetAttrNameByKey( nIdx );
}

const OUString& SvXMLNamespaceMap::GetPrefixByIndex( sal_uInt16 nIdx ) const
{
    auto aIter = maKeyToNamespaceMap.find (nIdx);
    return (aIter != maKeyToNamespaceMap.end()) ? (*aIter).second.sPrefix : sEmpty;
}

const OUString& SvXMLNamespaceMap::GetNameByIndex( sal_uInt16 nIdx ) const
{
    auto aIter = maKeyToNamespaceMap.find (nIdx);
    return (aIter != maKeyToNamespaceMap.end()) ? (*aIter).second.sName : sEmpty;
}

sal_uInt16 SvXMLNamespaceMap::GetIndexByPrefix( const OUString& rPrefix ) const
{
    NameSpaceHash::const_iterator aIter = m_aNameHash.find(rPrefix);
    return (aIter != m_aNameHash.end()) ? (*aIter).second.m_nKey : USHRT_MAX;
}
sal_uInt16 SvXMLNamespaceMap::GetKeyByAttrName(
                            const OUString& rAttrName,
                            OUString *pLocalName) const
{
    return GetKeyByQName(rAttrName, nullptr, pLocalName, nullptr, QNameMode::AttrNameCached);
}

sal_uInt16 SvXMLNamespaceMap::GetKeyByAttrName( const OUString& rAttrName,
                                            OUString *pPrefix,
                                            OUString *pLocalName,
                                            OUString *pNamespace ) const
{
    return GetKeyByQName(rAttrName, pPrefix, pLocalName, pNamespace, QNameMode::AttrNameCached);
}

bool SvXMLNamespaceMap::NormalizeURI( OUString& rName )
{
    // try OASIS + W3 URI normalization
    bool bSuccess = NormalizeOasisURN( rName );
    if( ! bSuccess )
        bSuccess = NormalizeW3URI( rName );
    return bSuccess;
}

bool SvXMLNamespaceMap::NormalizeW3URI( OUString& rName )
{
    // check if URI matches:
    // http://www.w3.org/[0-9]*/[:letter:]*
    //                   (year)/(WG name)
    // For the following WG/standards names:
    // - xforms

    bool bSuccess = false;
    const OUString& sURIPrefix = GetXMLToken( XML_URI_W3_PREFIX );
    if( rName.startsWith( sURIPrefix ) )
    {
        const OUString& sURISuffix = GetXMLToken( XML_URI_XFORMS_SUFFIX );
        sal_Int32 nCompareFrom = rName.getLength() - sURISuffix.getLength();
        if( rName.subView( nCompareFrom ) == sURISuffix )
        {
            // found W3 prefix, and xforms suffix
            rName = GetXMLToken( XML_N_XFORMS_1_0 );
            bSuccess = true;
        }
    }
    return bSuccess;
}

bool SvXMLNamespaceMap::NormalizeOasisURN( OUString& rName )
{
    // #i38644#
    // we exported the wrong namespace for smil, so we correct this here on load
    // for older documents
    if( IsXMLToken( rName, ::xmloff::token::XML_N_SVG ) )
    {
        rName = GetXMLToken( ::xmloff::token::XML_N_SVG_COMPAT );
        return true;
    }
    else if( IsXMLToken( rName, ::xmloff::token::XML_N_FO ) )
    {
        rName = GetXMLToken( ::xmloff::token::XML_N_FO_COMPAT );
        return true;
    }
    else if( IsXMLToken( rName, ::xmloff::token::XML_N_SMIL ) ||
               IsXMLToken( rName, ::xmloff::token::XML_N_SMIL_OLD )  )
    {
        rName = GetXMLToken( ::xmloff::token::XML_N_SMIL_COMPAT );
        return true;
    }


    // Check if URN matches
    // :urn:oasis:names:tc:[^:]*:xmlns:[^:]*:1.[^:]*
    //                     |---|       |---| |-----|
    //                     TC-Id      Sub-Id Version

    sal_Int32 nNameLen = rName.getLength();
    // :urn:oasis:names:tc.*
    const OUString& rOasisURN = GetXMLToken( XML_URN_OASIS_NAMES_TC );
    if( !rName.startsWith( rOasisURN ) )
        return false;

    // :urn:oasis:names:tc:.*
    sal_Int32 nPos = rOasisURN.getLength();
    if( nPos >= nNameLen || rName[nPos] != ':' )
        return false;

    // :urn:oasis:names:tc:[^:]:.*
    sal_Int32 nTCIdStart = nPos+1;
    sal_Int32 nTCIdEnd = rName.indexOf( ':', nTCIdStart );
    if( -1 == nTCIdEnd )
        return false;

    // :urn:oasis:names:tc:[^:]:xmlns.*
    nPos = nTCIdEnd + 1;
    std::u16string_view sTmp( rName.subView( nPos ) );
    const OUString& rXMLNS = GetXMLToken( XML_XMLNS );
    if( !o3tl::starts_with(sTmp, rXMLNS ) )
        return false;

    // :urn:oasis:names:tc:[^:]:xmlns:.*
    nPos += rXMLNS.getLength();
    if( nPos >= nNameLen || rName[nPos] != ':' )
        return false;

    // :urn:oasis:names:tc:[^:]:xmlns:[^:]*:.*
    nPos = rName.indexOf( ':', nPos+1 );
    if( -1 == nPos )
        return false;

    // :urn:oasis:names:tc:[^:]:xmlns:[^:]*:[^:][^:][^:][^:]*
    sal_Int32 nVersionStart = nPos+1;
    if( nVersionStart+2 >= nNameLen ||
        -1 != rName.indexOf( ':', nVersionStart ) )
        return false;

    // :urn:oasis:names:tc:[^:]:xmlns:[^:]*:1\.[^:][^:]*
    if( rName[nVersionStart] != '1' || rName[nVersionStart+1] != '.' )
        return false;

    // replace [tcid] with current TCID and version with current version.

    rName = rName.subView( 0, nTCIdStart ) +
            GetXMLToken( XML_OPENDOCUMENT ) +
            rName.subView( nTCIdEnd, nVersionStart-nTCIdEnd ) +
            GetXMLToken( XML_1_0 );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
