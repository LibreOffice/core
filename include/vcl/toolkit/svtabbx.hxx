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

#pragma once

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <config_options.h>
#include <accessibility/accessibletablistbox.hxx>
#include <vcl/dllapi.h>
#include <vcl/toolkit/treelistbox.hxx>
#include <vcl/accessibletableprovider.hxx>

#include <tools/debug.hxx>

#include <memory>
#include <vector>

class UNLESS_MERGELIBS_MORE(VCL_DLLPUBLIC) SvTabListBox : public SvTreeListBox
{
private:
    std::vector<SvLBoxTab>      mvTabList;
    OUString                    aCurEntry;

protected:
    static std::u16string_view  GetToken( std::u16string_view sStr, sal_Int32 &nIndex );

    virtual void                SetTabs() override;
    virtual void                InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&) override;

    OUString                    GetTabEntryText( sal_uInt32 nPos, sal_uInt16 nCol ) const;
    SvTreeListEntry*            GetEntryOnPos( sal_uInt32 _nEntryPos ) const;
    SvTreeListEntry*            GetChildOnPos( SvTreeListEntry* _pParent, sal_uInt32 _nEntryPos, sal_uInt32& _rPos ) const;

public:
    SvTabListBox( vcl::Window* pParent, WinBits );
    virtual ~SvTabListBox() override;
    virtual void dispose() override;
    void SetTabs(const std::vector<tools::Long>& rTabPositions, MapUnit = MapUnit::MapAppFont);
    using SvTreeListBox::GetTab;
    tools::Long            GetLogicTab( sal_uInt16 nTab );

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText, SvTreeListEntry* pParent = nullptr,
                                         bool bChildrenOnDemand = false,
                                         sal_uInt32 nPos=TREELIST_APPEND, void* pUserData = nullptr ) override;

    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, SvTreeListEntry* pParent,
                                 sal_uInt32 nPos, sal_uInt16 nCol, void* pUserData = nullptr );

    virtual OUString GetEntryText( SvTreeListEntry* pEntry ) const override;
    static OUString  GetEntryText( const SvTreeListEntry*, sal_uInt16 nCol );
    OUString         GetEntryText( sal_uInt32 nPos, sal_uInt16 nCol = 0xffff ) const;
    using SvTreeListBox::SetEntryText;
    OUString         GetCellText( sal_uInt32 nPos, sal_uInt16 nCol ) const;
    sal_uInt32       GetEntryPos( const SvTreeListEntry* pEntry ) const;

    void             SetTabAlignCenter(sal_uInt16 nTab);
    void             SetTabEditable( sal_uInt16 nTab, bool bEditable );

    virtual void     DumpAsPropertyTree(tools::JsonWriter& rJsonWriter) override;
};

// class SvHeaderTabListBox ---------------------------------------------------

class HeaderBar;
namespace vcl {
    struct SvHeaderTabListBoxImpl;
}
class AccessibleBrowseBoxHeaderCell;

class UNLESS_MERGELIBS_MORE(VCL_DLLPUBLIC) SvHeaderTabListBox : public SvTabListBox, public vcl::IAccessibleTableProvider
{
private:
    bool                            m_bFirstPaint;
    VclPtr<HeaderBar> m_xHeaderBar;
    rtl::Reference<AccessibleTabListBox>  m_xAccessible;
    std::vector<rtl::Reference<AccessibleBrowseBoxHeaderCell>> m_aAccessibleChildren;

    Link<SvTreeListEntry*, bool> m_aEditingEntryHdl;
    Link<const IterString&, bool> m_aEditedEntryHdl;

    DECL_DLLPRIVATE_LINK( ScrollHdl_Impl, SvTreeListBox*, void );
    DECL_DLLPRIVATE_LINK( CreateAccessibleHdl_Impl, HeaderBar*, void );

    void            RecalculateAccessibleChildren();

public:
    SvHeaderTabListBox(vcl::Window* pParent, WinBits nBits, HeaderBar* pHeaderBar);
    virtual ~SvHeaderTabListBox() override;
    virtual void dispose() override;

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;

    HeaderBar*      GetHeaderBar();
    static bool     IsItemChecked( SvTreeListEntry* pEntry, sal_uInt16 nCol );

    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, SvTreeListEntry* pParent,
                                 sal_uInt32 nPos, sal_uInt16 nCol, void* pUserData = nullptr ) override;
    virtual sal_uInt32 Insert( SvTreeListEntry* pEnt,SvTreeListEntry* pPar,sal_uInt32 nPos=TREELIST_APPEND) override;
    virtual sal_uInt32 Insert( SvTreeListEntry* pEntry, sal_uInt32 nRootPos = TREELIST_APPEND ) override;

    // Accessible -------------------------------------------------------------

    bool            IsCellCheckBox( sal_Int32 _nRow, sal_uInt16 _nColumn, TriState& _rState ) const;

    /** @return  The count of the rows. */
    virtual sal_Int32               GetRowCount() const override;
    /** @return  The count of the columns. */
    virtual sal_uInt16              GetColumnCount() const override;

    /** @return  The position of the current row. */
    virtual sal_Int32               GetCurrRow() const override;
    /** @return  The position of the current column. */
    virtual sal_uInt16              GetCurrColumn() const override;

    /** @return  The description of a row.
        @param _nRow The row which description is in demand. */
    virtual OUString         GetRowDescription( sal_Int32 _nRow ) const override;
    /** @return  The description of a column.
        @param _nColumn The column which description is in demand. */
    virtual OUString         GetColumnDescription( sal_uInt16 _nColumn ) const override;

    /** @return  <TRUE/>, if the object has a row header. */
    virtual bool                    HasRowHeader() const override; //GetColumnId
    /** @return  <TRUE/>, if the object can focus a cell. */
    virtual bool                    GoToCell( sal_Int32 _nRow, sal_uInt16 _nColumn ) override;

    virtual void                    SetNoSelection() override;
    using SvTabListBox::SelectAll;
    virtual void                    SelectAll() override;
    virtual void                    SelectRow( sal_Int32 _nRow, bool _bSelect = true, bool bExpand = true ) override;
    virtual void                    SelectColumn( sal_uInt16 _nColumn, bool _bSelect = true ) override;
    virtual sal_Int32               GetSelectedRowCount() const override;
    virtual sal_Int32               GetSelectedColumnCount() const override;
    /** @return  <TRUE/>, if the row is selected. */
    virtual bool                    IsRowSelected( sal_Int32 _nRow ) const override;
    virtual bool                    IsColumnSelected( sal_Int32 _nColumn ) const override;
    virtual void                    GetAllSelectedRows( css::uno::Sequence< sal_Int32 >& _rRows ) const override;
    virtual void                    GetAllSelectedColumns( css::uno::Sequence< sal_Int32 >& _rColumns ) const override;

    /** @return  <TRUE/>, if the cell is visible. */
    virtual bool                    IsCellVisible( sal_Int32 _nRow, sal_uInt16 _nColumn ) const override;
    virtual OUString                GetAccessibleCellText( sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const override;

    virtual tools::Rectangle calcHeaderRect(bool _bIsColumnBar) override;
    virtual tools::Rectangle calcTableRect() override;
    virtual tools::Rectangle calcFieldRectPixel(sal_Int32 _nRow, sal_uInt16 _nColumn, bool _bIsHeader) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleRowHeader( sal_Int32 _nRow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleColumnHeader( sal_uInt16 _nColumnPos ) override;

    virtual sal_Int32               GetAccessibleControlCount() const override;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleControl( sal_Int32 _nIndex ) override;
    virtual bool                    ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint ) override;

    virtual bool                    ConvertPointToCellAddress( sal_Int32& _rnRow, sal_uInt16& _rnColPos, const Point& _rPoint ) override;
    virtual bool                    ConvertPointToRowHeader( sal_Int32& _rnRow, const Point& _rPoint ) override;
    virtual bool                    ConvertPointToColumnHeader( sal_uInt16& _rnColPos, const Point& _rPoint ) override;

    virtual OUString                GetAccessibleObjectName( AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const override;
    virtual OUString                GetAccessibleObjectDescription( AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const override;
    virtual vcl::Window*                 GetWindowInstance() override;

    virtual void                    FillAccessibleStateSet( sal_Int64& _rStateSet, AccessibleBrowseBoxObjType _eType ) const override;
    virtual void                    FillAccessibleStateSetForCell( sal_Int64& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumn ) const override;
    virtual void                    GrabTableFocus() override;

    // OutputDevice
    virtual bool                    GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex, int nLen, std::vector< tools::Rectangle >& rVector ) override;

    // Window
    virtual tools::Rectangle        GetWindowExtentsRelative(const vcl::Window& rRelativeWindow) const override;
    virtual void                    GrabFocus() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > GetAccessible() override;
    /** Creates and returns the accessible object of the whole BrowseBox. */
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;
    virtual vcl::Window*            GetAccessibleParentWindow() const override;

    virtual tools::Rectangle        GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex) override;
    virtual sal_Int32               GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint) override;

    virtual void DumpAsPropertyTree(tools::JsonWriter& rJsonWriter) override;

    void SetEditingEntryHdl(const Link<SvTreeListEntry*, bool>& rLink)
    {
        m_aEditingEntryHdl = rLink;
    }

    void SetEditedEntryHdl(const Link<const IterString&, bool>& rLink)
    {
        m_aEditedEntryHdl = rLink;
    }

    //the default NotifyStartDrag is weird to me, and defaults to enabling all
    //possibilities when drag starts, while restricting it to some subset of
    //the configured drag drop mode would make more sense to me, but I'm not
    //going to change the baseclass
    virtual DragDropMode NotifyStartDrag() override { return GetDragDropMode(); }

    virtual bool EditingEntry(SvTreeListEntry* pEntry) override
    {
        return m_aEditingEntryHdl.Call(pEntry);
    }

    virtual bool EditedEntry(SvTreeListEntry* pEntry, const OUString& rNewText) override
    {
        return m_aEditedEntryHdl.Call(IterString(pEntry, rNewText));
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
