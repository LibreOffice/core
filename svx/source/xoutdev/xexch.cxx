/*************************************************************************
 *
 *  $RCSfile: xexch.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-20 20:00:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#include <sot/formats.hxx>
#include <tools/vcompat.hxx>

#ifndef _CLIP_HXX
#include <vcl/clip.hxx>
#endif
#ifndef _DRAG_HXX
#include <vcl/drag.hxx>
#endif
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
    pPool( NULL ),
    pXFillAttrSetItem( NULL )
{
}


/*************************************************************************
|*
|* Ctor
|*
*************************************************************************/
XFillExchangeData::XFillExchangeData( const XFillAttrSetItem rXFillAttrSetItem ) :
    pPool( rXFillAttrSetItem.GetItemSet().GetPool() ),
    pXFillAttrSetItem( (XFillAttrSetItem*) rXFillAttrSetItem.Clone( rXFillAttrSetItem.GetItemSet().GetPool() ) )
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
|* Laden
|*
*************************************************************************/
void XFillExchangeData::Load( SvStream& rIStm )
{
    rIStm >> *this;
}


/*************************************************************************
|*
|* Speichern
|*
*************************************************************************/
void XFillExchangeData::Save( SvStream& rOStm )
{
    rOStm << *this;
}


/*************************************************************************
|*
|* Zuweisen
|*
*************************************************************************/
void XFillExchangeData::Assign( const SvDataCopyStream& rCopyStream )
{
    *this = (const XFillExchangeData& ) rCopyStream;
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
        USHORT nItemVersion = rData.pXFillAttrSetItem->GetVersion( (USHORT) rOStm.GetVersion() );

        SfxWhichIter        aIter( rData.pXFillAttrSetItem->GetItemSet() );
        USHORT              nWhich = aIter.FirstWhich();
        const SfxPoolItem*  pItem;
        ULONG               nItemCount = 0, nFirstPos = rOStm.Tell();

        rOStm << nItemCount;

        while( nWhich )
        {
            if( SFX_ITEM_SET == rData.pXFillAttrSetItem->GetItemSet().GetItemState( nWhich, FALSE, &pItem ) )
            {
                VersionCompat   aCompat( rOStm, STREAM_WRITE );
                const USHORT    nItemVersion = pItem->GetVersion( (USHORT) rOStm.GetVersion() );

                rOStm << nWhich << nItemVersion;
                pItem->Store( rOStm, nItemVersion );

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
    ULONG           nItemCount = 0;
    USHORT          nWhich, nItemVersion;

    rIStm >> nItemCount;

    if( nItemCount > ( XATTR_FILL_LAST - XATTR_FILL_FIRST + 1 ) )
        nItemCount = ( XATTR_FILL_LAST - XATTR_FILL_FIRST + 1 );

    for( ULONG i = 0; i < nItemCount; i++ )
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
