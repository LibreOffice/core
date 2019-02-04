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

#include <svtools/toolbarmenu.hxx>
#include <svtools/popupwindowcontroller.hxx>

#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <helpids.h>

#include <svx/drawitem.hxx>
#include <svx/xattr.hxx>
#include <svx/xtable.hxx>
#include <svx/linectrl.hxx>
#include <svx/itemwin.hxx>
#include <svx/dialmgr.hxx>
#include <svx/unoapi.hxx>
#include <memory>

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

SvxLineStyleToolBoxControl::SvxLineStyleToolBoxControl( sal_uInt16 nSlotId,
                                                        sal_uInt16 nId,
                                                        ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    bUpdate         ( false )
{
    addStatusListener( ".uno:LineDash");
    addStatusListener( ".uno:DashListState");
}


SvxLineStyleToolBoxControl::~SvxLineStyleToolBoxControl()
{
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
                pStyleItem.reset( static_cast<XLineStyleItem*>(pState->Clone()) );
            }
            else if( nSID == SID_ATTR_LINE_DASH )
            {
                pDashItem.reset( static_cast<XLineDashItem*>(pState->Clone()) );
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
            eXLS = pStyleItem->GetValue();
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

    if ( auto pDashListItem = dynamic_cast<const SvxDashListItem*>( pState) )
    {
        // The list of line styles has changed
        SvxLineBox* pBox = static_cast<SvxLineBox*>(GetToolBox().GetItemWindow( GetId() ));
        DBG_ASSERT( pBox, "Window not found!" );

        OUString aString( pBox->GetSelectedEntry() );
        pBox->Clear();
        pBox->InsertEntry( SvxResId(RID_SVXSTR_INVISIBLE) );
        pBox->InsertEntry( SvxResId(RID_SVXSTR_SOLID) );
        pBox->Fill( pDashListItem->GetDashList() );
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
                // CD!!! GetCoreMetric();
                pFld->SetCoreUnit( MapUnit::Map100thMM );

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

class SvxLineEndWindow : public svtools::ToolbarPopup
{
private:
    XLineEndListRef mpLineEndList;
    VclPtr<ValueSet> mpLineEndSet;
    sal_uInt16 mnLines;
    Size maBmpSize;
    svt::ToolboxController& mrController;

    DECL_LINK( SelectHdl, ValueSet*, void );
    void FillValueSet();
    void SetSize();

protected:
    virtual void GetFocus() override;

public:
    SvxLineEndWindow( svt::ToolboxController& rController, vcl::Window* pParentWindow );
    virtual ~SvxLineEndWindow() override;
    virtual void dispose() override;
    virtual void statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;
};

static constexpr sal_uInt16 gnCols = 2;

SvxLineEndWindow::SvxLineEndWindow( svt::ToolboxController& rController, vcl::Window* pParentWindow )
    :  ToolbarPopup ( rController.getFrameInterface(), pParentWindow, WB_STDPOPUP | WB_MOVEABLE | WB_CLOSEABLE ),
    mpLineEndSet    ( VclPtr<ValueSet>::Create(this, WinBits( WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT ) )),
    mnLines         ( 12 ),
    mrController    ( rController )
{
    SetText( SvxResId( RID_SVXSTR_LINEEND ) );
    SetHelpId( HID_POPUP_LINEEND );
    mpLineEndSet->SetHelpId( HID_POPUP_LINEEND_CTRL );

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if ( pDocSh )
    {
        const SfxPoolItem*  pItem = pDocSh->GetItem( SID_LINEEND_LIST );
        if( pItem )
            mpLineEndList = static_cast<const SvxLineEndListItem*>( pItem )->GetLineEndList();
    }
    DBG_ASSERT( mpLineEndList.is(), "LineEndList not found" );

    mpLineEndSet->SetSelectHdl( LINK( this, SvxLineEndWindow, SelectHdl ) );
    mpLineEndSet->SetColCount( gnCols );

    // ValueSet fill with entries of LineEndList
    FillValueSet();

    AddStatusListener( ".uno:LineEndListState");

    mpLineEndSet->Show();
}

SvxLineEndWindow::~SvxLineEndWindow()
{
    disposeOnce();
}

void SvxLineEndWindow::dispose()
{
    mpLineEndSet.disposeAndClear();
    ToolbarPopup::dispose();
}

IMPL_LINK_NOARG(SvxLineEndWindow, SelectHdl, ValueSet*, void)
{
    std::unique_ptr<XLineEndItem> pLineEndItem;
    std::unique_ptr<XLineStartItem> pLineStartItem;
    sal_uInt16 nId = mpLineEndSet->GetSelectedItemId();

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

    mrController.dispatchCommand( mrController.getCommandURL(), aArgs );
}

void SvxLineEndWindow::FillValueSet()
{
    if( !mpLineEndList.is() )
        return;

    ScopedVclPtrInstance< VirtualDevice > pVD;

    long nCount = mpLineEndList->Count();

    // First entry: no line end.
    // An entry is temporarily added to get the UI bitmap
    basegfx::B2DPolyPolygon aNothing;
    mpLineEndList->Insert(std::make_unique<XLineEndEntry>(aNothing, SvxResId(RID_SVXSTR_NONE)));
    const XLineEndEntry* pEntry = mpLineEndList->GetLineEnd(nCount);
    BitmapEx aBmp = mpLineEndList->GetUiBitmap( nCount );
    OSL_ENSURE( !aBmp.IsEmpty(), "UI bitmap was not created" );

    maBmpSize = aBmp.GetSizePixel();
    pVD->SetOutputSizePixel( maBmpSize, false );
    maBmpSize.setWidth( maBmpSize.Width() / 2 );
    Point aPt0( 0, 0 );
    Point aPt1( maBmpSize.Width(), 0 );

    pVD->DrawBitmapEx( Point(), aBmp );
    mpLineEndSet->InsertItem(1, Image(pVD->GetBitmapEx(aPt0, maBmpSize)), pEntry->GetName());
    mpLineEndSet->InsertItem(2, Image(pVD->GetBitmapEx(aPt1, maBmpSize)), pEntry->GetName());

    mpLineEndList->Remove(nCount);

    for( long i = 0; i < nCount; i++ )
    {
        pEntry = mpLineEndList->GetLineEnd( i );
        DBG_ASSERT( pEntry, "Could not access LineEndEntry" );
        aBmp = mpLineEndList->GetUiBitmap( i );
        OSL_ENSURE( !aBmp.IsEmpty(), "UI bitmap was not created" );

        pVD->DrawBitmapEx( aPt0, aBmp );
        mpLineEndSet->InsertItem(static_cast<sal_uInt16>((i+1)*2L+1),
                Image(pVD->GetBitmapEx(aPt0, maBmpSize)), pEntry->GetName());
        mpLineEndSet->InsertItem(static_cast<sal_uInt16>((i+2)*2L),
                Image(pVD->GetBitmapEx(aPt1, maBmpSize)), pEntry->GetName());
    }
    mnLines = std::min( static_cast<sal_uInt16>(nCount + 1), sal_uInt16(MAX_LINES) );
    mpLineEndSet->SetLineCount( mnLines );

    SetSize();
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
    sal_uInt16 nMaxLines  = nItemCount / gnCols;

    WinBits nBits = mpLineEndSet->GetStyle();
    if ( mnLines == nMaxLines )
        nBits &= ~WB_VSCROLL;
    else
        nBits |= WB_VSCROLL;
    mpLineEndSet->SetStyle( nBits );

    Size aSize( maBmpSize );
    aSize.AdjustWidth(6 );
    aSize.AdjustHeight(6 );
    aSize = mpLineEndSet->CalcWindowSizePixel( aSize );
    mpLineEndSet->SetPosSizePixel( Point( 2, 2 ), aSize );
    aSize.AdjustWidth(4 );
    aSize.AdjustHeight(4 );
    SetOutputSizePixel( aSize );
}

void SvxLineEndWindow::GetFocus()
{
    if ( mpLineEndSet )
    {
        mpLineEndSet->GrabFocus();
        mpLineEndSet->StartSelection();
    }
}

class SvxLineEndToolBoxControl : public svt::PopupWindowController
{
public:
    explicit SvxLineEndToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence<css::uno::Any>& rArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:
    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;
    using svt::ToolboxController::createPopupWindow;
};

SvxLineEndToolBoxControl::SvxLineEndToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext )
    : svt::PopupWindowController( rContext, nullptr, OUString() )
{
}

void SvxLineEndToolBoxControl::initialize( const css::uno::Sequence<css::uno::Any>& rArguments )
{
    svt::PopupWindowController::initialize( rArguments );
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

VclPtr<vcl::Window> SvxLineEndToolBoxControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<SvxLineEndWindow>::Create( *this, pParent );
}

OUString SvxLineEndToolBoxControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.LineEndToolBoxControl" );
}

css::uno::Sequence<OUString> SvxLineEndToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_LineEndToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxLineEndToolBoxControl( rContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
