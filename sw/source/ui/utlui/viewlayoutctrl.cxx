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
#include "precompiled_sw.hxx"

// include ---------------------------------------------------------------
#include <viewlayoutctrl.hxx>

#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#include <vcl/image.hxx>
#include <svl/eitem.hxx>
#include <svx/viewlayoutitem.hxx>
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#include <swtypes.hxx>  // fuer Pathfinder

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

    const sal_Bool bHC = GetStatusBar().GetSettings().GetStyleSettings().GetHighContrastMode();
    mpImpl->maImageSingleColumn         = Image( bHC ? SW_RES(IMG_VIEWLAYOUT_SINGLECOLUMN_HC)          : SW_RES(IMG_VIEWLAYOUT_SINGLECOLUMN) );
    mpImpl->maImageSingleColumn_Active  = Image( bHC ? SW_RES(IMG_VIEWLAYOUT_SINGLECOLUMN_ACTIVE_HC) : SW_RES(IMG_VIEWLAYOUT_SINGLECOLUMN_ACTIVE) );
    mpImpl->maImageAutomatic            = Image( bHC ? SW_RES(IMG_VIEWLAYOUT_AUTOMATIC_HC)             : SW_RES(IMG_VIEWLAYOUT_AUTOMATIC) );
    mpImpl->maImageAutomatic_Active     = Image( bHC ? SW_RES(IMG_VIEWLAYOUT_AUTOMATIC_ACTIVE_HC)    : SW_RES(IMG_VIEWLAYOUT_AUTOMATIC_ACTIVE) );
    mpImpl->maImageBookMode             = Image( bHC ? SW_RES(IMG_VIEWLAYOUT_BOOKMODE_HC)              : SW_RES(IMG_VIEWLAYOUT_BOOKMODE) );
    mpImpl->maImageBookMode_Active      = Image( bHC ? SW_RES(IMG_VIEWLAYOUT_BOOKMODE_ACTIVE_HC)     : SW_RES(IMG_VIEWLAYOUT_BOOKMODE_ACTIVE) );
}

// -----------------------------------------------------------------------

SwViewLayoutControl::~SwViewLayoutControl()
{
    delete mpImpl;
}

// -----------------------------------------------------------------------

void SwViewLayoutControl::StateChanged( sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState || dynamic_cast< const SfxVoidItem* >(pState) )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( dynamic_cast< const SvxViewLayoutItem* >(pState), "invalid item type" );
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
