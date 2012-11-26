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
#include "precompiled_sd.hxx"
#include <sfx2/dispatch.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <svx/svdpagv.hxx>
#include <sfx2/request.hxx>
#include <svl/style.hxx>
#include <editeng/outliner.hxx>
#ifndef _VIEW3D_HXX //autogen
#include <svx/view3d.hxx>
#endif
#ifndef _SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
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
#include <svx/svdlegacy.hxx>

#include "undo/undomanager.hxx"

using namespace ::com::sun::star;

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|* zeigt die erste Seite des Dokuments auf Position 0,0 an;
|* falls noch keine Seite vorhanden ist, wird eine erzeugt
|*
\************************************************************************/

DrawView::DrawView( DrawDocShell* pDocSh, OutputDevice* pOutDev, DrawViewShell* pShell)
: ::sd::View(*pDocSh->GetDoc(), pOutDev, pShell)
, mpDocShell(pDocSh)
, mpDrawViewShell(pShell)
, mpVDev(NULL)
, mnPOCHSmph(0)
{
    setSdrObjectCreationInfo(SdrObjectCreationInfo(static_cast< sal_uInt16 >(OBJ_RECT), SdrInventor));
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

DrawView::~DrawView()
{
    // call here when still set due to it's virtual nature
    if(GetSdrPageView())
    {
        HideSdrPage();
    }

    delete mpVDev;
}

/*************************************************************************
|*
|* virtuelle Methode von SdrView, wird bei Selektionsaenderung gerufen
|*
\************************************************************************/

void DrawView::handleSelectionChange()
{
    // call parent
    ::sd::View::handleSelectionChange();

    // local reactions
    if (mpDrawViewShell)
        mpDrawViewShell->SelectionHasChanged();
}

/*************************************************************************
|*
|* virtuelle Methode von SdrView, wird bei Modelaenderung gerufen
|*
\************************************************************************/

void DrawView::LazyReactOnObjectChanges()
{
    ::sd::View::LazyReactOnObjectChanges();

    // den Gestalter zur Neudarstellung zwingen
    SfxStyleSheetBasePool* pSSPool = mpDoc->GetStyleSheetPool();
    pSSPool->Broadcast(SfxStyleSheetPoolHint(SFX_STYLESHEETPOOL_CHANGES));

    if( mpDrawViewShell )
    {
        mpDrawViewShell->LazyReactOnObjectChanges();
    }
}

/*************************************************************************
|*
|* Attribute auf Titel- und Gliederungtext und Hintergrundrechteck einer
|* Masterpage in Vorlagen umlenken, sonst an Basisklasse weitergeben
|*
\************************************************************************/

bool DrawView::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    bool bOk = false;

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

            sal_uInt32 nInv = pEditObject->GetObjInventor();

            if (nInv == SdrInventor)
            {
                sal_uInt16 eObjKind = pEditObject->GetObjIdentifier();
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
                    StyleSheetUndoAction* pAction = new StyleSheetUndoAction(*mpDoc, *pSheet, aTempSet);
                    mpDocSh->GetUndoManager()->AddUndoAction(pAction);

                    pSheet->GetItemSet().Put(aTempSet);
                    pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                    bOk = true;
                }
                else if (eObjKind == OBJ_OUTLINETEXT)
                {
                    // Presentation object outline
                    OutlinerView* pOV   = GetTextEditOutlinerView();
                    ::Outliner* pOutliner = pOV->GetOutliner();
                    List*         pList = (List*)pOV->CreateSelectionList();
                    aTemplateName += String(SdResId(STR_LAYOUT_OUTLINE));

                    pOutliner->SetUpdateMode(false);
                    mpDocSh->SetWaitCursor( true );

                    // Platzhalter durch Vorlagennamen ersetzen
                    String aComment(SdResId(STR_UNDO_CHANGE_PRES_OBJECT));
                    xub_StrLen nPos = aComment.Search( (sal_Unicode)'$' );
                    aComment.Erase(nPos, 1);
                    aComment.Insert( String((SdResId(STR_PSEUDOSHEET_OUTLINE))), nPos);
                    mpDocSh->GetUndoManager()->EnterListAction( aComment, String() );

                    Paragraph* pPara = (Paragraph*)pList->Last();
                    while (pPara)
                    {
                        sal_uLong nParaPos = pOutliner->GetAbsPos( pPara );
                        sal_Int16 nDepth = pOutliner->GetDepth( (sal_uInt16) nParaPos );
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
                        StyleSheetUndoAction* pAction = new StyleSheetUndoAction(*mpDoc, *pSheet, aTempSet);
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
                            pOutliner->GetDepth( (sal_uInt16) pOutliner->GetAbsPos( (Paragraph*) pList->First() ) ) > 0 )
                            pPara = pOutliner->GetParagraph( 0 );  // Put NumBulletItem in outline level 1
                    }

                    mpDocSh->SetWaitCursor( false );
                    pOV->GetOutliner()->SetUpdateMode(true);

                    mpDocSh->GetUndoManager()->LeaveListAction();

                    delete pList;
                    bOk = true;
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
            const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

            for (sal_uInt32 nMark(0); nMark < aSelection.size(); nMark++)
            {
                SdrObject* pObject = aSelection[nMark];
                sal_uInt32 nInv = pObject->GetObjInventor();

                if (nInv == SdrInventor)
                {
                    sal_uInt16 eObjKind = pObject->GetObjIdentifier();
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
                        StyleSheetUndoAction* pAction = new StyleSheetUndoAction(*mpDoc, *pSheet, aTempSet);
                        mpDocSh->GetUndoManager()->AddUndoAction(pAction);

                        pSheet->GetItemSet().Put(aTempSet,false);
                        pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                        bOk = true;
                    }
                    else if (eObjKind == OBJ_OUTLINETEXT)
                    {
                        // Presentation object outline
                        aTemplateName += String(SdResId(STR_LAYOUT_OUTLINE));
                        for (sal_uInt16 nLevel = 9; nLevel > 0; nLevel--)
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
                            StyleSheetUndoAction* pAction = new StyleSheetUndoAction(*mpDoc, *pSheet, aTempSet);
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

                        bOk = true;
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
    const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if ( mpDrawViewShell && pSdrHint )
    {
        SdrHintKind eHintKind = pSdrHint->GetSdrHintKind();

        if ( mnPOCHSmph == 0 && eHintKind == HINT_PAGEORDERCHG )
        {
            mpDrawViewShell->ResetActualPage();
        }
        else if ( eHintKind == HINT_LAYERCHG || eHintKind == HINT_LAYERORDERCHG )
        {
            mpDrawViewShell->ResetActualLayer();
        }

        // #94278# switch to that page when it's not a master page
        if(HINT_SWITCHTOPAGE == eHintKind)
        {
            const SdrPage* pPage = pSdrHint->GetSdrHintPage();

            if(pPage && !pPage->IsMasterPage())
            {
                if(mpDrawViewShell->GetActualPage() != pPage)
                {
                    sal_uInt32 nPageNum = (pPage->GetPageNumber() - 1) / 2; // Sdr --> Sd
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

void DrawView::BlockPageOrderChangedHint(bool bBlock)
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

bool DrawView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    bool bResult = true;

    // wird eine Masterpage bearbeitet?
    if (mpDrawViewShell && mpDrawViewShell->GetEditMode() == EM_MASTERPAGE)
    {
        if (IsPresObjSelected(false, true))
        {
            InfoBox(mpDrawViewShell->GetActiveWindow(), String(SdResId(STR_ACTION_NOTPOSSIBLE))).Execute();
            bResult = false;
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

    bool bStandardPaint = true;

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
                bStandardPaint = false;
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

bool DrawView::IsObjMarkable(const SdrObject& rObj) const
{
    return FmFormView::IsObjMarkable(rObj);
}

/*************************************************************************
|*
|* Uebergebenen Bereich sichtbar machen (es wird ggf. gescrollt)
|*
\************************************************************************/

void DrawView::MakeVisibleAtView(const basegfx::B2DRange& rRange, ::Window& rWin)
{
    if (!rRange.isEmpty())
    {
        mpDrawViewShell->MakeVisibleAtView(rRange, rWin);
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
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    if( pUndoManager )
    {
        String aUndo( SVX_RES(STR_EditDelete) );
        String aSearchString(RTL_CONSTASCII_USTRINGPARAM("%1"));
        aUndo.SearchAndReplace(aSearchString, getSelectionDescription(aSelection));
        pUndoManager->EnterListAction(aUndo, aUndo);
    }

    SdPage* pPage = 0;
    bool bResetLayout = false;

    for (sal_uInt32 nMark(0); nMark < aSelection.size(); nMark++)
    {
        SdrObject* pObj = aSelection[nMark];

        if( !pObj->IsEmptyPresObj() && findConnectionToSdrObject(pObj) )
            {
            pPage = static_cast< SdPage* >( pObj->getSdrPageFromSdrObject() );
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
                    const basegfx::B2DRange aRange(sdr::legacy::GetLogicRange(*pObj));
                    SdrObject* pNewObj = pPage->InsertAutoLayoutShape( 0, ePresObjKind, bVertical, aRange, true );

                    // Move the new PresObj to the position before the
                    // object it will replace.
                    pUndoManager->AddUndoAction(
                        mpDoc->GetSdrUndoFactory().CreateUndoObjectOrdNum(
                            *pNewObj,
                            pNewObj->GetNavigationPosition(),
                            pObj->GetNavigationPosition()));
                    pPage->SetNavigationPosition( pNewObj->GetNavigationPosition(), pObj->GetNavigationPosition() );

                    bResetLayout = true;

                    OSL_TRACE( "DrawView::InsertAutoLayoutShape() - InsertAutoLayoutShape" );
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
