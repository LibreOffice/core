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

#include <sot/formats.hxx>
#include <tools/vcompat.hxx>
#include <svx/xflasit.hxx>
#include <svx/xfillit0.hxx>
#include <svl/itempool.hxx>
#include <svl/whiter.hxx>
#include <svl/itemset.hxx>
#include <svx/xdef.hxx>
#include "svx/xexch.hxx"


TYPEINIT1_AUTOFACTORY( XFillExchangeData, SvDataCopyStream );

/// default CTOR, for Assign()
XFillExchangeData::XFillExchangeData() :
    pXFillAttrSetItem( NULL ),
    pPool( NULL )
{
}

XFillExchangeData::XFillExchangeData( const XFillAttrSetItem rXFillAttrSetItem ) :
    pXFillAttrSetItem( (XFillAttrSetItem*) rXFillAttrSetItem.Clone( rXFillAttrSetItem.GetItemSet().GetPool() ) ),
    pPool( rXFillAttrSetItem.GetItemSet().GetPool() )
{
}

XFillExchangeData::~XFillExchangeData()
{
    delete pXFillAttrSetItem;
}

/// binary export (currently w/o version control because it is not persistent)
SvStream& operator<<( SvStream& rOStm, const XFillExchangeData& rData )
{
    if( rData.pXFillAttrSetItem )
    {
        SfxWhichIter        aIter( rData.pXFillAttrSetItem->GetItemSet() );
        sal_uInt16              nWhich = aIter.FirstWhich();
        const SfxPoolItem*  pItem;
        sal_uInt32          nItemCount = 0;
        sal_Size            nFirstPos = rOStm.Tell();

        rOStm << nItemCount;

        while( nWhich )
        {
            if( SFX_ITEM_SET == rData.pXFillAttrSetItem->GetItemSet().GetItemState( nWhich, sal_False, &pItem ) )
            {
                VersionCompat   aCompat( rOStm, STREAM_WRITE );
                const sal_uInt16    nItemVersion2 = pItem->GetVersion( (sal_uInt16) rOStm.GetVersion() );

                rOStm << nWhich << nItemVersion2;
                pItem->Store( rOStm, nItemVersion2 );

                nItemCount++;
            }

            nWhich = aIter.NextWhich();
        }

        const sal_uIntPtr nLastPos = rOStm.Tell();
        rOStm.Seek( nFirstPos );
        rOStm << nItemCount;
        rOStm.Seek( nLastPos );
    }

    return rOStm;
}

/// binary export (currently w/o version control because it is not persistent)
SvStream& operator>>( SvStream& rIStm, XFillExchangeData& rData )
{
    DBG_ASSERT( rData.pPool, "XFillExchangeData has no pool" );

    SfxItemSet*     pSet = new SfxItemSet ( *rData.pPool, XATTR_FILL_FIRST, XATTR_FILL_LAST );
    SfxPoolItem*    pNewItem;
    sal_uInt32      nItemCount = 0;
    sal_uInt16          nWhich, nItemVersion;

    rIStm >> nItemCount;

    if( nItemCount > ( XATTR_FILL_LAST - XATTR_FILL_FIRST + 1 ) )
        nItemCount = ( XATTR_FILL_LAST - XATTR_FILL_FIRST + 1 );

    for( sal_uInt32 i = 0; i < nItemCount; i++ )
    {
        VersionCompat aCompat( rIStm, STREAM_READ );

        rIStm >> nWhich >> nItemVersion;

        if( nWhich )
        {
            pNewItem = rData.pPool->GetDefaultItem( nWhich ).Create( rIStm, nItemVersion );

            if( pNewItem )
            {
                pSet->Put( *pNewItem );
                delete pNewItem;
            }
        }
    }

    delete rData.pXFillAttrSetItem;
    rData.pXFillAttrSetItem = new XFillAttrSetItem( pSet );
    rData.pPool = rData.pXFillAttrSetItem->GetItemSet().GetPool();

    return rIStm;
}

XFillExchangeData& XFillExchangeData::operator=( const XFillExchangeData& rData )
{
    delete pXFillAttrSetItem;

    if( rData.pXFillAttrSetItem )
        pXFillAttrSetItem = (XFillAttrSetItem*) rData.pXFillAttrSetItem->Clone( pPool = rData.pXFillAttrSetItem->GetItemSet().GetPool() );
    else
    {
        pPool = NULL;
        pXFillAttrSetItem = NULL;
    }

    return( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
