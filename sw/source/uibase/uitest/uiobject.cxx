/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uiobject.hxx"
#include "edtwin.hxx"
#include "view.hxx"
#include "wrtsh.hxx"

SwEditWinUIObject::SwEditWinUIObject(VclPtr<SwEditWin> xEditWin):
    WindowUIObject(xEditWin),
    mxEditWin(xEditWin)
{
}

namespace {

SwWrtShell& getWrtShell(VclPtr<SwEditWin> xEditWin)
{
    return xEditWin->GetView().GetWrtShell();
}

}

StringMap SwEditWinUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    aMap["SelectedText"] = mxEditWin->GetView().GetSelectionText();

    sal_uInt16 nPageNum = 0;
    sal_uInt16 nVirtPageNum = 0;
    getWrtShell(mxEditWin).GetPageNum(nPageNum, nVirtPageNum);
    aMap["CurrentPage"] = OUString::number(nPageNum);
    getWrtShell(mxEditWin).GetPageNum(nPageNum, nVirtPageNum, false);
    aMap["TopVisiblePage"] = OUString::number(nPageNum);

    return aMap;
}

void SwEditWinUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SET")
    {
        if (rParameters.find("ZOOM") != rParameters.end())
        {
            auto itr = rParameters.find("ZOOM");
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            mxEditWin->GetView().SetZoom(SvxZoomType::PERCENT, nVal);
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

OUString SwEditWinUIObject::get_name() const
{
    return OUString("SwEditWinUIObject");
}

std::unique_ptr<UIObject> SwEditWinUIObject::create(vcl::Window* pWindow)
{
    SwEditWin* pEditWin = dynamic_cast<SwEditWin*>(pWindow);
    assert(pEditWin);
    return std::unique_ptr<UIObject>(new SwEditWinUIObject(pEditWin));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
