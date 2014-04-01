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
#ifndef INCLUDED_SVTOOLS_SVTABBX_HXX
#define INCLUDED_SVTOOLS_SVTABBX_HXX

#include <svtools/svtdllapi.h>
#include <svtools/treelistbox.hxx>
#include <svtools/accessibletableprovider.hxx>

#include <vector>

enum SvTabJustify
{
    AdjustRight = SV_LBOXTAB_ADJUST_RIGHT,
    AdjustLeft = SV_LBOXTAB_ADJUST_LEFT,
    AdjustCenter = SV_LBOXTAB_ADJUST_CENTER,
    AdjustNumeric = SV_LBOXTAB_ADJUST_NUMERIC
};

struct TabListBoxEventData
{
    SvTreeListEntry*    m_pEntry;
    sal_uInt16          m_nColumn;
    OUString            m_sOldText;

    TabListBoxEventData( SvTreeListEntry* pEntry, sal_uInt16 nColumn, const OUString& rOldText ) :
        m_pEntry( pEntry ), m_nColumn( nColumn ), m_sOldText( rOldText ) {}
};

class SVT_DLLPUBLIC SvTabListBox : public SvTreeListBox
{
private:
    SvLBoxTab*                  pTabList;
    sal_uInt16                  nTabCount;
    OUString                    aCurEntry;

protected:
    SvTreeListEntry*                pViewParent;

    static OUString             GetToken( const OUString &sStr, sal_Int32 &nIndex );

    virtual void                SetTabs() SAL_OVERRIDE;
    virtual void                InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind) SAL_OVERRIDE;

    OUString                    GetTabEntryText( sal_uLong nPos, sal_uInt16 nCol ) const;
    SvTreeListEntry*            GetEntryOnPos( sal_uLong _nEntryPos ) const;
    SvTreeListEntry*            GetChildOnPos( SvTreeListEntry* _pParent, sal_uLong _nEntryPos, sal_uLong& _rPos ) const;

public:
    SvTabListBox( Window* pParent, WinBits = WB_BORDER );
    SvTabListBox( Window* pParent, const ResId& );
    virtual ~SvTabListBox();
    void            SetTabs( long* pTabs, MapUnit = MAP_APPFONT );
    sal_uInt16          TabCount() const { return (sal_uInt16)nTabCount; }
    using SvTreeListBox::GetTab;
    long            GetTab( sal_uInt16 nTab ) const;
    void            SetTab( sal_uInt16 nTab, long nValue, MapUnit = MAP_APPFONT );
    long            GetLogicTab( sal_uInt16 nTab );

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText, SvTreeListEntry* pParent = 0,
                                         bool bChildrenOnDemand = false,
                                         sal_uLong nPos=TREELIST_APPEND, void* pUserData = 0,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox ) SAL_OVERRIDE;

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText,
                                         const Image& rExpandedEntryBmp,
                                         const Image& rCollapsedEntryBmp,
                                         SvTreeListEntry* pParent = 0,
                                         bool bChildrenOnDemand = false,
                                         sal_uLong nPos = TREELIST_APPEND, void* pUserData = 0,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox ) SAL_OVERRIDE;

    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, sal_uLong nPos = TREELIST_APPEND,
                                 sal_uInt16 nCol = 0xffff, void* pUserData = NULL );
    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, SvTreeListEntry* pParent,
                                 sal_uLong nPos, sal_uInt16 nCol, void* pUserData = NULL );
    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, const Image& rExpandedEntryBmp,
                                 const Image& rCollapsedEntryBmp, SvTreeListEntry* pParent = NULL,
                                 sal_uLong nPos = TREELIST_APPEND, sal_uInt16 nCol = 0xffff, void* pUserData = NULL );

    virtual OUString GetEntryText( SvTreeListEntry* pEntry ) const SAL_OVERRIDE;
    OUString         GetEntryText( SvTreeListEntry*, sal_uInt16 nCol ) const;
    OUString         GetEntryText( sal_uLong nPos, sal_uInt16 nCol = 0xffff ) const;
    using SvTreeListBox::SetEntryText;
    void             SetEntryText(const OUString&, sal_uLong, sal_uInt16 nCol=0xffff);
    void             SetEntryText(const OUString&, SvTreeListEntry*, sal_uInt16 nCol=0xffff);
    OUString         GetCellText( sal_uLong nPos, sal_uInt16 nCol ) const;
    sal_uLong        GetEntryPos( const OUString&, sal_uInt16 nCol = 0xffff );
    sal_uLong        GetEntryPos( const SvTreeListEntry* pEntry ) const;

    virtual void     Resize() SAL_OVERRIDE;
    void             SetTabJustify( sal_uInt16 nTab, SvTabJustify );
};

inline long SvTabListBox::GetTab( sal_uInt16 nTab ) const
{
    DBG_ASSERT( nTab < nTabCount, "GetTabPos:Invalid Tab" );
    return pTabList[nTab].GetPos();
}

// class SvHeaderTabListBox ---------------------------------------------------

class HeaderBar;
namespace svt {
    class AccessibleTabListBox;
    class IAccessibleTabListBox;
    struct SvHeaderTabListBoxImpl;
}

class SVT_DLLPUBLIC SvHeaderTabListBox : public SvTabListBox, public svt::IAccessibleTableProvider
{
private:
    typedef ::std::vector< css::uno::Reference< css::accessibility::XAccessible > > AccessibleChildren;

    bool                            m_bFirstPaint;
    ::svt::SvHeaderTabListBoxImpl*  m_pImpl;
    ::svt::IAccessibleTabListBox*   m_pAccessible;
    AccessibleChildren              m_aAccessibleChildren;

    DECL_DLLPRIVATE_LINK( ScrollHdl_Impl, void* );
    DECL_DLLPRIVATE_LINK( CreateAccessibleHdl_Impl, void* );

    void            RecalculateAccessibleChildren();

public:
    SvHeaderTabListBox( Window* pParent, WinBits nBits );
    virtual ~SvHeaderTabListBox();

    virtual void    Paint( const Rectangle& ) SAL_OVERRIDE;

    void            InitHeaderBar( HeaderBar* pHeaderBar );
    bool            IsItemChecked( SvTreeListEntry* pEntry, sal_uInt16 nCol ) const;

    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, sal_uLong nPos = TREELIST_APPEND,
                                 sal_uInt16 nCol = 0xffff, void* pUserData = NULL ) SAL_OVERRIDE;
    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, SvTreeListEntry* pParent,
                                 sal_uLong nPos, sal_uInt16 nCol, void* pUserData = NULL ) SAL_OVERRIDE;
    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, const Image& rExpandedEntryBmp,
                                 const Image& rCollapsedEntryBmp, SvTreeListEntry* pParent = NULL,
                                 sal_uLong nPos = TREELIST_APPEND, sal_uInt16 nCol = 0xffff, void* pUserData = NULL ) SAL_OVERRIDE;
    virtual sal_uLong Insert( SvTreeListEntry* pEnt,SvTreeListEntry* pPar,sal_uLong nPos=TREELIST_APPEND) SAL_OVERRIDE;
    virtual sal_uLong Insert( SvTreeListEntry* pEntry, sal_uLong nRootPos = TREELIST_APPEND ) SAL_OVERRIDE;
    void            RemoveEntry( SvTreeListEntry* _pEntry );
    void            Clear();

    // Accessible -------------------------------------------------------------

    inline void     DisableTransientChildren()          { SetChildrenNotTransient(); }
    inline bool     IsTransientChildrenDisabled() const { return !AreChildrenTransient(); }

    bool            IsCellCheckBox( long _nRow, sal_uInt16 _nColumn, TriState& _rState );

    /** @return  The count of the rows. */
    virtual long                    GetRowCount() const SAL_OVERRIDE;
    /** @return  The count of the columns. */
    virtual sal_uInt16              GetColumnCount() const SAL_OVERRIDE;

    /** @return  The position of the current row. */
    virtual sal_Int32               GetCurrRow() const SAL_OVERRIDE;
    /** @return  The position of the current column. */
    virtual sal_uInt16              GetCurrColumn() const SAL_OVERRIDE;

    /** @return  The description of a row.
        @param _nRow The row which description is in demand. */
    virtual OUString         GetRowDescription( sal_Int32 _nRow ) const SAL_OVERRIDE;
    /** @return  The description of a column.
        @param _nColumn The column which description is in demand. */
    virtual OUString         GetColumnDescription( sal_uInt16 _nColumn ) const SAL_OVERRIDE;

    /** @return  <TRUE/>, if the object has a row header. */
    virtual bool                    HasRowHeader() const SAL_OVERRIDE; //GetColumnId
    /** @return  <TRUE/>, if the object can focus a cell. */
    virtual bool                    IsCellFocusable() const SAL_OVERRIDE;
    virtual bool                    GoToCell( sal_Int32 _nRow, sal_uInt16 _nColumn ) SAL_OVERRIDE;

    virtual void                    SetNoSelection() SAL_OVERRIDE;
    using SvListView::SelectAll;
    virtual void                    SelectAll() SAL_OVERRIDE;
    virtual void                    SelectAll( bool bSelect, bool bPaint = true ) SAL_OVERRIDE;
    virtual void                    SelectRow( long _nRow, bool _bSelect = true, bool bExpand = true ) SAL_OVERRIDE;
    virtual void                    SelectColumn( sal_uInt16 _nColumn, bool _bSelect = true ) SAL_OVERRIDE;
    virtual sal_Int32               GetSelectedRowCount() const SAL_OVERRIDE;
    virtual sal_Int32               GetSelectedColumnCount() const SAL_OVERRIDE;
    /** @return  <TRUE/>, if the row is selected. */
    virtual bool                    IsRowSelected( long _nRow ) const SAL_OVERRIDE;
    virtual bool                    IsColumnSelected( long _nColumn ) const SAL_OVERRIDE;
    virtual void                    GetAllSelectedRows( css::uno::Sequence< sal_Int32 >& _rRows ) const SAL_OVERRIDE;
    virtual void                    GetAllSelectedColumns( css::uno::Sequence< sal_Int32 >& _rColumns ) const SAL_OVERRIDE;

    /** @return  <TRUE/>, if the cell is visible. */
    virtual bool                    IsCellVisible( sal_Int32 _nRow, sal_uInt16 _nColumn ) const SAL_OVERRIDE;
    virtual OUString                GetAccessibleCellText( long _nRow, sal_uInt16 _nColumnPos ) const SAL_OVERRIDE;

    virtual Rectangle               calcHeaderRect( bool _bIsColumnBar, bool _bOnScreen = true ) SAL_OVERRIDE;
    virtual Rectangle               calcTableRect( bool _bOnScreen = true ) SAL_OVERRIDE;
    virtual Rectangle               GetFieldRectPixelAbs( sal_Int32 _nRow, sal_uInt16 _nColumn, bool _bIsHeader, bool _bOnScreen = true ) SAL_OVERRIDE;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumn ) SAL_OVERRIDE;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleRowHeader( sal_Int32 _nRow ) SAL_OVERRIDE;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleColumnHeader( sal_uInt16 _nColumnPos ) SAL_OVERRIDE;

    virtual sal_Int32               GetAccessibleControlCount() const SAL_OVERRIDE;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleControl( sal_Int32 _nIndex ) SAL_OVERRIDE;
    virtual bool                    ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint ) SAL_OVERRIDE;

    virtual bool                    ConvertPointToCellAddress( sal_Int32& _rnRow, sal_uInt16& _rnColPos, const Point& _rPoint ) SAL_OVERRIDE;
    virtual bool                    ConvertPointToRowHeader( sal_Int32& _rnRow, const Point& _rPoint ) SAL_OVERRIDE;
    virtual bool                    ConvertPointToColumnHeader( sal_uInt16& _rnColPos, const Point& _rPoint ) SAL_OVERRIDE;

    virtual OUString                GetAccessibleObjectName( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const SAL_OVERRIDE;
    virtual OUString                GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const SAL_OVERRIDE;
    virtual Window*                 GetWindowInstance() SAL_OVERRIDE;

    using SvTreeListBox::FillAccessibleStateSet;
    virtual void                    FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& _rStateSet, ::svt::AccessibleBrowseBoxObjType _eType ) const SAL_OVERRIDE;
    virtual void                    FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumn ) const SAL_OVERRIDE;
    virtual void                    GrabTableFocus() SAL_OVERRIDE;

    // OutputDevice
    virtual bool                    GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex, int nLen, int nBase, MetricVector& rVector ) SAL_OVERRIDE;

    // Window
    virtual Rectangle               GetWindowExtentsRelative( Window *pRelativeWindow ) const SAL_OVERRIDE;
    virtual void                    GrabFocus() SAL_OVERRIDE;
    virtual css::uno::Reference< css::accessibility::XAccessible > GetAccessible( bool bCreate = true ) SAL_OVERRIDE;
    virtual Window*                 GetAccessibleParentWindow() const SAL_OVERRIDE;

    /** Creates and returns the accessible object of the whole BrowseBox. */
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;

    virtual Rectangle               GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex) SAL_OVERRIDE;
    virtual sal_Int32               GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint) SAL_OVERRIDE;
};

#endif // INCLUDED_SVTOOLS_SVTABBX_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
