/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewlayoutctrl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:09:49 $
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
#include "precompiled_sw.hxx"

// include ---------------------------------------------------------------

#ifndef _VIEWLAYOUTCTRL_HXX
#include <viewlayoutctrl.hxx>
#endif

#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_VIEWLAYOUTITEM_HXX
#include <svx/viewlayoutitem.hxx>
#endif
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>  // fuer Pathfinder
#endif

// STATIC DATA -----------------------------------------------------------

SFX_IMPL_STATUSBAR_CONTROL( SwViewLayoutControl, SvxViewLayoutItem );

// -----------------------------------------------------------------------

const long nImageWidthSingle = 15;
const long nImageWidthAuto = 25;
const long nImageWidthBook = 23;
const long nImageWidthSum = 63;
const long nImageHeight = 11;

// -----------------------------------------------------------------------

struct SwViewLayoutControl::SwViewLayoutControl_Impl
{
    USHORT      mnState; // 0 = single, 1 = auto, 2 = book, 3 = none
    Image       maImageSingleColumn;
    Image       maImageSingleColumn_Active;
    Image       maImageAutomatic;
    Image       maImageAutomatic_Active;
    Image       maImageBookMode;
    Image       maImageBookMode_Active;
};

// class SwViewLayoutControl ------------------------------------------

SwViewLayoutControl::SwViewLayoutControl( USHORT _nSlotId, USHORT _nId, StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    mpImpl( new SwViewLayoutControl_Impl )
{
    mpImpl->mnState = 0;

    const sal_Bool bIsDark = GetStatusBar().GetBackground().GetColor().IsDark();
    mpImpl->maImageSingleColumn         = Image( bIsDark ? SW_RES(IMG_VIEWLAYOUT_SINGLECOLUMN_HC)          : SW_RES(IMG_VIEWLAYOUT_SINGLECOLUMN) );
    mpImpl->maImageSingleColumn_Active  = Image( bIsDark ? SW_RES(IMG_VIEWLAYOUT_SINGLECOLUMN_ACTIVE_HC) : SW_RES(IMG_VIEWLAYOUT_SINGLECOLUMN_ACTIVE) );
    mpImpl->maImageAutomatic            = Image( bIsDark ? SW_RES(IMG_VIEWLAYOUT_AUTOMATIC_HC)             : SW_RES(IMG_VIEWLAYOUT_AUTOMATIC) );
    mpImpl->maImageAutomatic_Active     = Image( bIsDark ? SW_RES(IMG_VIEWLAYOUT_AUTOMATIC_ACTIVE_HC)    : SW_RES(IMG_VIEWLAYOUT_AUTOMATIC_ACTIVE) );
    mpImpl->maImageBookMode             = Image( bIsDark ? SW_RES(IMG_VIEWLAYOUT_BOOKMODE_HC)              : SW_RES(IMG_VIEWLAYOUT_BOOKMODE) );
    mpImpl->maImageBookMode_Active      = Image( bIsDark ? SW_RES(IMG_VIEWLAYOUT_BOOKMODE_ACTIVE_HC)     : SW_RES(IMG_VIEWLAYOUT_BOOKMODE_ACTIVE) );
}

// -----------------------------------------------------------------------

SwViewLayoutControl::~SwViewLayoutControl()
{
    delete mpImpl;
}

// -----------------------------------------------------------------------

void SwViewLayoutControl::StateChanged( USHORT /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState || pState->ISA( SfxVoidItem ) )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( pState->ISA( SvxViewLayoutItem ), "invalid item type" );
        const USHORT nColumns  = static_cast<const SvxViewLayoutItem*>( pState )->GetValue();
        const bool   bBookMode = static_cast<const SvxViewLayoutItem*>( pState )->IsBookMode();

        // SingleColumn Mode
        if ( 1 == nColumns )
            mpImpl->mnState = 0;
        // Automatic Mode
        else if ( 0 == nColumns )
            mpImpl->mnState = 1;
        // Book Mode
        else if ( bBookMode && 2 == nColumns )
            mpImpl->mnState = 2;
        else
            mpImpl->mnState = 3;
    }

    if ( GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), 0 );    // force repaint
}

// -----------------------------------------------------------------------

void SwViewLayoutControl::Paint( const UserDrawEvent& rUsrEvt )
{
    OutputDevice*       pDev =  rUsrEvt.GetDevice();
    Rectangle           aRect = rUsrEvt.GetRect();
    Color               aOldLineColor = pDev->GetLineColor();
    Color               aOldFillColor = pDev->GetFillColor();

    //pDev->SetLineColor();
    //pDev->SetFillColor( pDev->GetBackground().GetColor() );

    const bool bSingleColumn    = 0 == mpImpl->mnState;
    const bool bAutomatic       = 1 == mpImpl->mnState;
    const bool bBookMode        = 2 == mpImpl->mnState;

    const long nXOffset = (aRect.GetWidth()  - nImageWidthSum)/2;
    const long nYOffset = (aRect.GetHeight() - nImageHeight)/2;

    aRect.Left() = aRect.Left() + nXOffset;
    aRect.Top()  = aRect.Top() + nYOffset;

    // draw single column image:
    pDev->DrawImage( aRect.TopLeft(), bSingleColumn ? mpImpl->maImageSingleColumn_Active : mpImpl->maImageSingleColumn );

    // draw automatic image:
    aRect.Left() += nImageWidthSingle;
    pDev->DrawImage( aRect.TopLeft(), bAutomatic ? mpImpl->maImageAutomatic_Active       : mpImpl->maImageAutomatic );

    // draw bookmode image:
    aRect.Left() += nImageWidthAuto;
    pDev->DrawImage( aRect.TopLeft(), bBookMode ? mpImpl->maImageBookMode_Active         : mpImpl->maImageBookMode );

    // draw separators
    //aRect = rUsrEvt.GetRect();
    //aRect.Left() += nImageWidth;
    //aRect.setWidth( 1 );
    //pDev->DrawRect( aRect );
    //aRect.Left() += nImageWidth;
    //pDev->DrawRect( aRect );

    //pDev->SetLineColor( aOldLineColor );
    //pDev->SetFillColor( aOldFillColor );
}

BOOL SwViewLayoutControl::MouseButtonDown( const MouseEvent & rEvt )
{
    const Rectangle aRect = getControlRect();
    const Point aPoint = rEvt.GetPosPixel();
    const long nXDiff = aPoint.X() - aRect.Left();

    USHORT nColumns = 1;
    bool bBookMode = false;

    const long nXOffset = (aRect.GetWidth() - nImageWidthSum)/2;

    if ( nXDiff < nXOffset + nImageWidthSingle )
    {
        mpImpl->mnState = 0; // single
        nColumns = 1;
    }
    else if ( nXDiff < nXOffset + nImageWidthSingle + nImageWidthAuto )
    {
        mpImpl->mnState = 1; // auto
        nColumns = 0;
    }
    else
    {
        mpImpl->mnState = 2; // book
        nColumns = 2;
        bBookMode = true;
    }

    // commit state change
    SvxViewLayoutItem aViewLayout( nColumns, bBookMode );

    ::com::sun::star::uno::Any a;
    aViewLayout.QueryValue( a );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ViewLayout" ));
    aArgs[0].Value = a;

    execute( aArgs );

    return TRUE;
}
