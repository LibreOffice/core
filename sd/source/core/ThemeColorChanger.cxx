/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <theme/ThemeColorChanger.hxx>
#include <svx/theme/ThemeColorChangerCommon.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svditer.hxx>
#include <docmodel/theme/Theme.hxx>
#include <DrawDocShell.hxx>
#include <stlsheet.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xdef.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <editeng/eeitem.hxx>

#include <unchss.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>

using namespace css;

namespace sd
{
ThemeColorChanger::ThemeColorChanger(SdrPage* pMasterPage, sd::DrawDocShell* pDocShell)
    : mpMasterPage(pMasterPage)
    , mpDocShell(pDocShell)
{
}

ThemeColorChanger::~ThemeColorChanger() = default;

namespace
{
void changeTheTheme(SdrPage* pMasterPage, std::shared_ptr<model::ColorSet> const& pColorSet)
{
    auto pTheme = pMasterPage->getSdrPageProperties().GetTheme();
    if (!pTheme)
    {
        pTheme = std::make_shared<model::Theme>("Office");
        pMasterPage->getSdrPageProperties().SetTheme(pTheme);
    }
    pTheme->setColorSet(pColorSet);
}

bool changeStyle(sd::DrawDocShell* pDocShell, SdStyleSheet* pStyle,
                 std::shared_ptr<model::ColorSet> const& pColorSet)
{
    bool bChanged = false;

    auto aItemSet = pStyle->GetItemSet();
    if (const XFillColorItem* pItem = aItemSet.GetItemIfSet(XATTR_FILLCOLOR, false))
    {
        model::ComplexColor const& rComplexColor = pItem->getComplexColor();
        if (rComplexColor.isValidThemeType())
        {
            Color aNewColor = pColorSet->resolveColor(rComplexColor);
            std::unique_ptr<XFillColorItem> pNewItem(pItem->Clone());
            pNewItem->SetColorValue(aNewColor);
            aItemSet.Put(*pNewItem);
            bChanged = true;
        }
    }
    if (const XLineColorItem* pItem = aItemSet.GetItemIfSet(XATTR_LINECOLOR, false))
    {
        model::ComplexColor const& rComplexColor = pItem->getComplexColor();
        if (rComplexColor.isValidThemeType())
        {
            Color aNewColor = pColorSet->resolveColor(rComplexColor);
            std::unique_ptr<XLineColorItem> pNewItem(pItem->Clone());
            pNewItem->SetColorValue(aNewColor);
            aItemSet.Put(*pNewItem);
            bChanged = true;
        }
    }
    if (const SvxColorItem* pItem = aItemSet.GetItemIfSet(EE_CHAR_COLOR, false))
    {
        model::ComplexColor const& rComplexColor = pItem->getComplexColor();
        if (rComplexColor.isValidThemeType())
        {
            Color aNewColor = pColorSet->resolveColor(rComplexColor);
            std::unique_ptr<SvxColorItem> pNewItem(pItem->Clone());
            pNewItem->setColor(aNewColor);
            aItemSet.Put(*pNewItem);
            bChanged = true;
        }
    }
    if (bChanged)
    {
        pDocShell->GetUndoManager()->AddUndoAction(
            std::make_unique<StyleSheetUndoAction>(pDocShell->GetDoc(), pStyle, &aItemSet));
        pStyle->GetItemSet().Put(aItemSet);
        pStyle->Broadcast(SfxHint(SfxHintId::DataChanged));
    }
    return bChanged;
}

bool changeStyles(sd::DrawDocShell* pDocShell, std::shared_ptr<model::ColorSet> const& pColorSet)
{
    bool bChanged = false;
    SfxStyleSheetBasePool* pPool = pDocShell->GetStyleSheetPool();

    SdStyleSheet* pStyle = static_cast<SdStyleSheet*>(pPool->First(SfxStyleFamily::Para));
    while (pStyle)
    {
        bChanged = changeStyle(pDocShell, pStyle, pColorSet) || bChanged;
        pStyle = static_cast<SdStyleSheet*>(pPool->Next());
    }

    return bChanged;
}

} // end anonymous ns

void ThemeColorChanger::apply(std::shared_ptr<model::ColorSet> const& pColorSet)
{
    auto* pUndoManager = mpDocShell->GetUndoManager();

    ViewShellId nViewShellId(-1);
    if (sd::ViewShell* pViewShell = mpDocShell->GetViewShell())
        nViewShellId = pViewShell->GetViewShellBase().GetViewShellId();
    pUndoManager->EnterListAction(SvxResId(RID_SVXSTR_UNDO_THEME_COLOR_CHANGE), "", 0,
                                  nViewShellId);

    changeStyles(mpDocShell, pColorSet);

    SdrModel& rModel = mpMasterPage->getSdrModelFromSdrPage();
    for (sal_uInt16 nPage = 0; nPage < rModel.GetPageCount(); ++nPage)
    {
        SdrPage* pCurrentPage = rModel.GetPage(nPage);
        if (!pCurrentPage->TRG_HasMasterPage()
            || &pCurrentPage->TRG_GetMasterPage() != mpMasterPage)
            continue;

        for (size_t nObject = 0; nObject < pCurrentPage->GetObjCount(); ++nObject)
        {
            SdrObject* pObject = pCurrentPage->GetObj(nObject);
            svx::theme::updateSdrObject(*pColorSet, pObject);

            // update child objects
            SdrObjList* pList = pObject->GetSubList();
            if (pList)
            {
                SdrObjListIter aIter(pList, SdrIterMode::DeepWithGroups);
                while (aIter.IsMore())
                {
                    svx::theme::updateSdrObject(*pColorSet, aIter.Next());
                }
            }
        }
    }

    changeTheTheme(mpMasterPage, pColorSet);

    pUndoManager->LeaveListAction();
}

} // end sd namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
