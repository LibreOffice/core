/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_TitleRotation.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:49:40 $
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
#include "precompiled_chart2.hxx"
#include "tp_TitleRotation.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "chartview/ChartSfxItemIds.hxx"

// header for class SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

SchAlignmentTabPage::SchAlignmentTabPage(Window* pWindow,
                                         const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_ALIGNMENT), rInAttrs),
    aFlAlign        ( this, SchResId( FL_ALIGN ) ),
    aCtrlDial       ( this, SchResId( CTR_DIAL ) ),
    aFtRotate       ( this, SchResId( FT_DEGREES ) ),
    aNfRotate       ( this, SchResId( NF_ORIENT ) ),
    aCbStacked      ( this, SchResId( BTN_TXTSTACKED ) ),
    aOrientHlp      ( this, aCtrlDial, aNfRotate, aCbStacked )
{
    FreeResource();

    aCbStacked.EnableTriState( FALSE );
    aOrientHlp.AddDependentWindow( aFtRotate, STATE_CHECK );
}

SchAlignmentTabPage::~SchAlignmentTabPage()
{
}

SfxTabPage* SchAlignmentTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new SchAlignmentTabPage(pWindow, rOutAttrs);
}

BOOL SchAlignmentTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    //Seit 4/1998 koennen Texte frei gedreht werden: SCHATTR_TEXT_DEGREES
    bool bStacked = aOrientHlp.GetStackedState() == STATE_CHECK;
    rOutAttrs.Put( SfxBoolItem( SCHATTR_TEXT_STACKED, bStacked ) );

    sal_Int32 nDegrees = bStacked ? 0 : aCtrlDial.GetRotation();
    rOutAttrs.Put( SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );
    return TRUE;
}

void SchAlignmentTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;
    const SfxPoolItem* pItem = GetItem( rInAttrs, SCHATTR_TEXT_DEGREES );

    sal_Int32 nDegrees = pItem ? ((const SfxInt32Item*)pItem)->GetValue() : 0;
    aCtrlDial.SetRotation( nDegrees );

    pItem = GetItem( rInAttrs, SCHATTR_TEXT_STACKED );
    bool bStacked = pItem && ((const SfxBoolItem*)pItem)->GetValue();
    aOrientHlp.SetStackedState( bStacked ? STATE_CHECK : STATE_NOCHECK );
}

//.............................................................................
} //namespace chart
//.............................................................................


