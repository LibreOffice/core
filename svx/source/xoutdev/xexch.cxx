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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <sot/formats.hxx>
#include <tools/vcompat.hxx>
#include <svx/xflasit.hxx>
#include <svx/xfillit0.hxx>
#ifndef _SFXIPOOL_HXX
#include <svl/itempool.hxx>
#endif
#include <svl/whiter.hxx>
#ifndef _SFXIPOOL_HXX
#include <svl/itempool.hxx>
#endif
#include <svl/itemset.hxx>
#include <svx/xdef.hxx>
#include "svx/xexch.hxx"


TYPEINIT1_AUTOFACTORY( XFillExchangeData, SvDataCopyStream );


/*************************************************************************
|*
|* Default-Ctor (Fuer Assign())
|*
*************************************************************************/
XFillExchangeData::XFillExchangeData() :
    pXFillAttrSetItem( NULL ),
    pPool( NULL )
{
}


/*************************************************************************
|*
|* Ctor
|*
*************************************************************************/
XFillExchangeData::XFillExchangeData( const XFillAttrSetItem rXFillAttrSetItem ) :
    pXFillAttrSetItem( (XFillAttrSetItem*) rXFillAttrSetItem.Clone( rXFillAttrSetItem.GetItemSet().GetPool() ) ),
    pPool( rXFillAttrSetItem.GetItemSet().GetPool() )
{
}


/*************************************************************************
|*
|* Dtor
|*
*************************************************************************/
XFillExchangeData::~XFillExchangeData()
{
    delete pXFillAttrSetItem;
}

/*************************************************************************
|*
|*
|*
*************************************************************************/
sal_uIntPtr XFillExchangeData::RegisterClipboardFormatName()
{
    return( SOT_FORMATSTR_ID_XFA );
}

/******************************************************************************
|*
|*  Binaer-Export (z.Z. ohne Versionsverwaltung, da nicht persistent!)
|*
\******************************************************************************/

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


/******************************************************************************
|*
|*  Binaer-Import (z.Z. ohne Versionsverwaltung, da nicht persistent!)
|*
\******************************************************************************/

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

/*************************************************************************
|*
|*    XBitmap& XBitmap::operator=( const XBitmap& rXBmp )
|*
*************************************************************************/

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
