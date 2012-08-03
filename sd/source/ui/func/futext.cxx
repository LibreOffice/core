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

#include "futext.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/editerr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/svxerr.hxx>
#include <tools/urlobj.hxx>
#include <vcl/help.hxx>
#include <editeng/editstat.hxx>
#include <svl/aeitem.hxx>
#include <svl/intitem.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdogrp.hxx>
#include <editeng/flditem.hxx>
#include <svl/style.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svxids.hrc>
#include <sfx2/docfile.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/outlobj.hxx>
#include <svtools/langtab.hxx>

#include <editeng/frmdiritem.hxx>

#include <svx/svdetc.hxx>
#include <editeng/editview.hxx>

#include "sdresid.hxx"
#include "app.hrc"
#include "res_bmp.hrc"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "View.hxx"
#include "Outliner.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdmod.hxx"
#include "FrameView.hxx"
#include "ToolBarManager.hxx"
#include "DrawDocShell.hxx"
#include "glob.hrc"
#include "pres.hxx"
#include "optsitem.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

namespace sd {

static sal_uInt16 SidArray[] = {
    SID_STYLE_FAMILY2,                //    5542
    SID_STYLE_FAMILY5,                //    5545
    SID_REDO,                         //    5700
    SID_UNDO,                         //    5701
    SID_CUT,                          //    5710
    SID_COPY,                         //    5711
    SID_ATTR_TABSTOP,                 //   10002
    SID_ATTR_CHAR_FONT,               //   10007
    SID_ATTR_CHAR_POSTURE,            //   10008
    SID_ATTR_CHAR_WEIGHT,             //   10009
    SID_ATTR_CHAR_UNDERLINE,          //   10014
    SID_ATTR_CHAR_FONTHEIGHT,         //   10015
    SID_ATTR_CHAR_COLOR,              //   10017
    SID_ATTR_PARA_ADJUST_LEFT,        //   10028
    SID_ATTR_PARA_ADJUST_RIGHT,       //   10029
    SID_ATTR_PARA_ADJUST_CENTER,      //   10030
    SID_ATTR_PARA_ADJUST_BLOCK,       //   10031
    SID_ATTR_PARA_LINESPACE_10,       //   10034
    SID_ATTR_PARA_LINESPACE_15,       //   10035
    SID_ATTR_PARA_LINESPACE_20,       //   10036
    SID_ATTR_PARA_LRSPACE,            //   10043
    SID_OUTLINE_UP,                   //   10150
    SID_OUTLINE_DOWN,                 //   10151
    SID_OUTLINE_LEFT,                 //   10152
    SID_OUTLINE_RIGHT,                //   10153
    SID_FORMTEXT_STYLE,               //   10257
    SID_SET_SUPER_SCRIPT,             //   10294
    SID_SET_SUB_SCRIPT,               //   10295
    SID_HYPERLINK_GETLINK,            //   10361
    SID_CHARMAP,                      //   10503
    SID_TEXTDIRECTION_LEFT_TO_RIGHT,  //   10907
    SID_TEXTDIRECTION_TOP_TO_BOTTOM,  //   10908
    SID_ATTR_PARA_LEFT_TO_RIGHT,      //   10950
    SID_ATTR_PARA_RIGHT_TO_LEFT,      //   10951
    FN_NUM_BULLET_ON,                 //   20138
    SID_PARASPACE_INCREASE,           //   27346
    SID_PARASPACE_DECREASE,           //   27347
                            0 };

TYPEINIT1( FuText, FuConstruct );


static sal_Bool bTestText = 0;

/*************************************************************************
|*
|* Basisklasse fuer Textfunktionen
|*
\************************************************************************/

FuText::FuText( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
: FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
, bFirstObjCreated(sal_False)
, bJustEndedEdit(false)
, rRequest (rReq)
{
}

FunctionReference FuText::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuText( pViewSh, pWin, pView, pDoc, rReq ) );
    return xFunc;
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

void FuText::disposing()
{
    if(mpView)
    {
        if(mpView->SdrEndTextEdit(sal_False) == SDRENDTEXTEDIT_DELETED)
            mxTextObj.reset( 0 );

        // die RequestHandler der benutzten Outliner zuruecksetzen auf den
        // Handler am Dokument
        ::Outliner* pOutliner = mpView->GetTextEditOutliner();

        if (pOutliner)
            pOutliner->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(mpDoc->GetStyleSheetPool()));
    }
}

/*************************************************************************
|*
|* Execute functionality of this class:
|*
|* #71422: Start the functionality of this class in this method
|* and not in the ctor.
|* If you construct an object of this class and you put the
|* address of this object to pFuActual you've got a problem,
|* because some methods inside DoExecute use the pFuActual-Pointer.
|* If the code inside DoExecute is executed inside the ctor,
|* the value of pFuActual is not right. And the value will not
|* be right until the ctor finished !!!
|*
\************************************************************************/
void FuText::DoExecute( SfxRequest& )
{
    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBarShell(
        ToolBarManager::TBG_FUNCTION,
        RID_DRAW_TEXT_TOOLBOX);

    mpView->SetCurrentObj(OBJ_TEXT);
    mpView->SetEditMode(SDREDITMODE_EDIT);

    MouseEvent aMEvt(mpWindow->GetPointerPosPixel());

    if (nSlotId == SID_TEXTEDIT)
    {
        // Try to select an object
        SdrPageView* pPV = mpView->GetSdrPageView();
        SdrViewEvent aVEvt;
        mpView->PickAnything(aMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
        mpView->MarkObj(aVEvt.pRootObj, pPV);

        mxTextObj.reset( dynamic_cast< SdrTextObj* >( aVEvt.pObj ) );
    }
    else if (mpView->AreObjectsMarked())
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            mxTextObj.reset( dynamic_cast< SdrTextObj* >( pObj ) );
        }
    }

    // check for table
    if (mpView->AreObjectsMarked())
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if( pObj && (pObj->GetObjInventor() == SdrInventor ) && (pObj->GetObjIdentifier() == OBJ_TABLE) )
            {
                mpViewShell->GetViewShellBase().GetToolBarManager()->AddToolBarShell( ToolBarManager::TBG_FUNCTION, RID_DRAW_TABLE_TOOLBOX );
            }
        }
    }

    sal_Bool bQuickDrag = sal_True;

    const SfxItemSet* pArgs = rRequest.GetArgs();

    if (pArgs

        // test for type before using
        && SID_TEXTEDIT == nSlotId
        && SFX_ITEM_SET == pArgs->GetItemState(SID_TEXTEDIT)

        && (sal_uInt16)((SfxUInt16Item&)pArgs->Get(SID_TEXTEDIT)).GetValue() == 2)
    {
        // Selection by doubleclick -> don't allow QuickDrag
        bQuickDrag = sal_False;
    }

    SetInEditMode(aMEvt, bQuickDrag);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool FuText::MouseButtonDown(const MouseEvent& rMEvt)
{
    bMBDown = sal_True;

    sal_Bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    mpView->SetMarkHdlWhenTextEdit(sal_True);
    SdrViewEvent aVEvt;
    SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

    if (eHit == SDRHIT_TEXTEDIT)
    {
        // Text getroffen -> Event von SdrView auswerten lassen
        if (mpView->MouseButtonDown(rMEvt, mpWindow))
            return (sal_True);
    }

    if (rMEvt.GetClicks() == 1)
    {
        if (mpView->IsTextEdit() && eHit != SDRHIT_MARKEDOBJECT && eHit != SDRHIT_HANDLE)
        {
            // Texteingabe beenden
            if(mpView->SdrEndTextEdit() == SDRENDTEXTEDIT_DELETED)
            {
                // Bugfix von MBA: bei Doppelclick auf der Wiese im Modus Text wird
                // beim zweiten Click eHit = SDRHIT_TEXTEDITOBJ erhalten, weil ja der
                // zweite Click auf das im ersten Click angelegte TextObject geht.
                // Dieses wird aber in SdrEndTextEdit entfernt, weil es leer ist. Es
                // befindet sich aber noch in der Mark-Liste und der Aufruf MarkObj
                // weiter unten greift dann auf das tote Object zu.
                // Als einfacher Fix wird nach SdrEndTextEdit noch einmal eHit ermittelt,
                // was dann SDRHIT_NONE liefert.
                mxTextObj.reset( NULL );
                eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
            }

            mpView->SetCurrentObj(OBJ_TEXT);
            mpView->SetEditMode(SDREDITMODE_EDIT);
        }

        if (rMEvt.IsLeft() || rMEvt.IsRight())
        {
            mpWindow->CaptureMouse();
            SdrObject* pObj;
            SdrPageView* pPV = mpView->GetSdrPageView();

            if (eHit == SDRHIT_TEXTEDIT)
            {
                SetInEditMode(rMEvt, sal_False);
            }
            else
            {
                sal_Bool bMacro = sal_False;

                if (bMacro && mpView->PickObj(aMDPos,mpView->getHitTolLog(),pObj,pPV,SDRSEARCH_PICKMACRO))
                {
                    // Makro
                    sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
                    mpView->BegMacroObj(aMDPos,nHitLog,pObj,pPV,mpWindow);
                }
                else
                {
                    if (eHit != SDRHIT_HANDLE)
                    {
                        // Selektion aufheben
                        if (!rMEvt.IsShift() && eHit == SDRHIT_TEXTEDITOBJ)
                        {
                            mpView->UnmarkAll();
                            mpView->SetDragMode(SDRDRAG_MOVE);
                        }
                    }

                    if ( aVEvt.eEvent == SDREVENT_EXECUTEURL                   ||
                         eHit == SDRHIT_HANDLE                                 ||
                         eHit == SDRHIT_MARKEDOBJECT                           ||
                         eHit == SDRHIT_TEXTEDITOBJ                            ||
                         ( eHit == SDRHIT_UNMARKEDOBJECT && bFirstObjCreated &&
                           !bPermanent ) )
                    {
                        /**********************************************************
                        * Handle, markiertes oder unmarkiertes Objekt getroffen
                        **********************************************************/
                        if (eHit == SDRHIT_TEXTEDITOBJ)
                        {
                            /******************************************************
                            * Text eines unmarkierten Objekts getroffen:
                            * Objekt wird selektiert und in EditMode versetzt
                            ******************************************************/
                            mpView->MarkObj(aVEvt.pRootObj, pPV);

                            if (aVEvt.pObj && aVEvt.pObj->ISA(SdrTextObj))
                            {
                                mxTextObj.reset( static_cast<SdrTextObj*>(aVEvt.pObj) );
                            }

                            SetInEditMode(rMEvt, sal_True);
                        }
                        else if (aVEvt.eEvent == SDREVENT_EXECUTEURL && !rMEvt.IsMod2())
                        {
                            /******************************************************
                            * URL ausfuehren
                            ******************************************************/
                            mpWindow->ReleaseMouse();
                            SfxStringItem aStrItem(SID_FILE_NAME, aVEvt.pURLField->GetURL());
                            SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                            SfxBoolItem aBrowseItem( SID_BROWSE, sal_True );
                            SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                            mpWindow->ReleaseMouse();

                            if (rMEvt.IsMod1())
                            {
                                // Im neuen Frame oeffnen
                                pFrame->GetDispatcher()->Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                            &aStrItem, &aBrowseItem, &aReferer, 0L);
                            }
                            else
                            {
                                // Im aktuellen Frame oeffnen
                                SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                                pFrame->GetDispatcher()->Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                            &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);
                            }
                        }
                        else
                        {
                            /******************************************************
                            * Objekt oder Handle draggen
                            ******************************************************/

                            // #i78748#
                            // do the EndTextEdit first, it will delete the handles and force a
                            // recreation. This will make aVEvt.pHdl to point to a deleted handle,
                            // thus it is necessary to reset it and to get it again.

                            // #i112855#
                            // cl: I'm not sure why we checked here also for mxTextObj->GetOutlinerParaObjet
                            // this caused SdrEndTextEdit() to be called also when not in text editing and
                            // this does not make sense and caused troubles. (see issue 112855)

                            if( mpView->IsTextEdit() )
                            {
                                mpView->SdrEndTextEdit();
                                bJustEndedEdit = true;

                                if(aVEvt.pHdl)
                                {
                                    // force new handle identification, the pointer will be dead here
                                    // since SdrEndTextEdit has resetted (deleted) the handles.
                                    aVEvt.pHdl = 0;
                                    mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
                                }
                            }

                            if (!aVEvt.pHdl)
                            {
                                if( eHit == SDRHIT_UNMARKEDOBJECT )
                                {
                                    if ( !rMEvt.IsShift() )
                                        mpView->UnmarkAll();

                                    mpView->MarkObj(aVEvt.pRootObj, pPV);
                                }

                                // Drag object
                                bFirstMouseMove = sal_True;
                                aDragTimer.Start();
                            }


                            if ( ! rMEvt.IsRight())
                            {
                                // we need to pick again since SdrEndTextEdit can rebuild the handles list
                                eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
                                if( (eHit == SDRHIT_HANDLE) || (eHit == SDRHIT_MARKEDOBJECT) )
                                {
                                    sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
                                    mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
                                }
                            }
                            bReturn = true;
                        }
                    }
                    else if ( nSlotId != SID_TEXTEDIT &&
                              (bPermanent || !bFirstObjCreated) )
                    {
                        /**********************************************************
                        * Objekt erzeugen
                        **********************************************************/
                        mpView->SetCurrentObj(OBJ_TEXT);
                        mpView->SetEditMode(SDREDITMODE_CREATE);
                        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
                        mpView->BegCreateObj(aMDPos, (OutputDevice*) NULL, nDrgLog);
                    }
                    else
                    {
                        /**********************************************************
                        * Selektieren
                        **********************************************************/
                        if( !rMEvt.IsShift() )
                            mpView->UnmarkAll();

                        mpView->BegMarkObj( aMDPos );
                    }
                }
            }
        }
    }
    else if ( rMEvt.GetClicks() == 2 && !mpView->IsTextEdit() )
    {
        MouseEvent aMEvt( mpWindow->GetPointerPosPixel() );
        SetInEditMode( aMEvt, sal_False );
    }

    if (!bIsInDragMode)
    {
        ForcePointer(&rMEvt);
        mpViewShell->GetViewFrame()->GetBindings().Invalidate(SidArray);
    }

    return (bReturn);
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuText::MouseMove(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        if( bFirstMouseMove )
            bFirstMouseMove = sal_False;
        else
            aDragTimer.Stop();
    }

    if (!bReturn && mpView->IsAction() && !mpDocSh->IsReadOnly())
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(mpWindow->PixelToLogic(aPix));

        ForceScroll(aPix);
        mpView->MovAction(aPnt);
    }

    ForcePointer(&rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

void FuText::ImpSetAttributesForNewTextObject(SdrTextObj* pTxtObj)
{
    if(mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS)
    {
        if( nSlotId == SID_ATTR_CHAR )
        {
            // Impress-Textobjekt wird erzeugt (faellt auf Zeilenhoehe zusammen)
            // Damit das Objekt beim anschliessenden Erzeugen gleich die richtige
            // Hoehe bekommt (sonst wird zuviel gepainted)
            SfxItemSet aSet(mpViewShell->GetPool());
            aSet.Put(SdrTextMinFrameHeightItem(0));
            aSet.Put(SdrTextAutoGrowWidthItem(sal_False));
            aSet.Put(SdrTextAutoGrowHeightItem(sal_True));
            pTxtObj->SetMergedItemSet(aSet);
            pTxtObj->AdjustTextFrameWidthAndHeight();
            aSet.Put(SdrTextMaxFrameHeightItem(pTxtObj->GetLogicRect().GetSize().Height()));
            pTxtObj->SetMergedItemSet(aSet);
        }
        else if( nSlotId == SID_ATTR_CHAR_VERTICAL )
        {
            SfxItemSet aSet(mpViewShell->GetPool());
            aSet.Put(SdrTextMinFrameWidthItem(0));
            aSet.Put(SdrTextAutoGrowWidthItem(sal_True));
            aSet.Put(SdrTextAutoGrowHeightItem(sal_False));

            // Needs to be set since default is SDRTEXTHORZADJUST_BLOCK
            aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
            pTxtObj->SetMergedItemSet(aSet);
            pTxtObj->AdjustTextFrameWidthAndHeight();
            aSet.Put(SdrTextMaxFrameWidthItem(pTxtObj->GetLogicRect().GetSize().Width()));
            pTxtObj->SetMergedItemSet(aSet);
        }
    }
    else
    {
        if( nSlotId == SID_ATTR_CHAR_VERTICAL )
        {
            // draw text object, needs to be initialized when vertical text is used
            SfxItemSet aSet(mpViewShell->GetPool());

            aSet.Put(SdrTextAutoGrowWidthItem(sal_True));
            aSet.Put(SdrTextAutoGrowHeightItem(sal_False));

            // Set defaults for vertical klick-n'drag text object, pool defaults are:
            // SdrTextVertAdjustItem: SDRTEXTVERTADJUST_TOP
            // SdrTextHorzAdjustItem: SDRTEXTHORZADJUST_BLOCK
            // Analog to that:
            aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BLOCK));
            aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

            pTxtObj->SetMergedItemSet(aSet);
        }
    }
}

void FuText::ImpSetAttributesFitToSize(SdrTextObj* pTxtObj)
{
    // FitToSize (An Rahmen anpassen)
    SfxItemSet aSet(mpViewShell->GetPool(), SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWWIDTH);
    SdrFitToSizeType eFTS = SDRTEXTFIT_PROPORTIONAL;
    aSet.Put(SdrTextFitToSizeTypeItem(eFTS));
    aSet.Put(SdrTextAutoGrowHeightItem(sal_False));
    aSet.Put(SdrTextAutoGrowWidthItem(sal_False));
    pTxtObj->SetMergedItemSet(aSet);
    pTxtObj->AdjustTextFrameWidthAndHeight();
}

void FuText::ImpSetAttributesFitToSizeVertical(SdrTextObj* pTxtObj)
{
    SfxItemSet aSet(mpViewShell->GetPool(),
        SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWWIDTH);
    SdrFitToSizeType eFTS = SDRTEXTFIT_PROPORTIONAL;
    aSet.Put(SdrTextFitToSizeTypeItem(eFTS));
    aSet.Put(SdrTextAutoGrowHeightItem(sal_False));
    aSet.Put(SdrTextAutoGrowWidthItem(sal_False));
    pTxtObj->SetMergedItemSet(aSet);
    pTxtObj->AdjustTextFrameWidthAndHeight();
}

void FuText::ImpSetAttributesFitCommon(SdrTextObj* pTxtObj)
{
    // Normal Textobject
    if (mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS)
    {
        if( nSlotId == SID_ATTR_CHAR )
        {
            // Impress-Textobject (faellt auf Zeilenhoehe zusammen)
            SfxItemSet aSet(mpViewShell->GetPool());
            aSet.Put(SdrTextMinFrameHeightItem(0));
            aSet.Put(SdrTextMaxFrameHeightItem(0));
            aSet.Put(SdrTextAutoGrowHeightItem(sal_True));
            aSet.Put(SdrTextAutoGrowWidthItem(sal_False));
            pTxtObj->SetMergedItemSet(aSet);
        }
        else if( nSlotId == SID_ATTR_CHAR_VERTICAL )
        {
            SfxItemSet aSet(mpViewShell->GetPool());
            aSet.Put(SdrTextMinFrameWidthItem(0));
            aSet.Put(SdrTextMaxFrameWidthItem(0));
            aSet.Put(SdrTextAutoGrowWidthItem(sal_True));
            aSet.Put(SdrTextAutoGrowHeightItem(sal_False));
            pTxtObj->SetMergedItemSet(aSet);
        }

        pTxtObj->AdjustTextFrameWidthAndHeight();
    }
}

sal_Bool FuText::MouseButtonUp(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_False;

    if (aDragTimer.IsActive())
    {
        aDragTimer.Stop();
        bIsInDragMode = sal_False;
    }

    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    sal_Bool bOldReadOnly = sal_False;
    if ( mpView && mpDocSh->IsReadOnly() )
    {
        if ( mpView &&  mpView->GetTextEditOutlinerView() )
        {
            bOldReadOnly = mpView->GetTextEditOutlinerView()->IsReadOnly();
            mpView->GetTextEditOutlinerView()->SetReadOnly(sal_True);
        }
    }

    if( (mpView && mpView->MouseButtonUp(rMEvt, mpWindow)) || rMEvt.GetClicks() == 2 )
    {
        if ( mpView &&  mpView->GetTextEditOutlinerView() )
            mpView->GetTextEditOutlinerView()->SetReadOnly(bOldReadOnly);
        return (sal_True); // Event von der SdrView ausgewertet
    }
    if ( mpView &&  mpView->GetTextEditOutlinerView() )
        mpView->GetTextEditOutlinerView()->SetReadOnly(bOldReadOnly);

    sal_Bool bEmptyTextObj = sal_False;

    if (mxTextObj.is())
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1
            && ( rMarkList.GetMark(0)->GetMarkedSdrObj() == mxTextObj.get()) )
        {
            if( mxTextObj.is() && !GetTextObj()->GetOutlinerParaObject() )
                bEmptyTextObj = sal_True;
            else
                bFirstObjCreated = sal_True;
        }
        else
        {
            mxTextObj.reset( 0 );
        }
    }

    if( mpView && mpView->IsDragObj())
    {
        /**********************************************************************
        * Objekt wurde verschoben
        **********************************************************************/
        FrameView* pFrameView = mpViewShell->GetFrameView();
        sal_Bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        if (bDragWithCopy)
        {
            bDragWithCopy = !mpView->IsPresObjSelected(sal_False, sal_True);
        }

        mpView->SetDragWithCopy(bDragWithCopy);
        mpView->EndDragObj( mpView->IsDragWithCopy() );
        mpView->ForceMarkedToAnotherPage();
        mpView->SetCurrentObj(OBJ_TEXT);

        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        if ( mpView->IsRotateAllowed() && mpViewShell->GetFrameView()->IsClickChangeRotation() && (rMEvt.GetClicks() != 2) && !bJustEndedEdit &&
            !rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() && !rMEvt.IsRight() &&
            Abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
            Abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
        {
            // toggle to rotation mode
            mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_OBJECT_ROTATE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
        }

        if( bJustEndedEdit )
            bJustEndedEdit = false;
    }
    else if( mpView && mpView->IsCreateObj() && rMEvt.IsLeft())
    {
        /**********************************************************************
        * Objekt wurde erzeugt
        **********************************************************************/
        mxTextObj.reset( dynamic_cast< SdrTextObj* >( mpView->GetCreateObj() ) );

        if( mxTextObj.is() )
        {
            //AW outliner needs to be set to vertical when there is no
            // outliner object up to now; also it needs to be set back to not
            // vertical when there was a vertical one used last time.
            OutlinerParaObject* pOPO = GetTextObj()->GetOutlinerParaObject();
            SdrOutliner& rOutl = mxTextObj->GetModel()->GetDrawOutliner(GetTextObj());
            sal_Bool bVertical((pOPO && pOPO->IsVertical())
                || nSlotId == SID_ATTR_CHAR_VERTICAL
                || nSlotId == SID_TEXT_FITTOSIZE_VERTICAL);
            rOutl.SetVertical(bVertical);

            // Before ImpSetAttributesForNewTextObject the vertical writing mode
            // needs to be set at the object. This is done here at the OutlinerParaObject
            // directly to not mirror the layout text items involved. These items will be set
            // from ImpSetAttributesForNewTextObject and below.
            OutlinerParaObject* pPara = GetTextObj()->GetOutlinerParaObject();

            if(!pPara)
            {
                GetTextObj()->ForceOutlinerParaObject();
                pPara = GetTextObj()->GetOutlinerParaObject();
            }

            if(pPara && (bool)bVertical != pPara->IsVertical())
            {
                // set ParaObject orientation accordingly
                pPara->SetVertical(bVertical);
            }

            ImpSetAttributesForNewTextObject(GetTextObj());
        }

        if (!mpView->EndCreateObj(SDRCREATE_FORCEEND))
        {
            // Textobjekt konnte nicht erzeugt werden
            mxTextObj.reset(0);
        }
        else if (nSlotId == SID_TEXT_FITTOSIZE)
        {
            ImpSetAttributesFitToSize(GetTextObj());

            SetInEditMode(rMEvt, sal_False);
        }
        else if ( nSlotId == SID_TEXT_FITTOSIZE_VERTICAL )
        {
            ImpSetAttributesFitToSizeVertical(GetTextObj());

            SetInEditMode(rMEvt, sal_False);
        }
        else
        {
            ImpSetAttributesFitCommon(GetTextObj());

            // Damit die Handles und der graue Rahmen stimmen
            mpView->AdjustMarkHdl();
            mpView->PickHandle(aPnt);
            SetInEditMode(rMEvt, sal_False);
        }
    }
    else if ( mpView && mpView->IsAction())
    {
        mpView->EndAction();
    }

    ForcePointer(&rMEvt);
    mpWindow->ReleaseMouse();
    sal_uInt16 nDrgLog1 = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

    if ( mpView && !mpView->AreObjectsMarked() &&
         Abs(aMDPos.X() - aPnt.X()) < nDrgLog1 &&
         Abs(aMDPos.Y() - aPnt.Y()) < nDrgLog1 &&
         !rMEvt.IsShift() && !rMEvt.IsMod2() )
    {
        SdrPageView* pPV = mpView->GetSdrPageView();
        SdrViewEvent aVEvt;
        mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
        mpView->MarkObj(aVEvt.pRootObj, pPV);
    }

    if ( !mxTextObj.is() && mpView )
    {
        if ( ( (!bEmptyTextObj   &&  bPermanent) ||
             (!bFirstObjCreated && !bPermanent) ) &&
              !mpDocSh->IsReadOnly()               &&
              nSlotId != SID_TEXTEDIT )
        {
            /**********************************************************************
            * Mengentext (linksbuendiges AutoGrow)
            **********************************************************************/
            mpView->SetCurrentObj(OBJ_TEXT);
            mpView->SetEditMode(SDREDITMODE_CREATE);
            sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
            mpView->BegCreateObj(aMDPos, (OutputDevice*) NULL, nDrgLog);

            sal_Bool bSnapEnabled = mpView->IsSnapEnabled();

            if (bSnapEnabled)
                mpView->SetSnapEnabled(sal_False);

            aPnt.X() += nDrgLog + nDrgLog;
            aPnt.Y() += nDrgLog + nDrgLog;
            mpView->MovAction(aPnt);

            mxTextObj.reset( dynamic_cast< SdrTextObj* >( mpView->GetCreateObj() ) );

            if(mxTextObj.is())
            {
                GetTextObj()->SetDisableAutoWidthOnDragging(sal_True);
            }

            if(!mpView->EndCreateObj(SDRCREATE_FORCEEND))
            {
                mxTextObj.reset(0);
            }

            if(bSnapEnabled)
                mpView->SetSnapEnabled(bSnapEnabled);

            if(mxTextObj.is())
            {
                SfxItemSet aSet(mpViewShell->GetPool());
                aSet.Put(SdrTextMinFrameHeightItem(0));
                aSet.Put(SdrTextMinFrameWidthItem(0));
                aSet.Put(SdrTextAutoGrowHeightItem(sal_True));
                aSet.Put(SdrTextAutoGrowWidthItem(sal_True));

                if(nSlotId == SID_ATTR_CHAR_VERTICAL)
                {
                    // Here, all items which need to be different from pool default need to be set
                    // again on the newly created text object.
                    // Since this is a simple klick text object, it is first created, then SetVertical()
                    // is used, then ImpSetAttributesForNewTextObject is called and then the object is
                    // deleted again since not the minimum drag distance was travelled. Then, a new
                    // klick text object is created and thus all that stuff needs to be set again here.
                    //
                    // Before using the new object the vertical writing mode
                    // needs to be set. This is done here at the OutlinerParaObject
                    // directly to not mirror the layout text items involved. These items will be set
                    // below.
                    OutlinerParaObject* pPara = GetTextObj()->GetOutlinerParaObject();

                    if(!pPara)
                    {
                        GetTextObj()->ForceOutlinerParaObject();
                        pPara = GetTextObj()->GetOutlinerParaObject();
                    }

                    if(pPara && sal_True != pPara->IsVertical())
                    {
                        // set ParaObject orientation accordingly
                        pPara->SetVertical(sal_True);
                    }

                    aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                    // Analog to the else case below, for vertical simple click texts
                    // one of the defaulted setted items from ImpSetAttributesForNewTextObject
                    // needs to be adapted to non-block mode.
                    const SfxItemSet& rSet = mpView->GetDefaultAttr();
                    SvxFrameDirection eDirection = (SvxFrameDirection)((SvxFrameDirectionItem&)rSet.Get(EE_PARA_WRITINGDIR)).GetValue();

                    if(FRMDIR_HORI_RIGHT_TOP == eDirection || FRMDIR_VERT_TOP_RIGHT == eDirection)
                    {
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BOTTOM));
                    }
                    else
                    {
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                    }
                }
                else
                {
                    // This is for Format/Page settings. Since this also leads
                    // to the object defaults to be changed, i think this code can be
                    // removed. CL. wanted to take a look before adding this.

                    // Look in the object defaults if left-to-right is wanted. If
                    // yes, set text anchoring to right to let the box grow to left.
                    const SfxItemSet& rSet = mpView->GetDefaultAttr();
                    SvxFrameDirection eDirection = (SvxFrameDirection)((SvxFrameDirectionItem&)rSet.Get(EE_PARA_WRITINGDIR)).GetValue();

                    if(FRMDIR_HORI_RIGHT_TOP == eDirection)
                    {
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                    }
                    else
                    {
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT));
                    }
                }

                GetTextObj()->SetMergedItemSet(aSet);
                GetTextObj()->SetDisableAutoWidthOnDragging(sal_True);
                SetInEditMode(rMEvt, sal_False);
            }

            bFirstObjCreated = sal_True;
        }
        else
        {
            // In die Fkt. Selektion wechseln
            if (mpView->SdrEndTextEdit() == SDRENDTEXTEDIT_DELETED)
            {
                mxTextObj.reset(0);
            }

            mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_OBJECT_SELECT,
                                      SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
        }
    }

    bMBDown = sal_False;
    FuConstruct::MouseButtonUp(rMEvt);
    return (bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* sal_False.
|*
\************************************************************************/

sal_Bool FuText::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;
    mpView->SetMarkHdlWhenTextEdit(sal_True);

    KeyCode nCode = rKEvt.GetKeyCode();
    sal_Bool bShift = nCode.IsShift();

    if(mxTextObj.is())
    {
        // maybe object is deleted, test if it's equal to the selected object
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
        SdrObject* pSelectedObj = 0L;

        if(1 == rMarkList.GetMarkCount())
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            pSelectedObj = pMark->GetMarkedSdrObj();
        }

        if(mxTextObj.get() != pSelectedObj)
        {
            mxTextObj.reset(0);
        }
    }

    if ( mxTextObj.is() && mxTextObj->GetObjInventor() == SdrInventor && mxTextObj->GetObjIdentifier() == OBJ_TITLETEXT && rKEvt.GetKeyCode().GetCode() == KEY_RETURN )
    {
        // Titeltext-Objekt: immer "weiche" Umbrueche
        bShift = sal_True;
    }

    sal_uInt16 nKey = nCode.GetCode();
    KeyCode aKeyCode (nKey, bShift, nCode.IsMod1(), nCode.IsMod2(), nCode.IsMod3() );
    KeyEvent aKEvt(rKEvt.GetCharCode(), aKeyCode);

    sal_Bool bOK = sal_True;

    if (mpDocSh->IsReadOnly())
    {
        bOK = !EditEngine::DoesKeyChangeText(aKEvt);
    }
    if( aKeyCode.GetCode() == KEY_PAGEUP || aKeyCode.GetCode() == KEY_PAGEDOWN )
    {
        bOK = sal_False;   // default handling in base class
    }

    if (bOK && mpView->KeyInput(aKEvt, mpWindow) )
    {
        bReturn = sal_True;

        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

    }
    else if (aKeyCode == KEY_ESCAPE)
    {
        bReturn = cancel();
    }

    if( bPermanent )
    {
        mpView->SetCurrentObj(OBJ_TEXT);
        mpView->SetEditMode(SDREDITMODE_CREATE);
    }

    if (!bReturn)
    {
        bReturn = FuDraw::KeyInput(aKEvt);
    }

    return (bReturn);
}



/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuText::Activate()
{
    mpView->SetQuickTextEditMode(mpViewShell->GetFrameView()->IsQuickEdit());

    // #i89661# it's no longer necessary to make it so big here, it's fine tuned
    // for text objects in SdrMarkView::CheckSingleSdrObjectHit
    mpView->SetHitTolerancePixel( 2 * HITPIX );

    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

    if (pOLV)
        pOLV->ShowCursor();

    FuConstruct::Activate();

    if( pOLV )
        mpView->SetEditMode(SDREDITMODE_EDIT);
}


/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuText::Deactivate()
{
    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

    if (pOLV)
        pOLV->HideCursor();

    mpView->SetHitTolerancePixel( HITPIX );

    FuConstruct::Deactivate();
}


/*************************************************************************
|*
|* Objekt in Edit-Mode setzen
|*
\************************************************************************/

void FuText::SetInEditMode(const MouseEvent& rMEvt, sal_Bool bQuickDrag)
{
    SdrPageView* pPV = mpView->GetSdrPageView();
    if( mxTextObj.is() && (mxTextObj->GetPage() == pPV->GetPage()) )
    {
        mpView->SetCurrentObj(OBJ_TEXT);

        if( bPermanent )
            mpView->SetEditMode(SDREDITMODE_CREATE);
        else
            mpView->SetEditMode(SDREDITMODE_EDIT);

        sal_Bool bEmptyOutliner = sal_False;

        if (!GetTextObj()->GetOutlinerParaObject() && mpView->GetTextEditOutliner())
        {
            ::Outliner* pOutl = mpView->GetTextEditOutliner();
            sal_uLong nParaAnz = pOutl->GetParagraphCount();
            Paragraph* p1stPara = pOutl->GetParagraph( 0 );

            if (nParaAnz==1 && p1stPara)
            {
                // Bei nur einem Pararaph
                if (pOutl->GetText(p1stPara).Len() == 0)
                {
                    bEmptyOutliner = sal_True;
                }
            }
        }

        if (GetTextObj() != mpView->GetTextEditObject() || bEmptyOutliner)
        {
            sal_uInt32 nInv = mxTextObj->GetObjInventor();
            sal_uInt16 nSdrObjKind = mxTextObj->GetObjIdentifier();

            if (nInv == SdrInventor && GetTextObj()->HasTextEdit() &&
                (nSdrObjKind == OBJ_TEXT ||
                 nSdrObjKind == OBJ_TITLETEXT ||
                 nSdrObjKind == OBJ_OUTLINETEXT || !mxTextObj->IsEmptyPresObj() ) )
            {
                // Neuen Outliner machen (gehoert der SdrObjEditView)
                SdrOutliner* pOutl = SdrMakeOutliner( OUTLINERMODE_OUTLINEOBJECT, mpDoc );

                if (bEmptyOutliner)
                    mpView->SdrEndTextEdit(sal_True);

                SdrTextObj* pTextObj = GetTextObj();
                if( pTextObj )
                {
                    OutlinerParaObject* pOPO = pTextObj->GetOutlinerParaObject();
                    if( ( pOPO && pOPO->IsVertical() ) || (nSlotId == SID_ATTR_CHAR_VERTICAL) || (nSlotId == SID_TEXT_FITTOSIZE_VERTICAL) )
                        pOutl->SetVertical( sal_True );

                    if( pTextObj->getTextCount() > 1 )
                    {
                        Point aPix(rMEvt.GetPosPixel());
                        Point aPnt(mpWindow->PixelToLogic(aPix));
                        pTextObj->setActiveText( pTextObj->CheckTextHit(aPnt ) );
                    }

                    if (mpView->SdrBeginTextEdit(pTextObj, pPV, mpWindow, sal_True, pOutl) && mxTextObj->GetObjInventor() == SdrInventor)
                    {
                        bFirstObjCreated = sal_True;
                        DeleteDefaultText();

                        OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

                        nSdrObjKind = mxTextObj->GetObjIdentifier();

                        SdrViewEvent aVEvt;
                        SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                        if (eHit == SDRHIT_TEXTEDIT)
                        {
                            // Text getroffen
                            if (nSdrObjKind == OBJ_TEXT ||
                                nSdrObjKind == OBJ_TITLETEXT ||
                                nSdrObjKind == OBJ_OUTLINETEXT ||
                                nSdrObjKind == OBJ_TABLE ||
                                nSlotId == SID_TEXTEDIT ||
                                !bQuickDrag)
                            {
                                pOLV->MouseButtonDown(rMEvt);
                                pOLV->MouseMove(rMEvt);
                                pOLV->MouseButtonUp(rMEvt);
                            }

                            if (mpViewShell->GetFrameView()->IsQuickEdit() && bQuickDrag && GetTextObj()->GetOutlinerParaObject())
                            {
                                pOLV->MouseButtonDown(rMEvt);
                            }
                        }
                        else
                        {
                            // Move cursor to end of text
                            ESelection aNewSelection(EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND, EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND);
                            pOLV->SetSelection(aNewSelection);
                        }
                    }
                    else
                    {
                        mpView->RestoreDefaultText(dynamic_cast< SdrTextObj* >( mxTextObj.get() ));
                    }
                }
            }
        }
    }
    else
    {
        mxTextObj.reset(0);
    }
}

/*************************************************************************
|*
|* Texteingabe wird gestartet, ggf. Default-Text loeschen
|*
\************************************************************************/

sal_Bool FuText::DeleteDefaultText()
{
    sal_Bool bDeleted = sal_False;

    if ( mxTextObj.is() && mxTextObj->IsEmptyPresObj() )
    {
        String aString;
        SdPage* pPage = (SdPage*) mxTextObj->GetPage();

        if (pPage)
        {
            PresObjKind ePresObjKind = pPage->GetPresObjKind(mxTextObj.get());

            if ( (ePresObjKind == PRESOBJ_TITLE   ||
                  ePresObjKind == PRESOBJ_OUTLINE ||
                  ePresObjKind == PRESOBJ_NOTES   ||
                  ePresObjKind == PRESOBJ_TEXT) &&
                  !pPage->IsMasterPage() )
            {
                ::Outliner* pOutliner = mpView->GetTextEditOutliner();
                SfxStyleSheet* pSheet = pOutliner->GetStyleSheet( 0 );
                sal_Bool bIsUndoEnabled = pOutliner->IsUndoEnabled();
                if( bIsUndoEnabled )
                    pOutliner->EnableUndo(sal_False);

                pOutliner->SetText( String(), pOutliner->GetParagraph( 0 ) );

                if( bIsUndoEnabled )
                    pOutliner->EnableUndo(sal_True);

                if (pSheet &&
                    (ePresObjKind == PRESOBJ_NOTES || ePresObjKind == PRESOBJ_TEXT))
                    pOutliner->SetStyleSheet(0, pSheet);

                mxTextObj->SetEmptyPresObj(sal_True);
                bDeleted = sal_True;
            }
        }
    }

    return(bDeleted);
}

/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

sal_Bool FuText::Command(const CommandEvent& rCEvt)
{
    return( FuPoor::Command(rCEvt) );
}

/*************************************************************************
|*
|* Help-event
|*
\************************************************************************/

sal_Bool FuText::RequestHelp(const HelpEvent& rHEvt)
{
    sal_Bool bReturn = sal_False;

    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

    if ((Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled()) &&
        mxTextObj.is() && pOLV && pOLV->GetFieldUnderMousePointer())
    {
        String aHelpText;
        const SvxFieldItem* pFieldItem = pOLV->GetFieldUnderMousePointer();
        const SvxFieldData* pField = pFieldItem->GetField();

        if (pField && pField->ISA(SvxURLField))
        {
            /******************************************************************
            * URL-Field
            ******************************************************************/
            aHelpText = INetURLObject::decode( ((const SvxURLField*)pField)->GetURL(), '%', INetURLObject::DECODE_WITH_CHARSET );
        }
        if (aHelpText.Len())
        {
            Rectangle aLogicPix = mpWindow->LogicToPixel(mxTextObj->GetLogicRect());
            Rectangle aScreenRect(mpWindow->OutputToScreenPixel(aLogicPix.TopLeft()),
                                  mpWindow->OutputToScreenPixel(aLogicPix.BottomRight()));

            if (Help::IsBalloonHelpEnabled())
            {
                bReturn = Help::ShowBalloon( (Window*)mpWindow, rHEvt.GetMousePosPixel(), aScreenRect, aHelpText);
            }
            else if (Help::IsQuickHelpEnabled())
            {
                bReturn = Help::ShowQuickHelp( (Window*)mpWindow, aScreenRect, aHelpText);
            }
        }
    }

    if (!bReturn)
    {
        bReturn = FuConstruct::RequestHelp(rHEvt);
    }

    return(bReturn);
}

/*************************************************************************
|*
|* Request verarbeiten
|*
\************************************************************************/

void FuText::ReceiveRequest(SfxRequest& rReq)
{
    nSlotId = rReq.GetSlot();

    // Dann Basisklasse rufen (dort wird u.a. nSlotId NICHT gesetzt)
    FuPoor::ReceiveRequest(rReq);

    if (nSlotId == SID_TEXTEDIT || mpViewShell->GetFrameView()->IsQuickEdit() || SID_ATTR_CHAR == nSlotId)
    {
        MouseEvent aMEvt(mpWindow->GetPointerPosPixel());

        mxTextObj.reset(0);

        if (nSlotId == SID_TEXTEDIT)
        {
            // Wird gerade editiert?
            if(!bTestText)
                mxTextObj.reset( dynamic_cast< SdrTextObj* >( mpView->GetTextEditObject() ) );

            if (!mxTextObj.is())
            {
                // Try to select an object
                SdrPageView* pPV = mpView->GetSdrPageView();
                SdrViewEvent aVEvt;
                mpView->PickAnything(aMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
                mpView->MarkObj(aVEvt.pRootObj, pPV);

                if (aVEvt.pObj && aVEvt.pObj->ISA(SdrTextObj))
                {
                    mxTextObj.reset( static_cast< SdrTextObj* >( aVEvt.pObj ) );
                }
            }
        }
        else if (mpView->AreObjectsMarked())
        {
            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

            if (rMarkList.GetMarkCount() == 1)
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

                if (pObj->ISA(SdrTextObj))
                {
                    mxTextObj.reset( static_cast< SdrTextObj* >( pObj ) );
                }
            }
        }

        sal_Bool bQuickDrag = sal_True;

        const SfxItemSet* pArgs = rReq.GetArgs();

        if (pArgs

            // test for type before using
            && SID_TEXTEDIT == nSlotId
            && SFX_ITEM_SET == pArgs->GetItemState(SID_TEXTEDIT)

            && (sal_uInt16) ((SfxUInt16Item&) pArgs->Get(SID_TEXTEDIT)).GetValue() == 2)
        {
            // Anwahl per Doppelklick -> kein QuickDrag zulassen
            bQuickDrag = sal_False;
        }

        SetInEditMode(aMEvt, bQuickDrag);
    }
}

void FuText::DoubleClick(const MouseEvent& )
{
    // Nothing to do
}

/** Removed the insertion of default text and putting a new text
    object directly into edit mode.
*/
SdrObject* FuText::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->GetCurrentObjInventor(), mpView->GetCurrentObjIdentifier(),
        0L, mpDoc);

    if(pObj)
    {
        if(pObj->ISA(SdrTextObj))
        {
            SdrTextObj* pText = (SdrTextObj*)pObj;
            pText->SetLogicRect(rRectangle);

            sal_Bool bVertical = (SID_ATTR_CHAR_VERTICAL == nID || SID_TEXT_FITTOSIZE_VERTICAL == nID);
            pText->SetVerticalWriting(bVertical);

            ImpSetAttributesForNewTextObject(pText);

            if (nSlotId == SID_TEXT_FITTOSIZE)
            {
                ImpSetAttributesFitToSize(pText);
            }
            else if ( nSlotId == SID_TEXT_FITTOSIZE_VERTICAL )
            {
                ImpSetAttributesFitToSizeVertical(pText);
            }
            else
            {
                ImpSetAttributesFitCommon(pText);
            }

            // Put text object into edit mode.
            SdrPageView* pPV = mpView->GetSdrPageView();
            mpView->SdrBeginTextEdit(pText, pPV);
        }
        else
        {
            OSL_FAIL("Object is NO text object");
        }
    }

    return pObj;
}




/** is called when the currenct function should be aborted. <p>
    This is used when a function gets a KEY_ESCAPE but can also
    be called directly.

    @returns true if a active function was aborted
*/
bool FuText::cancel()
{
    if ( mpView->IsTextEdit() )
    {
        if(mpView->SdrEndTextEdit() == SDRENDTEXTEDIT_DELETED)
            mxTextObj.reset(0);

        mpView->SetCurrentObj(OBJ_TEXT);
        mpView->SetEditMode(SDREDITMODE_EDIT);
        return true;
    }
    else
    {
        return false;
    }
}

void FuText::ChangeFontSize( bool bGrow, OutlinerView* pOLV, const FontList* pFontList, ::sd::View* pView )
{
    if( !pFontList || !pView )
        return;

    if( pOLV )
    {
        pOLV->GetEditView().ChangeFontSize( bGrow, pFontList );
    }
    else
    {

        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        for( sal_uInt32 nMark = 0; nMark < rMarkList.GetMarkCount(); nMark++ )
        {
            SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( rMarkList.GetMark(nMark)->GetMarkedSdrObj() );
            if( pTextObj )
            {
                for( sal_Int32 nText = 0; nText < pTextObj->getTextCount(); nText++ )
                {
                    pTextObj->setActiveText( nText );

                    // Put text object into edit mode.
                    SdrPageView* pPV = pView->GetSdrPageView();
                    pView->SdrBeginTextEdit(pTextObj, pPV);

                    pOLV = pView->GetTextEditOutlinerView();
                    if( pOLV )
                    {
                        EditEngine* pEditEngine = pOLV->GetEditView().GetEditEngine();
                        if( pEditEngine )
                        {
                            ESelection aSel;
                            aSel.nEndPara = pEditEngine->GetParagraphCount()-1;
                            aSel.nEndPos = pEditEngine->GetTextLen(aSel.nEndPara);
                            pOLV->SetSelection(aSel);
                        }

                        ChangeFontSize( bGrow, pOLV, pFontList, pView );
                    }

                    pView->SdrEndTextEdit();
                }

                SfxItemSet aShapeSet( pTextObj->GetMergedItemSet() );
                if( EditView::ChangeFontSize( bGrow, aShapeSet, pFontList ) )
                {
                    pTextObj->SetObjectItemNoBroadcast( aShapeSet.Get( EE_CHAR_FONTHEIGHT ) );
                    pTextObj->SetObjectItemNoBroadcast( aShapeSet.Get( EE_CHAR_FONTHEIGHT_CJK ) );
                    pTextObj->SetObjectItemNoBroadcast( aShapeSet.Get( EE_CHAR_FONTHEIGHT_CTL ) );
                }
            }
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
