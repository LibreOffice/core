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

#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdview.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <cmdid.h>

#include <drawsh.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <memory>
#include <svl/stritem.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <svx/chrtitem.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xfltrit.hxx>
#include <comphelper/lok.hxx>
#include <textboxhelper.hxx>

void SwDrawShell::ExecDrawDlg(SfxRequest& rReq)
{
    SwWrtShell*     pSh     = &GetShell();
    SdrView*        pView   = pSh->GetDrawView();
    SdrModel& rModel = pView->GetModel();
    bool            bChanged = rModel.IsChanged();
    rModel.SetChanged(false);

    SfxItemSet aNewAttr(rModel.GetItemPool());
    pView->GetAttributes( aNewAttr );

    GetView().NoRotate();

    switch (rReq.GetSlot())
    {
        case FN_DRAWTEXT_ATTR_DLG:
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateTextTabDialog(rReq.GetFrameWeld(), &aNewAttr, pView));
            auto xRequest = std::make_shared<SfxRequest>(rReq);
            rReq.Ignore(); // the 'old' request is not relevant any more
            pDlg->StartExecuteAsync(
                [pDlg, xRequest=std::move(xRequest), pView, pSh] (sal_Int32 nResult)->void
                {
                    if (nResult == RET_OK)
                    {
                        if (pView->AreObjectsMarked())
                        {
                            pSh->StartAction();
                            pView->SetAttributes(*pDlg->GetOutputItemSet());
                            auto vMarkedObjs = pView->GetMarkedObjects();
                            for (auto pObj : vMarkedObjs)
                            {
                                // If the shape has textframe, set its params as well.
                                if (SwTextBoxHelper::hasTextFrame(pObj))
                                    SwTextBoxHelper::updateTextBoxMargin(pObj);
                            }
                            xRequest->Done(*(pDlg->GetOutputItemSet()));
                            pSh->EndAction();
                        }
                    }
                    pDlg->disposeOnce();
                }
            );
        }
        break;

        case SID_MEASURE_DLG:
        {
            bool bHasMarked = pView->AreObjectsMarked();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateSfxDialog(rReq.GetFrameWeld(),
                                                 aNewAttr, pView, RID_SVXPAGE_MEASURE));
            if (pDlg->Execute() == RET_OK)
            {
                pSh->StartAction();
                if (bHasMarked)
                    pView->SetAttrToMarked(*pDlg->GetOutputItemSet(), false);
                else
                    pView->SetDefaultAttr(*pDlg->GetOutputItemSet(), false);
                pSh->EndAction();
            }
        }
        break;

        case SID_ATTRIBUTES_AREA:
        {
            bool bHasMarked = pView->AreObjectsMarked();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            VclPtr<AbstractSvxAreaTabDialog> pDlg(pFact->CreateSvxAreaTabDialog(rReq.GetFrameWeld(),
                                                                            &aNewAttr,
                                                                            &rModel,
                                                                            true,
                                                                            false));

            pDlg->StartExecuteAsync([bChanged, bHasMarked, &rModel, pDlg, pSh, pView, this](
                                        sal_Int32 nResult){
                rModel.SetChanged(false);

                if (nResult == RET_OK)
                {
                    pSh->StartAction();
                    if (bHasMarked)
                        pView->SetAttributes(*pDlg->GetOutputItemSet());
                    else
                        pView->SetDefaultAttr(*pDlg->GetOutputItemSet(), false);
                    pSh->EndAction();

                    static sal_uInt16 aInval[] =
                    {
                        SID_ATTR_FILL_STYLE,
                        SID_ATTR_FILL_COLOR,
                        SID_ATTR_FILL_TRANSPARENCE,
                        SID_ATTR_FILL_FLOATTRANSPARENCE,
                        0
                    };
                    SfxBindings &rBnd = GetView().GetViewFrame().GetBindings();
                    rBnd.Invalidate(aInval);
                    rBnd.Update(SID_ATTR_FILL_STYLE);
                    rBnd.Update(SID_ATTR_FILL_COLOR);
                    rBnd.Update(SID_ATTR_FILL_TRANSPARENCE);
                    rBnd.Update(SID_ATTR_FILL_FLOATTRANSPARENCE);
                }

                if (rModel.IsChanged())
                    GetShell().SetModified();
                else if (bChanged)
                    rModel.SetChanged();

                pDlg->disposeOnce();
            });
        }
        break;

        case SID_ATTRIBUTES_LINE:
        {
            bool bHasMarked = pView->AreObjectsMarked();

            const SdrObject* pObj = nullptr;
            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
                pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateSvxLineTabDialog(rReq.GetFrameWeld(),
                    &aNewAttr,
                &rModel,
                pObj,
                bHasMarked));

            pDlg->StartExecuteAsync([bChanged, bHasMarked, &rModel, pDlg, pSh, pView, this](
                                        sal_Int32 nResult){
                rModel.SetChanged(false);

                if (nResult == RET_OK)
                {
                    pSh->StartAction();
                    if(bHasMarked)
                        pView->SetAttrToMarked(*pDlg->GetOutputItemSet(), false);
                    else
                        pView->SetDefaultAttr(*pDlg->GetOutputItemSet(), false);
                    pSh->EndAction();

                    static sal_uInt16 aInval[] =
                    {
                        SID_ATTR_LINE_STYLE,                // ( SID_SVX_START + 169 )
                        SID_ATTR_LINE_DASH,                 // ( SID_SVX_START + 170 )
                        SID_ATTR_LINE_WIDTH,                // ( SID_SVX_START + 171 )
                        SID_ATTR_LINE_COLOR,                // ( SID_SVX_START + 172 )
                        SID_ATTR_LINE_START,                // ( SID_SVX_START + 173 )
                        SID_ATTR_LINE_END,                  // ( SID_SVX_START + 174 )
                        SID_ATTR_LINE_TRANSPARENCE,         // (SID_SVX_START+1107)
                        SID_ATTR_LINE_JOINT,                // (SID_SVX_START+1110)
                        SID_ATTR_LINE_CAP,                  // (SID_SVX_START+1111)
                        0
                    };

                    GetView().GetViewFrame().GetBindings().Invalidate(aInval);
                }

                if (rModel.IsChanged())
                    GetShell().SetModified();
                else if (bChanged)
                    rModel.SetChanged();

                pDlg->disposeOnce();
            });
        }
        break;

        default:
            break;
    }

    if (rModel.IsChanged())
        GetShell().SetModified();
    else if (bChanged)
        rModel.SetChanged();
}

namespace
{
    void lcl_convertStringArguments(const std::unique_ptr<SfxItemSet>& pArgs)
    {
        if (const SvxDoubleItem* pWidthItem = pArgs->GetItemIfSet(SID_ATTR_LINE_WIDTH_ARG, false))
        {
            double fValue = pWidthItem->GetValue();
            // FIXME: different units...
            int nPow = 100;
            int nValue = fValue * nPow;

            XLineWidthItem aItem(nValue);
            pArgs->Put(aItem);
        }
        if (const SfxStringItem* pJSON = pArgs->GetItemIfSet(SID_FILL_GRADIENT_JSON, false))
        {
            basegfx::BGradient aGradient = basegfx::BGradient::fromJSON(pJSON->GetValue());
            XFillGradientItem aItem(aGradient);
            pArgs->Put(aItem);
        }
    }
}

void SwDrawShell::ExecDrawAttrArgs(SfxRequest const & rReq)
{
    SwWrtShell* pSh   = &GetShell();
    SdrView*    pView = pSh->GetDrawView();
    const SfxItemSet* pArgs = rReq.GetArgs();
    bool        bChanged = pView->GetModel().IsChanged();
    pView->GetModel().SetChanged(false);

    GetView().NoRotate();

    if (pArgs)
    {
        if(pView->AreObjectsMarked())
        {
            std::unique_ptr<SfxItemSet> pNewArgs = pArgs->Clone();
            lcl_convertStringArguments(pNewArgs);
            pView->SetAttrToMarked(*pNewArgs, false);
        }
        else
            pView->SetDefaultAttr(*rReq.GetArgs(), false);
    }
    else
    {
        SfxDispatcher* pDis = pSh->GetView().GetViewFrame().GetDispatcher();
        switch (rReq.GetSlot())
        {
            case SID_ATTR_FILL_STYLE:
            case SID_ATTR_FILL_COLOR:
            case SID_ATTR_FILL_GRADIENT:
            case SID_ATTR_FILL_HATCH:
            case SID_ATTR_FILL_BITMAP:
            case SID_ATTR_FILL_TRANSPARENCE:
            case SID_ATTR_FILL_FLOATTRANSPARENCE:
                pDis->Execute(SID_ATTRIBUTES_AREA);
                break;
            case SID_ATTR_LINE_STYLE:
            case SID_ATTR_LINE_DASH:
            case SID_ATTR_LINE_WIDTH:
            case SID_ATTR_LINE_COLOR:
            case SID_ATTR_LINE_TRANSPARENCE:
            case SID_ATTR_LINE_JOINT:
            case SID_ATTR_LINE_CAP:
                pDis->Execute(SID_ATTRIBUTES_LINE);
                break;
        }
    }
    if (pView->GetModel().IsChanged())
        GetShell().SetModified();
    else
        if (bChanged)
            pView->GetModel().SetChanged();
}

static void lcl_unifyFillTransparencyItems(const SfxItemSet& rSet)
{
    // Transparent fill options are None, Solid, Linear, Axial, Radial, Elliptical, Square, Rectangular.
    // But this is represented across two items namely XFillTransparenceItem (for None and Solid)
    // and XFillFloatTransparenceItem (for the rest). To simplify the representation in LOKit case let's
    // use XFillFloatTransparenceItem to carry the information of XFillTransparenceItem when gradients
    // are disabled. When gradient transparency is disabled, all fields of XFillFloatTransparenceItem are invalid
    // and not used. So convert XFillTransparenceItem's constant transparency percentage as an intensity
    // and assign this to the XFillFloatTransparenceItem's start-intensity and end-intensity fields.
    // Now the LOK clients need only listen to statechange messages of XFillFloatTransparenceItem
    // to get fill-transparency settings instead of listening to two separate items.

    XFillFloatTransparenceItem* pFillFloatTranspItem =
        const_cast<XFillFloatTransparenceItem*>
        (rSet.GetItem<XFillFloatTransparenceItem>(XATTR_FILLFLOATTRANSPARENCE));
    if (!pFillFloatTranspItem || pFillFloatTranspItem->IsEnabled())
        return;

    const XFillTransparenceItem* pFillTranspItem =
        rSet.GetItem<XFillTransparenceItem>(XATTR_FILLTRANSPARENCE);

    if (!pFillTranspItem)
        return;

    basegfx::BGradient aTmpGradient = pFillFloatTranspItem->GetGradientValue();
    sal_uInt16 nTranspPercent = pFillTranspItem->GetValue();
    // Encode transparency percentage as intensity
    sal_uInt16 nIntensity = 100 - std::min<sal_uInt16>
        (std::max<sal_uInt16>(nTranspPercent, 0), 100);
    aTmpGradient.SetStartIntens(nIntensity);
    aTmpGradient.SetEndIntens(nIntensity);
    pFillFloatTranspItem->SetGradientValue(aTmpGradient);
}

void SwDrawShell::GetDrawAttrState(SfxItemSet& rSet)
{
    SdrView* pSdrView = GetShell().GetDrawView();

    if (pSdrView->AreObjectsMarked())
    {
        bool bDisable = Disable( rSet );

        if( !bDisable )
        {
            SfxItemSet aSet(rSet);
            aSet.MergeRange(SDRATTR_TEXTCOLUMNS_NUMBER, SDRATTR_TEXTCOLUMNS_SPACING);
            pSdrView->GetAttributes(aSet);
            if (const SfxPoolItem* pItem = nullptr;
                aSet.GetItemState(SDRATTR_TEXTCOLUMNS_NUMBER, false, &pItem)
                    >= SfxItemState::DEFAULT
                && pItem)
            {
                aSet.Put(pItem->CloneSetWhich(SID_ATTR_TEXTCOLUMNS_NUMBER));
            }
            if (const SfxPoolItem* pItem = nullptr;
                aSet.GetItemState(SDRATTR_TEXTCOLUMNS_SPACING, false, &pItem)
                    >= SfxItemState::DEFAULT
                && pItem)
            {
                aSet.Put(pItem->CloneSetWhich(SID_ATTR_TEXTCOLUMNS_SPACING));
            }
            rSet.Put(aSet, false);
            if (comphelper::LibreOfficeKit::isActive())
                lcl_unifyFillTransparencyItems(rSet);
        }
    }
    else
        rSet.Put(pSdrView->GetDefaultAttr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
