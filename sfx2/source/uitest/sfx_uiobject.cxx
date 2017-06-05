/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <uitest/sfx_uiobject.hxx>

#include <sfx2/tabdlg.hxx>

SfxTabDialogUIObject::SfxTabDialogUIObject(const VclPtr<SfxTabDialog>& xTabDialog):
    WindowUIObject(xTabDialog),
    mxTabDialog(xTabDialog)
{
}

SfxTabDialogUIObject::~SfxTabDialogUIObject()
{
}

StringMap SfxTabDialogUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    sal_uInt16 nPageId = mxTabDialog->GetCurPageId();
    std::vector<sal_uInt16> aPageIds = mxTabDialog->m_pTabCtrl->GetPageIDs();
    OUString aStrIds;
    OUString aStrNames;
    for (auto itr = aPageIds.begin(), itrEnd = aPageIds.end();
            itr != itrEnd; ++itr)
    {
        aStrIds = aStrIds + OUString::number(*itr) + ";";
        aStrNames = aStrNames + mxTabDialog->GetPageText(*itr) + ";";
    }

    aMap["PageIds"] = aStrIds;
    aMap["PageNames"] = aStrNames;
    aMap["CurrentPageID"] = OUString::number(nPageId);
    aMap["CurrentPageText"] = mxTabDialog->GetPageText(nPageId);
    return aMap;
}

void SfxTabDialogUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        if (rParameters.find("POS") != rParameters.end())
        {
            auto itr = rParameters.find("POS");
            sal_uInt32 nPos = itr->second.toUInt32();
            std::vector<sal_uInt16> aIds = mxTabDialog->m_pTabCtrl->GetPageIDs();
            sal_uInt16 nPageId = aIds[nPos];
            mxTabDialog->ShowPage(nPageId);
        }
        else if (rParameters.find("NAME") != rParameters.end())
        {
            auto itr = rParameters.find("NAME");
            OUString aName = itr->second;
            std::vector<sal_uInt16> aIds = mxTabDialog->m_pTabCtrl->GetPageIDs();
            auto it = aIds.begin(), itEnd = aIds.end();
            for (; it != itEnd; ++it)
            {
                if (mxTabDialog->GetPageText(*it) == aName)
                    break;
            }

            if (it == aIds.end())
                return;

            mxTabDialog->ShowPage(*it);
        }
    }
}

std::unique_ptr<UIObject> SfxTabDialogUIObject::create(vcl::Window* pWindow)
{
    SfxTabDialog* pDialog = dynamic_cast<SfxTabDialog*>(pWindow);
    assert(pDialog);
    return std::unique_ptr<UIObject>(new SfxTabDialogUIObject(pDialog));
}

OUString SfxTabDialogUIObject::get_name() const
{
    return OUString("SfxTabDialogUIObject");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
