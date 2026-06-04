/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <SvHeaderTabListBox.hxx>
#include <accessibility/accessibletablistbox.hxx>
#include <strings.hrc>
#include <svdata.hxx>

#include <comphelper/types.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/json_writer.hxx>
#include <vcl/accessibility/AccessibleBrowseBoxCheckBoxCell.hxx>
#include <vcl/accessibility/AccessibleBrowseBoxHeaderBar.hxx>
#include <vcl/accessibility/AccessibleBrowseBoxHeaderCell.hxx>
#include <vcl/accessibility/AccessibleBrowseBoxTableCell.hxx>
#include <vcl/headbar.hxx>
#include <vcl/toolkit/svlbitm.hxx>
#include <vcl/toolkit/treelistbox.hxx>
#include <vcl/toolkit/treelistentry.hxx>

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

SvHeaderTabListBox::SvHeaderTabListBox(vcl::Window* pParent, WinBits nWinStyle,
                                       HeaderBar* pHeaderBar)
    : SvTabListBox(pParent, nWinStyle)
{
    assert(pHeaderBar);
    m_xHeaderBar = pHeaderBar;
    SetScrolledHdl(LINK(this, SvHeaderTabListBox, ScrollHdl_Impl));
    m_xHeaderBar->SetCreateAccessibleHdl(LINK(this, SvHeaderTabListBox, CreateAccessibleHdl_Impl));
}

SvHeaderTabListBox::~SvHeaderTabListBox() { disposeOnce(); }

void SvHeaderTabListBox::dispose()
{
    for (rtl::Reference<AccessibleBrowseBoxHeaderCell>& rxChild : m_aAccessibleChildren)
        comphelper::disposeComponent(rxChild);
    m_aAccessibleChildren.clear();
    m_xAccessible.clear();

    m_xHeaderBar.reset();
    SvTabListBox::dispose();
}

HeaderBar* SvHeaderTabListBox::GetHeaderBar() { return m_xHeaderBar; }

bool SvHeaderTabListBox::IsItemChecked(SvTreeListEntry* pEntry, sal_uInt16 nCol)
{
    SvButtonState eState = SvButtonState::Unchecked;
    SvLBoxButton& rItem = static_cast<SvLBoxButton&>(pEntry->GetItem(nCol + 1));

    if (rItem.GetType() == SvLBoxItemType::Button)
    {
        SvItemStateFlags nButtonFlags = rItem.GetButtonFlags();
        eState = SvLBoxButtonData::ConvertToButtonState(nButtonFlags);
    }

    return (eState == SvButtonState::Checked);
}

SvTreeListEntry* SvHeaderTabListBox::InsertEntryToColumn(const OUString& rStr,
                                                         SvTreeListEntry* pParent, sal_uInt32 nPos,
                                                         sal_uInt16 nCol, OUString* pUserData)
{
    SvTreeListEntry* pEntry
        = SvTabListBox::InsertEntryToColumn(rStr, pParent, nPos, nCol, pUserData);
    RecalculateAccessibleChildren();
    return pEntry;
}

void SvHeaderTabListBox::Insert(SvTreeListEntry* pEnt, SvTreeListEntry* pPar, sal_uInt32 nPos)
{
    SvTabListBox::Insert(pEnt, pPar, nPos);
    RecalculateAccessibleChildren();
}

void SvHeaderTabListBox::Insert(SvTreeListEntry* pEntry, sal_uInt32 nRootPos)
{
    SvTabListBox::Insert(pEntry, nRootPos);
    RecalculateAccessibleChildren();
}

IMPL_LINK_NOARG(SvHeaderTabListBox, ScrollHdl_Impl, SvTreeListBox*, void)
{
    m_xHeaderBar->SetOffset(-GetXOffset());
}

IMPL_LINK_NOARG(SvHeaderTabListBox, CreateAccessibleHdl_Impl, HeaderBar*, void)
{
    css::uno::Reference<XAccessible> xAccParent = m_xHeaderBar->GetAccessibleParent();
    if (xAccParent.is())
    {
        rtl::Reference<comphelper::OAccessible> pAccessible = new AccessibleBrowseBoxHeaderBar(
            xAccParent, *this, AccessibleBrowseBoxObjType::ColumnHeaderBar);
        m_xHeaderBar->SetAccessible(pAccessible);
    }
}

void SvHeaderTabListBox::RecalculateAccessibleChildren()
{
    if (!m_aAccessibleChildren.empty())
    {
        sal_uInt32 nCount = (GetRowCount() + 1) * GetColumnCount();
        if (m_aAccessibleChildren.size() < nCount)
            m_aAccessibleChildren.resize(nCount);
        else
        {
            DBG_ASSERT(m_aAccessibleChildren.size() == nCount, "wrong children count");
        }
    }
}

bool SvHeaderTabListBox::IsCellCheckBox(sal_Int32 _nRow, sal_uInt16 _nColumn,
                                        TriState& _rState) const
{
    bool bRet = false;
    SvTreeListEntry* pEntry = GetEntryOnPos(_nRow);
    if (pEntry)
    {
        sal_uInt16 nItemCount = pEntry->ItemCount();
        if (nItemCount > (_nColumn + 1))
        {
            SvLBoxItem& rItem = pEntry->GetItem(_nColumn + 1);
            if (rItem.GetType() == SvLBoxItemType::Button)
            {
                bRet = true;
                _rState = ((static_cast<SvLBoxButton&>(rItem).GetButtonFlags()
                            & SvItemStateFlags::UNCHECKED)
                           == SvItemStateFlags::NONE)
                              ? TRISTATE_TRUE
                              : TRISTATE_FALSE;
            }
        }
        else
        {
            SAL_WARN("svtools.contnr", "SvHeaderTabListBox::IsCellCheckBox(): column out of range");
        }
    }
    return bRet;
}
sal_Int32 SvHeaderTabListBox::GetRowCount() const { return GetEntryCount(); }

sal_uInt16 SvHeaderTabListBox::GetColumnCount() const { return m_xHeaderBar->GetItemCount(); }

sal_Int32 SvHeaderTabListBox::GetCurrRow() const
{
    sal_Int32 nRet = -1;
    SvTreeListEntry* pEntry = GetCurEntry();
    if (pEntry)
    {
        sal_uInt32 nCount = GetEntryCount();
        for (sal_uInt32 i = 0; i < nCount; ++i)
        {
            if (pEntry == GetEntryOnPos(i))
            {
                nRet = i;
                break;
            }
        }
    }

    return nRet;
}

sal_uInt16 SvHeaderTabListBox::GetCurrColumn() const { return 0; }

OUString SvHeaderTabListBox::GetRowDescription(sal_Int32 _nRow) const
{
    return GetEntryText(_nRow);
}

OUString SvHeaderTabListBox::GetColumnDescription(sal_uInt16 _nColumn) const
{
    return m_xHeaderBar->GetItemText(m_xHeaderBar->GetItemId(_nColumn));
}

bool SvHeaderTabListBox::HasRowHeader() const { return false; }

bool SvHeaderTabListBox::GoToCell(sal_Int32 /*_nRow*/, sal_uInt16 /*_nColumn*/) { return false; }

void SvHeaderTabListBox::SetNoSelection() { SvTreeListBox::SelectAll(false); }

void SvHeaderTabListBox::SelectAll() { SvTreeListBox::SelectAll(true); }

void SvHeaderTabListBox::SelectRow(sal_Int32 _nRow, bool _bSelect, bool)
{
    Select(GetEntryOnPos(_nRow), _bSelect);
}

void SvHeaderTabListBox::SelectColumn(sal_uInt16, bool) {}

sal_Int32 SvHeaderTabListBox::GetSelectedRowCount() const { return GetSelectionCount(); }

sal_Int32 SvHeaderTabListBox::GetSelectedColumnCount() const { return 0; }

bool SvHeaderTabListBox::IsRowSelected(sal_Int32 _nRow) const
{
    SvTreeListEntry* pEntry = GetEntryOnPos(_nRow);
    return (pEntry && IsSelected(pEntry));
}

bool SvHeaderTabListBox::IsColumnSelected(sal_Int32) const { return false; }

void SvHeaderTabListBox::GetAllSelectedRows(css::uno::Sequence<sal_Int32>& rRowIndices) const
{
    const sal_Int32 nCount = GetSelectedRowCount();
    rRowIndices.realloc(nCount);
    auto pRows = rRowIndices.getArray();
    SvTreeListEntry* pEntry = FirstSelected();
    sal_Int32 nIndex = 0;
    while (nIndex < nCount && pEntry)
    {
        pRows[nIndex] = GetEntryPos(pEntry);
        pEntry = NextSelected(pEntry);
        ++nIndex;
    }
    assert(nIndex == nCount
           && "Mismatch between GetSelectedRowCount() and count of selected rows when iterating.");
}

void SvHeaderTabListBox::GetAllSelectedColumns(css::uno::Sequence<sal_Int32>&) const {}

bool SvHeaderTabListBox::IsCellVisible(sal_Int32, sal_uInt16) const { return true; }

OUString SvHeaderTabListBox::GetAccessibleCellText(sal_Int32 _nRow, sal_uInt16 _nColumnPos) const
{
    return GetTabEntryText(_nRow, _nColumnPos);
}

tools::Rectangle SvHeaderTabListBox::calcHeaderRect(bool _bIsColumnBar)
{
    tools::Rectangle aRect;
    if (_bIsColumnBar)
    {
        vcl::Window* pParent = m_xHeaderBar->GetAccessibleParentWindow();
        assert(pParent);
        aRect = m_xHeaderBar->GetWindowExtentsRelative(*pParent);
    }
    return aRect;
}

tools::Rectangle SvHeaderTabListBox::calcTableRect()
{
    tools::Rectangle aScreenRect(GetWindowExtentsAbsolute());
    return tools::Rectangle(Point(0, 0), aScreenRect.GetSize());
}

tools::Rectangle SvHeaderTabListBox::calcFieldRectPixel(sal_Int32 _nRow, sal_uInt16 _nColumn,
                                                        bool _bIsHeader)
{
    DBG_ASSERT(!_bIsHeader || 0 == _nRow, "invalid parameters");
    tools::Rectangle aRect;
    SvTreeListEntry* pEntry = GetEntryOnPos(_nRow);
    if (pEntry)
    {
        aRect = _bIsHeader ? calcHeaderRect(true) : GetBoundingRect(pEntry);
        Point aTopLeft = aRect.TopLeft();
        DBG_ASSERT(m_xHeaderBar->GetItemCount() > _nColumn, "invalid column");
        tools::Rectangle aItemRect = m_xHeaderBar->GetItemRect(m_xHeaderBar->GetItemId(_nColumn));
        aTopLeft.setX(aItemRect.Left());
        Size aSize = aItemRect.GetSize();
        aRect = tools::Rectangle(aTopLeft, aSize);
    }

    return aRect;
}

rtl::Reference<comphelper::OAccessible>
SvHeaderTabListBox::CreateAccessibleCell(sal_Int32 _nRow, sal_uInt16 _nColumnPos)
{
    OSL_ENSURE(m_xAccessible.is(), "Invalid call: Accessible is null");

    rtl::Reference<AccessibleBrowseBoxCell> xChild;

    TriState eState = TRISTATE_INDET;
    bool bIsCheckBox = IsCellCheckBox(_nRow, _nColumnPos, eState);
    if (bIsCheckBox)
        xChild = new AccessibleCheckBoxCell(m_xAccessible->getTable(), *this, _nRow, _nColumnPos,
                                            eState, false);
    else
        xChild = new AccessibleBrowseBoxTableCell(m_xAccessible->getTable(), *this, _nRow,
                                                  _nColumnPos);

    return xChild;
}

Reference<XAccessible> SvHeaderTabListBox::CreateAccessibleRowHeader(sal_Int32)
{
    Reference<XAccessible> xHeader;
    return xHeader;
}

Reference<XAccessible> SvHeaderTabListBox::CreateAccessibleColumnHeader(sal_uInt16 _nColumn)
{
    // first call? -> initial list
    if (m_aAccessibleChildren.empty())
    {
        const sal_uInt16 nColumnCount = GetColumnCount();
        m_aAccessibleChildren.resize(nColumnCount);
    }

    // get header
    rtl::Reference<AccessibleBrowseBoxHeaderCell> xChild = m_aAccessibleChildren[_nColumn];
    // already exists?
    if (!xChild.is() && m_xAccessible.is())
    {
        // no -> create new header cell
        xChild = new AccessibleBrowseBoxHeaderCell(_nColumn, m_xAccessible->getHeaderBar(), *this,
                                                   AccessibleBrowseBoxObjType::ColumnHeaderCell);

        // insert into list
        m_aAccessibleChildren[_nColumn] = xChild;
    }
    return xChild;
}

sal_Int32 SvHeaderTabListBox::GetAccessibleControlCount() const { return -1; }

rtl::Reference<comphelper::OAccessible> SvHeaderTabListBox::CreateAccessibleControl(sal_Int32)
{
    return {};
}

bool SvHeaderTabListBox::ConvertPointToControlIndex(sal_Int32&, const Point&) { return false; }

bool SvHeaderTabListBox::ConvertPointToCellAddress(sal_Int32&, sal_uInt16&, const Point&)
{
    return false;
}

bool SvHeaderTabListBox::ConvertPointToRowHeader(sal_Int32&, const Point&) { return false; }

bool SvHeaderTabListBox::ConvertPointToColumnHeader(sal_uInt16&, const Point&) { return false; }

OUString SvHeaderTabListBox::GetAccessibleObjectName(AccessibleBrowseBoxObjType _eType,
                                                     sal_Int32 _nPos) const
{
    OUString aRetText;
    switch (_eType)
    {
        case AccessibleBrowseBoxObjType::BrowseBox:
        case AccessibleBrowseBoxObjType::Table:
        case AccessibleBrowseBoxObjType::ColumnHeaderBar:
            // should be empty now (see #i63983)
            aRetText.clear();
            break;

        case AccessibleBrowseBoxObjType::TableCell:
        {
            // here we need a valid pos, we can not handle -1
            if (_nPos >= 0)
            {
                sal_uInt16 nColumnCount = GetColumnCount();
                if (nColumnCount > 0)
                {
                    sal_Int32 nRow = _nPos / nColumnCount;
                    sal_uInt16 nColumn = static_cast<sal_uInt16>(_nPos % nColumnCount);
                    aRetText = GetCellText(nRow, nColumn);
                }
            }
            break;
        }
        case AccessibleBrowseBoxObjType::CheckBoxCell:
        {
            break; // checkbox cells have no name
        }
        case AccessibleBrowseBoxObjType::ColumnHeaderCell:
        {
            aRetText = m_xHeaderBar->GetItemText(
                m_xHeaderBar->GetItemId(static_cast<sal_uInt16>(_nPos)));
            break;
        }

        case AccessibleBrowseBoxObjType::RowHeaderBar:
        case AccessibleBrowseBoxObjType::RowHeaderCell:
            aRetText = "error";
            break;

        default:
            OSL_FAIL("BrowseBox::GetAccessibleName: invalid enum!");
    }
    return aRetText;
}

OUString SvHeaderTabListBox::GetAccessibleObjectDescription(AccessibleBrowseBoxObjType _eType,
                                                            sal_Int32 _nPos) const
{
    OUString aRetText;

    if (_eType == AccessibleBrowseBoxObjType::TableCell && _nPos != -1)
    {
        sal_uInt16 nColumnCount = GetColumnCount();
        if (nColumnCount > 0)
        {
            sal_Int32 nRow = _nPos / nColumnCount;
            sal_uInt16 nColumn = static_cast<sal_uInt16>(_nPos % nColumnCount);

            OUString aText(VclResId(STR_SVT_ACC_DESC_TABLISTBOX));
            aText = aText.replaceFirst("%1", OUString::number(nRow));
            OUString sColHeader = m_xHeaderBar->GetItemText(m_xHeaderBar->GetItemId(nColumn));
            if (sColHeader.isEmpty())
                sColHeader = OUString::number(nColumn);
            aText = aText.replaceFirst("%2", sColHeader);
            aRetText = aText;
        }
    }

    return aRetText;
}

void SvHeaderTabListBox::FillAccessibleStateSet(sal_Int64& _rStateSet,
                                                AccessibleBrowseBoxObjType _eType) const
{
    switch (_eType)
    {
        case AccessibleBrowseBoxObjType::BrowseBox:
        case AccessibleBrowseBoxObjType::Table:
        {
            _rStateSet |= AccessibleStateType::FOCUSABLE;
            if (HasFocus())
                _rStateSet |= AccessibleStateType::FOCUSED;
            if (IsActive())
                _rStateSet |= AccessibleStateType::ACTIVE;
            if (IsEnabled())
            {
                _rStateSet |= AccessibleStateType::ENABLED;
                _rStateSet |= AccessibleStateType::SENSITIVE;
            }
            if (IsReallyVisible())
                _rStateSet |= AccessibleStateType::VISIBLE;
            if (_eType == AccessibleBrowseBoxObjType::Table)
            {
                _rStateSet |= AccessibleStateType::MANAGES_DESCENDANTS;
                _rStateSet |= AccessibleStateType::MULTI_SELECTABLE;
            }
            break;
        }

        case AccessibleBrowseBoxObjType::ColumnHeaderBar:
        {
            sal_Int32 nCurRow = GetCurrRow();
            sal_uInt16 nCurColumn = GetCurrColumn();
            if (IsCellVisible(nCurRow, nCurColumn))
                _rStateSet |= AccessibleStateType::VISIBLE;
            if (IsEnabled())
                _rStateSet |= AccessibleStateType::ENABLED;
            _rStateSet |= AccessibleStateType::TRANSIENT;
            break;
        }

        case AccessibleBrowseBoxObjType::RowHeaderCell:
        case AccessibleBrowseBoxObjType::ColumnHeaderCell:
        {
            _rStateSet |= AccessibleStateType::VISIBLE;
            _rStateSet |= AccessibleStateType::FOCUSABLE;
            _rStateSet |= AccessibleStateType::TRANSIENT;
            if (IsEnabled())
                _rStateSet |= AccessibleStateType::ENABLED;
            break;
        }
        default:
            break;
    }
}

void SvHeaderTabListBox::FillAccessibleStateSetForCell(sal_Int64& _rStateSet, sal_Int32 _nRow,
                                                       sal_uInt16 _nColumn) const
{
    _rStateSet |= AccessibleStateType::FOCUSABLE;
    _rStateSet |= AccessibleStateType::SELECTABLE;
    _rStateSet |= AccessibleStateType::TRANSIENT;

    if (IsCellVisible(_nRow, _nColumn))
    {
        _rStateSet |= AccessibleStateType::VISIBLE;
        _rStateSet |= AccessibleStateType::ENABLED;
    }

    if (IsRowSelected(_nRow))
    {
        _rStateSet |= AccessibleStateType::ACTIVE;
        if (HasChildPathFocus())
            _rStateSet |= AccessibleStateType::FOCUSED;
        _rStateSet |= AccessibleStateType::SELECTED;
    }
    if (IsEnabled())
        _rStateSet |= AccessibleStateType::ENABLED;
}

void SvHeaderTabListBox::GrabTableFocus() { GrabFocus(); }

tools::Rectangle
SvHeaderTabListBox::GetWindowExtentsRelative(const vcl::Window& rRelativeWindow) const
{
    return Control::GetWindowExtentsRelative(rRelativeWindow);
}

void SvHeaderTabListBox::GrabFocus() { Control::GrabFocus(); }

rtl::Reference<comphelper::OAccessible> SvHeaderTabListBox::GetAccessible()
{
    return Control::GetAccessible();
}

vcl::Window* SvHeaderTabListBox::GetAccessibleParentWindow() const
{
    return Control::GetAccessibleParentWindow();
}

vcl::Window* SvHeaderTabListBox::GetWindowInstance() { return this; }

rtl::Reference<comphelper::OAccessible> SvHeaderTabListBox::CreateAccessible()
{
    if (m_xAccessible.is())
        return m_xAccessible;

    rtl::Reference<comphelper::OAccessible> pAccParent = GetAccessibleParent();
    if (pAccParent.is())
    {
        m_xAccessible = new AccessibleTabListBox(pAccParent, *this);
        return m_xAccessible;
    }
    return nullptr;
}

tools::Rectangle SvHeaderTabListBox::GetFieldCharacterBounds(sal_Int32, sal_Int32, sal_Int32)
{
    return tools::Rectangle();
}

sal_Int32 SvHeaderTabListBox::GetFieldIndexAtPoint(sal_Int32 _nRow, sal_Int32 _nColumnPos,
                                                   const Point& _rPoint)
{
    OUString sText = GetAccessibleCellText(_nRow, static_cast<sal_uInt16>(_nColumnPos));
    std::vector<tools::Rectangle> aRects;
    if (GetOutDev()->GetGlyphBoundRects(Point(0, 0), sText, 0, sText.getLength(), aRects))
    {
        sal_Int32 nPos = 0;
        for (auto const& rectangle : aRects)
        {
            if (rectangle.Contains(_rPoint))
                return nPos;
            ++nPos;
        }
    }

    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
