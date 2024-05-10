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

    aMap[u"SelectionCount"_ustr] = OUString::number(mxTreeList->GetSelectionCount());
    aMap[u"VisibleCount"_ustr] = OUString::number(mxTreeList->GetVisibleCount());
    aMap[u"Children"_ustr] = OUString::number(mxTreeList->GetChildCount(nullptr));
    aMap[u"LevelChildren"_ustr] = OUString::number(mxTreeList->GetLevelChildCount(nullptr));
    aMap[u"CheckBoxList"_ustr] = OUString::boolean(isCheckBoxList(mxTreeList));
    SvTreeListEntry* pEntry = mxTreeList->FirstSelected();
    aMap[u"SelectEntryText"_ustr] = pEntry ? mxTreeList->GetEntryText(pEntry) : OUString();

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
    return u"TreeListUIObject"_ustr;
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

    aMap[u"Text"_ustr] = mxTreeList->GetEntryText(pEntry);
    aMap[u"Children"_ustr] = OUString::number(mxTreeList->GetLevelChildCount(pEntry));
    aMap[u"VisibleChildCount"_ustr] = OUString::number(mxTreeList->GetVisibleChildCount(pEntry));
    aMap[u"IsSelected"_ustr] = OUString::boolean(mxTreeList->IsSelected(pEntry));

    aMap[u"IsSemiTransparent"_ustr] = OUString::boolean(bool(pEntry->GetFlags() & SvTLEntryFlags::SEMITRANSPARENT));

    SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SvLBoxItemType::Button));
    if (pItem)
        aMap[u"IsChecked"_ustr] = OUString::boolean(pItem->IsStateChecked());

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
    return u"TreeListEntry"_ustr;
}

bool TreeListEntryUIObject::equals(const UIObject& rOther) const
{
    const TreeListEntryUIObject* pOther = dynamic_cast<const TreeListEntryUIObject*>(&rOther);
    if (!pOther)
        return false;
    return mxTreeList.get() == pOther->mxTreeList.get()
            && maTreePath == pOther->maTreePath;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
