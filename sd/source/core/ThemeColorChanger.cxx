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

bool changeStyles(sd::DrawDocShell* pDocShell, std::shared_ptr<model::ColorSet> const& pColorSet)
{
    SfxStyleSheetBasePool* pPool = pDocShell->GetStyleSheetPool();

    SdStyleSheet* pStyle = static_cast<SdStyleSheet*>(pPool->First(SfxStyleFamily::Para));
    while (pStyle)
    {
        auto& rItemSet = pStyle->GetItemSet();
        if (const XFillColorItem* pItem = rItemSet.GetItemIfSet(XATTR_FILLCOLOR, false))
        {
            model::ComplexColor const& rComplexColor = pItem->getComplexColor();
            if (rComplexColor.isValidThemeType())
            {
                Color aNewColor = pColorSet->resolveColor(rComplexColor);
                std::unique_ptr<XFillColorItem> pNewItem(pItem->Clone());
                pNewItem->SetColorValue(aNewColor);
                rItemSet.Put(*pNewItem);
            }
        }
        if (const XLineColorItem* pItem = rItemSet.GetItemIfSet(XATTR_LINECOLOR, false))
        {
            model::ComplexColor const& rComplexColor = pItem->getComplexColor();
            if (rComplexColor.isValidThemeType())
            {
                Color aNewColor = pColorSet->resolveColor(rComplexColor);
                std::unique_ptr<XLineColorItem> pNewItem(pItem->Clone());
                pNewItem->SetColorValue(aNewColor);
                rItemSet.Put(*pNewItem);
            }
        }
        pStyle = static_cast<SdStyleSheet*>(pPool->Next());
    }

    return true;
}

} // end anonymous ns

void ThemeColorChanger::apply(std::shared_ptr<model::ColorSet> const& pColorSet)
{
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
}

} // end sd namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
