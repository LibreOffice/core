/*************************************************************************
 *
 *  $RCSfile: drawview.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:55:34 $
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

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _VIEW3D_HXX //autogen
#include <svx/view3d.hxx>
#endif
#ifndef _SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDORECT_HXX //autogen
#include <svx/svdorect.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_BULITEM_HXX //autogen
#include <svx/bulitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#ifndef _SVX_NUMITEM_HXX //autogen
#include <svx/numitem.hxx>
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#ifndef _SD_STLSHEET_HXX
#include "stlsheet.hxx"
#endif

#include <svx/svdoutl.hxx>

#include "glob.hrc"
#include "strings.hrc"

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "sdattr.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#include "pres.hxx"
#include "sdresid.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "unchss.hxx"
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "anminfo.hxx"
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#ifndef SD_PREVIEW_WINDOW_HXX
#include "PreviewWindow.hxx"
#endif
#ifndef SD_PREVIEW_CHILD_WINDOW_HXX
#include "PreviewChildWindow.hxx"
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

using namespace ::com::sun::star;

namespace sd {

TYPEINIT1(DrawView, View);

/*************************************************************************
|*
|* Konstruktor
|* zeigt die erste Seite des Dokuments auf Position 0,0 an;
|* falls noch keine Seite vorhanden ist, wird eine erzeugt
|*
\************************************************************************/

DrawView::DrawView (
    DrawDocShell* pDocSh,
    OutputDevice* pOutDev,
    DrawViewShell* pShell)
    : ::sd::View(pDocSh->GetDoc(), pOutDev, pShell),
      pDrawViewShell(pShell),
      pDocShell(pDocSh),
      nPOCHSmph(0),
      nPresPaintSmph(0),
      pVDev(NULL),
      bPixelMode(FALSE),
      pSlideShow(NULL),
      nMagic(SDDRAWVIEW_MAGIC),
      bInAnimation(FALSE),
      bActionMode(TRUE)
{
    SetCurrentObj(OBJ_RECT, SdrInventor);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

DrawView::~DrawView()
{
    nMagic = 0;
    delete pVDev;

    if( pSlideShow )
        pSlideShow->Destroy();
}

/*************************************************************************
|*
|* virtuelle Methode von SdrView, wird bei Selektionsaenderung gerufen
|*
\************************************************************************/

void DrawView::MarkListHasChanged()
{
    ::sd::View::MarkListHasChanged();

    if (pDrawViewShell)
        pDrawViewShell->SelectionHasChanged();
}

/*************************************************************************
|*
|* virtuelle Methode von SdrView, wird bei Modelaenderung gerufen
|*
\************************************************************************/

void DrawView::ModelHasChanged()
{
    ::sd::View::ModelHasChanged();

    // den Gestalter zur Neudarstellung zwingen
    SfxStyleSheetBasePool* pSSPool = pDoc->GetStyleSheetPool();
    pSSPool->Broadcast(SfxStyleSheetPoolHint(SFX_STYLESHEETPOOL_CHANGES));

    if (pViewSh)
    {
        // falls wir in der Diashow sind und das Gliederungstextobjekt selektiert
        // ist wird der Gliederungsclone der Diashow aktualisiert
        FuSlideShow* pFuSlideShow = pSlideShow;

        if (!pFuSlideShow)
        {
            pFuSlideShow = pViewSh->GetSlideShow();
        }

        if (pFuSlideShow)
        {
            SdrRectObj* pLayoutText = pFuSlideShow->GetLayoutText();
            if (pLayoutText)
            {
                const SdrMarkList& rMarkList  = GetMarkedObjectList();
                ULONG              nMarkCount = rMarkList.GetMarkCount();
                ULONG              nMark;

                for (nMark = 0; nMark < nMarkCount; nMark++ )
                {
                    SdrObject* pObject = rMarkList.GetMark(nMark)->GetObj();
                    if (pObject == pLayoutText)
                        break;
                }

                if (nMark < nMarkCount)     // gefunden?
                {
                    pFuSlideShow->CreateLayoutClone();
                }
            }
        }

        pDrawViewShell->ModelHasChanged();
    }
}

/*************************************************************************
|*
|* Attribute auf Titel- und Gliederungtext und Hintergrundrechteck einer
|* Masterpage in Vorlagen umlenken, sonst an Basisklasse weitergeben
|*
\************************************************************************/

BOOL DrawView::SetAttributes(const SfxItemSet& rSet,
                                            BOOL bReplaceAll)
{
    BOOL bOk = FALSE;

    // wird eine Masterpage bearbeitet?
    if ( pDrawViewShell && pDrawViewShell->GetEditMode() == EM_MASTERPAGE )
    {
        SfxStyleSheetBasePool* pStShPool = pDoc->GetStyleSheetPool();
        SdPage& rPage = (SdPage&)pDrawViewShell->GetActualPage()->TRG_GetMasterPage();
        String aLayoutName = rPage.GetName();
        SdrTextObj* pObject = (SdrTextObj*)GetTextEditObject();

        if (pObject)
        {
            // Textedit
            String aTemplateName(aLayoutName);

            UINT32 nInv = pObject->GetObjInventor();

            if (nInv == SdrInventor)
            {
                UINT16 eObjKind = pObject->GetObjIdentifier();
                PresObjKind ePresObjKind = rPage.GetPresObjKind(pObject);

                if ( ePresObjKind == PRESOBJ_TITLE ||
                     ePresObjKind == PRESOBJ_NOTES )
                {
                    // Presentation object (except outline)
                    SfxStyleSheet* pSheet = rPage.GetStyleSheetForPresObj( ePresObjKind );
                    DBG_ASSERT(pSheet, "StyleSheet nicht gefunden");

                    SfxItemSet aTempSet( pSheet->GetItemSet() );
                    aTempSet.Put( rSet );
                    aTempSet.ClearInvalidItems();

                    // Undo-Action
                    StyleSheetUndoAction* pAction = new StyleSheetUndoAction(pDoc, pSheet, &aTempSet);
                    pDocSh->GetUndoManager()->AddUndoAction(pAction);

                    pSheet->GetItemSet().Put(aTempSet);
                    pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                    bOk = TRUE;
                }
                else if (eObjKind == OBJ_OUTLINETEXT)
                {
                    // Presentation object outline
                    OutlinerView* pOV   = GetTextEditOutlinerView();
                    ::Outliner* pOutliner = pOV->GetOutliner();
                    List*         pList = (List*)pOV->CreateSelectionList();
                    aTemplateName += String(SdResId(STR_LAYOUT_OUTLINE));

                    pOutliner->SetUpdateMode(FALSE);
                    pDocSh->SetWaitCursor( TRUE );

                    // Platzhalter durch Vorlagennamen ersetzen
                    String aComment(SdResId(STR_UNDO_CHANGE_PRES_OBJECT));
                    xub_StrLen nPos = aComment.Search( (sal_Unicode)'$' );
                    aComment.Erase(nPos, 1);
                    aComment.Insert( String((SdResId(STR_PSEUDOSHEET_OUTLINE))), nPos);
                    pDocSh->GetUndoManager()->EnterListAction( aComment, String() );

                    Paragraph* pPara = (Paragraph*)pList->Last();
                    while (pPara)
                    {
                        ULONG nParaPos = pOutliner->GetAbsPos( pPara );
                        USHORT nDepth = pOutliner->GetDepth( (USHORT) nParaPos );
                        String aName(rPage.GetLayoutName());
                        aName += (sal_Unicode)(' ');
                        aName += String::CreateFromInt32( (sal_Int32)nDepth );
                        SfxStyleSheet* pSheet = (SfxStyleSheet*)pStShPool->Find(aName, SD_LT_FAMILY);
                        DBG_ASSERT(pSheet, "StyleSheet nicht gefunden");

                        SfxItemSet aTempSet( pSheet->GetItemSet() );
                        aTempSet.Put( rSet );
                        aTempSet.ClearInvalidItems();

                        if( nDepth > 1 && aTempSet.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
                        {
                            // Kein SvxNumBulletItem in Gliederungsebenen 2 bis 9!
                            aTempSet.ClearItem( EE_PARA_NUMBULLET );
                        }

                        // Undo-Action
                        StyleSheetUndoAction* pAction = new StyleSheetUndoAction(pDoc, pSheet, &aTempSet);
                        pDocSh->GetUndoManager()->AddUndoAction(pAction);

                        pSheet->GetItemSet().Put(aTempSet);
                        pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));

                        // now also broadcast any child sheets
                        USHORT nChild;
                        for( nChild = nDepth + 1; nChild < 10; nChild++ )
                        {
                            String aName(rPage.GetLayoutName());
                            aName += (sal_Unicode)(' ');
                            aName += String::CreateFromInt32( (sal_Int32)nChild );
                            SfxStyleSheet* pSheet = (SfxStyleSheet*)pStShPool->Find(aName, SD_LT_FAMILY);

                            if( pSheet )
                                pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                        }

                        pPara = (Paragraph*)pList->Prev();

                        if( !pPara && nDepth > 1 &&  rSet.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON &&
                            pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( (Paragraph*) pList->First() ) ) > 1 )
                            pPara = pOutliner->GetParagraph( 0 );  // Put NumBulletItem in outline level 1
                    }

                    pDocSh->SetWaitCursor( FALSE );
                    pOV->GetOutliner()->SetUpdateMode(TRUE);

                    pDocSh->GetUndoManager()->LeaveListAction();

                    delete pList;
                    bOk = TRUE;
                }
                else
                {
                    bOk = ::sd::View::SetAttributes(rSet, bReplaceAll);
                }
            }
        }
        else
        {
            // Selection
            const SdrMarkList& rList = GetMarkedObjectList();
            ULONG nMarkCount         = rList.GetMarkCount();
            SdrObject* pObject = NULL;
            for (ULONG nMark = 0; nMark < nMarkCount; nMark++)
            {
                pObject = rList.GetMark(nMark)->GetObj();
                UINT32 nInv = pObject->GetObjInventor();

                if (nInv == SdrInventor)
                {
                    UINT16 eObjKind = pObject->GetObjIdentifier();
                    PresObjKind ePresObjKind = rPage.GetPresObjKind(pObject);
                    String aTemplateName(aLayoutName);

                    if (ePresObjKind == PRESOBJ_TITLE ||
                        ePresObjKind == PRESOBJ_NOTES ||
                        ePresObjKind == PRESOBJ_BACKGROUND)
                    {
                        // Presentation object (except outline)
                        SfxStyleSheet* pSheet = rPage.GetStyleSheetForPresObj( ePresObjKind );
                        DBG_ASSERT(pSheet, "StyleSheet not found");

                        SfxItemSet aTempSet( pSheet->GetItemSet() );
                        aTempSet.Put( rSet );
                        aTempSet.ClearInvalidItems();

                        // Undo-Action
                        StyleSheetUndoAction* pAction = new StyleSheetUndoAction(pDoc, pSheet, &aTempSet);
                        pDocSh->GetUndoManager()->AddUndoAction(pAction);

                        pSheet->GetItemSet().Put(aTempSet);
                        pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                        bOk = TRUE;
                    }
                    else if (eObjKind == OBJ_OUTLINETEXT)
                    {
                        // Presentation object outline
                        aTemplateName += String(SdResId(STR_LAYOUT_OUTLINE));
                        for (USHORT nLevel = 9; nLevel > 0; nLevel--)
                        {
                            String aName(rPage.GetLayoutName());
                            aName += (sal_Unicode)(' ');
                            aName += String::CreateFromInt32( (sal_Int32)nLevel );
                            SfxStyleSheet* pSheet = (SfxStyleSheet*)pStShPool->
                                                Find(aName, SD_LT_FAMILY);
                            DBG_ASSERT(pSheet, "StyleSheet nicht gefunden");

                            SfxItemSet aTempSet( pSheet->GetItemSet() );

                            if( nLevel > 1 )
                            {
                                // for all levels over 1, clear all items that will be
                                // hard set to level 1
                                SfxWhichIter aWhichIter(rSet);
                                sal_uInt16 nWhich(aWhichIter.FirstWhich());
                                while( nWhich )
                                {
                                    if( SFX_ITEM_ON == rSet.GetItemState( nWhich ) )
                                        aTempSet.ClearItem( nWhich );
                                    nWhich = aWhichIter.NextWhich();
                                }

                            }
                            else
                            {
                                // put the items hard into level one
                                aTempSet.Put( rSet );
                            }

                            aTempSet.ClearInvalidItems();

                            // Undo-Action
                            StyleSheetUndoAction* pAction = new StyleSheetUndoAction(pDoc, pSheet, &aTempSet);
                            pDocSh->GetUndoManager()->AddUndoAction(pAction);

                            pSheet->GetItemSet().Set(aTempSet,false);
                            pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                        }

                        // remove all hard set items from shape that are now set in style
                        SfxWhichIter aWhichIter(rSet);
                        sal_uInt16 nWhich(aWhichIter.FirstWhich());
                        while( nWhich )
                        {
                            if( SFX_ITEM_ON == rSet.GetItemState( nWhich ) )
                                pObject->ClearMergedItem( nWhich );
                            nWhich = aWhichIter.NextWhich();
                        }

                        bOk = TRUE;
                    }
                }
            }

            if(!bOk)
                bOk = ::sd::View::SetAttributes(rSet, bReplaceAll);
        }
    }
    else    // nicht auf der Masterpage
    {
        bOk = ::sd::View::SetAttributes(rSet, bReplaceAll);
    }

    return (bOk);
}

/*************************************************************************
|*
|* Notify fuer Aenderung der Seitenanordnung
|*
\************************************************************************/

void DrawView::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                                 const SfxHint& rHint, const TypeId& rHintType)
{
    if ( pDrawViewShell && rHint.ISA(SdrHint) )
    {
        SdrHintKind eHintKind = ( (SdrHint&) rHint).GetKind();

        if ( nPOCHSmph == 0 && eHintKind == HINT_PAGEORDERCHG )
        {
            pDrawViewShell->ResetActualPage();
        }
        else if ( eHintKind == HINT_LAYERCHG || eHintKind == HINT_LAYERORDERCHG )
        {
            pDrawViewShell->ResetActualLayer();
        }

        // #94278# switch to that page when it's not a master page
        if(HINT_SWITCHTOPAGE == eHintKind)
        {
            const SdrPage* pPage = ((const SdrHint&)rHint).GetPage();

            if(pPage && !pPage->IsMasterPage())
            {
                if(pDrawViewShell->GetActualPage() != pPage)
                {
                    sal_uInt16 nPageNum = (pPage->GetPageNum() - 1) / 2; // Sdr --> Sd
                    pDrawViewShell->SwitchPage(nPageNum);
                }
            }
        }
    }

    ::sd::View::SFX_NOTIFY(rBC, rBCType, rHint, rHintType);
}

/*************************************************************************
|*
|* PageOrderChangedHint blockieren/freigeben
|*
\************************************************************************/

void DrawView::BlockPageOrderChangedHint(BOOL bBlock)
{
    if (bBlock)
        nPOCHSmph++;
    else
    {
        DBG_ASSERT(nPOCHSmph, "Zaehlerunterlauf");
        nPOCHSmph--;
    }
}

/*************************************************************************
|*
|* StyleSheet-Setzen auf der Masterpage abfangen, wenn Praesentationsobjekte
|* selektiert sind
|*
\************************************************************************/

BOOL DrawView::SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    BOOL bResult = TRUE;

    // wird eine Masterpage bearbeitet?
    if (pDrawViewShell && pDrawViewShell->GetEditMode() == EM_MASTERPAGE)
    {
        if (IsPresObjSelected(FALSE, TRUE))
        {

            InfoBox(pDrawViewShell->GetActiveWindow(),
                    String(SdResId(STR_ACTION_NOTPOSSIBLE))).Execute();
            bResult = FALSE;
        }
        else
        {
            bResult = ::sd::View::SetStyleSheet(pStyleSheet, bDontRemoveHardAttr);
        }
    }
    else
    {
        bResult = ::sd::View::SetStyleSheet(pStyleSheet, bDontRemoveHardAttr);
    }
    return bResult;
}

/*************************************************************************
|*
|* Paint-Methode: das Ereignis wird an die View weitergeleitet
|*
\************************************************************************/

void DrawView::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, ::sdr::contact::ViewObjectContactRedirector* pRedirector /*=0L*/)
{

    BOOL bMPCache = FALSE;

// #110094#-7
//  if (pViewSh && pViewSh == (ViewShell*) SfxViewShell::Current() &&
//      pViewSh->GetFrameView()->IsMasterPagePaintCaching() &&
//      pOutDev->GetOutDevType() != OUTDEV_PRINTER)
//  {
//      // Aktive ViewShell
//      bMPCache = TRUE;
//  }
//
//  if( bMPCache )
//  {
//      if( !IsMasterPagePaintCaching() )
//      {
//          SetMasterPagePaintCaching( TRUE );
//      }
//  }
//  else
//  {
//      if( IsMasterPagePaintCaching() )
//      {
//          ReleaseMasterPagePaintCache();
//          SetMasterPagePaintCaching( FALSE );
//      }
//  }

    if (bPixelMode)
    {
        if (!pVDev)
        {
            /******************************************************************
            * VDev erzeugen
            ******************************************************************/
            delete pVDev;
            pVDev = new VirtualDevice(*pOutDev);
            MapMode aMM(pOutDev->GetMapMode().GetMapUnit());
            aMM.SetOrigin(Point(0, 0));
            pVDev->SetMapMode(aMM);
        }

        /**********************************************************************
        * Groesse des VDevs auf Seitengroesse setzen
        **********************************************************************/
        SdPage* pPage = pDrawViewShell->GetActualPage();
        Size aPgSize(pPage->GetSize());
        aPgSize.Width()  -= pPage->GetLftBorder();
        aPgSize.Width()  -= pPage->GetRgtBorder();
        aPgSize.Height() -= pPage->GetUppBorder();
        aPgSize.Height() -= pPage->GetLwrBorder();

        if (aPgSize != pVDev->GetOutputSize())
        {
            BOOL bAbort = !pVDev->SetOutputSize(aPgSize);
            DBG_ASSERT(!bAbort, "VDev nicht korrekt erzeugt");

            if (bAbort)
            {
                delete pVDev;
                pVDev = NULL;
                SetPixelMode(FALSE);
            }
        }
    }
    else if (!bPixelMode && pVDev)
    {
        delete pVDev;
        pVDev = NULL;
    }

    BOOL bStandardPaint = TRUE;

    SdDrawDocument* pDoc = pDocShell->GetDoc();
    if(pDoc && pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS)
    {
        /*******************************************************************
        |* Paint-Event fuer eine Praesentation?
        \******************************************************************/
        FuSlideShow* pFuSlideShow = pSlideShow;

        // Paint-Event fuer eine Praesentation im Vollbildmodus oder Fenster?
        if (!pFuSlideShow && pViewSh)
        {
            pFuSlideShow = pViewSh->GetSlideShow();
        }
        // Paint-Event fuer das Preview-Fenster?
        else
        {
            SfxViewFrame*   pViewFrm = pDrawViewShell ? pDrawViewShell->GetViewFrame() : NULL;
            SfxChildWindow* pPreviewChildWindow = pViewFrm ?
                pViewFrm->GetChildWindow(PreviewChildWindow::GetChildWindowId()) : NULL;
            if (pPreviewChildWindow)
            {
                PreviewWindow* pPreviewWin = static_cast<PreviewWindow*>(
                    pPreviewChildWindow->GetWindow());
                if (pPreviewWin && pPreviewWin->GetDoc() == pDoc)
                    pFuSlideShow = pPreviewWin->GetSlideShow();
            }
        }

        if (pFuSlideShow)
        {
            OutputDevice* pShowWindow = (OutputDevice*)pFuSlideShow->GetShowWindow();
            if (pShowWindow == pOutDev ||
                pFuSlideShow->GetAnimationMode() == ANIMATIONMODE_VIEW)
            {
                PresPaint(rReg);
                bStandardPaint = FALSE;
            }
        }
    }


    if (bStandardPaint)
    {
        if (!bPixelMode)
        {
            ::sd::View::CompleteRedraw(pOutDev, rReg, pRedirector);
        }
        else
        {
            /******************************************************************
            * Pixelmodus
            ******************************************************************/
            // Objekte ins VDev zeichnen
            ::sd::View::CompleteRedraw(pVDev, rReg, pRedirector );

            // VDev auf Window ausgeben
            pOutDev->DrawOutDev(Point(), pVDev->GetOutputSize(),
                                Point(), pVDev->GetOutputSize(), *pVDev);

            if (IsShownXorVisible(pOutDev))
            {
                // Handles auf Window ausgeben
                ToggleShownXor(pOutDev, &rReg);
            }
        }
    }
}


/*************************************************************************
|*
|* PaintEvents waehrend der Praesentation erlauben oder auch nicht.
|*
\************************************************************************/

void  DrawView::AllowPresPaint(BOOL bAllowed)
{
    if (bAllowed)
    {
        DBG_ASSERT(nPresPaintSmph != 0, "Unterlauf im PaintSemaphor");
        nPresPaintSmph--;
    }
    else
    {
        DBG_ASSERT(nPresPaintSmph + 1 != 0, "Ueberlauf im PaintSemaphor");
        nPresPaintSmph++;
    }
}

/*************************************************************************
|*
|* Paint-Proc filtert fuer die Praesentation Objekte heraus, die unsichtbar
|* sind oder anders dargestellt werden muessen.
|*
\************************************************************************/

class DrawViewRedirector : public ::sdr::contact::ViewObjectContactRedirector
{
    DrawView&                           mrDrawView;

public:
    DrawViewRedirector(DrawView& rDrawView);
    virtual ~DrawViewRedirector();

    // all default implementations just call the same methods at the original. To do something
    // different, overload the method and at least do what the method does.
    virtual void PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo);
};

DrawViewRedirector::DrawViewRedirector(DrawView& rDrawView)
:   mrDrawView(rDrawView)
{
}

DrawViewRedirector::~DrawViewRedirector()
{
}

// all default implementations just call the same methods at the original. To do something
// different, overload the method and at least do what the method does.
void DrawViewRedirector::PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject)
    {
        OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();
        SdAnimationInfo*    pInfo = mrDrawView.pDoc->GetAnimationInfo(pObject);
        const USHORT nOldAntialiasing = pOutDev->GetAntialiasing();

        if( !pObject->IsEmptyPresObj() )
        {
            FuSlideShow* pFuSlideShow = mrDrawView.pSlideShow;

            // Paint-Event fuer eine Praesentation im Vollbildmodus oder Fenster?
            if( !pFuSlideShow && mrDrawView.pViewSh )
                pFuSlideShow = mrDrawView.pViewSh->GetSlideShow();
            else
            {
                // Paint-Event fuer das Preview-Fenster?
                SfxViewFrame*   pViewFrm = mrDrawView.pDrawViewShell ? mrDrawView.pDrawViewShell->GetViewFrame() : NULL;
                SfxChildWindow* pPreviewChildWindow = pViewFrm ?
                    pViewFrm->GetChildWindow(
                        PreviewChildWindow::GetChildWindowId()) : NULL;
                if (pPreviewChildWindow)
                {
                    PreviewWindow* pPreviewWin = static_cast<PreviewWindow*>(
                        pPreviewChildWindow->GetWindow());
                    if (pPreviewWin && pPreviewWin->GetDoc() == mrDrawView.pDoc)
                        pFuSlideShow = pPreviewWin->GetSlideShow();
                }
            }

            const BOOL  bPreview = pFuSlideShow->GetAnimationMode() == ANIMATIONMODE_PREVIEW;
            BOOL        bDrawn = FALSE;

            if( bPreview && ( !pInfo || pInfo->bIsShown ) )
                pFuSlideShow->StopTextOrGraphicAnimation( pObject, TRUE );

            if( pInfo && ( pInfo->bInvisibleInPresentation || !pInfo->bIsShown ) )
                bDrawn = TRUE;
            else if( pInfo && pInfo->bIsShown )
            {
                const BOOL      bLive = pFuSlideShow->IsLivePresentation();
                const BOOL      bDimmed = pInfo->bDimmed;

                // in case of a move effect we have to disable antialiasing
                // (we don't want to use alpha masks for performance reasons)
                switch( pInfo->eEffect )
                {
                    case presentation::AnimationEffect_MOVE_FROM_LEFT:
                    case presentation::AnimationEffect_MOVE_FROM_UPPERLEFT:
                    case presentation::AnimationEffect_MOVE_FROM_TOP:
                    case presentation::AnimationEffect_MOVE_FROM_UPPERRIGHT:
                    case presentation::AnimationEffect_MOVE_FROM_RIGHT :
                    case presentation::AnimationEffect_MOVE_FROM_LOWERRIGHT:
                    case presentation::AnimationEffect_MOVE_FROM_BOTTOM:
                    case presentation::AnimationEffect_MOVE_FROM_LOWERLEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_LEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_UPPERLEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_TOP:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_UPPERRIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_RIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_LOWERRIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_BOTTOM:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_LOWERLEFT:
                    case presentation::AnimationEffect_MOVE_TO_LEFT:
                    case presentation::AnimationEffect_MOVE_TO_UPPERLEFT:
                    case presentation::AnimationEffect_MOVE_TO_TOP:
                    case presentation::AnimationEffect_MOVE_TO_UPPERRIGHT:
                    case presentation::AnimationEffect_MOVE_TO_RIGHT :
                    case presentation::AnimationEffect_MOVE_TO_LOWERRIGHT:
                    case presentation::AnimationEffect_MOVE_TO_BOTTOM:
                    case presentation::AnimationEffect_MOVE_TO_LOWERLEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_LEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_UPPERLEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_TOP:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_RIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_BOTTOM:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_LOWERLEFT:
                    case presentation::AnimationEffect_STRETCH_FROM_LEFT:
                    case presentation::AnimationEffect_STRETCH_FROM_UPPERLEFT:
                    case presentation::AnimationEffect_STRETCH_FROM_TOP:
                    case presentation::AnimationEffect_STRETCH_FROM_UPPERRIGHT:
                    case presentation::AnimationEffect_STRETCH_FROM_RIGHT:
                    case presentation::AnimationEffect_STRETCH_FROM_LOWERRIGHT:
                    case presentation::AnimationEffect_STRETCH_FROM_BOTTOM:
                    case presentation::AnimationEffect_STRETCH_FROM_LOWERLEFT:
                    case presentation::AnimationEffect_HORIZONTAL_STRETCH:
                    case presentation::AnimationEffect_VERTICAL_STRETCH:
                    case presentation::AnimationEffect_HORIZONTAL_ROTATE:
                    case presentation::AnimationEffect_VERTICAL_ROTATE:
                    case presentation::AnimationEffect_PATH:
                    case presentation::AnimationEffect_LASER_FROM_LEFT:
                    case presentation::AnimationEffect_LASER_FROM_UPPERLEFT:
                    case presentation::AnimationEffect_LASER_FROM_TOP:
                    case presentation::AnimationEffect_LASER_FROM_UPPERRIGHT:
                    case presentation::AnimationEffect_LASER_FROM_RIGHT:
                    case presentation::AnimationEffect_LASER_FROM_LOWERRIGHT:
                    case presentation::AnimationEffect_LASER_FROM_BOTTOM:
                    case presentation::AnimationEffect_LASER_FROM_LOWERLEFT:
                    {
                        pOutDev->SetAntialiasing( ANTIALIASING_DISABLE_TEXT );
                    }
                }

                switch( pInfo->eTextEffect )
                {
                    case presentation::AnimationEffect_MOVE_FROM_LEFT:
                    case presentation::AnimationEffect_MOVE_FROM_UPPERLEFT:
                    case presentation::AnimationEffect_MOVE_FROM_TOP:
                    case presentation::AnimationEffect_MOVE_FROM_UPPERRIGHT:
                    case presentation::AnimationEffect_MOVE_FROM_RIGHT :
                    case presentation::AnimationEffect_MOVE_FROM_LOWERRIGHT:
                    case presentation::AnimationEffect_MOVE_FROM_BOTTOM:
                    case presentation::AnimationEffect_MOVE_FROM_LOWERLEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_LEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_UPPERLEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_TOP:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_UPPERRIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_RIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_LOWERRIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_BOTTOM:
                    case presentation::AnimationEffect_MOVE_SHORT_FROM_LOWERLEFT:
                    case presentation::AnimationEffect_MOVE_TO_LEFT:
                    case presentation::AnimationEffect_MOVE_TO_UPPERLEFT:
                    case presentation::AnimationEffect_MOVE_TO_TOP:
                    case presentation::AnimationEffect_MOVE_TO_UPPERRIGHT:
                    case presentation::AnimationEffect_MOVE_TO_RIGHT :
                    case presentation::AnimationEffect_MOVE_TO_LOWERRIGHT:
                    case presentation::AnimationEffect_MOVE_TO_BOTTOM:
                    case presentation::AnimationEffect_MOVE_TO_LOWERLEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_LEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_UPPERLEFT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_TOP:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_RIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_BOTTOM:
                    case presentation::AnimationEffect_MOVE_SHORT_TO_LOWERLEFT:
                    case presentation::AnimationEffect_STRETCH_FROM_LEFT:
                    case presentation::AnimationEffect_STRETCH_FROM_UPPERLEFT:
                    case presentation::AnimationEffect_STRETCH_FROM_TOP:
                    case presentation::AnimationEffect_STRETCH_FROM_UPPERRIGHT:
                    case presentation::AnimationEffect_STRETCH_FROM_RIGHT:
                    case presentation::AnimationEffect_STRETCH_FROM_LOWERRIGHT:
                    case presentation::AnimationEffect_STRETCH_FROM_BOTTOM:
                    case presentation::AnimationEffect_STRETCH_FROM_LOWERLEFT:
                    case presentation::AnimationEffect_HORIZONTAL_STRETCH:
                    case presentation::AnimationEffect_VERTICAL_STRETCH:
                    case presentation::AnimationEffect_HORIZONTAL_ROTATE:
                    case presentation::AnimationEffect_VERTICAL_ROTATE:
                    case presentation::AnimationEffect_PATH:
                    case presentation::AnimationEffect_LASER_FROM_LEFT:
                    case presentation::AnimationEffect_LASER_FROM_UPPERLEFT:
                    case presentation::AnimationEffect_LASER_FROM_TOP:
                    case presentation::AnimationEffect_LASER_FROM_UPPERRIGHT:
                    case presentation::AnimationEffect_LASER_FROM_RIGHT:
                    case presentation::AnimationEffect_LASER_FROM_LOWERRIGHT:
                    case presentation::AnimationEffect_LASER_FROM_BOTTOM:
                    case presentation::AnimationEffect_LASER_FROM_LOWERLEFT:
                    {
                        pOutDev->SetAntialiasing( ANTIALIASING_DISABLE_TEXT );
                    }
                }

                if( pObject == pFuSlideShow->GetLayoutText() )
                {
                    pFuSlideShow->PaintLayoutClone(pOutDev);
                    bDrawn = TRUE;
                }
                else if( pInfo->bIsMovie )
                {
                    if( bDimmed )
                    {
                        if( !bLive )
                        {
                            pFuSlideShow->PaintDimmedObject(((SdrAttrObj*) pObject), pOutDev, pInfo->aDimColor, TRUE );
                        }
                        else
                        {
                            rOriginal.PaintObject(rDisplayInfo);
                        }
                    }
                    else if( bLive )
                    {
                        rOriginal.PaintObject(rDisplayInfo);
                    }
                    else
                    {
                        // nur das letzte Bild
                        SdrObjList* pObjList = ((SdrObjGroup*)pObject)->GetSubList();
                        SdrObject* pLast = (SdrObject*)pObjList->GetObj(pObjList->GetObjCount() - 1);
                        pLast->SingleObjectPainter(*rDisplayInfo.GetExtendedOutputDevice(), *rDisplayInfo.GetPaintInfoRec());
                    }

                    bDrawn = TRUE;
                }
                else if( bDimmed )
                {
                    pFuSlideShow->PaintDimmedObject(((SdrAttrObj*)pObject), pOutDev, pInfo->aDimColor, TRUE );
                    bDrawn = TRUE;
                }
            }

            if( !bDrawn )
            {
                const BOOL bGrafAnim = OBJIS_GRAFANIM( pObject );
                const BOOL bTextAnim = OBJIS_TEXTANIM( pObject );

                // falls es sich um ein animiertes Objekt handelt,
                // das nicht animiert dargestellt werden soll, muessen
                // wir ein nicht animiertes Clone-Objekt ausgeben
                if( !pFuSlideShow->IsAnimationAllowed() && ( bGrafAnim || bTextAnim ) )
                {
                    SdrObject* pClone = pObject->Clone();

                    if( bGrafAnim )
                        ( (SdrGrafObj*) pClone )->SetGraphic( ( (SdrGrafObj*) pObject )->GetTransformedGraphic().GetBitmapEx() );

                    if( bTextAnim )
                    {
                        SfxItemSet aTempAttr( mrDrawView.pDoc->GetPool(), SDRATTR_TEXT_ANIKIND, SDRATTR_TEXT_ANIKIND );
                        aTempAttr.InvalidateItem( SDRATTR_TEXT_ANIKIND );
                        aTempAttr.Put( SdrTextAniKindItem() );
                        pClone->SetMergedItemSet(aTempAttr);
                    }

                    pClone->SingleObjectPainter(*rDisplayInfo.GetExtendedOutputDevice(), *rDisplayInfo.GetPaintInfoRec());
                    delete pClone;
                }
                else
                {
                    SdrObject* pObj = pObject;
                    if( pObj->GetPage() && pObj->GetPage()->checkVisibility(rOriginal, rDisplayInfo, false))
                    {
                        rOriginal.PaintObject(rDisplayInfo);
                    }
                }
            }
        }
        // das Hintergrundrechteck gibt sich faelschlicherweise als EmptyPresObj
        // aus, das kann auch nicht geaendert werden, da es sonst von anderen
        // Methoden nicht erkannt wird und neue Hintergrundrechtecke erzeugt
        // werden --> sieht aus wie n-faches Redraw des Hintergrunds
        // (siehe auch FuSlideShow::PaintProc)
        else
        {
            SdPage* pPage = (SdPage*)pObject->GetPage();

            if( ( pPage->GetPresObj(PRESOBJ_BACKGROUND) == pObject ) ||
                ( ANIMATIONMODE_PREVIEW == mrDrawView.eAnimationMode ) )
            {
                rOriginal.PaintObject(rDisplayInfo);
            }
        }

        pOutDev->SetAntialiasing( nOldAntialiasing );
    }
    else
    {
        // not an object, maybe a page
        rOriginal.PaintObject(rDisplayInfo);
    }
}

/*************************************************************************
|*
|* Paint-Event waehrend der Praesenation
|*
\************************************************************************/

void DrawView::PresPaint(const Region& rRegion)
{
    if (nPresPaintSmph == 0)
    {
        FuSlideShow* pFuSlideShow = pSlideShow;

        // Paint-Event fuer eine Praesentation im Vollbildmodus oder Fenster?
        if (!pFuSlideShow && pViewSh)
        {
            pFuSlideShow = pViewSh->GetSlideShow();
        }
        // Paint-Event fuer das Preview-Fenster?
        else
        {
            SfxViewFrame*   pViewFrm = pDrawViewShell ? pDrawViewShell->GetViewFrame() : NULL;
            SfxChildWindow* pPreviewChildWindow = pViewFrm ?
                pViewFrm->GetChildWindow(
                    PreviewChildWindow::GetChildWindowId()) : NULL;
            if (pPreviewChildWindow)
            {
                PreviewWindow* pPreviewWin = static_cast<PreviewWindow*>(
                        pPreviewChildWindow->GetWindow());
                if (pPreviewWin && pPreviewWin->GetDoc() == pDoc )
                    pFuSlideShow = pPreviewWin->GetSlideShow();
            }
        }

        ::sd::Window* pWindow = static_cast< ::sd::Window*>(GetWin(0));

        BOOL bLivePresentation = FALSE;

        if (pFuSlideShow)
        {
            const ULONG nFuslCacheMode = pFuSlideShow->GetMasterPageCacheMode();

            bLivePresentation = pFuSlideShow->IsLivePresentation();

// #110094#-7
//          if( IsMasterPagePaintCaching() && ( nFuslCacheMode != GetMasterPagePaintCacheMode() ) )
//          {
//              if( nFuslCacheMode == SDR_MASTERPAGECACHE_NONE )
//              {
//                  ReleaseMasterPagePaintCache();
//                  SetMasterPagePaintCaching( FALSE );
//              }
//              else
//                  SetMasterPagePaintCaching( TRUE, nFuslCacheMode );
//          }
        }

        if (!bLivePresentation || IsShownXorVisible(pWindow))
        {
            // Selektionsdarstellung ausblenden
            HideShownXor(pWindow);
        }

        if (pFuSlideShow &&
            (pFuSlideShow->GetAnimationMode() == ANIMATIONMODE_SHOW ||
             pFuSlideShow->GetAnimationMode() == ANIMATIONMODE_PREVIEW))
        {
            // Schwarzer Hintergrund
            Point       aPos(pWindow->PixelToLogic(Point(0,0)));
            Size        aSize(pWindow->GetOutputSize());
            Rectangle   aRect(aPos, aSize);
            const Color aBlack(COL_BLACK);
            const Color aOldColor( pWindow->GetFillColor() );
            const ULONG nOldDrawMode( pWindow->GetDrawMode() );

            pWindow->SetDrawMode( DRAWMODE_DEFAULT );
            pWindow->SetFillColor( aBlack );

            pWindow->DrawRect( aRect );

            pWindow->SetFillColor( aOldColor );
            pWindow->SetDrawMode( nOldDrawMode );
        }

        // Clipping auf angezeigten Seitenbereich
        SdrPageView* pPageView = GetPageViewPvNum( 0 );

        if( pPageView )
        {
            // #i18640#
            // We know always display a black page only before the
            // presentation displays anything
            bool bAllowPaint = true;
            if( pFuSlideShow && (pFuSlideShow->GetAnimationMode() == ANIMATIONMODE_SHOW) && (pFuSlideShow->GetActualPage() == NULL) )
                bAllowPaint = false;

            if( bAllowPaint )
            {
                SdrOutliner& rOutl=pDoc->GetDrawOutliner(NULL);
                rOutl.SetBackgroundColor( pPageView->GetPage()->GetBackgroundColor(pPageView) );

                pWindow->Push( PUSH_CLIPREGION );
                pWindow->IntersectClipRegion( pPageView->GetPageRect() );

                OutputDevice* pOut = pPageView->GetView().GetWin(0);
                if(pOut)
                {
                    DrawViewRedirector aDrawViewRedirector(*this);
                    pPageView->CompleteRedraw( pOut, rRegion, 0, &aDrawViewRedirector );
                }

                pWindow->Pop();
            }

            if( ( bLivePresentation && !IsShownXorVisible( pWindow ) ) || pSlideShow )
                ShowShownXor( pWindow );
        }
    }
}

/*************************************************************************
|* entscheidet, ob ein Objekt markiert werden kann (z. B. noch nicht
|* erschienene Animationsobjekte in der Diashow)
\************************************************************************/

BOOL DrawView::IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const
{
    // erstmal die DrawingEngine pruefen lassen (gesperrter oder unsichtbarer
    // Layer usw.)
    BOOL bResult = FmFormView::IsObjMarkable(pObj, pPV);

    // wenn die DrawingEngine ihr OK gibt, pruefen wir noch nach weiteren
    // Kriterien, die sie nicht kennen kann;
    // dies ist nur fuer die Praesentation interessant (pViewSh != NULL);
    // in der Preview kann sowieso niemand selektieren
    if (bResult && pViewSh)
    {
        FuSlideShow* pFuSlideShow = pSlideShow;

        // Paint-Event fuer eine Praesentation im Vollbildmodus oder Fenster?
        if (!pFuSlideShow && pViewSh)
        {
            pFuSlideShow = pViewSh->GetSlideShow();
        }

        if (pFuSlideShow)
        {
            SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pObj);
            if (pInfo)
            {
                if (!pInfo->bIsShown)
                    bResult = FALSE;
            }
            else
            {
                if (pObj->IsEmptyPresObj())
                    bResult = FALSE;
            }
        }
    }
    return bResult;
}

/*************************************************************************
|*
|* Pixelmodus ein- oder ausschalten
|*
\************************************************************************/

void DrawView::SetPixelMode(BOOL bOn)
{
    if (bPixelMode != bOn)
    {
        bPixelMode = bOn;

        // Sollte der Pixelmode mal aktiv werden, sollte die FieldUnit nicht
        // mehr von der SFX_APP sondern mit SD_MOD()->GetOptions()->GetMetric()
        // besorgt werden (SOLL 364 i) !!!

        /**********************************************************************
        * Default-Units
        **********************************************************************/
        MapUnit eMapUnit = MAP_100TH_MM;
        FieldUnit eFieldUnit = FUNIT_100TH_MM;
        SfxMapUnit eSfxMapUnit = SFX_MAPUNIT_100TH_MM;

        if (bPixelMode)
        {
            /******************************************************************
            * Units fuer Pixelmodus
            ******************************************************************/
            eMapUnit = MAP_PIXEL;
            eFieldUnit = FUNIT_CUSTOM;      // Pixel gibt es noch nicht
            eSfxMapUnit = SFX_MAPUNIT_PIXEL;
        }

        /**********************************************************************
        * Units setzen
        **********************************************************************/
        pDoc->SetScaleUnit(eMapUnit);
        pDoc->SetUIUnit(eFieldUnit);
        pDoc->GetItemPool().SetDefaultMetric(eSfxMapUnit);

        MapMode aMapMode = pDocSh->GetFrame()->GetWindow().GetMapMode();
        aMapMode.SetMapUnit(eMapUnit);
        pDocSh->GetFrame()->GetWindow().SetMapMode(aMapMode);

        for (USHORT nWin = 0; nWin < GetWinCount(); nWin++)
        {
            OutputDevice* pOutDev = GetWin(nWin);

            if (pOutDev && pOutDev->GetOutDevType() == OUTDEV_WINDOW)
            {
                MapMode aSrcMapMode = pOutDev->GetMapMode();
//                MapUnit aSrcMapUnit = aSrcMapMode.GetMapUnit();
//
//                Point aOrigin = pOutDev->LogicToLogic(aSrcMapMode.GetOrigin(),
//                                                      aSrcMapUnit, eMapUnit);
//                Fraction aScaleX = pOutDev->LogicToLogic(aSrcMapMode.GetScaleX(),
//                                                         aSrcMapUnit, eMapUnit);
//                Fraction aScaleY = pOutDev->LogicToLogic(aSrcMapMode.GetScaleY(),
//                                                         aSrcMapUnit, eMapUnit);
//                aSrcMapMode.SetOrigin(aOrigin);
//                aSrcMapMode.SetScaleX(aScaleX);
//                aSrcMapMode.SetScaleY(aScaleY);

                aSrcMapMode.SetMapUnit(eMapUnit);
                pOutDev->SetMapMode(aSrcMapMode);
            }
        }

        /**********************************************************************
        * Windows initialisieren und auf Seitengoesse zoomen
        **********************************************************************/
        Size aPageSize = pDrawViewShell->GetActualPage()->GetSize();
        Point aPageOrg = Point(aPageSize.Width(), aPageSize.Height() / 2);
        Size aViewSize = Size(aPageSize.Width() * 3, aPageSize.Height() * 2);
        pViewSh->InitWindows(aPageOrg, aViewSize, Point(-1, -1));
        pViewSh->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE);
    }
}



/*************************************************************************
|*
|* Uebergebenen Bereich sichtbar machen (es wird ggf. gescrollt)
|*
\************************************************************************/

void DrawView::MakeVisible(const Rectangle& rRect, ::Window& rWin)
{
    if (!rRect.IsEmpty())
    {
        pDrawViewShell->MakeVisible(rRect, rWin);
    }
}


/*************************************************************************
|*
|* Animations-Modus starten
|*
\************************************************************************/

void DrawView::SetAnimationMode(BOOL bStart)
{
    if (!pSlideShow || !bStart || pDrawViewShell->GetEditMode() != EM_MASTERPAGE)
    {
        // Verhindern, dass im EM_MASTERPAGE die Show mehrfach gestartet wird

        if( pSlideShow )
        {
            ::sd::Window* pWindow = static_cast< ::sd::Window*>(GetWin(0));
            const MapMode   aMapMode( pWindow->GetMapMode() );

            pSlideShow->Destroy();
            pSlideShow = NULL;

            pWindow->SetMapMode( aMapMode );
        }

        if (bStart)
        {
            // Aktuelle Einstellungen merken
            pViewSh->WriteFrameViewData();

            // SlideShow erzeugen
            SfxAllItemSet aSet(pDoc->GetItemPool());
            {
            SfxBoolItem aBitem(ATTR_PRESENT_ALL, FALSE);
            aSet.Put(aBitem, aBitem.Which());
            }
            SdPage* pPage = (SdPage*) GetPageViewPvNum(0)->GetPage();
            SfxStringItem aSitem(ATTR_PRESENT_DIANAME, pPage->GetName());
            aSet.Put(aSitem, aSitem.Which());
            {
            SfxBoolItem aBitem(ATTR_PRESENT_ENDLESS, FALSE);
            aSet.Put(aBitem, aBitem.Which());
            }
            {
            SfxBoolItem aBitem(ATTR_PRESENT_MANUEL, TRUE);
            aSet.Put(aBitem, aBitem.Which());
            }
            {
            SfxBoolItem aBitem(ATTR_PRESENT_MOUSE, TRUE);
            aSet.Put(aBitem, aBitem.Which());
            }
            {
            SfxBoolItem aBitem(ATTR_PRESENT_PEN, FALSE);
            aSet.Put(aBitem, aBitem.Which());
            }
            {
            SfxBoolItem aBitem(ATTR_PRESENT_NAVIGATOR, FALSE);
            aSet.Put(aBitem, aBitem.Which());
            }
            {
            SfxBoolItem aBitem(ATTR_PRESENT_CHANGE_PAGE, TRUE);
            aSet.Put(aBitem, aBitem.Which());
            }
            {
            SfxBoolItem aBitem(ATTR_PRESENT_ALWAYS_ON_TOP, FALSE);
            aSet.Put(aBitem, aBitem.Which());
            }
            {
            SfxBoolItem aBitem(ATTR_PRESENT_FULLSCREEN, FALSE);
            aSet.Put(aBitem, aBitem.Which());
            }
            {
            SfxBoolItem aBitem(ATTR_PRESENT_ANIMATION_ALLOWED, TRUE);
            aSet.Put(aBitem, aBitem.Which());
            }
            {
            SfxUInt32Item aUInt32Item(ATTR_PRESENT_PAUSE_TIMEOUT, 0);
            aSet.Put(aUInt32Item, aUInt32Item.Which());
            }
            {
            SfxBoolItem aBitem(ATTR_PRESENT_SHOW_PAUSELOGO, FALSE);
            aSet.Put(aBitem, aBitem.Which());
            }

            SfxRequest aReq(SID_PRESENTATION, 0, aSet);
            pSlideShow = new FuSlideShow(NULL, NULL, this, pDoc, aReq);

            // SlideShow starten und aktuellen MapMode setzen
            ::sd::Window* pWindow = static_cast< ::sd::Window*>(GetWin(0));
            MapMode aMapMode = pWindow->GetMapMode();
            pSlideShow->SetAnimationMode(ANIMATIONMODE_VIEW,
                static_cast<ShowWindow*>(pWindow));  // CAST IST FALSCH!
            pSlideShow->StartShow();
            pSlideShow->Resize( pWindow->GetOutputSizePixel() );
            pWindow->SetMapMode(aMapMode);
        }
        else if (pViewSh)
        {
            // Einstellungen restaurieren
            pViewSh->ReadFrameViewData(pViewSh->GetFrameView());
        }

        ::sd::Window* pWindow = static_cast< ::sd::Window*>(GetWin(0));
        pWindow->Invalidate();
        pWindow->Update();
    }
}


/*************************************************************************
|*
|* Objekt animieren
|*
\************************************************************************/

void DrawView::HideAndAnimateObject(SdrObject* pObj)
{
    if (pSlideShow && !bInAnimation)
    {
        bInAnimation = TRUE;
        SetAnimationEnabled(FALSE);

        pSlideShow->HideAndAnimateObject(pObj);

        // im Reschedule des Effekts zerstoert?
        if (nMagic != SDDRAWVIEW_MAGIC)
        {
            return;
        }

        SetAnimationEnabled(TRUE);
        bInAnimation = FALSE;
    }
}


/*************************************************************************
|*
|* Seite animieren
|*
\************************************************************************/

void DrawView::AnimatePage()
{
    if (pSlideShow && !bInAnimation)
    {
        bInAnimation = TRUE;
        SetAnimationEnabled(FALSE);

        pSlideShow->AnimatePage();

        // im Reschedule des Effekts zerstoert?
        if (nMagic != SDDRAWVIEW_MAGIC)
        {
            return;
        }

        SetAnimationEnabled(TRUE);
        bInAnimation = FALSE;
    }
}

/*************************************************************************
|*
|* Seite wird gehided
|*
\************************************************************************/

void DrawView::HidePage(SdrPageView* pPV)
{
    if (pDrawViewShell)
    {
        pDrawViewShell->HidePage(pPV);
    }

    ::sd::View::HidePage(pPV);
}

SdrObject* DrawView::GetMaxToBtmObj(SdrObject* pObj) const
{
    if( pObj )
    {
        SdPage* pPage = (SdPage*)pObj->GetPage();
        if( pPage && pPage->IsMasterPage() )
            return pPage->GetPresObj( PRESOBJ_BACKGROUND ) ;
    }
    return NULL;
}

} // end of namespace sd
