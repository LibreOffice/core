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
#ifndef INCLUDED_VCL_SVTABBX_HXX
#define INCLUDED_VCL_SVTABBX_HXX

#include <vcl/dllapi.h>
#include <vcl/treelistbox.hxx>
#include <vcl/accessibletableprovider.hxx>

#include <tools/debug.hxx>

#include <memory>
#include <vector>

enum class SvTabJustify
{
    AdjustRight = static_cast<int>(SvLBoxTabFlags::ADJUST_RIGHT),
    AdjustLeft = static_cast<int>(SvLBoxTabFlags::ADJUST_LEFT),
    AdjustCenter = static_cast<int>(SvLBoxTabFlags::ADJUST_CENTER)
};

struct TabListBoxEventData
{
    SvTreeListEntry* const    m_pEntry;
    sal_uInt16 const          m_nColumn;
    OUString const            m_sOldText;

    TabListBoxEventData( SvTreeListEntry* pEntry, sal_uInt16 nColumn, const OUString& rOldText ) :
        m_pEntry( pEntry ), m_nColumn( nColumn ), m_sOldText( rOldText ) {}
};

class VCL_DLLPUBLIC SvTabListBox : public SvTreeListBox
{
private:
    std::vector<SvLBoxTab>      mvTabList;
    OUString                    aCurEntry;

protected:
    static OUString             GetToken( const OUString &sStr, sal_Int32 &nIndex );

    virtual void                SetTabs() override;
    virtual void                InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind) override;

    OUString                    GetTabEntryText( sal_uLong nPos, sal_uInt16 nCol ) const;
    SvTreeListEntry*            GetEntryOnPos( sal_uLong _nEntryPos ) const;
    SvTreeListEntry*            GetChildOnPos( SvTreeListEntry* _pParent, sal_uLong _nEntryPos, sal_uLong& _rPos ) const;

public:
    SvTabListBox( vcl::Window* pParent, WinBits );
    virtual ~SvTabListBox() override;
    virtual void dispose() override;
    void            SetTabs(sal_uInt16 nTabs, long const pTabPositions[], MapUnit = MapUnit::MapAppFont);
    sal_uInt16      TabCount() const { return mvTabList.size(); }
    using SvTreeListBox::GetTab;
    long            GetTab( sal_uInt16 nTab ) const;
    void            SetTab( sal_uInt16 nTab, long nValue, MapUnit = MapUnit::MapAppFont );
    long            GetLogicTab( sal_uInt16 nTab );

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText, SvTreeListEntry* pParent = nullptr,
                                         bool bChildrenOnDemand = false,
                                         sal_uLong nPos=TREELIST_APPEND, void* pUserData = nullptr,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind::EnabledCheckbox ) override;

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText,
                                         const Image& rExpandedEntryBmp,
                                         const Image& rCollapsedEntryBmp,
                                         SvTreeListEntry* pParent = nullptr,
                                         bool bChildrenOnDemand = false,
                                         sal_uLong nPos = TREELIST_APPEND, void* pUserData = nullptr,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind::EnabledCheckbox ) override;

    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, sal_uLong nPos = TREELIST_APPEND,
                                 sal_uInt16 nCol = 0xffff, void* pUserData = nullptr );
    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, SvTreeListEntry* pParent,
                                 sal_uLong nPos, sal_uInt16 nCol, void* pUserData = nullptr );
    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, const Image& rExpandedEntryBmp,
                                 const Image& rCollapsedEntryBmp, SvTreeListEntry* pParent,
                                 sal_uLong nPos = TREELIST_APPEND, sal_uInt16 nCol = 0xffff, void* pUserData = nullptr );

    virtual OUString GetEntryText( SvTreeListEntry* pEntry ) const override;
    static OUString  GetEntryText( SvTreeListEntry*, sal_uInt16 nCol );
    OUString         GetEntryText( sal_uLong nPos, sal_uInt16 nCol = 0xffff ) const;
    using SvTreeListBox::SetEntryText;
    void             SetEntryText(const OUString&, sal_uLong, sal_uInt16 nCol);
    void             SetEntryText(const OUString&, SvTreeListEntry*, sal_uInt16 nCol=0xffff);
    OUString         GetCellText( sal_uLong nPos, sal_uInt16 nCol ) const;
    sal_uLong        GetEntryPos( const OUString&, sal_uInt16 nCol = 0xffff );
    sal_uLong        GetEntryPos( const SvTreeListEntry* pEntry ) const;

    void             SetTabJustify( sal_uInt16 nTab, SvTabJustify );
};

inline long SvTabListBox::GetTab( sal_uInt16 nTab ) const
{
    DBG_ASSERT( nTab < mvTabList.size(), "GetTabPos:Invalid Tab" );
    return mvTabList[nTab].GetPos();
}

// class SvHeaderTabListBox ---------------------------------------------------

class HeaderBar;
namespace vcl {
    struct SvHeaderTabListBoxImpl;
}

class VCL_DLLPUBLIC SvHeaderTabListBox : public SvTabListBox, public vcl::IAccessibleTableProvider
{
private:
    typedef ::std::vector< css::uno::Reference< css::accessibility::XAccessible > > AccessibleChildren;

    bool                            m_bFirstPaint;
    std::unique_ptr<::vcl::SvHeaderTabListBoxImpl>  m_pImpl;
    ::vcl::IAccessibleTabListBox*   m_pAccessible;
    AccessibleChildren              m_aAccessibleChildren;

    DECL_DLLPRIVATE_LINK( ScrollHdl_Impl, SvTreeListBox*, void );
    DECL_DLLPRIVATE_LINK( CreateAccessibleHdl_Impl, HeaderBar*, void );

    void            RecalculateAccessibleChildren();

public:
    SvHeaderTabListBox( vcl::Window* pParent, WinBits nBits );
    virtual ~SvHeaderTabListBox() override;
    virtual void dispose() override;

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;

    void            InitHeaderBar(HeaderBar* pHeaderBar);
    HeaderBar*      GetHeaderBar();
    static bool     IsItemChecked( SvTreeListEntry* pEntry, sal_uInt16 nCol );

    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, sal_uLong nPos = TREELIST_APPEND,
                                 sal_uInt16 nCol = 0xffff, void* pUserData = nullptr ) override;
    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, SvTreeListEntry* pParent,
                                 sal_uLong nPos, sal_uInt16 nCol, void* pUserData = nullptr ) override;
    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, const Image& rExpandedEntryBmp,
                                 const Image& rCollapsedEntryBmp, SvTreeListEntry* pParent,
                                 sal_uLong nPos = TREELIST_APPEND, sal_uInt16 nCol = 0xffff, void* pUserData = nullptr ) override;
    virtual sal_uLong Insert( SvTreeListEntry* pEnt,SvTreeListEntry* pPar,sal_uLong nPos=TREELIST_APPEND) override;
    virtual sal_uLong Insert( SvTreeListEntry* pEntry, sal_uLong nRootPos = TREELIST_APPEND ) override;
    void            RemoveEntry( SvTreeListEntry const * _pEntry );
    void            Clear();

    // Accessible -------------------------------------------------------------

    void     DisableTransientChildren()          { SetChildrenNotTransient(); }
    bool     IsTransientChildrenDisabled() const { return !AreChildrenTransient(); }

    bool            IsCellCheckBox( long _nRow, sal_uInt16 _nColumn, TriState& _rState );

    /** @return  The count of the rows. */
    virtual long                    GetRowCount() const override;
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
    virtual void                    SelectRow( long _nRow, bool _bSelect = true, bool bExpand = true ) override;
    virtual void                    SelectColumn( sal_uInt16 _nColumn, bool _bSelect = true ) override;
    virtual sal_Int32               GetSelectedRowCount() const override;
    virtual sal_Int32               GetSelectedColumnCount() const override;
    /** @return  <TRUE/>, if the row is selected. */
    virtual bool                    IsRowSelected( long _nRow ) const override;
    virtual bool                    IsColumnSelected( long _nColumn ) const override;
    virtual void                    GetAllSelectedRows( css::uno::Sequence< sal_Int32 >& _rRows ) const override;
    virtual void                    GetAllSelectedColumns( css::uno::Sequence< sal_Int32 >& _rColumns ) const override;

    /** @return  <TRUE/>, if the cell is visible. */
    virtual bool                    IsCellVisible( sal_Int32 _nRow, sal_uInt16 _nColumn ) const override;
    virtual OUString                GetAccessibleCellText( long _nRow, sal_uInt16 _nColumnPos ) const override;

    virtual tools::Rectangle               calcHeaderRect( bool _bIsColumnBar, bool _bOnScreen = true ) override;
    virtual tools::Rectangle               calcTableRect( bool _bOnScreen = true ) override;
    virtual tools::Rectangle               GetFieldRectPixelAbs( sal_Int32 _nRow, sal_uInt16 _nColumn, bool _bIsHeader, bool _bOnScreen = true ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumn ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleRowHeader( sal_Int32 _nRow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleColumnHeader( sal_uInt16 _nColumnPos ) override;

    virtual sal_Int32               GetAccessibleControlCount() const override;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleControl( sal_Int32 _nIndex ) override;
    virtual bool                    ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint ) override;

    virtual bool                    ConvertPointToCellAddress( sal_Int32& _rnRow, sal_uInt16& _rnColPos, const Point& _rPoint ) override;
    virtual bool                    ConvertPointToRowHeader( sal_Int32& _rnRow, const Point& _rPoint ) override;
    virtual bool                    ConvertPointToColumnHeader( sal_uInt16& _rnColPos, const Point& _rPoint ) override;

    virtual OUString                GetAccessibleObjectName( ::vcl::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const override;
    virtual OUString                GetAccessibleObjectDescription( ::vcl::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const override;
    virtual vcl::Window*                 GetWindowInstance() override;

    virtual void                    FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& _rStateSet, ::vcl::AccessibleBrowseBoxObjType _eType ) const override;
    virtual void                    FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumn ) const override;
    virtual void                    GrabTableFocus() override;

    // OutputDevice
    virtual bool                    GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex, int nLen, MetricVector& rVector ) override;

    // Window
    virtual tools::Rectangle        GetWindowExtentsRelative( vcl::Window *pRelativeWindow ) const override;
    virtual void                    GrabFocus() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > GetAccessible() override;
    /** Creates and returns the accessible object of the whole BrowseBox. */
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;
    virtual vcl::Window*            GetAccessibleParentWindow() const override;

    virtual tools::Rectangle        GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex) override;
    virtual sal_Int32               GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint) override;
};

#endif // INCLUDED_VCL_SVTABBX_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
