/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawview.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:24:19 $
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

#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>

#ifndef _SD_STLSHEET_HXX
#include "stlsheet.hxx"
#endif

#include <svx/svdoutl.hxx>
#include <svx/svdstr.hrc>
#include <svx/dialmgr.hxx>

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
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
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

#include "ShowWindow.hxx"
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

DrawView::DrawView (
    DrawDocShell* pDocSh,
    OutputDevice* pOutDev,
    DrawViewShell* pShell)
    : ::sd::View(pDocSh->GetDoc(), pOutDev, pShell),
      pDrawViewShell(pShell),
      pDocShell(pDocSh),
      nPOCHSmph(0),
      pVDev(NULL),
      mpSlideShow(NULL)
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
    delete pVDev;

    if( mpSlideShow )
        mpSlideShow->dispose();
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

    if( pDrawViewShell )
        pDrawViewShell->ModelHasChanged();

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
                pObject = rList.GetMark(nMark)->GetMarkedSdrObj();
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

    if( pVDev )
    {
        delete pVDev;
        pVDev = NULL;
    }

    BOOL bStandardPaint = TRUE;

    SdDrawDocument* pDoc = pDocShell->GetDoc();
    if(pDoc && pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS)
    {
        sd::Slideshow* pSlideShow = mpSlideShow;

        // Paint-Event fuer eine Praesentation im Vollbildmodus oder Fenster?
        if(!pSlideShow && pViewSh)
            pSlideShow = pViewSh->GetSlideShow();

        if(pSlideShow)
        {
            OutputDevice* pShowWindow = ( OutputDevice* )pSlideShow->getShowWindow();
            if (pShowWindow == pOutDev || pSlideShow->getAnimationMode() == ANIMATIONMODE_PREVIEW)
            {
                if( pShowWindow == pOutDev )
                    PresPaint(rReg);
                bStandardPaint = FALSE;
            }
        }
    }

    if (bStandardPaint)
    {
        ::sd::View::CompleteRedraw(pOutDev, rReg, pRedirector);
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
    // not an object, maybe a page
    rOriginal.PaintObject(rDisplayInfo);
}

/*************************************************************************
|*
|* Paint-Event during running slide show
|*
\************************************************************************/

void DrawView::PresPaint(const Region& rRegion)
{
    Slideshow* pSlideShow = mpSlideShow;

    if(!pSlideShow && pViewSh)
        pSlideShow = pViewSh->GetSlideShow();

    if( pSlideShow )
        pSlideShow->paint( rRegion.GetBoundRect() );
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
        pDrawViewShell->MakeVisible(rRect, rWin);
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

void DrawView::DeleteMarked()
{
    sd::UndoManager* pUndoManager = pDoc->GetUndoManager();
    DBG_ASSERT( pUndoManager, "sd::DrawView::DeleteMarked(), ui action without undo manager!?" );

    if( pUndoManager )
    {
        String aUndo( SVX_RES(STR_EditDelete) );
        String aSearchString(RTL_CONSTASCII_USTRINGPARAM("%O"));
        aUndo.SearchAndReplace(aSearchString, GetDescriptionOfMarkedObjects());
        pUndoManager->EnterListAction(aUndo, aUndo);
    }

    SdPage* pPage = 0;

    const SdrMarkList& rList = GetMarkedObjectList();
    ULONG nMarkCount         = rList.GetMarkCount();
    for (ULONG nMark = 0; nMark < nMarkCount; nMark++)
    {
        SdrObject* pObj = rList.GetMark(nMark)->GetMarkedSdrObj();
        if( pObj && !pObj->IsEmptyPresObj() && pObj->GetUserCall() )
        {
            pPage = dynamic_cast< SdPage* >( pObj->GetPage() );
            PresObjKind ePresObjKind;
            if( pPage && ((ePresObjKind = pPage->GetPresObjKind(pObj)) != PRESOBJ_NONE))
            {
                SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                bool bVertical = pTextObj && pTextObj->IsVerticalWriting();
                Rectangle aRect( pObj->GetLogicRect() );
                pPage->InsertAutoLayoutShape( 0, ePresObjKind, bVertical, aRect, true );
            }
        }
    }

    ::sd::View::DeleteMarked();

    if( pPage )
        pPage->SetAutoLayout( pPage->GetAutoLayout() );

    if( pUndoManager )
        pUndoManager->LeaveListAction();
}

} // end of namespace sd
