/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <editeng/eeitem.hxx>

#include <editeng/flditem.hxx>

#include <editeng/editview.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svtools/imapobj.hxx>
#include <vcl/cursor.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/viewfrm.hxx>

#include <unotools/localedatawrapper.hxx>
#include <unotools/securityoptions.hxx>

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
#include "global.hxx"
#include "globstr.hrc"


bool ScGridWindow::ShowNoteMarker( SCsCOL nPosX, SCsROW nPosY, bool bKeyboard )
{
    bool bDone = false;

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB       nTab = pViewData->GetTabNo();
    ScAddress   aCellPos( nPosX, nPosY, nTab );

    OUString aTrackText;
    bool bLeftEdge = false;

    

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
                        pFound = pAction;       
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
                pFound = pFoundContent;     
            if ( pFoundMove && pFound->GetType() != SC_CAT_MOVE &&
                    pFoundMove->GetActionNumber() >
                    pFound->GetActionNumber() )
                pFound = pFoundMove;        

            
            if ( pFound->GetType() == SC_CAT_DELETE_COLS )
                bLeftEdge = true;

            DateTime aDT = pFound->GetDateTime();
            aTrackText  = pFound->GetUser();
            aTrackText += ", ";
            aTrackText += ScGlobal::pLocaleData->getDate(aDT);
            aTrackText += " ";
            aTrackText += ScGlobal::pLocaleData->getTime(aDT);
            aTrackText += ":\n";
            OUString aComStr=pFound->GetComment();
            if(!aComStr.isEmpty())
            {
                aTrackText += aComStr;
                aTrackText += "\n( ";
            }
            OUString aTmp;
            pFound->GetDescription(aTmp, pDoc);
            aTrackText += aTmp;
            if(!aComStr.isEmpty())
            {
                aTrackText += ")";
            }
        }
    }

    
    const ScPostIt* pNote = pDoc->GetNote( aCellPos );
    if ( (!aTrackText.isEmpty()) || (pNote && !pNote->IsCaptionShown()) )
    {
        bool bNew = true;
        bool bFast = false;
        if ( pNoteMarker )          
        {
            if ( pNoteMarker->GetDocPos() == aCellPos ) 
                bNew = false;                           
            else
                bFast = true;                           

            
            if ( pNoteMarker->IsByKeyboard() && !bKeyboard )
                bNew = false;
        }
        if ( bNew )
        {
            if ( bKeyboard )
                bFast = true;           

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
                
                if ( pDoc )
                {
                    
                    Point aOldPos( pDoc->GetColOffset( aCellPos.Col(), aCellPos.Tab() ), pDoc->GetRowOffset( aCellPos.Row(), aCellPos.Tab() ) );
                    aOldPos.X() = sc::TwipsToHMM( aOldPos.X() );
                    aOldPos.Y() = sc::TwipsToHMM( aOldPos.Y() );
                    
                    
                    Point aScreenPos = pViewData->GetScrPos( aCellPos.Col(), aCellPos.Row(), eWhich, true );
                    MapMode aDrawMode = GetDrawMapMode();
                    Point aCurPosHmm = PixelToLogic(aScreenPos, aDrawMode );
                    Point aGridOff = aCurPosHmm -aOldPos;
                    
                    
                    if ( pDoc->IsNegativePage(nTab))
                        aGridOff.setX(aCurPosHmm.getX() + aOldPos.getX());
                    pNoteMarker->SetGridOff( aGridOff );
                }
            }
        }

        bDone = true;       
    }

    return bDone;
}



void ScGridWindow::RequestHelp(const HelpEvent& rHEvt)
{
    
    
    SvtSecurityOptions aSecOpt;
    bool bCtrlClickHlink = aSecOpt.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
    
    OUString aCtrlClickHlinkStr = ScGlobal::GetRscString( STR_CTRLCLICKHYPERLINK );
    
    OUString aClickHlinkStr = ScGlobal::GetRscString( STR_CLICKHYPERLINK );
    sal_Bool bDone = false;
    sal_Bool bHelpEnabled = ( rHEvt.GetMode() & ( HELPMODE_BALLOON | HELPMODE_QUICK ) ) != 0;
    SdrView* pDrView = pViewData->GetScDrawView();
    sal_Bool bDrawTextEdit = false;
    if (pDrView)
        bDrawTextEdit = pDrView->IsTextEdit();
    
    if ( bHelpEnabled && !bDrawTextEdit )
    {
        Point       aPosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        SCsCOL nPosX;
        SCsROW nPosY;
        pViewData->GetPosFromPixel( aPosPixel.X(), aPosPixel.Y(), eWhich, nPosX, nPosY );

        if ( ShowNoteMarker( nPosX, nPosY, false ) )
        {
            Window::RequestHelp( rHEvt );   
            bDone = sal_True;
        }
    }

    if ( !bDone && pNoteMarker )
    {
        if ( pNoteMarker->IsByKeyboard() )
        {
            
        }
        else
            DELETEZ(pNoteMarker);
    }

    

    if ( bHelpEnabled && !bDone && !nButtonDown )       
    {
        OUString aHelpText;
        Rectangle aPixRect;
        Point aPosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );

        if ( pDrView )                                      
        {
            SdrViewEvent aVEvt;
            MouseEvent aMEvt( aPosPixel, 1, 0, MOUSE_LEFT );
            SdrHitKind eHit = pDrView->PickAnything( aMEvt, SDRMOUSEBUTTONDOWN, aVEvt );

            if ( eHit != SDRHIT_NONE && aVEvt.pObj != NULL )
            {
                
                if ( ScDrawLayer::GetIMapInfo( aVEvt.pObj ) )
                {
                    Point aLogicPos = PixelToLogic( aPosPixel );
                    IMapObject* pIMapObj = ScDrawLayer::GetHitIMapObject(
                                                    aVEvt.pObj, aLogicPos, *this );

                    if ( pIMapObj )
                    {
                        
                        aHelpText = pIMapObj->GetAltText();
                        if (aHelpText.isEmpty())
                            aHelpText = pIMapObj->GetURL();
                        if( bCtrlClickHlink )
                        {
                            
                            aHelpText = aCtrlClickHlinkStr + aHelpText;
                        }
                        else
                        {
                            
                            aHelpText = aClickHlinkStr + aHelpText;
                        }
                        aPixRect = LogicToPixel(aVEvt.pObj->GetLogicRect());
                    }
                }
                
                if ( aHelpText.isEmpty() )
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
                                if( bCtrlClickHlink )
                                {
                                    
                                    aHelpText = aCtrlClickHlinkStr + aHelpText;
                                }
                                else
                                {
                                    
                                    aHelpText = aClickHlinkStr + aHelpText;
                                }

                            }
                        }
                    }
                }
            }
        }

        if ( aHelpText.isEmpty() )                                 
        {
            OUString aUrl;
            if ( GetEditUrl( aPosPixel, NULL, &aUrl, NULL ) )
            {
                aHelpText = INetURLObject::decode( aUrl, INET_HEX_ESCAPE,
                    INetURLObject::DECODE_UNAMBIGUOUS );

                if( bCtrlClickHlink )
                {
                    
                    aHelpText = aCtrlClickHlinkStr + aHelpText;
                }
                else
                {
                    
                    aHelpText = aClickHlinkStr + aHelpText;
                }


                ScDocument* pDoc = pViewData->GetDocument();
                SCsCOL nPosX;
                SCsROW nPosY;
                SCTAB       nTab = pViewData->GetTabNo();
                pViewData->GetPosFromPixel( aPosPixel.X(), aPosPixel.Y(), eWhich, nPosX, nPosY );
                const ScPatternAttr* pPattern = pDoc->GetPattern( nPosX, nPosY, nTab );

                
                aPixRect = pViewData->GetEditArea( eWhich, nPosX, nPosY, this, pPattern, false );
            }
        }

        if ( !aHelpText.isEmpty() )
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

    

    if ( pDrView && bHelpEnabled && !bDone )
    {
        SdrPageView* pPV = pDrView->GetSdrPageView();
        OSL_ENSURE( pPV, "SdrPageView* ist NULL" );
        if (pPV)
            bDone = ((ScDrawPage*)pPV->GetPage())->RequestHelp( this, pDrView, rHEvt );
    }

    

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
    com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc= GetAccessible(false);
    if (xAcc.is())
    {
        return xAcc;
    }

    ScAccessibleDocument* pAccessibleDocument =
        new ScAccessibleDocument(GetAccessibleParentWindow()->GetAccessible(),
            pViewData->GetViewShell(), eWhich);

    xAcc = pAccessibleDocument;
    SetAccessible(xAcc);

    pAccessibleDocument->Init();

    return xAcc;
 }




void ScGridWindow::SwitchView()
{
    ScAccessibleDocumentBase* pAccDoc = static_cast<ScAccessibleDocumentBase*>(GetAccessible(false).get());
    if (pAccDoc)
    {
        pAccDoc->SwitchViewFireFocus();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
