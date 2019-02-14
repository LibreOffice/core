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

#include <DrawViewShell.hxx>
#include <PresentationViewShell.hxx>
#include <editeng/outliner.hxx>
#include <svx/svxids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/settings.hxx>
#include <sdcommands.h>
#include <sal/log.hxx>

#include <tools/poly.hxx>
#include <svx/fmshell.hxx>
#include <editeng/eeitem.hxx>
#include <svtools/colorcfg.hxx>
#include <AccessibleDrawDocumentView.hxx>

#include <sfx2/viewfrm.hxx>
#include <LayerTabBar.hxx>

#include <app.hrc>
#include <helpids.h>
#include <optsitem.hxx>
#include <sdmod.hxx>
#include <FrameView.hxx>
#include <sdattr.hxx>
#include <futext.hxx>
#include <sdpage.hxx>
#include <stlpool.hxx>
#include <prntopts.hxx>
#include <Window.hxx>
#include <drawview.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <Outliner.hxx>
#include <Client.hxx>
#include <slideshow.hxx>
#include <unokywds.hxx>
#include <SdUnoDrawView.hxx>
#include <ViewShellBase.hxx>
#include <FormShellManager.hxx>
#include <DrawController.hxx>
#include <memory>

namespace sd {

void DrawViewShell::ModelHasChanged()
{
    Invalidate();
    // that the navigator also gets an up to date state
    GetViewFrame()->GetBindings().Invalidate( SID_NAVIGATOR_STATE, true );

    SfxBoolItem aItem( SID_3D_STATE, true );
    GetViewFrame()->GetDispatcher()->ExecuteList(
        SID_3D_STATE, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });

    // now initialize the TextEditOutliner which was newly created by the draw engine
    ::Outliner* pOutliner     = mpDrawView->GetTextEditOutliner();
    if (pOutliner)
    {
        SfxStyleSheetPool* pSPool = static_cast<SfxStyleSheetPool*>( GetDocSh()->GetStyleSheetPool() );
        pOutliner->SetStyleSheetPool(pSPool);
    }
}

void DrawViewShell::Resize()
{
    ViewShell::Resize();

    if ( GetDocSh()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
    {
        SetZoomRect( GetDocSh()->GetVisArea(ASPECT_CONTENT) );
    }

    rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( GetViewShellBase() ) );
    if( xSlideshow.is() && xSlideshow->isRunning() && !xSlideshow->isFullScreen() )
    {
        xSlideshow->resize(maViewSize);
    }
}

void DrawViewShell::ArrangeGUIElements()
{
    // Retrieve the current size (thickness) of the scroll bars.  That is
    // the width of the vertical and the height of the horizontal scroll
    // bar.
    int nScrollBarSize = GetParentWindow()->GetSettings().GetStyleSettings().GetScrollBarSize();
    maScrBarWH = Size (nScrollBarSize, nScrollBarSize);

    ViewShell::ArrangeGUIElements ();

    maTabControl->Hide();

    OSL_ASSERT (GetViewShell()!=nullptr);
    Client* pIPClient = static_cast<Client*>(GetViewShell()->GetIPClient());
    bool bClientActive = false;
    if ( pIPClient && pIPClient->IsObjectInPlaceActive() )
        bClientActive = true;

    bool bInPlaceActive = GetViewFrame()->GetFrame().IsInPlace();

    if ( mbZoomOnPage && !bInPlaceActive && !bClientActive )
    {
        // with split, always resize first window
        //af pWindow = mpContentWindow.get();
        SfxRequest aReq(SID_SIZE_PAGE, SfxCallMode::SLOT, GetDoc()->GetItemPool());
        ExecuteSlot( aReq );
    }
}

/**
 * Apply data of the FrameView on the current view
 */
void DrawViewShell::ReadFrameViewData(FrameView* pView)
{
    ModifyGuard aGuard( GetDoc() );

    // this option has to be adjust at the model
    GetDoc()->SetPickThroughTransparentTextFrames(
             SD_MOD()->GetSdOptions(GetDoc()->GetDocumentType())->IsPickThrough());

    // initialization of the Character-(Screen-) attribute
    if (HasRuler() != pView->HasRuler())
        SetRuler( pView->HasRuler() );

    if (mpDrawView->GetGridCoarse() != pView->GetGridCoarse())
        mpDrawView->SetGridCoarse( pView->GetGridCoarse() );

    if (mpDrawView->GetGridFine() != pView->GetGridFine())
        mpDrawView->SetGridFine( pView->GetGridFine() );

    if (mpDrawView->GetSnapGridWidthX() != pView->GetSnapGridWidthX() || mpDrawView->GetSnapGridWidthY() != pView->GetSnapGridWidthY())
        mpDrawView->SetSnapGridWidth(pView->GetSnapGridWidthX(), pView->GetSnapGridWidthY());

    if (mpDrawView->IsGridVisible() != pView->IsGridVisible())
        mpDrawView->SetGridVisible( pView->IsGridVisible() );

    if (mpDrawView->IsGridFront() != pView->IsGridFront())
        mpDrawView->SetGridFront( pView->IsGridFront() );

    if (mpDrawView->GetSnapAngle() != pView->GetSnapAngle())
        mpDrawView->SetSnapAngle( pView->GetSnapAngle() );

    if (mpDrawView->IsGridSnap() !=  pView->IsGridSnap() )
        mpDrawView->SetGridSnap( pView->IsGridSnap() );

    if (mpDrawView->IsBordSnap() !=  pView->IsBordSnap() )
        mpDrawView->SetBordSnap( pView->IsBordSnap() );

    if (mpDrawView->IsHlplSnap() !=  pView->IsHlplSnap() )
        mpDrawView->SetHlplSnap( pView->IsHlplSnap() );

    if (mpDrawView->IsOFrmSnap() !=  pView->IsOFrmSnap() )
        mpDrawView->SetOFrmSnap( pView->IsOFrmSnap() );

    if (mpDrawView->IsOPntSnap() !=  pView->IsOPntSnap() )
        mpDrawView->SetOPntSnap( pView->IsOPntSnap() );

    if (mpDrawView->IsOConSnap() !=  pView->IsOConSnap() )
        mpDrawView->SetOConSnap( pView->IsOConSnap() );

    if (mpDrawView->IsHlplVisible() != pView->IsHlplVisible() )
        mpDrawView->SetHlplVisible( pView->IsHlplVisible() );

    if (mpDrawView->IsDragStripes() != pView->IsDragStripes() )
        mpDrawView->SetDragStripes( pView->IsDragStripes() );

    if (mpDrawView->IsPlusHandlesAlwaysVisible() != pView->IsPlusHandlesAlwaysVisible() )
        mpDrawView->SetPlusHandlesAlwaysVisible( pView->IsPlusHandlesAlwaysVisible() );

    if (mpDrawView->GetSnapMagneticPixel() != pView->GetSnapMagneticPixel() )
        mpDrawView->SetSnapMagneticPixel( pView->GetSnapMagneticPixel() );

    if (mpDrawView->IsMarkedHitMovesAlways() != pView->IsMarkedHitMovesAlways() )
        mpDrawView->SetMarkedHitMovesAlways( pView->IsMarkedHitMovesAlways() );

    if (mpDrawView->IsMoveOnlyDragging() != pView->IsMoveOnlyDragging() )
        mpDrawView->SetMoveOnlyDragging( pView->IsMoveOnlyDragging() );

    if (mpDrawView->IsNoDragXorPolys() != pView->IsNoDragXorPolys() )
        mpDrawView->SetNoDragXorPolys( pView->IsNoDragXorPolys() );

    if (mpDrawView->IsCrookNoContortion() != pView->IsCrookNoContortion() )
        mpDrawView->SetCrookNoContortion( pView->IsCrookNoContortion() );

    if (mpDrawView->IsAngleSnapEnabled() != pView->IsAngleSnapEnabled() )
        mpDrawView->SetAngleSnapEnabled( pView->IsAngleSnapEnabled() );

    if (mpDrawView->IsBigOrtho() != pView->IsBigOrtho() )
        mpDrawView->SetBigOrtho( pView->IsBigOrtho() );

    if (mpDrawView->IsOrtho() != pView->IsOrtho() )
        mpDrawView->SetOrtho( pView->IsOrtho() );

    if (mpDrawView->GetEliminatePolyPointLimitAngle() != pView->GetEliminatePolyPointLimitAngle() )
        mpDrawView->SetEliminatePolyPointLimitAngle( pView->GetEliminatePolyPointLimitAngle() );

    if (mpDrawView->IsEliminatePolyPoints() != pView->IsEliminatePolyPoints() )
        mpDrawView->SetEliminatePolyPoints( pView->IsEliminatePolyPoints() );

    if (mpDrawView->IsSolidDragging() != pView->IsSolidDragging() )
        mpDrawView->SetSolidDragging( pView->IsSolidDragging() );

    if (mpDrawView->IsQuickTextEditMode() != pView->IsQuickEdit())
        mpDrawView->SetQuickTextEditMode( pView->IsQuickEdit() );

    // #i26631#
    if (mpDrawView->IsMasterPagePaintCaching() != pView->IsMasterPagePaintCaching())
        mpDrawView->SetMasterPagePaintCaching( pView->IsMasterPagePaintCaching() );

    // handle size: 9 pixels
    sal_uInt16 nTmp = mpDrawView->GetMarkHdlSizePixel();
    if( nTmp != 9 )
        mpDrawView->SetMarkHdlSizePixel( 9 );

    SdrPageView* pPageView = mpDrawView->GetSdrPageView();
    if (pPageView)
    {
        if ( pPageView->GetVisibleLayers() != pView->GetVisibleLayers() )
            pPageView->SetVisibleLayers( pView->GetVisibleLayers() );

        if ( pPageView->GetPrintableLayers() != pView->GetPrintableLayers() )
            pPageView->SetPrintableLayers( pView->GetPrintableLayers() );

        if ( pPageView->GetLockedLayers() != pView->GetLockedLayers() )
            pPageView->SetLockedLayers( pView->GetLockedLayers() );

        if (mePageKind == PageKind::Notes)
        {
            if (pPageView->GetHelpLines() != pView->GetNotesHelpLines())
                pPageView->SetHelpLines( pView->GetNotesHelpLines() );
        }
        else if (mePageKind == PageKind::Handout)
        {
            if (pPageView->GetHelpLines() != pView->GetHandoutHelpLines())
                pPageView->SetHelpLines( pView->GetHandoutHelpLines() );
        }
        else
        {
            if (pPageView->GetHelpLines() != pView->GetStandardHelpLines())
                pPageView->SetHelpLines( pView->GetStandardHelpLines() );
        }
    }

    if ( mpDrawView->GetActiveLayer() != pView->GetActiveLayer() )
        mpDrawView->SetActiveLayer( pView->GetActiveLayer() );

    sal_uInt16 nSelectedPage = 0;

    if (mePageKind != PageKind::Handout)
    {
        nSelectedPage = pView->GetSelectedPage();
    }

    EditMode eNewEditMode = pView->GetViewShEditMode(/*mePageKind*/);
    bool bNewLayerMode = pView->IsLayerMode();

    if(IsLayerModeActive() && bNewLayerMode)
    {
        // #i57936# Force mbIsLayerModeActive to false so that ChangeEditMode
        // below does something regarding LayerTabBar content refresh. That refresh
        // is only done when IsLayerModeActive changes. It needs to be done
        // since e.g. Layer vsisibility was changed above and this may need
        // a refresh to show the correct graphical representation
        mbIsLayerModeActive = false;
    }

    ChangeEditMode(eNewEditMode, bNewLayerMode);
    SwitchPage(nSelectedPage);

    // restore DrawMode for 'normal' window
    if(GetActiveWindow()->GetDrawMode() != pView->GetDrawMode())
      GetActiveWindow()->SetDrawMode(pView->GetDrawMode());

    if ( mpDrawView->IsDesignMode() != pView->IsDesignMode() )
    {
        SfxBoolItem aDesignModeItem( SID_FM_DESIGN_MODE, pView->IsDesignMode() );
        GetViewFrame()->GetDispatcher()->ExecuteList(SID_FM_DESIGN_MODE,
                SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                { &aDesignModeItem });
    }

    // has to be called in the end, because it executes a WriteFrameViewData()
    if (mpDrawView->IsFrameDragSingles() != pView->IsFrameDragSingles() )
        mpDrawView->SetFrameDragSingles( pView->IsFrameDragSingles() );
}

/**
 * Apply data of the current view on the FrameView
 */
void DrawViewShell::WriteFrameViewData()
{
    // store character-(screen-) attribute of FrameView
    mpFrameView->SetRuler( HasRuler() );
    mpFrameView->SetGridCoarse( mpDrawView->GetGridCoarse() );
    mpFrameView->SetGridFine( mpDrawView->GetGridFine() );
    mpFrameView->SetSnapGridWidth(mpDrawView->GetSnapGridWidthX(), mpDrawView->GetSnapGridWidthY());
    mpFrameView->SetGridVisible( mpDrawView->IsGridVisible() );
    mpFrameView->SetGridFront( mpDrawView->IsGridFront() );
    mpFrameView->SetSnapAngle( mpDrawView->GetSnapAngle() );
    mpFrameView->SetGridSnap( mpDrawView->IsGridSnap() );
    mpFrameView->SetBordSnap( mpDrawView->IsBordSnap() );
    mpFrameView->SetHlplSnap( mpDrawView->IsHlplSnap() );
    mpFrameView->SetOFrmSnap( mpDrawView->IsOFrmSnap() );
    mpFrameView->SetOPntSnap( mpDrawView->IsOPntSnap() );
    mpFrameView->SetOConSnap( mpDrawView->IsOConSnap() );
    mpFrameView->SetHlplVisible( mpDrawView->IsHlplVisible() );
    mpFrameView->SetDragStripes( mpDrawView->IsDragStripes() );
    mpFrameView->SetPlusHandlesAlwaysVisible( mpDrawView->IsPlusHandlesAlwaysVisible() );
    mpFrameView->SetFrameDragSingles( mpDrawView->IsFrameDragSingles() );
    mpFrameView->SetMarkedHitMovesAlways( mpDrawView->IsMarkedHitMovesAlways() );
    mpFrameView->SetMoveOnlyDragging( mpDrawView->IsMoveOnlyDragging() );
    mpFrameView->SetNoDragXorPolys( mpDrawView->IsNoDragXorPolys() );
    mpFrameView->SetCrookNoContortion( mpDrawView->IsCrookNoContortion() );
    mpFrameView->SetBigOrtho( mpDrawView->IsBigOrtho() );
    mpFrameView->SetEliminatePolyPointLimitAngle( mpDrawView->GetEliminatePolyPointLimitAngle() );
    mpFrameView->SetEliminatePolyPoints( mpDrawView->IsEliminatePolyPoints() );

    mpFrameView->SetSolidDragging( mpDrawView->IsSolidDragging() );
    mpFrameView->SetQuickEdit( mpDrawView->IsQuickTextEditMode() );

    mpFrameView->SetDesignMode( mpDrawView->IsDesignMode() );

    Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
    ::tools::Rectangle aVisArea = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0), aVisSizePixel) );
    mpFrameView->SetVisArea(aVisArea);

    if( mePageKind == PageKind::Handout )
        mpFrameView->SetSelectedPage(0);
    else
    {
        mpFrameView->SetSelectedPage( maTabControl->GetCurPagePos() );
    }

    mpFrameView->SetViewShEditMode(meEditMode);
    mpFrameView->SetLayerMode(IsLayerModeActive());

    SdrPageView* pPageView = mpDrawView->GetSdrPageView();

    if (pPageView)
    {
        if ( mpFrameView->GetVisibleLayers() != pPageView->GetVisibleLayers() )
            mpFrameView->SetVisibleLayers( pPageView->GetVisibleLayers() );

        if ( mpFrameView->GetPrintableLayers() != pPageView->GetPrintableLayers() )
            mpFrameView->SetPrintableLayers( pPageView->GetPrintableLayers() );

        if ( mpFrameView->GetLockedLayers() != pPageView->GetLockedLayers() )
            mpFrameView->SetLockedLayers( pPageView->GetLockedLayers() );

        if (mePageKind == PageKind::Notes)
        {
            mpFrameView->SetNotesHelpLines( pPageView->GetHelpLines() );
        }
        else if (mePageKind == PageKind::Handout)
        {
            mpFrameView->SetHandoutHelpLines( pPageView->GetHelpLines() );
        }
        else
        {
            mpFrameView->SetStandardHelpLines( pPageView->GetHelpLines() );
        }
    }

    if ( mpFrameView->GetActiveLayer() != mpDrawView->GetActiveLayer() )
        mpFrameView->SetActiveLayer( mpDrawView->GetActiveLayer() );

    // store DrawMode for 'normal' window
    if(mpFrameView->GetDrawMode() != GetActiveWindow()->GetDrawMode())
      mpFrameView->SetDrawMode(GetActiveWindow()->GetDrawMode());
}

void DrawViewShell::PrePaint()
{
    mpDrawView->PrePaint();
}

/**
 * The event is forwarded to the Viewshell and the current function by the
 * window pWin.
 *
 * Remark: pWin==NULL, if Paint() is called from ShowWindow!
 */
void DrawViewShell::Paint(const ::tools::Rectangle& rRect, ::sd::Window* pWin)
{
    /* This is done before each text edit, so why not do it before every paint.
                The default language is only used if the outliner only contains one
                character in a symbol font */
    GetDoc()->GetDrawOutliner().SetDefaultLanguage( GetDoc()->GetLanguage( EE_CHAR_LANGUAGE ) );

    // Set Application Background color for usage in SdrPaintView(s)
    mpDrawView->SetApplicationBackgroundColor( mnAppBackgroundColor );

    /* This is done before each text edit, so why not do it before every paint.
                The default language is only used if the outliner only contains one
                character in a symbol font */
    GetDoc()->GetDrawOutliner().SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );

    mpDrawView->CompleteRedraw( pWin, vcl::Region( rRect ) );
}

/**
 * adjust zoom factor for InPlace
 */
void DrawViewShell::SetZoomFactor(const Fraction& rZoomX, const Fraction& rZoomY)
{
    ViewShell::SetZoomFactor(rZoomX, rZoomY);
    mbZoomOnPage = false;
    Point aOrigin = GetActiveWindow()->GetViewOrigin();
    GetActiveWindow()->SetWinViewPos(aOrigin);
}

void DrawViewShell::HidePage()
{
    FmFormShell* pFormShell = GetViewShellBase().GetFormShellManager()->GetFormShell();
    if (pFormShell != nullptr)
        pFormShell->PrepareClose(false);
}

void DrawViewShell::WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >& rSequence )
{
    WriteFrameViewData();

    ViewShell::WriteUserDataSequence( rSequence );

    const sal_Int32 nIndex = rSequence.getLength();
    rSequence.realloc( nIndex + 1 );
    rSequence[nIndex].Name = sUNO_View_ZoomOnPage ;
    rSequence[nIndex].Value <<= mbZoomOnPage;

    // Common SdrModel processing
    GetDocSh()->GetDoc()->WriteUserDataSequence(rSequence);
}

void DrawViewShell::ReadUserDataSequence ( const css::uno::Sequence < css::beans::PropertyValue >& rSequence )
{
    WriteFrameViewData();

    ViewShell::ReadUserDataSequence( rSequence );

    const sal_Int32 nLength = rSequence.getLength();
    const css::beans::PropertyValue *pValue = rSequence.getConstArray();
    for (sal_Int32 i = 0 ; i < nLength; i++, pValue++ )
    {
        if ( pValue->Name == sUNO_View_ZoomOnPage )
        {
            bool bZoomPage = false;
            if( pValue->Value >>= bZoomPage )
            {
                mbZoomOnPage = bZoomPage;
            }
        }
        // Fallback to common SdrModel processing
        else GetDocSh()->GetDoc()->ReadUserDataSequenceValue(pValue);
    }

    // The parameter rSequence contains the config-items from
    // <config:config-item-set config:name="ooo:view-settings">. Determine, whether
    // they contain "VisibleLayers", "PrintableLayers" and "LockedLayers". If not, it
    // is a foreign document or a new document after transition period and the corresponding
    // information were read from <draw:layer-set>. In that case we need to bring
    // the information from model to view.
    bool bHasVisiPrnLockSettings(false);
    for ( auto & rPropertyValue : rSequence )
    {
        if ( rPropertyValue.Name == sUNO_View_VisibleLayers
          || rPropertyValue.Name == sUNO_View_PrintableLayers
          || rPropertyValue.Name == sUNO_View_LockedLayers )
        {
            bHasVisiPrnLockSettings = true;
            break;
        }
    }
    if ( !bHasVisiPrnLockSettings )
    {
        const SdrLayerAdmin& rLayerAdmin = GetDocSh()->GetDoc()->GetLayerAdmin();
        SdrLayerIDSet aSdrLayerIDSet;
        rLayerAdmin.getVisibleLayersODF( aSdrLayerIDSet );
        mpFrameView -> SetVisibleLayers( aSdrLayerIDSet );
        rLayerAdmin.getPrintableLayersODF( aSdrLayerIDSet );
        mpFrameView -> SetPrintableLayers( aSdrLayerIDSet );
        rLayerAdmin.getLockedLayersODF( aSdrLayerIDSet );
        mpFrameView -> SetLockedLayers( aSdrLayerIDSet );
    }


    if( mpFrameView->GetPageKind() != mePageKind )
    {
        mePageKind = mpFrameView->GetPageKind();

        if (mePageKind == PageKind::Notes)
        {
            GetActiveWindow()->SetHelpId( CMD_SID_NOTES_MODE );
        }
        else if (mePageKind == PageKind::Handout)
        {
            GetActiveWindow()->SetHelpId( CMD_SID_HANDOUT_MASTER_MODE );
        }
        else
        {
            GetActiveWindow()->SetHelpId( HID_SDDRAWVIEWSHELL );
        }
    }

    ReadFrameViewData( mpFrameView );

    if( !mbZoomOnPage )
    {
        const ::tools::Rectangle aVisArea( mpFrameView->GetVisArea() );

        if ( GetDocSh()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
        {
            GetDocSh()->SetVisArea(aVisArea);
        }

        VisAreaChanged(aVisArea);

        ::sd::View* pView = GetView();

        if (pView)
        {
            pView->VisAreaChanged(GetActiveWindow());
        }

        SetZoomRect(aVisArea);
    }
    ChangeEditMode (meEditMode, ! IsLayerModeActive());
    ResetActualLayer();
}

void DrawViewShell::VisAreaChanged(const ::tools::Rectangle& rRect)
{
    ViewShell::VisAreaChanged( rRect );

    DrawController& rController = GetViewShellBase().GetDrawController();
    rController.FireVisAreaChanged (rRect);
}

/** If there is a valid controller then create a new instance of
    <type>AccessibleDrawDocumentView</type>.  Otherwise return an empty
    reference.
*/
css::uno::Reference<css::accessibility::XAccessible>
    DrawViewShell::CreateAccessibleDocumentView (::sd::Window* pWindow)
{
    if (GetViewShellBase().GetController() != nullptr)
    {
        accessibility::AccessibleDrawDocumentView* pDocumentView =
            new accessibility::AccessibleDrawDocumentView (
                pWindow,
                this,
                GetViewShellBase().GetController(),
                pWindow->GetAccessibleParentWindow()->GetAccessible());
        pDocumentView->Init();
        return css::uno::Reference<css::accessibility::XAccessible>
            (static_cast< css::uno::XWeak*>(pDocumentView),
                css::uno::UNO_QUERY);
    }

    SAL_WARN("sd", "DrawViewShell::CreateAccessibleDocumentView: no controller");
    return css::uno::Reference< css::accessibility::XAccessible>();
}

int DrawViewShell::GetActiveTabLayerIndex() const
{
    const LayerTabBar* pBar
        = const_cast<DrawViewShell*>(this)->GetLayerTabControl ();
    if (pBar != nullptr)
        return pBar->GetPagePos (pBar->GetCurPageId());
    else
        return -1;
}

void DrawViewShell::SetActiveTabLayerIndex (int nIndex)
{
    LayerTabBar* pBar = GetLayerTabControl ();
    if (pBar == nullptr)
        return;

    // Ignore invalid indices silently.
    if (nIndex>=0 && nIndex<pBar->GetPageCount())
    {
        // Tell the draw view and the tab control of the new active layer.
        mpDrawView->SetActiveLayer (pBar->GetLayerName (pBar->GetPageId (static_cast<sal_uInt16>(nIndex))));
        pBar->SetCurPageId (pBar->GetPageId (static_cast<sal_uInt16>(nIndex)));
        rtl::Reference<SdUnoDrawView> pUnoDrawView(new SdUnoDrawView (
            *this,
            *GetView()));
        css::uno::Reference< css::drawing::XLayer> rLayer = pUnoDrawView->getActiveLayer();
        GetViewShellBase().GetDrawController().fireChangeLayer( &rLayer );
    }
}

LayerTabBar* DrawViewShell::GetLayerTabControl()
{
    return mpLayerTabBar.get();
}

int DrawViewShell::GetTabLayerCount() const
{
    const LayerTabBar* pBar
        = const_cast<DrawViewShell*>(this)->GetLayerTabControl ();
    if (pBar != nullptr)
        return pBar->GetPageCount();
    else
        return 0;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
