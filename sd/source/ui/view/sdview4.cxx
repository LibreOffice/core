/*************************************************************************
 *
 *  $RCSfile: sdview4.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:44 $
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

class SbxArray;

#pragma hdrstop

#include <tools/ref.hxx>

#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif

#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SOT_SOTREF_HXX //autogen
#include <sot/sotref.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif


#include "app.hrc"
#include "strings.hrc"

#include "sdview.hxx"
#include "sdmod.hxx"
#include "drviewsh.hxx"
#include "navigatr.hxx"
#include "docshell.hxx"
#include "sdwindow.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "anminfo.hxx"
#include "fupoor.hxx"
#include "unoaprms.hxx"
#include "dragserv.hxx"

using namespace ::com::sun::star;

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

// statisches Flag, das anzeigt, ob momentan gedropt
// werden darf
extern BOOL bIsDropAllowed;

/*************************************************************************
|*
|* Drop-Event
|*
\************************************************************************/

BOOL __EXPORT SdView::Drop(const DropEvent& rMEvt, SdWindow* pWin,
                           USHORT nPage, USHORT nLayer)
{
    if (pDropMarker)
    {
        pDropMarker->Hide();
        delete pDropMarker;
        pDropMarker = NULL;
        pDropMarkerObj = NULL;
    }

    SdrPageView*    pPV = GetPageViewPvNum(0);
    String          aActiveLayer = GetActiveLayer();
    BOOL            bReturn = FALSE;

    if (!pPV->IsLayerLocked(aActiveLayer))
    {
        /**********************************************************************
        * Drop nur moeglich, wenn aktiver Layer nicht gesperrt ist
        **********************************************************************/
        const OutlinerView* pOLV = GetTextEditOutlinerView();
        BOOL                bIsInsideOutlinerView = FALSE;

        if (pOLV)
        {
            Rectangle aRect( pOLV->GetOutputArea() );

            if (aMark.GetMarkCount() == 1)
            {
                SdrMark* pMark = aMark.GetMark(0);
                SdrObject* pObj = pMark->GetObj();
                aRect.Union( pObj->GetLogicRect() );
            }

            Point aPos = pOLV->GetWindow()->PixelToLogic( rMEvt.GetPosPixel() );

            if ( aRect.IsInside(aPos) )
            {
                bIsInsideOutlinerView = TRUE;
                bReturn = ( (OutlinerView*) pOLV)->Drop(rMEvt);
            }
        }

        if (!bReturn && !bIsInsideOutlinerView)
        {
            SvDataObjectRef     aDataObj;
            Point               aPos;
            SdDrawViewShell*    pDrViewSh = (SdDrawViewShell*) pDocSh->
                                            GetViewShell();
            SdrPage*            pPage = NULL;

            if (pWin)
                aPos = pWin->PixelToLogic( rMEvt.GetPosPixel() );

            aDataObj = SD_MOD()->pDragData;

            if ( !aDataObj.Is() )
                aDataObj = SvDataObject::PasteDragServer(rMEvt);

            DropAction eAction = rMEvt.GetAction();

            if ( !( bReturn = FmFormView::Drop(rMEvt, pWin) ) )
                bReturn = InsertData(aDataObj, aPos, eAction, TRUE, 0, nPage, nLayer);

            if (!bReturn && pViewSh)
            {
                if( !bReturn && pViewSh )
                {
                    /**************************************************************
                    * URLs droppen
                    **************************************************************/
                    String aTmpString1;
                    String aTmpString2;
                    INetBookmark aINetBookmark(aTmpString1, aTmpString2);
                    const USHORT nCount = DragServer::GetItemCount();

                    SdNavigatorWin* pNavWin = NULL;
                    USHORT nId = SID_NAVIGATOR;

                    if (pViewSh->GetViewFrame()->HasChildWindow(nId))
                        pNavWin = (SdNavigatorWin*) (pViewSh->GetViewFrame()->GetChildWindow(nId)->GetContextWindow( SD_MOD() ));

                    NavigatorDragType eDragType = NAVIGATOR_DRAGTYPE_NONE;

                    if (pNavWin && pNavWin->GetDropDocSh())
                    {
                        eDragType = pNavWin->GetNavigatorDragType();
                    }

                    if (eDragType == NAVIGATOR_DRAGTYPE_LINK ||
                        eDragType == NAVIGATOR_DRAGTYPE_EMBEDDED)
                    {
                        /**********************************************************
                        * Als Objekt oder gelinktes Objekt
                        **********************************************************/
                        List aBookmarkList;
                        String aBookmark;
                        String aFile;

                        for (USHORT i = 0; i < nCount; i++)
                        {
                            // Bookmark-Liste fuellen
                            if (aINetBookmark.PasteDragServer(i))
                            {
                                if (i==0)
                                {
                                    aFile = aINetBookmark.GetURL().GetToken(0, '#');
                                }

                                aBookmark = aINetBookmark.GetURL().GetToken(1, '#');
                                aBookmarkList.Insert(&aBookmark);
                            }
                        }

                        SdPage* pPage = (SdPage*) GetPageViewPvNum(0)->GetPage();
                        USHORT nPgPos = 0xFFFF;

                        if (!pPage->IsMasterPage())
                        {
                            if (pPage->GetPageKind() == PK_STANDARD)
                            {
                                nPgPos = pPage->GetPageNum() + 2;
                            }
                            else if (pPage->GetPageKind() == PK_NOTES)
                            {
                                nPgPos = pPage->GetPageNum() + 1;
                            }
                        }

                        BOOL bLink = eDragType == NAVIGATOR_DRAGTYPE_LINK ? TRUE : FALSE;
                        BOOL bReplace = FALSE;

                        // Um zu gewaehrleisten, dass alle Seitennamen eindeutig sind, werden
                        // die einzufuegenden geprueft und gegebenenfalls in einer Ersatzliste
                        // aufgenommen
                        // bNameOK == FALSE -> Benutzer hat abgebrochen
                        List* pExchangeList = NULL;
                        BOOL bNameOK = GetExchangeList( pExchangeList, &aBookmarkList, 2 );

                        // Da man hier nicht weiss, ob es sich um eine Seite oder ein Objekt
                        // handelt, wird eine Liste sowohl mit Seiten, als auch mit Objekten
                        // gefuellt.
                        // Sollten Seitennamen und Objektnamen identisch sein gibt es hier
                        // natuerlich Probleme !!!

                        if( bNameOK )
                            bReturn = pDoc->InsertBookmark(&aBookmarkList, pExchangeList, bLink,
                                        bReplace, nPgPos, FALSE, pNavWin->GetDropDocSh(),
                                        TRUE, &aPos);

                        // Loeschen der ExchangeList
                        if( pExchangeList )
                        {
                            String* pString = (String*) pExchangeList->First();
                            while( pString )
                            {
                                delete pString;
                                pString = (String*) pExchangeList->Next();
                            }
                            delete pExchangeList;
                        }
                    }
                    else
                    {
                        /**********************************************************
                        * Als URL
                        **********************************************************/
                        SdrObject* pPickObj = NULL;
                        SdrPageView* pPV = NULL;
                        SdWindow* pWindow = pViewSh->GetActiveWindow();
                        USHORT nHitLog = USHORT(pWindow->PixelToLogic(Size(HITPIX,0)).Width());

                        if (nCount == 1 && PickObj(aPos, pPickObj, pPV))
                        {
                            /******************************************************
                            * URL dem getroffenen Objekt zuweisen (presentation::ClickAction)
                            ******************************************************/
                            aINetBookmark.PasteDragServer(0);

                            String aBookmark(aINetBookmark.GetURL());

                            if (aBookmark.Len())
                            {
                                presentation::ClickAction eClickAction = presentation::ClickAction_DOCUMENT;
                                String aDocName(aBookmark.GetToken(0, '#'));

                                if (pDocSh->GetMedium()->GetName() == aDocName ||
                                    pDocSh->GetName() == aDocName)
                                {
                                    // Interner Sprung -> nur "#Bookmark" verwenden
                                    aBookmark = aBookmark.GetToken(1, '#');
                                    eClickAction = presentation::ClickAction_BOOKMARK;
                                }

                                SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pPickObj);

                                BOOL bCreated = FALSE;
                                if (!pInfo)
                                {
                                    pInfo = new SdAnimationInfo(pDoc);
                                    pPickObj->InsertUserData(pInfo);
                                    bCreated = TRUE;
                                }

                                // Undo-Action mit alten und neuen Groessen erzeugen
                                SdAnimationPrmsUndoAction* pAction = new SdAnimationPrmsUndoAction
                                                                (pDoc, pPickObj, bCreated);
                                pAction->SetActive(pInfo->bActive, pInfo->bActive);
                                pAction->SetEffect(pInfo->eEffect, pInfo->eEffect);
                                pAction->SetTextEffect(pInfo->eTextEffect, pInfo->eTextEffect);
                                pAction->SetSpeed(pInfo->eSpeed, pInfo->eSpeed);
                                pAction->SetDim(pInfo->bDimPrevious, pInfo->bDimPrevious);
                                pAction->SetDimColor(pInfo->aDimColor, pInfo->aDimColor);
                                pAction->SetDimHide(pInfo->bDimHide, pInfo->bDimHide);
                                pAction->SetSoundOn(pInfo->bSoundOn, pInfo->bSoundOn);
                                pAction->SetSound(pInfo->aSoundFile, pInfo->aSoundFile);
                                pAction->SetBlueScreen(pInfo->aBlueScreen, pInfo->aBlueScreen);
                                pAction->SetPlayFull(pInfo->bPlayFull, pInfo->bPlayFull);
                                pAction->SetPathObj(pInfo->pPathObj, pInfo->pPathObj);
                                pAction->SetClickAction(pInfo->eClickAction, eClickAction);
                                pAction->SetBookmark(pInfo->aBookmark, aBookmark);
                                pAction->SetInvisibleInPres(pInfo->bInvisibleInPresentation, TRUE);
                                pAction->SetVerb(pInfo->nVerb, pInfo->nVerb);
                                pAction->SetSecondEffect(pInfo->eSecondEffect, pInfo->eSecondEffect);
                                pAction->SetSecondSpeed(pInfo->eSecondSpeed, pInfo->eSecondSpeed);
                                pAction->SetSecondSoundOn(pInfo->bSecondSoundOn, pInfo->bSecondSoundOn);
                                pAction->SetSecondPlayFull(pInfo->bSecondPlayFull, pInfo->bSecondPlayFull);

                                String aString(SdResId(STR_UNDO_ANIMATION));
                                pAction->SetComment(aString);

                                pDocSh->GetUndoManager()->AddUndoAction(pAction);

                                pInfo->eClickAction = eClickAction;
                                pInfo->aBookmark = aBookmark;

                                // Model geaendert
                                pDoc->SetChanged();
                            }
                        }
                        else if (pViewSh->ISA(SdDrawViewShell))
                        {
                            /******************************************************
                            * URLs als Buttons einfuegen
                            ******************************************************/
                            for (USHORT i = 0; i < nCount; i++)
                            {
                                if (aINetBookmark.PasteDragServer(i))
                                {
                                    ((SdDrawViewShell*)pViewSh)->InsertURLButton(
                                        aINetBookmark.GetURL(), aINetBookmark.GetDescription(),
                                        String(), &aPos);
                                    bReturn = TRUE;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* Rueckgabeparameter:
|* pExchangeList == NULL -> Namen sind alle eindeutig
|* bNameOK == FALSE -> Benutzer hat abgebrochen
|* nType == 0 -> Seiten
|* nType == 1 -> Objekte
|* nType == 2 -> Seiten + Objekte
|*
\************************************************************************/

BOOL SdView::GetExchangeList( List*& rpExchangeList, List* pBookmarkList, USHORT nType )
{
    DBG_ASSERT( !rpExchangeList, "ExchangeList muss NULL sein!");

    BOOL bListIdentical = TRUE; // BookmarkList und ExchangeList sind gleich
    BOOL bNameOK = TRUE;        // Name ist eindeutig

    rpExchangeList = new List();

    if( pBookmarkList )
    {
        String* pString = (String*) pBookmarkList->First();
        while( pString && bNameOK )
        {
            String* pNewName = new String( *pString );
            if( nType == 0  || nType == 2 )
                bNameOK = pDocSh->CheckPageName( pViewSh->GetWindow(), *pNewName );
            if( bNameOK && (nType == 1  || nType == 2) )
                bNameOK = pDocSh->CheckObjectName( pViewSh->GetWindow(), *pNewName );

            if( bListIdentical )
                bListIdentical = ( *pString == *pNewName );

            rpExchangeList->Insert( pNewName, LIST_APPEND );
            pString = (String*) pBookmarkList->Next();
        }
    }

    // ExchangeList ist mit BookmarkList identisch
    if( rpExchangeList && bListIdentical )
    {
        String* pString = (String*) rpExchangeList->First();
        while( pString )
        {
            delete pString;
            pString = (String*) rpExchangeList->Next();
        }
        delete rpExchangeList;
        rpExchangeList = NULL;
    }

    return( bNameOK );
}


