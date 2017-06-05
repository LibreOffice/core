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

#include "rangeutl.hxx"
#include "gridwin.hxx"

#include "viewdata.hxx"
#include "dbfunc.hxx"
#include "tabvwsh.hxx"

#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>

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

ScGridWinUIObject::ScGridWinUIObject(const VclPtr<ScGridWindow>& xGridWin):
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

    ScSplitPos eAlign = mxGridWindow->getViewData()->GetActivePart();
    ScHSplitPos eAlignX = WhichH(eAlign);
    ScVSplitPos eAlignY = WhichV(eAlign);
    aMap["TopVisibleRow"] = OUString::number(mxGridWindow->getViewData()->GetPosY(eAlignY));
    aMap["TopVisibleColumn"] = OUString::number(mxGridWindow->getViewData()->GetPosX(eAlignX));

    ScRangeList aMarkedArea = mxGridWindow->getViewData()->GetMarkData().GetMarkedRanges();
    OUString aMarkedAreaString;
    ScRangeStringConverter::GetStringFromRangeList(aMarkedAreaString, &aMarkedArea, mxGridWindow->getViewData()->GetDocument(), formula::FormulaGrammar::CONV_OOO);

    aMap["MarkedArea"] = aMarkedAreaString;
    return aMap;
}

ScDBFunc* ScGridWinUIObject::getDBFunc()
{
    ScViewData* pViewData = mxGridWindow->getViewData();
    ScDBFunc* pFunc = pViewData->GetView();

    return pFunc;
}

ScDrawView* ScGridWinUIObject::getDrawView()
{
    ScViewData* pViewData = mxGridWindow->getViewData();
    ScDrawView* pDrawView = pViewData->GetScDrawView();

    return pDrawView;
}

ScTabViewShell* ScGridWinUIObject::getViewShell()
{
    ScViewData* pViewData = mxGridWindow->getViewData();
    ScTabViewShell* pViewShell = pViewData->GetViewShell();

    return pViewShell;
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
            if (itr->second.equalsIgnoreAsciiCase("true") || itr->second == "1")
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
        else if (rParameters.find("OBJECT") != rParameters.end())
        {
            auto itr = rParameters.find("OBJECT");
            const OUString rStr = itr->second;

            ScDrawView* pDrawView = getDrawView();
            pDrawView->SelectObject(rStr);
        }
        else
        {
            SAL_WARN("sc.uitest", "unknown selection method");
        }
    }
    else if (rAction == "DESELECT")
    {
        if (rParameters.find("OBJECT") != rParameters.end())
        {
            ScDrawView* pDrawView = getDrawView();
            pDrawView->UnmarkAll();

            ScTabViewShell* pViewShell = getViewShell();
            pViewShell->SetDrawShell(false);
        }
    }
    else if (rAction == "ACTIVATE")
    {
        ScDrawView* pDrawView = getDrawView();
        const SdrMarkList& rMarkList = pDrawView->GetMarkedObjectList();
        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetMarkedSdrObj();
            sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();
            if (nSdrObjKind == OBJ_OLE2)
            {
                ScTabViewShell* pViewShell = getViewShell();
                pViewShell->ActivateObject( static_cast<SdrOle2Obj*>(pObj), 0 );
            }
            else
            {
                SAL_WARN("sc.uitest", "can't activate non-ole objects");
            }
        }
        else
            SAL_WARN("sc.uitest", "can't activate the current selection");
    }
    else if (rAction == "LAUNCH")
    {
        auto itr = rParameters.find("AUTOFILTER");
        if (itr != rParameters.end())
        {
            auto itrCol = rParameters.find("COL");
            if (itrCol == rParameters.end())
            {
                SAL_WARN("sc.uitest", "missing COL parameter");
                return;
            }

            auto itrRow = rParameters.find("ROW");
            if (itrRow == rParameters.end())
            {
                SAL_WARN("sc.uitest", "missing ROW parameter");
                return;
            }
            SCROW nRow = itrRow->second.toUInt32();
            SCCOL nCol = itrCol->second.toUInt32();
            mxGridWindow->LaunchAutoFilterMenu(nCol, nRow);
        }
    }
    else
    {
        WindowUIObject::execute(rAction, rParameters);
    }
}

namespace {

ScDrawLayer* get_draw_layer(VclPtr<ScGridWindow> const & xGridWindow)
{
    return xGridWindow->getViewData()->GetDocument()->GetDrawLayer();
}

SdrPage* get_draw_page(VclPtr<ScGridWindow> const & xGridWindow, SCTAB nTab)
{
    ScDrawLayer* pDrawLayer = get_draw_layer(xGridWindow);

    return pDrawLayer->GetPage(nTab);
}

std::set<OUString> collect_charts(VclPtr<ScGridWindow> const & xGridWindow)
{
    SCTAB nTab = xGridWindow->getViewData()->GetTabNo();
    SdrPage* pPage = get_draw_page(xGridWindow, nTab);

    std::set<OUString> aRet;

    if (!pPage)
        return aRet;

    SdrObjListIter aIter( *pPage, SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        if (pObject->GetObjIdentifier() == OBJ_OLE2)
        {
            aRet.insert(static_cast<SdrOle2Obj*>(pObject)->GetPersistName());
        }
        else
            SAL_DEBUG(pObject->GetName());
        pObject = aIter.Next();
    }

    return aRet;
}

}

std::set<OUString> ScGridWinUIObject::get_children() const
{
    std::set<OUString> aChildren = collect_charts(mxGridWindow);
    return aChildren;
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
