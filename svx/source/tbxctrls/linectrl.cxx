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

#include <string>

#include <vcl/toolbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>

#include <svx/dialogs.hrc>
#include "helpid.hrc"

#include "svx/drawitem.hxx"
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include "svx/linectrl.hxx"
#include <svx/itemwin.hxx>
#include <svx/dialmgr.hxx>
#include <svx/unoapi.hxx>
#include <memory>
#include <o3tl/make_unique.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;

// For End Line Controller
#define MAX_LINES 12

SFX_IMPL_TOOLBOX_CONTROL( SvxLineStyleToolBoxControl, XLineStyleItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxLineWidthToolBoxControl, XLineWidthItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxLineEndToolBoxControl,   SfxBoolItem );

SvxLineStyleToolBoxControl::SvxLineStyleToolBoxControl( sal_uInt16 nSlotId,
                                                        sal_uInt16 nId,
                                                        ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pStyleItem      ( nullptr ),
    pDashItem       ( nullptr ),
    bUpdate         ( false )
{
    addStatusListener( ".uno:LineDash");
    addStatusListener( ".uno:DashListState");
}


SvxLineStyleToolBoxControl::~SvxLineStyleToolBoxControl()
{
    delete pStyleItem;
    delete pDashItem;
}


void SvxLineStyleToolBoxControl::StateChanged (

    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    SvxLineBox* pBox = static_cast<SvxLineBox*>( GetToolBox().GetItemWindow( GetId() ) );
    DBG_ASSERT( pBox, "Window not found!" );

    if( eState == SfxItemState::DISABLED )
    {
        pBox->Disable();
        pBox->SetNoSelection();
    }
    else
    {
        pBox->Enable();

        if ( eState == SfxItemState::DEFAULT )
        {
            if( nSID == SID_ATTR_LINE_STYLE )
            {
                delete pStyleItem;
                pStyleItem = static_cast<XLineStyleItem*>(pState->Clone());
            }
            else if( nSID == SID_ATTR_LINE_DASH )
            {
                delete pDashItem;
                pDashItem = static_cast<XLineDashItem*>(pState->Clone());
            }

            bUpdate = true;
            Update( pState );
        }
        else if ( nSID != SID_DASH_LIST )
        {
            // no or ambiguous status
            pBox->SetNoSelection();
        }
    }
}


void SvxLineStyleToolBoxControl::Update( const SfxPoolItem* pState )
{
    if ( pState && bUpdate )
    {
        bUpdate = false;

        SvxLineBox* pBox = static_cast<SvxLineBox*>(GetToolBox().GetItemWindow( GetId() ));
        DBG_ASSERT( pBox, "Window not found!" );

        // Since the timer can strike unexpectedly, it may happen that
        // the LB is not yet filled. A ClearCache() on the control
        // in DelayHdl () was unsuccessful.
        if( pBox->GetEntryCount() == 0 )
            pBox->FillControl();

        drawing::LineStyle eXLS;

        if ( pStyleItem )
            eXLS = ( drawing::LineStyle )pStyleItem->GetValue();
        else
            eXLS = drawing::LineStyle_NONE;

        switch( eXLS )
        {
            case drawing::LineStyle_NONE:
                pBox->SelectEntryPos( 0 );
                break;

            case drawing::LineStyle_SOLID:
                pBox->SelectEntryPos( 1 );
                break;

            case drawing::LineStyle_DASH:
            {
                if( pDashItem )
                {
                    OUString aString = SvxUnogetInternalNameForItem(
                        XATTR_LINEDASH, pDashItem->GetName());
                    pBox->SelectEntry( aString );
                }
                else
                    pBox->SetNoSelection();
            }
            break;

            default:
                OSL_FAIL( "Unsupported type of line" );
                break;
        }
    }

    if ( pState && ( dynamic_cast<const SvxDashListItem*>( pState) !=  nullptr ) )
    {
        // The list of line styles has changed
        SvxLineBox* pBox = static_cast<SvxLineBox*>(GetToolBox().GetItemWindow( GetId() ));
        DBG_ASSERT( pBox, "Window not found!" );

        OUString aString( pBox->GetSelectEntry() );
        pBox->Clear();
        pBox->InsertEntry( SVX_RESSTR(RID_SVXSTR_INVISIBLE) );
        pBox->InsertEntry( SVX_RESSTR(RID_SVXSTR_SOLID) );
        pBox->Fill( static_cast<const SvxDashListItem*>(pState)->GetDashList() );
        pBox->SelectEntry( aString );
    }
}


VclPtr<vcl::Window> SvxLineStyleToolBoxControl::CreateItemWindow( vcl::Window *pParent )
{
    return VclPtr<SvxLineBox>::Create( pParent, m_xFrame ).get();
}

SvxLineWidthToolBoxControl::SvxLineWidthToolBoxControl(
    sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    addStatusListener( ".uno:MetricUnit");
}


SvxLineWidthToolBoxControl::~SvxLineWidthToolBoxControl()
{
}


void SvxLineWidthToolBoxControl::StateChanged(
    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    SvxMetricField* pFld = static_cast<SvxMetricField*>(
                           GetToolBox().GetItemWindow( GetId() ));
    DBG_ASSERT( pFld, "Window not found" );

    if ( nSID == SID_ATTR_METRIC )
    {
        pFld->RefreshDlgUnit();
    }
    else
    {
        if ( eState == SfxItemState::DISABLED )
        {
            pFld->Disable();
            pFld->SetText( "" );
        }
        else
        {
            pFld->Enable();

            if ( eState == SfxItemState::DEFAULT )
            {
                DBG_ASSERT( dynamic_cast<const XLineWidthItem*>( pState) !=  nullptr, "wrong ItemType" );

                // Core-Unit handed over to MetricField
                // Should not happen in CreateItemWin ()!
                MapUnit eUnit = MapUnit::Map100thMM; // CD!!! GetCoreMetric();
                pFld->SetCoreUnit( eUnit );

                pFld->Update( static_cast<const XLineWidthItem*>(pState) );
            }
            else
                pFld->Update( nullptr );
        }
    }
}


VclPtr<vcl::Window> SvxLineWidthToolBoxControl::CreateItemWindow( vcl::Window *pParent )
{
    return VclPtr<SvxMetricField>::Create( pParent, m_xFrame ).get();
}

SvxLineEndWindow::SvxLineEndWindow(
    sal_uInt16 nSlotId,
    const Reference< XFrame >& rFrame,
    vcl::Window* pParentWindow,
    const OUString& rWndTitle ) :
    SfxPopupWindow( nSlotId,
                    rFrame,
                    pParentWindow,
                    WinBits( WB_STDPOPUP | WB_OWNERDRAWDECORATION ) ),
    mpLineEndSet    ( VclPtr<ValueSet>::Create(this, WinBits( WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT ) )),
    mnCols          ( 2 ),
    mnLines         ( 12 ),
    mxFrame         ( rFrame )
{
    SetText( rWndTitle );
    implInit();
}

void SvxLineEndWindow::implInit()
{
    SfxObjectShell*     pDocSh  = SfxObjectShell::Current();

    SetHelpId( HID_POPUP_LINEEND );
    mpLineEndSet->SetHelpId( HID_POPUP_LINEEND_CTRL );

    if ( pDocSh )
    {
        const SfxPoolItem*  pItem = pDocSh->GetItem( SID_LINEEND_LIST );
        if( pItem )
            mpLineEndList = static_cast<const SvxLineEndListItem*>( pItem )->GetLineEndList();
    }
    DBG_ASSERT( mpLineEndList.is(), "LineEndList not found" );

    mpLineEndSet->SetSelectHdl( LINK( this, SvxLineEndWindow, SelectHdl ) );
    mpLineEndSet->SetColCount( mnCols );

    // ValueSet fill with entries of LineEndList
    FillValueSet();

    AddStatusListener( ".uno:LineEndListState");

    //ChangeHelpId( HID_POPUP_LINEENDSTYLE );
    mpLineEndSet->Show();
}

SvxLineEndWindow::~SvxLineEndWindow()
{
    disposeOnce();
}

void SvxLineEndWindow::dispose()
{
    mpLineEndSet.disposeAndClear();
    SfxPopupWindow::dispose();
}

IMPL_LINK_NOARG(SvxLineEndWindow, SelectHdl, ValueSet*, void)
{
    std::unique_ptr<XLineEndItem> pLineEndItem;
    std::unique_ptr<XLineStartItem> pLineStartItem;
    sal_uInt16 nId = mpLineEndSet->GetSelectItemId();

    if( nId == 1 )
    {
        pLineStartItem.reset(new XLineStartItem());
    }
    else if( nId == 2 )
    {
        pLineEndItem.reset(new XLineEndItem());
    }
    else if( nId % 2 ) // beginning of line
    {
        const XLineEndEntry* pEntry = mpLineEndList->GetLineEnd( (nId - 1) / 2 - 1 );
        pLineStartItem.reset(new XLineStartItem(pEntry->GetName(), pEntry->GetLineEnd()));
    }
    else // end of line
    {
        const XLineEndEntry* pEntry = mpLineEndList->GetLineEnd( nId / 2 - 2 );
        pLineEndItem.reset(new XLineEndItem(pEntry->GetName(), pEntry->GetLineEnd()));
    }

    if ( IsInPopupMode() )
        EndPopupMode();

    Sequence< PropertyValue > aArgs( 1 );
    Any a;

    if ( pLineStartItem )
    {
        aArgs[0].Name = "LineStart";
        pLineStartItem->QueryValue( a );
        aArgs[0].Value = a;
    }
    else
    {
        aArgs[0].Name = "LineEnd";
        pLineEndItem->QueryValue( a );
        aArgs[0].Value = a;
    }

    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    mpLineEndSet->SetNoSelection();

    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                 ".uno:LineEndStyle",
                                 aArgs );
}


void SvxLineEndWindow::FillValueSet()
{
    if( mpLineEndList.is() )
    {
        ScopedVclPtrInstance< VirtualDevice > pVD;

        long nCount = mpLineEndList->Count();

        // First entry: no line end.
        // An entry is temporarly added to get the UI bitmap
        basegfx::B2DPolyPolygon aNothing;
        mpLineEndList->Insert(o3tl::make_unique<XLineEndEntry>(aNothing, SVX_RESSTR(RID_SVXSTR_NONE)));
        const XLineEndEntry* pEntry = mpLineEndList->GetLineEnd(nCount);
        Bitmap aBmp = mpLineEndList->GetUiBitmap( nCount );
        OSL_ENSURE( !aBmp.IsEmpty(), "UI bitmap was not created" );

        maBmpSize = aBmp.GetSizePixel();
        pVD->SetOutputSizePixel( maBmpSize, false );
        maBmpSize.Width() = maBmpSize.Width() / 2;
        Point aPt0( 0, 0 );
        Point aPt1( maBmpSize.Width(), 0 );

        pVD->DrawBitmap( Point(), aBmp );
        mpLineEndSet->InsertItem(1, Image(pVD->GetBitmap(aPt0, maBmpSize)), pEntry->GetName());
        mpLineEndSet->InsertItem(2, Image(pVD->GetBitmap(aPt1, maBmpSize)), pEntry->GetName());

        mpLineEndList->Remove(nCount);

        for( long i = 0; i < nCount; i++ )
        {
            pEntry = mpLineEndList->GetLineEnd( i );
            DBG_ASSERT( pEntry, "Could not access LineEndEntry" );
            aBmp = mpLineEndList->GetUiBitmap( i );
            OSL_ENSURE( !aBmp.IsEmpty(), "UI bitmap was not created" );

            pVD->DrawBitmap( aPt0, aBmp );
            mpLineEndSet->InsertItem((sal_uInt16)((i+1L)*2L+1L),
                    Image(pVD->GetBitmap(aPt0, maBmpSize)), pEntry->GetName());
            mpLineEndSet->InsertItem((sal_uInt16)((i+2L)*2L),
                    Image(pVD->GetBitmap(aPt1, maBmpSize)), pEntry->GetName());
        }
        mnLines = std::min( (sal_uInt16)(nCount + 1), (sal_uInt16) MAX_LINES );
        mpLineEndSet->SetLineCount( mnLines );

        SetSize();
    }
}

void SvxLineEndWindow::StartSelection()
{
    mpLineEndSet->StartSelection();
}


void SvxLineEndWindow::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if ( rEvent.FeatureURL.Complete == ".uno:LineEndListState" )
    {
        // The list of line ends (LineEndList) has changed
        css::uno::Reference< css::uno::XWeak > xWeak;
        if ( rEvent.State >>= xWeak )
        {
            mpLineEndList.set( static_cast< XLineEndList* >( xWeak.get() ) );
            DBG_ASSERT( mpLineEndList.is(), "LineEndList not found" );

            mpLineEndSet->Clear();
            FillValueSet();
        }
    }
}

void SvxLineEndWindow::SetSize()
{
    sal_uInt16 nItemCount = mpLineEndSet->GetItemCount();
    sal_uInt16 nMaxLines  = nItemCount / mnCols;

    WinBits nBits = mpLineEndSet->GetStyle();
    if ( mnLines == nMaxLines )
        nBits &= ~WB_VSCROLL;
    else
        nBits |= WB_VSCROLL;
    mpLineEndSet->SetStyle( nBits );

    Size aSize( maBmpSize );
    aSize.Width()  += 6;
    aSize.Height() += 6;
    aSize = mpLineEndSet->CalcWindowSizePixel( aSize );
    mpLineEndSet->SetPosSizePixel( Point( 2, 2 ), aSize );
    aSize.Width()  += 4;
    aSize.Height() += 4;
    SetOutputSizePixel( aSize );
}

void SvxLineEndWindow::GetFocus()
{
    SfxPopupWindow::GetFocus();
    // Grab the focus to the line ends value set so that it can be controlled
    // with the keyboard.
    if ( mpLineEndSet )
        mpLineEndSet->GrabFocus();
}

SvxLineEndToolBoxControl::SvxLineEndToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWNONLY | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}


SvxLineEndToolBoxControl::~SvxLineEndToolBoxControl()
{
}


VclPtr<SfxPopupWindow> SvxLineEndToolBoxControl::CreatePopupWindow()
{
    VclPtrInstance<SvxLineEndWindow> pLineEndWin( GetId(), m_xFrame, &GetToolBox(), SVX_RESSTR( RID_SVXSTR_LINEEND ) );
    pLineEndWin->StartPopupMode( &GetToolBox(),
                                 FloatWinPopupFlags::GrabFocus |
                                 FloatWinPopupFlags::AllowTearOff |
                                 FloatWinPopupFlags::NoAppFocusClose );
    pLineEndWin->StartSelection();
    SetPopupWindow( pLineEndWin );
    return pLineEndWin;
}


void SvxLineEndToolBoxControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* )
{
    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();

    rTbx.EnableItem( nId, SfxItemState::DISABLED != eState );
    rTbx.SetItemState( nId, ( SfxItemState::DONTCARE == eState ) ? TRISTATE_INDET : TRISTATE_FALSE );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
