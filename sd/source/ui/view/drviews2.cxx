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


#include "DrawViewShell.hxx"
#include "ViewShellImplementation.hxx"
#include <vcl/waitobj.hxx>
#include <svx/svdograf.hxx>
#include <svx/svxids.hrc>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/zoomitem.hxx>
#include <editeng/editdata.hxx>
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
#include <svx/graphichelper.hxx>
#include <svx/compressgraphicdialog.hxx>
#include <svx/extedit.hxx>
#include <svx/svdoutl.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/svdoattr.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/sdtmfitm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/fontworkbar.hxx>

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
#include "stlpool.hxx"

#include "optsitem.hxx"
#include "sdabstdlg.hxx"
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>

#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

class SdExternalToolEdit : public ExternalToolEdit
{
    FmFormView* m_pView;
    SdrObject*  m_pObj;

public:
    SdExternalToolEdit ( FmFormView* pView, SdrObject* pObj ) :
        m_pView   (pView),
        m_pObj (pObj)
    {}

    virtual void Update( Graphic& aGraphic )
    {
        SdrPageView* pPageView = m_pView->GetSdrPageView();
        if( pPageView )
        {
            SdrGrafObj* pNewObj = (SdrGrafObj*) m_pObj->Clone();
            String      aStr( m_pView->GetDescriptionOfMarkedObjects() );
            aStr.Append( sal_Unicode(' ') );
            aStr.Append( String( "External Edit" ) );
            m_pView->BegUndo( aStr );
            pNewObj->SetGraphicObject( aGraphic );
            m_pView->ReplaceObjectAtView( m_pObj, *pPageView, pNewObj );
            m_pView->EndUndo();
        }
    }
};

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
        case SID_OUTLINE_TEXT_AUTOFIT:
        {
            ::svl::IUndoManager* pUndoManager = GetDocSh()->GetUndoManager();
            SdrObject* pObj = NULL;
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                pUndoManager->EnterListAction( String(), String() );
                mpDrawView->BegUndo();

                pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                bool bSet = ((const SdrTextFitToSizeTypeItem*)pObj->GetMergedItemSet().GetItem(SDRATTR_TEXT_FITTOSIZE))->GetValue() != SDRTEXTFIT_NONE;

                mpDrawView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));

                pObj->SetMergedItem(SdrTextFitToSizeTypeItem(bSet ? SDRTEXTFIT_NONE : SDRTEXTFIT_AUTOFIT));

                mpDrawView->EndUndo();
                pUndoManager->LeaveListAction();
            }
            Cancel();
            rReq.Done();
        }
        break;

        // Flaechen und Linien-Attribute:
        // Sollten (wie StateMethode) eine eigene
        // Execute-Methode besitzen
        case SID_ATTR_FILL_STYLE:
        case SID_ATTR_FILL_COLOR:
        case SID_ATTR_FILL_GRADIENT:
        case SID_ATTR_FILL_HATCH:
        case SID_ATTR_FILL_BITMAP:
        case SID_ATTR_FILL_SHADOW:

        case SID_ATTR_LINE_STYLE:
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_WIDTH:
        case SID_ATTR_LINE_COLOR:
        case SID_ATTR_LINEEND_STYLE:

        case SID_ATTR_TEXT_FITTOSIZE:
        {
            if( rReq.GetArgs() )
            {
                sal_Bool bMergeUndo = sal_False;
                ::svl::IUndoManager* pUndoManager = GetDocSh()->GetUndoManager();

                // Anpassungen Start/EndWidth
                if(nSId == SID_ATTR_LINE_WIDTH)
                {
                    SdrObject* pObj = NULL;
                    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                    sal_uLong nCount = rMarkList.GetMarkCount();

                    sal_Int32 nNewLineWidth = ((const XLineWidthItem&)rReq.GetArgs()->Get(XATTR_LINEWIDTH)).GetValue();

                    for (sal_uLong i=0; i<nCount; i++)
                    {
                        SfxItemSet aAttr(GetDoc()->GetPool());
                        pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                        aAttr.Put(pObj->GetMergedItemSet());

                        sal_Int32 nActLineWidth = ((const XLineWidthItem&)aAttr.Get(XATTR_LINEWIDTH)).GetValue();

                        if(nActLineWidth != nNewLineWidth)
                        {
                            sal_Bool bSetItemSet(sal_False);

                            // do this for SFX_ITEM_DEFAULT and for SFX_ITEM_SET
                            if(SFX_ITEM_DONTCARE != aAttr.GetItemState(XATTR_LINESTARTWIDTH))
                            {
                                sal_Int32 nValAct = ((const XLineStartWidthItem&)aAttr.Get(XATTR_LINESTARTWIDTH)).GetValue();
                                sal_Int32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
                                if(nValNew < 0)
                                    nValNew = 0;
                                bSetItemSet = sal_True;
                                aAttr.Put(XLineStartWidthItem(nValNew));
                            }

                            // do this for SFX_ITEM_DEFAULT and for SFX_ITEM_SET
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
                                pObj->SetMergedItemSet(aAttr);
                        }
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

                            const XFillStyleItem& rFillStyle =
                            (const XFillStyleItem&) aAttr.Get(XATTR_FILLSTYLE);

                            if (rFillStyle.GetValue() == XFILL_NONE)
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
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_AREA, SFX_CALLMODE_ASYNCHRON );
                        break;
                    case SID_ATTR_LINE_STYLE:
                    case SID_ATTR_LINE_DASH:
                    case SID_ATTR_LINE_WIDTH:
                    case SID_ATTR_LINE_COLOR:
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
                OSL_FAIL(" Kein Wert fuer Silbentrennung!");
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
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
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
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
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
#ifndef DISABLE_SCRIPTING
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
#endif
                    rReq.Ignore ();
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
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
                        OSL_FAIL("sd::DrawViewShell::FuTemporary(), SVX_ZOOM_PAGEWIDTH_NOBORDER not handled!" );
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
                        SetCurrentFunction( FuVectorize::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
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

                // create SdrGrafObj from metafile/bitmap
                Graphic aGraphic;
                switch (nSId)
                {
                    case SID_CONVERT_TO_METAFILE:
                    {
                        // switch on undo for the next operations
                        mpDrawView->BegUndo( String( SdResId( STR_UNDO_CONVERT_TO_METAFILE )));
                        GDIMetaFile aMetaFile(mpDrawView->GetAllMarkedMetaFile ());
                        aGraphic = Graphic(aMetaFile);
                    }
                    break;
                    case SID_CONVERT_TO_BITMAP:
                    {
                        // switch on undo for the next operations
                        mpDrawView->BegUndo( String( SdResId( STR_UNDO_CONVERT_TO_BITMAP )));
                        Bitmap aBitmap (mpDrawView->GetAllMarkedBitmap ());
                        aGraphic = Graphic(aBitmap);
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
                std::vector<std::pair<SfxItemSet*,SdrObjUserCall*> > aAttrList;
                SdPage* pPresPage = (SdPage*) mpDrawView->GetSdrPageView()->GetPage();
                sal_uLong i;

                for ( i = 0; i < nCount; i++ )
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        SfxItemSet* pNewSet = new SfxItemSet( GetDoc()->GetPool(), SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT, 0 );
                        pNewSet->Put(pObj->GetMergedItemSet());
                        aAttrList.push_back(std::make_pair(pNewSet, pObj->GetUserCall()));
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
                        std::pair<SfxItemSet*,SdrObjUserCall*> &rAttr = aAttrList[j++];

                        SfxItemSet* pNewSet = rAttr.first;
                        SdrObjUserCall* pUserCall = rAttr.second;

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

        case SID_SAVE_GRAPHIC:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if( pObj && pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
                {
                    GraphicObject aGraphicObject( ( (SdrGrafObj*) pObj )->GetGraphicObject() );
                    {
                        GraphicHelper::ExportGraphic( aGraphicObject.GetGraphic(), String("") );
                    }
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_EXTERNAL_EDIT:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if( pObj && pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
                {
                    GraphicObject aGraphicObject( ( (SdrGrafObj*) pObj )->GetGraphicObject() );
                    SdExternalToolEdit* aExternalToolEdit = new SdExternalToolEdit( mpDrawView, pObj );
                    aExternalToolEdit->Edit( &aGraphicObject );
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_COMPRESS_GRAPHIC:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if( pObj && pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
                {
                    GraphicObject aGraphicObject( ( (SdrGrafObj*) pObj )->GetGraphicObject() );
                    {
                        CompressGraphicsDialog dialog( GetParentWindow(), aGraphicObject.GetGraphic(), pObj->GetLogicRect().GetSize(), GetViewFrame()->GetBindings() );
                        if ( dialog.Execute() == RET_OK )
                        {
                            SdrGrafObj* pNewObject = (SdrGrafObj*) pObj->Clone();
                            const Graphic aNewGraphic = dialog.GetCompressedGraphic();
                            SdrPageView* pPageView = mpDrawView->GetSdrPageView();
                            pNewObject->SetEmptyPresObj( sal_False );
                            pNewObject->SetGraphic( aNewGraphic );
                            String aUndoString( mpDrawView->GetDescriptionOfMarkedObjects() );
                            aUndoString += (sal_Unicode) ( ' ' );
                            aUndoString += String( "Compress" );
                            mpDrawView->BegUndo( aUndoString );
                            mpDrawView->ReplaceObjectAtView( pObj, *pPageView, pNewObject );
                            mpDrawView->EndUndo();
                        }
                    }
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

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
