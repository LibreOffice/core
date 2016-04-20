/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include "scitems.hxx"

#include <editeng/eeitem.hxx>
#include <svx/fontwork.hxx>
#include <svl/srchitem.hxx>
#include <svx/svdpage.hxx>
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
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>
#include "tabvwsh.hxx"
#include <gridwin.hxx>
#include <sfx2/bindings.hxx>

#define ScDrawShell
#include "scslots.hxx"

#include "userdat.hxx"
#include <svl/macitem.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/viewsh.hxx>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <memory>


SFX_IMPL_INTERFACE(ScDrawShell, SfxShell)

void ScDrawShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                            RID_DRAW_OBJECTBAR);

    GetStaticInterface()->RegisterPopupMenu(ScResId(RID_POPUP_DRAW));

    GetStaticInterface()->RegisterChildWindow(SvxFontWorkChildWindow::GetChildWindowId());
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

static void lcl_setModified( SfxObjectShell*  pShell )
{
    if ( pShell )
    {
        css::uno::Reference< css::util::XModifiable > xModif( pShell->GetModel(), css::uno::UNO_QUERY );
        if ( xModif.is() )
            xModif->setModified( true );
    }
}

void ScDrawShell::ExecDrawAttr( SfxRequest& rReq )
{
    sal_uInt16              nSlot       = rReq.GetSlot();
    vcl::Window*             pWin        = pViewData->GetActiveWin();
    ScDrawView*         pView       = pViewData->GetScDrawView();
    SdrModel*           pDoc        = pViewData->GetDocument()->GetDrawLayer();

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    const size_t nMarkCount = rMarkList.GetMarkCount();
    SdrObject* pSingleSelectedObj = nullptr;
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
                pView->SetAttributes(aEmptyAttr, true);
            }
            break;

        case SID_ATTR_LINE_STYLE:
        case SID_ATTR_LINEEND_STYLE:
        case SID_ATTR_LINE_START:
        case SID_ATTR_LINE_END:
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_WIDTH:
        case SID_ATTR_LINE_COLOR:
        case SID_ATTR_LINE_TRANSPARENCE:
        case SID_ATTR_LINE_JOINT:
        case SID_ATTR_LINE_CAP:
        case SID_ATTR_FILL_STYLE:
        case SID_ATTR_FILL_COLOR:
        case SID_ATTR_FILL_GRADIENT:
        case SID_ATTR_FILL_HATCH:
        case SID_ATTR_FILL_BITMAP:
        case SID_ATTR_FILL_TRANSPARENCE:
        case SID_ATTR_FILL_FLOATTRANSPARENCE:

        // #i25616#
        case SID_ATTR_FILL_SHADOW:
        case SID_ATTR_SHADOW_TRANSPARENCE:
        case SID_ATTR_SHADOW_COLOR:
        case SID_ATTR_SHADOW_XDISTANCE:
        case SID_ATTR_SHADOW_YDISTANCE:
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
                        case SID_ATTR_LINE_TRANSPARENCE:
                        case SID_ATTR_LINE_JOINT:
                        case SID_ATTR_LINE_CAP:
                            ExecuteLineDlg( rReq );
                            break;

                        case SID_ATTR_FILL_STYLE:
                        case SID_ATTR_FILL_COLOR:
                        case SID_ATTR_FILL_GRADIENT:
                        case SID_ATTR_FILL_HATCH:
                        case SID_ATTR_FILL_BITMAP:
                        case SID_ATTR_FILL_TRANSPARENCE:
                        case SID_ATTR_FILL_FLOATTRANSPARENCE:

                        // #i25616#
                        case SID_ATTR_FILL_SHADOW:
                        case SID_ATTR_SHADOW_TRANSPARENCE:
                        case SID_ATTR_SHADOW_COLOR:
                        case SID_ATTR_SHADOW_XDISTANCE:
                        case SID_ATTR_SHADOW_YDISTANCE:
                            ExecuteAreaDlg( rReq );
                            break;

                        default:
                            break;
                    }

                    return;

                }

                if( pView->AreObjectsMarked() )
                    pView->SetAttrToMarked( *rReq.GetArgs(), false );
                else
                    pView->SetDefaultAttr( *rReq.GetArgs(), false);
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

        case SID_DRAW_HLINK_EDIT:
            if ( pSingleSelectedObj )
                pViewData->GetDispatcher().Execute( SID_HYPERLINK_DIALOG );
            break;

        case SID_DRAW_HLINK_DELETE:
            if ( pSingleSelectedObj )
                SetHlinkForObject( pSingleSelectedObj, OUString() );
            break;

        case SID_OPEN_HYPERLINK:
            if ( nMarkCount == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if ( pObj->IsGroupObject() )
                {
                    SdrPageView* pPV = nullptr;
                    SdrObject* pHit = nullptr;
                    if ( pView->PickObj( pWin->PixelToLogic( pViewData->GetMousePosPixel() ), pView->getHitTolLog(), pHit, pPV, SdrSearchOptions::DEEP ) )
                        pObj = pHit;
                }

                ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj );
                if ( pInfo && !pInfo->GetHlink().isEmpty() )
                    ScGlobal::OpenURL( pInfo->GetHlink(), OUString() );
            }
            break;

        case SID_ATTR_TRANSFORM:
        {
            {
                if ( pView->AreObjectsMarked() )
                {
                    const SfxItemSet* pArgs = rReq.GetArgs();

                    if( !pArgs )
                    {
                        if( rMarkList.GetMark(0) != nullptr )
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                            if( pObj->GetObjIdentifier() == OBJ_CAPTION )
                            {
                                // --------- Itemset fuer Caption --------
                                SfxItemSet aNewAttr(pDoc->GetItemPool());
                                pView->GetAttributes(aNewAttr);
                                // --------- Itemset fuer Groesse und Position --------
                                SfxItemSet aNewGeoAttr(pView->GetGeoAttrFromMarked());

                                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                                if ( pFact )
                                {
                                    std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateCaptionDialog( pWin, pView ));

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
                                }
                            }
                            else
                            {
                                SfxItemSet aNewAttr(pView->GetGeoAttrFromMarked());
                                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                                if(pFact)
                                {
                                    std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSvxTransformTabDialog( pWin, &aNewAttr,pView ));
                                    OSL_ENSURE(pDlg, "Dialog creation failed!");
                                    if (pDlg->Execute() == RET_OK)
                                    {
                                        rReq.Done(*(pDlg->GetOutputItemSet()));
                                        pView->SetGeoAttrToMarked(*pDlg->GetOutputItemSet());
                                    }
                                }
                            }
                        }

                    }
                    else
                        pView->SetGeoAttrToMarked( *pArgs );
                }
            }

            ScTabViewShell* pViewShell = pViewData->GetViewShell();
            SfxBindings& rBindings=pViewShell->GetViewFrame()->GetBindings();
            rBindings.Invalidate(SID_ATTR_TRANSFORM_WIDTH);
            rBindings.Invalidate(SID_ATTR_TRANSFORM_HEIGHT);
            rBindings.Invalidate(SID_ATTR_TRANSFORM_POS_X);
            rBindings.Invalidate(SID_ATTR_TRANSFORM_POS_Y);
            rBindings.Invalidate(SID_ATTR_TRANSFORM_ANGLE);
            rBindings.Invalidate(SID_ATTR_TRANSFORM_ROT_X);
            rBindings.Invalidate(SID_ATTR_TRANSFORM_ROT_Y);
            rBindings.Invalidate(SID_ATTR_TRANSFORM_AUTOWIDTH);
            rBindings.Invalidate(SID_ATTR_TRANSFORM_AUTOHEIGHT);
            break;
        }

        default:
            break;
    }
}

void ScDrawShell::ExecuteMacroAssign( SdrObject* pObj, vcl::Window* pWin )
{
    SvxMacroItem aItem ( SfxGetpApp()->GetPool().GetWhich( SID_ATTR_MACROITEM ) );
    ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj, true );
    if ( !pInfo->GetMacro().isEmpty() )
    {
        SvxMacroTableDtor aTab;
        OUString sMacro = pInfo->GetMacro();
        aTab.Insert(SFX_EVENT_MOUSECLICK_OBJECT, SvxMacro(sMacro, OUString()));
        aItem.SetMacroTable( aTab );
    }

    // create empty itemset for macro-dlg
    std::unique_ptr<SfxItemSet> pItemSet(new SfxItemSet(SfxGetpApp()->GetPool(), SID_ATTR_MACROITEM, SID_ATTR_MACROITEM, SID_EVENTCONFIG, SID_EVENTCONFIG, 0 ));
    pItemSet->Put ( aItem, SID_ATTR_MACROITEM );

    SfxEventNamesItem aNamesItem(SID_EVENTCONFIG);
    aNamesItem.AddEvent( ScResId(RID_SCSTR_ONCLICK), OUString(), SFX_EVENT_MOUSECLICK_OBJECT );
    pItemSet->Put( aNamesItem, SID_EVENTCONFIG );

    css::uno::Reference < css::frame::XFrame > xFrame;
    if (GetViewShell())
        xFrame = GetViewShell()->GetViewFrame()->GetFrame().GetFrameInterface();

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    std::unique_ptr<SfxAbstractDialog> pMacroDlg(pFact->CreateSfxDialog( pWin, *pItemSet, xFrame, SID_EVENTCONFIG ));
    if ( pMacroDlg && pMacroDlg->Execute() == RET_OK )
    {
        const SfxItemSet* pOutSet = pMacroDlg->GetOutputItemSet();
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pOutSet->GetItemState( SID_ATTR_MACROITEM, false, &pItem ))
        {
            OUString sMacro;
            const SvxMacro* pMacro = static_cast<const SvxMacroItem*>(pItem)->GetMacroTable().Get( SFX_EVENT_MOUSECLICK_OBJECT );
            if ( pMacro )
                sMacro = pMacro->GetMacName();

            if ( pObj->IsGroupObject() )
            {
                SdrObjList* pOL = pObj->GetSubList();
                const size_t nObj = pOL->GetObjCount();
                for ( size_t index=0; index<nObj; ++index )
                {
                    pInfo = ScDrawLayer::GetMacroInfo( pOL->GetObj(index), true );
                    pInfo->SetMacro( sMacro );
                }
            }
            else
                pInfo->SetMacro( sMacro );
            lcl_setModified( GetObjectShell() );
        }
    }
}

void ScDrawShell::ExecuteLineDlg( SfxRequest& rReq, sal_uInt16 nTabPage )
{
    ScDrawView*         pView       = pViewData->GetScDrawView();
    bool                bHasMarked  = pView->AreObjectsMarked();
    const SdrObject*    pObj        = nullptr;
    const SdrMarkList&  rMarkList   = pView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    SfxItemSet  aNewAttr( pView->GetDefaultAttr() );
    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, false );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");
    std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSvxLineTabDialog( pViewData->GetDialogParent(),
                &aNewAttr,
            pViewData->GetDocument()->GetDrawLayer(),
            pObj,
            bHasMarked));
    OSL_ENSURE(pDlg, "Dialog creation failed!");
    if ( nTabPage != 0xffff )
        pDlg->SetCurPageId( nTabPage );

    if ( pDlg->Execute() == RET_OK )
    {
        if( bHasMarked )
            pView->SetAttrToMarked( *pDlg->GetOutputItemSet(), false );
        else
            pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), false );

        pView->InvalidateAttribs();
        rReq.Done();
    }
}

void ScDrawShell::ExecuteAreaDlg( SfxRequest& rReq, sal_uInt16 nTabPage )
{
    ScDrawView* pView       = pViewData->GetScDrawView();
    bool        bHasMarked  = pView->AreObjectsMarked();

    SfxItemSet  aNewAttr( pView->GetDefaultAttr() );
    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, false );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    std::unique_ptr<AbstractSvxAreaTabDialog> pDlg(pFact->CreateSvxAreaTabDialog(
        pViewData->GetDialogParent(), &aNewAttr,
        pViewData->GetDocument()->GetDrawLayer(), true));

    if ( nTabPage != 0xffff )
        pDlg->SetCurPageId( nTabPage );

    if ( pDlg->Execute() == RET_OK )
    {
        if( bHasMarked )
            pView->SetAttrToMarked( *pDlg->GetOutputItemSet(), false );
        else
            pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), false );

        pView->InvalidateAttribs();
        rReq.Done();
    }
}

void ScDrawShell::ExecuteTextAttrDlg( SfxRequest& rReq, sal_uInt16 /* nTabPage */ )
{
    ScDrawView* pView       = pViewData->GetScDrawView();
    bool        bHasMarked  = pView->AreObjectsMarked();
    SfxItemSet  aNewAttr    ( pView->GetDefaultAttr() );

    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, false );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateTextTabDialog( pViewData->GetDialogParent(), &aNewAttr, pView ));

    sal_uInt16 nResult = pDlg->Execute();

    if ( RET_OK == nResult )
    {
        if ( bHasMarked )
            pView->SetAttributes( *pDlg->GetOutputItemSet() );
        else
            pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), false );

        pView->InvalidateAttribs();
        rReq.Done();
    }
}

void ScDrawShell::SetHlinkForObject( SdrObject* pObj, const OUString& rHlnk )
{
    if ( pObj )
    {
        ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj, true );
        pInfo->SetHlink( rHlnk );
        lcl_setModified( GetObjectShell() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
