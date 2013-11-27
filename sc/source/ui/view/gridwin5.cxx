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
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

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
#include <svx/svdlegacy.hxx>

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
//IAccessibility2 Implementation 2009-----
#include <vcl/svapp.hxx>
//-----IAccessibility2 Implementation 2009

// -----------------------------------------------------------------------

ScHideTextCursor::ScHideTextCursor( ScViewData* pData, ScSplitPos eW ) :
    pViewData(pData),
    eWhich(eW)
{
    Window* pWin = pViewData->GetView()->GetWindowByPos( eWhich );
    if (pWin)
    {
        Cursor* pCur = pWin->GetCursor();
        if ( pCur && pCur->IsVisible() )
            pCur->Hide();
    }
}

ScHideTextCursor::~ScHideTextCursor()
{
    Window* pWin = pViewData->GetView()->GetWindowByPos( eWhich );
    if (pWin)
    {
        //  restore text cursor
        if ( pViewData->HasEditView(eWhich) && pWin->HasFocus() )
            pViewData->GetEditView(eWhich)->ShowCursor( false, true );
    }
}

// -----------------------------------------------------------------------

sal_Bool ScGridWindow::ShowNoteMarker( SCsCOL nPosX, SCsROW nPosY, sal_Bool bKeyboard )
{
    sal_Bool bDone = sal_False;

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB       nTab = pViewData->GetTabNo();
    ScAddress   aCellPos( nPosX, nPosY, nTab );

    String aTrackText;
    sal_Bool bLeftEdge = sal_False;

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
                bLeftEdge = sal_True;

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
            pFound->GetDescription( aTrackText, pDoc );
            if(aComStr.Len()>0)
            {
                aTrackText +=')';
            }
        }
    }

    //  Notiz nur, wenn sie nicht schon auf dem Drawing-Layer angezeigt wird:
    const ScPostIt* pNote = pDoc->GetNote( aCellPos );
    if ( (aTrackText.Len() > 0) || (pNote && !pNote->IsCaptionShown()) )
    {
        sal_Bool bNew = sal_True;
        sal_Bool bFast = sal_False;
        if ( pNoteMarker )          // schon eine Notiz angezeigt
        {
            if ( pNoteMarker->GetDocPos() == aCellPos ) // dieselbe
                bNew = sal_False;                           // dann stehenlassen
            else
                bFast = sal_True;                           // sonst sofort

            //  marker which was shown for ctrl-F1 isn't removed by mouse events
            if ( pNoteMarker->IsByKeyboard() && !bKeyboard )
                bNew = sal_False;
        }
        if ( bNew )
        {
            if ( bKeyboard )
                bFast = sal_True;           // keyboard also shows the marker immediately

            delete pNoteMarker;

            bool bHSplit = pViewData->GetHSplitMode() != SC_SPLIT_NONE;
            bool bVSplit = pViewData->GetVSplitMode() != SC_SPLIT_NONE;

            Window* pLeft = pViewData->GetView()->GetWindowByPos( bVSplit ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT );
            Window* pRight = bHSplit ? pViewData->GetView()->GetWindowByPos( bVSplit ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT ) : 0;
            Window* pBottom = bVSplit ? pViewData->GetView()->GetWindowByPos( SC_SPLIT_BOTTOMLEFT ) : 0;
            Window* pDiagonal = (bHSplit && bVSplit) ? pViewData->GetView()->GetWindowByPos( SC_SPLIT_BOTTOMRIGHT ) : 0;
            DBG_ASSERT( pLeft, "ScGridWindow::ShowNoteMarker - missing top-left grid window" );

            /*  If caption is shown from right or bottom windows, adjust
                mapmode to include size of top-left window. */
            MapMode aMapMode = GetDrawMapMode( sal_True );
            Size aLeftSize = pLeft->PixelToLogic( pLeft->GetOutputSizePixel(), aMapMode );
            Point aOrigin = aMapMode.GetOrigin();
            if( (this == pRight) || (this == pDiagonal) )
                aOrigin.X() += aLeftSize.Width();
            if( (this == pBottom) || (this == pDiagonal) )
                aOrigin.Y() += aLeftSize.Height();
            aMapMode.SetOrigin( aOrigin );

            pNoteMarker = new ScNoteMarker( pLeft, pRight, pBottom, pDiagonal,
                                            pDoc, aCellPos, aTrackText,
                                            aMapMode, bLeftEdge, bFast, bKeyboard );
        }

        bDone = sal_True;       // something is shown (old or new)
    }

    return bDone;
}

// -----------------------------------------------------------------------

void ScGridWindow::RequestHelp(const HelpEvent& rHEvt)
{
    sal_Bool bDone = sal_False;
    sal_Bool bHelpEnabled = ( rHEvt.GetMode() & ( HELPMODE_BALLOON | HELPMODE_QUICK ) ) != 0;
    SdrView* pDrView = pViewData->GetScDrawView();

    sal_Bool bDrawTextEdit = sal_False;
    if (pDrView)
        bDrawTextEdit = pDrView->IsTextEdit();

    //  notes or change tracking

    if ( bHelpEnabled && !bDrawTextEdit )
    {
        Point       aPosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        SCsCOL nPosX;
        SCsROW nPosY;
        pViewData->GetPosFromPixel( aPosPixel.X(), aPosPixel.Y(), eWhich, nPosX, nPosY );

        if ( ShowNoteMarker( nPosX, nPosY, sal_False ) )
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

            if ( eHit != SDRHIT_NONE && aVEvt.mpObj != NULL )
            {
                // URL fuer IMapObject unter Pointer ist Hilfetext
                if ( ScDrawLayer::GetIMapInfo( aVEvt.mpObj ) )
                {
                    const basegfx::B2DPoint aPixelPos(aPosPixel.X(), aPosPixel.Y());
                    const basegfx::B2DPoint aLogicPos(GetInverseViewTransformation() * aPixelPos);
                    IMapObject* pIMapObj = ScDrawLayer::GetHitIMapObject( *aVEvt.mpObj, aLogicPos, *this );

                    if ( pIMapObj )
                    {
                        //  #44990# Bei ImageMaps die Description anzeigen, wenn vorhanden
                        aHelpText = pIMapObj->GetAltText();
                        if (!aHelpText.Len())
                            aHelpText = pIMapObj->GetURL();
                        aPixRect = LogicToPixel(sdr::legacy::GetLogicRect(*aVEvt.mpObj));
                    }
                }
                // URL in shape text or at shape itself (URL in text overrides object URL)
                if ( aHelpText.Len() == 0 )
                {
                    if( aVEvt.meEvent == SDREVENT_EXECUTEURL )
                    {
                        aHelpText = aVEvt.maURLField;
                        aPixRect = LogicToPixel(sdr::legacy::GetLogicRect(*aVEvt.mpObj));
                    }
                    else
                    {
                        SdrObject* pObj = 0;
                        const basegfx::B2DPoint aMDPos(GetInverseViewTransformation() * basegfx::B2DPoint(aPosPixel.X(), aPosPixel.Y()));
                        if ( pDrView->PickObj(aMDPos, pDrView->getHitTolLog(), pObj, SDRSEARCH_ALSOONMASTER) )
                        {
                            if ( pObj->getChildrenOfSdrObject() )
                            {
                                    SdrObject* pHit = 0;
                                    if ( pDrView->PickObj(aMDPos, pDrView->getHitTolLog(), pHit, SDRSEARCH_DEEP ) )
                                        pObj = pHit;
                            }
#ifdef ISSUE66550_HLINK_FOR_SHAPES
                            ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj );
                            if ( pInfo && (pInfo->GetHlink().getLength() > 0) )
                            {
                                aPixRect = LogicToPixel(aVEvt.mpObj->GetLogicRect());
                                aHelpText = pInfo->GetHlink();
                            }
#endif
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

                ScHideTextCursor aHideCursor( pViewData, eWhich );      // MapMode is changed in GetEditArea

                // bForceToTop = sal_False, use the cell's real position
                aPixRect = pViewData->GetEditArea( eWhich, nPosX, nPosY, this, pPattern, sal_False );
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
        DBG_ASSERT( pPV, "SdrPageView* ist NULL" );
        if (pPV)
            bDone = ((ScDrawPage&)pPV->getSdrPageFromSdrPageView()).RequestHelp( this, pDrView, rHEvt );
    }

    //  Wenn QuickHelp fuer AutoFill angezeigt wird, nicht wieder wegnehmen lassen

    if ( nMouseStatus == SC_GM_TABDOWN && pViewData->GetRefType() == SC_REFTYPE_FILL &&
            Help::IsQuickHelpEnabled() )
        bDone = sal_True;

    if (!bDone)
        Window::RequestHelp( rHEvt );
}

sal_Bool ScGridWindow::IsMyModel(SdrEditView* pSdrView)
{
    return pSdrView && &pSdrView->getSdrModelFromSdrView() == pViewData->GetDocument()->GetDrawLayer();
}

void ScGridWindow::HideNoteMarker()
{
    DELETEZ(pNoteMarker);
}

com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
    ScGridWindow::CreateAccessible()
{
//IAccessibility2 Implementation 2009-----
    com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc= GetAccessible(sal_False);
    if (xAcc.is())
    {
        return xAcc;
    }
//-----IAccessibility2 Implementation 2009
    ScAccessibleDocument* pAccessibleDocument =
        new ScAccessibleDocument(GetAccessibleParentWindow()->GetAccessible(),
            pViewData->GetViewShell(), eWhich);
//IAccessibility2 Implementation 2009-----
    //com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > xAccessible = pAccessibleDocument;
    xAcc = pAccessibleDocument;
    SetAccessible(xAcc);
//-----IAccessibility2 Implementation 2009

    pAccessibleDocument->Init();
//IAccessibility2 Implementation 2009-----
    //return xAccessible;
    return xAcc;
//-----IAccessibility2 Implementation 2009
}
//IAccessibility2 Implementation 2009-----
// MT: Removed Windows::SwitchView() introduced with IA2 CWS.
// There are other notifications for this when the active view has chnaged, so please update the code to use that event mechanism
void ScGridWindow::SwitchView()
{
    if (!Application::IsAccessibilityEnabled())
    {
        return ;
    }
    ScAccessibleDocumentBase* pAccDoc = static_cast<ScAccessibleDocumentBase*>(GetAccessible(sal_False).get());
    if (pAccDoc)
    {
        pAccDoc->SwitchViewFireFocus();
    }
}
//-----IAccessibility2 Implementation 2009

