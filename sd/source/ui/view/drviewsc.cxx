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

#include "DrawViewShell.hxx"
#include "ViewShellImplementation.hxx"
#include <vcl/waitobj.hxx>

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svx/dialogs.hrc>
#ifndef _IMAPDLG_HXX
#include <svx/imapdlg.hxx>
#endif
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/svxdlg.hxx>
#ifndef _BINDING_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#include <sfx2/dispatch.hxx>
#include <svx/svdoole2.hxx>
#include <svl/style.hxx>
#include <svx/svdpagv.hxx>
#include <svx/grafctrl.hxx>
#include "stlsheet.hxx"

#include <sfx2/viewfrm.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "helpids.h"
#include "misc.hxx"
#include "Window.hxx"
#include "imapinfo.hxx"
#include "futempl.hxx"
#include "fusel.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "drawview.hxx"
#include "sdabstdlg.hxx"
#include "brkdlg.hrc"
namespace sd {

#define MIN_ACTIONS_FOR_DIALOG  5000    // bei mehr als 1600 Metaobjekten
                                        // wird beim Aufbrechen ein Dialog
                                        // angezeigt.
/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void DrawViewShell::FuTemp03(SfxRequest& rReq)
{
    sal_uInt16 nSId = rReq.GetSlot();
    switch( nSId )
    {
        case SID_GROUP:  // BASIC
        {
            if ( mpDrawView->IsPresObjSelected( sal_True, sal_True, sal_True ) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                mpDrawView->GroupMarked();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_UNGROUP:  // BASIC
        {
            mpDrawView->UnGroupMarked();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_NAME_GROUP:
        {
            // only allow for single object selection since the name of an object needs
            // to be unique
            if(1L == mpDrawView->GetMarkedObjectCount())
            {
                // #i68101#
                SdrObject* pSelected = mpDrawView->GetMarkedObjectByIndex(0L);
                OSL_ENSURE(pSelected, "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                String aName(pSelected->GetName());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialogdiet fail!");
                AbstractSvxObjectNameDialog* pDlg = pFact->CreateSvxObjectNameDialog(NULL, aName);
                OSL_ENSURE(pDlg, "Dialogdiet fail!");

                pDlg->SetCheckNameHdl(LINK(this, DrawViewShell, NameObjectHdl));

                if(RET_OK == pDlg->Execute())
                {
                    pDlg->GetName(aName);
                    pSelected->SetName(aName);
                }

                delete pDlg;
            }

            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_NAVIGATOR_STATE, sal_True, sal_False );
            rBindings.Invalidate( SID_CONTEXT );

            Cancel();
            rReq.Ignore();
            break;
        }

        // #i68101#
        case SID_OBJECT_TITLE_DESCRIPTION:
        {
            if(1L == mpDrawView->GetMarkedObjectCount())
            {
                SdrObject* pSelected = mpDrawView->GetMarkedObjectByIndex(0L);
                OSL_ENSURE(pSelected, "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                String aTitle(pSelected->GetTitle());
                String aDescription(pSelected->GetDescription());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialogdiet fail!");
                AbstractSvxObjectTitleDescDialog* pDlg = pFact->CreateSvxObjectTitleDescDialog(NULL, aTitle, aDescription);
                OSL_ENSURE(pDlg, "Dialogdiet fail!");

                if(RET_OK == pDlg->Execute())
                {
                    pDlg->GetTitle(aTitle);
                    pDlg->GetDescription(aDescription);
                    pSelected->SetTitle(aTitle);
                    pSelected->SetDescription(aDescription);
                }

                delete pDlg;
            }

            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_NAVIGATOR_STATE, sal_True, sal_False );
            rBindings.Invalidate( SID_CONTEXT );

            Cancel();
            rReq.Ignore();
            break;
        }

        case SID_ENTER_GROUP:  // BASIC
        {
            mpDrawView->EnterMarkedGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_LEAVE_GROUP:  // BASIC
        {
            mpDrawView->LeaveOneGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_LEAVE_ALL_GROUPS:  // BASIC
        {
            mpDrawView->LeaveAllGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_COMBINE:  // BASIC
        {
            // #88224# End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                mpDrawView->CombineMarkedObjects(sal_False);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DISTRIBUTE_DLG:
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                mpDrawView->DistributeMarkedObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_MERGE:
        {
            // #88224# End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                mpDrawView->MergeMarkedObjects(SDR_MERGE_MERGE);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_SUBSTRACT:
        {
            // #88224# End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                mpDrawView->MergeMarkedObjects(SDR_MERGE_SUBSTRACT);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_INTERSECT:
        {
            // #88224# End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                mpDrawView->MergeMarkedObjects(SDR_MERGE_INTERSECT);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DISMANTLE:  // BASIC
        {
            if ( mpDrawView->IsDismantlePossible(sal_False) )
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                mpDrawView->DismantleMarkedObjects(sal_False);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_CONNECT:  // BASIC
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                mpDrawView->CombineMarkedObjects(sal_True);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_BREAK:  // BASIC
        {
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            if ( mpDrawView->IsBreak3DObjPossible() )
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                mpDrawView->Break3DObj();
            }
            else if ( mpDrawView->IsDismantlePossible(sal_True) )
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                mpDrawView->DismantleMarkedObjects(sal_True);
            }
            else if ( mpDrawView->IsImportMtfPossible() )
            {

                WaitObject aWait( (Window*)GetActiveWindow() );
                const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                sal_uLong nAnz=rMarkList.GetMarkCount();

                // Summe der Metaobjekte aller sel. Metafiles erm.
                sal_uLong nCount = 0;
                for(sal_uLong nm=0; nm<nAnz; nm++)
                {
                    SdrMark*     pM=rMarkList.GetMark(nm);
                    SdrObject*   pObj=pM->GetMarkedSdrObj();
                    SdrGrafObj*  pGraf=PTR_CAST(SdrGrafObj,pObj);
                    SdrOle2Obj*  pOle2=PTR_CAST(SdrOle2Obj,pObj);
                    if (pGraf!=NULL && pGraf->HasGDIMetaFile())
                        nCount += pGraf->GetGraphic().GetGDIMetaFile().GetActionCount();
                    if(pOle2!=NULL && pOle2->GetGraphic())
                        nCount += pOle2->GetGraphic()->GetGDIMetaFile().GetActionCount();
                }

                // anhand der erm. Summe entscheiden ob mit
                // oder ohne Dialog aufgebrochen wird.
                if(nCount < MIN_ACTIONS_FOR_DIALOG)
                {
                    // ohne Dialog aufbrechen
                    mpDrawView->DoImportMarkedMtf();
                }
                else
                {
                    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                    if( pFact )
                    {
                        VclAbstractDialog* pDlg = pFact->CreateBreakDlg(GetActiveWindow(), mpDrawView, GetDocSh(), nCount, nAnz );
                        if( pDlg )
                        {
                            pDlg->Execute();
                            delete pDlg;
                        }
                    }
                }
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_CONVERT_TO_3D:
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                if (mpDrawView->IsConvertTo3DObjPossible())
                {
                    if (mpDrawView->IsTextEdit())
                    {
                        mpDrawView->SdrEndTextEdit();
                    }

                    WaitObject aWait( (Window*)GetActiveWindow() );
                    mpDrawView->ConvertMarkedObjTo3D(sal_True);
                }
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_FRAME_TO_TOP:  // BASIC
        {
            mpDrawView->PutMarkedToTop();
            Cancel();
            Invalidate( SID_POSITION );
            rReq.Done ();
        }
        break;

        case SID_MOREFRONT:  // BASIC
        {
            mpDrawView->MovMarkedToTop();
            Cancel();
            Invalidate( SID_POSITION );
            rReq.Done ();
        }
        break;

        case SID_MOREBACK:  // BASIC
        {
            mpDrawView->MovMarkedToBtm();
            Cancel();
            Invalidate( SID_POSITION );
            rReq.Done ();
        }
        break;

        case SID_FRAME_TO_BOTTOM:   // BASIC
        {
            mpDrawView->PutMarkedToBtm();
            Cancel();
            Invalidate( SID_POSITION );
            rReq.Done ();
        }
        break;

        case SID_HORIZONTAL:  // BASIC
        {
            mpDrawView->MirrorAllMarkedHorizontal();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_VERTICAL:  // BASIC
        {
            mpDrawView->MirrorAllMarkedVertical();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_LEFT:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_LEFT, SDRVALIGN_NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_CENTER:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_CENTER, SDRVALIGN_NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_RIGHT:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_RIGHT, SDRVALIGN_NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_UP:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_TOP);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_MIDDLE:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_CENTER);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_DOWN:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_BOTTOM);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SELECTALL:  // BASIC
        {
            if( (dynamic_cast<FuSelection*>( GetOldFunction().get() ) != 0) &&
                !GetView()->IsFrameDragSingles() && GetView()->HasMarkablePoints())
            {
                if ( !mpDrawView->IsAction() )
                    mpDrawView->MarkAllPoints();
            }
            else
                mpDrawView->SelectAll();

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_STYLE_NEW: // BASIC ???
        case SID_STYLE_APPLY:
        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_FAMILY:
        case SID_STYLE_WATERCAN:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            if( rReq.GetSlot() == SID_STYLE_EDIT && !rReq.GetArgs() )
            {
                SfxStyleSheet* pStyleSheet = mpDrawView->GetStyleSheet();
                if( pStyleSheet && pStyleSheet->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE)
                    pStyleSheet = ((SdStyleSheet*)pStyleSheet)->GetPseudoStyleSheet();

                if( (pStyleSheet == NULL) && GetView()->IsTextEdit() )
                {
                    GetView()->SdrEndTextEdit();

                    pStyleSheet = mpDrawView->GetStyleSheet();
                    if(pStyleSheet && pStyleSheet->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE)
                        pStyleSheet = ((SdStyleSheet*)pStyleSheet)->GetPseudoStyleSheet();
                }

                if( pStyleSheet == NULL )
                {
                    rReq.Ignore();
                    break;
                }

                SfxAllItemSet aSet(GetDoc()->GetPool());

                SfxStringItem aStyleNameItem( SID_STYLE_EDIT, pStyleSheet->GetName() );
                aSet.Put(aStyleNameItem);

                SfxUInt16Item aStyleFamilyItem( SID_STYLE_FAMILY, (sal_uInt16)pStyleSheet->GetFamily() );
                aSet.Put(aStyleFamilyItem);

                rReq.SetArgs(aSet);
            }

            if( rReq.GetArgs() )
            {
                SetCurrentFunction( FuTemplate::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
                if( rReq.GetSlot() == SID_STYLE_APPLY )
                    GetViewFrame()->GetBindings().Invalidate( SID_STYLE_APPLY );
                Cancel();
            }
            else if( rReq.GetSlot() == SID_STYLE_APPLY )
                GetViewFrame()->GetDispatcher()->Execute( SID_STYLE_DESIGNER, SFX_CALLMODE_ASYNCHRON );
            rReq.Ignore ();
        }
        break;

        case SID_IMAP:
        {
            SvxIMapDlg* pDlg;
            sal_uInt16      nId = SvxIMapDlgChildWindow::GetChildWindowId();

            GetViewFrame()->ToggleChildWindow( nId );
            GetViewFrame()->GetBindings().Invalidate( SID_IMAP );

            if ( GetViewFrame()->HasChildWindow( nId )
                && ( ( pDlg = ViewShell::Implementation::GetImageMapDialog() ) != NULL ) )
            {
                const SdrMarkList&  rMarkList = mpDrawView->GetMarkedObjectList();

                if ( rMarkList.GetMarkCount() == 1 )
                    UpdateIMapDlg( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_GRID_FRONT:
        {
            mpDrawView->SetGridFront( !mpDrawView->IsGridFront() );
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HELPLINES_FRONT:
        {
            mpDrawView->SetHlplFront( !mpDrawView->IsHlplFront() );
            Cancel();
            rReq.Done ();
        }
        break;

        default:
            FuTemp04(rReq);
        break;
    };
};

/*************************************************************************
|*
|* Liefert die globale/Haupt-ID zurueck, also die ID, ueber die die
|* Toolbox ausgeloest wird
|*
\************************************************************************/

sal_uInt16 DrawViewShell::GetIdBySubId( sal_uInt16 nSId )
{
    sal_uInt16 nMappedSId = 0;
    switch( nSId )
    {
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
            nMappedSId = SID_OBJECT_CHOOSE_MODE;
        }
        break;

        case SID_OBJECT_ALIGN_LEFT:
        case SID_OBJECT_ALIGN_CENTER:
        case SID_OBJECT_ALIGN_RIGHT:
        case SID_OBJECT_ALIGN_UP:
        case SID_OBJECT_ALIGN_MIDDLE:
        case SID_OBJECT_ALIGN_DOWN:
        {
            nMappedSId = SID_OBJECT_ALIGN;
        }
        break;

        case SID_FRAME_TO_TOP:
        case SID_MOREFRONT:
        case SID_MOREBACK:
        case SID_FRAME_TO_BOTTOM:
        case SID_BEFORE_OBJ:
        case SID_BEHIND_OBJ:
        case SID_REVERSE_ORDER:
        {
            nMappedSId = SID_POSITION;
        }
        break;

        case SID_ZOOM_OUT:
        case SID_ZOOM_IN:
        case SID_SIZE_REAL:
        case SID_ZOOM_PANNING:
        case SID_SIZE_PAGE:
        case SID_SIZE_PAGE_WIDTH:
        case SID_SIZE_ALL:
        case SID_SIZE_OPTIMAL:
        case SID_ZOOM_NEXT:
        case SID_ZOOM_PREV:
        {
            nMappedSId = SID_ZOOM_TOOLBOX;
        }
        break;

        case SID_ATTR_CHAR:
        case SID_TEXT_FITTOSIZE:
        case SID_DRAW_CAPTION:
        case SID_DRAW_FONTWORK:
        case SID_DRAW_FONTWORK_VERTICAL:
        {
            nMappedSId = SID_DRAWTBX_TEXT;
        }
        break;

        case SID_DRAW_RECT:
        case SID_DRAW_SQUARE:
        case SID_DRAW_RECT_ROUND:
        case SID_DRAW_SQUARE_ROUND:
        case SID_DRAW_RECT_NOFILL:
        case SID_DRAW_SQUARE_NOFILL:
        case SID_DRAW_RECT_ROUND_NOFILL:
        case SID_DRAW_SQUARE_ROUND_NOFILL:
        {
            nMappedSId = SID_DRAWTBX_RECTANGLES;
        }
        break;

        case SID_DRAW_ELLIPSE:
        case SID_DRAW_CIRCLE:
        case SID_DRAW_PIE:
        case SID_DRAW_CIRCLEPIE:
        case SID_DRAW_ELLIPSECUT:
        case SID_DRAW_CIRCLECUT:
        case SID_DRAW_ARC:
        case SID_DRAW_CIRCLEARC:
        case SID_DRAW_ELLIPSE_NOFILL:
        case SID_DRAW_CIRCLE_NOFILL:
        case SID_DRAW_PIE_NOFILL:
        case SID_DRAW_CIRCLEPIE_NOFILL:
        case SID_DRAW_ELLIPSECUT_NOFILL:
        case SID_DRAW_CIRCLECUT_NOFILL:
        {
            nMappedSId = SID_DRAWTBX_ELLIPSES;
        }
        break;

        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON_NOFILL:
        case SID_DRAW_FREELINE_NOFILL:
        case SID_DRAW_BEZIER_FILL:
        case SID_DRAW_POLYGON:
        case SID_DRAW_XPOLYGON:
        case SID_DRAW_FREELINE:
        {
            nMappedSId = SID_DRAWTBX_LINES;
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
        {
            nMappedSId = SID_DRAWTBX_ARROWS;
        }
        break;

        case SID_3D_CUBE:
        case SID_3D_TORUS:
        case SID_3D_SPHERE:
        case SID_3D_SHELL:
        case SID_3D_HALF_SPHERE:
        case SID_3D_CYLINDER:
        case SID_3D_CONE:
        case SID_3D_PYRAMID:
        {
            nMappedSId = SID_DRAWTBX_3D_OBJECTS;
        }
        break;

        case SID_INSERT_DIAGRAM:
        case SID_ATTR_TABLE:
        case SID_INSERTFILE:
        case SID_INSERT_GRAPHIC:
        case SID_INSERT_AVMEDIA:
        case SID_INSERTPAGE:
        case SID_INSERT_MATH:
        case SID_INSERT_FLOATINGFRAME:
        case SID_INSERT_OBJECT:
        case SID_INSERT_PLUGIN:
        case SID_INSERT_SOUND:
        case SID_INSERT_VIDEO:
        case SID_INSERT_TABLE:
        {
            nMappedSId = SID_DRAWTBX_INSERT;
        }
        break;

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
            nMappedSId = SID_DRAWTBX_CONNECTORS;
        }
    }
    return( nMappedSId );
}

/*************************************************************************
|*
|* Fuellt das SlotArray, um das aktuelle Mapping des ToolboxSlots zu
|* bekommen
|*
\************************************************************************/

void DrawViewShell::MapSlot( sal_uInt16 nSId )
{
    sal_uInt16 nMappedSId = GetIdBySubId( nSId );

    if( nMappedSId > 0 )
    {
        sal_uInt16 nID = GetArrayId( nMappedSId ) + 1;
        mpSlotArray[ nID ] = nSId;
    }
}

/*************************************************************************
|*
|* Ermoeglicht ueber das SlotArray ein ImageMapping
|*
\************************************************************************/

void DrawViewShell::UpdateToolboxImages( SfxItemSet &rSet, sal_Bool bPermanent )
{
    if( !bPermanent )
    {
        sal_uInt16 nId = GetArrayId( SID_ZOOM_TOOLBOX ) + 1;
        rSet.Put( TbxImageItem( SID_ZOOM_TOOLBOX, mpSlotArray[nId] ) );

        nId = GetArrayId( SID_DRAWTBX_INSERT ) + 1;
        rSet.Put( TbxImageItem( SID_DRAWTBX_INSERT, mpSlotArray[nId] ) );

        nId = GetArrayId( SID_POSITION ) + 1;
        rSet.Put( TbxImageItem( SID_POSITION, mpSlotArray[nId] ) );

        nId = GetArrayId( SID_OBJECT_ALIGN ) + 1;
        rSet.Put( TbxImageItem( SID_OBJECT_ALIGN, mpSlotArray[nId] ) );
    }
    else
    {
        for( sal_uInt16 nId = 0; nId < SLOTARRAY_COUNT; nId += 2 )
        {
            rSet.Put( TbxImageItem( mpSlotArray[nId], mpSlotArray[nId+1] ) );
        }
    }
}

/*************************************************************************
|*
|* Gibt den gemappten Slot zurueck
|*
\************************************************************************/

sal_uInt16 DrawViewShell::GetMappedSlot( sal_uInt16 nSId )
{
    sal_uInt16 nSlot = 0;
    sal_uInt16 nId = GetArrayId( nSId );
    if( nId != USHRT_MAX )
        nSlot = mpSlotArray[ nId+1 ];

    // Wenn der Slot noch auf sich selbst gemapped ist, muss 0 zurueck-
    // gegeben werden, da sonst der Slot immer wieder selbst executet
    // wird. Im Array ist der Slot selbst initial vorhanden, damit das
    // Image richtig angezeigt wird.
    if( nSId == nSlot )
        return( 0 );

    return( nSlot );
}

/*************************************************************************
|*
|* Gibt die Nummer des HauptSlots im SlotArray zurueck
|*
\************************************************************************/

sal_uInt16 DrawViewShell::GetArrayId( sal_uInt16 nSId )
{
    for( sal_uInt16 i = 0; i < SLOTARRAY_COUNT; i += 2 )
    {
        if( mpSlotArray[ i ] == nSId )
            return( i );
    }
    DBG_ERROR( "Slot im Array nicht gefunden!" );
    return( USHRT_MAX );
}


/*************************************************************************
|*
|* IMap-Dlg updaten
|*
\************************************************************************/

void DrawViewShell::UpdateIMapDlg( SdrObject* pObj )
{
    if( ( pObj->ISA( SdrGrafObj ) || pObj->ISA( SdrOle2Obj ) ) && !mpDrawView->IsTextEdit() &&
         GetViewFrame()->HasChildWindow( SvxIMapDlgChildWindow::GetChildWindowId() ) )
    {
        Graphic     aGraphic;
        ImageMap*   pIMap = NULL;
        TargetList* pTargetList = NULL;
        SdIMapInfo* pIMapInfo = GetDoc()->GetIMapInfo( pObj );

        // get graphic from shape
        SdrGrafObj* pGrafObj = dynamic_cast< SdrGrafObj* >( pObj );
        if( pGrafObj )
            aGraphic = pGrafObj->GetGraphic();

        if ( pIMapInfo )
        {
            pIMap = (ImageMap*) &pIMapInfo->GetImageMap();
            pTargetList = new TargetList;
            GetViewFrame()->GetTargetList( *pTargetList );
        }

        SvxIMapDlgChildWindow::UpdateIMapDlg( aGraphic, pIMap, pTargetList, pObj );

        // TargetListe kann von uns wieder geloescht werden
        if ( pTargetList )
        {
            String* pEntry = pTargetList->First();
            while( pEntry )
            {
                delete pEntry;
                pEntry = pTargetList->Next();
            }

            delete pTargetList;
        }
    }
}

// -----------------------------------------------------------------------------

IMPL_LINK( DrawViewShell, NameObjectHdl, AbstractSvxNameDialog*, pDialog )
{
    String aName;

    if( pDialog )
        pDialog->GetName( aName );

    return( ( !aName.Len() || ( GetDoc() && !GetDoc()->GetObj( aName ) ) ) ? 1 : 0 );
}

} // end of namespace sd
