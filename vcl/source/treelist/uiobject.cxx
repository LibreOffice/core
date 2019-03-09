/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <vcl/svlbitm.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/treelistbox.hxx>
#include <vcl/treelistentry.hxx>

TreeListUIObject::TreeListUIObject(const VclPtr<SvTreeListBox>& xTreeList):
    WindowUIObject(xTreeList),
    mxTreeList(xTreeList)
{
}

namespace {

bool isCheckBoxList(const VclPtr<SvTreeListBox>& xTreeList)
{
    return (xTreeList->GetTreeFlags() & SvTreeFlags::CHKBTN) == SvTreeFlags::CHKBTN;
}

}

StringMap TreeListUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    aMap["SelectionCount"] = OUString::number(mxTreeList->GetSelectionCount());
    aMap["VisibleCount"] = OUString::number(mxTreeList->GetVisibleCount());
    aMap["Children"] = OUString::number(mxTreeList->GetChildCount(nullptr));
    aMap["LevelChildren"] = OUString::number(mxTreeList->GetLevelChildCount(nullptr));
    aMap["CheckBoxList"] = OUString::boolean(isCheckBoxList(mxTreeList));
    SvTreeListEntry* pEntry = mxTreeList->FirstSelected();
    aMap["SelectEntryText"] = pEntry ? mxTreeList->GetEntryText(pEntry) : OUString();

    return aMap;
}

void TreeListUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction.isEmpty())
    {
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> TreeListUIObject::get_child(const OUString& rID)
{
    sal_Int32 nID = rID.toInt32();
    if (nID >= 0)
    {
        SvTreeListEntry* pEntry = mxTreeList->GetEntry(nullptr, nID);
        if (!pEntry)
            return nullptr;

        return std::unique_ptr<UIObject>(new TreeListEntryUIObject(mxTreeList, pEntry));
    }

    return nullptr;
}

std::set<OUString> TreeListUIObject::get_children() const
{
    std::set<OUString> aChildren;

    size_t nChildren = mxTreeList->GetLevelChildCount(nullptr);
    for (size_t i = 0; i < nChildren; ++i)
    {
        aChildren.insert(OUString::number(i));
    }

    return aChildren;
}

OUString TreeListUIObject::get_name() const
{
    return OUString("TreeListUIObject");
}

std::unique_ptr<UIObject> TreeListUIObject::create(vcl::Window* pWindow)
{
    SvTreeListBox* pTreeList = dynamic_cast<SvTreeListBox*>(pWindow);
    assert(pTreeList);
    return std::unique_ptr<UIObject>(new TreeListUIObject(pTreeList));
}

TreeListEntryUIObject::TreeListEntryUIObject(const VclPtr<SvTreeListBox>& xTreeList, SvTreeListEntry* pEntry):
    mxTreeList(xTreeList),
    mpEntry(pEntry)
{
}

StringMap TreeListEntryUIObject::get_state()
{
    StringMap aMap;

    aMap["Text"] = mxTreeList->GetEntryText(mpEntry);
    aMap["Children"] = OUString::number(mxTreeList->GetLevelChildCount(mpEntry));
    aMap["VisibleChildCount"] = OUString::number(mxTreeList->GetVisibleChildCount(mpEntry));

    return aMap;
}

void TreeListEntryUIObject::execute(const OUString& rAction, const StringMap& /*rParameters*/)
{
    if (rAction == "COLLAPSE")
    {
        mxTreeList->Collapse(mpEntry);
    }
    else if (rAction == "EXPAND")
    {
        mxTreeList->Expand(mpEntry);
    }
    else if (rAction == "SELECT")
    {
        mxTreeList->Select(mpEntry);
    }
    else if (rAction == "DESELECT")
    {
        mxTreeList->Select(mpEntry, false);
    }
    else if (rAction == "CLICK")
    {
        SvLBoxButton* pItem = static_cast<SvLBoxButton*>(mpEntry->GetFirstItem(SvLBoxItemType::Button));
        if (!pItem)
            return;
        pItem->ClickHdl(mpEntry);
    }
}

std::unique_ptr<UIObject> TreeListEntryUIObject::get_child(const OUString& rID)
{
    sal_Int32 nID = rID.toInt32();
    if (nID >= 0)
    {
        SvTreeListEntry* pEntry = mxTreeList->GetEntry(mpEntry, nID);
        if (!pEntry)
            return nullptr;

        return std::unique_ptr<UIObject>(new TreeListEntryUIObject(mxTreeList, pEntry));
    }

    return nullptr;
}

std::set<OUString> TreeListEntryUIObject::get_children() const
{
    std::set<OUString> aChildren;

    size_t nChildren = mxTreeList->GetLevelChildCount(mpEntry);
    for (size_t i = 0; i < nChildren; ++i)
    {
        aChildren.insert(OUString::number(i));
    }

    return aChildren;
}

OUString TreeListEntryUIObject::get_type() const
{
    return OUString("TreeListEntry");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
