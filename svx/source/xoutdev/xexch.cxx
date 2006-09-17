/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xexch.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:23:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <sot/formats.hxx>
#include <tools/vcompat.hxx>

#ifndef _SVX_XFLASIT_HXX
#include <xflasit.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX
#include <xfillit0.hxx>
#endif
#ifndef _SFXIPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXIPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#include "xdef.hxx"
#include "xexch.hxx"


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
ULONG XFillExchangeData::RegisterClipboardFormatName()
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
        USHORT              nWhich = aIter.FirstWhich();
        const SfxPoolItem*  pItem;
        sal_uInt32          nItemCount = 0;
        sal_Size            nFirstPos = rOStm.Tell();

        rOStm << nItemCount;

        while( nWhich )
        {
            if( SFX_ITEM_SET == rData.pXFillAttrSetItem->GetItemSet().GetItemState( nWhich, FALSE, &pItem ) )
            {
                VersionCompat   aCompat( rOStm, STREAM_WRITE );
                const USHORT    nItemVersion2 = pItem->GetVersion( (USHORT) rOStm.GetVersion() );

                rOStm << nWhich << nItemVersion2;
                pItem->Store( rOStm, nItemVersion2 );

                nItemCount++;
            }

            nWhich = aIter.NextWhich();
        }

        const ULONG nLastPos = rOStm.Tell();
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
    USHORT          nWhich, nItemVersion;

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
