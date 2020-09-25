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

#include <editeng/flditem.hxx>

#include <svx/fmpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/ImageMapInfo.hxx>
#include <vcl/imapobj.hxx>
#include <vcl/help.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/sfxhelp.hxx>

#include <AccessibleDocument.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>

#include <gridwin.hxx>
#include <viewdata.hxx>
#include <drawview.hxx>
#include <drwlayer.hxx>
#include <document.hxx>
#include <notemark.hxx>
#include <chgtrack.hxx>
#include <chgviset.hxx>
#include <dbfunc.hxx>
#include <userdat.hxx>
#include <postit.hxx>
#include <global.hxx>

bool ScGridWindow::ShowNoteMarker( SCCOL nPosX, SCROW nPosY, bool bKeyboard )
{
    bool bDone = false;

    ScDocument& rDoc = mrViewData.GetDocument();
    SCTAB       nTab = mrViewData.GetTabNo();
    ScAddress   aCellPos( nPosX, nPosY, nTab );

    OUString aTrackText;
    bool bLeftEdge = false;

    // change tracking

    ScChangeTrack* pTrack = rDoc.GetChangeTrack();
    ScChangeViewSettings* pSettings = rDoc.GetChangeViewSettings();
    if ( pTrack && pTrack->GetFirst() && pSettings && pSettings->ShowChanges())
    {
        const ScChangeAction* pFound = nullptr;
        const ScChangeAction* pFoundContent = nullptr;
        const ScChangeAction* pFoundMove = nullptr;
        const ScChangeAction* pAction = pTrack->GetFirst();
        while (pAction)
        {
            if ( pAction->IsVisible() &&
                 ScViewUtil::IsActionShown( *pAction, *pSettings, rDoc ) )
            {
                ScChangeActionType eType = pAction->GetType();
                const ScBigRange& rBig = pAction->GetBigRange();
                if ( rBig.aStart.Tab() == nTab )
                {
                    ScRange aRange = rBig.MakeRange();

                    if ( eType == SC_CAT_DELETE_ROWS )
                        aRange.aEnd.SetRow( aRange.aStart.Row() );
                    else if ( eType == SC_CAT_DELETE_COLS )
                        aRange.aEnd.SetCol( aRange.aStart.Col() );

                    if ( aRange.In( aCellPos ) )
                    {
                        pFound = pAction;       // the last one wins
                        switch ( eType )
                        {
                            case SC_CAT_CONTENT :
                                pFoundContent = pAction;
                            break;
                            case SC_CAT_MOVE :
                                pFoundMove = pAction;
                            break;
                            default:
                            {
                                // added to avoid warnings
                            }
                        }
                    }
                }
                if ( eType == SC_CAT_MOVE )
                {
                    ScRange aRange =
                        static_cast<const ScChangeActionMove*>(pAction)->
                        GetFromRange().MakeRange();
                    if ( aRange.In( aCellPos ) )
                    {
                        pFound = pAction;
                    }
                }
            }
            pAction = pAction->GetNext();
        }

        if ( pFound )
        {
            if ( pFoundContent && pFound->GetType() != SC_CAT_CONTENT )
                pFound = pFoundContent;     // content wins
            if ( pFoundMove && pFound->GetType() != SC_CAT_MOVE &&
                    pFoundMove->GetActionNumber() >
                    pFound->GetActionNumber() )
                pFound = pFoundMove;        // move wins

            // for deleted columns: Arrow on the left side of the cell
            if ( pFound->GetType() == SC_CAT_DELETE_COLS )
                bLeftEdge = true;

            DateTime aDT = pFound->GetDateTime();
            aTrackText  = pFound->GetUser()
                        + ", "
                        + ScGlobal::getLocaleDataPtr()->getDate(aDT)
                        + " "
                        + ScGlobal::getLocaleDataPtr()->getTime(aDT)
                        + ":\n";
            OUString aComStr=pFound->GetComment();
            if(!aComStr.isEmpty())
            {
                aTrackText += aComStr + "\n( ";
            }
            OUString aTmp;
            pFound->GetDescription(aTmp, rDoc);
            aTrackText += aTmp;
            if(!aComStr.isEmpty())
            {
                aTrackText += ")";
            }
        }
    }

    // Note, only if it is not already displayed on the Drawing Layer:
    const ScPostIt* pNote = rDoc.GetNote( aCellPos );
    if ( (!aTrackText.isEmpty()) || (pNote && !pNote->IsCaptionShown()) )
    {
        bool bNew = true;
        bool bFast = false;
        if (mpNoteMarker) // A note already shown
        {
            if (mpNoteMarker->GetDocPos() == aCellPos)
                bNew = false; // then stop
            else
                bFast = true; // otherwise, at once

            //  marker which was shown for ctrl-F1 isn't removed by mouse events
            if (mpNoteMarker->IsByKeyboard() && !bKeyboard)
                bNew = false;
        }
        if (bNew)
        {
            if (bKeyboard)
                bFast = true; // keyboard also shows the marker immediately

            mpNoteMarker.reset();

            bool bHSplit = mrViewData.GetHSplitMode() != SC_SPLIT_NONE;
            bool bVSplit = mrViewData.GetVSplitMode() != SC_SPLIT_NONE;

            vcl::Window* pLeft = mrViewData.GetView()->GetWindowByPos( bVSplit ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT );
            vcl::Window* pRight = bHSplit ? mrViewData.GetView()->GetWindowByPos( bVSplit ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT ) : nullptr;
            vcl::Window* pBottom = bVSplit ? mrViewData.GetView()->GetWindowByPos( SC_SPLIT_BOTTOMLEFT ) : nullptr;
            vcl::Window* pDiagonal = (bHSplit && bVSplit) ? mrViewData.GetView()->GetWindowByPos( SC_SPLIT_BOTTOMRIGHT ) : nullptr;
            OSL_ENSURE( pLeft, "ScGridWindow::ShowNoteMarker - missing top-left grid window" );

            /*  If caption is shown from right or bottom windows, adjust
                mapmode to include size of top-left window. */
            MapMode aMapMode = GetDrawMapMode( true );
            Size aLeftSize = pLeft->PixelToLogic( pLeft->GetOutputSizePixel(), aMapMode );
            Point aOrigin = aMapMode.GetOrigin();
            if( (this == pRight) || (this == pDiagonal) )
                aOrigin.AdjustX(aLeftSize.Width() );
            if( (this == pBottom) || (this == pDiagonal) )
                aOrigin.AdjustY(aLeftSize.Height() );
            aMapMode.SetOrigin( aOrigin );

            mpNoteMarker.reset(new ScNoteMarker(pLeft, pRight, pBottom, pDiagonal,
                                                &rDoc, aCellPos, aTrackText,
                                                aMapMode, bLeftEdge, bFast, bKeyboard));
        }

        bDone = true;       // something is shown (old or new)
    }

    return bDone;
}

void ScGridWindow::RequestHelp(const HelpEvent& rHEvt)
{
    bool bDone = false;
    bool bHelpEnabled = bool(rHEvt.GetMode() & ( HelpEventMode::BALLOON | HelpEventMode::QUICK ));
    SdrView* pDrView = mrViewData.GetScDrawView();
    bool bDrawTextEdit = false;
    if (pDrView)
        bDrawTextEdit = pDrView->IsTextEdit();
    //  notes or change tracking
    if ( bHelpEnabled && !bDrawTextEdit )
    {
        Point       aPosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        SCCOL nPosX;
        SCROW nPosY;
        mrViewData.GetPosFromPixel( aPosPixel.X(), aPosPixel.Y(), eWhich, nPosX, nPosY );

        if ( ShowNoteMarker( nPosX, nPosY, false ) )
        {
            Window::RequestHelp( rHEvt );   // turn off old Tip/Balloon
            bDone = true;
        }
    }

    if (!bDone && mpNoteMarker)
    {
        if (mpNoteMarker->IsByKeyboard())
        {
            //  marker which was shown for ctrl-F1 isn't removed by mouse events
        }
        else
        {
            mpNoteMarker.reset();
        }
    }

    //  Image-Map / Text-URL

    if ( bHelpEnabled && !bDone && !nButtonDown )       // only without pressed button
    {
        OUString aHelpText;
        tools::Rectangle aPixRect;
        Point aPosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );

        if ( pDrView )                                      // URL / Image-Map
        {
            SdrViewEvent aVEvt;
            MouseEvent aMEvt( aPosPixel, 1, MouseEventModifiers::NONE, MOUSE_LEFT );
            SdrHitKind eHit = pDrView->PickAnything( aMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt );

            if ( eHit != SdrHitKind::NONE && aVEvt.pObj != nullptr )
            {
                // URL for IMapObject below Pointer is help text
                if ( SvxIMapInfo::GetIMapInfo( aVEvt.pObj ) )
                {
                    Point aLogicPos = PixelToLogic( aPosPixel );
                    IMapObject* pIMapObj = SvxIMapInfo::GetHitIMapObject(
                                                    aVEvt.pObj, aLogicPos, this );

                    if ( pIMapObj )
                    {
                        // For image maps show the description, if available
                        aHelpText = pIMapObj->GetAltText();
                        if (aHelpText.isEmpty())
                            aHelpText = SfxHelp::GetURLHelpText(pIMapObj->GetURL());
                        aPixRect = LogicToPixel(aVEvt.pObj->GetLogicRect());
                    }
                }
                // URL in shape text or at shape itself (URL in text overrides object URL)
                if ( aHelpText.isEmpty() )
                {
                    if( aVEvt.eEvent == SdrEventKind::ExecuteUrl )
                    {
                        aHelpText = SfxHelp::GetURLHelpText(aVEvt.pURLField->GetURL());
                        aPixRect = LogicToPixel(aVEvt.pObj->GetLogicRect());
                    }
                    else
                    {
                        SdrPageView* pPV = nullptr;
                        Point aMDPos = PixelToLogic( aPosPixel );
                        SdrObject* pObj = pDrView->PickObj(aMDPos, pDrView->getHitTolLog(), pPV, SdrSearchOptions::ALSOONMASTER);
                        if (pObj)
                        {
                            if ( pObj->IsGroupObject() )
                            {
                                    SdrObject* pHit = pDrView->PickObj(aMDPos, pDrView->getHitTolLog(), pPV, SdrSearchOptions::DEEP);
                                    if (pHit)
                                        pObj = pHit;
                            }
                            ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj );
                            if ( pInfo && (pInfo->GetHlink().getLength() > 0) )
                            {
                                aPixRect = LogicToPixel(aVEvt.pObj->GetLogicRect());
                                aHelpText = SfxHelp::GetURLHelpText(pInfo->GetHlink());
                            }
                        }
                    }
                }
            }
        }

        if ( aHelpText.isEmpty() )                                 // Text-URL
        {
            OUString aUrl;
            if ( GetEditUrl( aPosPixel, nullptr, &aUrl ) )
            {
                aHelpText = SfxHelp::GetURLHelpText(
                    INetURLObject::decode(aUrl, INetURLObject::DecodeMechanism::Unambiguous));

                ScDocument& rDoc = mrViewData.GetDocument();
                SCCOL nPosX;
                SCROW nPosY;
                SCTAB       nTab = mrViewData.GetTabNo();
                mrViewData.GetPosFromPixel( aPosPixel.X(), aPosPixel.Y(), eWhich, nPosX, nPosY );
                const ScPatternAttr* pPattern = rDoc.GetPattern( nPosX, nPosY, nTab );

                // bForceToTop = sal_False, use the cell's real position
                aPixRect = mrViewData.GetEditArea( eWhich, nPosX, nPosY, this, pPattern, false );
            }
        }

        if ( !aHelpText.isEmpty() )
        {
            tools::Rectangle aScreenRect(OutputToScreenPixel(aPixRect.TopLeft()),
                                         OutputToScreenPixel(aPixRect.BottomRight()));

            if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
                Help::ShowBalloon(this,rHEvt.GetMousePosPixel(), aScreenRect, aHelpText);
            else if ( rHEvt.GetMode() & HelpEventMode::QUICK )
                Help::ShowQuickHelp(this,aScreenRect, aHelpText);

            bDone = true;
        }
    }

    // basic controls

    if ( pDrView && bHelpEnabled && !bDone )
    {
        SdrPageView* pPV = pDrView->GetSdrPageView();
        OSL_ENSURE( pPV, "SdrPageView* is NULL" );
        if (pPV)
            bDone = FmFormPage::RequestHelp( this, pDrView, rHEvt );
    }

    // If QuickHelp for AutoFill is shown, do not allow it to be removed

    if ( nMouseStatus == SC_GM_TABDOWN && mrViewData.GetRefType() == SC_REFTYPE_FILL &&
            Help::IsQuickHelpEnabled() )
        bDone = true;

    if (!bDone)
        Window::RequestHelp( rHEvt );
}

bool ScGridWindow::IsMyModel(const SdrEditView* pSdrView)
{
    return pSdrView &&
            pSdrView->GetModel() == mrViewData.GetDocument().GetDrawLayer();
}

void ScGridWindow::HideNoteMarker()
{
    mpNoteMarker.reset();
}

css::uno::Reference< css::accessibility::XAccessible >
    ScGridWindow::CreateAccessible()
{
    css::uno::Reference< css::accessibility::XAccessible > xAcc= GetAccessible(false);
    if (xAcc.is())
    {
        return xAcc;
    }

    ScAccessibleDocument* pAccessibleDocument =
        new ScAccessibleDocument(GetAccessibleParentWindow()->GetAccessible(),
            mrViewData.GetViewShell(), eWhich);
    pAccessibleDocument->PreInit();

    xAcc = pAccessibleDocument;
    SetAccessible(xAcc);

    pAccessibleDocument->Init();

    return xAcc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
