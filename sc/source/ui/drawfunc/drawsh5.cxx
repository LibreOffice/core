/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawsh5.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:08:02 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <svx/eeitem.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <tools/urlobj.hxx>
//CHINA001 #include <svx/dlgname.hxx>
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#include <svx/fmglob.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/fontwork.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/xdef.hxx>
#include <svx/xftsfit.hxx>
#include <vcl/msgbox.hxx>
#ifndef _SVX_EXTRUSION_BAR_HXX
#include <svx/extrusionbar.hxx>
#endif
#ifndef _SVX_FONTWORK_BAR_HXX
#include <svx/fontworkbar.hxx>
#endif
#include <sfx2/docfile.hxx>

#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/awt/XControlModel.hpp>

#include "drawsh.hxx"
#include "drawview.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
//CHINA001 #include "strindlg.hxx"
#include "scresid.hxx"
#include "undotab.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"

#include "sc.hrc"

using namespace com::sun::star;

//------------------------------------------------------------------

void ScDrawShell::GetHLinkState( SfxItemSet& rSet )             //  Hyperlink
{
    ScDrawView* pView = pViewData->GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    ULONG nMarkCount = rMarkList.GetMarkCount();

        //  Hyperlink

    SvxHyperlinkItem aHLinkItem;

    if ( nMarkCount == 1 )              // URL-Button markiert ?
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
        SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pObj);
        if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
        {
            uno::Reference<awt::XControlModel> xControlModel = pUnoCtrl->GetUnoControlModel();
            DBG_ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
            if( !xControlModel.is() )
                return;

            uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
            uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();

            rtl::OUString sPropButtonType  = rtl::OUString::createFromAscii( "ButtonType" );
            rtl::OUString sPropTargetURL   = rtl::OUString::createFromAscii( "TargetURL" );
            rtl::OUString sPropTargetFrame = rtl::OUString::createFromAscii( "TargetFrame" );
            rtl::OUString sPropLabel       = rtl::OUString::createFromAscii( "Label" );

            if(xInfo->hasPropertyByName( sPropButtonType ))
            {
                uno::Any aAny = xPropSet->getPropertyValue( sPropButtonType );
                form::FormButtonType eTmp;
                if ( (aAny >>= eTmp) && eTmp == form::FormButtonType_URL )
                {
                    rtl::OUString sTmp;
                    // Label
                    if(xInfo->hasPropertyByName( sPropLabel ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropLabel );
                        if ( (aAny >>= sTmp) && sTmp.getLength() )
                        {
                            aHLinkItem.SetName(sTmp);
                        }
                    }
                    // URL
                    if(xInfo->hasPropertyByName( sPropTargetURL ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropTargetURL );
                        if ( (aAny >>= sTmp) && sTmp.getLength() )
                        {
                            aHLinkItem.SetURL(sTmp);
                        }
                    }
                    // Target
                    if(xInfo->hasPropertyByName( sPropTargetFrame ))
                    {
                        aAny = xPropSet->getPropertyValue( sPropTargetFrame );
                        if ( (aAny >>= sTmp) && sTmp.getLength() )
                        {
                            aHLinkItem.SetTargetFrame(sTmp);
                        }
                    }
                    aHLinkItem.SetInsertMode(HLINK_BUTTON);
                }
            }
        }
    }

    rSet.Put(aHLinkItem);
}

void ScDrawShell::ExecuteHLink( SfxRequest& rReq )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    USHORT nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_HYPERLINK_SETLINK:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( SID_HYPERLINK_SETLINK, TRUE, &pItem ) == SFX_ITEM_SET )
                {
                    const SvxHyperlinkItem* pHyper = (const SvxHyperlinkItem*) pItem;
                    const String& rName     = pHyper->GetName();
                    const String& rURL      = pHyper->GetURL();
                    const String& rTarget   = pHyper->GetTargetFrame();
                    SvxLinkInsertMode eMode = pHyper->GetInsertMode();

                    BOOL bDone = FALSE;
                    if ( eMode == HLINK_DEFAULT || eMode == HLINK_BUTTON )
                    {
                        ScDrawView* pView = pViewData->GetScDrawView();
                        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                        if ( rMarkList.GetMarkCount() == 1 )
                        {
                            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetObj());
                            if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
                            {
                                uno::Reference<awt::XControlModel> xControlModel =
                                                        pUnoCtrl->GetUnoControlModel();
                                DBG_ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
                                if( !xControlModel.is() )
                                    return;

                                uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
                                uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();

                                rtl::OUString sPropTargetURL =
                                    rtl::OUString::createFromAscii( "TargetURL" );

                                // Darf man eine URL an dem Objekt setzen?
                                if (xInfo->hasPropertyByName( sPropTargetURL ))
                                {
                                    // Ja!

                                    rtl::OUString sPropButtonType =
                                        rtl::OUString::createFromAscii( "ButtonType" );
                                    rtl::OUString sPropTargetFrame =
                                        rtl::OUString::createFromAscii( "TargetFrame" );
                                    rtl::OUString sPropLabel =
                                        rtl::OUString::createFromAscii( "Label" );

                                    uno::Any aAny;
                                    aAny <<= rtl::OUString(rName);
                                    xPropSet->setPropertyValue( sPropLabel, aAny );

                                    ::rtl::OUString aTmp = INetURLObject::GetAbsURL( pViewData->GetDocShell()->GetMedium()->GetBaseURL(), rURL );
                                    aAny <<= aTmp;
                                    xPropSet->setPropertyValue( sPropTargetURL, aAny );

                                    if( rTarget.Len() )
                                    {
                                        aAny <<= rtl::OUString(rTarget);
                                        xPropSet->setPropertyValue( sPropTargetFrame, aAny );
                                    }

                                    form::FormButtonType eButtonType = form::FormButtonType_URL;
                                    aAny <<= eButtonType;
                                    xPropSet->setPropertyValue( sPropButtonType, aAny );

                                    //! Undo ???
                                    pViewData->GetDocShell()->SetDocumentModified();
                                    bDone = TRUE;
                                }
                            }
                        }
                    }

                    if (!bDone)
                        pViewData->GetViewShell()->
                            InsertURL( rName, rURL, rTarget, (USHORT) eMode );

                    //  InsertURL an der ViewShell schaltet bei "Text" die DrawShell ab !!!
                }
            }
            break;
        default:
            DBG_ERROR("falscher Slot");
    }
}

USHORT ScGetFontWorkId();       // wegen CLOOKs - in drtxtob2

//------------------------------------------------------------------

//
//          Funktionen auf Drawing-Objekten
//

void ScDrawShell::ExecDrawFunc( SfxRequest& rReq )
{
    SfxBindings& rBindings = pViewData->GetBindings();
    ScTabView*   pTabView  = pViewData->GetView();
    ScDrawView*  pView     = pTabView->GetScDrawView();
    Window*      pWin      = pTabView->GetActiveWin();
    const SfxItemSet *pArgs = rReq.GetArgs();
    USHORT nSlotId = rReq.GetSlot();

    //!!!
    // wer weiss, wie lange das funktioniert? (->vom Abreisscontrol funktioniert es)
    //
    if (nSlotId == SID_OBJECT_ALIGN && pArgs)
        nSlotId = SID_OBJECT_ALIGN + ((SfxEnumItem&)pArgs->Get(SID_OBJECT_ALIGN)).GetValue() + 1;

    switch (nSlotId)
    {
        case SID_OBJECT_HEAVEN:
            pView->SetMarkedToLayer( SC_LAYER_FRONT );
            rBindings.Invalidate(SID_OBJECT_HEAVEN);
            rBindings.Invalidate(SID_OBJECT_HELL);
            break;
        case SID_OBJECT_HELL:
            pView->SetMarkedToLayer( SC_LAYER_BACK );
            rBindings.Invalidate(SID_OBJECT_HEAVEN);
            rBindings.Invalidate(SID_OBJECT_HELL);
            //  leave draw shell if nothing selected (layer may be locked)
            if ( pView->GetMarkedObjectList().GetMarkCount() == 0 )
                pViewData->GetViewShell()->SetDrawShell( FALSE );
            break;

        case SID_FRAME_TO_TOP:
            pView->PutMarkedToTop();
            break;
        case SID_FRAME_TO_BOTTOM:
            pView->PutMarkedToBtm();
            break;
        case SID_FRAME_UP:
            pView->MovMarkedToTop();
            break;
        case SID_FRAME_DOWN:
            pView->MovMarkedToBtm();
            break;

        case SID_GROUP:
            pView->GroupMarked();
            break;
        case SID_UNGROUP:
            pView->UnGroupMarked();
            break;
        case SID_ENTER_GROUP:
            pView->EnterMarkedGroup();
            break;
        case SID_LEAVE_GROUP:
            pView->LeaveOneGroup();
            break;

        case SID_MIRROR_HORIZONTAL:
            pView->MirrorAllMarkedHorizontal();
            break;
        case SID_MIRROR_VERTICAL:
            pView->MirrorAllMarkedVertical();
            break;

        case SID_OBJECT_ALIGN_LEFT:
        case SID_ALIGN_ANY_LEFT:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_LEFT, SDRVALIGN_NONE);
            break;
        case SID_OBJECT_ALIGN_CENTER:
        case SID_ALIGN_ANY_HCENTER:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_CENTER, SDRVALIGN_NONE);
            break;
        case SID_OBJECT_ALIGN_RIGHT:
        case SID_ALIGN_ANY_RIGHT:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_RIGHT, SDRVALIGN_NONE);
            break;
        case SID_OBJECT_ALIGN_UP:
        case SID_ALIGN_ANY_TOP:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_TOP);
            break;
        case SID_OBJECT_ALIGN_MIDDLE:
        case SID_ALIGN_ANY_VCENTER:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_CENTER);
            break;
        case SID_OBJECT_ALIGN_DOWN:
        case SID_ALIGN_ANY_BOTTOM:
            if (pView->IsAlignPossible())
                pView->AlignMarkedObjects(SDRHALIGN_NONE, SDRVALIGN_BOTTOM);
            break;

        case SID_DELETE:
        case SID_DELETE_CONTENTS:
        {
            const SdrMarkList& rNoteMarkList = pView->GetMarkedObjectList();
            if(rNoteMarkList.GetMarkCount() == 1)
            {
                SdrObject* pObj = rNoteMarkList.GetMark( 0 )->GetObj();
                if ( pObj && pObj->GetLayer() == SC_LAYER_INTERN && pObj->ISA(SdrCaptionObj) )
                {
                    ScAddress aTabPos;
                    ScDrawObjData* pData = ScDrawLayer::GetObjDataTab( pObj, pViewData->GetTabNo() );
                    if( pData )
                        aTabPos = pData->aStt;
                    ScDocument* pDoc = pViewData->GetDocument();
                    ScPostIt aNote(pDoc);
                    pViewData->GetViewShell()->SetNote( aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), aNote );   // with Undo

                    ScDrawLayer* pModel = pDoc->GetDrawLayer();
                    if (pModel)
                    {
                        SdrPage* pPage = pModel->GetPage( aTabPos.Tab() );
                        if(pPage)
                        {
                            ScDocShell* pDocSh = pViewData->GetDocShell();
                            pDocSh->GetUndoManager()->AddUndoAction( new SdrUndoRemoveObj( *pObj));
                            pPage->RemoveObject( pObj->GetOrdNum() );
                        }
                    }
                    if (!pTabView->IsDrawSelMode())
                        pViewData->GetViewShell()->SetDrawShell( FALSE );
                    break;
                }
            }
            pView->DeleteMarked();
            if (!pTabView->IsDrawSelMode())
                pViewData->GetViewShell()->SetDrawShell( FALSE );
        }
        break;

        case SID_CUT:
            pView->DoCut();
            if (!pTabView->IsDrawSelMode())
                pViewData->GetViewShell()->SetDrawShell( FALSE );
            break;

        case SID_COPY:
            pView->DoCopy();
            break;

        case SID_PASTE:
            DBG_ERROR( "SdrView::PasteClipboard not supported anymore" );
            // pView->PasteClipboard( pWin );
            break;

        case SID_SELECTALL:
            pView->MarkAll();
            break;

        case SID_ANCHOR_PAGE:
            pView->SetAnchor( SCA_PAGE );
            rBindings.Invalidate( SID_ANCHOR_PAGE );
            rBindings.Invalidate( SID_ANCHOR_CELL );
            break;

        case SID_ANCHOR_CELL:
            pView->SetAnchor( SCA_CELL );
            rBindings.Invalidate( SID_ANCHOR_PAGE );
            rBindings.Invalidate( SID_ANCHOR_CELL );
            break;

        case SID_ANCHOR_TOGGLE:
            {
                switch( pView->GetAnchor() )
                {
                    case SCA_CELL:
                    pView->SetAnchor( SCA_PAGE );
                    break;
                    default:
                    pView->SetAnchor( SCA_CELL );
                    break;
                }
            }
            rBindings.Invalidate( SID_ANCHOR_PAGE );
            rBindings.Invalidate( SID_ANCHOR_CELL );
            break;

        case SID_OBJECT_ROTATE:
            {
                SdrDragMode eMode;
                if (pView->GetDragMode() == SDRDRAG_ROTATE)
                    eMode = SDRDRAG_MOVE;
                else
                    eMode = SDRDRAG_ROTATE;
                pView->SetDragMode( eMode );
                rBindings.Invalidate( SID_OBJECT_ROTATE );
                rBindings.Invalidate( SID_OBJECT_MIRROR );
                if (eMode == SDRDRAG_ROTATE && !pView->IsFrameDragSingles())
                {
                    pView->SetFrameDragSingles( TRUE );
                    rBindings.Invalidate( SID_BEZIER_EDIT );
                }
            }
            break;
        case SID_OBJECT_MIRROR:
            {
                SdrDragMode eMode;
                if (pView->GetDragMode() == SDRDRAG_MIRROR)
                    eMode = SDRDRAG_MOVE;
                else
                    eMode = SDRDRAG_MIRROR;
                pView->SetDragMode( eMode );
                rBindings.Invalidate( SID_OBJECT_ROTATE );
                rBindings.Invalidate( SID_OBJECT_MIRROR );
                if (eMode == SDRDRAG_MIRROR && !pView->IsFrameDragSingles())
                {
                    pView->SetFrameDragSingles( TRUE );
                    rBindings.Invalidate( SID_BEZIER_EDIT );
                }
            }
            break;
        case SID_BEZIER_EDIT:
            {
                BOOL bOld = pView->IsFrameDragSingles();
                pView->SetFrameDragSingles( !bOld );
                rBindings.Invalidate( SID_BEZIER_EDIT );
                if (bOld && pView->GetDragMode() != SDRDRAG_MOVE)
                {
                    pView->SetDragMode( SDRDRAG_MOVE );
                    rBindings.Invalidate( SID_OBJECT_ROTATE );
                    rBindings.Invalidate( SID_OBJECT_MIRROR );
                }
            }
            break;

        case SID_FONTWORK:
        {
            USHORT nId = ScGetFontWorkId();
            SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();

            if ( rReq.GetArgs() )
                pViewFrm->SetChildWindow( nId,
                                           ((const SfxBoolItem&)
                                            (rReq.GetArgs()->Get(SID_FONTWORK))).
                                                GetValue() );
            else
                pViewFrm->ToggleChildWindow( nId );

            rBindings.Invalidate( SID_FONTWORK );
            rReq.Done();
        }
        break;

        case SID_ORIGINALSIZE:
            pView->SetMarkedOriginalSize();
            break;

        case SID_ENABLE_HYPHENATION:
            {
                SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, SID_ENABLE_HYPHENATION, FALSE);
                if( pItem )
                {
                    SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
                    BOOL bValue = ( (const SfxBoolItem*) pItem)->GetValue();
                    aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                    pView->SetAttributes( aSet );
                }
                rReq.Done();
            }
            break;

        case SID_RENAME_OBJECT:
            {
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if ( rMarkList.GetMarkCount() == 1 )
                {
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();
                    if ( pObj->GetLayer() != SC_LAYER_INTERN )
                    {
                        UINT16 nObjType = pObj->GetObjIdentifier();

                        // PersistName is used to identify object in Undo
                        String aPersistName;
                        if ( nObjType == OBJ_OLE2 )
                            aPersistName = static_cast<SdrOle2Obj*>(pObj)->GetPersistName();

                        String aOldName = pObj->GetName();
                        String aTitle(ScResId(SCSTR_RENAMEOBJECT));
                        String aDesc(ScResId(SCSTR_NAME));

                        //CHINA001 SvxNameDialog* pDlg = new SvxNameDialog( NULL, aOldName, aDesc );
                        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                        DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
                        AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( NULL, aOldName, aDesc, ResId(RID_SVXDLG_NAME) );
                        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                        pDlg->SetEditHelpId( HID_SC_DRAW_RENAME );
                        pDlg->SetText( aTitle );
                        pDlg->SetCheckNameHdl( LINK( this, ScDrawShell, NameObjectHdl ) );

                        USHORT nRet = pDlg->Execute();
                        if ( nRet == RET_OK )
                        {
                            String aNewName;
                            pDlg->GetName( aNewName );
                            if ( aNewName != aOldName )
                            {
                                if ( nObjType == OBJ_GRAF && aNewName.Len() == 0 )
                                {
                                    //  graphics objects must have names
                                    //  (all graphics are supposed to be in the navigator)
                                    ScDrawLayer* pModel = pViewData->GetDocument()->GetDrawLayer();
                                    if ( pModel )
                                        aNewName = pModel->GetNewGraphicName();
                                }

                                pObj->SetName( aNewName );              // set new name

                                ScDocShell* pDocSh = pViewData->GetDocShell();

                                //  An undo action for renaming is missing in svdraw (99363).
                                //  For OLE objects (which can be identified using the persist name),
                                //  ScUndoRenameObject can be used until there is a common action for all objects.
                                if ( aPersistName.Len() )
                                {
                                    pDocSh->GetUndoManager()->AddUndoAction(
                                                new ScUndoRenameObject( pDocSh, aPersistName, aOldName, aNewName ) );
                                }

                                // ChartListenerCollectionNeedsUpdate is needed for Navigator update
                                pDocSh->GetDocument()->SetChartListenerCollectionNeedsUpdate( TRUE );
                                pDocSh->SetDrawModified();
                            }
                        }
                        delete pDlg;
                    }
                }
            }
            break;

        case SID_EXTRUSION_TOOGLE:
        case SID_EXTRUSION_TILT_DOWN:
        case SID_EXTRUSION_TILT_UP:
        case SID_EXTRUSION_TILT_LEFT:
        case SID_EXTRUSION_TILT_RIGHT:
        case SID_EXTRUSION_3D_COLOR:
        case SID_EXTRUSION_DEPTH:
        case SID_EXTRUSION_DIRECTION:
        case SID_EXTRUSION_PROJECTION:
        case SID_EXTRUSION_LIGHTING_DIRECTION:
        case SID_EXTRUSION_LIGHTING_INTENSITY:
        case SID_EXTRUSION_SURFACE:
        case SID_EXTRUSION_DEPTH_FLOATER:
        case SID_EXTRUSION_DIRECTION_FLOATER:
        case SID_EXTRUSION_LIGHTING_FLOATER:
        case SID_EXTRUSION_SURFACE_FLOATER:
        case SID_EXTRUSION_DEPTH_DIALOG:
            svx::ExtrusionBar::execute( pView, rReq, rBindings );
            rReq.Ignore ();
            break;

        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_SHAPE_TYPE:
        case SID_FONTWORK_ALIGNMENT:
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        case SID_FONTWORK_CHARACTER_SPACING:
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        case SID_FONTWORK_CHARACTER_SPACING_FLOATER:
        case SID_FONTWORK_ALIGNMENT_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_DIALOG:
            svx::FontworkBar::execute( pView, rReq, rBindings );
            rReq.Ignore ();
            break;

        default:
            break;
    }
}

IMPL_LINK( ScDrawShell, NameObjectHdl, AbstractSvxNameDialog*, pDialog )
{
    String aName;

    if( pDialog )
        pDialog->GetName( aName );

    ScDrawLayer* pModel = pViewData->GetDocument()->GetDrawLayer();
    if ( aName.Len() && pModel )
    {
        SCTAB nDummyTab;
        if ( pModel->GetNamedObject( aName, 0, nDummyTab ) )
        {
            // existing object found -> name invalid
            return 0;
        }
    }

    return 1;   // name is valid
}

//------------------------------------------------------------------

void ScDrawShell::ExecFormText(SfxRequest& rReq)
{
    ScDrawView*         pDrView     = pViewData->GetScDrawView();
    const SdrMarkList&  rMarkList   = pDrView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();
        const SfxPoolItem* pItem;

        if ( pDrView->IsTextEdit() )
            pDrView->ScEndTextEdit();

        if (    SFX_ITEM_SET ==
                rSet.GetItemState(XATTR_FORMTXTSTDFORM, TRUE, &pItem)
             && XFTFORM_NONE !=
                ((const XFormTextStdFormItem*) pItem)->GetValue() )
        {

            USHORT nId              = SvxFontWorkChildWindow::GetChildWindowId();
            SfxViewFrame* pViewFrm  = pViewData->GetViewShell()->GetViewFrame();
            SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*)
                                       (pViewFrm->
                                            GetChildWindow(nId)->GetWindow());

            pDlg->CreateStdFormObj(*pDrView, *pDrView->GetPageViewPvNum(0),
                                    rSet, *rMarkList.GetMark(0)->GetObj(),
                                   ((const XFormTextStdFormItem*) pItem)->
                                   GetValue());
        }
        else
            pDrView->SetAttributes(rSet);
    }
}

//------------------------------------------------------------------

void ScDrawShell::ExecFormatPaintbrush( SfxRequest& rReq )
{
    ScViewFunc* pView = pViewData->GetView();
    if ( pView->HasPaintBrush() )
    {
        // cancel paintbrush mode
        pView->ResetBrushDocument();
    }
    else
    {
        BOOL bLock = FALSE;
        const SfxItemSet *pArgs = rReq.GetArgs();
        if( pArgs && pArgs->Count() >= 1 )
            bLock = static_cast<const SfxBoolItem&>(pArgs->Get(SID_FORMATPAINTBRUSH)).GetValue();

        ScDrawView* pDrawView = pViewData->GetScDrawView();
        if ( pDrawView && pDrawView->AreObjectsMarked() )
        {
            BOOL bOnlyHardAttr = TRUE;
            SfxItemSet* pItemSet = new SfxItemSet( pDrawView->GetAttrFromMarked(bOnlyHardAttr) );
            pView->SetDrawBrushSet( pItemSet, bLock );
        }
    }
}

void ScDrawShell::StateFormatPaintbrush( SfxItemSet& rSet )
{
    ScDrawView* pDrawView = pViewData->GetScDrawView();
    BOOL bSelection = pDrawView && pDrawView->AreObjectsMarked();
    BOOL bHasPaintBrush = pViewData->GetView()->HasPaintBrush();

    if ( !bHasPaintBrush && !bSelection )
        rSet.DisableItem( SID_FORMATPAINTBRUSH );
    else
        rSet.Put( SfxBoolItem( SID_FORMATPAINTBRUSH, bHasPaintBrush ) );
}






