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

#include <rtl/ustring.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <utility>

using namespace ::xmloff::token;

class SvXMLTokenMap_Impl
{
private:
    struct PairHash
    {
        std::size_t operator()(const std::pair<sal_uInt16,OUString> &pair) const
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, pair.first);
            boost::hash_combine(seed, pair.second.hashCode());
            return seed;
        }
    };
    std::unordered_map< std::pair<sal_uInt16, OUString>,
                        sal_uInt16, PairHash> m_aPrefixAndNameToTokenMap;
    std::unordered_map< sal_Int32, sal_uInt16> m_aFastTokenToTokenMap;

public:
    void insert( const SvXMLTokenMapEntry& rEntry );
    sal_uInt16 get( sal_uInt16 nKeyPrefix, const OUString& rLName ) const;
    sal_uInt16 get( sal_Int32 nFastTok ) const;
};

void SvXMLTokenMap_Impl::insert( const SvXMLTokenMapEntry& rEntry )
{
    bool inserted = m_aPrefixAndNameToTokenMap.insert( std::make_pair( std::make_pair( rEntry.nPrefixKey,
                                       GetXMLToken( rEntry.eLocalName ) ), rEntry.nToken ) ).second;
    assert(inserted && "duplicate token");
    (void)inserted;
    if( rEntry.nFastToken )
    {
        bool inserted2 = m_aFastTokenToTokenMap.insert( std::make_pair( rEntry.nFastToken, rEntry.nToken ) ).second;
        assert(inserted2 && "duplicate token");
        (void)inserted2;
    }
}

sal_uInt16 SvXMLTokenMap_Impl::get( sal_uInt16 nKeyPrefix, const OUString& rLName ) const
{
    auto aIter( m_aPrefixAndNameToTokenMap.find( std::make_pair( nKeyPrefix, rLName ) ) );
    if ( aIter != m_aPrefixAndNameToTokenMap.end() )
        return (*aIter).second;
    else
        return XML_TOK_UNKNOWN;
}

sal_uInt16 SvXMLTokenMap_Impl::get( sal_Int32 nFastTok ) const
{
    auto aIter( m_aFastTokenToTokenMap.find( nFastTok ) );
    if ( aIter != m_aFastTokenToTokenMap.end() )
        return (*aIter).second;
    else
        return XML_TOK_UNKNOWN;
}

SvXMLTokenMap::SvXMLTokenMap( const SvXMLTokenMapEntry *pMap )
    : m_pImpl( new SvXMLTokenMap_Impl )
{
    while( pMap->eLocalName != XML_TOKEN_INVALID )
    {
        m_pImpl->insert( *pMap );
        pMap++;
    }
}

SvXMLTokenMap::~SvXMLTokenMap()
{
}

sal_uInt16 SvXMLTokenMap::Get( sal_uInt16 nKeyPrefix,
                               const OUString& rLName ) const
{
    return m_pImpl->get( nKeyPrefix, rLName );
}

sal_uInt16 SvXMLTokenMap::Get( sal_Int32 nFastTok ) const
{
    return m_pImpl->get( nFastTok );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
