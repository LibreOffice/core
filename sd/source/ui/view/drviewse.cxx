/*************************************************************************
 *
 *  $RCSfile: drviewse.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 15:15:18 $
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

#include "DrawViewShell.hxx"

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
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
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
#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx>
#endif

// #UndoRedo#
#ifndef _SFXSLSTITM_HXX
#include <svtools/slstitm.hxx>
#endif

#pragma hdrstop

#include <svtools/urihelper.hxx>
#include <sfx2/topfrm.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "drawdoc.hxx"
#ifndef SD_FU_SELECTION_HXX
#include "fusel.hxx"
#endif
#ifndef SD_FU_TEXT_HXX
#include "futext.hxx"
#endif
#ifndef SD_FU_CONSTRUCT_RECTANGLE_HXX
#include "fuconrec.hxx"
#endif
#ifndef SD_FU_CONSTRUCT_UNO_CONTROL_HXX
#include "fuconuno.hxx"
#endif
#ifndef SD_FU_CONSTRUCT_BEZIER_HXX
#include "fuconbez.hxx"
#endif
#ifndef SD_FU_EDIT_GLUE_POINTS_HXX
#include "fuediglu.hxx"
#endif
#ifndef SD_FU_CONSTRUCT_ARC_HXX
#include "fuconarc.hxx"
#endif
#ifndef SD_FU_CONSTRUCT_3D_OBJECT_HXX
#include "fucon3d.hxx"
#endif
#include "sdresid.hxx"
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#ifndef SD_PRESENTATION_VIEW_SHELL_HXX
#include "PresentationViewShell.hxx"
#endif
#include "sdpage.hxx"
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "zoomlist.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "DrawDocShell.hxx"
#include "sdattr.hxx"
#include "PaneManager.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_LAYER_DIALOG_CHILD_WINDOW_HXX
#include "LayerDialogChildWindow.hxx"
#endif

// #97016#
#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif

// #98721#
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

namespace sd {

#ifdef WNT
#pragma optimize ( "", off )
#endif

/*************************************************************************
|*
|* Permanente Funktionen
|*
\************************************************************************/

void ImpAddPrintableCharactersToTextEdit(SfxRequest& rReq, ::sd::View* pView)
{
    // #98198# evtl. feed characters to activated textedit
    const SfxItemSet* pSet = rReq.GetArgs();

    if(pSet)
    {
        String aInputString;

        if(SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_CHAR))
            aInputString = ((SfxStringItem&)pSet->Get(SID_ATTR_CHAR)).GetValue();

        if(aInputString.Len())
        {
            OutlinerView* pOLV = pView->GetTextEditOutlinerView();

            if(pOLV)
            {
                for(sal_uInt16 a(0); a < aInputString.Len(); a++)
                {
                    sal_Char aChar = (sal_Char)aInputString.GetChar(a);
                    KeyCode aKeyCode;
                    KeyEvent aKeyEvent(aChar, aKeyCode);

                    // add actual character
                    pOLV->PostKeyEvent(aKeyEvent);
                }
            }
        }
    }
}

void DrawViewShell::FuPermanent(SfxRequest& rReq)
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

        Invalidate();

        // #98198# evtl. feed characters to activated textedit
        if(SID_ATTR_CHAR == nSId && GetView() && GetView()->IsTextEdit())
            ImpAddPrintableCharactersToTextEdit(rReq, GetView());

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
            pFuActual = new FuText(this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            ( (FuText*) pFuActual)->DoExecute();
            // Das Setzen des Permanent-Status erfolgt weiter oben!

            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_ATTR_CHAR );
            rBindings.Invalidate( SID_ATTR_CHAR_VERTICAL );
            rBindings.Invalidate( SID_TEXT_FITTOSIZE );
            rBindings.Invalidate( SID_TEXT_FITTOSIZE_VERTICAL );

            // #98198# evtl. feed characters to activated textedit
            if(SID_ATTR_CHAR == nSId && GetView() && GetView()->IsTextEdit())
                ImpAddPrintableCharactersToTextEdit(rReq, GetView());

            rReq.Done();
        }
        break;

        case SID_FM_CREATE_CONTROL:
        {
            pFuActual = new FuConstructUnoControl(
                this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            static_cast<FuConstructUnoControl*>(pFuActual)
                ->SetPermanent(bPermanent);
            rReq.Done();
        }
        break;

        // #98721#
        case SID_FM_CREATE_FIELDCONTROL:
        {
            SFX_REQUEST_ARG( rReq, pDescriptorItem, SfxUnoAnyItem, SID_FM_DATACCESS_DESCRIPTOR, sal_False );
            DBG_ASSERT( pDescriptorItem, "DrawViewShell::FuPermanent(SID_FM_CREATE_FIELDCONTROL): invalid request args!" );

            if(pDescriptorItem)
            {
                // get the form view
                FmFormView* pFormView = PTR_CAST(FmFormView, pDrView);
                SdrPageView* pPageView = pFormView ? pFormView->GetPageViewPvNum(0) : NULL;

                if(pPageView)
                {
                    ::svx::ODataAccessDescriptor aDescriptor(pDescriptorItem->GetValue());
                    SdrObject* pNewDBField = pFormView->CreateFieldControl(aDescriptor);

                    if(pNewDBField)
                    {
                        Rectangle aVisArea = GetActiveWindow()->PixelToLogic(Rectangle(Point(0,0), GetActiveWindow()->GetOutputSizePixel()));
                        Point aObjPos(aVisArea.Center());
                        Size aObjSize(pNewDBField->GetLogicRect().GetSize());
                        aObjPos.X() -= aObjSize.Width() / 2;
                        aObjPos.Y() -= aObjSize.Height() / 2;
                        Rectangle aNewObjectRectangle(aObjPos, aObjSize);

                        pNewDBField->SetLogicRect(aNewObjectRectangle);

                        GetView()->InsertObject(pNewDBField, *pPageView, GetView()->IsSolidDraggingNow() ? SDRINSERT_NOBROADCAST : 0);
                    }
                }
            }
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
                if ( pDrView->GetMarkedObjectList().GetMarkCount() > 0 &&
                    !pDrView->IsCrookAllowed( pDrView->IsCrookNoContortion() ) )
                {
                    if ( pDrView->IsPresObjSelected() )
                    {
                        ::sd::Window* pWindow = GetActiveWindow();
                        InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
                    }
                    else if ( QueryBox(GetActiveWindow(), WB_YES_NO,
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
                const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
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
                        ::sd::Window* pWindow = GetActiveWindow();
                        InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
                    }
                    else if ( QueryBox(GetActiveWindow(), WB_YES_NO,
                                      String(SdResId(STR_ASK_FOR_CONVERT_TO_BEZIER) )
                                      ).Execute() == RET_YES )
                    {
                        // Implizite Wandlung in Bezier
                        WaitObject aWait( (Window*)GetActiveWindow() );
                        pDrView->ConvertMarkedToPathObj(FALSE);
                    }
                }
            }

            pFuActual = new FuSelection(this, GetActiveWindow(), pDrView,
                                              GetDoc(), rReq);

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
            pFuActual = new FuConstructRectangle(
                this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            static_cast<FuConstructRectangle*>(pFuActual)
                ->SetPermanent(bPermanent);

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
            pFuActual = new FuConstructBezierPolygon(
                this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            static_cast<FuConstructBezierPolygon*>(pFuActual)
                ->SetPermanent(bPermanent);
            rReq.Done();
        }
        break;

        case SID_GLUE_EDITMODE:
        {
            if (nOldSId != SID_GLUE_EDITMODE)
            {
                pFuActual = new FuEditGluePoints( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
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
            pFuActual = new FuConstructArc(
                this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            static_cast<FuConstructArc*>(pFuActual)->SetPermanent(bPermanent);
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
            pFuActual = new FuConstruct3dObject(
                this, GetActiveWindow(), pDrView, GetDoc(), rReq);
            static_cast<FuConstruct3dObject*>(pFuActual)
                ->SetPermanent(bPermanent);
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

    // #97016# III CTRL-SID_OBJECT_SELECT -> select first draw object if none is selected yet
    if(SID_OBJECT_SELECT == nSId && pFuActual && (rReq.GetModifier() & KEY_MOD1))
    {
        if(!GetView()->AreObjectsMarked())
        {
            // select first object
            GetView()->UnmarkAllObj();
            GetView()->MarkNextObj(TRUE);

            // ...and make it visible
            if(GetView()->AreObjectsMarked())
                GetView()->MakeVisible(pView->GetAllMarkedRect(), *pWindow);
        }
    }

    // #97016# with qualifier construct directly
    if(pFuActual && (rReq.GetModifier() & KEY_MOD1))
    {
        // get SdOptions
        SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDoc()->GetDocumentType());
        sal_uInt32 nDefaultObjectSizeWidth(pOptions->GetDefaultObjectSizeWidth());
        sal_uInt32 nDefaultObjectSizeHeight(pOptions->GetDefaultObjectSizeHeight());

        // calc position and size
        Rectangle aVisArea = GetActiveWindow()->PixelToLogic(Rectangle(Point(0,0), GetActiveWindow()->GetOutputSizePixel()));
        Point aPagePos = aVisArea.Center();
        aPagePos.X() -= nDefaultObjectSizeWidth / 2;
        aPagePos.Y() -= nDefaultObjectSizeHeight / 2;
        Rectangle aNewObjectRectangle(aPagePos, Size(nDefaultObjectSizeWidth, nDefaultObjectSizeHeight));
        SdrPageView* pPageView = pDrView->GetPageViewPvNum(0);

        if(pPageView)
        {
            // create the default object
            SdrObject* pObj = pFuActual->CreateDefaultObject(nSId, aNewObjectRectangle);

            if(pObj)
            {
                // insert into page
                GetView()->InsertObject(pObj, *pPageView, GetView()->IsSolidDraggingNow() ? SDRINSERT_NOBROADCAST : 0);

                // Now that pFuActual has done what it was created for we
                // can switch on the edit mode for callout objects.
                switch (nSId)
                {
                    case SID_DRAW_CAPTION:
                    case SID_DRAW_CAPTION_VERTICAL:
                    {
                        // Make FuText the current function.
                        SfxUInt16Item aItem (SID_TEXTEDIT, 1);
                        GetViewFrame()->GetDispatcher()->
                            Execute(SID_TEXTEDIT, SFX_CALLMODE_SYNCHRON |
                                SFX_CALLMODE_RECORD, &aItem, 0L);
                        // Put text object into edit mode.
                        GetView()->BegTextEdit (reinterpret_cast<SdrTextObj*>(pObj), pPageView);
                        break;
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// service routine for Undo/Redo implementation
extern SfxUndoManager* ImpGetUndoManagerFromViewShell(DrawViewShell& rDViewShell);

/*************************************************************************
|*
|* SfxRequests fuer Support-Funktionen
|*
\************************************************************************/

void DrawViewShell::FuSupport(SfxRequest& rReq)
{
    if( rReq.GetSlot() == SID_STYLE_FAMILY && rReq.GetArgs())
        GetDocSh()->SetStyleFamily(((SfxUInt16Item&)rReq.GetArgs()->Get( SID_STYLE_FAMILY )).GetValue());

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
                SfxRequest aReq( nMappedSlot, 0, GetDoc()->GetItemPool() );
                ExecuteSlot( aReq );
            }
        }
        break;

        case SID_PRESENTATION:
        case SID_REHEARSE_TIMINGS:
        {
            if( !pFuSlideShow )
            {
                if( pDrView->IsTextEdit() )
                    pDrView->EndTextEdit();

                SFX_REQUEST_ARG( rReq, pFullScreen, SfxBoolItem, ATTR_PRESENT_FULLSCREEN, FALSE );
                const BOOL bFullScreen = ( ( SID_REHEARSE_TIMINGS != rReq.GetSlot() ) && pFullScreen ) ? pFullScreen->GetValue() : GetDoc()->GetPresFullScreen();

                if( bFullScreen )
                    PresentationViewShell::CreateFullScreenShow( this, rReq );
                else
                {
                    pFrameView->SetPreviousViewShellType(GetShellType());
                    pFuSlideShow = new FuSlideShow(
                        this, GetActiveWindow(), pDrView, GetDoc(), rReq );
                    pFuSlideShow->StartShow();
                    pFuSlideShow->Activate();
                }
            }

            rReq.Ignore ();
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

                if( ISA(PresentationViewShell))
                {
                    GetViewFrame()->GetDispatcher()->Execute(
                        SID_CLOSEWIN, SFX_CALLMODE_ASYNCHRON );
                }
                else if( pFrameView->GetPresentationViewShellId() != SID_VIEWSHELL0 )
                {
                    ViewShell::ShellType ePreviousType (
                        pFrameView->GetPreviousViewShellType());

                    pFrameView->SetPresentationViewShellId(SID_VIEWSHELL0);
                    pFrameView->SetSlotId(SID_OBJECT_SELECT);
                    pFrameView->SetPreviousViewShellType(GetShellType());

                    GetViewShellBase().GetPaneManager()
                        .RequestMainViewShellChange(ePreviousType);
                }
            }

            rReq.Ignore ();
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

        case SID_BEZIER_EDIT:
        {
            pDrView->SetFrameDragSingles(!pDrView->IsFrameDragSingles());

            /******************************************************************
            * ObjectBar einschalten
            ******************************************************************/
            if (pFuActual &&
                (pFuActual->ISA(FuSelection)
                    || pFuActual->ISA(FuConstructBezierPolygon)))
            {
                USHORT nObjectBarId = RID_DRAW_OBJ_TOOLBOX;

                if (pDrView->HasMarkablePoints())
                {
                    nObjectBarId = RID_BEZIER_TOOLBOX;
                }

                GetObjectBarManager().SwitchObjectBar (nObjectBarId);
            }

            Invalidate(SID_BEZIER_EDIT);
            rReq.Ignore();
        }
        break;

        case SID_OBJECT_CLOSE:
        {
            const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
            if ( rMarkList.GetMark(0) && !pDrView->IsAction() )
            {
                SdrPathObj* pPathObj = (SdrPathObj*) rMarkList.GetMark(0)->GetObj();
                pDrView->BegUndo(String(SdResId(STR_UNDO_BEZCLOSE)));
                pDrView->UnmarkAllPoints();
                Size aDist(GetActiveWindow()->PixelToLogic(Size(8,8)));
                pDrView->AddUndo(new SdrUndoGeoObj(*pPathObj));
                pPathObj->ToggleClosed(aDist.Width());
                pDrView->EndUndo();
            }
            rReq.Done();
        }
        break;

        case SID_CUT:
        {
            if ( pDrView->IsPresObjSelected(FALSE, TRUE, FALSE, TRUE) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
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
            if ( pDrView->IsPresObjSelected(FALSE, TRUE, FALSE, TRUE) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
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

        case SID_CLIPBOARD_FORMAT_ITEMS:
        {
            WaitObject              aWait( (Window*)GetActiveWindow() );
            TransferableDataHelper  aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
            const SfxItemSet*       pReqArgs = rReq.GetArgs();
            UINT32                  nFormat = 0;

            if( pReqArgs )
            {
                SFX_REQUEST_ARG( rReq, pIsActive, SfxUInt32Item, SID_CLIPBOARD_FORMAT_ITEMS, FALSE );
                nFormat = pIsActive->GetValue();
            }


            if( nFormat && aDataHelper.GetTransferable().is() )
            {
                sal_Int8 nAction = DND_ACTION_COPY;

                if( !pDrView->InsertData( aDataHelper,
                                          GetActiveWindow()->PixelToLogic( Rectangle( Point(), GetActiveWindow()->GetOutputSizePixel() ).Center() ),
                                          nAction, FALSE, nFormat ) )
                {
                    String          aEmptyStr;
                    INetBookmark    aINetBookmark( aEmptyStr, aEmptyStr );

                    if( ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR, aINetBookmark ) ) )
                    {
                        InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), aEmptyStr, NULL );
                    }
                }
            }
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
            else if ( pDrView->IsPresObjSelected(FALSE, TRUE, FALSE, TRUE) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
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

        case SID_NOTESMODE:
        case SID_HANDOUTMODE:
            // AutoLayouts have to be ready.
            GetDoc()->StopWorkStartupDelay();

            // Turn off effects.
            //            pDrView->SetAnimationMode(FALSE);

            // Fall through to following case statements.

        case SID_DRAWINGMODE:
        case SID_DIAMODE:
        case SID_OUTLINEMODE:
            // Let the sub-shell manager handle the slot handling.
            GetViewShellBase().GetPaneManager().HandleModeChangeSlot (
                nSId,
                rReq);
            rReq.Ignore ();
            break;

        case SID_MASTERPAGE:          // BASIC
        case SID_SLIDE_MASTERPAGE:    // BASIC
        case SID_TITLE_MASTERPAGE:    // BASIC
        case SID_NOTES_MASTERPAGE:    // BASIC
        case SID_HANDOUT_MASTERPAGE:  // BASIC
        {
            // AutoLayouts muessen fertig sein
            GetDoc()->StopWorkStartupDelay();

            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, SID_MASTERPAGE, FALSE);
                mbIsLayerModeActive = pIsActive->GetValue ();
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
                    USHORT nCount = GetDoc()->GetSdPageCount(PK_STANDARD);

                    while (i < nCount && !bFound)
                    {
                        SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

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

                ChangeEditMode(EM_MASTERPAGE, mbIsLayerModeActive);

                if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT)
                    GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }
            else
            {
                // Switch to requested ViewShell.
                ViewShell::ShellType eShellType;
                PageKind ePageKind;
                switch (nSId)
                {
                    case SID_SLIDE_MASTERPAGE:
                    case SID_TITLE_MASTERPAGE:
                    default:
                        eShellType = ViewShell::ST_IMPRESS;
                        ePageKind = PK_STANDARD;
                        break;

                    case SID_NOTES_MASTERPAGE:
                        eShellType = ViewShell::ST_NOTES;
                        ePageKind = PK_NOTES;
                        break;

                    case SID_HANDOUT_MASTERPAGE:
                        eShellType = ViewShell::ST_HANDOUT;
                        ePageKind = PK_HANDOUT;
                        break;
                }

                pFrameView->SetViewShEditMode(EM_MASTERPAGE, ePageKind);
                pFrameView->SetLayerMode(mbIsLayerModeActive);
                GetViewShellBase().GetPaneManager().RequestMainViewShellChange(
                    eShellType);
            }

            InvalidateWindows();
            Invalidate();

            rReq.Done();
        }
        break;

        case SID_RULER:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            // #97516# Remember old ruler state
            BOOL bOldHasRuler(HasRuler());

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, SID_RULER, FALSE);
                SetRuler (pIsActive->GetValue ());
            }
            else SetRuler (!HasRuler());

            // #97516# Did ruler state change? Tell that to SdOptions, too.
            BOOL bHasRuler(HasRuler());

            if(bOldHasRuler != bHasRuler)
            {
                SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDoc()->GetDocumentType());

                if(pOptions && pOptions->IsRulerVisible() != bHasRuler)
                {
                    pOptions->SetRulerVisible(bHasRuler);
                }
            }

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
                    Point aPt = GetActiveWindow()->PixelToLogic( Point( 0, GetActiveWindow()->GetSizePixel().Height() / 2 ) );
                    aPagePos.Y() += aPt.Y();
                    aPageSize.Height() = 2;
                }

                aPagePos.X() -= aPageSize.Width()  / 2;

                SetZoomRect( Rectangle( aPagePos, aPageSize ) );

                Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
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
            Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
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
            SetZoom( Max( (long) ( GetActiveWindow()->GetZoom() / 2 ), (long) GetActiveWindow()->GetMinZoom() ) );
            Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
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
            if ( pDrView->AreObjectsMarked() )
            {
                long nW = (long) (aMarkRect.GetWidth()  * 1.03);
                long nH = (long) (aMarkRect.GetHeight() * 1.03);
                Point aPos = aMarkRect.Center();
                aPos.X() -= nW / 2;
                aPos.Y() -= nH / 2;
                if ( nW && nH )
                {
                    SetZoomRect(Rectangle(aPos, Size(nW, nH)));

                    Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
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

                    Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
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
            BOOL bOnlineSpell = !GetDoc()->GetOnlineSpell();
            GetDoc()->SetOnlineSpell(bOnlineSpell);

            ::Outliner* pOL = pDrView->GetTextEditOutliner();

            if (pOL)
            {
                ULONG nCntrl = pOL->GetControlWord();

                if (bOnlineSpell)
                    nCntrl |= EE_CNTRL_ONLINESPELLING;
                else
                    nCntrl &= ~EE_CNTRL_ONLINESPELLING;

                pOL->SetControlWord(nCntrl);
            }

            GetActiveWindow()->Invalidate();
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
            const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

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
            OutlinerView* pOLV = GetView()->GetTextEditOutlinerView();
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

        // #UndoRedo#
        case SID_UNDO :
        {
            // #96090# moved implementation to BaseClass
            ImpSidUndo(TRUE, rReq);
        }
        break;
        case SID_REDO :
        {
            // #96090# moved implementation to BaseClass
            ImpSidRedo(TRUE, rReq);
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

void DrawViewShell::InsertURLField(const String& rURL, const String& rText,
                                     const String& rTarget, const Point* pPos)
{
    SvxURLField aURLField(rURL, rText, SVXURLFORMAT_REPR);
    aURLField.SetTargetFrame(rTarget);
    SvxFieldItem aURLItem(aURLField);

    OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();

    if (pOLV)
    {
        ESelection aSel( pOLV->GetSelection() );
        SvxFieldItem aURLItem( SvxURLField( rURL, rText, SVXURLFORMAT_REPR ) );
        pOLV->InsertField( aURLItem );
        if ( aSel.nStartPos <= aSel.nEndPos )
            aSel.nEndPos = aSel.nStartPos + 1;
        else
            aSel.nStartPos = aSel.nEndPos + 1;
        pOLV->SetSelection( aSel );
    }
    else
    {
        Outliner* pOutl = GetDoc()->GetInternalOutliner();
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
            Rectangle aRect(aPos, GetActiveWindow()->GetOutputSizePixel() );
            aPos = aRect.Center();
            aPos = GetActiveWindow()->PixelToLogic(aPos);
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

void DrawViewShell::InsertURLButton(const String& rURL, const String& rText,
                                      const String& rTarget, const Point* pPos)
{
    BOOL bNewObj = TRUE;

    if (pDrView->GetMarkedObjectList().GetMarkCount() > 0)
    {
        SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pDrView->GetMarkedObjectList().GetMark(0)->GetObj());

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
            if ( Sound::IsSoundFile( rURL ) )
            {
                // #105638# OJ
                aTmp <<= sal_True;
                xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DispatchURLInternal" )), aTmp );
            }
        }
    }

    if (bNewObj)
    {
        SdrUnoObj* pUnoCtrl = (SdrUnoObj*) SdrObjFactory::MakeNewObject(FmFormInventor, OBJ_FM_BUTTON,
                                pDrView->GetPageViewPvNum(0)->GetPage(), GetDoc());

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
        // #105638# OJ
        if ( Sound::IsSoundFile( rURL ) )
        {
            aTmp <<= sal_True;
            xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DispatchURLInternal" )), aTmp );
        }

        Point aPos;

        if (pPos)
        {
            aPos = *pPos;
        }
        else
        {
            aPos = Rectangle(aPos, GetActiveWindow()->GetOutputSizePixel()).Center();
            aPos = GetActiveWindow()->PixelToLogic(aPos);
        }

        Size aSize(4000, 1000);
        aPos.X() -= aSize.Width() / 2;
        aPos.Y() -= aSize.Height() / 2;
        pUnoCtrl->SetLogicRect(Rectangle(aPos, aSize));

        ULONG nOptions = SDRINSERT_SETDEFLAYER;

        OSL_ASSERT (GetViewShell()!=NULL);
        SfxInPlaceClient* pIpClient = GetViewShell()->GetIPClient();
        if (pIpClient!=NULL && pIpClient->IsInPlaceActive())
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

void DrawViewShell::ShowUIControls( sal_Bool bVisible )
{
    if (mbHasRulers)
    {
        if (mpHorizontalRuler.get() != NULL)
            mpHorizontalRuler->Show( bVisible );

        if (mpVerticalRuler.get() != NULL)
            mpVerticalRuler->Show( bVisible );
    }

    if (mpVerticalScrollBar.get() != NULL)
        mpVerticalScrollBar->Show( bVisible );

    if (mpHorizontalScrollBar.get() != NULL)
        mpHorizontalScrollBar->Show( bVisible );

    GetViewFrame()->SetChildWindow(
        LayerDialogChildWindow::GetChildWindowId(),
        IsLayerModeActive() && bVisible);
    aTabControl.Show (bVisible);

    if (mpContentWindow.get() != NULL)
        mpContentWindow->Show( bVisible );
}

#ifdef WNT
#pragma optimize ( "", on )
#endif

} // end of namespace sd
