/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <uiobject.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <navipi.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/viewfrm.hxx>

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
    else if (rAction == "SIDEBAR")
    {
        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        DBG_ASSERT(pViewFrm, "SwEditWinUIObject::execute: no viewframe");
        pViewFrm->ShowChildWindow(SID_SIDEBAR);

        if (rParameters.find("PANEL") != rParameters.end())
        {
            auto itr = rParameters.find("PANEL");
            OUString aVal = itr->second;
            ::sfx2::sidebar::Sidebar::ShowPanel(aVal, pViewFrm->GetFrame().GetFrameInterface());
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

OUString SwEditWinUIObject::get_name() const
{
    return "SwEditWinUIObject";
}

std::unique_ptr<UIObject> SwEditWinUIObject::create(vcl::Window* pWindow)
{
    SwEditWin* pEditWin = dynamic_cast<SwEditWin*>(pWindow);
    assert(pEditWin);
    return std::unique_ptr<UIObject>(new SwEditWinUIObject(pEditWin));
}

SwNavigationPIUIObject::SwNavigationPIUIObject(const VclPtr<SwNavigationPI>& xSwNavigationPI):
    WindowUIObject(xSwNavigationPI),
    mxSwNavigationPI(xSwNavigationPI)
{
}

StringMap SwNavigationPIUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    aMap["selectioncount"] = OUString::number(mxSwNavigationPI->m_xContentTree->count_selected_rows());
    aMap["selectedtext"] = mxSwNavigationPI->m_xContentTree->get_selected_text();

    return aMap;
}

void SwNavigationPIUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "ROOT")
    {
        mxSwNavigationPI->m_xContentTree->grab_focus();
        mxSwNavigationPI->ToolBoxSelectHdl("root");
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> SwNavigationPIUIObject::create(vcl::Window* pWindow)
{
    SwNavigationPI* pSwNavigationPI = dynamic_cast<SwNavigationPI*>(pWindow);
    assert(pSwNavigationPI);
    return std::unique_ptr<UIObject>(new SwNavigationPIUIObject(pSwNavigationPI));
}

OUString SwNavigationPIUIObject::get_name() const
{
    return "SwNavigationPIUIObject";
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
