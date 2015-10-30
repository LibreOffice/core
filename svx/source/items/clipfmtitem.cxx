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

#include <vector>

#include <svx/clipfmtitem.hxx>
#include <com/sun/star/frame/status/ClipboardFormats.hpp>

struct SvxClipboardFormatItem_Impl
{
    std::vector<OUString> aFmtNms;
    std::vector<SotClipboardFormatId> aFmtIds;

    SvxClipboardFormatItem_Impl() {}
    SvxClipboardFormatItem_Impl( const SvxClipboardFormatItem_Impl& );
};

TYPEINIT1_FACTORY( SvxClipboardFormatItem, SfxPoolItem , new  SvxClipboardFormatItem(0));

SvxClipboardFormatItem_Impl::SvxClipboardFormatItem_Impl(
                            const SvxClipboardFormatItem_Impl& rCpy )
    : aFmtNms(rCpy.aFmtNms)
    , aFmtIds(rCpy.aFmtIds)
{
}

SvxClipboardFormatItem::SvxClipboardFormatItem( sal_uInt16 nId )
    : SfxPoolItem( nId ), pImpl( new SvxClipboardFormatItem_Impl )
{
}

SvxClipboardFormatItem::SvxClipboardFormatItem( const SvxClipboardFormatItem& rCpy )
    : SfxPoolItem( rCpy.Which() ),
    pImpl( new SvxClipboardFormatItem_Impl( *rCpy.pImpl ) )
{
}

SvxClipboardFormatItem::~SvxClipboardFormatItem()
{
    delete pImpl;
}

bool SvxClipboardFormatItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    sal_uInt16 nCount = Count();

    css::frame::status::ClipboardFormats aClipFormats;

    aClipFormats.Identifiers.realloc( nCount );
    aClipFormats.Names.realloc( nCount );
    for ( sal_uInt16 n=0; n < nCount; n++ )
    {
        aClipFormats.Identifiers[n] = (sal_Int64)GetClipbrdFormatId( n );
        aClipFormats.Names[n] = GetClipbrdFormatName( n );
    }

    rVal <<= aClipFormats;
    return true;
}

bool SvxClipboardFormatItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    css::frame::status::ClipboardFormats aClipFormats;
    if ( rVal >>= aClipFormats )
    {
        sal_uInt16 nCount = sal_uInt16( aClipFormats.Identifiers.getLength() );

        pImpl->aFmtIds.clear();
        pImpl->aFmtNms.clear();
        for ( sal_uInt16 n=0; n < nCount; ++n )
            AddClipbrdFormat( static_cast<SotClipboardFormatId>(aClipFormats.Identifiers[n]), aClipFormats.Names[n], n );

        return true;
    }

    return false;
}

bool SvxClipboardFormatItem::operator==( const SfxPoolItem& rComp ) const
{
    const SvxClipboardFormatItem& rCmp = static_cast<const SvxClipboardFormatItem&>(rComp);
    if(rCmp.pImpl->aFmtNms.size() != pImpl->aFmtNms.size())
        return false;

    int nRet = 1;
    for( sal_uInt16 n = 0, nEnd = rCmp.pImpl->aFmtNms.size(); n < nEnd; ++n )
    {
        if( pImpl->aFmtIds[ n ] != rCmp.pImpl->aFmtIds[ n ] ||
            pImpl->aFmtNms[n] != rCmp.pImpl->aFmtNms[n] )
        {
            nRet = 0;
            break;
        }
    }

    return nRet;
}

SfxPoolItem* SvxClipboardFormatItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxClipboardFormatItem( *this );
}

void SvxClipboardFormatItem::AddClipbrdFormat( SotClipboardFormatId nId, sal_uInt16 nPos )
{
    if( nPos > pImpl->aFmtNms.size() )
        nPos = pImpl->aFmtNms.size();

    pImpl->aFmtNms.insert(pImpl->aFmtNms.begin() + nPos, OUString());
    pImpl->aFmtIds.insert( pImpl->aFmtIds.begin()+nPos, nId );
}

void SvxClipboardFormatItem::AddClipbrdFormat( SotClipboardFormatId nId, const OUString& rName,
                            sal_uInt16 nPos )
{
    if( nPos > pImpl->aFmtNms.size() )
        nPos = pImpl->aFmtNms.size();

    pImpl->aFmtNms.insert(pImpl->aFmtNms.begin() + nPos, rName);
    pImpl->aFmtIds.insert( pImpl->aFmtIds.begin()+nPos, nId );
}

sal_uInt16 SvxClipboardFormatItem::Count() const
{
    return pImpl->aFmtIds.size();
}

SotClipboardFormatId SvxClipboardFormatItem::GetClipbrdFormatId( sal_uInt16 nPos ) const
{
    return pImpl->aFmtIds[ nPos ];
}

const OUString SvxClipboardFormatItem::GetClipbrdFormatName( sal_uInt16 nPos ) const
{
    return pImpl->aFmtNms[nPos];
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
