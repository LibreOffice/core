/*************************************************************************
 *
 *  $RCSfile: futext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:11:56 $
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

#pragma hdrstop

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#include <svx/svdetc.hxx>
#include <svx/dlgutil.hxx>
#ifndef _SVXERR_HXX //autogen
#include <svx/svxerr.hxx>
#endif
#ifndef _OFA_OSPLCFG_HXX //autogen
#include <offmgr/osplcfg.hxx>
#endif
#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SVDOGROUP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#define ITEMID_FIELD    EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#include <basctl/idetemp.hxx>
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _MyEDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif
#ifndef _SVDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif
#include <svx/svxids.hrc>
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#include "sdresid.hxx"
#include "app.hrc"
#include "res_bmp.hrc"
#include "futext.hxx"
#include "viewshel.hxx"
#include "sdview.hxx"
#include "sdoutl.hxx"
#include "sdwindow.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdmod.hxx"
#include "frmview.hxx"
#include "docshell.hxx"
#include "glob.hrc"
#include "pres.hxx"

using namespace ::com::sun::star;

static USHORT SidArray[] = {
    SID_STYLE_FAMILY2,                //    5542
    SID_STYLE_FAMILY5,                //    5545
    SID_CUT,                          //    5710
    SID_COPY,                         //    5711
//  SID_ATTR_TABSTOP,                 //   10002
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
    FN_NUM_BULLET_ON,                 //   20138
    SID_BULLET,                       //   27019
    SID_PARASPACE_INCREASE,           //   27346
    SID_PARASPACE_DECREASE,           //   27347
                            0 };

TYPEINIT1( FuText, FuConstruct );

#include <stdio.h>

static BOOL bTestText = 0;

/*************************************************************************
|*
|* Basisklasse fuer Textfunktionen
|*
\************************************************************************/

FuText::FuText(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
               SdDrawDocument* pDoc, SfxRequest& rReq) :
    FuConstruct(pViewSh, pWin, pView, pDoc, rReq),
    pTextObj(NULL),
    bFirstObjCreated(FALSE),
    rRequest (rReq)
{}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuText::~FuText()
{
    if (pView->EndTextEdit() == SDRENDTEXTEDIT_DELETED)
    {
        pTextObj = NULL;
    }

    // die RequestHandler der benutzten Outliner zuruecksetzen auf den
    // Handler am Dokument
    Outliner* pOutliner = pView->GetTextEditOutliner();

    if (pOutliner)
    {
        pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)
                    pDoc->GetStyleSheetPool());
        pOutliner->SetMinDepth(0);
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
void FuText::DoExecute ()
{
    pViewShell->SwitchObjectBar(RID_DRAW_TEXT_TOOLBOX);
    pView->SetCurrentObj(OBJ_TEXT);
    pView->SetEditMode(SDREDITMODE_EDIT);

    SdrViewEvent aVEvt;

    if (nSlotId == SID_TEXTEDIT || pViewShell->GetFrameView()->IsQuickEdit())
    {
        MouseEvent aMEvt(pWindow->GetPointerPosPixel());

        if (nSlotId == SID_TEXTEDIT)
        {
            // Try to select an object
            SdrPageView* pPV = pView->GetPageViewPvNum(0);
            SdrViewEvent aVEvt;
            SdrHitKind eHit = pView->PickAnything(aMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
            pView->MarkObj(aVEvt.pRootObj, pPV);

            if (aVEvt.pObj && aVEvt.pObj->ISA(SdrTextObj))
            {
                pTextObj = (SdrTextObj*) aVEvt.pObj;
            }
        }
        else if (pView->HasMarkedObj())
        {
            const SdrMarkList& rMarkList = pView->GetMarkList();

            if (rMarkList.GetMarkCount() == 1)
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetObj();

                if (pObj->ISA(SdrTextObj))
                {
                    pTextObj = (SdrTextObj*) pObj;
                }
            }
        }

        BOOL bQuickDrag = TRUE;

        const SfxItemSet* pArgs = rRequest.GetArgs();

        if (pArgs &&
            (UINT16) ((SfxUInt16Item&) pArgs->Get(SID_TEXTEDIT)).GetValue() == 2)
        {
            // Selection by doubleclick -> don't allow QuickDrag
            bQuickDrag = FALSE;
        }

        SetInEditMode(aMEvt, bQuickDrag);
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuText::MouseButtonDown(const MouseEvent& rMEvt)
{
    bMBDown = TRUE;

    BOOL bReturn = FuDraw::MouseButtonDown(rMEvt);

    // Fuer PopupMenu (vorher DrawViewShell)
    if ((rMEvt.GetButtons() == MOUSE_RIGHT) && rMEvt.GetClicks() == 1 &&
        pView->IsTextEdit())
    {
        return (TRUE);
    }

    pView->SetMarkHdlWhenTextEdit(TRUE);
    SdrViewEvent aVEvt;
    SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

    if (eHit == SDRHIT_TEXTEDIT)
    {
        // Text getroffen -> Event von SdrView auswerten lassen
        if (pView->MouseButtonDown(rMEvt, pWindow))
            return (TRUE);
    }

    if (rMEvt.GetClicks() == 1)
    {
        if (pView->IsTextEdit() && eHit != SDRHIT_MARKEDOBJECT && eHit != SDRHIT_HANDLE)
        {
            // Texteingabe beenden
            if (pView->EndTextEdit() == SDRENDTEXTEDIT_DELETED)
            {
                // Bugfix von MBA: bei Doppelclick auf der Wiese im Modus Text wird
                // beim zweiten Click eHit = SDRHIT_TEXTEDITOBJ erhalten, weil ja der
                // zweite Click auf das im ersten Click angelegte TextObject geht.
                // Dieses wird aber in EndTextEdit entfernt, weil es leer ist. Es
                // befindet sich aber noch in der Mark-Liste und der Aufruf MarkObj
                // weiter unten greift dann auf das tote Object zu.
                // Als einfacher Fix wird nach EndTextEdit noch einmal eHit ermittelt,
                // was dann SDRHIT_NONE liefert.
                pTextObj = NULL;
                eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
            }

            pView->SetCurrentObj(OBJ_TEXT);
            pView->SetEditMode(SDREDITMODE_EDIT);
        }

        if (rMEvt.IsLeft())
        {
            pWindow->CaptureMouse();
            SdrObject* pObj;
            SdrPageView* pPV = pView->GetPageViewPvNum(0);

            if (eHit == SDRHIT_TEXTEDIT)
            {
                SetInEditMode(rMEvt, FALSE);
            }
            else
            {
                BOOL bMacro = FALSE;

                if (bMacro && pView->PickObj(aMDPos,pObj,pPV,SDRSEARCH_PICKMACRO))
                {
                    // Makro
                    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );
                    pView->BegMacroObj(aMDPos,nHitLog,pObj,pPV,pWindow);
                }
                else
                {
                    if (eHit != SDRHIT_HANDLE)
                    {
                        // Selektion aufheben
                        if (!rMEvt.IsShift() && eHit == SDRHIT_TEXTEDITOBJ)
                        {
                            pView->UnmarkAll();
                            pView->SetDragMode(SDRDRAG_MOVE);
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
                            pView->MarkObj(aVEvt.pRootObj, pPV);

                            if (aVEvt.pObj && aVEvt.pObj->ISA(SdrTextObj))
                            {
                                pTextObj = (SdrTextObj*) aVEvt.pObj;
                            }

                            SetInEditMode(rMEvt, TRUE);
                        }
                        else if (aVEvt.eEvent == SDREVENT_EXECUTEURL && !rMEvt.IsMod2())
                        {
                            /******************************************************
                            * URL ausfuehren
                            ******************************************************/
                            pWindow->ReleaseMouse();
                            SfxStringItem aStrItem(SID_FILE_NAME, aVEvt.pURLField->GetURL());
                            SfxStringItem aReferer(SID_REFERER, pDocSh->GetMedium()->GetName());
                            SfxBoolItem aBrowseItem( SID_BROWSING, TRUE );
                            SfxViewFrame* pFrame = pViewShell->GetViewFrame();
                            pWindow->ReleaseMouse();

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
                            if (!aVEvt.pHdl)
                            {
                                if( eHit == SDRHIT_UNMARKEDOBJECT )
                                {
                                    if ( !rMEvt.IsShift() )
                                        pView->UnmarkAll();

                                    pView->MarkObj(aVEvt.pRootObj, pPV);
                                }

                                // Objekt draggen
                                bFirstMouseMove = TRUE;
                                aDragTimer.Start();
                            }

                            Outliner* pOutl = pView->GetTextEditOutliner();

                            if (pTextObj && (pTextObj->GetOutlinerParaObject() ||
                                (pOutl && pOutl->GetText(pOutl->GetParagraph( 0 )).Len() != 0)))
                            {
                                pView->EndTextEdit();
                            }

                            USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
                            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
                        }
                    }
                    else if ( nSlotId != SID_TEXTEDIT &&
                              (bPermanent || !bFirstObjCreated) )
                    {
                        /**********************************************************
                        * Objekt erzeugen
                        **********************************************************/
                        pView->SetCurrentObj(OBJ_TEXT);
                        pView->SetEditMode(SDREDITMODE_CREATE);
                        USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
                        pView->BegCreateObj(aMDPos, (OutputDevice*) NULL, nDrgLog);
                    }
                    else
                    {
                        /**********************************************************
                        * Selektieren
                        **********************************************************/
                        if( !rMEvt.IsShift() )
                            pView->UnmarkAll();

                        pView->BegMarkObj( aMDPos );
                    }
                }
            }
        }
    }
    else if ( rMEvt.GetClicks() == 2 && !pView->IsTextEdit() )
    {
        MouseEvent aMEvt( pWindow->GetPointerPosPixel() );
        SetInEditMode( aMEvt, FALSE );
    }

    if (!bIsInDragMode)
    {
        ForcePointer(&rMEvt);
        pViewShell->GetViewFrame()->GetBindings().Invalidate(SidArray);
    }

    return (bReturn);
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuText::MouseMove(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        if( bFirstMouseMove )
            bFirstMouseMove = FALSE;
        else
            aDragTimer.Stop();
    }

    if (!bReturn && pView->IsAction())
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(pWindow->PixelToLogic(aPix));

        ForceScroll(aPix);
        pView->MovAction(aPnt);
    }

    ForcePointer(&rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuText::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if (aDragTimer.IsActive())
    {
        aDragTimer.Stop();
        bIsInDragMode = FALSE;
    }

    pViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if (pView->MouseButtonUp(rMEvt, pWindow) || rMEvt.GetClicks() == 2 )
        return (TRUE); // Event von der SdrView ausgewertet

    BOOL bEmptyTextObj = FALSE;

    if (pTextObj)
    {
        const SdrMarkList& rMarkList = pView->GetMarkList();

        if (rMarkList.GetMarkCount() == 1 &&
            ( rMarkList.GetMark(0)->GetObj() == pTextObj ||
              rMarkList.GetMark(0)->GetObj()->ISA( SdrObjGroup ) ) )
        {
            // Ist pTextObj wirklich noch gueltig?
            // (Im FontWork wird z.B. das Objekt ausgetauscht!)
            if (!pTextObj->GetOutlinerParaObject())
                bEmptyTextObj = TRUE;
            else
                bFirstObjCreated = TRUE;
        }
        else
            pTextObj = NULL;
    }

    if (pView->IsDragObj())
    {
        /**********************************************************************
        * Objekt wurde verschoben
        **********************************************************************/
        FrameView* pFrameView = pViewShell->GetFrameView();
        BOOL bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        if (bDragWithCopy)
        {
            bDragWithCopy = !pView->IsPresObjSelected(FALSE, TRUE);
        }

        pView->SetDragWithCopy(bDragWithCopy);
        pView->EndDragObj( pView->IsDragWithCopy() );
        pView->ForceMarkedToAnotherPage();
        pView->SetCurrentObj(OBJ_TEXT);
    }
    else if (pView->IsCreateObj() && rMEvt.IsLeft())
    {
        /**********************************************************************
        * Objekt wurde erzeugt
        **********************************************************************/
        pTextObj = (SdrTextObj*) pView->GetCreateObj();

        if (pTextObj && nSlotId != SID_TEXT_FITTOSIZE && pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS)
        {
            // Impress-Textobjekt wird erzeugt (faellt auf Zeilenhoehe zusammen)
            // Damit das Objekt beim anschliessenden Erzeugen gleich die richtige
            // Hoehe bekommt (sonst wird zuviel gepainted)
            SfxItemSet aSet(pViewShell->GetPool());
            SdrTextMinFrameHeightItem aMinHeight(0);
            aSet.Put(aMinHeight);
            SdrTextAutoGrowHeightItem aAutoGrowHeight(TRUE);
            aSet.Put(aAutoGrowHeight);
            pTextObj->NbcSetAttributes(aSet, FALSE);
            pTextObj->AdjustTextFrameWidthAndHeight();
            SdrTextMaxFrameHeightItem aMaxHeight(pTextObj->GetLogicRect().GetSize().Height());
            aSet.Put(aMaxHeight);
            pTextObj->NbcSetAttributes(aSet, FALSE);
        }

        if (!pView->EndCreateObj(SDRCREATE_FORCEEND))
        {
            // Textobjekt konnte nicht erzeugt werden
            pTextObj = NULL;
        }
        else if (nSlotId == SID_TEXT_FITTOSIZE)
        {
            // FitToSize (An Rahmen anpassen)
            SfxItemSet aSet(pViewShell->GetPool(), SDRATTR_TEXT_AUTOGROWHEIGHT,
                                                   SDRATTR_TEXT_AUTOGROWWIDTH, 0L);
            SdrFitToSizeType eFTS = SDRTEXTFIT_PROPORTIONAL;
            aSet.Put(SdrTextFitToSizeTypeItem(eFTS));
            aSet.Put(SdrTextAutoGrowHeightItem(FALSE));
            aSet.Put(SdrTextAutoGrowWidthItem(FALSE));
            pTextObj->NbcSetAttributes(aSet, FALSE);
            pTextObj->AdjustTextFrameWidthAndHeight();
            SetInEditMode(rMEvt, FALSE);
        }
        else
        {
            // Normales Textobjekt
            if (pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS)
            {
                // Impress-Textobjekt (faellt auf Zeilenhoehe zusammen)
                SfxItemSet aSet(pViewShell->GetPool());
                SdrTextMinFrameHeightItem aMinHeight(0);
                aSet.Put(aMinHeight);
                SdrTextMaxFrameHeightItem aMaxHeight(0);
                aSet.Put(aMaxHeight);
                SdrTextAutoGrowHeightItem aAutoGrowHeight(TRUE);
                aSet.Put(aAutoGrowHeight);
                pTextObj->NbcSetAttributes(aSet, FALSE);
                pTextObj->AdjustTextFrameWidthAndHeight();
            }

            // Damit die Handles und der graue Rahmen stimmen
            pView->AdjustMarkHdl();
            pView->HitHandle(aPnt, *pWindow);
            SetInEditMode(rMEvt, FALSE);
        }
    }
    else if (pView->IsAction())
    {
        pView->EndAction();
    }

    ForcePointer(&rMEvt);
    pWindow->ReleaseMouse();
    USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

    if ( !pView->HasMarkedObj() &&
         Abs(aMDPos.X() - aPnt.X()) < nDrgLog &&
         Abs(aMDPos.Y() - aPnt.Y()) < nDrgLog &&
         !rMEvt.IsShift() && !rMEvt.IsMod2() )
    {
        SdrPageView* pPV = pView->GetPageViewPvNum(0);
        SdrViewEvent aVEvt;
        SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
        pView->MarkObj(aVEvt.pRootObj, pPV);
    }

    if ( !pTextObj )
    {
        if ( ( (!bEmptyTextObj   &&  bPermanent) ||
             (!bFirstObjCreated && !bPermanent) ) &&
              !pDocSh->IsReadOnly()               &&
              nSlotId != SID_TEXTEDIT )
        {
            /**********************************************************************
            * Mengentext (linksbuendiges AutoGrow)
            **********************************************************************/
            pView->SetCurrentObj(OBJ_TEXT);
            pView->SetEditMode(SDREDITMODE_CREATE);
            USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
            pView->BegCreateObj(aMDPos, (OutputDevice*) NULL, nDrgLog);

            BOOL bSnapEnabled = pView->IsSnapEnabled();

            if (bSnapEnabled)
                pView->SetSnapEnabled(FALSE);

            aPnt.X() += nDrgLog + nDrgLog;
            aPnt.Y() += nDrgLog + nDrgLog;
            pView->MovAction(aPnt);

            pTextObj = (SdrTextObj*) pView->GetCreateObj();

            if (pTextObj)
            {
                pTextObj->SetDisableAutoWidthOnDragging(TRUE);
            }

            if (!pView->EndCreateObj(SDRCREATE_FORCEEND))
            {
                pTextObj = NULL;
            }

            if (bSnapEnabled)
                pView->SetSnapEnabled(bSnapEnabled);

            if (pTextObj)
            {
                SfxItemSet aSet(pViewShell->GetPool());
                SdrTextMinFrameHeightItem aMinHeight(0);
                aSet.Put(aMinHeight);
                SdrTextMinFrameWidthItem aMinWidth(0);
                aSet.Put(aMinWidth);
                SdrTextAutoGrowHeightItem aAutoGrowHeight(TRUE);
                aSet.Put(aAutoGrowHeight);
                SdrTextAutoGrowWidthItem aAutoGrowWidth(TRUE);
                aSet.Put(aAutoGrowWidth);
                SdrTextHorzAdjustItem aTextHorzAdjust(SDRTEXTHORZADJUST_LEFT);
                aSet.Put(aTextHorzAdjust);
                pTextObj->NbcSetAttributes(aSet, FALSE);
                pTextObj->SetDisableAutoWidthOnDragging(TRUE);
                SetInEditMode(rMEvt, FALSE);
            }

            bFirstObjCreated = TRUE;
        }
        else
        {
            // In die Fkt. Selektion wechseln
            if (pView->EndTextEdit() == SDRENDTEXTEDIT_DELETED)
            {
                pTextObj = NULL;
            }

            pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_OBJECT_SELECT,
                                      SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
        }
    }

    bMBDown = FALSE;
    FuConstruct::MouseButtonUp(rMEvt);
    return (bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuText::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;
    pView->SetMarkHdlWhenTextEdit(TRUE);

    KeyCode nCode = rKEvt.GetKeyCode();
    BOOL bShift = nCode.IsShift();

    if ( pTextObj && pTextObj->GetObjInventor() == SdrInventor &&
         pTextObj->GetObjIdentifier() == OBJ_TITLETEXT
         && rKEvt.GetKeyCode().GetCode() == KEY_RETURN )
    {
        // Titeltext-Objekt: immer "weiche" Umbrueche
        bShift = TRUE;
    }

    USHORT nKey = nCode.GetCode();
    KeyCode aKeyCode (nKey, bShift, nCode.IsMod1(), nCode.IsMod2() );
    KeyEvent aKEvt(rKEvt.GetCharCode(), aKeyCode);

    BOOL bOK = TRUE;

    if (pDocSh->IsReadOnly())
    {
        bOK = !EditEngine::DoesKeyChangeText(aKEvt);
    }
    if( aKeyCode.GetCode() == KEY_PAGEUP || aKeyCode.GetCode() == KEY_PAGEDOWN )
    {
        bOK = FALSE;   // default handling in base class
    }

    if (bOK && pView->KeyInput(aKEvt, pWindow) )
    {
        bReturn = TRUE;

        pViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

        if ( pTextObj )
        {
            pTextObj->SetEmptyPresObj(FALSE);
        }
    }
    else if (aKeyCode == KEY_ESCAPE)
    {
        if ( pView->IsTextEdit() )
        {
            if (pView->EndTextEdit() == SDRENDTEXTEDIT_DELETED)
            {
                pTextObj = NULL;
            }

            pView->SetCurrentObj(OBJ_TEXT);
            pView->SetEditMode(SDREDITMODE_EDIT);
            bReturn = TRUE;
        }
    }

    if( bPermanent )
    {
        pView->SetCurrentObj(OBJ_TEXT);
        pView->SetEditMode(SDREDITMODE_CREATE);
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
    pView->SetQuickTextEditMode(pViewShell->GetFrameView()->IsQuickEdit());

    pView->SetHitTolerancePixel( 4 * HITPIX );

    OutlinerView* pOLV = pView->GetTextEditOutlinerView();

    if (pOLV)
        pOLV->ShowCursor();

    FuConstruct::Activate();

    if( pOLV )
        pView->SetEditMode(SDREDITMODE_EDIT);
}


/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuText::Deactivate()
{
    OutlinerView* pOLV = pView->GetTextEditOutlinerView();

    if (pOLV)
        pOLV->HideCursor();

    pView->SetHitTolerancePixel( HITPIX );

    FuConstruct::Deactivate();
}


/*************************************************************************
|*
|* Objekt in Edit-Mode setzen
|*
\************************************************************************/

void FuText::SetInEditMode(const MouseEvent& rMEvt, BOOL bQuickDrag)
{
    SdrPageView* pPV = pView->GetPageViewPvNum(0);
    if( pTextObj && pTextObj->GetPage() == pPV->GetPage() )
    {
        pView->SetCurrentObj(OBJ_TEXT);

        if( bPermanent )
        {
            pView->SetCurrentObj(OBJ_TEXT);
            pView->SetEditMode(SDREDITMODE_CREATE);
        }
        else
            pView->SetEditMode(SDREDITMODE_EDIT);

        BOOL bEmptyOutliner = FALSE;

        if (!pTextObj->GetOutlinerParaObject() && pView->GetTextEditOutliner())
        {
            Outliner* pOutl = pView->GetTextEditOutliner();
            ULONG nParaAnz = pOutl->GetParagraphCount();
            Paragraph* p1stPara = pOutl->GetParagraph( 0 );

            if (nParaAnz==1 && p1stPara)
            {
                // Bei nur einem Pararaph
                if (pOutl->GetText(p1stPara).Len() == 0)
                {
                    bEmptyOutliner = TRUE;
                }
            }
        }

        if (pTextObj != pView->GetTextEditObject() || bEmptyOutliner)
        {
            UINT32 nInv = pTextObj->GetObjInventor();
            UINT16 nSdrObjKind = pTextObj->GetObjIdentifier();

            if (nInv == SdrInventor && pTextObj->HasTextEdit() &&
                (nSdrObjKind == OBJ_TEXT ||
                 nSdrObjKind == OBJ_TITLETEXT ||
                 nSdrObjKind == OBJ_OUTLINETEXT ||
                 (pTextObj->ISA(SdrTextObj) && !pTextObj->IsEmptyPresObj())))
            {
                // Neuen Outliner machen (gehoert der SdrObjEditView)
                SdrOutliner* pOutl = SdrMakeOutliner( OUTLINERMODE_OUTLINEOBJECT, pDoc );
                pOutl->SetMinDepth(0);
                pOutl->SetStyleSheetPool((SfxStyleSheetPool*) pDoc->GetStyleSheetPool());
                pOutl->SetCalcFieldValueHdl(LINK(SFX_APP(), SdModule, CalcFieldValueHdl));
                ULONG nCntrl = pOutl->GetControlWord();
                nCntrl |= EE_CNTRL_ALLOWBIGOBJS;
                nCntrl |= EE_CNTRL_URLSFXEXECUTE;
                nCntrl |= EE_CNTRL_MARKFIELDS;
                nCntrl |= EE_CNTRL_AUTOCORRECT;
                SetSpellOptions( nCntrl );

                pOutl->SetControlWord(nCntrl);

                uno::Reference< linguistic::XSpellChecker1 > xSpellChecker( SvxGetSpellChecker() );
                if ( xSpellChecker.is() )
                    pOutl->SetSpeller(xSpellChecker);

                uno::Reference< linguistic::XHyphenator > xHyphenator( OFF_APP()->GetHyphenator() );
                if( xHyphenator.is() )
                    pOutl->SetHyphenator( xHyphenator );

                pOutl->SetDefaultLanguage( pDoc->GetLanguage() );

                // in einem Gliederungstext darf nicht auf die 0-te
                // Ebene ausgerueckt werden
                if (pTextObj->GetObjInventor() == SdrInventor &&
                    pTextObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
                {
                    pOutl->SetMinDepth(1);
                }

                if (bEmptyOutliner)
                {
                    pView->EndTextEdit(TRUE);
                }

                FASTBOOL bNewObj = TRUE;
                if (pView->BegTextEdit(pTextObj, pPV, pWindow, bNewObj, pOutl) &&
                    pTextObj->GetObjInventor() == SdrInventor)
                {
                    bFirstObjCreated = TRUE;
                    DeleteDefaultText();

                    OutlinerView* pOLV = pView->GetTextEditOutlinerView();

                    UINT16 nSdrObjKind = pTextObj->GetObjIdentifier();

                    SdrViewEvent aVEvt;
                    SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                    if (eHit == SDRHIT_TEXTEDIT)
                    {
                        // Text getroffen
                        if (nSdrObjKind == OBJ_TEXT ||
                            nSdrObjKind == OBJ_TITLETEXT ||
                            nSdrObjKind == OBJ_OUTLINETEXT ||
                            nSlotId == SID_TEXTEDIT ||
                            !bQuickDrag)
                        {
                            pOLV->MouseButtonDown(rMEvt);
                            pOLV->MouseMove(rMEvt);
                            pOLV->MouseButtonUp(rMEvt);
                        }

                        if (pViewShell->GetFrameView()->IsQuickEdit() &&
                            bQuickDrag && pTextObj->GetOutlinerParaObject())
                        {
                            pOLV->MouseButtonDown(rMEvt);
                        }
                    }
                }
                else
                {
                    RestoreDefaultText();
                }
            }
        }
    }
    else
        pTextObj = NULL;
}

/*************************************************************************
|*
|* Texteingabe wird beendet, ggf. Default-Text setzen
|*
\************************************************************************/

BOOL FuText::RestoreDefaultText()
{
    BOOL bRestored = FALSE;

    if ( pTextObj && !pTextObj->HasText() && (pTextObj == pView->GetTextEditObject()) )
    {
        SdPage* pPage = (SdPage*) pTextObj->GetPage();

        if (pPage)
        {
            PresObjKind ePresObjKind = pPage->GetPresObjKind(pTextObj);

            if (ePresObjKind == PRESOBJ_TITLE   ||
                ePresObjKind == PRESOBJ_OUTLINE ||
                ePresObjKind == PRESOBJ_NOTES   ||
                ePresObjKind == PRESOBJ_TEXT)
            {
                String aString = pPage->GetPresObjText(ePresObjKind);

                if (aString.Len())
                {
                    SdOutliner* pInternalOutl = pDoc->GetInternalOutliner();
                    pInternalOutl->SetMinDepth(0);
                    pPage->SetObjText( pTextObj, pInternalOutl, ePresObjKind, aString );

                    SdrOutliner* pOutliner = pView->GetTextEditOutliner();
                    pTextObj->SetTextEditOutliner( NULL );  // to make stylesheet settings work
                    pTextObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj(ePresObjKind), TRUE );
                    pTextObj->SetTextEditOutliner( pOutliner );

                    pInternalOutl->Clear();
                    OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();

                    if (pOutliner)
                        pOutliner->SetText(*pParaObj);

                    pTextObj->SetEmptyPresObj(TRUE);
                    bRestored = TRUE;
                }
            }
        }
    }

    return(bRestored);
}

/*************************************************************************
|*
|* Texteingabe wird gestartet, ggf. Default-Text loeschen
|*
\************************************************************************/

BOOL FuText::DeleteDefaultText()
{
    BOOL bDeleted = FALSE;

    if ( pTextObj && pTextObj->IsEmptyPresObj() )
    {
        String aString;
        SdPage* pPage = (SdPage*) pTextObj->GetPage();

        if (pPage)
        {
            PresObjKind ePresObjKind = pPage->GetPresObjKind(pTextObj);

            if ( (ePresObjKind == PRESOBJ_TITLE   ||
                  ePresObjKind == PRESOBJ_OUTLINE ||
                  ePresObjKind == PRESOBJ_NOTES   ||
                  ePresObjKind == PRESOBJ_TEXT) &&
                  !pPage->IsMasterPage() )
            {
                Outliner* pOutliner = pView->GetTextEditOutliner();
                SfxStyleSheet* pSheet = pOutliner->GetStyleSheet( 0 );
                pOutliner->SetText( String(), pOutliner->GetParagraph( 0 ) );

                if (pSheet &&
                    (ePresObjKind == PRESOBJ_NOTES || ePresObjKind == PRESOBJ_TEXT))
                    pOutliner->SetStyleSheet(0, pSheet);

                pTextObj->SetEmptyPresObj(TRUE);
                bDeleted = TRUE;
            }
        }
    }

    return(bDeleted);
}

/*************************************************************************
|*
|* Objekt wurde veraendert
|*
\************************************************************************/

void FuText::ObjectChanged()
{
    if (pTextObj)
        pTextObj->SetEmptyPresObj(FALSE);
}

/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

BOOL FuText::Command(const CommandEvent& rCEvt)
{
    return( FuPoor::Command(rCEvt) );
}

/*************************************************************************
|*
|* Help-event
|*
\************************************************************************/

BOOL FuText::RequestHelp(const HelpEvent& rHEvt)
{
    BOOL bReturn = FALSE;

    OutlinerView* pOLV = pView->GetTextEditOutlinerView();

    if ((Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled()) &&
        pTextObj && pOLV && pOLV->GetFieldUnderMousePointer())
    {
        String aHelpText;
        const SvxFieldItem* pFieldItem = pOLV->GetFieldUnderMousePointer();
        const SvxFieldData* pField = pFieldItem->GetField();

        if (pField && pField->ISA(SvxURLField))
        {
            /******************************************************************
            * URL-Field
            ******************************************************************/
            aHelpText = ((const SvxURLField*) pField)->GetURL();
        }

        if (aHelpText.Len())
        {
            Rectangle aLogicPix = pWindow->LogicToPixel(pTextObj->GetLogicRect());
            Rectangle aScreenRect(pWindow->OutputToScreenPixel(aLogicPix.TopLeft()),
                                  pWindow->OutputToScreenPixel(aLogicPix.BottomRight()));

#ifdef OS2
            aScreenRect = Rectangle(rHEvt.GetMousePosPixel(), Size(5, 5));
#endif

            if (Help::IsBalloonHelpEnabled())
            {
                bReturn = Help::ShowBalloon( (Window*)pWindow, rHEvt.GetMousePosPixel(), aScreenRect, aHelpText);
            }
            else if (Help::IsQuickHelpEnabled())
            {
                bReturn = Help::ShowQuickHelp( (Window*)pWindow, aScreenRect, aHelpText);
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

    SdrViewEvent aVEvt;

    if (nSlotId == SID_TEXTEDIT || pViewShell->GetFrameView()->IsQuickEdit())
    {
        MouseEvent aMEvt(pWindow->GetPointerPosPixel());

        pTextObj = NULL;

        if (nSlotId == SID_TEXTEDIT)
        {
            // Wird gerade editiert?
            if(!bTestText)
                pTextObj = (SdrTextObj*) pView->GetTextEditObject();

            if (!pTextObj)
            {
                // Versuchen, ein Obj zu selektieren
                SdrPageView* pPV = pView->GetPageViewPvNum(0);
                SdrViewEvent aVEvt;
                SdrHitKind eHit = pView->PickAnything(aMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
                pView->MarkObj(aVEvt.pRootObj, pPV);

                if (aVEvt.pObj && aVEvt.pObj->ISA(SdrTextObj))
                {
                    pTextObj = (SdrTextObj*) aVEvt.pObj;
                }
            }
        }
        else if (pView->HasMarkedObj())
        {
            const SdrMarkList& rMarkList = pView->GetMarkList();

            if (rMarkList.GetMarkCount() == 1)
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetObj();

                if (pObj->ISA(SdrTextObj))
                {
                    pTextObj = (SdrTextObj*) pObj;
                }
            }
        }

        BOOL bQuickDrag = TRUE;

        const SfxItemSet* pArgs = rReq.GetArgs();

        if (pArgs &&
            (UINT16) ((SfxUInt16Item&) pArgs->Get(SID_TEXTEDIT)).GetValue() == 2)
        {
            // Anwahl per Doppelklick -> kein QuickDrag zulassen
            bQuickDrag = FALSE;
        }

        SetInEditMode(aMEvt, bQuickDrag);
    }
}



/*************************************************************************
|*
|* SpellChecker: Error-LinkHdl
|*
\************************************************************************/

IMPL_LINK( FuText, SpellError, void *, nLang )
{
    String aError( ::GetLanguageString( (LanguageType)(ULONG)nLang ) );
    ErrorHandler::HandleError(* new StringErrorInfo(
                                ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aError) );
    return 0;
}


/*************************************************************************
|*
|* Reaktion auf Doppelklick
|*
\************************************************************************/
void FuText::DoubleClick(const MouseEvent& rMEvt)
{
    // Nichts zu tun
}



