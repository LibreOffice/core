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
#include <svtools/svtreebx.hxx>
#include <svtools/accessibletableprovider.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

enum SvTabJustify
{
    AdjustRight = SV_LBOXTAB_ADJUST_RIGHT,
    AdjustLeft = SV_LBOXTAB_ADJUST_LEFT,
    AdjustCenter = SV_LBOXTAB_ADJUST_CENTER,
    AdjustNumeric = SV_LBOXTAB_ADJUST_NUMERIC
};

struct TabListBoxEventData
{
    SvLBoxEntry*    m_pEntry;
    USHORT          m_nColumn;
    String          m_sOldText;

    TabListBoxEventData( SvLBoxEntry* pEntry, USHORT nColumn, const String& rOldText ) :
        m_pEntry( pEntry ), m_nColumn( nColumn ), m_sOldText( rOldText ) {}
};

class SVT_DLLPUBLIC SvTabListBox : public SvTreeListBox
{
private:
    SvLBoxTab*                  pTabList;
    USHORT                      nTabCount;
    XubString                   aCurEntry;
    ULONG                       nDummy1;
    ULONG                       nDummy2;

protected:
    SvLBoxEntry*                pViewParent;

    static const xub_Unicode*   GetToken( const xub_Unicode* pPtr, USHORT& rLen );

    virtual void                SetTabs();
    virtual void                InitEntry( SvLBoxEntry*, const XubString&, const Image&, const Image&, SvLBoxButtonKind );

    String                      GetTabEntryText( ULONG nPos, USHORT nCol ) const;
    SvLBoxEntry*                GetEntryOnPos( ULONG _nEntryPos ) const;
    SvLBoxEntry*                GetChildOnPos( SvLBoxEntry* _pParent, ULONG _nEntryPos, ULONG& _rPos ) const;

public:
    SvTabListBox( Window* pParent, WinBits = WB_BORDER );
    SvTabListBox( Window* pParent, const ResId& );
    ~SvTabListBox();
    void            SetTabs( long* pTabs, MapUnit = MAP_APPFONT );
    USHORT          TabCount() const { return (USHORT)nTabCount; }
    using SvTreeListBox::GetTab;
    long            GetTab( USHORT nTab ) const;
    void            SetTab( USHORT nTab, long nValue, MapUnit = MAP_APPFONT );
    long            GetLogicTab( USHORT nTab );

    virtual SvLBoxEntry*    InsertEntry( const XubString& rText, SvLBoxEntry* pParent = 0,
                                         BOOL bChildsOnDemand = FALSE,
                                         ULONG nPos=LIST_APPEND, void* pUserData = 0,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox );

    virtual SvLBoxEntry*    InsertEntry( const XubString& rText,
                                         const Image& rExpandedEntryBmp,
                                         const Image& rCollapsedEntryBmp,
                                         SvLBoxEntry* pParent = 0,
                                         BOOL bChildsOnDemand = FALSE,
                                         ULONG nPos = LIST_APPEND, void* pUserData = 0,
                                         SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox );

    virtual SvLBoxEntry* InsertEntryToColumn( const XubString&, ULONG nPos = LIST_APPEND,
                                 USHORT nCol = 0xffff, void* pUserData = NULL );
    virtual SvLBoxEntry* InsertEntryToColumn( const XubString&, SvLBoxEntry* pParent,
                                 ULONG nPos, USHORT nCol, void* pUserData = NULL );
    virtual SvLBoxEntry* InsertEntryToColumn( const XubString&, const Image& rExpandedEntryBmp,
                                 const Image& rCollapsedEntryBmp, SvLBoxEntry* pParent = NULL,
                                 ULONG nPos = LIST_APPEND, USHORT nCol = 0xffff, void* pUserData = NULL );

    virtual String  GetEntryText( SvLBoxEntry* pEntry ) const;
    String          GetEntryText( SvLBoxEntry*, USHORT nCol ) const;
    String          GetEntryText( ULONG nPos, USHORT nCol = 0xffff ) const;
    using SvTreeListBox::SetEntryText;
    void            SetEntryText( const XubString&, ULONG, USHORT nCol=0xffff );
    void            SetEntryText(const XubString&,SvLBoxEntry*,USHORT nCol=0xffff);
    String          GetCellText( ULONG nPos, USHORT nCol ) const;
    ULONG           GetEntryPos( const XubString&, USHORT nCol = 0xffff );
    ULONG           GetEntryPos( const SvLBoxEntry* pEntry ) const;

    virtual void    Resize();
    void            SetTabJustify( USHORT nTab, SvTabJustify );
    SvTabJustify    GetTabJustify( USHORT nTab ) const;
};

inline long SvTabListBox::GetTab( USHORT nTab ) const
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

    DECL_DLLPRIVATE_LINK( ScrollHdl_Impl, SvTabListBox* );
    DECL_DLLPRIVATE_LINK( CreateAccessibleHdl_Impl, HeaderBar* );

    void            RecalculateAccessibleChildren();

public:
    SvHeaderTabListBox( Window* pParent, WinBits nBits );
    SvHeaderTabListBox( Window* pParent, const ResId& );
    ~SvHeaderTabListBox();

    virtual void    Paint( const Rectangle& );

    void            InitHeaderBar( HeaderBar* pHeaderBar );
    sal_Bool        IsItemChecked( SvLBoxEntry* pEntry, USHORT nCol ) const;

    virtual SvLBoxEntry* InsertEntryToColumn( const XubString&, ULONG nPos = LIST_APPEND,
                                 USHORT nCol = 0xffff, void* pUserData = NULL );
    virtual SvLBoxEntry* InsertEntryToColumn( const XubString&, SvLBoxEntry* pParent,
                                 ULONG nPos, USHORT nCol, void* pUserData = NULL );
    virtual SvLBoxEntry* InsertEntryToColumn( const XubString&, const Image& rExpandedEntryBmp,
                                 const Image& rCollapsedEntryBmp, SvLBoxEntry* pParent = NULL,
                                 ULONG nPos = LIST_APPEND, USHORT nCol = 0xffff, void* pUserData = NULL );
    virtual ULONG Insert( SvLBoxEntry* pEnt,SvLBoxEntry* pPar,ULONG nPos=LIST_APPEND);
    virtual ULONG Insert( SvLBoxEntry* pEntry, ULONG nRootPos = LIST_APPEND );
    void            RemoveEntry( SvLBoxEntry* _pEntry );
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
    virtual BOOL                    GoToCell( sal_Int32 _nRow, sal_uInt16 _nColumn );

    virtual void                    SetNoSelection();
    using SvListView::SelectAll;
    virtual void                    SelectAll();
    virtual void                    SelectAll( BOOL bSelect, BOOL bPaint = TRUE );
    virtual void                    SelectRow( long _nRow, BOOL _bSelect = TRUE, BOOL bExpand = TRUE );
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
    virtual String                  GetAccessibleCellText( long _nRow, USHORT _nColumnPos ) const;

    virtual Rectangle               calcHeaderRect( sal_Bool _bIsColumnBar, BOOL _bOnScreen = TRUE );
    virtual Rectangle               calcTableRect( BOOL _bOnScreen = TRUE );
    virtual Rectangle               GetFieldRectPixelAbs( sal_Int32 _nRow, sal_uInt16 _nColumn, BOOL _bIsHeader, BOOL _bOnScreen = TRUE );

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
    virtual BOOL                    GetGlyphBoundRects( const Point& rOrigin, const String& rStr, int nIndex, int nLen, int nBase, MetricVector& rVector );

    // Window
    virtual Rectangle               GetWindowExtentsRelative( Window *pRelativeWindow ) const;
    virtual void                    GrabFocus();
    virtual XACC                    GetAccessible( BOOL bCreate = TRUE );
    virtual Window*                 GetAccessibleParentWindow() const;

    /** Creates and returns the accessible object of the whole BrowseBox. */
    virtual XACC                    CreateAccessible();

    virtual Rectangle               GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex);
    virtual sal_Int32               GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint);
};

#endif // #ifndef _SVTABBX_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
