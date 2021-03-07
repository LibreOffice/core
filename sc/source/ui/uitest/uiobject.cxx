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

#include <rangeutl.hxx>
#include <gridwin.hxx>

#include <viewdata.hxx>
#include <viewfunc.hxx>
#include <dbfunc.hxx>
#include <tabvwsh.hxx>
#include <drwlayer.hxx>
#include <navipi.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <appoptio.hxx>
#include <scmod.hxx>
#include <fudraw.hxx>
#include <postit.hxx>

#include <com/sun/star/embed/EmbedVerbs.hpp>

#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <sal/log.hxx>

namespace {

ScAddress get_address_from_string(const ScDocument& rDoc, const OUString& rStr)
{
    ScAddress aAddr;
    sal_Int32 nOffset = 0;
    ScRangeStringConverter::GetAddressFromString(aAddr, rStr, rDoc, formula::FormulaGrammar::CONV_OOO, nOffset);
    return aAddr;
}

ScRange get_range_from_string(const ScDocument& rDoc, const OUString& rStr)
{
    ScRange aRange;
    sal_Int32 nOffset = 0;
    ScRangeStringConverter::GetRangeFromString(aRange, rStr, rDoc, formula::FormulaGrammar::CONV_OOO, nOffset);

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

    aMap["SelectedTable"] = OUString::number(mxGridWindow->getViewData().GetTabNo());
    aMap["CurrentColumn"] = OUString::number(mxGridWindow->getViewData().GetCurX());
    aMap["CurrentRow"] = OUString::number(mxGridWindow->getViewData().GetCurY());

    ScSplitPos eAlign = mxGridWindow->getViewData().GetActivePart();
    ScHSplitPos eAlignX = WhichH(eAlign);
    ScVSplitPos eAlignY = WhichV(eAlign);
    aMap["TopVisibleRow"] = OUString::number(mxGridWindow->getViewData().GetPosY(eAlignY));
    aMap["TopVisibleColumn"] = OUString::number(mxGridWindow->getViewData().GetPosX(eAlignX));

    ScRangeList aMarkedArea = mxGridWindow->getViewData().GetMarkData().GetMarkedRanges();
    OUString aMarkedAreaString;
    ScRangeStringConverter::GetStringFromRangeList(aMarkedAreaString, &aMarkedArea, &mxGridWindow->getViewData().GetDocument(), formula::FormulaGrammar::CONV_OOO);

    aMap["MarkedArea"] = aMarkedAreaString;

    ScDocument& rDoc = mxGridWindow->getViewData().GetDocument();
    ScAddress aPos( mxGridWindow->getViewData().GetCurX() , mxGridWindow->getViewData().GetCurY() , mxGridWindow->getViewData().GetTabNo() );
    if ( rDoc.HasNote( aPos ) )
    {
        ScPostIt* pNote = rDoc.GetNote(aPos);
        assert(pNote);
        aMap["CurrentCellCommentText"] = pNote->GetText();
    }

    ScAppOptions aOpt = SC_MOD()->GetAppOptions();
    aMap["Zoom"] = OUString::number( aOpt.GetZoom() );
    return aMap;
}

ScDBFunc* ScGridWinUIObject::getDBFunc()
{
    ScViewData& rViewData = mxGridWindow->getViewData();
    ScDBFunc* pFunc = rViewData.GetView();

    return pFunc;
}

ScDrawView* ScGridWinUIObject::getDrawView()
{
    ScViewData& rViewData = mxGridWindow->getViewData();
    ScDrawView* pDrawView = rViewData.GetScDrawView();

    return pDrawView;
}

ScTabViewShell* ScGridWinUIObject::getViewShell()
{
    ScViewData& rViewData = mxGridWindow->getViewData();
    ScTabViewShell* pViewShell = rViewData.GetViewShell();

    return pViewShell;
}

ScViewFunc* ScGridWinUIObject::getViewFunc()
{
    ScViewData& rViewData = mxGridWindow->getViewData();
    ScViewFunc* pViewFunc = rViewData.GetView();

    return pViewFunc;
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
            ScAddress aAddr = get_address_from_string(mxGridWindow->getViewData().GetDocument(), rStr);
            ScDBFunc* pFunc = getDBFunc();
            pFunc->MarkRange(ScRange(aAddr), true, bExtend);
            mxGridWindow->CursorChanged();
        }
        else if (rParameters.find("RANGE") != rParameters.end())
        {
            auto itr = rParameters.find("RANGE");
            const OUString rStr = itr->second;
            ScRange aRange = get_range_from_string(mxGridWindow->getViewData().GetDocument(), rStr);
            ScDBFunc* pFunc = getDBFunc();
            pFunc->MarkRange(aRange, true, bExtend);
            mxGridWindow->CursorChanged();
        }
        else if (rParameters.find("TABLE") != rParameters.end())
        {
            auto itr = rParameters.find("TABLE");
            const OUString rStr = itr->second;
            sal_Int32 nTab = rStr.toUInt32();
            ScTabView* pTabView = mxGridWindow->getViewData().GetView();
            if (pTabView)
                pTabView->SetTabNo(nTab);
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
                pViewShell->ActivateObject(static_cast<SdrOle2Obj*>(pObj), css::embed::EmbedVerbs::MS_OLEVERB_PRIMARY);
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
        if ( rParameters.find("AUTOFILTER") != rParameters.end())
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
        else if ( rParameters.find("SELECTMENU") != rParameters.end())
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
            mxGridWindow->LaunchDataSelectMenu(nCol, nRow);
        }
    }
    else if (rAction == "COMMENT")
    {
        if ( rParameters.find("OPEN") != rParameters.end() )
        {
            ScViewFunc* pViewFunc = getViewFunc();
            pViewFunc->EditNote();
        }
        else if ( rParameters.find("CLOSE") != rParameters.end() )
        {
            FuDraw* pDraw = dynamic_cast<FuDraw*>(getViewFunc()->GetDrawFuncPtr());
            assert(pDraw);
            ScViewData& rViewData = mxGridWindow->getViewData();
            rViewData.GetDispatcher().Execute( pDraw->GetSlotID() , SfxCallMode::SLOT | SfxCallMode::RECORD );
        }
        else if ( rParameters.find("SETTEXT") != rParameters.end() )
        {
            auto itr = rParameters.find("SETTEXT");
            const OUString rStr = itr->second;
            ScDocument& rDoc = mxGridWindow->getViewData().GetDocument();
            ScAddress aPos( mxGridWindow->getViewData().GetCurX() , mxGridWindow->getViewData().GetCurY() , mxGridWindow->getViewData().GetTabNo() );
            rDoc.GetOrCreateNote( aPos )->SetText( aPos , rStr );
        }
    }
    else if (rAction == "SIDEBAR")
    {
        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        DBG_ASSERT(pViewFrm, "ScGridWinUIObject::execute: no viewframe");
        pViewFrm->ShowChildWindow(SID_SIDEBAR);

        auto itr = rParameters.find("PANEL");
        if (itr != rParameters.end())
        {
            OUString aVal = itr->second;
            ::sfx2::sidebar::Sidebar::ShowPanel(aVal, pViewFrm->GetFrame().GetFrameInterface());
        }
    }
    else if (rAction == "SET")
    {
        if (rParameters.find("ZOOM") != rParameters.end())
        {
            auto itr = rParameters.find("ZOOM");
            OUString aVal = itr->second;
            sal_Int32 nVal = aVal.toInt32();
            ScTabViewShell* pViewShell = getViewShell();
            ScModule*  pScMod = SC_MOD();
            if( nVal )
            {
                ScAppOptions aNewOpt = pScMod->GetAppOptions();
                aNewOpt.SetZoom( nVal );
                pScMod->SetAppOptions( aNewOpt );
                Fraction aFract( nVal, 100 );
                pViewShell->SetZoom( aFract, aFract, true );
                pViewShell->PaintGrid();
                pViewShell->PaintTop();
                pViewShell->PaintLeft();
            }
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
    return xGridWindow->getViewData().GetDocument().GetDrawLayer();
}

SdrPage* get_draw_page(VclPtr<ScGridWindow> const & xGridWindow, SCTAB nTab)
{
    ScDrawLayer* pDrawLayer = get_draw_layer(xGridWindow);

    return pDrawLayer->GetPage(nTab);
}

std::set<OUString> collect_charts(VclPtr<ScGridWindow> const & xGridWindow)
{
    SCTAB nTab = xGridWindow->getViewData().GetTabNo();
    SdrPage* pPage = get_draw_page(xGridWindow, nTab);

    std::set<OUString> aRet;

    if (!pPage)
        return aRet;

    SdrObjListIter aIter( pPage, SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        if (pObject->GetObjIdentifier() == OBJ_OLE2)
        {
            aRet.insert(static_cast<SdrOle2Obj*>(pObject)->GetPersistName());
        }
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
    return "ScGridWinUIObject";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
