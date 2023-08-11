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

using namespace css;

namespace sd
{
ThemeColorChanger::ThemeColorChanger(SdrPage* pMasterPage)
    : mpMasterPage(pMasterPage)
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

} // end anonymous ns

void ThemeColorChanger::apply(std::shared_ptr<model::ColorSet> const& pColorSet)
{
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
