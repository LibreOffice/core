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

#ifndef _SVTOOLS_ACCESSIBLETABLE_HXX
#define _SVTOOLS_ACCESSIBLETABLE_HXX

#include <vcl/window.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

// ============================================================================

namespace svt{ namespace table
{

typedef sal_Int32   RowPos;

// ============================================================================

enum AccessibleTableType
{
    /** Child index of the column header bar (first row). */
    TCINDEX_COLUMNHEADERBAR = 0,
    /** Child index of the row header bar ("handle column"). */
    TCINDEX_ROWHEADERBAR    = 1,
    /** Child index of the data table. */
    TCINDEX_TABLE           = 2
};

enum AccessibleTableControlObjType
{
    TCTYPE_GRIDCONTROL,           /// The GridControl itself.
    TCTYPE_TABLE,               /// The data table.
    TCTYPE_ROWHEADERBAR,        /// The row header bar.
    TCTYPE_COLUMNHEADERBAR,     /// The horizontal column header bar.
    TCTYPE_TABLECELL,           /// A cell of the data table.
    TCTYPE_ROWHEADERCELL,       /// A cell of the row header bar.
    TCTYPE_COLUMNHEADERCELL,    /// A cell of the column header bar.
};

// ============================================================================

#define XACC ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >

/** This abstract class provides methods to implement an accessible table object.
*/
class IAccessibleTable
{
public:
    /** @return  The position of the current row. */
    virtual sal_Int32               GetCurrentRow() const = 0;
    /** @return  The position of the current column. */
    virtual sal_Int32               GetCurrentColumn() const = 0;
    /** Creates and returns the accessible object of the whole GridControl. */
    virtual XACC CreateAccessible()= 0;
    virtual XACC CreateAccessibleControl( sal_Int32 _nIndex )= 0;
    virtual ::rtl::OUString GetAccessibleObjectName(AccessibleTableControlObjType eObjType, sal_Int32 _nRow, sal_Int32 _nCol) const= 0;
    virtual sal_Bool    GoToCell( sal_Int32 _nColumnPos, sal_Int32 _nRow )= 0;
    virtual sal_Bool    HasColHeader() = 0;
    virtual sal_Bool    HasRowHeader() = 0;

    /** return the description of the specified object.
        @param  eObjType
            The type to ask for
        @param  _nPosition
            The position of a tablecell (index position), header bar  colum/row cell
        @return
            The description of the specified object.
    */
    virtual ::rtl::OUString GetAccessibleObjectDescription(AccessibleTableControlObjType eObjType, sal_Int32 _nPosition = -1) const= 0;

    /** Fills the StateSet with all states (except DEFUNC and SHOWING, done by
            the accessible object), depending on the specified object type. */
    virtual void FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& rStateSet,
            AccessibleTableControlObjType eObjType ) const= 0;

    // Window
    virtual Rectangle GetWindowExtentsRelative( Window *pRelativeWindow ) const = 0;
    virtual void GrabFocus()= 0;
    virtual XACC GetAccessible( sal_Bool bCreate = sal_True )= 0;
    virtual Window* GetAccessibleParentWindow() const= 0;
    virtual Window* GetWindowInstance()= 0;
    virtual sal_Int32 GetAccessibleControlCount() const = 0;
    virtual sal_Bool ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint )= 0;
    virtual long GetRowCount() const= 0;
    virtual long GetColumnCount() const= 0;
    virtual sal_Bool HasRowHeader() const= 0;
    virtual sal_Bool ConvertPointToCellAddress( sal_Int32& _rnRow, sal_Int32& _rnColPos, const Point& _rPoint )= 0;
    virtual Rectangle calcHeaderRect( sal_Bool _bIsColumnBar, sal_Bool _bOnScreen = sal_True ) = 0;
    virtual Rectangle calcHeaderCellRect( sal_Bool _bColHeader, sal_Int32 _nPos ) = 0;
    virtual Rectangle calcTableRect( sal_Bool _bOnScreen = sal_True ) = 0;
    virtual Rectangle calcCellRect( sal_Int32 _nRowPos, sal_Int32 _nColPos ) = 0;
    virtual Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex)= 0;
    virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint)= 0;
    virtual void FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const= 0;
    virtual ::rtl::OUString GetRowDescription( sal_Int32 _nRow ) const = 0;
    virtual ::rtl::OUString GetRowName(sal_Int32 _nIndex) const = 0;
    virtual ::rtl::OUString GetColumnDescription( sal_uInt16 _nColumnPos ) const = 0;
    virtual ::rtl::OUString GetColumnName( sal_Int32 _nIndex ) const = 0;
    virtual ::com::sun::star::uno::Any GetCellContent( sal_Int32 _nRowPos, sal_Int32 _nColPos) const = 0;
    virtual ::rtl::OUString GetAccessibleCellText(sal_Int32 _nRowPos, sal_Int32 _nColPos) const = 0;

    virtual sal_Int32 GetSelectedRowCount() const = 0;
    virtual sal_Int32 GetSelectedRowIndex( sal_Int32 const i_selectionIndex ) const = 0;
    virtual bool IsRowSelected( sal_Int32 const i_rowIndex ) const = 0;
    virtual void SelectRow( sal_Int32 const i_rowIndex, bool const i_select ) = 0;
    virtual void SelectAllRows( bool const i_select ) = 0;

protected:
    ~IAccessibleTable() {}
};

// ----------------------------------------------------------------------------

/** interface for an implementation of a table control's Accesible component
*/
class IAccessibleTableControl
{
public:
    /** returns the XAccessible object itself

        The reference returned here can be used to control the life time of the
        IAccessibleTableImplementation object.

        The returned reference is guaranteed to not be <NULL/>.
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getMyself() = 0;

    /** disposes the accessible implementation, so that it becomes defunc
    */
    virtual void dispose() = 0;

    /** checks whether the accessible implementation, and its context, are still alive
        @return  <TRUE/>, if the object is not disposed or disposing.
    */
    virtual sal_Bool isAlive() const = 0;

    /** returns the accessible object for the row or the column header bar
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getTableHeader( ::svt::table::AccessibleTableControlObjType _eObjType ) = 0;

    /** returns the accessible object for the table representation
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getTable() = 0;

    /** commits the event at all listeners of the cell
         @param nEventId
             the event id
         @param rNewValue
             the new value
          @param rOldValue
             the old value
      */
     virtual void commitCellEvent(
         sal_Int16 nEventId,
         const ::com::sun::star::uno::Any& rNewValue,
         const ::com::sun::star::uno::Any& rOldValue
     ) = 0;
    /** commits the event at all listeners of the table
         @param nEventId
             the event id
         @param rNewValue
             the new value
          @param rOldValue
             the old value
      */
     virtual void commitTableEvent(
         sal_Int16 nEventId,
         const ::com::sun::star::uno::Any& rNewValue,
         const ::com::sun::star::uno::Any& rOldValue
     ) = 0;

    ///** Commits an event to all listeners. */
    virtual void commitEvent(
        sal_Int16 nEventId,
        const ::com::sun::star::uno::Any& rNewValue,
        const ::com::sun::star::uno::Any& rOldValue
    ) = 0;

protected:
    ~IAccessibleTableControl() {}
};

// ----------------------------------------------------------------------------

// ============================================================================
} // namespace table
} // namespace svt

// ============================================================================

#endif // _SVTOOLS_ACCESSIBLETABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
