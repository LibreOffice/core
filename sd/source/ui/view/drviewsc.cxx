/*************************************************************************
 *
 *  $RCSfile: drviewsc.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dl $ $Date: 2001-02-13 12:39:56 $
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

#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _IMAPDLG_HXX
#include <svx/imapdlg.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVX_DLG_NAME_HXX //autogen
#include <svx/dlgname.hxx>
#endif
#ifndef _BINDING_HXX //autogen
#include <sfx2/binding.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
#include <svx/grafctrl.hxx>

#ifndef _SD_STLSHEET_HXX
#include "stlsheet.hxx"
#endif

#include "app.hrc"
#include "strings.hrc"

#include "misc.hxx"
#include "sdwindow.hxx"
#include "imapinfo.hxx"
#include "futempl.hxx"
#include "fusel.hxx"
#include "sdresid.hxx"
#include "drviewsh.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "drawview.hxx"
#include "brkdlg.hxx"

#define MIN_ACTIONS_FOR_DIALOG  5000    // bei mehr als 1600 Metaobjekten
                                        // wird beim Aufbrechen ein Dialog
                                        // angezeigt.
/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void SdDrawViewShell::FuTemp03(SfxRequest& rReq)
{
    USHORT nSId = rReq.GetSlot();
    switch( nSId )
    {
        case SID_GROUP:  // BASIC
        {
            if ( pDrView->IsPresObjSelected() )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                pDrView->GroupMarked();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_UNGROUP:  // BASIC
        {
            pDrView->UnGroupMarked();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_NAME_GROUP:
        {
            // Jetzt nur noch fuer ein Objekt moeglich (28.10.96)
            const SdrMarkList& rMarkList = pDrView->GetMarkList();
            SdrObject* pObj = NULL;
            ULONG nMarkCount = rMarkList.GetMarkCount();
            String aName;
            String aDesc( SdResId( STR_DESC_NAMEGROUP ) );

            if( nMarkCount == 1 )
            {
                pObj = rMarkList.GetMark( 0 )->GetObj();
                if (pObj->ISA(SdrObjGroup) || pObj->ISA(SdrGrafObj))
                {
                    aName = pObj->GetName();
                }
                SvxNameDialog* pDlg = new SvxNameDialog( NULL, aName, aDesc );

                if( pDlg->Execute() == RET_OK )
                {
                    pDlg->GetName( aName );

                    if( pDocSh->CheckObjectName( NULL, aName ) )
                    {
                        if (pObj->ISA(SdrObjGroup) || pObj->ISA(SdrGrafObj))
                        {
                            pObj->SetName(aName);
                        }
                    }
                }
                delete pDlg;
            }

            // In der Hoffnung, dass Dieter die nachfolgende Zeile stehen laesst
            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_NAVIGATOR_STATE, TRUE, FALSE );
            rBindings.Invalidate( SID_CONTEXT );

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_ENTER_GROUP:  // BASIC
        {
            pDrView->EnterMarkedGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_LEAVE_GROUP:  // BASIC
        {
            pDrView->LeaveOneGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_LEAVE_ALL_GROUPS:  // BASIC
        {
            pDrView->LeaveAllGroup();
            Cancel();
            rReq.Done ();
        }
        break;

//      {
//          // Versuchen, die betretene Gruppe zu bekommen
//          SdrObject* pGroup = NULL;
//          SdrPageView* pPV = pDrView->GetPageViewPvNum(0);
//          if(pPV)
//              pGroup = pPV->GetAktGroup();
//
//          if(nSId == SID_LEAVE_ALL_GROUPS)
//          {
//              // Alle Gruppen verlassen
//              pDrView->LeaveAllGroup();
//
//              // Objekt bis zur obersten Gruppe zurueckverfolgen
//              while(pGroup && pGroup->GetObjList() && pGroup->GetObjList()->GetOwnerObj())
//                  pGroup = pGroup->GetObjList()->GetOwnerObj();
//          }
//          else
//          {
//              // Eine Gruppe verlassen
//              pDrView->LeaveOneGroup();
//          }
//
//          pDrView->UnmarkAll();
//          Cancel();
//          rReq.Done ();
//
//          // Falls die Gruppe geholt werden konnte, selektiere diese
//          if(pGroup)
//          {
//              for (USHORT nv=0; nv<pDrView->GetPageViewCount(); nv++)
//              {
//                  SdrPageView* pPV = pDrView->GetPageViewPvNum(nv);
//                  pDrView->MarkObj(pGroup, pPV);
//              }
//          }
//      }
//      break;

        case SID_COMBINE:  // BASIC
        {
            if ( pDrView->IsPresObjSelected() )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                pDrView->CombineMarkedObjects(FALSE);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DISTRIBUTE_DLG:
        {
            if ( pDrView->IsPresObjSelected() )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                pDrView->DistributeMarkedObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_MERGE:
        {
            if ( pDrView->IsPresObjSelected() )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                pDrView->MergeMarkedObjects(SDR_MERGE_MERGE);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_SUBSTRACT:
        {
            if ( pDrView->IsPresObjSelected() )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                pDrView->MergeMarkedObjects(SDR_MERGE_SUBSTRACT);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_INTERSECT:
        {
            if ( pDrView->IsPresObjSelected() )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                pDrView->MergeMarkedObjects(SDR_MERGE_INTERSECT);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DISMANTLE:  // BASIC
        {
            if ( pDrView->IsDismantlePossible(FALSE) )
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                pDrView->DismantleMarkedObjects(FALSE);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_CONNECT:  // BASIC
        {
            if ( pDrView->IsPresObjSelected() )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                pDrView->CombineMarkedObjects(TRUE);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_BREAK:  // BASIC
        {
            if ( pDrView->IsTextEdit() )
            {
                pDrView->EndTextEdit();
            }

            if ( pDrView->IsBreak3DObjPossible() )
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                pDrView->Break3DObj();
            }
            else if ( pDrView->IsDismantlePossible(TRUE) )
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                pDrView->DismantleMarkedObjects(TRUE);
            }
            else if ( pDrView->IsImportMtfPossible() )
            {

                WaitObject aWait( (Window*)GetActiveWindow() );
                const SdrMarkList& rMarkList = pDrView->GetMarkList();
                ULONG nAnz=rMarkList.GetMarkCount();

                // Summe der Metaobjekte aller sel. Metafiles erm.
                ULONG nCount = 0;
                for(ULONG nm=0; nm<nAnz; nm++)
                {
                    SdrMark*     pM=rMarkList.GetMark(nm);
                    SdrObject*   pObj=pM->GetObj();
                    SdrGrafObj*  pGraf=PTR_CAST(SdrGrafObj,pObj);
                    SdrOle2Obj*  pOle2=PTR_CAST(SdrOle2Obj,pObj);
                    if (pGraf!=NULL && pGraf->HasGDIMetaFile())
                        nCount += pGraf->GetGraphic().GetGDIMetaFile().GetActionCount();
                    if(pOle2!=NULL && pOle2->HasGDIMetaFile())
                    {
                        const GDIMetaFile* pMtf=pOle2->GetGDIMetaFile();
                        nCount += pMtf->GetActionCount();
                    }
                }

                // anhand der erm. Summe entscheiden ob mit
                // oder ohne Dialog aufgebrochen wird.
                if(nCount < MIN_ACTIONS_FOR_DIALOG)
                {
                    // ohne Dialog aufbrechen
                    pDrView->DoImportMarkedMtf();
                }
                else
                {
                    // mit Dialog aufbrechen
                    SdBreakDlg aDlg( pWindow, pDrView, GetDocSh(), nCount, nAnz );
                    aDlg.Execute();
                }
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_CONVERT_TO_3D:
        {
            if ( pDrView->IsPresObjSelected() )
            {
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                if (pDrView->IsConvertTo3DObjPossible())
                {
                    if (pDrView->IsTextEdit())
                    {
                        pDrView->EndTextEdit();
                    }

                    WaitObject aWait( (Window*)GetActiveWindow() );
                    pDrView->ConvertMarkedObjTo3D(TRUE);
                }
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_FRAME_TO_TOP:  // BASIC
        {
            pDrView->PutMarkedToTop();
            Cancel();
            Invalidate( SID_POSITION );
            rReq.Done ();
        }
        break;

        case SID_MOREFRONT:  // BASIC
        {
            pDrView->MovMarkedToTop();
            Cancel();
            Invalidate( SID_POSITION );
            rReq.Done ();
        }
        break;

        case SID_MOREBACK:  // BASIC
        {
            pDrView->MovMarkedToBtm();
            Cancel();
            Invalidate( SID_POSITION );
            rReq.Done ();
        }
        break;

        case SID_FRAME_TO_BOTTOM:   // BASIC
        {
            pDrView->PutMarkedToBtm();
            Cancel();
            Invalidate( SID_POSITION );
            rReq.Done ();
        }
        break;

        case SID_HORIZONTAL:  // BASIC
        {
            pDrView->MirrorAllMarkedHorizontal();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_VERTICAL:  // BASIC
        {
            pDrView->MirrorAllMarkedVertical();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_LEFT:  // BASIC
        {
            pDrView->AlignMarkedObjects(SDRHALIGN_LEFT, SDRVALIGN_NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_CENTER:  // BASIC
        {
            pDrView->AlignMarkedObjects(SDRHALIGN_CENTER, SDRVALIGN_NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_RIGHT:  // BASIC
        {
            pDrView->AlignMarkedObjects(SDRHALIGN_RIGHT, SDRVALIGN_NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_UP:  // BASIC
        {
            pDrView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_TOP);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_MIDDLE:  // BASIC
        {
            pDrView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_CENTER);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_DOWN:  // BASIC
        {
            pDrView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_BOTTOM);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SELECTALL:  // BASIC
        {
            if (pFuOld && pFuOld->ISA(FuSelection) &&
                !pView->IsFrameDragSingles() && pView->HasMarkablePoints())
            {
                if ( !pDrView->IsAction() )
                    pDrView->MarkAllPoints();
            }
            else
                pDrView->SelectAll();

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
                SfxStyleSheet* pStyleSheet = pDrView->GetStyleSheet();
                if( pStyleSheet && pStyleSheet->GetFamily() == SD_LT_FAMILY)
                    pStyleSheet = ((SdStyleSheet*)pStyleSheet)->GetPseudoStyleSheet();

                if( (pStyleSheet == NULL) && pView->IsTextEdit() )
                {
                    pView->EndTextEdit();

                    pStyleSheet = pDrView->GetStyleSheet();
                    if(pStyleSheet && pStyleSheet->GetFamily() == SD_LT_FAMILY)
                        pStyleSheet = ((SdStyleSheet*)pStyleSheet)->GetPseudoStyleSheet();
                }

                if( pStyleSheet == NULL )
                {
                    rReq.Ignore();
                    break;
                }

                SfxAllItemSet aSet(pDoc->GetPool());

                SfxStringItem aStyleNameItem( SID_STYLE_EDIT, pStyleSheet->GetName() );
                aSet.Put(aStyleNameItem);

                SfxUInt16Item aStyleFamilyItem( SID_STYLE_FAMILY, pStyleSheet->GetFamily() );
                aSet.Put(aStyleFamilyItem);

                rReq.SetArgs(aSet);
            }

            if( rReq.GetArgs() )
            {
                pFuActual = new FuTemplate( this, pWindow, pDrView, pDoc, rReq );
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
            USHORT      nId = SvxIMapDlgChildWindow::GetChildWindowId();

            GetViewFrame()->ToggleChildWindow( nId );
            GetViewFrame()->GetBindings().Invalidate( SID_IMAP );

            if ( GetViewFrame()->HasChildWindow( nId ) && ( ( pDlg = SVXIMAPDLG() ) != NULL ) )
            {
                const SdrMarkList&  rMarkList = pDrView->GetMarkList();

                if ( rMarkList.GetMarkCount() == 1 )
                    UpdateIMapDlg( rMarkList.GetMark( 0 )->GetObj() );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_GRID_FRONT:
        {
            pDrView->SetGridFront( !pDrView->IsGridFront() );
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HELPLINES_FRONT:
        {
            pDrView->SetHlplFront( !pDrView->IsHlplFront() );
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

USHORT SdDrawViewShell::GetIdBySubId( USHORT nSId )
{
    USHORT nMappedSId = 0;
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
#ifdef STARIMAGE_AVAILABLE
        case SID_INSERT_IMAGE:
#endif
        case SID_ATTR_TABLE:
        case SID_INSERTFILE:
        case SID_INSERT_GRAPHIC:
        case SID_INSERTPAGE:
        case SID_INSERT_MATH:
        case SID_INSERT_FLOATINGFRAME:
        case SID_INSERT_OBJECT:
        case SID_INSERT_PLUGIN:
        case SID_INSERT_SOUND:
        case SID_INSERT_VIDEO:
        case SID_INSERT_APPLET:
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

void SdDrawViewShell::MapSlot( USHORT nSId )
{
    USHORT nMappedSId = GetIdBySubId( nSId );

    if( nMappedSId > 0 )
    {
        USHORT nID = GetArrayId( nMappedSId ) + 1;
        pSlotArray[ nID ] = nSId;
    }
}

/*************************************************************************
|*
|* Ermoeglicht ueber das SlotArray ein ImageMapping
|*
\************************************************************************/

void SdDrawViewShell::UpdateToolboxImages( SfxItemSet &rSet, BOOL bPermanent )
{
    if( !bPermanent )
    {
        USHORT nId = GetArrayId( SID_ZOOM_TOOLBOX ) + 1;
        rSet.Put( TbxImageItem( SID_ZOOM_TOOLBOX, pSlotArray[nId] ) );

        nId = GetArrayId( SID_DRAWTBX_INSERT ) + 1;
        rSet.Put( TbxImageItem( SID_DRAWTBX_INSERT, pSlotArray[nId] ) );

        nId = GetArrayId( SID_POSITION ) + 1;
        rSet.Put( TbxImageItem( SID_POSITION, pSlotArray[nId] ) );
    }
    else
    {
        for( USHORT nId = 0; nId < SLOTARRAY_COUNT; nId += 2 )
        {
            rSet.Put( TbxImageItem( pSlotArray[nId], pSlotArray[nId+1] ) );
        }
    }
}

/*************************************************************************
|*
|* Gibt den gemappten Slot zurueck
|*
\************************************************************************/

USHORT SdDrawViewShell::GetMappedSlot( USHORT nSId )
{
    USHORT nSlot = 0;
    USHORT nId = GetArrayId( nSId );
    if( nId != USHRT_MAX )
        nSlot = pSlotArray[ nId+1 ];

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

USHORT SdDrawViewShell::GetArrayId( USHORT nSId )
{
    for( int i = 0; i < SLOTARRAY_COUNT; i += 2 )
    {
        if( pSlotArray[ i ] == nSId )
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

void SdDrawViewShell::UpdateIMapDlg( SdrObject* pObj )
{
    if( ( pObj->ISA( SdrGrafObj ) || pObj->ISA( SdrOle2Obj ) ) && !pDrView->IsTextEdit() &&
         GetViewFrame()->HasChildWindow( SvxIMapDlgChildWindow::GetChildWindowId() ) )
    {
        Graphic     aGraphic;
        ImageMap*   pIMap = NULL;
        TargetList* pTargetList = NULL;
        SdIMapInfo* pIMapInfo = pDoc->GetIMapInfo( pObj );

        // Grafik vom Objekt besorgen
        if ( pObj->ISA( SdrGrafObj ) )
            aGraphic = ( (SdrGrafObj*) pObj )->GetGraphic();
        else
            aGraphic = pDoc->GetGraphicFromOle2Obj( (const SdrOle2Obj*) pObj );

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



