/*************************************************************************
 *
 *  $RCSfile: gridwin5.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#define ITEMID_FIELD EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svtools/imapobj.hxx>
#include <vcl/help.hxx>
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif


// INCLUDE ---------------------------------------------------------------

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


// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

void __EXPORT ScGridWindow::RequestHelp(const HelpEvent& rHEvt)
{
    BOOL bDone = FALSE;
    BOOL bHelpEnabled = ( rHEvt.GetMode() & ( HELPMODE_BALLOON | HELPMODE_QUICK ) ) != 0;
    SdrView* pDrView = pViewData->GetScDrawView();

    BOOL bDrawTextEdit = FALSE;
    if (pDrView)
        bDrawTextEdit = pDrView->IsTextEdit();

    //  Notizen

    if ( bHelpEnabled && !bDrawTextEdit )
    {
        ScPostIt    aNote;
        Point       aPosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        ScDocument* pDoc = pViewData->GetDocument();
        short       nPosX, nPosY;
        pViewData->GetPosFromPixel( aPosPixel.X(), aPosPixel.Y(), eWhich, nPosX, nPosY );
        USHORT      nTab = pViewData->GetTabNo();
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
                aTrackText += ScGlobal::pScInternational->GetDate(aDT);
                aTrackText += ' ';
                aTrackText += ScGlobal::pScInternational->GetTime(aDT);
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
            }
            if ( bNew )
            {
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
                                                GetDrawMapMode(TRUE), bLeftEdge, bFast );
            }

            Window::RequestHelp( rHEvt );   // alte Tip/Balloon ausschalten
            bDone = TRUE;
        }
    }

    if (!bDone)
        DELETEZ(pNoteMarker);

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
                        aHelpText = pIMapObj->GetDescription();
                        if (!aHelpText.Len())
                            aHelpText = pIMapObj->GetURL();
                        aPixRect = LogicToPixel(aVEvt.pObj->GetLogicRect());
                    }
                }
                // URL in Textobjekt
                if ( !aHelpText.Len() && aVEvt.eEvent == SDREVENT_EXECUTEURL )
                {
                    aHelpText = aVEvt.pURLField->GetURL();
                    aPixRect = LogicToPixel(aVEvt.pObj->GetLogicRect());
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
                short       nPosX, nPosY;
                USHORT      nTab = pViewData->GetTabNo();
                pViewData->GetPosFromPixel( aPosPixel.X(), aPosPixel.Y(), eWhich, nPosX, nPosY );
                const ScPatternAttr* pPattern = pDoc->GetPattern( nPosX, nPosY, nTab );
                aPixRect = pViewData->GetEditArea( eWhich, nPosX, nPosY, this, pPattern );
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
        SdrPageView* pPV = pDrView->GetPageViewPvNum(0);
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

