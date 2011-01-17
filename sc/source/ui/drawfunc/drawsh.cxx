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
#include "precompiled_sc.hxx"

#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001

#include "scitems.hxx"

#include <editeng/eeitem.hxx>
#include <svx/fontwork.hxx>
//#include <svx/labdlg.hxx> CHINA001
#include <svl/srchitem.hxx>
#include <svx/tabarea.hxx>
#include <svx/tabline.hxx>
//CHINA001 #include <svx/transfrm.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>

#include "drawsh.hxx"
#include "drwlayer.hxx"
#include "sc.hrc"
#include "viewdata.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "drawview.hxx"
#include "scresid.hxx"
#include <svx/svdobj.hxx>
//add header of cui CHINA001
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>

#define ScDrawShell
#include "scslots.hxx"

#include "userdat.hxx"
#include <sfx2/objsh.hxx>
#include <svl/macitem.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/viewsh.hxx>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XFrame.hpp>

//------------------------------------------------------------------

TYPEINIT1( ScDrawShell, SfxShell );

SFX_IMPL_INTERFACE(ScDrawShell, SfxShell, ScResId(SCSTR_DRAWSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                ScResId(RID_DRAW_OBJECTBAR) );
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_DRAW) );
    SFX_CHILDWINDOW_REGISTRATION( SvxFontWorkChildWindow::GetChildWindowId() );
}


// abschalten der nicht erwuenschten Acceleratoren:

void ScDrawShell::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

void lcl_setModified( SfxObjectShell*  pShell )
{
    if ( pShell )
    {
        com::sun::star::uno::Reference< com::sun::star::util::XModifiable > xModif( pShell->GetModel(), com::sun::star::uno::UNO_QUERY );
        if ( xModif.is() )
            xModif->setModified( sal_True );
    }
}

void ScDrawShell::ExecDrawAttr( SfxRequest& rReq )
{
    sal_uInt16              nSlot       = rReq.GetSlot();
    Window*             pWin        = pViewData->GetActiveWin();
//  SfxViewFrame*       pViewFrame  = SfxViewShell::Current()->GetViewFrame(); //!!! koennte knallen
    ScDrawView*         pView       = pViewData->GetScDrawView();
    SdrModel*           pDoc        = pViewData->GetDocument()->GetDrawLayer();

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    sal_uLong nMarkCount = rMarkList.GetMarkCount();
    SdrObject* pSingleSelectedObj = NULL;
    if ( nMarkCount > 0 )
        pSingleSelectedObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

    switch ( nSlot )
    {
        case SID_ASSIGNMACRO:
            {
                if ( pSingleSelectedObj )
                    ExecuteMacroAssign( pSingleSelectedObj, pWin );
            }
            break;

        case SID_TEXT_STANDARD: // Harte Textattributierung loeschen
            {
                SfxItemSet aEmptyAttr(GetPool(), EE_ITEMS_START, EE_ITEMS_END);
                pView->SetAttributes(aEmptyAttr, sal_True);
            }
            break;

        case SID_ATTR_LINE_STYLE:
        case SID_ATTR_LINEEND_STYLE:
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_WIDTH:
        case SID_ATTR_LINE_COLOR:
        case SID_ATTR_FILL_STYLE:
        case SID_ATTR_FILL_COLOR:
        case SID_ATTR_FILL_GRADIENT:
        case SID_ATTR_FILL_HATCH:
        case SID_ATTR_FILL_BITMAP:

        // #i25616#
        case SID_ATTR_FILL_SHADOW:
            {
                // Wenn ToolBar vertikal :
                if ( !rReq.GetArgs() )
                {
                    switch ( nSlot )
                    {
                        case SID_ATTR_LINE_STYLE:
                        case SID_ATTR_LINE_DASH:
                        case SID_ATTR_LINE_WIDTH:
                        case SID_ATTR_LINE_COLOR:
                            ExecuteLineDlg( rReq );
                            break;

                        case SID_ATTR_FILL_STYLE:
                        case SID_ATTR_FILL_COLOR:
                        case SID_ATTR_FILL_GRADIENT:
                        case SID_ATTR_FILL_HATCH:
                        case SID_ATTR_FILL_BITMAP:

                        // #i25616#
                        case SID_ATTR_FILL_SHADOW:

                            ExecuteAreaDlg( rReq );
                            break;

                        default:
                            break;
                    }

                    //=====
                    return;
                    //=====
                }

                if( pView->AreObjectsMarked() )
                    pView->SetAttrToMarked( *rReq.GetArgs(), sal_False );
                else
                    pView->SetDefaultAttr( *rReq.GetArgs(), sal_False);
                pView->InvalidateAttribs();
            }
            break;

        case SID_ATTRIBUTES_LINE:
            ExecuteLineDlg( rReq );
            break;

        case SID_ATTRIBUTES_AREA:
            ExecuteAreaDlg( rReq );
            break;

        case SID_DRAWTEXT_ATTR_DLG:
            ExecuteTextAttrDlg( rReq );
            break;

#ifdef ISSUE66550_HLINK_FOR_SHAPES
        case SID_DRAW_HLINK_EDIT:
            if ( pSingleSelectedObj )
                pViewData->GetDispatcher().Execute( SID_HYPERLINK_DIALOG );
            break;

        case SID_DRAW_HLINK_DELETE:
            if ( pSingleSelectedObj )
                SetHlinkForObject( pSingleSelectedObj, rtl::OUString() );
            break;

        case SID_OPEN_HYPERLINK:
            if ( nMarkCount == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if ( pObj->IsGroupObject() )
                {
                    SdrPageView* pPV = 0;
                    SdrObject* pHit = 0;
                    if ( pView->PickObj( pWin->PixelToLogic( pViewData->GetMousePosPixel() ), pView->getHitTolLog(), pHit, pPV, SDRSEARCH_DEEP ) )
                        pObj = pHit;
                }

                ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj );
                if ( pInfo && (pInfo->GetHlink().getLength() > 0) )
                    ScGlobal::OpenURL( pInfo->GetHlink(), String::EmptyString() );
            }
            break;
#endif

        case SID_ATTR_TRANSFORM:
            {
                if ( pView->AreObjectsMarked() )
                {
                    const SfxItemSet* pArgs = rReq.GetArgs();

                    if( !pArgs )
                    {
                        // const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                        if( rMarkList.GetMark(0) != 0 )
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                            if( pObj->GetObjIdentifier() == OBJ_CAPTION )
                            {
                                // --------- Itemset fuer Caption --------
                                SfxItemSet aNewAttr(pDoc->GetItemPool());
                                pView->GetAttributes(aNewAttr);
                                // --------- Itemset fuer Groesse und Position --------
                                SfxItemSet aNewGeoAttr(pView->GetGeoAttrFromMarked());

                                //SvxCaptionTabDialog* pDlg = new SvxCaptionTabDialog(pWin, pView);
                                //change for cui CHINA001
                                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                                if ( pFact )
                                {
                                    SfxAbstractTabDialog *pDlg = pFact->CreateCaptionDialog( pWin, pView );

                                    const sal_uInt16* pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
                                    SfxItemSet aCombSet( *aNewAttr.GetPool(), pRange );
                                    aCombSet.Put( aNewAttr );
                                    aCombSet.Put( aNewGeoAttr );
                                    pDlg->SetInputSet( &aCombSet );

                                    if (pDlg->Execute() == RET_OK)
                                    {
                                        rReq.Done(*(pDlg->GetOutputItemSet()));
                                        pView->SetAttributes(*pDlg->GetOutputItemSet());
                                        pView->SetGeoAttrToMarked(*pDlg->GetOutputItemSet());
                                    }

                                    delete pDlg;
                                }// change for cui
                            }
                            else
                            {
                                SfxItemSet aNewAttr(pView->GetGeoAttrFromMarked());
                                //CHINA001 SvxTransformTabDialog* pDlg = new SvxTransformTabDialog(pWin, &aNewAttr, pView);
                                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                                if(pFact)
                                {
                                    SfxAbstractTabDialog* pDlg = pFact->CreateSvxTransformTabDialog( pWin, &aNewAttr,pView );
                                    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                                    if (pDlg->Execute() == RET_OK)
                                    {
                                        rReq.Done(*(pDlg->GetOutputItemSet()));
                                        pView->SetGeoAttrToMarked(*pDlg->GetOutputItemSet());
                                    }
                                    delete pDlg;
                                }
                            }
                        }


                    }
                    else
                        pView->SetGeoAttrToMarked( *pArgs );
                }
            }
            break;

        default:
            break;
    }
}

void ScDrawShell::ExecuteMacroAssign( SdrObject* pObj, Window* pWin )
{
    SvxMacroItem aItem ( SFX_APP()->GetPool().GetWhich( SID_ATTR_MACROITEM ) );
    ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj, sal_True );
    if ( pInfo->GetMacro().getLength() > 0 )
    {
        SvxMacroTableDtor aTab;
        String sMacro(  pInfo->GetMacro() );
        aTab.Insert( SFX_EVENT_MOUSECLICK_OBJECT, new SvxMacro( sMacro, String() ) );
        aItem.SetMacroTable( aTab );
    }

    // create empty itemset for macro-dlg
    SfxItemSet* pItemSet = new SfxItemSet(SFX_APP()->GetPool(), SID_ATTR_MACROITEM, SID_ATTR_MACROITEM, SID_EVENTCONFIG, SID_EVENTCONFIG, 0 );
    pItemSet->Put ( aItem, SID_ATTR_MACROITEM );

    SfxEventNamesItem aNamesItem(SID_EVENTCONFIG);
    aNamesItem.AddEvent( ScResId(RID_SCSTR_ONCLICK), String(), SFX_EVENT_MOUSECLICK_OBJECT );
    pItemSet->Put( aNamesItem, SID_EVENTCONFIG );

    com::sun::star::uno::Reference < com::sun::star::frame::XFrame > xFrame;
    if (GetViewShell())
        xFrame = GetViewShell()->GetViewFrame()->GetFrame().GetFrameInterface();

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractDialog* pMacroDlg = pFact->CreateSfxDialog( pWin, *pItemSet, xFrame, SID_EVENTCONFIG );
    if ( pMacroDlg && pMacroDlg->Execute() == RET_OK )
    {
        const SfxItemSet* pOutSet = pMacroDlg->GetOutputItemSet();
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pOutSet->GetItemState( SID_ATTR_MACROITEM, sal_False, &pItem ))
        {
            rtl::OUString sMacro;
            SvxMacro* pMacro = ((SvxMacroItem*)pItem)->GetMacroTable().Get( SFX_EVENT_MOUSECLICK_OBJECT );
            if ( pMacro )
                sMacro = pMacro->GetMacName();

            if ( pObj->IsGroupObject() )
            {
                SdrObjList* pOL = pObj->GetSubList();
                sal_uLong nObj = pOL->GetObjCount();
                for ( sal_uLong index=0; index<nObj; ++index )
                {
                    pInfo = ScDrawLayer::GetMacroInfo( pOL->GetObj(index), sal_True );
                    pInfo->SetMacro( sMacro );
                }
            }
            else
                pInfo->SetMacro( sMacro );
            lcl_setModified( GetObjectShell() );
        }
    }

    delete pMacroDlg;
    delete pItemSet;
}

void ScDrawShell::ExecuteLineDlg( SfxRequest& rReq, sal_uInt16 nTabPage )
{
    ScDrawView*         pView       = pViewData->GetScDrawView();
    sal_Bool                bHasMarked  = pView->AreObjectsMarked();
    const SdrObject*    pObj        = NULL;
    const SdrMarkList&  rMarkList   = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    SfxItemSet  aNewAttr( pView->GetDefaultAttr() );
    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, sal_False );

//CHINA001  SvxLineTabDialog* pDlg
//CHINA001  = new SvxLineTabDialog( pViewData->GetDialogParent(),
//CHINA001  &aNewAttr,
//CHINA001  pViewData->GetDocument()->GetDrawLayer(),
//CHINA001  pObj,
//CHINA001  bHasMarked );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet Factory fail!");//CHINA001
        SfxAbstractTabDialog * pDlg = pFact->CreateSvxLineTabDialog( pViewData->GetDialogParent(),
                    &aNewAttr,
                pViewData->GetDocument()->GetDrawLayer(),
                pObj,
                bHasMarked);
        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
    if ( nTabPage != 0xffff )
        pDlg->SetCurPageId( nTabPage );

    if ( pDlg->Execute() == RET_OK )
    {
        if( bHasMarked )
            pView->SetAttrToMarked( *pDlg->GetOutputItemSet(), sal_False );
        else
            pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), sal_False );

        pView->InvalidateAttribs();
        rReq.Done();
    }

    delete pDlg;
}

void ScDrawShell::ExecuteAreaDlg( SfxRequest& rReq, sal_uInt16 nTabPage )
{
    ScDrawView* pView       = pViewData->GetScDrawView();
    sal_Bool        bHasMarked  = pView->AreObjectsMarked();

    SfxItemSet  aNewAttr( pView->GetDefaultAttr() );
    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, sal_False );

    //CHINA001 SvxAreaTabDialog* pDlg
    //CHINA001  = new SvxAreaTabDialog( pViewData->GetDialogParent(),
//CHINA001                              &aNewAttr,
//CHINA001                              pViewData->GetDocument()->GetDrawLayer(),
//CHINA001                              pView );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet Factory fail!");//CHINA001
    AbstractSvxAreaTabDialog * pDlg = pFact->CreateSvxAreaTabDialog( pViewData->GetDialogParent(),
                                                                    &aNewAttr,
                                                            pViewData->GetDocument()->GetDrawLayer(),
                                                            pView);
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001

    // #i74099# by default, the dialog deletes the current color table if a different one is loaded
    // (see SwDrawShell::ExecDrawDlg)
    const SvxColorTableItem* pColorItem =
        static_cast<const SvxColorTableItem*>( pViewData->GetSfxDocShell()->GetItem(SID_COLOR_TABLE) );
    if (pColorItem->GetColorTable() == XColorTable::GetStdColorTable())
        pDlg->DontDeleteColorTable();

    if ( nTabPage != 0xffff )
        pDlg->SetCurPageId( nTabPage );

    if ( pDlg->Execute() == RET_OK )
    {
        if( bHasMarked )
            pView->SetAttrToMarked( *pDlg->GetOutputItemSet(), sal_False );
        else
            pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), sal_False );

        pView->InvalidateAttribs();
        rReq.Done();
    }

    delete pDlg;
}

void ScDrawShell::ExecuteTextAttrDlg( SfxRequest& rReq, sal_uInt16 /* nTabPage */ )
{
    ScDrawView* pView       = pViewData->GetScDrawView();
    sal_Bool        bHasMarked  = pView->AreObjectsMarked();
    SfxItemSet  aNewAttr    ( pView->GetDefaultAttr() );

    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, sal_False );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractTabDialog *pDlg = pFact->CreateTextTabDialog( pViewData->GetDialogParent(), &aNewAttr, pView );

    sal_uInt16 nResult = pDlg->Execute();

    if ( RET_OK == nResult )
    {
        if ( bHasMarked )
            pView->SetAttributes( *pDlg->GetOutputItemSet() );
        else
            pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), sal_False );

        pView->InvalidateAttribs();
        rReq.Done();
    }
    delete( pDlg );
}

#ifdef ISSUE66550_HLINK_FOR_SHAPES
void ScDrawShell::SetHlinkForObject( SdrObject* pObj, const rtl::OUString& rHlnk )
{
    if ( pObj )
    {
        ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj, sal_True );
        pInfo->SetHlink( rHlnk );
        lcl_setModified( GetObjectShell() );
    }
}
#endif

