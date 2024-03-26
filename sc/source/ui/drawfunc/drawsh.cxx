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
#include <sc.hrc>

#include <editeng/eeitem.hxx>
#include <svx/fontwork.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/whiter.hxx>

#include <drawsh.hxx>
#include <drwlayer.hxx>
#include <strings.hrc>
#include <viewdata.hxx>
#include <document.hxx>
#include <drawview.hxx>
#include <scresid.hxx>
#include <svx/svdobj.hxx>
#include <tabvwsh.hxx>
#include <gridwin.hxx>
#include <sfx2/bindings.hxx>

#define ShellClass_ScDrawShell
#include <scslots.hxx>

#include <userdat.hxx>
#include <svl/macitem.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/viewsh.hxx>
#include <com/sun/star/util/XModifiable.hpp>
#include <memory>
#include <svx/xlnwtit.hxx>
#include <svx/chrtitem.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <tools/UnitConversion.hxx>
#include <comphelper/lok.hxx>
#include <vcl/unohelp2.hxx>

using namespace css;

SFX_IMPL_INTERFACE(ScDrawShell, SfxShell)

namespace
{
    void lcl_convertStringArguments(SfxItemSet& rArgs)
    {
        if (const SvxDoubleItem* pWidthItem = rArgs.GetItemIfSet(SID_ATTR_LINE_WIDTH_ARG, false))
        {
            double fValue = pWidthItem->GetValue();
            // FIXME: different units...
            int nPow = 100;
            int nValue = fValue * nPow;

            XLineWidthItem aItem(nValue);
            rArgs.Put(aItem);
        }
        if (const SfxStringItem* pJSON = rArgs.GetItemIfSet(SID_FILL_GRADIENT_JSON, false))
        {
            basegfx::BGradient aGradient = basegfx::BGradient::fromJSON(pJSON->GetValue());
            XFillGradientItem aItem(aGradient);
            rArgs.Put(aItem);
        }
    }
}

void ScDrawShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT,
                                            SfxVisibilityFlags::Standard | SfxVisibilityFlags::Server,
                                            ToolbarId::Draw_Objectbar);

    GetStaticInterface()->RegisterPopupMenu("draw");

    GetStaticInterface()->RegisterChildWindow(SvxFontWorkChildWindow::GetChildWindowId());
}

// disable the unwanted Accelerators

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

void ScDrawShell::setModified()
{
    const SfxObjectShell* pShell = GetObjectShell();
    if ( pShell )
    {
        css::uno::Reference< css::util::XModifiable > xModif( pShell->GetModel(), css::uno::UNO_QUERY );
        if ( xModif.is() )
            xModif->setModified( true );
    }
}

static void lcl_invalidateTransformAttr(const ScTabViewShell* pViewShell)
{
    SfxBindings& rBindings=pViewShell->GetViewFrame().GetBindings();
    rBindings.Invalidate(SID_ATTR_TRANSFORM_WIDTH);
    rBindings.Invalidate(SID_ATTR_TRANSFORM_HEIGHT);
    rBindings.Invalidate(SID_ATTR_TRANSFORM_POS_X);
    rBindings.Invalidate(SID_ATTR_TRANSFORM_POS_Y);
    rBindings.Invalidate(SID_ATTR_TRANSFORM_ANGLE);
    rBindings.Invalidate(SID_ATTR_TRANSFORM_ROT_X);
    rBindings.Invalidate(SID_ATTR_TRANSFORM_ROT_Y);
    rBindings.Invalidate(SID_ATTR_TRANSFORM_AUTOWIDTH);
    rBindings.Invalidate(SID_ATTR_TRANSFORM_AUTOHEIGHT);
}

void ScDrawShell::ExecDrawAttr( SfxRequest& rReq )
{
    sal_uInt16              nSlot       = rReq.GetSlot();
    vcl::Window*             pWin        = rViewData.GetActiveWin();
    ScDrawView*         pView       = rViewData.GetScDrawView();
    SdrModel*           pDoc        = rViewData.GetDocument().GetDrawLayer();

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
                    ExecuteMacroAssign(pSingleSelectedObj, pWin ? pWin->GetFrameWeld() : nullptr);
            }
            break;

        case SID_CELL_FORMAT_RESET:
        case SID_TEXT_STANDARD:
            {
                SfxItemSetFixed<SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_MINFRAMEHEIGHT,
                                SDRATTR_TEXT_MAXFRAMEHEIGHT, SDRATTR_TEXT_MAXFRAMEWIDTH> aEmptyAttr(GetPool());

                if (ScDrawLayer::IsNoteCaption(pSingleSelectedObj))
                    aEmptyAttr.Put(pView->GetAttrFromMarked(true));

                pView->SetAttributes(aEmptyAttr, true);
            }
            break;
        case SID_MOVE_SHAPE_HANDLE:
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();
            if (pArgs && pArgs->Count () >= 3)
            {
                const SfxUInt32Item* handleNumItem = rReq.GetArg<SfxUInt32Item>(FN_PARAM_1);
                const SfxUInt32Item* newPosXTwips = rReq.GetArg<SfxUInt32Item>(FN_PARAM_2);
                const SfxUInt32Item* newPosYTwips = rReq.GetArg<SfxUInt32Item>(FN_PARAM_3);
                const SfxInt32Item* OrdNum = rReq.GetArg<SfxInt32Item>(FN_PARAM_4);

                const sal_uLong handleNum = handleNumItem->GetValue();
                const sal_uLong newPosX = convertTwipToMm100(newPosXTwips->GetValue());
                const sal_uLong newPosY = convertTwipToMm100(newPosYTwips->GetValue());

                bool bNegateX = comphelper::LibreOfficeKit::isActive() && rViewData.GetDocument().IsLayoutRTL(rViewData.GetTabNo());
                pView->MoveShapeHandle(handleNum, Point(bNegateX ? -static_cast<tools::Long>(newPosX) : newPosX, newPosY), OrdNum ? OrdNum->GetValue() : -1);
            }
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
                // if toolbar is vertical :
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
                {
                    std::unique_ptr<SfxItemSet> pNewArgs = rReq.GetArgs()->Clone();
                    lcl_convertStringArguments(*pNewArgs);
                    pView->SetAttrToMarked(*pNewArgs, false);
                }
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

        case SID_MEASURE_DLG:
            ExecuteMeasureDlg( rReq );
            break;

        case SID_DRAWTEXT_ATTR_DLG:
            ExecuteTextAttrDlg( rReq );
            break;

        case SID_EDIT_HYPERLINK:
            if ( pSingleSelectedObj )
                rViewData.GetDispatcher().Execute( SID_HYPERLINK_DIALOG );
            break;

        case SID_REMOVE_HYPERLINK:
            if ( pSingleSelectedObj )
            {
                pSingleSelectedObj->setHyperlink(OUString());
                setModified();
            }
            break;

        case SID_OPEN_HYPERLINK:
        case SID_COPY_HYPERLINK_LOCATION:
            if ( nMarkCount == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if ( pObj->IsGroupObject() )
                {
                    SdrPageView* pPV = nullptr;
                    SdrObject* pHit = pView->PickObj(pWin->PixelToLogic(rViewData.GetMousePosPixel()), pView->getHitTolLog(), pPV, SdrSearchOptions::DEEP);
                    if (pHit)
                        pObj = pHit;
                }

                if (!pObj->getHyperlink().isEmpty())
                {
                    if (nSlot == SID_OPEN_HYPERLINK)
                    {
                        ScGlobal::OpenURL(pObj->getHyperlink(), OUString(), true);
                    }
                    else if (nSlot == SID_COPY_HYPERLINK_LOCATION)
                    {
                        uno::Reference<datatransfer::clipboard::XClipboard> xClipboard
                            = GetViewShell()->GetWindow()->GetClipboard();
                        vcl::unohelper::TextDataObject::CopyStringTo(pObj->getHyperlink(), xClipboard);
                    }
                }
            }
            break;

        case SID_ATTR_TRANSFORM:
            {
                if ( pView->AreObjectsMarked() )
                {
                    const SfxItemSet* pArgs = rReq.GetArgs();

                    if( !pArgs )
                    {
                        if( rMarkList.GetMark(0) != nullptr )
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                            std::shared_ptr<SfxRequest> pRequest = std::make_shared<SfxRequest>(rReq);

                            if( pObj->GetObjIdentifier() == SdrObjKind::Caption )
                            {
                                // Caption Itemset
                                SfxItemSet aNewAttr(pDoc->GetItemPool());
                                pView->GetAttributes(aNewAttr);
                                // Size and Position Itemset
                                SfxItemSet aNewGeoAttr(pView->GetGeoAttrFromMarked());

                                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                                VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateCaptionDialog(pWin ? pWin->GetFrameWeld() : nullptr, pView));

                                const WhichRangesContainer& pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
                                SfxItemSet aCombSet( *aNewAttr.GetPool(), pRange );
                                aCombSet.Put( aNewAttr );
                                aCombSet.Put( aNewGeoAttr );
                                pDlg->SetInputSet( &aCombSet );

                                pDlg->StartExecuteAsync([pDlg, pRequest, pView, this](
                                                            sal_Int32 nResult){
                                    if (nResult == RET_OK)
                                    {
                                        pRequest->Done(*(pDlg->GetOutputItemSet()));
                                        pView->SetAttributes(*pDlg->GetOutputItemSet());
                                        pView->SetGeoAttrToMarked(*pDlg->GetOutputItemSet());
                                    }

                                    lcl_invalidateTransformAttr(rViewData.GetViewShell());
                                    pDlg->disposeOnce();
                                });
                            }
                            else
                            {
                                SfxItemSet aNewAttr(pView->GetGeoAttrFromMarked());
                                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                                VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateSvxTransformTabDialog(pWin ? pWin->GetFrameWeld() : nullptr, &aNewAttr, pView));

                                pDlg->StartExecuteAsync([pDlg, pRequest, pView, this](
                                                            sal_Int32 nResult){
                                    if (nResult == RET_OK)
                                    {
                                        pRequest->Done(*(pDlg->GetOutputItemSet()));
                                        pView->SetGeoAttrToMarked(*pDlg->GetOutputItemSet());
                                    }

                                    lcl_invalidateTransformAttr(rViewData.GetViewShell());
                                    pDlg->disposeOnce();
                                });
                            }
                        }

                    }
                    else
                        pView->SetGeoAttrToMarked( *pArgs );
                }

                lcl_invalidateTransformAttr(rViewData.GetViewShell());
            }
            break;

        case SID_ATTR_GLOW_COLOR:
        case SID_ATTR_GLOW_RADIUS:
        case SID_ATTR_GLOW_TRANSPARENCY:
        case SID_ATTR_SOFTEDGE_RADIUS:
        case SID_ATTR_TEXTCOLUMNS_NUMBER:
        case SID_ATTR_TEXTCOLUMNS_SPACING:
            if (const SfxItemSet* pNewArgs = rReq.GetArgs())
                pView->SetAttrToMarked(*pNewArgs, false);
            rReq.Done();
            break;

        default:
            break;
    }
}

void ScDrawShell::ExecuteMacroAssign(SdrObject* pObj, weld::Window* pWin)
{
    SvxMacroItem aItem ( SfxGetpApp()->GetPool().GetWhichIDFromSlotID( SID_ATTR_MACROITEM ) );
    ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj, true );
    if ( !pInfo->GetMacro().isEmpty() )
    {
        SvxMacroTableDtor aTab;
        const OUString& sMacro = pInfo->GetMacro();
        aTab.Insert(SvMacroItemId::OnClick, SvxMacro(sMacro, OUString()));
        aItem.SetMacroTable( aTab );
    }

    // create empty itemset for macro-dlg
    SfxItemSetFixed<SID_ATTR_MACROITEM, SID_ATTR_MACROITEM, SID_EVENTCONFIG, SID_EVENTCONFIG> aItemSet(SfxGetpApp()->GetPool() );
    aItemSet.Put ( aItem );

    SfxEventNamesItem aNamesItem(SID_EVENTCONFIG);
    aNamesItem.AddEvent( ScResId(RID_SCSTR_ONCLICK), OUString(), SvMacroItemId::OnClick );
    aItemSet.Put( aNamesItem );

    css::uno::Reference < css::frame::XFrame > xFrame;
    if (GetViewShell())
        xFrame = GetViewShell()->GetViewFrame().GetFrame().GetFrameInterface();

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    VclPtr<SfxAbstractDialog> pMacroDlg(pFact->CreateEventConfigDialog( pWin, aItemSet, xFrame ));
    pMacroDlg->StartExecuteAsync(
        [this, pMacroDlg, pObj, pInfo] (sal_Int32 nResult) mutable -> void
        {
            ScopedVclPtr<SfxAbstractDialog> pDlgDisposer(std::move(pMacroDlg));
            if (nResult != RET_OK)
                return;

            const SfxItemSet* pOutSet = pDlgDisposer->GetOutputItemSet();
            const SvxMacroItem* pItem = pOutSet->GetItemIfSet( SID_ATTR_MACROITEM, false );
            if( !pItem )
                return;

            OUString sMacro;
            const SvxMacro* pMacro = pItem->GetMacroTable().Get( SvMacroItemId::OnClick );
            if ( pMacro )
                sMacro = pMacro->GetMacName();

            if ( pObj->IsGroupObject() )
            {
                SdrObjList* pOL = pObj->GetSubList();
                for (const rtl::Reference<SdrObject>& pChildObj : *pOL)
                {
                    pInfo = ScDrawLayer::GetMacroInfo( pChildObj.get(), true );
                    pInfo->SetMacro( sMacro );
                }
            }
            else
                pInfo->SetMacro( sMacro );
            setModified();
        }
    );

}

void ScDrawShell::ExecuteLineDlg( const SfxRequest& rReq )
{
    ScDrawView*         pView       = rViewData.GetScDrawView();
    bool                bHasMarked  = pView->AreObjectsMarked();
    const SdrObject*    pObj        = nullptr;
    const SdrMarkList&  rMarkList   = pView->GetMarkedObjectList();

    std::shared_ptr<SfxRequest> xRequest = std::make_shared<SfxRequest>(rReq);

    if( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    SfxItemSet  aNewAttr( pView->GetDefaultAttr() );
    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, false );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateSvxLineTabDialog( rViewData.GetDialogParent(),
                &aNewAttr,
            rViewData.GetDocument().GetDrawLayer(),
            pObj,
            bHasMarked));

    pDlg->StartExecuteAsync([pDlg, xRequest=std::move(xRequest), pView, bHasMarked](sal_Int32 nResult){
        if ( nResult == RET_OK )
        {
            if( bHasMarked )
                pView->SetAttrToMarked( *pDlg->GetOutputItemSet(), false );
            else
                pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), false );

            pView->InvalidateAttribs();
            xRequest->Done();
        }
        pDlg->disposeOnce();
    });
}

void ScDrawShell::ExecuteAreaDlg( const SfxRequest& rReq )
{
    ScDrawView* pView       = rViewData.GetScDrawView();
    bool        bHasMarked  = pView->AreObjectsMarked();

    std::shared_ptr<SfxRequest> xRequest = std::make_shared<SfxRequest>(rReq);

    SfxItemSet  aNewAttr( pView->GetDefaultAttr() );
    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, false );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    weld::Window* pWin = rViewData.GetDialogParent();
    VclPtr<AbstractSvxAreaTabDialog> pDlg(pFact->CreateSvxAreaTabDialog(
        pWin, &aNewAttr,
        rViewData.GetDocument().GetDrawLayer(), true, false));

    pDlg->StartExecuteAsync([pDlg, xRequest=std::move(xRequest), pView, bHasMarked](sal_Int32 nResult){
        if ( nResult == RET_OK )
        {
            if( bHasMarked )
                pView->SetAttrToMarked( *pDlg->GetOutputItemSet(), false );
            else
                pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), false );

            pView->InvalidateAttribs();
            xRequest->Done();
        }
        pDlg->disposeOnce();
    });
}

void ScDrawShell::ExecuteTextAttrDlg( SfxRequest& rReq )
{
    ScDrawView* pView       = rViewData.GetScDrawView();
    bool        bHasMarked  = pView->AreObjectsMarked();
    SfxItemSet  aNewAttr    ( pView->GetDefaultAttr() );

    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, false );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    weld::Window* pWin = rViewData.GetDialogParent();
    ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateTextTabDialog(pWin, &aNewAttr, pView));

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

void ScDrawShell::ExecuteMeasureDlg( SfxRequest& rReq )
{
    ScDrawView* pView       = rViewData.GetScDrawView();
    bool        bHasMarked  = pView->AreObjectsMarked();
    SfxItemSet  aNewAttr    ( pView->GetDefaultAttr() );

    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, false );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    weld::Window* pWin = rViewData.GetDialogParent();
    ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateSfxDialog(pWin, aNewAttr, pView, RID_SVXPAGE_MEASURE));

    sal_uInt16 nResult = pDlg->Execute();

    if ( RET_OK == nResult )
    {
        if ( bHasMarked )
            pView->SetAttrToMarked( *pDlg->GetOutputItemSet(), false );
        else
            pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), false );

        pView->InvalidateAttribs();
        rReq.Done();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
