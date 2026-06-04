/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#pragma once

#include <sal/config.h>
#include <config_options.h>

#include "accessibility/accessibletablistbox.hxx"
#include "svtabbx.hxx"

#include <vcl/dllapi.h>
#include <vcl/toolkit/treelistbox.hxx>
#include <vcl/accessibletableprovider.hxx>

#include <vector>

class HeaderBar;
class AccessibleBrowseBoxHeaderCell;

class UNLESS_MERGELIBS_MORE(VCL_DLLPUBLIC) SvHeaderTabListBox final
    : public SvTabListBox,
      public vcl::IAccessibleTableProvider
{
private:
    VclPtr<HeaderBar> m_xHeaderBar;
    rtl::Reference<AccessibleTabListBox> m_xAccessible;
    std::vector<rtl::Reference<AccessibleBrowseBoxHeaderCell>> m_aAccessibleChildren;

    DECL_DLLPRIVATE_LINK(ScrollHdl_Impl, SvTreeListBox*, void);
    DECL_DLLPRIVATE_LINK(CreateAccessibleHdl_Impl, HeaderBar*, void);

    void RecalculateAccessibleChildren();

public:
    SvHeaderTabListBox(vcl::Window* pParent, WinBits nBits, HeaderBar* pHeaderBar);
    virtual ~SvHeaderTabListBox() override;
    virtual void dispose() override;

    HeaderBar* GetHeaderBar();
    static bool IsItemChecked(SvTreeListEntry* pEntry, sal_uInt16 nCol);

    virtual SvTreeListEntry* InsertEntryToColumn(const OUString&, SvTreeListEntry* pParent,
                                                 sal_uInt32 nPos, sal_uInt16 nCol,
                                                 OUString* pUserData = nullptr) override;
    virtual void Insert(SvTreeListEntry* pEnt, SvTreeListEntry* pPar,
                        sal_uInt32 nPos = TREELIST_APPEND) override;
    virtual void Insert(SvTreeListEntry* pEntry, sal_uInt32 nRootPos = TREELIST_APPEND) override;

    // Accessible -------------------------------------------------------------

    bool IsCellCheckBox(sal_Int32 _nRow, sal_uInt16 _nColumn, TriState& _rState) const;

    virtual sal_Int32 GetRowCount() const override;
    virtual sal_uInt16 GetColumnCount() const override;

    virtual sal_Int32 GetCurrRow() const override;
    virtual sal_uInt16 GetCurrColumn() const override;

    virtual OUString GetRowDescription(sal_Int32 _nRow) const override;
    virtual OUString GetColumnDescription(sal_uInt16 _nColumn) const override;

    virtual bool HasRowHeader() const override;
    /** @return  <TRUE/>, if the object can focus a cell. */
    virtual bool GoToCell(sal_Int32 _nRow, sal_uInt16 _nColumn) override;

    virtual void SetNoSelection() override;
    using SvTabListBox::SelectAll;
    virtual void SelectAll() override;
    virtual void SelectRow(sal_Int32 _nRow, bool _bSelect = true, bool bExpand = true) override;
    virtual void SelectColumn(sal_uInt16 _nColumn, bool _bSelect = true) override;
    virtual sal_Int32 GetSelectedRowCount() const override;
    virtual sal_Int32 GetSelectedColumnCount() const override;
    virtual bool IsRowSelected(sal_Int32 _nRow) const override;
    virtual bool IsColumnSelected(sal_Int32 _nColumn) const override;
    virtual void GetAllSelectedRows(css::uno::Sequence<sal_Int32>& _rRows) const override;
    virtual void GetAllSelectedColumns(css::uno::Sequence<sal_Int32>& _rColumns) const override;

    virtual bool IsCellVisible(sal_Int32 _nRow, sal_uInt16 _nColumn) const override;
    virtual OUString GetAccessibleCellText(sal_Int32 _nRow, sal_uInt16 _nColumnPos) const override;

    virtual tools::Rectangle calcHeaderRect(bool _bIsColumnBar) override;
    virtual tools::Rectangle calcTableRect() override;
    virtual tools::Rectangle calcFieldRectPixel(sal_Int32 _nRow, sal_uInt16 _nColumn,
                                                bool _bIsHeader) override;

    rtl::Reference<comphelper::OAccessible> CreateAccessibleCell(sal_Int32 _nRow,
                                                                 sal_uInt16 _nColumn) override;
    virtual css::uno::Reference<css::accessibility::XAccessible>
    CreateAccessibleRowHeader(sal_Int32 _nRow) override;
    virtual css::uno::Reference<css::accessibility::XAccessible>
    CreateAccessibleColumnHeader(sal_uInt16 _nColumnPos) override;

    virtual sal_Int32 GetAccessibleControlCount() const override;
    rtl::Reference<comphelper::OAccessible> CreateAccessibleControl(sal_Int32 _nIndex) override;
    virtual bool ConvertPointToControlIndex(sal_Int32& _rnIndex, const Point& _rPoint) override;

    virtual bool ConvertPointToCellAddress(sal_Int32& _rnRow, sal_uInt16& _rnColPos,
                                           const Point& _rPoint) override;
    virtual bool ConvertPointToRowHeader(sal_Int32& _rnRow, const Point& _rPoint) override;
    virtual bool ConvertPointToColumnHeader(sal_uInt16& _rnColPos, const Point& _rPoint) override;

    virtual OUString GetAccessibleObjectName(AccessibleBrowseBoxObjType _eType,
                                             sal_Int32 _nPos = -1) const override;
    virtual OUString GetAccessibleObjectDescription(AccessibleBrowseBoxObjType _eType,
                                                    sal_Int32 _nPos = -1) const override;
    virtual vcl::Window* GetWindowInstance() override;

    virtual void FillAccessibleStateSet(sal_Int64& _rStateSet,
                                        AccessibleBrowseBoxObjType _eType) const override;
    virtual void FillAccessibleStateSetForCell(sal_Int64& _rStateSet, sal_Int32 _nRow,
                                               sal_uInt16 _nColumn) const override;
    virtual void GrabTableFocus() override;

    // Window
    virtual tools::Rectangle
    GetWindowExtentsRelative(const vcl::Window& rRelativeWindow) const override;
    virtual void GrabFocus() override;
    virtual rtl::Reference<comphelper::OAccessible> GetAccessible() override;
    /** Creates and returns the accessible object of the whole BrowseBox. */
    virtual rtl::Reference<comphelper::OAccessible> CreateAccessible() override;
    virtual vcl::Window* GetAccessibleParentWindow() const override;

    virtual tools::Rectangle GetFieldCharacterBounds(sal_Int32 _nRow, sal_Int32 _nColumnPos,
                                                     sal_Int32 nIndex) override;
    virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow, sal_Int32 _nColumnPos,
                                           const Point& _rPoint) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
