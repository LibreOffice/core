/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <rtl/ustring.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <boost/ptr_container/ptr_set.hpp>

using namespace ::xmloff::token;

using ::rtl::OUString;

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

    SvXMLTokenMapEntry_Impl( const SvXMLTokenMapEntry& rEntry ) :
        nPrefixKey( rEntry.nPrefixKey ),
        sLocalName( GetXMLToken( rEntry.eLocalName ) ),
        nToken( rEntry.nToken )
    {}

    sal_Bool operator==( const SvXMLTokenMapEntry_Impl& r ) const
    {
        return nPrefixKey == r.nPrefixKey &&
               sLocalName == r.sLocalName;
    }

    sal_Bool operator<( const SvXMLTokenMapEntry_Impl& r ) const
    {
        return nPrefixKey < r.nPrefixKey ||
               ( nPrefixKey == r.nPrefixKey &&
                 sLocalName < r.sLocalName);
    }
};

class SvXMLTokenMap_Impl : public boost::ptr_set<SvXMLTokenMapEntry_Impl> {};

// ---------------------------------------------------------------------

SvXMLTokenMapEntry_Impl *SvXMLTokenMap::_Find( sal_uInt16 nKeyPrefix,
                                                const OUString& rLName ) const
{
    SvXMLTokenMapEntry_Impl *pRet = 0;
    SvXMLTokenMapEntry_Impl aTst( nKeyPrefix, rLName );

    SvXMLTokenMap_Impl::iterator it = pImpl->find( aTst );
    if( it != pImpl->end() )
    {
        pRet = &*it;
    }

    return pRet;
}

SvXMLTokenMap::SvXMLTokenMap( const SvXMLTokenMapEntry *pMap ) :
    pImpl( new SvXMLTokenMap_Impl )
{
    while( pMap->eLocalName != XML_TOKEN_INVALID )
    {
        pImpl->insert( new SvXMLTokenMapEntry_Impl( *pMap ) );
        pMap++;
    }
}

SvXMLTokenMap::~SvXMLTokenMap()
{
    delete pImpl;
}

sal_uInt16 SvXMLTokenMap::Get( sal_uInt16 nKeyPrefix,
                               const OUString& rLName ) const
{
    SvXMLTokenMapEntry_Impl *pEntry = _Find( nKeyPrefix, rLName );
    if( pEntry )
        return pEntry->GetToken();
    else
        return XML_TOK_UNKNOWN;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
