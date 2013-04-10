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

#include "DrawViewShell.hxx"
#include "ViewShellImplementation.hxx"
#include <vcl/waitobj.hxx>
#include <svx/svdograf.hxx>
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#ifndef _ZOOMITEM_HXX
#include <svx/zoomitem.hxx>
#endif
#ifndef _EDITDATA_HXX
#include <editeng/editdata.hxx>
#endif
#include <basic/sberrors.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svl/aeitem.hxx>
#include <editeng/eeitem.hxx>
#include <basic/sbstar.hxx>
#include <editeng/flditem.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xfillit0.hxx>

#ifndef _SDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif
#include <svx/xlnwtit.hxx>
#include <svx/svdoattr.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/sdtmfitm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/fontworkbar.hxx>

#include <editeng/escpitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/shdditem.hxx>
#include <svx/xtable.hxx>
#include <svx/svdobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brshitem.hxx>

#include <svl/whiter.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <sfx2/viewfrm.hxx>
#include "sdgrffilter.hxx"

#include "app.hrc"
#include "glob.hrc"
#include "helpids.h"
#include "sdattr.hxx"
#include "drawview.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#include "fuscale.hxx"
#include "sdresid.hxx"
#include "GraphicViewShell.hxx"
#include "unmodpg.hxx"
#include "slideshow.hxx"
#include "fuvect.hxx"
#include "futext.hxx"
#include "stlpool.hxx"

// #90356#
#include "optsitem.hxx"
#include "sdabstdlg.hxx"
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>

#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void DrawViewShell::FuTemporary(SfxRequest& rReq)
{
    // Waehrend einer Native-Diashow wird nichts ausgefuehrt!
    if(SlideShow::IsRunning( GetViewShellBase() ) && (rReq.GetSlot() != SID_NAVIGATOR))
        return;

    DBG_ASSERT( mpDrawView, "sd::DrawViewShell::FuTemporary(), no draw view!" );
    if( !mpDrawView )
        return;

    CheckLineTo (rReq);

    DeactivateCurrentFunction();

    sal_uInt16 nSId = rReq.GetSlot();

    // Slot wird gemapped (ToolboxImages/-Slots)
    MapSlot( nSId );

    switch ( nSId )
    {
        // Flaechen und Linien-Attribute:
        // Sollten (wie StateMethode) eine eigene
        // Execute-Methode besitzen
        case SID_ATTR_FILL_STYLE:
        case SID_ATTR_FILL_COLOR:
        case SID_ATTR_FILL_GRADIENT:
        case SID_ATTR_FILL_HATCH:
        case SID_ATTR_FILL_BITMAP:
        case SID_ATTR_FILL_SHADOW:
        case SID_ATTR_FILL_TRANSPARENCE:
        case SID_ATTR_FILL_FLOATTRANSPARENCE:

        case SID_ATTR_LINE_STYLE:
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_WIDTH:
        case SID_ATTR_LINE_COLOR:
        case SID_ATTR_LINEEND_STYLE:
        case SID_ATTR_LINE_START:
        case SID_ATTR_LINE_END:
        case SID_ATTR_LINE_TRANSPARENCE:
        case SID_ATTR_LINE_JOINT:
        case SID_ATTR_LINE_CAP:

        case SID_ATTR_TEXT_FITTOSIZE:
        {
            if( rReq.GetArgs() )
            {
                sal_Bool bMergeUndo = sal_False;
                ::svl::IUndoManager* pUndoManager = GetDocSh()->GetUndoManager();

                // Anpassungen Start/EndWidth #63083#
                if(nSId == SID_ATTR_LINE_WIDTH)
                {
                    SdrObject* pObj = NULL;
                    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                    sal_uLong nCount = rMarkList.GetMarkCount();
                    const sal_Int32 nNewLineWidth(((const XLineWidthItem&)rReq.GetArgs()->Get(XATTR_LINEWIDTH)).GetValue());
                    const bool bUndo(mpDrawView->IsUndoEnabled());

                    for (sal_uLong i=0; i<nCount; i++)
                    {
                        SfxItemSet aAttr(GetDoc()->GetPool());
                        pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                        aAttr.Put(pObj->GetMergedItemSet());

                        sal_Int32 nActLineWidth = ((const XLineWidthItem&)aAttr.Get(XATTR_LINEWIDTH)).GetValue();

                        if(nActLineWidth != nNewLineWidth)
                        {
                            sal_Bool bSetItemSet(sal_False);

                            // #86265# do this for SFX_ITEM_DEFAULT and for SFX_ITEM_SET
                            if(SFX_ITEM_DONTCARE != aAttr.GetItemState(XATTR_LINESTARTWIDTH))
                            {
                                sal_Int32 nValAct = ((const XLineStartWidthItem&)aAttr.Get(XATTR_LINESTARTWIDTH)).GetValue();
                                sal_Int32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
                                if(nValNew < 0)
                                    nValNew = 0;
                                bSetItemSet = sal_True;
                                aAttr.Put(XLineStartWidthItem(nValNew));
                            }

                            // #86265# do this for SFX_ITEM_DEFAULT and for SFX_ITEM_SET
                            if(SFX_ITEM_DONTCARE != aAttr.GetItemState(XATTR_LINEENDWIDTH))
                            {
                                sal_Int32 nValAct = ((const XLineEndWidthItem&)aAttr.Get(XATTR_LINEENDWIDTH)).GetValue();
                                sal_Int32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
                                if(nValNew < 0)
                                    nValNew = 0;
                                bSetItemSet = sal_True;
                                aAttr.Put(XLineEndWidthItem(nValNew));
                            }

                            if(bSetItemSet)
                            {
                                if(bUndo)
                                {
                                    if(!bMergeUndo)
                                    {
                                        pUndoManager->EnterListAction( String(), String() );
                                        mpDrawView->BegUndo();
                                        bMergeUndo = sal_True;
                                    }

                                    mpDrawView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));
                                }

                                pObj->SetMergedItemSet(aAttr);
                            }
                        }
                    }

                    if(bMergeUndo)
                    {
                        mpDrawView->EndUndo();
                    }
                }

                if (nSId == SID_ATTR_FILL_SHADOW)
                {
                    // Ggf. werden transparente Objekte wei?gefuellt
                    SdrObject* pObj = NULL;
                    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                    sal_uLong nCount = rMarkList.GetMarkCount();
                    const bool bUndo = mpDrawView->IsUndoEnabled();

                    for (sal_uLong i=0; i<nCount; i++)
                    {
                        SfxItemSet aAttr(GetDoc()->GetPool());
                        pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                        // #i25616#
                        if(!pObj->ISA(SdrGrafObj))
                        {
                            aAttr.Put(pObj->GetMergedItemSet());

                            const XFillStyleItem& rFillStyle = (const XFillStyleItem&) aAttr.Get(XATTR_FILLSTYLE);
                            const XLineStyleItem& rLineStyle = (const XLineStyleItem&) aAttr.Get(XATTR_LINESTYLE);

                            if(XFILL_NONE == rFillStyle.GetValue() && XLINE_NONE == rLineStyle.GetValue())
                            {
                                if( bUndo )
                                {
                                    // Vorlage hat keine Fuellung,
                                    // daher hart attributieren: Fuellung setzen
                                    if (!bMergeUndo)
                                    {
                                        bMergeUndo = sal_True;
                                        pUndoManager->EnterListAction( String(), String() );
                                        mpDrawView->BegUndo();
                                    }

                                    mpDrawView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));
                                }

                                aAttr.Put(XFillStyleItem(XFILL_SOLID));
                                aAttr.Put(XFillColorItem(String(), COL_WHITE));

                                pObj->SetMergedItemSet(aAttr);
                            }
                        }
                    }

                    if (bMergeUndo)
                    {
                        mpDrawView->EndUndo();
                    }
                }

                mpDrawView->SetAttributes(*rReq.GetArgs());

                if (bMergeUndo)
                {
                    pUndoManager->LeaveListAction();
                }

                rReq.Done();
            }
            else
            {
                switch( rReq.GetSlot() )
                {
                    case SID_ATTR_FILL_SHADOW:
                    case SID_ATTR_FILL_STYLE:
                    case SID_ATTR_FILL_COLOR:
                    case SID_ATTR_FILL_GRADIENT:
                    case SID_ATTR_FILL_HATCH:
                    case SID_ATTR_FILL_BITMAP:
                    case SID_ATTR_FILL_TRANSPARENCE:
                    case SID_ATTR_FILL_FLOATTRANSPARENCE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_AREA, SFX_CALLMODE_ASYNCHRON );
                        break;
                    case SID_ATTR_LINE_STYLE:
                    case SID_ATTR_LINE_DASH:
                    case SID_ATTR_LINE_WIDTH:
                    case SID_ATTR_LINE_COLOR:
                    case SID_ATTR_LINE_TRANSPARENCE:
                    case SID_ATTR_LINE_JOINT:
                    case SID_ATTR_LINE_CAP:
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_LINE, SFX_CALLMODE_ASYNCHRON );
                        break;
                    case SID_ATTR_TEXT_FITTOSIZE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_TEXTATTR_DLG, SFX_CALLMODE_ASYNCHRON );
                        break;
                }
            }
            Cancel();
        }
        break;

        case SID_HYPHENATION:
        {
            // const SfxPoolItem* pItem = rReq.GetArg( SID_HYPHENATION );
            //  ^-- Soll so nicht benutzt werden (Defaults sind falsch) !
            SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, SID_HYPHENATION, sal_False);

            if( pItem )
            {
                SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
                sal_Bool bValue = ( (const SfxBoolItem*) pItem)->GetValue();
                aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                mpDrawView->SetAttributes( aSet );
            }
            else // nur zum Test
            {
                DBG_ERROR(" Kein Wert fuer Silbentrennung!");
                SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
                sal_Bool bValue = sal_True;
                aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                mpDrawView->SetAttributes( aSet );
            }
            rReq.Done();
            Cancel();
        }
        break;

        case SID_INSERTPAGE:
        case SID_INSERTPAGE_QUICK:
        case SID_DUPLICATE_PAGE:
        {
            SdPage* pNewPage = CreateOrDuplicatePage (rReq, mePageKind, GetActualPage());
            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT) )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            if (pNewPage != NULL)
                SwitchPage((pNewPage->GetPageNum()-1)/2);
            rReq.Done ();
        }
        break;

        case SID_INSERT_MASTER_PAGE:
        {
            // Use the API to create a new page.
            Reference<drawing::XMasterPagesSupplier> xMasterPagesSupplier (
                GetDoc()->getUnoModel(), UNO_QUERY);
            if (xMasterPagesSupplier.is())
            {
                Reference<drawing::XDrawPages> xMasterPages (
                    xMasterPagesSupplier->getMasterPages());
                if (xMasterPages.is())
                {
                    sal_uInt16 nIndex = GetCurPageId();
                    xMasterPages->insertNewByIndex (nIndex);

                    // Create shapes for the default layout.
                    SdPage* pMasterPage = GetDoc()->GetMasterSdPage(
                        nIndex, PK_STANDARD);
                    pMasterPage->CreateTitleAndLayout (sal_True,sal_True);
                }
            }

            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT))
                GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            rReq.Done ();
        }
        break;

        case SID_MODIFYPAGE:
        {
            if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES ||
                (mePageKind==PK_HANDOUT && meEditMode==EM_MASTERPAGE) )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }
                sal_uInt16 nPage = maTabControl.GetCurPageId() - 1;
                mpActualPage = GetDoc()->GetSdPage(nPage, mePageKind);
                ::sd::ViewShell::mpImpl->ProcessModifyPageSlot (
                    rReq,
                    mpActualPage,
                    mePageKind);
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_ASSIGN_LAYOUT:
        {
            if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES || (mePageKind==PK_HANDOUT && meEditMode==EM_MASTERPAGE))
            {
                if ( mpDrawView->IsTextEdit() )
                    mpDrawView->SdrEndTextEdit();

                ::sd::ViewShell::mpImpl->AssignLayout(rReq, mePageKind);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_RENAMEPAGE:
        case SID_RENAME_MASTER_PAGE:
        {
            if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                sal_uInt16 nPageId = maTabControl.GetCurPageId();
                SdPage* pCurrentPage = ( GetEditMode() == EM_PAGE )
                    ? GetDoc()->GetSdPage( nPageId - 1, GetPageKind() )
                    : GetDoc()->GetMasterSdPage( nPageId - 1, GetPageKind() );

                String aTitle( SdResId( STR_TITLE_RENAMESLIDE ) );
                String aDescr( SdResId( STR_DESC_RENAMESLIDE ) );
                String aPageName = pCurrentPage->GetName();

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialogdiet fail!");
                AbstractSvxNameDialog* aNameDlg = pFact->CreateSvxNameDialog( GetActiveWindow(), aPageName, aDescr );
                DBG_ASSERT(aNameDlg, "Dialogdiet fail!");
                aNameDlg->SetText( aTitle );
                aNameDlg->SetCheckNameHdl( LINK( this, DrawViewShell, RenameSlideHdl ), true );
                aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

                if( aNameDlg->Execute() == RET_OK )
                {
                    String aNewName;
                    aNameDlg->GetName( aNewName );
                    if( ! aNewName.Equals( aPageName ) )
                    {
#ifdef DBG_UTIL
                        bool bResult =
#endif
                            RenameSlide( nPageId, aNewName );
                        DBG_ASSERT( bResult, "Couldn't rename slide" );
                    }
                }
                delete aNameDlg;
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_RENAMEPAGE_QUICK:
        {
            if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                maTabControl.StartEditMode( maTabControl.GetCurPageId() );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_PAGESIZE :  // entweder dieses (kein menueeintrag o. ae. !!)
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if (pArgs)
                if (pArgs->Count () == 3)
                {
                    SFX_REQUEST_ARG (rReq, pWidth, SfxUInt32Item, ID_VAL_PAGEWIDTH, sal_False);
                    SFX_REQUEST_ARG (rReq, pHeight, SfxUInt32Item, ID_VAL_PAGEHEIGHT, sal_False);
                    SFX_REQUEST_ARG (rReq, pScaleAll, SfxBoolItem, ID_VAL_SCALEOBJECTS, sal_False);

                    Size aSize (pWidth->GetValue (), pHeight->GetValue ());

                    SetupPage (aSize, 0, 0, 0, 0, sal_True, sal_False, pScaleAll->GetValue ());
                    rReq.Ignore ();
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            rReq.Ignore ();
            break;
        }

        case SID_PAGEMARGIN :  // oder dieses (kein menueeintrag o. ae. !!)
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if (pArgs)
                if (pArgs->Count () == 5)
                {
                    SFX_REQUEST_ARG (rReq, pLeft, SfxUInt32Item, ID_VAL_PAGELEFT, sal_False);
                    SFX_REQUEST_ARG (rReq, pRight, SfxUInt32Item, ID_VAL_PAGERIGHT, sal_False);
                    SFX_REQUEST_ARG (rReq, pUpper, SfxUInt32Item, ID_VAL_PAGETOP, sal_False);
                    SFX_REQUEST_ARG (rReq, pLower, SfxUInt32Item, ID_VAL_PAGEBOTTOM, sal_False);
                    SFX_REQUEST_ARG (rReq, pScaleAll, SfxBoolItem, ID_VAL_SCALEOBJECTS, sal_False);

                    Size aEmptySize (0, 0);

                    SetupPage (aEmptySize, pLeft->GetValue (), pRight->GetValue (),
                               pUpper->GetValue (), pLower->GetValue (),
                               sal_False, sal_True, pScaleAll->GetValue ());
                    rReq.Ignore ();
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            rReq.Ignore ();
            break;
        }

        case SID_ATTR_ZOOMSLIDER:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (pArgs && pArgs->Count () == 1 )
            {
                SFX_REQUEST_ARG (rReq, pScale, SfxUInt16Item, SID_ATTR_ZOOMSLIDER, sal_False);
                if (CHECK_RANGE (5, pScale->GetValue (), 3000))
                {
                    SetZoom (pScale->GetValue ());

                    SfxBindings& rBindings = GetViewFrame()->GetBindings();
                    rBindings.Invalidate( SID_ATTR_ZOOM );
                    rBindings.Invalidate( SID_ZOOM_IN );
                    rBindings.Invalidate( SID_ZOOM_OUT );
                    rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );

                }
            }

            Cancel();
            rReq.Done ();
            break;
        }
        case SID_ZOOMING :  // kein Menueintrag, sondern aus dem Zoomdialog generiert
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pScale, SfxUInt32Item, ID_VAL_ZOOM, sal_False);
                    if (CHECK_RANGE (10, pScale->GetValue (), 1000))
                    {
                        SetZoom (pScale->GetValue ());

                        SfxBindings& rBindings = GetViewFrame()->GetBindings();
                        rBindings.Invalidate( SID_ATTR_ZOOM );
                        rBindings.Invalidate( SID_ZOOM_IN );
                        rBindings.Invalidate( SID_ZOOM_OUT );
                        rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );
                    }
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

                    rReq.Ignore ();
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            rReq.Ignore ();
            break;
        }

        case SID_ATTR_ZOOM:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            mbZoomOnPage = sal_False;

            if ( pArgs )
            {
                SvxZoomType eZT = ( ( const SvxZoomItem& ) pArgs->
                                            Get( SID_ATTR_ZOOM ) ).GetType();
                switch( eZT )
                {
                    case SVX_ZOOM_PERCENT:
                        SetZoom( (long) ( ( const SvxZoomItem& ) pArgs->
                                            Get( SID_ATTR_ZOOM ) ).GetValue() );
                        break;

                    case SVX_ZOOM_OPTIMAL:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                        break;

                    case SVX_ZOOM_PAGEWIDTH:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                        break;

                    case SVX_ZOOM_WHOLEPAGE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                        break;
                    case SVX_ZOOM_PAGEWIDTH_NOBORDER:
                        DBG_ERROR("sd::DrawViewShell::FuTemporary(), SVX_ZOOM_PAGEWIDTH_NOBORDER not handled!" );
                        break;
                }
                rReq.Ignore ();
            }
            else
            {
                // hier den Zoom-Dialog oeffnen
                SetCurrentFunction( FuScale::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            }
            Cancel();
        }
        break;

        case SID_CHANGEBEZIER:
        case SID_CHANGEPOLYGON:
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                if( rReq.GetSlot() == SID_CHANGEBEZIER )
                {
                    WaitObject aWait( (Window*)GetActiveWindow() );
                    mpDrawView->ConvertMarkedToPathObj(sal_False);
                }
                else
                {
                    if( mpDrawView->IsVectorizeAllowed() )
                    {
                        SetCurrentFunction( FuVectorize::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
                    }
                    else
                    {
                        WaitObject aWait( (Window*)GetActiveWindow() );
                        mpDrawView->ConvertMarkedToPolyObj(sal_False);
                    }
                }

                Invalidate(SID_CHANGEBEZIER);
                Invalidate(SID_CHANGEPOLYGON);
            }
            Cancel();

            if( HasCurrentFunction(SID_BEZIER_EDIT) )
            {   // ggf. die richtige Editfunktion aktivieren
                GetViewFrame()->GetDispatcher()->Execute(SID_SWITCH_POINTEDIT,
                                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
            rReq.Ignore ();
            break;

        case SID_CONVERT_TO_CONTOUR:
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                mpDrawView->ConvertMarkedToPathObj(sal_True);

                Invalidate(SID_CONVERT_TO_CONTOUR);
            }
            Cancel();

            rReq.Ignore ();
            break;

        case SID_CONVERT_TO_METAFILE:
        case SID_CONVERT_TO_BITMAP:
        {
            // End text edit mode when it is active because the metafile or
            // bitmap that will be created does not support it.
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected(true,true,true) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );

                // switch on undo for the next operations
                mpDrawView->BegUndo(
                    String(
                    SdResId (nSId==SID_CONVERT_TO_METAFILE ? STR_UNDO_CONVERT_TO_METAFILE : STR_UNDO_CONVERT_TO_BITMAP)));

                // create SdrGrafObj from metafile/bitmap
                Graphic aGraphic;
                switch (nSId)
                {
                    case SID_CONVERT_TO_METAFILE:
                    {
                        GDIMetaFile aMetaFile(mpDrawView->GetMarkedObjMetaFile());
                        aGraphic = Graphic(aMetaFile);
                    }
                    break;
                    case SID_CONVERT_TO_BITMAP:
                    {
                        bool bDone(false);

                        // I have to get the image here directly since GetMarkedObjBitmapEx works
                        // based on Bitmaps, but not on BitmapEx, thus throwing away the alpha
                        // channel. Argh! GetMarkedObjBitmapEx itself is too widely used to safely
                        // change that, e.g. in the exchange formats. For now I can only add this
                        // exception to get good results for Svgs. This is how the code gets more
                        // and more crowded, at last I made a remark for myself to change this
                        // as one of the next tasks.
                        if(1 == mpDrawView->GetMarkedObjectCount())
                        {
                            const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(mpDrawView->GetMarkedObjectByIndex(0));

                            if(pSdrGrafObj && pSdrGrafObj->isEmbeddedSvg())
                            {
                                aGraphic = Graphic(pSdrGrafObj->GetGraphic().getSvgData()->getReplacement());
                                bDone = true;
                            }
                        }

                        if(!bDone)
                        {
                            aGraphic = Graphic(mpDrawView->GetMarkedObjBitmapEx());
                        }
                    }
                    break;
                }

                // create new object
                SdrGrafObj* pGraphicObj = new SdrGrafObj (aGraphic);

                // get some necessary info and ensure it
                const SdrMarkList& rMarkList(mpDrawView->GetMarkedObjectList());
                const sal_uInt32 nMarkCount(rMarkList.GetMarkCount());
                SdrPageView* pPageView = mpDrawView->GetSdrPageView();
                OSL_ENSURE(nMarkCount, "DrawViewShell::FuTemporary: SID_CONVERT_TO_BITMAP with empty selection (!)");
                OSL_ENSURE(pPageView, "DrawViewShell::FuTemporary: SID_CONVERT_TO_BITMAP without SdrPageView (!)");

                // fit rectangle of new graphic object to selection's mark rect
                Rectangle aAllMarkedRect;
                rMarkList.TakeBoundRect(pPageView, aAllMarkedRect);
                pGraphicObj->SetLogicRect(aAllMarkedRect);

                // #i71540# to keep the order, it is necessary to replace the lowest object
                // of the selection with the new object. This also means that with multi
                // selection, all other objects need to be deleted first
                SdrMark* pFirstMark = rMarkList.GetMark(0L);
                SdrObject* pReplacementCandidate = pFirstMark->GetMarkedSdrObj();

                if(nMarkCount > 1L)
                {
                    // take first object out of selection
                    mpDrawView->MarkObj(pReplacementCandidate, pPageView, true, true);

                    // clear remaining selection
                    mpDrawView->DeleteMarkedObj();
                }

                // now replace lowest object with new one
                mpDrawView->ReplaceObjectAtView(pReplacementCandidate, *pPageView, pGraphicObj);

                // switch off undo
                mpDrawView->EndUndo();
            }
        }

        Cancel();

        rReq.Done ();
        break;

        case SID_SET_DEFAULT:
        {
            SfxItemSet* pSet = NULL;

            if (mpDrawView->IsTextEdit())
            {
                ::Outliner* pOutl = mpDrawView->GetTextEditOutliner();
                if (pOutl)
                {
                    pOutl->RemoveFields(sal_True, (TypeId) SvxURLField::StaticType());
                }

                pSet = new SfxItemSet( GetPool(), EE_ITEMS_START, EE_ITEMS_END );
                mpDrawView->SetAttributes( *pSet, sal_True );
            }
            else
            {
                const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                sal_uLong nCount = rMarkList.GetMarkCount();

                // In diese Liste werden fuer jedes Praesentationsobjekt ein SfxItemSet
                // der harten Attribute sowie der UserCall eingetragen, da diese beim nachfolgenden
                // mpDrawView->SetAttributes( *pSet, sal_True ) verloren gehen und spaeter restauriert
                // werden muessen
                List* pAttrList = new List();
                SdPage* pPresPage = (SdPage*) mpDrawView->GetSdrPageView()->GetPage();
                sal_uLong i;

                for ( i = 0; i < nCount; i++ )
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        SfxItemSet* pNewSet = new SfxItemSet( GetDoc()->GetPool(), SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT, 0 );
                        pNewSet->Put(pObj->GetMergedItemSet());
                        pAttrList->Insert( pNewSet, LIST_APPEND );
                        pAttrList->Insert( pObj->GetUserCall(), LIST_APPEND );
                    }
                }

                pSet = new SfxItemSet( GetPool() );
                mpDrawView->SetAttributes( *pSet, sal_True );

                sal_uLong j = 0;

                for ( i = 0; i < nCount; i++ )
                {
                    SfxStyleSheet* pSheet = NULL;
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                    if (pObj->GetObjIdentifier() == OBJ_TITLETEXT)
                    {
                        pSheet = mpActualPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
                        if (pSheet)
                            pObj->SetStyleSheet(pSheet, sal_False);
                    }
                    else if(pObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
                    {
                        for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
                        {
                            pSheet = mpActualPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
                            DBG_ASSERT(pSheet, "Vorlage fuer Gliederungsobjekt nicht gefunden");
                            if (pSheet)
                            {
                                pObj->StartListening(*pSheet);

                                if( nLevel == 1 )
                                    // Textrahmen hoert auf StyleSheet der Ebene1
                                    pObj->NbcSetStyleSheet(pSheet, sal_False);

                            }
                        }
                    }

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        SfxItemSet* pNewSet = (SfxItemSet*) pAttrList->GetObject(j++);
                        SdrObjUserCall* pUserCall = (SdrObjUserCall*) pAttrList->GetObject(j++);

                        if ( pNewSet && pNewSet->GetItemState( SDRATTR_TEXT_MINFRAMEHEIGHT ) == SFX_ITEM_ON )
                        {
                            pObj->SetMergedItem(pNewSet->Get(SDRATTR_TEXT_MINFRAMEHEIGHT));
                        }

                        if ( pNewSet && pNewSet->GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) == SFX_ITEM_ON )
                        {
                            pObj->SetMergedItem(pNewSet->Get(SDRATTR_TEXT_AUTOGROWHEIGHT));
                        }

                        if( pUserCall )
                            pObj->SetUserCall( pUserCall );

                        delete pNewSet;
                    }
                }

                delete pAttrList;
            }

            delete pSet;
            Cancel();
        }
        break;

        case SID_DELETE_SNAPITEM:
        {
            SdrPageView* pPV;
            Point   aMPos = GetActiveWindow()->PixelToLogic( maMousePos );
            sal_uInt16  nHitLog = (sal_uInt16) GetActiveWindow()->PixelToLogic( Size(
                FuPoor::HITPIX, 0 ) ).Width();
            sal_uInt16  nHelpLine;

            mbMousePosFreezed = sal_False;

            if( mpDrawView->PickHelpLine( aMPos, nHitLog, *GetActiveWindow(), nHelpLine, pPV) )
            {
                pPV->DeleteHelpLine( nHelpLine );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_DELETE_PAGE:
        case SID_DELETE_MASTER_PAGE:
            DeleteActualPage();
            Cancel();
            rReq.Ignore ();
        break;

        case SID_DELETE_LAYER:
            DeleteActualLayer();
            Cancel();
            rReq.Ignore ();
        break;

        case SID_ORIGINAL_SIZE:
            mpDrawView->SetMarkedOriginalSize();
            Cancel();
            rReq.Done();
        break;

        case SID_DRAW_FONTWORK:
        case SID_DRAW_FONTWORK_VERTICAL:
        {
            svx::FontworkBar::execute( mpView, rReq, GetViewFrame()->GetBindings() );       // SJ: can be removed  (I think)
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SAVEGRAPHIC:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrGrafObj *pGrafObj = dynamic_cast< SdrGrafObj* >( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );
                if(pGrafObj )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape( pGrafObj->getUnoShape(), com::sun::star::uno::UNO_QUERY );
                    SdGRFFilter::SaveGraphic( xShape );
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        default:
        {
            // switch Anweisung wegen CLOOKS aufgeteilt. Alle case-Anweisungen die
            // eine Fu???? -Funktion aufrufen, sind in die Methode FuTemp01 (drviews8)
            // gewandert.
            FuTemp01(rReq);
        }
        break;
    }

    if(HasCurrentFunction())
    {
        GetCurrentFunction()->Activate();
    }
}

void DrawViewShell::ExecChar( SfxRequest &rReq )
{
    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
    return;

    SfxItemSet aEditAttr( pDoc->GetPool() );
    mpDrawView->GetAttributes( aEditAttr );

    //modified by wj for sym2_1580, if put old itemset into new set,
    //when mpDrawView->SetAttributes(aNewAttr) it will invalidate all the item
    // and use old attr to update all the attributes
//  SfxItemSet aNewAttr( GetPool(),
//  EE_ITEMS_START, EE_ITEMS_END );
//  aNewAttr.Put( aEditAttr, sal_False );
    SfxItemSet aNewAttr( pDoc->GetPool() );
    //modified end

    sal_uInt16 nSId = rReq.GetSlot();

    MapSlot( nSId );

    switch ( nSId )
    {
    case SID_ATTR_CHAR_FONT:
        if( rReq.GetArgs() )
        {
            SFX_REQUEST_ARG( rReq, pItem, SvxFontItem, SID_ATTR_CHAR_FONT , sal_False );
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_FONTHEIGHT:
        if( rReq.GetArgs() )
        {
            SFX_REQUEST_ARG( rReq, pItem, SvxFontHeightItem, SID_ATTR_CHAR_FONTHEIGHT , sal_False );
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_WEIGHT:
        if( rReq.GetArgs() )
        {
            //const SvxWeightItem *pItem = (const SvxWeightItem*) rReq.GetArg( SID_ATTR_CHAR_WEIGHT, sal_False, TYPE(SvxWeightItem) );
            SFX_REQUEST_ARG( rReq, pItem, SvxWeightItem, SID_ATTR_CHAR_WEIGHT , sal_False );
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_POSTURE:
        if( rReq.GetArgs() )
        {
            //const SvxPostureItem *pItem = (const SvxPostureItem*) rReq.GetArg( SID_ATTR_CHAR_POSTURE, sal_False, TYPE(SvxPostureItem) );
            SFX_REQUEST_ARG( rReq, pItem, SvxPostureItem, SID_ATTR_CHAR_POSTURE , sal_False );
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_UNDERLINE:
        if( rReq.GetArgs() )
        {
            //<<modify by wj for sym2_1873
            //SFX_REQUEST_ARG( rReq, pItem, SvxTextLineItem, SID_ATTR_CHAR_UNDERLINE , sal_False );
            SFX_REQUEST_ARG( rReq, pItem, SvxUnderlineItem, SID_ATTR_CHAR_UNDERLINE , sal_False );
            //end>>
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
            else
            {
                FontUnderline eFU = ( (const SvxUnderlineItem&) aEditAttr.Get( EE_CHAR_UNDERLINE ) ).GetLineStyle();
                aNewAttr.Put( SvxUnderlineItem( eFU != UNDERLINE_NONE ?UNDERLINE_NONE : UNDERLINE_SINGLE,  EE_CHAR_UNDERLINE ) );
            }//aNewAttr.Put( (const SvxUnderlineItem&)aEditAttr.Get( EE_CHAR_UNDERLINE ) );
        }
        break;
    case SID_ATTR_CHAR_SHADOWED:
        if( rReq.GetArgs() )
        {
            SFX_REQUEST_ARG( rReq, pItem, SvxShadowedItem, SID_ATTR_CHAR_SHADOWED , sal_False );
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_STRIKEOUT:
        if( rReq.GetArgs() )
        {
            SFX_REQUEST_ARG( rReq, pItem, SvxCrossedOutItem, SID_ATTR_CHAR_STRIKEOUT , sal_False );
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_COLOR:
        if( rReq.GetArgs() )
        {
            SFX_REQUEST_ARG( rReq, pItem, SvxColorItem, SID_ATTR_CHAR_COLOR , sal_False );
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_KERNING:
        if( rReq.GetArgs() )
        {
            SFX_REQUEST_ARG( rReq, pItem, SvxKerningItem, SID_ATTR_CHAR_KERNING , sal_False );
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_SET_SUB_SCRIPT:
        if( rReq.GetArgs() )
        {
            SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
            SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();
            if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUBSCRIPT );
            aNewAttr.Put( aItem );
        }
        break;
    case SID_SET_SUPER_SCRIPT:
        if( rReq.GetArgs() )
        {
            SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
            SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();
            if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
            aNewAttr.Put( aItem );
        }
        break;
    case SID_SHRINK_FONT_SIZE:
        case SID_GROW_FONT_SIZE:
        //if (rReq.GetArgs())
        {
            const SvxFontListItem* pFonts = dynamic_cast<const SvxFontListItem*>(GetDocSh()->GetItem( SID_ATTR_CHAR_FONTLIST ) );
            const FontList* pFontList = pFonts->GetFontList();
            if( pFontList )
            {
                FuText::ChangeFontSize( nSId == SID_GROW_FONT_SIZE, NULL, pFontList, mpView );
                GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
        }
    default:
        ;
    }

    mpDrawView->SetAttributes(aNewAttr);
    rReq.Done();
    Cancel();
}


/** This method consists basically of three parts:
    1. Process the arguments of the SFX request.
    2. Use the model to create a new page or duplicate an existing one.
    3. Update the tab control and switch to the new page.
*/
SdPage* DrawViewShell::CreateOrDuplicatePage (
    SfxRequest& rRequest,
    PageKind ePageKind,
    SdPage* pPage,
    const sal_Int32 nInsertPosition)
{
    SdPage* pNewPage = NULL;
    if (ePageKind == PK_STANDARD && meEditMode != EM_MASTERPAGE)
    {
        if ( mpDrawView->IsTextEdit() )
        {
            mpDrawView->SdrEndTextEdit();
        }
        pNewPage = ViewShell::CreateOrDuplicatePage (rRequest, ePageKind, pPage, nInsertPosition);
    }
    return pNewPage;
}

void DrawViewShell::ExecutePropPanelAttr (SfxRequest& rReq)
{
    if(SlideShow::IsRunning( GetViewShellBase() ))
        return;

    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    sal_uInt16 nSId = rReq.GetSlot();
    SfxItemSet aAttrs( pDoc->GetPool() );

    switch ( nSId )
    {
    case SID_TABLE_VERT_NONE:
    case SID_TABLE_VERT_CENTER:
    case SID_TABLE_VERT_BOTTOM:
        SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_TOP;
        if (nSId == SID_TABLE_VERT_CENTER)
            eTVA = SDRTEXTVERTADJUST_CENTER;
        else if (nSId == SID_TABLE_VERT_BOTTOM)
            eTVA = SDRTEXTVERTADJUST_BOTTOM;

        aAttrs.Put( SdrTextVertAdjustItem(eTVA) );
        mpDrawView->SetAttributes(aAttrs);

        break;
    }
}

void DrawViewShell::GetStatePropPanelAttr(SfxItemSet& rSet)
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16          nWhich = aIter.FirstWhich();

    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    SfxItemSet aAttrs( pDoc->GetPool() );
    mpDrawView->GetAttributes( aAttrs );

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;
        switch ( nSlotId )
        {
            case SID_TABLE_VERT_NONE:
            case SID_TABLE_VERT_CENTER:
            case SID_TABLE_VERT_BOTTOM:
                sal_Bool bContour = sal_False;
                SfxItemState eConState = aAttrs.GetItemState( SDRATTR_TEXT_CONTOURFRAME );
                if( eConState != SFX_ITEM_DONTCARE )
                {
                    bContour = ( ( const SdrTextContourFrameItem& )aAttrs.Get( SDRATTR_TEXT_CONTOURFRAME ) ).GetValue();
                }
                if (bContour) break;

                SfxItemState eVState = aAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
                //SfxItemState eHState = aAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );

                //if(SFX_ITEM_DONTCARE != eVState && SFX_ITEM_DONTCARE != eHState)
                if(SFX_ITEM_DONTCARE != eVState)
                {
                    SdrTextVertAdjust eTVA = (SdrTextVertAdjust)((const SdrTextVertAdjustItem&)aAttrs.Get(SDRATTR_TEXT_VERTADJUST)).GetValue();
                    sal_Bool bSet = nSlotId == SID_TABLE_VERT_NONE && eTVA == SDRTEXTVERTADJUST_TOP||
                            nSlotId == SID_TABLE_VERT_CENTER && eTVA == SDRTEXTVERTADJUST_CENTER ||
                            nSlotId == SID_TABLE_VERT_BOTTOM && eTVA == SDRTEXTVERTADJUST_BOTTOM;
                    rSet.Put(SfxBoolItem(nSlotId, bSet));
                }
                else
                {
                    rSet.Put(SfxBoolItem(nSlotId, sal_False));
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

} // end of namespace sd
