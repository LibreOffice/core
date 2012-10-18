/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVTABBX_HXX
#define _SVTABBX_HXX

#include "svtools/svtdllapi.h"
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
    String          m_sOldText;

    TabListBoxEventData( SvTreeListEntry* pEntry, sal_uInt16 nColumn, const String& rOldText ) :
        m_pEntry( pEntry ), m_nColumn( nColumn ), m_sOldText( rOldText ) {}
};

class SVT_DLLPUBLIC SvTabListBox : public SvTreeListBox
{
private:
    SvLBoxTab*                  pTabList;
    sal_uInt16                      nTabCount;
    XubString                   aCurEntry;

protected:
    SvTreeListEntry*                pViewParent;

    static const xub_Unicode*   GetToken( const xub_Unicode* pPtr, sal_uInt16& rLen );

    virtual void                SetTabs();
    virtual void                InitEntry( SvTreeListEntry*, const XubString&, const Image&, const Image&, SvLBoxButtonKind );

    String                      GetTabEntryText( sal_uLong nPos, sal_uInt16 nCol ) const;
    SvTreeListEntry*                GetEntryOnPos( sal_uLong _nEntryPos ) const;
    SvTreeListEntry*                GetChildOnPos( SvTreeListEntry* _pParent, sal_uLong _nEntryPos, sal_uLong& _rPos ) const;

public:
    SvTabListBox( Window* pParent, WinBits = WB_BORDER );
    SvTabListBox( Window* pParent, const ResId& );
    ~SvTabListBox();
    void            SetTabs( long* pTabs, MapUnit = MAP_APPFONT );
    sal_uInt16          TabCount() const { return (sal_uInt16)nTabCount; }
    using SvTreeListBox::GetTab;
    long            GetTab( sal_uInt16 nTab ) const;
    void            SetTab( sal_uInt16 nTab, long nValue, MapUnit = MAP_APPFONT );
    long            GetLogicTab( sal_uInt16 nTab );

    virtual SvTreeListEntry*    InsertEntry( const XubString& rText, SvTreeListEntry* pParent = 0,
                                         sal_Bool bChildrenOnDemand = sal_False,
                                         sal_uLong nPos=LIST_APPEND, void* pUserData = 0,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox );

    virtual SvTreeListEntry*    InsertEntry( const XubString& rText,
                                         const Image& rExpandedEntryBmp,
                                         const Image& rCollapsedEntryBmp,
                                         SvTreeListEntry* pParent = 0,
                                         sal_Bool bChildrenOnDemand = sal_False,
                                         sal_uLong nPos = LIST_APPEND, void* pUserData = 0,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox );

    virtual SvTreeListEntry* InsertEntryToColumn( const XubString&, sal_uLong nPos = LIST_APPEND,
                                 sal_uInt16 nCol = 0xffff, void* pUserData = NULL );
    virtual SvTreeListEntry* InsertEntryToColumn( const XubString&, SvTreeListEntry* pParent,
                                 sal_uLong nPos, sal_uInt16 nCol, void* pUserData = NULL );
    virtual SvTreeListEntry* InsertEntryToColumn( const XubString&, const Image& rExpandedEntryBmp,
                                 const Image& rCollapsedEntryBmp, SvTreeListEntry* pParent = NULL,
                                 sal_uLong nPos = LIST_APPEND, sal_uInt16 nCol = 0xffff, void* pUserData = NULL );

    virtual String  GetEntryText( SvTreeListEntry* pEntry ) const;
    String          GetEntryText( SvTreeListEntry*, sal_uInt16 nCol ) const;
    String          GetEntryText( sal_uLong nPos, sal_uInt16 nCol = 0xffff ) const;
    using SvTreeListBox::SetEntryText;
    void            SetEntryText( const XubString&, sal_uLong, sal_uInt16 nCol=0xffff );
    void            SetEntryText(const XubString&,SvTreeListEntry*,sal_uInt16 nCol=0xffff);
    String          GetCellText( sal_uLong nPos, sal_uInt16 nCol ) const;
    sal_uLong           GetEntryPos( const XubString&, sal_uInt16 nCol = 0xffff );
    sal_uLong           GetEntryPos( const SvTreeListEntry* pEntry ) const;

    virtual void    Resize();
    void            SetTabJustify( sal_uInt16 nTab, SvTabJustify );
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
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > > AccessibleChildren;

    sal_Bool                        m_bFirstPaint;
    ::svt::SvHeaderTabListBoxImpl*  m_pImpl;
    ::svt::IAccessibleTabListBox*   m_pAccessible;
    AccessibleChildren              m_aAccessibleChildren;

    DECL_DLLPRIVATE_LINK( ScrollHdl_Impl, void* );
    DECL_DLLPRIVATE_LINK( CreateAccessibleHdl_Impl, void* );

    void            RecalculateAccessibleChildren();

public:
    SvHeaderTabListBox( Window* pParent, WinBits nBits );
    SvHeaderTabListBox( Window* pParent, const ResId& );
    ~SvHeaderTabListBox();

    virtual void    Paint( const Rectangle& );

    void            InitHeaderBar( HeaderBar* pHeaderBar );
    sal_Bool        IsItemChecked( SvTreeListEntry* pEntry, sal_uInt16 nCol ) const;

    virtual SvTreeListEntry* InsertEntryToColumn( const XubString&, sal_uLong nPos = LIST_APPEND,
                                 sal_uInt16 nCol = 0xffff, void* pUserData = NULL );
    virtual SvTreeListEntry* InsertEntryToColumn( const XubString&, SvTreeListEntry* pParent,
                                 sal_uLong nPos, sal_uInt16 nCol, void* pUserData = NULL );
    virtual SvTreeListEntry* InsertEntryToColumn( const XubString&, const Image& rExpandedEntryBmp,
                                 const Image& rCollapsedEntryBmp, SvTreeListEntry* pParent = NULL,
                                 sal_uLong nPos = LIST_APPEND, sal_uInt16 nCol = 0xffff, void* pUserData = NULL );
    virtual sal_uLong Insert( SvTreeListEntry* pEnt,SvTreeListEntry* pPar,sal_uLong nPos=LIST_APPEND);
    virtual sal_uLong Insert( SvTreeListEntry* pEntry, sal_uLong nRootPos = LIST_APPEND );
    void            RemoveEntry( SvTreeListEntry* _pEntry );
    void            Clear();

    // Accessible -------------------------------------------------------------

    inline void     DisableTransientChildren()          { SetChildrenNotTransient(); }
    inline sal_Bool IsTransientChildrenDisabled() const { return !AreChildrenTransient(); }

    sal_Bool        IsCellCheckBox( long _nRow, sal_uInt16 _nColumn, TriState& _rState );

    /** @return  The count of the rows. */
    virtual long                    GetRowCount() const;
    /** @return  The count of the columns. */
    virtual sal_uInt16              GetColumnCount() const;

    /** @return  The position of the current row. */
    virtual sal_Int32               GetCurrRow() const;
    /** @return  The position of the current column. */
    virtual sal_uInt16              GetCurrColumn() const;

    /** @return  The description of a row.
        @param _nRow The row which description is in demand. */
    virtual ::rtl::OUString         GetRowDescription( sal_Int32 _nRow ) const;
    /** @return  The description of a column.
        @param _nColumn The column which description is in demand. */
    virtual ::rtl::OUString         GetColumnDescription( sal_uInt16 _nColumn ) const;

    /** @return  <TRUE/>, if the object has a row header. */
    virtual sal_Bool                HasRowHeader() const; //GetColumnId
    /** @return  <TRUE/>, if the object can focus a cell. */
    virtual sal_Bool                IsCellFocusable() const;
    virtual sal_Bool                    GoToCell( sal_Int32 _nRow, sal_uInt16 _nColumn );

    virtual void                    SetNoSelection();
    using SvListView::SelectAll;
    virtual void                    SelectAll();
    virtual void                    SelectAll( sal_Bool bSelect, sal_Bool bPaint = sal_True );
    virtual void                    SelectRow( long _nRow, sal_Bool _bSelect = sal_True, sal_Bool bExpand = sal_True );
    virtual void                    SelectColumn( sal_uInt16 _nColumn, sal_Bool _bSelect = sal_True );
    virtual sal_Int32               GetSelectedRowCount() const;
    virtual sal_Int32               GetSelectedColumnCount() const;
    /** @return  <TRUE/>, if the row is selected. */
    virtual bool                    IsRowSelected( long _nRow ) const;
    virtual sal_Bool                IsColumnSelected( long _nColumn ) const;
    virtual void                    GetAllSelectedRows( ::com::sun::star::uno::Sequence< sal_Int32 >& _rRows ) const;
    virtual void                    GetAllSelectedColumns( ::com::sun::star::uno::Sequence< sal_Int32 >& _rColumns ) const;

    /** @return  <TRUE/>, if the cell is visible. */
    virtual sal_Bool                IsCellVisible( sal_Int32 _nRow, sal_uInt16 _nColumn ) const;
    virtual String                  GetAccessibleCellText( long _nRow, sal_uInt16 _nColumnPos ) const;

    virtual Rectangle               calcHeaderRect( sal_Bool _bIsColumnBar, sal_Bool _bOnScreen = sal_True );
    virtual Rectangle               calcTableRect( sal_Bool _bOnScreen = sal_True );
    virtual Rectangle               GetFieldRectPixelAbs( sal_Int32 _nRow, sal_uInt16 _nColumn, sal_Bool _bIsHeader, sal_Bool _bOnScreen = sal_True );

    virtual XACC                    CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumn );
    virtual XACC                    CreateAccessibleRowHeader( sal_Int32 _nRow );
    virtual XACC                    CreateAccessibleColumnHeader( sal_uInt16 _nColumnPos );

    virtual sal_Int32               GetAccessibleControlCount() const;
    virtual XACC                    CreateAccessibleControl( sal_Int32 _nIndex );
    virtual sal_Bool                ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint );

    virtual sal_Bool                ConvertPointToCellAddress( sal_Int32& _rnRow, sal_uInt16& _rnColPos, const Point& _rPoint );
    virtual sal_Bool                ConvertPointToRowHeader( sal_Int32& _rnRow, const Point& _rPoint );
    virtual sal_Bool                ConvertPointToColumnHeader( sal_uInt16& _rnColPos, const Point& _rPoint );

    virtual ::rtl::OUString         GetAccessibleObjectName( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const;
    virtual ::rtl::OUString         GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const;
    virtual Window*                 GetWindowInstance();

    using SvTreeListBox::FillAccessibleStateSet;
    virtual void                    FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& _rStateSet, ::svt::AccessibleBrowseBoxObjType _eType ) const;
    virtual void                    FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumn ) const;
    virtual void                    GrabTableFocus();

    // OutputDevice
    virtual sal_Bool                    GetGlyphBoundRects( const Point& rOrigin, const String& rStr, int nIndex, int nLen, int nBase, MetricVector& rVector );

    // Window
    virtual Rectangle               GetWindowExtentsRelative( Window *pRelativeWindow ) const;
    virtual void                    GrabFocus();
    virtual XACC                    GetAccessible( sal_Bool bCreate = sal_True );
    virtual Window*                 GetAccessibleParentWindow() const;

    /** Creates and returns the accessible object of the whole BrowseBox. */
    virtual XACC                    CreateAccessible();

    virtual Rectangle               GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex);
    virtual sal_Int32               GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint);
};

#endif // #ifndef _SVTABBX_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
