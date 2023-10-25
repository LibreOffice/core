/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

#include <vcl/toolkit/edit.hxx>
#include <vcl/toolkit/svlbitm.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/toolkit/treelistbox.hxx>
#include <vcl/toolkit/treelistentry.hxx>

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
    else if (auto const pEdit = mxTreeList->GetEditWidget())
    {
        std::unique_ptr<UIObject>(new EditUIObject(pEdit))->execute(rAction, rParameters);
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

        return std::unique_ptr<UIObject>(new TreeListEntryUIObject(mxTreeList, {nID}));
    }
    else if (nID == -1) // FIXME hack?
    {
        if (auto const pEdit = mxTreeList->GetEditWidget())
        {
            return std::unique_ptr<UIObject>(new EditUIObject(pEdit));
        }
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
    return "TreeListUIObject";
}

std::unique_ptr<UIObject> TreeListUIObject::create(vcl::Window* pWindow)
{
    SvTreeListBox* pTreeList = dynamic_cast<SvTreeListBox*>(pWindow);
    assert(pTreeList);
    return std::unique_ptr<UIObject>(new TreeListUIObject(pTreeList));
}

TreeListEntryUIObject::TreeListEntryUIObject(const VclPtr<SvTreeListBox>& xTreeList, std::vector<sal_Int32> nTreePath):
    mxTreeList(xTreeList),
    maTreePath(std::move(nTreePath))
{
}

SvTreeListEntry* TreeListEntryUIObject::getEntry() const
{
    SvTreeListEntry* pEntry = nullptr;
    for (sal_Int32 nID : maTreePath)
    {
        pEntry = mxTreeList->GetEntry(pEntry, nID);
        if (!pEntry)
            throw css::uno::RuntimeException("Could not find child with id: " + OUString::number(nID));
    }
    return pEntry;
}

StringMap TreeListEntryUIObject::get_state()
{
    SvTreeListEntry* pEntry = getEntry();

    StringMap aMap;

    aMap["Text"] = mxTreeList->GetEntryText(pEntry);
    aMap["Children"] = OUString::number(mxTreeList->GetLevelChildCount(pEntry));
    aMap["VisibleChildCount"] = OUString::number(mxTreeList->GetVisibleChildCount(pEntry));
    aMap["IsSelected"] = OUString::boolean(mxTreeList->IsSelected(pEntry));

    aMap["IsSemiTransparent"] = OUString::boolean(bool(pEntry->GetFlags() & SvTLEntryFlags::SEMITRANSPARENT));

    SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SvLBoxItemType::Button));
    if (pItem)
        aMap["IsChecked"] = OUString::boolean(pItem->IsStateChecked());

    return aMap;
}

void TreeListEntryUIObject::execute(const OUString& rAction, const StringMap& /*rParameters*/)
{
    SvTreeListEntry* pEntry = getEntry();

    if (rAction == "COLLAPSE")
    {
        mxTreeList->Collapse(pEntry);
    }
    else if (rAction == "EXPAND")
    {
        mxTreeList->Expand(pEntry);
    }
    else if (rAction == "SELECT")
    {
        mxTreeList->Select(pEntry);
    }
    else if (rAction == "DESELECT")
    {
        mxTreeList->Select(pEntry, false);
    }
    else if (rAction == "CLICK")
    {
        SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SvLBoxItemType::Button));
        if (!pItem)
            return;
        pItem->ClickHdl(pEntry);
    }
    else if (rAction == "DOUBLECLICK")
    {
        mxTreeList->SetCurEntry(pEntry);
        mxTreeList->DoubleClickHdl();
    }
}

std::unique_ptr<UIObject> TreeListEntryUIObject::get_child(const OUString& rID)
{
    SvTreeListEntry* pParentEntry = getEntry();

    sal_Int32 nID = rID.toInt32();
    if (nID >= 0)
    {
        SvTreeListEntry* pEntry = mxTreeList->GetEntry(pParentEntry, nID);
        if (!pEntry)
            return nullptr;

        std::vector<sal_Int32> aChildTreePath(maTreePath);
        aChildTreePath.push_back(nID);
        return std::unique_ptr<UIObject>(new TreeListEntryUIObject(mxTreeList, std::move(aChildTreePath)));
    }

    return nullptr;
}

std::set<OUString> TreeListEntryUIObject::get_children() const
{
    SvTreeListEntry* pEntry = getEntry();

    std::set<OUString> aChildren;

    size_t nChildren = mxTreeList->GetLevelChildCount(pEntry);
    for (size_t i = 0; i < nChildren; ++i)
    {
        aChildren.insert(OUString::number(i));
    }

    return aChildren;
}

OUString TreeListEntryUIObject::get_type() const
{
    return "TreeListEntry";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
