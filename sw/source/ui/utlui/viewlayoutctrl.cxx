/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sw.hxx"

// include ---------------------------------------------------------------
#include <viewlayoutctrl.hxx>

#include <vcl/status.hxx>
#include <vcl/image.hxx>
#include <svl/eitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <utlui.hrc>
#include <swtypes.hxx>  // fuer Pathfinder

// STATIC DATA -----------------------------------------------------------

SFX_IMPL_STATUSBAR_CONTROL( SwViewLayoutControl, SvxViewLayoutItem );

const long nImageWidthSingle = 15;
const long nImageWidthAuto = 25;
const long nImageWidthBook = 23;
const long nImageWidthSum = 63;
const long nImageHeight = 11;

struct SwViewLayoutControl::SwViewLayoutControl_Impl
{
    sal_uInt16      mnState; // 0 = single, 1 = auto, 2 = book, 3 = none
    Image       maImageSingleColumn;
    Image       maImageSingleColumn_Active;
    Image       maImageAutomatic;
    Image       maImageAutomatic_Active;
    Image       maImageBookMode;
    Image       maImageBookMode_Active;
};

// class SwViewLayoutControl ------------------------------------------

SwViewLayoutControl::SwViewLayoutControl( sal_uInt16 _nSlotId, sal_uInt16 _nId, StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    mpImpl( new SwViewLayoutControl_Impl )
{
    mpImpl->mnState = 0;

    mpImpl->maImageSingleColumn         = Image( SW_RES(IMG_VIEWLAYOUT_SINGLECOLUMN) );
    mpImpl->maImageSingleColumn_Active  = Image( SW_RES(IMG_VIEWLAYOUT_SINGLECOLUMN_ACTIVE) );
    mpImpl->maImageAutomatic            = Image( SW_RES(IMG_VIEWLAYOUT_AUTOMATIC) );
    mpImpl->maImageAutomatic_Active     = Image( SW_RES(IMG_VIEWLAYOUT_AUTOMATIC_ACTIVE) );
    mpImpl->maImageBookMode             = Image( SW_RES(IMG_VIEWLAYOUT_BOOKMODE) );
    mpImpl->maImageBookMode_Active      = Image( SW_RES(IMG_VIEWLAYOUT_BOOKMODE_ACTIVE) );
}

SwViewLayoutControl::~SwViewLayoutControl()
{
    delete mpImpl;
}

void SwViewLayoutControl::StateChanged( sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState || pState->ISA( SfxVoidItem ) )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        OSL_ENSURE( pState->ISA( SvxViewLayoutItem ), "invalid item type" );
        const sal_uInt16 nColumns  = static_cast<const SvxViewLayoutItem*>( pState )->GetValue();
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

void SwViewLayoutControl::Paint( const UserDrawEvent& rUsrEvt )
{
    OutputDevice*       pDev =  rUsrEvt.GetDevice();
    Rectangle           aRect = rUsrEvt.GetRect();

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
}

sal_Bool SwViewLayoutControl::MouseButtonDown( const MouseEvent & rEvt )
{
    const Rectangle aRect = getControlRect();
    const Point aPoint = rEvt.GetPosPixel();
    const long nXDiff = aPoint.X() - aRect.Left();

    sal_uInt16 nColumns = 1;
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

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
