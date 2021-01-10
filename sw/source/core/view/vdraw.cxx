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

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <swmodule.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svx/svdpagv.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>

#include <svx/svdoutl.hxx>

#include <drawdoc.hxx>
#include <fesh.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <viewimp.hxx>
#include <dflyobj.hxx>
#include <printdata.hxx>
#include <dcontact.hxx>
#include <dview.hxx>
#include <flyfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/canvastools.hxx>
#include <sal/log.hxx>

#include <basegfx/range/b2irectangle.hxx>

#include <IDocumentDrawModelAccess.hxx>

void SwViewShellImp::StartAction()
{
    if ( HasDrawView() )
    {
        CurrShell aCurr( GetShell() );
        if ( auto pFEShell = dynamic_cast<SwFEShell*>( m_pShell) )
            pFEShell->HideChainMarker(); // might have changed
    }
}

void SwViewShellImp::EndAction()
{
    if ( HasDrawView() )
    {
        CurrShell aCurr( GetShell() );
        if ( auto pFEShell = dynamic_cast<SwFEShell*>(m_pShell) )
            pFEShell->SetChainMarker(); // might have changed
    }
}

void SwViewShellImp::LockPaint()
{
    if ( HasDrawView() )
    {
        m_bResetHdlHiddenPaint = !GetDrawView()->areMarkHandlesHidden();
        GetDrawView()->hideMarkHandles();
    }
    else
    {
        m_bResetHdlHiddenPaint = false;
    }
}

void SwViewShellImp::UnlockPaint()
{
    if ( m_bResetHdlHiddenPaint )
        GetDrawView()->showMarkHandles();
}

void SwViewShellImp::PaintLayer( const SdrLayerID _nLayerID,
                            SwPrintData const*const pPrintData,
                            SwPageFrame const& rPageFrame,
                            const SwRect& aPaintRect,
                            const Color* _pPageBackgrdColor,
                            const bool _bIsPageRightToLeft,
                            sdr::contact::ViewObjectContactRedirector* pRedirector )
{
    if ( !HasDrawView() )
        return;

    //change the draw mode in high contrast mode
    OutputDevice* pOutDev = GetShell()->GetOut();
    DrawModeFlags nOldDrawMode = pOutDev->GetDrawMode();
    if( GetShell()->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() &&
        (!GetShell()->IsPreview()||SW_MOD()->GetAccessibilityOptions().GetIsForPagePreviews()))
    {
        pOutDev->SetDrawMode( nOldDrawMode | DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill |
                            DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient );
    }

    // For correct handling of accessibility, high contrast, the
    // page background color is set as the background color at the
    // outliner of the draw view.  Only necessary for the layers
    // hell and heaven
    Color aOldOutlinerBackgrdColor;
    // set default horizontal text direction on painting <hell> or
    // <heaven>.
    EEHorizontalTextDirection aOldEEHoriTextDir = EEHorizontalTextDirection::L2R;
    const IDocumentDrawModelAccess& rIDDMA = GetShell()->getIDocumentDrawModelAccess();
    if ( (_nLayerID == rIDDMA.GetHellId()) ||
         (_nLayerID == rIDDMA.GetHeavenId()) )
    {
        OSL_ENSURE( _pPageBackgrdColor,
                "incorrect usage of SwViewShellImp::PaintLayer: pPageBackgrdColor have to be set for painting layer <hell> or <heaven>");
        if ( _pPageBackgrdColor )
        {
            aOldOutlinerBackgrdColor =
                    GetDrawView()->GetModel()->GetDrawOutliner().GetBackgroundColor();
            GetDrawView()->GetModel()->GetDrawOutliner().SetBackgroundColor( *_pPageBackgrdColor );
        }

        aOldEEHoriTextDir =
            GetDrawView()->GetModel()->GetDrawOutliner().GetDefaultHorizontalTextDirection();
        EEHorizontalTextDirection aEEHoriTextDirOfPage =
            _bIsPageRightToLeft ? EEHorizontalTextDirection::R2L : EEHorizontalTextDirection::L2R;
        GetDrawView()->GetModel()->GetDrawOutliner().SetDefaultHorizontalTextDirection( aEEHoriTextDirOfPage );
    }

    pOutDev->Push( PushFlags::LINECOLOR );
    if (pPrintData)
    {
        // hide drawings but not form controls (form controls are handled elsewhere)
        SdrView &rSdrView = GetPageView()->GetView();
        rSdrView.setHideDraw( !pPrintData->IsPrintDraw() );
    }
    basegfx::B2IRectangle const pageFrame = vcl::unotools::b2IRectangleFromRectangle(rPageFrame.getFrameArea().SVRect());
    GetPageView()->DrawLayer(_nLayerID, pOutDev, pRedirector, aPaintRect.SVRect(), &pageFrame);
    pOutDev->Pop();

    // reset background color of the outliner & default horiz. text dir.
    if ( (_nLayerID == rIDDMA.GetHellId()) ||
         (_nLayerID == rIDDMA.GetHeavenId()) )
    {
        GetDrawView()->GetModel()->GetDrawOutliner().SetBackgroundColor( aOldOutlinerBackgrdColor );
        GetDrawView()->GetModel()->GetDrawOutliner().SetDefaultHorizontalTextDirection( aOldEEHoriTextDir );
    }

    pOutDev->SetDrawMode( nOldDrawMode );

}

#define FUZZY_EDGE 400

bool SwViewShellImp::IsDragPossible( const Point &rPoint )
{
    if ( !HasDrawView() )
        return false;

    const SdrMarkList &rMrkList = GetDrawView()->GetMarkedObjectList();

    if( !rMrkList.GetMarkCount() )
        return false;

    SdrObject *pO = rMrkList.GetMark(rMrkList.GetMarkCount()-1)->GetMarkedSdrObj();

    SwRect aRect;
    if( pO && ::CalcClipRect( pO, aRect, false ) )
    {
        SwRect aTmp;
        ::CalcClipRect( pO, aTmp );
        aRect.Union( aTmp );
    }
    else
        aRect = GetShell()->GetLayout()->getFrameArea();

    aRect.AddTop   (- FUZZY_EDGE );
    aRect.AddBottom(  FUZZY_EDGE );
    aRect.AddLeft  (- FUZZY_EDGE );
    aRect.AddRight (  FUZZY_EDGE );
    return aRect.IsInside( rPoint );
}

void SwViewShellImp::NotifySizeChg( const Size &rNewSz )
{
    if ( !HasDrawView() )
        return;

    if ( GetPageView() )
        GetPageView()->GetPage()->SetSize( rNewSz );

    // Limitation of the work area
    const tools::Rectangle aDocRect( Point( DOCUMENTBORDER, DOCUMENTBORDER ), rNewSz );
    const tools::Rectangle &rOldWork = GetDrawView()->GetWorkArea();
    bool bCheckDrawObjs = false;
    if ( aDocRect != rOldWork )
    {
        if ( rOldWork.Bottom() > aDocRect.Bottom() || rOldWork.Right() > aDocRect.Right())
            bCheckDrawObjs = true;
        GetDrawView()->SetWorkArea( aDocRect );
    }
    if ( !bCheckDrawObjs )
        return;

    OSL_ENSURE( m_pShell->getIDocumentDrawModelAccess().GetDrawModel(), "NotifySizeChg without DrawModel" );
    SdrPage* pPage = m_pShell->getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 );
    const size_t nObjs = pPage->GetObjCount();
    for( size_t nObj = 0; nObj < nObjs; ++nObj )
    {
        SdrObject *pObj = pPage->GetObj( nObj );
        if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
        {
            // Objects not anchored to the frame, do not need to be adjusted
            const SwContact *pCont = GetUserCall(pObj);
            // this function might be called by the InsertDocument, when
            // a PageDesc-Attribute is set on a node. Then the SdrObject
            // must not have an UserCall.
            if( !pCont )
                continue;
            auto pDrawContact = dynamic_cast<const SwDrawContact*>( pCont);
            if( !pDrawContact )
                continue;

            const SwFrame *pAnchor = pDrawContact->GetAnchorFrame();
            if ( !pAnchor || pAnchor->IsInFly() || !pAnchor->isFrameAreaDefinitionValid() ||
                 !pAnchor->GetUpper() || !pAnchor->FindPageFrame() ||
                 (RndStdIds::FLY_AS_CHAR == pCont->GetFormat()->GetAnchor().GetAnchorId()) )
            {
                continue;
            }
            else
            {
                // Actually this should never happen but currently layouting
                // is broken. So don't move anchors, if the page is invalid.
                // This should be turned into a DBG_ASSERT, once layouting is fixed!
                const SwPageFrame *pPageFrame = pAnchor->FindPageFrame();
                if (!pPageFrame || pPageFrame->IsInvalid() ) {
                    SAL_WARN( "sw.core", "Trying to move anchor from invalid page - fix layouting!" );
                    continue;
                }
            }

            // no move for drawing objects in header/footer
            if ( pAnchor->FindFooterOrHeader() )
            {
                continue;
            }

            const tools::Rectangle aObjBound( pObj->GetCurrentBoundRect() );
            if ( !aDocRect.IsInside( aObjBound ) )
            {
                Size aSz;
                if ( aObjBound.Left() > aDocRect.Right() )
                    aSz.setWidth( (aDocRect.Right() - aObjBound.Left()) - MINFLY );
                if ( aObjBound.Top() > aDocRect.Bottom() )
                    aSz.setHeight( (aDocRect.Bottom() - aObjBound.Top()) - MINFLY );
                if ( aSz.Width() || aSz.Height() )
                    pObj->Move( aSz );

                // Don't let large objects disappear to the top
                aSz.setWidth(0);
                aSz.setHeight(0);
                if ( aObjBound.Right() < aDocRect.Left() )
                    aSz.setWidth( (aDocRect.Left() - aObjBound.Right()) + MINFLY );
                if ( aObjBound.Bottom() < aDocRect.Top() )
                    aSz.setHeight( (aDocRect.Top() - aObjBound.Bottom()) + MINFLY );
                if ( aSz.Width() || aSz.Height() )
                    pObj->Move( aSz );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
