/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uiobject.hxx"

#include "rangeutl.hxx"
#include "gridwin.hxx"

#include "viewdata.hxx"
#include "dbfunc.hxx"

namespace {

ScAddress get_address_from_string(const OUString& rStr)
{
    ScAddress aAddr;
    sal_Int32 nOffset = 0;
    ScRangeStringConverter::GetAddressFromString(aAddr, rStr, nullptr, formula::FormulaGrammar::CONV_OOO, nOffset);
    return aAddr;
}

ScRange get_range_from_string(const OUString& rStr)
{
    ScRange aRange;
    sal_Int32 nOffset = 0;
    ScRangeStringConverter::GetRangeFromString(aRange, rStr, nullptr, formula::FormulaGrammar::CONV_OOO, nOffset);

    return aRange;
}

}

ScGridWinUIObject::ScGridWinUIObject(VclPtr<ScGridWindow> xGridWin):
    WindowUIObject(xGridWin),
    mxGridWindow(xGridWin)
{
}

StringMap ScGridWinUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    aMap["SelectedTable"] = OUString::number(mxGridWindow->getViewData()->GetTabNo());
    aMap["CurrentColumn"] = OUString::number(mxGridWindow->getViewData()->GetCurX());
    aMap["CurrentRow"] = OUString::number(mxGridWindow->getViewData()->GetCurY());
    return aMap;
}

ScDBFunc* ScGridWinUIObject::getDBFunc()
{
    ScViewData* pViewData = mxGridWindow->getViewData();
    ScDBFunc* pFunc = pViewData->GetView();

    return pFunc;
}

void ScGridWinUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        bool bExtend = false;
        if (rParameters.find("EXTEND") != rParameters.end())
        {
            auto itr = rParameters.find("EXTEND");
            if (itr->second.equalsIgnoreAsciiCaseAscii("true") || itr->second == "1")
                bExtend = true;
        }

        if (rParameters.find("CELL") != rParameters.end())
        {
            auto itr = rParameters.find("CELL");
            const OUString& rStr = itr->second;
            ScAddress aAddr = get_address_from_string(rStr);
            ScDBFunc* pFunc = getDBFunc();
            pFunc->MarkRange(ScRange(aAddr), true, bExtend);
            mxGridWindow->CursorChanged();
        }
        else if (rParameters.find("RANGE") != rParameters.end())
        {
            auto itr = rParameters.find("RANGE");
            const OUString rStr = itr->second;
            ScRange aRange = get_range_from_string(rStr);
            ScDBFunc* pFunc = getDBFunc();
            pFunc->MarkRange(aRange, true, bExtend);
            mxGridWindow->CursorChanged();
        }
        else if (rParameters.find("TABLE") != rParameters.end())
        {
            auto itr = rParameters.find("TABLE");
            const OUString rStr = itr->second;
            sal_Int32 nTab = rStr.toUInt32();
            mxGridWindow->getViewData()->SetTabNo(nTab);
        }
        else
        {
            SAL_WARN("sc.uitest", "unknown selection method");
        }
    }
    else
    {
        WindowUIObject::execute(rAction, rParameters);
    }
}

std::unique_ptr<UIObject> ScGridWinUIObject::get_child(const OUString& /*rID*/)
{
    return nullptr;
}

std::unique_ptr<UIObject> ScGridWinUIObject::create(vcl::Window* pWindow)
{
    ScGridWindow* pGridWin = dynamic_cast<ScGridWindow*>(pWindow);
    assert(pGridWin);
    return std::unique_ptr<UIObject>(new ScGridWinUIObject(pGridWin));
}

OUString ScGridWinUIObject::get_name() const
{
    return OUString("ScGridWinUIObject");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
