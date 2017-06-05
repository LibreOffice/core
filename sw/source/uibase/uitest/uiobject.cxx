/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include "uiobject.hxx"
#include "edtwin.hxx"
#include "view.hxx"
#include "wrtsh.hxx"

SwEditWinUIObject::SwEditWinUIObject(const VclPtr<SwEditWin>& xEditWin):
    WindowUIObject(xEditWin),
    mxEditWin(xEditWin)
{
}

namespace {

SwWrtShell& getWrtShell(VclPtr<SwEditWin> const & xEditWin)
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
    SwWrtShell& rWrtShell = getWrtShell(mxEditWin);
    rWrtShell.GetPageNum(nPageNum, nVirtPageNum);
    aMap["CurrentPage"] = OUString::number(nPageNum);
    rWrtShell.GetPageNum(nPageNum, nVirtPageNum, false);
    aMap["TopVisiblePage"] = OUString::number(nPageNum);

    sal_uInt16 nPages = rWrtShell.GetPageCnt();
    aMap["Pages"] = OUString::number(nPages);

    aMap["StartWord"] = OUString::boolean(rWrtShell.IsStartWord());
    aMap["EndWord"] = OUString::boolean(rWrtShell.IsEndWord());
    aMap["StartSentence"] = OUString::boolean(rWrtShell.IsStartSentence());
    aMap["EndSentence"] = OUString::boolean(rWrtShell.IsEndSentence());
    aMap["StartPara"] = OUString::boolean(rWrtShell.IsSttPara());
    aMap["EndPara"] = OUString::boolean(rWrtShell.IsEndPara());
    aMap["StartDoc"] = OUString::boolean(rWrtShell.IsStartOfDoc());
    aMap["EndDoc"] = OUString::boolean(rWrtShell.IsEndOfDoc());

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
    else if (rAction == "GOTO")
    {
        if (rParameters.find("PAGE") != rParameters.end())
        {
            auto itr = rParameters.find("PAGE");
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            getWrtShell(mxEditWin).GotoPage(nVal, false);
        }
    }
    else if (rAction == "SELECT")
    {
        if (rParameters.find("START_POS") != rParameters.end())
        {
            auto itr = rParameters.find("START_POS");
            OUString aStartPos = itr->second;
            sal_Int32 nStartPos = aStartPos.toInt32();

            itr = rParameters.find("END_POS");
            assert(itr != rParameters.end());
            OUString aEndPos = itr->second;
            sal_Int32 nEndPos = aEndPos.toInt32();

            getWrtShell(mxEditWin).SelectText(nStartPos, nEndPos);
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
