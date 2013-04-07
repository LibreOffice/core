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

#include <editeng/eeitem.hxx>

#include <editeng/flditem.hxx>

#include <editeng/editview.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svtools/imapobj.hxx>
#include <vcl/cursor.hxx>
#include <vcl/help.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/viewfrm.hxx>

#include <unotools/localedatawrapper.hxx>

#include "viewuno.hxx"
#include "AccessibleDocument.hxx"
#include <com/sun/star/accessibility/XAccessible.hpp>

#include "gridwin.hxx"
#include "viewdata.hxx"
#include "drawview.hxx"
#include "drwlayer.hxx"
#include "drawpage.hxx"
#include "document.hxx"
#include "notemark.hxx"
#include "chgtrack.hxx"
#include "chgviset.hxx"
#include "dbfunc.hxx"
#include "tabvwsh.hxx"
#include "userdat.hxx"
#include "postit.hxx"

// -----------------------------------------------------------------------

bool ScGridWindow::ShowNoteMarker( SCsCOL nPosX, SCsROW nPosY, bool bKeyboard )
{
    bool bDone = false;

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB       nTab = pViewData->GetTabNo();
    ScAddress   aCellPos( nPosX, nPosY, nTab );

    String aTrackText;
    bool bLeftEdge = false;

    //  Change-Tracking

    ScChangeTrack* pTrack = pDoc->GetChangeTrack();
    ScChangeViewSettings* pSettings = pDoc->GetChangeViewSettings();
    if ( pTrack && pTrack->GetFirst() && pSettings && pSettings->ShowChanges())
    {
        const ScChangeAction* pFound = NULL;
        const ScChangeAction* pFoundContent = NULL;
        const ScChangeAction* pFoundMove = NULL;
        long nModified = 0;
        const ScChangeAction* pAction = pTrack->GetFirst();
        while (pAction)
        {
            if ( pAction->IsVisible() &&
                 ScViewUtil::IsActionShown( *pAction, *pSettings, *pDoc ) )
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
                        pFound = pAction;       // der letzte gewinnt
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
                        ++nModified;
                    }
                }
                if ( eType == SC_CAT_MOVE )
                {
                    ScRange aRange =
                        ((const ScChangeActionMove*)pAction)->
                        GetFromRange().MakeRange();
                    if ( aRange.In( aCellPos ) )
                    {
                        pFound = pAction;
                        ++nModified;
                    }
                }
            }
            pAction = pAction->GetNext();
        }

        if ( pFound )
        {
            if ( pFoundContent && pFound->GetType() != SC_CAT_CONTENT )
                pFound = pFoundContent;     // Content gewinnt
            if ( pFoundMove && pFound->GetType() != SC_CAT_MOVE &&
                    pFoundMove->GetActionNumber() >
                    pFound->GetActionNumber() )
                pFound = pFoundMove;        // Move gewinnt

            //  bei geloeschten Spalten: Pfeil auf die linke Seite der Zelle
            if ( pFound->GetType() == SC_CAT_DELETE_COLS )
                bLeftEdge = true;

            DateTime aDT = pFound->GetDateTime();
            aTrackText  = pFound->GetUser();
            aTrackText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ", " ));
            aTrackText += ScGlobal::pLocaleData->getDate(aDT);
            aTrackText += ' ';
            aTrackText += ScGlobal::pLocaleData->getTime(aDT);
            aTrackText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ":\n" ));
            String aComStr=pFound->GetComment();
            if(aComStr.Len()>0)
            {
                aTrackText += aComStr;
                aTrackText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "\n( " ));
            }
            OUString aTmp;
            pFound->GetDescription(aTmp, pDoc);
            aTrackText += String(aTmp);
            if(aComStr.Len()>0)
            {
                aTrackText +=')';
            }
        }
    }

    //  Notiz nur, wenn sie nicht schon auf dem Drawing-Layer angezeigt wird:
    const ScPostIt* pNote = pDoc->GetNotes( aCellPos.Tab() )->findByAddress( aCellPos );
    if ( (aTrackText.Len() > 0) || (pNote && !pNote->IsCaptionShown()) )
    {
        bool bNew = true;
        bool bFast = false;
        if ( pNoteMarker )          // schon eine Notiz angezeigt
        {
            if ( pNoteMarker->GetDocPos() == aCellPos ) // dieselbe
                bNew = false;                           // dann stehenlassen
            else
                bFast = true;                           // sonst sofort

            //  marker which was shown for ctrl-F1 isn't removed by mouse events
            if ( pNoteMarker->IsByKeyboard() && !bKeyboard )
                bNew = false;
        }
        if ( bNew )
        {
            if ( bKeyboard )
                bFast = true;           // keyboard also shows the marker immediately

            delete pNoteMarker;

            bool bHSplit = pViewData->GetHSplitMode() != SC_SPLIT_NONE;
            bool bVSplit = pViewData->GetVSplitMode() != SC_SPLIT_NONE;

            Window* pLeft = pViewData->GetView()->GetWindowByPos( bVSplit ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT );
            Window* pRight = bHSplit ? pViewData->GetView()->GetWindowByPos( bVSplit ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT ) : 0;
            Window* pBottom = bVSplit ? pViewData->GetView()->GetWindowByPos( SC_SPLIT_BOTTOMLEFT ) : 0;
            Window* pDiagonal = (bHSplit && bVSplit) ? pViewData->GetView()->GetWindowByPos( SC_SPLIT_BOTTOMRIGHT ) : 0;
            OSL_ENSURE( pLeft, "ScGridWindow::ShowNoteMarker - missing top-left grid window" );

            /*  If caption is shown from right or bottom windows, adjust
                mapmode to include size of top-left window. */
            MapMode aMapMode = GetDrawMapMode( true );
            Size aLeftSize = pLeft->PixelToLogic( pLeft->GetOutputSizePixel(), aMapMode );
            Point aOrigin = aMapMode.GetOrigin();
            if( (this == pRight) || (this == pDiagonal) )
                aOrigin.X() += aLeftSize.Width();
            if( (this == pBottom) || (this == pDiagonal) )
                aOrigin.Y() += aLeftSize.Height();
            aMapMode.SetOrigin( aOrigin );

            pNoteMarker = new ScNoteMarker(  pLeft, pRight, pBottom, pDiagonal,
                                            pDoc, aCellPos, aTrackText,
                                            aMapMode, bLeftEdge, bFast, bKeyboard );
            if ( pViewData->GetScDrawView() )
            {
                // get position for aCellPos
                if ( pDoc )
                {
                    // get draw position in hmm for aCellPos
                    Point aOldPos( pDoc->GetColOffset( aCellPos.Col(), aCellPos.Tab() ), pDoc->GetRowOffset( aCellPos.Row(), aCellPos.Tab() ) );
                    aOldPos.X() = sc::TwipsToHMM( aOldPos.X() );
                    aOldPos.Y() = sc::TwipsToHMM( aOldPos.Y() );
                    // get screen pos in hmm for aCellPos
                    // and equiv screen pos
                    Point aScreenPos = pViewData->GetScrPos( aCellPos.Col(), aCellPos.Row(), eWhich, sal_True );
                    MapMode aDrawMode = GetDrawMapMode();
                    Point aCurPosHmm = PixelToLogic(aScreenPos, aDrawMode );
                    Point aGridOff = aCurPosHmm -aOldPos;
                    pNoteMarker->SetGridOff( aGridOff );
                }
            }
        }

        bDone = true;       // something is shown (old or new)
    }

    return bDone;
}

// -----------------------------------------------------------------------

void ScGridWindow::RequestHelp(const HelpEvent& rHEvt)
{
    sal_Bool bDone = false;
    sal_Bool bHelpEnabled = ( rHEvt.GetMode() & ( HELPMODE_BALLOON | HELPMODE_QUICK ) ) != 0;
    SdrView* pDrView = pViewData->GetScDrawView();

    sal_Bool bDrawTextEdit = false;
    if (pDrView)
        bDrawTextEdit = pDrView->IsTextEdit();

    //  notes or change tracking

    if ( bHelpEnabled && !bDrawTextEdit )
    {
        Point       aPosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        SCsCOL nPosX;
        SCsROW nPosY;
        pViewData->GetPosFromPixel( aPosPixel.X(), aPosPixel.Y(), eWhich, nPosX, nPosY );

        if ( ShowNoteMarker( nPosX, nPosY, false ) )
        {
            Window::RequestHelp( rHEvt );   // alte Tip/Balloon ausschalten
            bDone = sal_True;
        }
    }

    if ( !bDone && pNoteMarker )
    {
        if ( pNoteMarker->IsByKeyboard() )
        {
            //  marker which was shown for ctrl-F1 isn't removed by mouse events
        }
        else
            DELETEZ(pNoteMarker);
    }

    //  Image-Map / Text-URL

    if ( bHelpEnabled && !bDone && !nButtonDown )       // nur ohne gedrueckten Button
    {
        String aHelpText;
        Rectangle aPixRect;
        Point aPosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );

        if ( pDrView )                                      // URL / Image-Map
        {
            SdrViewEvent aVEvt;
            MouseEvent aMEvt( aPosPixel, 1, 0, MOUSE_LEFT );
            SdrHitKind eHit = pDrView->PickAnything( aMEvt, SDRMOUSEBUTTONDOWN, aVEvt );

            if ( eHit != SDRHIT_NONE && aVEvt.pObj != NULL )
            {
                // URL fuer IMapObject unter Pointer ist Hilfetext
                if ( ScDrawLayer::GetIMapInfo( aVEvt.pObj ) )
                {
                    Point aLogicPos = PixelToLogic( aPosPixel );
                    IMapObject* pIMapObj = ScDrawLayer::GetHitIMapObject(
                                                    aVEvt.pObj, aLogicPos, *this );

                    if ( pIMapObj )
                    {
                        //  Bei ImageMaps die Description anzeigen, wenn vorhanden
                        aHelpText = pIMapObj->GetAltText();
                        if (!aHelpText.Len())
                            aHelpText = pIMapObj->GetURL();
                        aPixRect = LogicToPixel(aVEvt.pObj->GetLogicRect());
                    }
                }
                // URL in shape text or at shape itself (URL in text overrides object URL)
                if ( aHelpText.Len() == 0 )
                {
                    if( aVEvt.eEvent == SDREVENT_EXECUTEURL )
                    {
                        aHelpText = aVEvt.pURLField->GetURL();
                        aPixRect = LogicToPixel(aVEvt.pObj->GetLogicRect());
                    }
                    else
                    {
                        SdrObject* pObj = 0;
                        SdrPageView* pPV = 0;
                        Point aMDPos = PixelToLogic( aPosPixel );
                        if ( pDrView->PickObj(aMDPos, pDrView->getHitTolLog(), pObj, pPV, SDRSEARCH_ALSOONMASTER) )
                        {
                            if ( pObj->IsGroupObject() )
                            {
                                    SdrObject* pHit = 0;
                                    if ( pDrView->PickObj(aMDPos, pDrView->getHitTolLog(), pHit, pPV, SDRSEARCH_DEEP ) )
                                        pObj = pHit;
                            }
                            ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj );
                            if ( pInfo && (pInfo->GetHlink().getLength() > 0) )
                            {
                                aPixRect = LogicToPixel(aVEvt.pObj->GetLogicRect());
                                aHelpText = pInfo->GetHlink();
                            }
                        }
                    }
                }
            }
        }

        if ( !aHelpText.Len() )                                 // Text-URL
        {
            String aUrl;
            if ( GetEditUrl( aPosPixel, NULL, &aUrl, NULL ) )
            {
                aHelpText = INetURLObject::decode( aUrl, INET_HEX_ESCAPE,
                    INetURLObject::DECODE_UNAMBIGUOUS );

                ScDocument* pDoc = pViewData->GetDocument();
                SCsCOL nPosX;
                SCsROW nPosY;
                SCTAB       nTab = pViewData->GetTabNo();
                pViewData->GetPosFromPixel( aPosPixel.X(), aPosPixel.Y(), eWhich, nPosX, nPosY );
                const ScPatternAttr* pPattern = pDoc->GetPattern( nPosX, nPosY, nTab );

                // bForceToTop = sal_False, use the cell's real position
                aPixRect = pViewData->GetEditArea( eWhich, nPosX, nPosY, this, pPattern, false );
            }
        }

        if ( aHelpText.Len() )
        {
            Rectangle aScreenRect(OutputToScreenPixel(aPixRect.TopLeft()),
                                    OutputToScreenPixel(aPixRect.BottomRight()));

            if ( rHEvt.GetMode() & HELPMODE_BALLOON )
                Help::ShowBalloon(this,rHEvt.GetMousePosPixel(), aScreenRect, aHelpText);
            else if ( rHEvt.GetMode() & HELPMODE_QUICK )
                Help::ShowQuickHelp(this,aScreenRect, aHelpText);

            bDone = sal_True;
        }
    }

    //  Basic-Controls

    if ( pDrView && bHelpEnabled && !bDone )
    {
        SdrPageView* pPV = pDrView->GetSdrPageView();
        OSL_ENSURE( pPV, "SdrPageView* ist NULL" );
        if (pPV)
            bDone = ((ScDrawPage*)pPV->GetPage())->RequestHelp( this, pDrView, rHEvt );
    }

    //  Wenn QuickHelp fuer AutoFill angezeigt wird, nicht wieder wegnehmen lassen

    if ( nMouseStatus == SC_GM_TABDOWN && pViewData->GetRefType() == SC_REFTYPE_FILL &&
            Help::IsQuickHelpEnabled() )
        bDone = sal_True;

    if (!bDone)
        Window::RequestHelp( rHEvt );
}

bool ScGridWindow::IsMyModel(SdrEditView* pSdrView)
{
    return pSdrView &&
            pSdrView->GetModel() == pViewData->GetDocument()->GetDrawLayer();
}

void ScGridWindow::HideNoteMarker()
{
    DELETEZ(pNoteMarker);
}

com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
    ScGridWindow::CreateAccessible()
{
    ScAccessibleDocument* pAccessibleDocument =
        new ScAccessibleDocument(GetAccessibleParentWindow()->GetAccessible(),
            pViewData->GetViewShell(), eWhich);

    com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > xAccessible = pAccessibleDocument;

    pAccessibleDocument->Init();

    return xAccessible;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
