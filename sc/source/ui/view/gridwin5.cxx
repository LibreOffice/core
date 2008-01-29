/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gridwin5.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:49:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <svx/eeitem.hxx>

#include <svx/flditem.hxx>

#include <svx/editview.hxx>
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
            pViewData->GetEditView(eWhich)->ShowCursor( FALSE, TRUE );
    }
}

// -----------------------------------------------------------------------

BOOL ScGridWindow::ShowNoteMarker( SCsCOL nPosX, SCsROW nPosY, BOOL bKeyboard )
{
    BOOL bDone = FALSE;

    ScDocument* pDoc = pViewData->GetDocument();
    ScPostIt    aNote(pDoc);
    SCTAB       nTab = pViewData->GetTabNo();
    ScAddress   aCellPos( nPosX, nPosY, nTab );

    String aTrackText;
    BOOL bLeftEdge = FALSE;

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
                bLeftEdge = TRUE;

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
    if ( aTrackText.Len() || ( pDoc->GetNote( nPosX, nPosY, nTab, aNote ) &&
                                 !pDoc->HasNoteObject( nPosX, nPosY, nTab ) ) )
    {
        BOOL bNew = TRUE;
        BOOL bFast = FALSE;
        if ( pNoteMarker )          // schon eine Notiz angezeigt
        {
            if ( pNoteMarker->GetDocPos() == aCellPos ) // dieselbe
                bNew = FALSE;                           // dann stehenlassen
            else
                bFast = TRUE;                           // sonst sofort

            //  marker which was shown for ctrl-F1 isn't removed by mouse events
            if ( pNoteMarker->IsByKeyboard() && !bKeyboard )
                bNew = FALSE;
        }
        if ( bNew )
        {
            if ( bKeyboard )
                bFast = TRUE;           // keyboard also shows the marker immediately

            delete pNoteMarker;

            Window* pRight = NULL;
            Window* pBottom = NULL;
            Window* pDiagonal = NULL;
            if ( pViewData->GetHSplitMode() == SC_SPLIT_FIX && eHWhich == SC_SPLIT_LEFT )
            {
                ScSplitPos eRight = ( eVWhich == SC_SPLIT_TOP ) ?
                                        SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT;
                pRight = pViewData->GetView()->GetWindowByPos(eRight);
            }
            if ( pViewData->GetVSplitMode() == SC_SPLIT_FIX && eVWhich == SC_SPLIT_TOP )
            {
                ScSplitPos eBottom = ( eHWhich == SC_SPLIT_LEFT ) ?
                                        SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;
                pBottom = pViewData->GetView()->GetWindowByPos(eBottom);
            }
            if ( pRight && pBottom )
                pDiagonal = pViewData->GetView()->GetWindowByPos(SC_SPLIT_BOTTOMRIGHT);

            pNoteMarker = new ScNoteMarker( this, pRight, pBottom, pDiagonal,
                                            pDoc, aCellPos, aTrackText,
                                            GetDrawMapMode(TRUE), bLeftEdge, bFast, bKeyboard );
        }

        bDone = TRUE;       // something is shown (old or new)
    }

    return bDone;
}

// -----------------------------------------------------------------------

void ScGridWindow::RequestHelp(const HelpEvent& rHEvt)
{
    BOOL bDone = FALSE;
    BOOL bHelpEnabled = ( rHEvt.GetMode() & ( HELPMODE_BALLOON | HELPMODE_QUICK ) ) != 0;
    SdrView* pDrView = pViewData->GetScDrawView();

    BOOL bDrawTextEdit = FALSE;
    if (pDrView)
        bDrawTextEdit = pDrView->IsTextEdit();

    //  notes or change tracking

    if ( bHelpEnabled && !bDrawTextEdit )
    {
        Point       aPosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        SCsCOL nPosX;
        SCsROW nPosY;
        pViewData->GetPosFromPixel( aPosPixel.X(), aPosPixel.Y(), eWhich, nPosX, nPosY );

        if ( ShowNoteMarker( nPosX, nPosY, FALSE ) )
        {
            Window::RequestHelp( rHEvt );   // alte Tip/Balloon ausschalten
            bDone = TRUE;
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
                        //  #44990# Bei ImageMaps die Description anzeigen, wenn vorhanden
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
                        if ( pDrView->PickObj(aMDPos, pObj, pPV, SDRSEARCH_ALSOONMASTER) )
                        {
                            if ( pObj->IsGroupObject() )
                            {
                                    SdrObject* pHit = 0;
                                    if ( pDrView->PickObj(aMDPos, pHit, pPV, SDRSEARCH_DEEP ) )
                                        pObj = pHit;
                            }
#ifdef ISSUE66550_HLINK_FOR_SHAPES
                            ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj );
                            if ( pInfo && (pInfo->GetHlink().getLength() > 0) )
                            {
                                aPixRect = LogicToPixel(aVEvt.pObj->GetLogicRect());
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

                // bForceToTop = FALSE, use the cell's real position
                aPixRect = pViewData->GetEditArea( eWhich, nPosX, nPosY, this, pPattern, FALSE );
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

            bDone = TRUE;
        }
    }

    //  Basic-Controls

    if ( pDrView && bHelpEnabled && !bDone )
    {
        SdrPageView* pPV = pDrView->GetSdrPageView();
        DBG_ASSERT( pPV, "SdrPageView* ist NULL" );
        if (pPV)
            bDone = ((ScDrawPage*)pPV->GetPage())->RequestHelp( this, pDrView, rHEvt );
    }

    //  Wenn QuickHelp fuer AutoFill angezeigt wird, nicht wieder wegnehmen lassen

    if ( nMouseStatus == SC_GM_TABDOWN && pViewData->GetRefType() == SC_REFTYPE_FILL &&
            Help::IsQuickHelpEnabled() )
        bDone = TRUE;

    if (!bDone)
        Window::RequestHelp( rHEvt );
}

BOOL ScGridWindow::IsMyModel(SdrEditView* pSdrView)
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
