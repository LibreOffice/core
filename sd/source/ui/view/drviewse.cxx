/*************************************************************************
 *
 *  $RCSfile: drviewse.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dl $ $Date: 2001-03-12 07:54:26 $
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

#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HDL_
#include <com/sun/star/i18n/TransliterationModules.hdl>
#endif

#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX
#include <svx/fmshell.hxx>
#endif

#ifndef _SV_SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SVDORECT_HXX //autogen
#include <svx/svdorect.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#define ITEMID_FIELD    EE_FEATURE_FIELD
#ifndef _FLDITEM_HXX
#include <svx/flditem.hxx>
#endif
#ifndef _RULER_HXX
#include <svx/ruler.hxx>
#endif
#ifndef _OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#pragma hdrstop

#include <svtools/urihelper.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "drawdoc.hxx"
#include "fusel.hxx"
#include "futext.hxx"
#include "fuconrec.hxx"
#include "fuconuno.hxx"
#include "fuconbez.hxx"
#include "fuediglu.hxx"
#include "fuconarc.hxx"
#include "fucon3d.hxx"
#include "sdresid.hxx"
#include "fuslshow.hxx"
#include "sdoutl.hxx"
#include "drviewsh.hxx"
#include "sdpage.hxx"
#include "frmview.hxx"
#include "zoomlist.hxx"
#include "drawview.hxx"
#include "docshell.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

#ifdef WNT
#pragma optimize ( "", off )
#endif

/*************************************************************************
|*
|* Permanente Funktionen
|*
\************************************************************************/

void SdDrawViewShell::FuPermanent(SfxRequest& rReq)
{
    // Waehrend einer Native-Diashow wird nichts ausgefuehrt!
    if (pFuSlideShow && !pFuSlideShow->IsLivePresentation())
        return;

    USHORT nSId = rReq.GetSlot();

    if( pFuActual && pFuActual->ISA(FuText) &&
        ( nSId == SID_TEXTEDIT || nSId == SID_ATTR_CHAR || nSId == SID_TEXT_FITTOSIZE ||
          nSId == SID_ATTR_CHAR_VERTICAL || nSId == SID_TEXT_FITTOSIZE_VERTICAL ) )
    {
        ((FuText*) pFuActual)->SetPermanent(TRUE);
        pFuActual->ReceiveRequest( rReq );

        MapSlot( nSId );

        Invalidate();
        rReq.Done();
        return;
    }

    CheckLineTo (rReq);
    USHORT nOldSId = 0;
    BOOL bPermanent = FALSE;

    if (pFuActual)
    {
        if (pFuOld == pFuActual)
        {
            pFuOld = NULL;
        }

        if ( nSId != SID_TEXTEDIT && nSId != SID_ATTR_CHAR && nSId != SID_TEXT_FITTOSIZE &&
             nSId != SID_ATTR_CHAR_VERTICAL && nSId != SID_TEXT_FITTOSIZE_VERTICAL &&
             pDrView->IsTextEdit() )
        {
            pDrView->EndTextEdit();
        }

        nOldSId = pFuActual->GetSlotID();

        if (nOldSId == nSId ||
            ((nOldSId == SID_TEXTEDIT || nOldSId == SID_ATTR_CHAR || nOldSId == SID_TEXT_FITTOSIZE ||
              nOldSId == SID_ATTR_CHAR_VERTICAL || nOldSId == SID_TEXT_FITTOSIZE_VERTICAL) &&
             (nSId == SID_TEXTEDIT || nSId == SID_ATTR_CHAR || nSId == SID_TEXT_FITTOSIZE ||
              nSId == SID_ATTR_CHAR_VERTICAL || nSId == SID_TEXT_FITTOSIZE_VERTICAL )))
        {
            bPermanent = TRUE;
        }

        pFuActual->Deactivate();
        delete pFuActual;
        pFuActual = NULL;

        SfxBindings& rBind = GetViewFrame()->GetBindings();
        rBind.Invalidate(nOldSId);
        rBind.Update(nOldSId);
    }

    // Slot wird gemapped (ToolboxImages/-Slots)
    MapSlot( nSId );

    switch ( nSId )
    {
        case SID_TEXTEDIT:  // BASIC ???
        case SID_ATTR_CHAR:
        case SID_ATTR_CHAR_VERTICAL:
        case SID_TEXT_FITTOSIZE:
        case SID_TEXT_FITTOSIZE_VERTICAL:
        {
            pFuActual = new FuText(this, pWindow, pDrView, pDoc, rReq);
            ( (FuText*) pFuActual)->DoExecute();
            // Das Setzen des Permanent-Status erfolgt weiter oben!

            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_ATTR_CHAR );
            rBindings.Invalidate( SID_ATTR_CHAR_VERTICAL );
            rBindings.Invalidate( SID_TEXT_FITTOSIZE );
            rBindings.Invalidate( SID_TEXT_FITTOSIZE_VERTICAL );
            rReq.Done();
        }
        break;

        case SID_FM_CREATE_CONTROL:
        {
            pFuActual = new FuConstUnoControl(this, pWindow, pDrView, pDoc, rReq);
            ((FuConstUnoControl*) pFuActual)->SetPermanent(bPermanent);
            rReq.Done();
        }
        break;

        case SID_OBJECT_SELECT:
        case SID_OBJECT_ROTATE:
        case SID_OBJECT_MIRROR:
        case SID_OBJECT_TRANSPARENCE:
        case SID_OBJECT_GRADIENT:
        case SID_OBJECT_SHEAR:
        case SID_OBJECT_CROOK_ROTATE:
        case SID_OBJECT_CROOK_SLANT:
        case SID_OBJECT_CROOK_STRETCH:
        case SID_CONVERT_TO_3D_LATHE:
        {
            short nSlotId = rReq.GetSlot();

            if (nSlotId == SID_OBJECT_CROOK_ROTATE ||
                nSlotId == SID_OBJECT_CROOK_SLANT ||
                nSlotId == SID_OBJECT_CROOK_STRETCH)
            {
                if ( pDrView->GetMarkList().GetMarkCount() > 0 &&
                    !pDrView->IsCrookAllowed( pDrView->IsCrookNoContortion() ) )
                {
                    if ( pDrView->IsPresObjSelected() )
                    {
                        InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
                    }
                    else if ( QueryBox(pWindow, WB_YES_NO,
                                      String(SdResId(STR_ASK_FOR_CONVERT_TO_BEZIER) )
                                      ).Execute() == RET_YES )
                    {
                        // Implizite Wandlung in Bezier
                        WaitObject aWait( (Window*)GetActiveWindow() );
                        pDrView->ConvertMarkedToPathObj(FALSE);
                    }
                }
            }
            else if (nSlotId == SID_OBJECT_SHEAR)
            {
                ULONG i = 0;
                const SdrMarkList& rMarkList = pDrView->GetMarkList();
                ULONG nMarkCnt = rMarkList.GetMarkCount();
                BOOL b3DObjMarked = FALSE;

                while (i < nMarkCnt && !b3DObjMarked)
                {
                    if (rMarkList.GetMark(i)->GetObj()->ISA(E3dObject))
                    {
                        b3DObjMarked = TRUE;
                    }
                    else
                    {
                        i++;
                    }
                }

                if ( nMarkCnt > 0 && !b3DObjMarked &&
                     (!pDrView->IsShearAllowed() || !pDrView->IsDistortAllowed()) )
                {
                    if ( pDrView->IsPresObjSelected() )
                    {
                        InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
                    }
                    else if ( QueryBox(pWindow, WB_YES_NO,
                                      String(SdResId(STR_ASK_FOR_CONVERT_TO_BEZIER) )
                                      ).Execute() == RET_YES )
                    {
                        // Implizite Wandlung in Bezier
                        WaitObject aWait( (Window*)GetActiveWindow() );
                        pDrView->ConvertMarkedToPathObj(FALSE);
                    }
                }
            }

            pFuActual = new FuSelection(this, pWindow, pDrView,
                                              pDoc, rReq);

            rReq.Done();
            Invalidate( SID_OBJECT_SELECT );
        }
        break;

        case SID_DRAW_LINE:
        case SID_DRAW_XLINE:
        case SID_DRAW_MEASURELINE:
        case SID_LINE_ARROW_START:
        case SID_LINE_ARROW_END:
        case SID_LINE_ARROWS:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_ARROW_SQUARE:
        case SID_LINE_SQUARE_ARROW:

        case SID_DRAW_RECT:
        case SID_DRAW_RECT_NOFILL:
        case SID_DRAW_RECT_ROUND:
        case SID_DRAW_RECT_ROUND_NOFILL:
        case SID_DRAW_SQUARE:
        case SID_DRAW_SQUARE_NOFILL:
        case SID_DRAW_SQUARE_ROUND:
        case SID_DRAW_SQUARE_ROUND_NOFILL:
        case SID_DRAW_ELLIPSE:
        case SID_DRAW_ELLIPSE_NOFILL:
        case SID_DRAW_CIRCLE:
        case SID_DRAW_CIRCLE_NOFILL:
        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
        case SID_TOOL_CONNECTOR:
        case SID_CONNECTOR_ARROW_START:
        case SID_CONNECTOR_ARROW_END:
        case SID_CONNECTOR_ARROWS:
        case SID_CONNECTOR_CIRCLE_START:
        case SID_CONNECTOR_CIRCLE_END:
        case SID_CONNECTOR_CIRCLES:
        case SID_CONNECTOR_LINE:
        case SID_CONNECTOR_LINE_ARROW_START:
        case SID_CONNECTOR_LINE_ARROW_END:
        case SID_CONNECTOR_LINE_ARROWS:
        case SID_CONNECTOR_LINE_CIRCLE_START:
        case SID_CONNECTOR_LINE_CIRCLE_END:
        case SID_CONNECTOR_LINE_CIRCLES:
        case SID_CONNECTOR_CURVE:
        case SID_CONNECTOR_CURVE_ARROW_START:
        case SID_CONNECTOR_CURVE_ARROW_END:
        case SID_CONNECTOR_CURVE_ARROWS:
        case SID_CONNECTOR_CURVE_CIRCLE_START:
        case SID_CONNECTOR_CURVE_CIRCLE_END:
        case SID_CONNECTOR_CURVE_CIRCLES:
        case SID_CONNECTOR_LINES:
        case SID_CONNECTOR_LINES_ARROW_START:
        case SID_CONNECTOR_LINES_ARROW_END:
        case SID_CONNECTOR_LINES_ARROWS:
        case SID_CONNECTOR_LINES_CIRCLE_START:
        case SID_CONNECTOR_LINES_CIRCLE_END:
        case SID_CONNECTOR_LINES_CIRCLES:
        {
            pFuActual = new FuConstRectangle(this, pWindow, pDrView,
                                                   pDoc, rReq);
            ((FuConstRectangle*) pFuActual)->SetPermanent(bPermanent);

            rReq.Done();
        }
        break;

        case SID_DRAW_POLYGON:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON:
        case SID_DRAW_XPOLYGON_NOFILL:
        case SID_DRAW_FREELINE:
        case SID_DRAW_FREELINE_NOFILL:
        case SID_DRAW_BEZIER_FILL:          // BASIC
        case SID_DRAW_BEZIER_NOFILL:        // BASIC
        {
            pFuActual = new FuConstBezPoly(this, pWindow, pDrView,
                                                 pDoc, rReq);
            ((FuConstBezPoly*) pFuActual)->SetPermanent(bPermanent);
            rReq.Done();
        }
        break;

        case SID_GLUE_EDITMODE:
        {
            if (nOldSId != SID_GLUE_EDITMODE)
            {
                pFuActual = new FuEditGluePoints( this, pWindow, pDrView, pDoc, rReq );
                ((FuEditGluePoints*) pFuActual)->SetPermanent(bPermanent);
            }
            else
            {
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }

            rReq.Done();
        }
        break;

        case SID_DRAW_ARC:
        case SID_DRAW_CIRCLEARC:
        case SID_DRAW_PIE:
        case SID_DRAW_PIE_NOFILL:
        case SID_DRAW_CIRCLEPIE:
        case SID_DRAW_CIRCLEPIE_NOFILL:
        case SID_DRAW_ELLIPSECUT:
        case SID_DRAW_ELLIPSECUT_NOFILL:
        case SID_DRAW_CIRCLECUT:
        case SID_DRAW_CIRCLECUT_NOFILL:
        {
            pFuActual = new FuConstArc( this, pWindow, pDrView,
                                              pDoc, rReq );
            ((FuConstArc*) pFuActual)->SetPermanent(bPermanent);
            rReq.Done();
        }
        break;

        case SID_3D_CUBE:
        case SID_3D_SHELL:
        case SID_3D_SPHERE:
        case SID_3D_TORUS:
        case SID_3D_HALF_SPHERE:
        case SID_3D_CYLINDER:
        case SID_3D_CONE:
        case SID_3D_PYRAMID:
        {
            pFuActual = new FuConst3dObj(this, pWindow, pDrView, pDoc, rReq);
            ((FuConst3dObj*) pFuActual)->SetPermanent(bPermanent);
            rReq.Done();
        }
        break;

        default:
        break;
    }

    if (pFuOld)
    {
        USHORT nSId = pFuOld->GetSlotID();

        pFuOld->Deactivate();
        delete pFuOld;
        pFuOld = NULL;

        SfxBindings& rBind = GetViewFrame()->GetBindings();
        rBind.Invalidate( nSId );
        rBind.Update( nSId );
    }

    if (pFuActual)
    {
        pFuActual->Activate();
        pFuOld = pFuActual;

        SetHelpId( pFuActual->GetSlotID() );
    }

    // Shell wird invalidiert, schneller als einzeln (laut MI)
    // Jetzt explizit der letzte Slot incl. Update()
    Invalidate();
}

/*************************************************************************
|*
|* SfxRequests fuer Support-Funktionen
|*
\************************************************************************/

void SdDrawViewShell::FuSupport(SfxRequest& rReq)
{
    if( rReq.GetSlot() == SID_STYLE_FAMILY && rReq.GetArgs())
        pDocSh->SetStyleFamily(((SfxUInt16Item&)rReq.GetArgs()->Get( SID_STYLE_FAMILY )).GetValue());

    // Waehrend einer Native-Diashow wird nichts ausgefuehrt!
    if (pFuSlideShow && !pFuSlideShow->IsLivePresentation() &&
        (rReq.GetSlot() != SID_PRESENTATION_END &&
         rReq.GetSlot() != SID_LIVE_PRESENTATION &&
         rReq.GetSlot() != SID_SIZE_PAGE))
        return;

    CheckLineTo (rReq);

    USHORT nSId = rReq.GetSlot();

    // Slot wird evtl. gemapped (ToolboxImages/-Slots)
    MapSlot( nSId );

    switch ( nSId )
    {
        // Slots der ToolboxController gemapped ausfuehren
        case SID_OBJECT_CHOOSE_MODE:
        case SID_POSITION:
        case SID_OBJECT_ALIGN:
        case SID_ZOOM_TOOLBOX:
        case SID_DRAWTBX_TEXT:
        case SID_DRAWTBX_RECTANGLES:
        case SID_DRAWTBX_ELLIPSES:
        case SID_DRAWTBX_LINES:
        case SID_DRAWTBX_ARROWS:
        case SID_DRAWTBX_3D_OBJECTS:
        case SID_DRAWTBX_CONNECTORS:
        case SID_DRAWTBX_INSERT:
        {
            USHORT nMappedSlot = GetMappedSlot( nSId );
            if( nMappedSlot > 0 )
            {
                SfxRequest aReq( nMappedSlot, 0, pDoc->GetItemPool() );
                ExecuteSlot( aReq );
            }
        }
        break;

        case SID_PRESENTATION:
        case SID_REHEARSE_TIMINGS:
        {
            if (!pFuSlideShow)
            {
                if (pDrView->IsTextEdit())
                {
                    pDrView->EndTextEdit();
                }

                pFuSlideShow = new FuSlideShow(this, pWindow, pDrView, pDoc, rReq);
                pFuSlideShow->StartShow();
                pFuSlideShow->Activate();
            }
            rReq.Ignore ();
        }
        break;

        case SID_BEZIER_EDIT:
        {
            pDrView->SetFrameDragSingles(!pDrView->IsFrameDragSingles());

            /******************************************************************
            * ObjectBar einschalten
            ******************************************************************/
            if (pFuActual &&
                (pFuActual->ISA(FuSelection) || pFuActual->ISA(FuConstBezPoly)))
            {
                USHORT nObjBarId = RID_DRAW_OBJ_TOOLBOX;

                if (pDrView->HasMarkablePoints())
                {
                    nObjBarId = RID_BEZIER_TOOLBOX;
                }

                SwitchObjectBar(nObjBarId);
            }

            Invalidate(SID_BEZIER_EDIT);
            rReq.Ignore();
        }
        break;

        case SID_PRESENTATION_END:
        {
            if( pFuSlideShow )
            {
                if( pDrView->IsTextEdit() )
                    pDrView->EndTextEdit();

                pFuSlideShow->Deactivate();
                pFuSlideShow->Destroy();
                pFuSlideShow = NULL;

                USHORT nPresViewShellId = pFrameView->GetPresentationViewShellId();

                if (nPresViewShellId != SID_VIEWSHELL0)
                {
                    // Die Praesentation ist aus einer anderen ViewShell
                    // angewaehlt worden, zu dieser wird nun zurueckgekehrt
                    pFrameView->SetPresentationViewShellId(SID_VIEWSHELL0);
                    pFrameView->SetSlotId(SID_OBJECT_SELECT);
                    GetViewFrame()->GetDispatcher()->Execute(nPresViewShellId,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                }
            }
            rReq.Ignore ();

            // sonst bleiben alle Draw-Slots disabled
            GetViewFrame()->GetBindings().InvalidateAll( TRUE );
        }
        break;

        case SID_LIVE_PRESENTATION:
        {
            if( pFuSlideShow && !pFuSlideShow->IsInputLocked() )
            {
                if (pDrView->IsTextEdit())
                {
                    pDrView->EndTextEdit();
                }

                pFuSlideShow->ReceiveRequest(rReq);
            }
            rReq.Ignore ();
        }
        break;

        case SID_OBJECT_CLOSE:
        {
            const SdrMarkList& rMarkList = pDrView->GetMarkList();
            if ( rMarkList.GetMark(0) && !pDrView->IsAction() )
            {
                SdrPathObj* pPathObj = (SdrPathObj*) rMarkList.GetMark(0)->GetObj();
                pDrView->BegUndo(String(SdResId(STR_UNDO_BEZCLOSE)));
                pDrView->UnmarkAllPoints();
                Size aDist(pWindow->PixelToLogic(Size(8,8)));
                pDrView->AddUndo(new SdrUndoGeoObj(*pPathObj));
                pPathObj->ToggleClosed(aDist.Width());
                pDrView->EndUndo();
            }
            rReq.Done();
        }
        break;

        case SID_CUT:
        {
            if ( pDrView->IsPresObjSelected(FALSE, TRUE) )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                if (pFuActual)      pFuActual->DoCut();
                else if (pDrView)   pDrView->DoCut();
            }
            rReq.Ignore ();
        }
        break;

        case SID_COPY:
        {
            if ( pDrView->IsPresObjSelected(FALSE, TRUE) )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                if (pFuActual)
                    pFuActual->DoCopy();
                else
                    pDrView->DoCopy();
            }
            rReq.Ignore ();
        }
        break;

        case SID_PASTE:
        {
            WaitObject aWait( (Window*)GetActiveWindow() );

            if (pFuActual)      pFuActual->DoPaste();
            else if (pDrView)   pDrView->DoPaste();

            rReq.Ignore ();
        }
        break;

        case SID_DELETE:
        {
            if ( pDrView->IsTextEdit() )
            {
                OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();

                if (pOLV)
                {
                    KeyCode aKCode(KEY_DELETE);
                    KeyEvent aKEvt( 0, aKCode);
                    pOLV->PostKeyEvent(aKEvt);
                }
            }
            else if ( pDrView->IsPresObjSelected(FALSE, TRUE) )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else if (pFuActual)
            {
                KeyCode aKCode(KEY_DELETE);
                KeyEvent aKEvt( 0, aKCode);

                if ( !pFuActual->KeyInput(aKEvt) )
                    pDrView->DeleteMarked();
            }
            else
            {
                pDrView->DeleteMarked();
            }
            rReq.Ignore ();
        }
        break;

       case SID_PIXELMODE:
       {
            const SfxItemSet* pReqArgs = rReq.GetArgs();
            BOOL  bPixelMode = pDrView->IsPixelMode();

            if (pReqArgs)
            {
                SFX_REQUEST_ARG(rReq, pIsActive, SfxBoolItem, SID_PIXELMODE, FALSE);
                bPixelMode = pIsActive->GetValue();
            }

            pDrView->SetPixelMode(!bPixelMode);

            Invalidate(SID_PIXELMODE);
       }
       break;

       case SID_ANIMATIONMODE:
       {
            const SfxItemSet* pReqArgs = rReq.GetArgs();
            BOOL  bAnimationMode = FALSE;

            if (pDrView->GetSlideShow())
            {
                bAnimationMode = TRUE;
            }

            if (pReqArgs)
            {
                SFX_REQUEST_ARG(rReq, pIsActive, SfxBoolItem, SID_ANIMATIONMODE, FALSE);
                bAnimationMode = pIsActive->GetValue();
            }

            pDrView->SetAnimationMode(!bAnimationMode);

            Invalidate(SID_ANIMATIONMODE);
       }
       break;

       case SID_ACTIONMODE:
       {
            const SfxItemSet* pReqArgs = rReq.GetArgs();
            BOOL  bActionMode = FALSE;

            if (pDrView->IsActionMode())
            {
                bActionMode = TRUE;
            }

            if (pReqArgs)
            {
                SFX_REQUEST_ARG(rReq, pIsActive, SfxBoolItem, SID_ACTIONMODE, FALSE);
                bActionMode = pIsActive->GetValue();
            }

            pDrView->SetActionMode(!bActionMode);

            Invalidate(SID_ACTIONMODE);
       }
       break;

       case SID_DRAWINGMODE:  // BASIC
       {
            const SfxItemSet* pReqArgs = rReq.GetArgs();
            BOOL  bIsActive = TRUE;

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, SID_DRAWINGMODE, FALSE);
                bIsActive = pIsActive->GetValue ();
            }

            if ((ePageKind != PK_STANDARD) && bIsActive)
            {
                pFrameView->SetPageKind(PK_STANDARD);
                GetViewFrame()->GetDispatcher()->Execute (SID_VIEWSHELL0,
                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }

            Invalidate ();
            rReq.Ignore ();
        }
        break;

        case SID_NOTESMODE:  // BASIC
        {
            // AutoLayouts muessen fertig sein
            pDoc->StopWorkStartupDelay();

            // Effekte abschalten
            pDrView->SetAnimationMode(FALSE);

            const SfxItemSet* pReqArgs = rReq.GetArgs();
            BOOL  bIsActive = TRUE;

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, SID_NOTESMODE, FALSE);
                bIsActive = pIsActive->GetValue ();
            }

            if ((ePageKind != PK_NOTES) && bIsActive)
            {
                pFrameView->SetPageKind(PK_NOTES);
                GetViewFrame()->GetDispatcher()->Execute (SID_VIEWSHELL0,
                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }

            Invalidate ();
            rReq.Ignore ();
        }
        break;

        case SID_HANDOUTMODE:  // BASIC
        {
            // AutoLayouts muessen fertig sein
            pDoc->StopWorkStartupDelay();

            // Effekte abschalten
            pDrView->SetAnimationMode(FALSE);

            const SfxItemSet* pReqArgs = rReq.GetArgs();
            BOOL  bIsActive = TRUE;

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, SID_HANDOUTMODE, FALSE);
                bIsActive = pIsActive->GetValue ();
            }

            if ((ePageKind != PK_HANDOUT) && bIsActive)
            {
                pFrameView->SetPageKind(PK_HANDOUT);
                GetViewFrame()->GetDispatcher()->Execute (SID_VIEWSHELL0,
                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }

            Invalidate ();
            rReq.Ignore ();
        }
        break;

        case SID_DIAMODE:  // BASIC
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, SID_DIAMODE, FALSE);
                if( pIsActive && pIsActive->GetValue ())
                    GetViewFrame()->GetDispatcher()->Execute (SID_VIEWSHELL1,
                                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
            else
                GetViewFrame()->GetDispatcher()->Execute (SID_VIEWSHELL1,
                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

            Invalidate ();
            rReq.Ignore ();
        }
        break;

        case SID_OUTLINEMODE:  // BASIC
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, SID_OUTLINEMODE, FALSE);
                if (pIsActive->GetValue ()) GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL2,
                                                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
            else GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL2,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

            Invalidate ();
            rReq.Ignore ();
        }
        break;

        case SID_MASTERPAGE:          // BASIC
        case SID_SLIDE_MASTERPAGE:    // BASIC
        case SID_TITLE_MASTERPAGE:    // BASIC
        case SID_NOTES_MASTERPAGE:    // BASIC
        case SID_HANDOUT_MASTERPAGE:  // BASIC
        {
            // AutoLayouts muessen fertig sein
            pDoc->StopWorkStartupDelay();

            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, SID_MASTERPAGE, FALSE);
                bLayerMode = pIsActive->GetValue ();
            }

            if (nSId == SID_MASTERPAGE                                       ||
                (nSId == SID_SLIDE_MASTERPAGE   && ePageKind == PK_STANDARD) ||
                (nSId == SID_TITLE_MASTERPAGE   && ePageKind == PK_STANDARD) ||
                (nSId == SID_NOTES_MASTERPAGE   && ePageKind == PK_NOTES)    ||
                (nSId == SID_HANDOUT_MASTERPAGE && ePageKind == PK_HANDOUT))
            {
                if (nSId == SID_TITLE_MASTERPAGE ||
                    nSId == SID_SLIDE_MASTERPAGE)
                {
                    // Gibt es eine Seite mit dem AutoLayout "Titel"?
                    BOOL bFound = FALSE;
                    USHORT i = 0;
                    USHORT nCount = pDoc->GetSdPageCount(PK_STANDARD);

                    while (i < nCount && !bFound)
                    {
                        SdPage* pPage = pDoc->GetSdPage(i, PK_STANDARD);

                        if (nSId == SID_TITLE_MASTERPAGE && pPage->GetAutoLayout() == AUTOLAYOUT_TITLE)
                        {
                            bFound = TRUE;
                            SwitchPage((pPage->GetPageNum() - 1) / 2);
                        }
                        else if (nSId == SID_SLIDE_MASTERPAGE && pPage->GetAutoLayout() != AUTOLAYOUT_TITLE)
                        {
                            bFound = TRUE;
                            SwitchPage((pPage->GetPageNum() - 1) / 2);
                        }

                        i++;
                    }
                }

                // Default-Layer der MasterPage einschalten
                pDrView->SetActiveLayer( String( SdResId(STR_LAYER_BCKGRNDOBJ) ) );

                ChangeEditMode(EM_MASTERPAGE, bLayerMode);

                if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT)
                    GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }
            else
            {
                // Gewuenschte ViewShell einschalten

                if (nSId == SID_SLIDE_MASTERPAGE || nSId == SID_TITLE_MASTERPAGE)
                {
                    pFrameView->SetPageKind(PK_STANDARD);
                }
                else if (nSId == SID_NOTES_MASTERPAGE)
                {
                    pFrameView->SetPageKind(PK_NOTES);
                }
                else if (nSId == SID_HANDOUT_MASTERPAGE)
                {
                    pFrameView->SetPageKind(PK_HANDOUT);
                }

                pFrameView->SetViewShEditMode(EM_MASTERPAGE, pFrameView->GetPageKind());
                pFrameView->SetLayerMode(bLayerMode);
                GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL0,
                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }

            InvalidateWindows();
            Invalidate();

            rReq.Done();
        }
        break;

        case SID_RULER:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, ID_VAL_ISACTIVE, FALSE);
                SetRuler (pIsActive->GetValue ());
            }
            else SetRuler (!HasRuler());

            Invalidate (SID_RULER);
            rReq.Done ();
        }
        break;

        case SID_SIZE_PAGE:
        case SID_SIZE_PAGE_WIDTH:  // BASIC
        {
            bZoomOnPage = ( rReq.GetSlot() == SID_SIZE_PAGE );

            SdrPageView* pPageView = pDrView->GetPageViewPvNum(0);

            if ( pPageView )
            {
                Point aPagePos = pPageView->GetOffset();
                Size aPageSize = pPageView->GetPage()->GetSize();

                aPagePos.X() += aPageSize.Width()  / 2;
                aPageSize.Width() = (long) (aPageSize.Width() * 1.03);

                if( rReq.GetSlot() == SID_SIZE_PAGE )
                {
                    aPagePos.Y() += aPageSize.Height() / 2;
                    aPageSize.Height() = (long) (aPageSize.Height() * 1.03);
                    aPagePos.Y() -= aPageSize.Height() / 2;
                }
                else
                {
                    Point aPt = pWindow->PixelToLogic( Point( 0, pWindow->GetSizePixel().Height() / 2 ) );
                    aPagePos.Y() += aPt.Y();
                    aPageSize.Height() = 2;
                }

                aPagePos.X() -= aPageSize.Width()  / 2;

                SetZoomRect( Rectangle( aPagePos, aPageSize ) );

                Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0),
                                              pWindow->GetOutputSizePixel()) );
                pZoomList->InsertZoomRect(aVisAreaWin);
            }
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Done ();
        }
        break;

        case SID_SIZE_REAL:  // BASIC
        {
            bZoomOnPage = FALSE;
            SetZoom( 100 );
            Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0),
                                              pWindow->GetOutputSizePixel()) );
            pZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Done ();
        }
        break;

        case SID_ZOOM_IN:  // BASIC
        {
            bZoomOnPage = FALSE;
            SetZoom( Max( (long) ( pWindow->GetZoom() / 2 ), (long) pWindow->GetMinZoom() ) );
            Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0),
                                              pWindow->GetOutputSizePixel()) );
            pZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Done ();
        }
        break;

        case SID_SIZE_VISAREA:
        {
            Rectangle aVisArea = pFrameView->GetVisArea();
            Size aVisAreaSize = aVisArea.GetSize();

            if (aVisAreaSize.Height()!=0 && aVisAreaSize.Width()!=0)
            {
                bZoomOnPage = FALSE;
                SetZoomRect(aVisArea);
                Invalidate( SID_ZOOM_IN );
                Invalidate( SID_ZOOM_OUT );
                Invalidate( SID_ZOOM_PANNING );
                Invalidate( SID_ZOOM_TOOLBOX );
            }
            rReq.Done ();
        }
        break;

        // Namensverwirrung: SID_SIZE_OPTIMAL -> Zoom auf selektierte Objekte
        // --> Wird als Objektzoom im Programm angeboten
        case SID_SIZE_OPTIMAL:  // BASIC
        {
            bZoomOnPage = FALSE;
            if ( pDrView->HasMarkedObj() )
            {
                long nW = (long) (aMarkRect.GetWidth()  * 1.03);
                long nH = (long) (aMarkRect.GetHeight() * 1.03);
                Point aPos = aMarkRect.Center();
                aPos.X() -= nW / 2;
                aPos.Y() -= nH / 2;
                if ( nW && nH )
                {
                    SetZoomRect(Rectangle(aPos, Size(nW, nH)));

                    Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0),
                                              pWindow->GetOutputSizePixel()) );
                    pZoomList->InsertZoomRect(aVisAreaWin);
                }
            }
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Done ();
        }
        break;

        // Namensverwirrung: SID_SIZE_ALL -> Zoom auf alle Objekte
        // --> Wird als Optimal im Programm angeboten
        case SID_SIZE_ALL:  // BASIC
        {
            bZoomOnPage = FALSE;
            SdrPageView* pPageView = pDrView->GetPageViewPvNum( 0 );

            if( pPageView )
            {
                Rectangle aBoundRect( pPageView->GetObjList()->GetAllObjBoundRect() );

                long nW = (long) (aBoundRect.GetWidth() * 1.03);
                long nH = (long) (aBoundRect.GetHeight() * 1.03);
                Point aPos = aBoundRect.Center();
                aPos.X() -= nW / 2;
                aPos.Y() -= nH / 2;
                if ( nW && nH )
                {
                    SetZoomRect( Rectangle( aPos, Size( nW, nH ) ) );

                    Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0),
                                              pWindow->GetOutputSizePixel()) );
                    pZoomList->InsertZoomRect(aVisAreaWin);
                }

                Invalidate( SID_ZOOM_IN );
                Invalidate( SID_ZOOM_OUT );
                Invalidate( SID_ZOOM_PANNING );
                Invalidate( SID_ZOOM_TOOLBOX );
            }
            rReq.Done ();
        }
        break;

        case SID_ZOOM_PREV:
        {
            if (pDrView->IsTextEdit())
            {
                pDrView->EndTextEdit();
            }

            if (pZoomList->IsPreviousPossible())
            {
                // Vorheriges ZoomRect einstellen
                SetZoomRect(pZoomList->GetPreviousZoomRect());
            }
            rReq.Done ();
            Invalidate( SID_ZOOM_TOOLBOX );
        }
        break;

        case SID_ZOOM_NEXT:
        {
            if (pDrView->IsTextEdit())
            {
                pDrView->EndTextEdit();
            }

            if (pZoomList->IsNextPossible())
            {
                // Naechstes ZoomRect einstellen
                SetZoomRect(pZoomList->GetNextZoomRect());
            }
            rReq.Done ();
            Invalidate( SID_ZOOM_TOOLBOX );
        }
        break;

        case SID_GLUE_INSERT_POINT:
        case SID_GLUE_PERCENT:
        case SID_GLUE_ESCDIR:
        case SID_GLUE_ESCDIR_LEFT:
        case SID_GLUE_ESCDIR_RIGHT:
        case SID_GLUE_ESCDIR_TOP:
        case SID_GLUE_ESCDIR_BOTTOM:
        case SID_GLUE_HORZALIGN_CENTER:
        case SID_GLUE_HORZALIGN_LEFT:
        case SID_GLUE_HORZALIGN_RIGHT:
        case SID_GLUE_VERTALIGN_CENTER:
        case SID_GLUE_VERTALIGN_TOP:
        case SID_GLUE_VERTALIGN_BOTTOM:
        {
            if (pFuActual && pFuActual->ISA(FuEditGluePoints))
            {
                ((FuEditGluePoints*) pFuActual)->ReceiveRequest(rReq);
            }

            rReq.Done();
        }
        break;

#ifndef PRODUCT
        case SID_SHOW_ITEMBROWSER:
        {
            pDrView->ShowItemBrowser( !pDrView->IsItemBrowserVisible() );

            rReq.Done ();
        }
        break;
#endif

        case SID_AUTOSPELL_CHECK:
        {
            BOOL bOnlineSpell = !pDoc->GetOnlineSpell();
            pDoc->SetOnlineSpell(bOnlineSpell);

            Outliner* pOL = pDrView->GetTextEditOutliner();

            if (pOL)
            {
                ULONG nCntrl = pOL->GetControlWord();

                if (bOnlineSpell)
                    nCntrl |= EE_CNTRL_ONLINESPELLING;
                else
                    nCntrl &= ~EE_CNTRL_ONLINESPELLING;

                pOL->SetControlWord(nCntrl);
            }

            pWindow->Invalidate();
            rReq.Done ();
        }
        break;

        case SID_CONVERT_TO_1BIT_THRESHOLD:
        case SID_CONVERT_TO_1BIT_MATRIX:
        case SID_CONVERT_TO_4BIT_GRAYS:
        case SID_CONVERT_TO_4BIT_COLORS:
        case SID_CONVERT_TO_8BIT_GRAYS:
        case SID_CONVERT_TO_8BIT_COLORS:
        case SID_CONVERT_TO_24BIT:
        {
            BmpConversion eBmpConvert = BMP_CONVERSION_NONE;

            switch( nSId )
            {
                case SID_CONVERT_TO_1BIT_THRESHOLD:
                    eBmpConvert = BMP_CONVERSION_1BIT_THRESHOLD;
                    break;

                case SID_CONVERT_TO_1BIT_MATRIX:
                    eBmpConvert = BMP_CONVERSION_1BIT_MATRIX;
                    break;

                case SID_CONVERT_TO_4BIT_GRAYS:
                    eBmpConvert = BMP_CONVERSION_4BIT_GREYS;
                    break;

                case SID_CONVERT_TO_4BIT_COLORS:
                    eBmpConvert = BMP_CONVERSION_4BIT_COLORS;
                    break;

                case SID_CONVERT_TO_8BIT_GRAYS:
                    eBmpConvert = BMP_CONVERSION_8BIT_GREYS;
                    break;

                case SID_CONVERT_TO_8BIT_COLORS:
                    eBmpConvert = BMP_CONVERSION_8BIT_COLORS;
                    break;

                case SID_CONVERT_TO_24BIT:
                    eBmpConvert = BMP_CONVERSION_24BIT;
                    break;
            }

            pDrView->BegUndo(String(SdResId(STR_UNDO_COLORRESOLUTION)));
            const SdrMarkList& rMarkList = pDrView->GetMarkList();

            for (ULONG i=0; i<rMarkList.GetMarkCount(); i++)
            {
                SdrObject* pObj = rMarkList.GetMark(i)->GetObj();

                if (pObj->GetObjInventor() == SdrInventor)
                {
                    if (pObj->GetObjIdentifier() == OBJ_GRAF && !((SdrGrafObj*) pObj)->IsLinkedGraphic())
                    {
                        const Graphic& rGraphic = ((SdrGrafObj*) pObj)->GetGraphic();

                        if( rGraphic.GetType() == GRAPHIC_BITMAP )
                        {
                            SdrGrafObj* pNewObj = (SdrGrafObj*) pObj->Clone();

                            if( rGraphic.IsAnimated() )
                            {
                                Animation aAnim( rGraphic.GetAnimation() );
                                aAnim.Convert( eBmpConvert );
                                pNewObj->SetGraphic( aAnim );
                            }
                            else
                            {
                                BitmapEx aBmpEx( rGraphic.GetBitmapEx() );
                                aBmpEx.Convert( eBmpConvert );
                                pNewObj->SetGraphic( aBmpEx );
                            }

                            pDrView->ReplaceObject( pObj, *pDrView->GetPageViewPvNum(0), pNewObj );
                        }
                    }
                }
            }

            pDrView->EndUndo();
            rReq.Done ();
        }
        break;

        case SID_TRANSLITERATE_UPPER:
        case SID_TRANSLITERATE_LOWER:
        case SID_TRANSLITERATE_HALFWIDTH:
        case SID_TRANSLITERATE_FULLWIDTH:
        case SID_TRANSLITERATE_HIRAGANA:
        case SID_TRANSLITERATE_KATAGANA:
        {
            OutlinerView* pOLV = pView->GetTextEditOutlinerView();
            if( pOLV )
            {
                using namespace ::com::sun::star::i18n;
                sal_Int32 nType = 0;

                switch( nSId )
                {
                    case SID_TRANSLITERATE_UPPER:
                        nType = TransliterationModules_LOWERCASE_UPPERCASE;
                        break;
                    case SID_TRANSLITERATE_LOWER:
                        nType = TransliterationModules_UPPERCASE_LOWERCASE;
                        break;
                    case SID_TRANSLITERATE_HALFWIDTH:
                        nType = TransliterationModules_FULLWIDTH_HALFWIDTH;
                        break;
                    case SID_TRANSLITERATE_FULLWIDTH:
                        nType = TransliterationModules_HALFWIDTH_FULLWIDTH;
                        break;
                    case SID_TRANSLITERATE_HIRAGANA:
                        nType = TransliterationModules_KATAKANA_HIRAGANA;
                        break;
                    case SID_TRANSLITERATE_KATAGANA:
                        nType = TransliterationModules_HIRAGANA_KATAKANA;
                        break;
                }

                pOLV->TransliterateText( nType );
            }

            rReq.Done();
        }
        break;

        default:
        break;
    }
}

/*************************************************************************
|*
|* URL-Feld einfuegen
|*
\************************************************************************/

void SdDrawViewShell::InsertURLField(const String& rURL, const String& rText,
                                     const String& rTarget, const Point* pPos)
{
    SvxURLField aURLField(rURL, rText, SVXURLFORMAT_REPR);
    aURLField.SetTargetFrame(rTarget);
    SvxFieldItem aURLItem(aURLField);

    OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();

    if (pOLV)
    {
        const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();

        if (pFieldItem && pFieldItem->GetField()->ISA(SvxURLField))
        {
            // Feld selektieren, so dass es beim Insert geloescht wird
            ESelection aSel = pOLV->GetSelection();
            if( aSel.nStartPos == aSel.nEndPos )
                aSel.nEndPos++;
            pOLV->SetSelection(aSel);
        }

        SvxFieldItem aURLItem(SvxURLField(rURL, rText, SVXURLFORMAT_REPR));
        pOLV->InsertField(aURLItem);
    }
    else
    {
        Outliner* pOutl = pDoc->GetInternalOutliner();
        pOutl->Init( OUTLINERMODE_TEXTOBJECT );
        USHORT nOutlMode = pOutl->GetMode();
        pOutl->QuickInsertField( aURLItem, ESelection() );
        OutlinerParaObject* pOutlParaObject = pOutl->CreateParaObject();

        SdrRectObj* pRectObj = new SdrRectObj(OBJ_TEXT);

        pOutl->UpdateFields();
        pOutl->SetUpdateMode( TRUE );
        Size aSize(pOutl->CalcTextSize());
        pOutl->SetUpdateMode( FALSE );

        Point aPos;

        if (pPos)
        {
            aPos = *pPos;
        }
        else
        {
            Rectangle aRect(aPos, pWindow->GetOutputSizePixel() );
            aPos = aRect.Center();
            aPos = pWindow->PixelToLogic(aPos);
            aPos.X() -= aSize.Width() / 2;
            aPos.Y() -= aSize.Height() / 2;
        }

        Rectangle aLogicRect(aPos, aSize);
        pRectObj->SetLogicRect(aLogicRect);
        pRectObj->SetOutlinerParaObject( pOutlParaObject );
        pActualPage->InsertObject(pRectObj);
        pOutl->Init( nOutlMode );
    }
}

/*************************************************************************
|*
|* URL-Button einfuegen
|*
\************************************************************************/

void SdDrawViewShell::InsertURLButton(const String& rURL, const String& rText,
                                      const String& rTarget, const Point* pPos)
{
    BOOL bNewObj = TRUE;

    if (pDrView->GetMarkList().GetMarkCount() > 0)
    {
        SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pDrView->GetMarkList().GetMark(0)->GetObj());

        if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor() &&
               pUnoCtrl->GetObjIdentifier() == OBJ_FM_BUTTON)
           {
               // Markiertes Objekt aendern
            bNewObj = FALSE;
            uno::Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel() );

               if( !xControlModel.is() )
                   return;

            uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );

            uno::Any aTmp;

            aTmp <<= rtl::OUString( rText );
            xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" )), aTmp );

            aTmp <<= rtl::OUString( ::URIHelper::SmartRelToAbs( rURL, FALSE,
                                                                INetURLObject::WAS_ENCODED,
                                                                INetURLObject::DECODE_UNAMBIGUOUS ) );
            xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetURL" )), aTmp );

            if( rTarget.Len() )
            {
                aTmp <<= rtl::OUString(rTarget);
                xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetFrame" )), aTmp );
            }

            form::FormButtonType eButtonType = form::FormButtonType_URL;
            aTmp <<= eButtonType;
            xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" )), aTmp );
        }
    }

    if (bNewObj)
    {
        SdrUnoObj* pUnoCtrl = (SdrUnoObj*) SdrObjFactory::MakeNewObject(FmFormInventor, OBJ_FM_BUTTON,
                                pDrView->GetPageViewPvNum(0)->GetPage(), pDoc);

        uno::Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel() );

        if( !xControlModel.is())
            return;

        uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );

        uno::Any aTmp;

        aTmp <<= rtl::OUString(rText);
        xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" )), aTmp );

        aTmp <<= rtl::OUString( ::URIHelper::SmartRelToAbs( rURL, FALSE,
                                                            INetURLObject::WAS_ENCODED,
                                                            INetURLObject::DECODE_UNAMBIGUOUS ) );
        xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetURL" )), aTmp );

        if( rTarget.Len() )
        {
            aTmp <<= rtl::OUString(rTarget);
            xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetFrame" )), aTmp );
        }

        form::FormButtonType eButtonType = form::FormButtonType_URL;
        aTmp <<= eButtonType;
        xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" )), aTmp );

        Point aPos;

        if (pPos)
        {
            aPos = *pPos;
        }
        else
        {
            aPos = Rectangle(aPos, pWindow->GetOutputSizePixel()).Center();
            aPos = pWindow->PixelToLogic(aPos);
        }

        Size aSize(4000, 1000);
        aPos.X() -= aSize.Width() / 2;
        aPos.Y() -= aSize.Height() / 2;
        pUnoCtrl->SetLogicRect(Rectangle(aPos, aSize));

        ULONG nOptions = SDRINSERT_SETDEFLAYER;

        if (GetIPClient() && GetIPClient()->IsInPlaceActive())
        {
            nOptions |= SDRINSERT_DONTMARK;
        }

        pDrView->InsertObject(pUnoCtrl, *pDrView->GetPageViewPvNum(0), nOptions);
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdDrawViewShell::PreparePresentation()
{
    if( bHasRuler )
    {
        if( pHRulerArray[0] )
            pHRulerArray[0]->Hide();

        if( pHRulerArray[1] )
            pHRulerArray[1]->Hide();

        if( pVRulerArray[0] )
            pVRulerArray[0]->Hide();

        if( pVRulerArray[1] )
            pVRulerArray[1]->Hide();
    }

    if( pVScrlArray[0] )
        pVScrlArray[0]->Hide();

    if( pVScrlArray[1] )
        pVScrlArray[1]->Hide();

    if( pHScrlArray[0] )
        pHScrlArray[0]->Hide();

    if( pHScrlArray[1] )
        pHScrlArray[1]->Hide();

    aVSplit.Hide();
    aHSplit.Hide();

    aDrawBtn.Hide();
    aSlideBtn.Hide();
    aOutlineBtn.Hide();
    aNotesBtn.Hide();
    aHandoutBtn.Hide();
    aPresentationBtn.Hide();
    aPageBtn.Hide();
    aMasterPageBtn.Hide();
    aLayerBtn.Hide();

    if( bLayerMode )
        aLayerTab.Hide();
    else
        aTabControl.Hide();

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
            if( pWinArray[nX][nY] )
                pWinArray[nX][nY]->Hide();
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void  SdDrawViewShell::EndPresentation()
{
    if( bHasRuler )
    {
        if( pHRulerArray[0] )
            pHRulerArray[0]->Show();

        if( pHRulerArray[1] )
            pHRulerArray[1]->Show();

        if( pVRulerArray[0] )
            pVRulerArray[0]->Show();

        if( pVRulerArray[1] )
            pVRulerArray[1]->Show();
    }

    if( pVScrlArray[0] )
        pVScrlArray[0]->Show();

    if( pVScrlArray[1] )
        pVScrlArray[1]->Show();

    if( pHScrlArray[0] )
        pHScrlArray[0]->Show();

    if( pHScrlArray[1] )
        pHScrlArray[1]->Show();

    aVSplit.Show();
    aHSplit.Show();

    aDrawBtn.Show();
    aSlideBtn.Show();
    aOutlineBtn.Show();
    aNotesBtn.Show();
    aHandoutBtn.Show();
    aPresentationBtn.Show();
    aPageBtn.Show();
    aMasterPageBtn.Show();
    aLayerBtn.Show();

    if( bLayerMode )
        aLayerTab.Show();
    else
        aTabControl.Show();

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
            if( pWinArray[nX][nY] )
                pWinArray[nX][nY]->Show();
}

#ifdef WNT
#pragma optimize ( "", on )
#endif


