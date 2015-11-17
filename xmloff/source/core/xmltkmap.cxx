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

#include <set>

using namespace ::xmloff::token;

class SvXMLTokenMapEntry_Impl
{
    sal_uInt16  nPrefixKey;
    OUString    sLocalName;
    sal_uInt16  nToken;

public:

    sal_uInt16 GetToken() const { return nToken; }

    SvXMLTokenMapEntry_Impl( sal_uInt16 nPrefix, const OUString& rLName,
                             sal_uInt16 nTok=XML_TOK_UNKNOWN ) :
        nPrefixKey( nPrefix ),
        sLocalName( rLName  ),
        nToken( nTok )
    {}

    explicit SvXMLTokenMapEntry_Impl( const SvXMLTokenMapEntry& rEntry ) :
        nPrefixKey( rEntry.nPrefixKey ),
        sLocalName( GetXMLToken( rEntry.eLocalName ) ),
        nToken( rEntry.nToken )
    {}

    bool operator<( const SvXMLTokenMapEntry_Impl& r ) const
    {
        return nPrefixKey < r.nPrefixKey ||
               ( nPrefixKey == r.nPrefixKey &&
                 sLocalName < r.sLocalName);
    }
};

class SvXMLTokenMap_Impl : public std::set<SvXMLTokenMapEntry_Impl> {};

SvXMLTokenMap::SvXMLTokenMap( const SvXMLTokenMapEntry *pMap )
    : m_pImpl( new SvXMLTokenMap_Impl )
{
    while( pMap->eLocalName != XML_TOKEN_INVALID )
    {
        m_pImpl->insert(SvXMLTokenMapEntry_Impl( *pMap ));
        pMap++;
    }
}

SvXMLTokenMap::~SvXMLTokenMap()
{
}

sal_uInt16 SvXMLTokenMap::Get( sal_uInt16 nKeyPrefix,
                               const OUString& rLName ) const
{
    SvXMLTokenMapEntry_Impl const* pEntry = nullptr;
    SvXMLTokenMapEntry_Impl aTst( nKeyPrefix, rLName );

    SvXMLTokenMap_Impl::iterator it = m_pImpl->find( aTst );
    if (it != m_pImpl->end())
    {
        pEntry = &*it;
    }

    if( pEntry )
        return pEntry->GetToken();
    else
        return XML_TOK_UNKNOWN;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
