/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
