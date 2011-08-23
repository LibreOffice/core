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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

namespace binfilter {

using namespace rtl;
using namespace ::binfilter::xmloff::token;

class SvXMLTokenMapEntry_Impl
{
    sal_uInt16	nPrefixKey;
    OUString	sLocalName;
    sal_uInt16	nToken;

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

    BOOL operator==( const SvXMLTokenMapEntry_Impl& r ) const
    {
        return nPrefixKey == r.nPrefixKey &&
               sLocalName == r.sLocalName;
    }

    BOOL operator<( const SvXMLTokenMapEntry_Impl& r ) const
    {
        return nPrefixKey < r.nPrefixKey ||
               ( nPrefixKey == r.nPrefixKey &&
                 sLocalName < r.sLocalName);
    }
};

typedef SvXMLTokenMapEntry_Impl *SvXMLTokenMapEntry_ImplPtr;
SV_DECL_PTRARR_SORT_DEL( SvXMLTokenMap_Impl, SvXMLTokenMapEntry_ImplPtr, 5, 5 )
SV_IMPL_OP_PTRARR_SORT( SvXMLTokenMap_Impl, SvXMLTokenMapEntry_ImplPtr )

// ---------------------------------------------------------------------

SvXMLTokenMapEntry_Impl *SvXMLTokenMap::_Find( sal_uInt16 nKeyPrefix,
                                                const OUString& rLName ) const
{
    SvXMLTokenMapEntry_Impl *pRet = 0;
    SvXMLTokenMapEntry_Impl aTst( nKeyPrefix, rLName );

    sal_uInt16 nPos;
    if( pImpl->Seek_Entry( &aTst, &nPos ) )
    {
        pRet = (*pImpl)[nPos];
    }

    return pRet;
}

SvXMLTokenMap::SvXMLTokenMap( SvXMLTokenMapEntry *pMap ) :
    pImpl( new SvXMLTokenMap_Impl )
{
    while( pMap->eLocalName != XML_TOKEN_INVALID )
    {
        pImpl->Insert( new SvXMLTokenMapEntry_Impl( *pMap ) );
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


}//end of namespace binfilter
