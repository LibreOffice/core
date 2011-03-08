/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
    /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/request.hxx>
#include <svl/style.hxx>
#include <editeng/outliner.hxx>
#include <svx/view3d.hxx>
#include <svx/svxids.hrc>
#include <svx/svdotext.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdorect.hxx>
#include <svl/poolitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/bulitem.hxx>
#include <svl/itempool.hxx>
#include <editeng/numitem.hxx>
#include <svl/whiter.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include "stlsheet.hxx"

#include <svx/svdoutl.hxx>
#undef BMP_OLEOBJ
#include <svx/svdstr.hrc>
#include <svx/dialmgr.hxx>

#include "glob.hrc"
#include "strings.hrc"
#include "View.hxx"
#include "sdattr.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#include "DrawViewShell.hxx"
#include "pres.hxx"
#include "sdresid.hxx"
#include "Window.hxx"
#include "unchss.hxx"
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "anminfo.hxx"
#include "slideshow.hxx"
#include <vcl/virdev.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

#include "undo/undomanager.hxx"

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

DrawView::DrawView( DrawDocShell* pDocSh, OutputDevice* pOutDev, DrawViewShell* pShell)
: ::sd::View(pDocSh->GetDoc(), pOutDev, pShell)
, mpDocShell(pDocSh)
, mpDrawViewShell(pShell)
, mpVDev(NULL)
, mnPOCHSmph(0)
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
    delete mpVDev;
}

/*************************************************************************
|*
|* virtuelle Methode von SdrView, wird bei Selektionsaenderung gerufen
|*
\************************************************************************/

void DrawView::MarkListHasChanged()
{
    ::sd::View::MarkListHasChanged();

    if (mpDrawViewShell)
        mpDrawViewShell->SelectionHasChanged();
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
    SfxStyleSheetBasePool* pSSPool = mpDoc->GetStyleSheetPool();
    pSSPool->Broadcast(SfxStyleSheetPoolHint(SFX_STYLESHEETPOOL_CHANGES));

    if( mpDrawViewShell )
        mpDrawViewShell->ModelHasChanged();

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
    if ( mpDrawViewShell && mpDrawViewShell->GetEditMode() == EM_MASTERPAGE )
    {
        SfxStyleSheetBasePool* pStShPool = mpDoc->GetStyleSheetPool();
        SdPage& rPage = *mpDrawViewShell->getCurrentPage();
        String aLayoutName = rPage.GetName();
        SdrTextObj* pEditObject = static_cast< SdrTextObj* >( GetTextEditObject() );

        if (pEditObject)
        {
            // Textedit
            String aTemplateName(aLayoutName);

            UINT32 nInv = pEditObject->GetObjInventor();

            if (nInv == SdrInventor)
            {
                UINT16 eObjKind = pEditObject->GetObjIdentifier();
                PresObjKind ePresObjKind = rPage.GetPresObjKind(pEditObject);

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
                    StyleSheetUndoAction* pAction = new StyleSheetUndoAction(mpDoc, pSheet, &aTempSet);
                    mpDocSh->GetUndoManager()->AddUndoAction(pAction);

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
                    mpDocSh->SetWaitCursor( TRUE );

                    // Platzhalter durch Vorlagennamen ersetzen
                    String aComment(SdResId(STR_UNDO_CHANGE_PRES_OBJECT));
                    xub_StrLen nPos = aComment.Search( (sal_Unicode)'$' );
                    aComment.Erase(nPos, 1);
                    aComment.Insert( String((SdResId(STR_PSEUDOSHEET_OUTLINE))), nPos);
                    mpDocSh->GetUndoManager()->EnterListAction( aComment, String() );

                    Paragraph* pPara = (Paragraph*)pList->Last();
                    while (pPara)
                    {
                        ULONG nParaPos = pOutliner->GetAbsPos( pPara );
                        sal_Int16 nDepth = pOutliner->GetDepth( (USHORT) nParaPos );
                        String aName(rPage.GetLayoutName());
                        aName += (sal_Unicode)(' ');
                        aName += String::CreateFromInt32( (nDepth <= 0) ? 1 : nDepth + 1 );
                        SfxStyleSheet* pSheet = (SfxStyleSheet*)pStShPool->Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
                        DBG_ASSERT(pSheet, "StyleSheet nicht gefunden");

                        SfxItemSet aTempSet( pSheet->GetItemSet() );
                        aTempSet.Put( rSet );
                        aTempSet.ClearInvalidItems();

                        if( nDepth > 0 && aTempSet.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
                        {
                            // no SvxNumBulletItem in outline level 1 to 8!
                            aTempSet.ClearItem( EE_PARA_NUMBULLET );
                        }

                        // Undo-Action
                        StyleSheetUndoAction* pAction = new StyleSheetUndoAction(mpDoc, pSheet, &aTempSet);
                        mpDocSh->GetUndoManager()->AddUndoAction(pAction);

                        pSheet->GetItemSet().Put(aTempSet);
                        pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));

                        // now also broadcast any child sheets
                        sal_Int16 nChild;
                        for( nChild = nDepth + 1; nChild < 9; nChild++ )
                        {
                            String aSheetName(rPage.GetLayoutName());
                            aSheetName += (sal_Unicode)(' ');
                            aSheetName += String::CreateFromInt32( nChild <= 0 ? 1 : nChild + 1 );
                            SfxStyleSheet* pOutlSheet = static_cast< SfxStyleSheet* >(pStShPool->Find(aSheetName, SD_STYLE_FAMILY_MASTERPAGE));

                            if( pOutlSheet )
                                pOutlSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                        }

                        pPara = (Paragraph*)pList->Prev();

                        if( !pPara && nDepth > 0 &&  rSet.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON &&
                            pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( (Paragraph*) pList->First() ) ) > 0 )
                            pPara = pOutliner->GetParagraph( 0 );  // Put NumBulletItem in outline level 1
                    }

                    mpDocSh->SetWaitCursor( FALSE );
                    pOV->GetOutliner()->SetUpdateMode(TRUE);

                    mpDocSh->GetUndoManager()->LeaveListAction();

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
            for (ULONG nMark = 0; nMark < nMarkCount; nMark++)
            {
                SdrObject* pObject = rList.GetMark(nMark)->GetMarkedSdrObj();
                UINT32 nInv = pObject->GetObjInventor();

                if (nInv == SdrInventor)
                {
                    UINT16 eObjKind = pObject->GetObjIdentifier();
                    PresObjKind ePresObjKind = rPage.GetPresObjKind(pObject);
                    String aTemplateName(aLayoutName);

                    if (ePresObjKind == PRESOBJ_TITLE ||
                        ePresObjKind == PRESOBJ_NOTES)
                    {
                        // Presentation object (except outline)
                        SfxStyleSheet* pSheet = rPage.GetStyleSheetForPresObj( ePresObjKind );
                        DBG_ASSERT(pSheet, "StyleSheet not found");

                        SfxItemSet aTempSet( pSheet->GetItemSet() );
                        aTempSet.Put( rSet );
                        aTempSet.ClearInvalidItems();

                        // Undo-Action
                        StyleSheetUndoAction* pAction = new StyleSheetUndoAction(mpDoc, pSheet, &aTempSet);
                        mpDocSh->GetUndoManager()->AddUndoAction(pAction);

                        pSheet->GetItemSet().Put(aTempSet,false);
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
                                                Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
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
                            StyleSheetUndoAction* pAction = new StyleSheetUndoAction(mpDoc, pSheet, &aTempSet);
                            mpDocSh->GetUndoManager()->AddUndoAction(pAction);

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

void DrawView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if ( mpDrawViewShell && rHint.ISA(SdrHint) )
    {
        SdrHintKind eHintKind = ( (SdrHint&) rHint).GetKind();

        if ( mnPOCHSmph == 0 && eHintKind == HINT_PAGEORDERCHG )
        {
            mpDrawViewShell->ResetActualPage();
        }
        else if ( eHintKind == HINT_LAYERCHG || eHintKind == HINT_LAYERORDERCHG )
        {
            mpDrawViewShell->ResetActualLayer();
        }

        // switch to that page when it's not a master page
        if(HINT_SWITCHTOPAGE == eHintKind)
        {
            const SdrPage* pPage = ((const SdrHint&)rHint).GetPage();

            if(pPage && !pPage->IsMasterPage())
            {
                if(mpDrawViewShell->GetActualPage() != pPage)
                {
                    sal_uInt16 nPageNum = (pPage->GetPageNum() - 1) / 2; // Sdr --> Sd
                    mpDrawViewShell->SwitchPage(nPageNum);
                }
            }
        }
    }

    ::sd::View::Notify(rBC, rHint);
}

/*************************************************************************
|*
|* PageOrderChangedHint blockieren/freigeben
|*
\************************************************************************/

void DrawView::BlockPageOrderChangedHint(BOOL bBlock)
{
    if (bBlock)
        mnPOCHSmph++;
    else
    {
        DBG_ASSERT(mnPOCHSmph, "Zaehlerunterlauf");
        mnPOCHSmph--;
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
    if (mpDrawViewShell && mpDrawViewShell->GetEditMode() == EM_MASTERPAGE)
    {
        if (IsPresObjSelected(FALSE, TRUE))
        {

            InfoBox(mpDrawViewShell->GetActiveWindow(),
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

void DrawView::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector /*=0L*/)
{
    if( mpVDev )
    {
        delete mpVDev;
        mpVDev = NULL;
    }

    BOOL bStandardPaint = TRUE;

    SdDrawDocument* pDoc = mpDocShell->GetDoc();
    if( pDoc && pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS)
    {
        rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( pDoc ) );
        if(xSlideshow.is() && xSlideshow->isRunning())
        {
            OutputDevice* pShowWindow = ( OutputDevice* )xSlideshow->getShowWindow();
            if( (pShowWindow == pOutDev) || (xSlideshow->getAnimationMode() == ANIMATIONMODE_PREVIEW) )
            {
                if( pShowWindow == pOutDev )
                    PresPaint(rReg);
                bStandardPaint = FALSE;
            }
        }
    }

    if(bStandardPaint)
    {
        ::sd::View::CompleteRedraw(pOutDev, rReg, pRedirector);
    }
}

/*************************************************************************
|*
|* Paint-Event during running slide show
|*
\************************************************************************/

void DrawView::PresPaint(const Region& rRegion)
{
    if(mpViewSh)
    {
        rtl::Reference< SlideShow > xSlideshow( SlideShow::GetSlideShow( GetDoc() ) );
        if( xSlideshow.is() && xSlideshow->isRunning() )
            xSlideshow->paint( rRegion.GetBoundRect() );
    }
}

/*************************************************************************
|* entscheidet, ob ein Objekt markiert werden kann (z. B. noch nicht
|* erschienene Animationsobjekte in der Diashow)
\************************************************************************/

BOOL DrawView::IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const
{
    return FmFormView::IsObjMarkable(pObj, pPV);;
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
        mpDrawViewShell->MakeVisible(rRect, rWin);
    }
}
/*************************************************************************
|*
|* Seite wird gehided
|*
\************************************************************************/

void DrawView::HideSdrPage()
{
    if (mpDrawViewShell)
    {
        mpDrawViewShell->HidePage();
    }

    ::sd::View::HideSdrPage();
}

void DrawView::DeleteMarked()
{
    OSL_TRACE( "DrawView::DeleteMarked() - enter" );

    sd::UndoManager* pUndoManager = mpDoc->GetUndoManager();
    DBG_ASSERT( pUndoManager, "sd::DrawView::DeleteMarked(), ui action without undo manager!?" );

    if( pUndoManager )
    {
        String aUndo( SVX_RES(STR_EditDelete) );
        String aSearchString(RTL_CONSTASCII_USTRINGPARAM("%1"));
        aUndo.SearchAndReplace(aSearchString, GetDescriptionOfMarkedObjects());
        pUndoManager->EnterListAction(aUndo, aUndo);
    }

    SdPage* pPage = 0;
    bool bResetLayout = false;

    const ULONG nMarkCount = GetMarkedObjectList().GetMarkCount();
    if( nMarkCount )
    {
        SdrMarkList aList( GetMarkedObjectList() );
        for (ULONG nMark = 0; nMark < nMarkCount; nMark++)
        {
            SdrObject* pObj = aList.GetMark(nMark)->GetMarkedSdrObj();
            if( pObj && !pObj->IsEmptyPresObj() && pObj->GetUserCall() )
            {
                pPage = static_cast< SdPage* >( pObj->GetPage() );
                PresObjKind ePresObjKind;
                if( pPage && ((ePresObjKind = pPage->GetPresObjKind(pObj)) != PRESOBJ_NONE))
                {
                    switch( ePresObjKind )
                    {
                    case PRESOBJ_GRAPHIC:
                    case PRESOBJ_OBJECT:
                    case PRESOBJ_CHART:
                    case PRESOBJ_ORGCHART:
                    case PRESOBJ_TABLE:
                    case PRESOBJ_CALC:
                    case PRESOBJ_IMAGE:
                    case PRESOBJ_MEDIA:
                        ePresObjKind = PRESOBJ_OUTLINE;
                        break;
                    default:
                        break;
                    }
                    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                    bool bVertical = pTextObj && pTextObj->IsVerticalWriting();
                    Rectangle aRect( pObj->GetLogicRect() );
                    SdrObject* pNewObj = pPage->InsertAutoLayoutShape( 0, ePresObjKind, bVertical, aRect, true );

                    pPage->SetObjectOrdNum( pNewObj->GetOrdNum(), pObj->GetOrdNum() );

                    bResetLayout = true;

                    OSL_TRACE( "DrawView::InsertAutoLayoutShape() - InsertAutoLayoutShape" );
                }
            }
        }
    }

    ::sd::View::DeleteMarked();

    if( pPage && bResetLayout )
        pPage->SetAutoLayout( pPage->GetAutoLayout() );

    if( pUndoManager )
        pUndoManager->LeaveListAction();

    OSL_TRACE( "DrawView::InsertAutoLayoutShape() - leave" );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
