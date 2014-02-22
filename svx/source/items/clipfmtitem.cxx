/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/clipfmtitem.hxx>
#include <com/sun/star/frame/status/ClipboardFormats.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

struct SvxClipboardFmtItem_Impl
{
    boost::ptr_vector< boost::nullable<OUString> > aFmtNms;
    std::vector<sal_uIntPtr> aFmtIds;

    SvxClipboardFmtItem_Impl() {}
    SvxClipboardFmtItem_Impl( const SvxClipboardFmtItem_Impl& );
};

TYPEINIT1_FACTORY( SvxClipboardFmtItem, SfxPoolItem , new  SvxClipboardFmtItem(0));

SvxClipboardFmtItem_Impl::SvxClipboardFmtItem_Impl(
                            const SvxClipboardFmtItem_Impl& rCpy )
    : aFmtNms(rCpy.aFmtNms)
    , aFmtIds(rCpy.aFmtIds)
{
}

SvxClipboardFmtItem::SvxClipboardFmtItem( sal_uInt16 nId )
    : SfxPoolItem( nId ), pImpl( new SvxClipboardFmtItem_Impl )
{
}

SvxClipboardFmtItem::SvxClipboardFmtItem( const SvxClipboardFmtItem& rCpy )
    : SfxPoolItem( rCpy.Which() ),
    pImpl( new SvxClipboardFmtItem_Impl( *rCpy.pImpl ) )
{
}

SvxClipboardFmtItem::~SvxClipboardFmtItem()
{
    delete pImpl;
}

bool SvxClipboardFmtItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    sal_uInt16 nCount = Count();

    ::com::sun::star::frame::status::ClipboardFormats aClipFormats;

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

bool SvxClipboardFmtItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    ::com::sun::star::frame::status::ClipboardFormats aClipFormats;
    if ( rVal >>= aClipFormats )
    {
        sal_uInt16 nCount = sal_uInt16( aClipFormats.Identifiers.getLength() );

        pImpl->aFmtIds.clear();
        pImpl->aFmtNms.clear();
        for ( sal_uInt16 n=0; n < nCount; ++n )
            AddClipbrdFormat( sal_uIntPtr( aClipFormats.Identifiers[n] ), aClipFormats.Names[n], n );

        return true;
    }

    return false;
}

bool SvxClipboardFmtItem::operator==( const SfxPoolItem& rComp ) const
{
    const SvxClipboardFmtItem& rCmp = (SvxClipboardFmtItem&)rComp;
    if(rCmp.pImpl->aFmtNms.size() != pImpl->aFmtNms.size())
        return false;

    int nRet = 1;
    for( sal_uInt16 n = 0, nEnd = rCmp.pImpl->aFmtNms.size(); n < nEnd; ++n )
    {
        if( pImpl->aFmtIds[ n ] != rCmp.pImpl->aFmtIds[ n ] ||
            ( pImpl->aFmtNms.is_null(n) != rCmp.pImpl->aFmtNms.is_null(n) ) ||
            ( !pImpl->aFmtNms.is_null(n) && pImpl->aFmtNms[n] != rCmp.pImpl->aFmtNms[n] ) )
        {
            nRet = 0;
            break;
        }
    }

    return nRet;
}

SfxPoolItem* SvxClipboardFmtItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxClipboardFmtItem( *this );
}

void SvxClipboardFmtItem::AddClipbrdFormat( sal_uIntPtr nId, sal_uInt16 nPos )
{
    if( nPos > pImpl->aFmtNms.size() )
        nPos = pImpl->aFmtNms.size();

    pImpl->aFmtNms.insert(pImpl->aFmtNms.begin() + nPos, 0);
    pImpl->aFmtIds.insert( pImpl->aFmtIds.begin()+nPos, nId );
}

void SvxClipboardFmtItem::AddClipbrdFormat( sal_uIntPtr nId, const OUString& rName,
                            sal_uInt16 nPos )
{
    if( nPos > pImpl->aFmtNms.size() )
        nPos = pImpl->aFmtNms.size();

    pImpl->aFmtNms.insert(pImpl->aFmtNms.begin() + nPos, new OUString(rName));
    pImpl->aFmtIds.insert( pImpl->aFmtIds.begin()+nPos, nId );
}

sal_uInt16 SvxClipboardFmtItem::Count() const
{
    return pImpl->aFmtIds.size();
}

sal_uIntPtr SvxClipboardFmtItem::GetClipbrdFormatId( sal_uInt16 nPos ) const
{
    return pImpl->aFmtIds[ nPos ];
}

const OUString SvxClipboardFmtItem::GetClipbrdFormatName( sal_uInt16 nPos ) const
{
    return pImpl->aFmtNms.is_null(nPos) ? OUString() : pImpl->aFmtNms[nPos];
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
