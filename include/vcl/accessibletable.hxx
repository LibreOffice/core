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

#ifndef INCLUDED_VCL_ACCESSIBLETABLE_HXX
#define INCLUDED_VCL_ACCESSIBLETABLE_HXX

#include <tools/gen.hxx>

#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star { namespace accessibility { class XAccessible; } } } }
namespace utl { class AccessibleStateSetHelper; }
namespace vcl { class Window; }

namespace vcl { namespace table
{

typedef sal_Int32   RowPos;

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


/** This abstract class provides methods to implement an accessible table object.
*/
class IAccessibleTable
{
public:
    /** @return  The position of the current row. */
    virtual sal_Int32               GetCurrentRow() const = 0;
    /** @return  The position of the current column. */
    virtual sal_Int32               GetCurrentColumn() const = 0;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleControl( sal_Int32 _nIndex )= 0;
    virtual OUString GetAccessibleObjectName(AccessibleTableControlObjType eObjType, sal_Int32 _nRow, sal_Int32 _nCol) const= 0;
    virtual void    GoToCell( sal_Int32 _nColumnPos, sal_Int32 _nRow )= 0;
    virtual bool    HasColHeader() = 0;
    virtual bool    HasRowHeader() = 0;

    /** return the description of the specified object.
        @param  eObjType
            The type to ask for
        @return
            The description of the specified object.
    */
    virtual OUString GetAccessibleObjectDescription(AccessibleTableControlObjType eObjType) const= 0;

    /** Fills the StateSet with all states (except DEFUNC and SHOWING, done by
            the accessible object), depending on the specified object type. */
    virtual void FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& rStateSet,
            AccessibleTableControlObjType eObjType ) const= 0;

    // Window
    virtual tools::Rectangle GetWindowExtentsRelative( vcl::Window *pRelativeWindow ) const = 0;
    virtual void GrabFocus()= 0;
    virtual css::uno::Reference< css::accessibility::XAccessible > GetAccessible()= 0;
    virtual vcl::Window* GetAccessibleParentWindow() const= 0;
    virtual vcl::Window* GetWindowInstance()= 0;
    virtual sal_Int32 GetAccessibleControlCount() const = 0;
    virtual bool ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint )= 0;
    virtual long GetRowCount() const= 0;
    virtual long GetColumnCount() const= 0;
    virtual bool ConvertPointToCellAddress( sal_Int32& _rnRow, sal_Int32& _rnColPos, const Point& _rPoint )= 0;
    virtual tools::Rectangle calcHeaderRect( bool _bIsColumnBar ) = 0;
    virtual tools::Rectangle calcHeaderCellRect( bool _bColHeader, sal_Int32 _nPos ) = 0;
    virtual tools::Rectangle calcTableRect() = 0;
    virtual tools::Rectangle calcCellRect( sal_Int32 _nRowPos, sal_Int32 _nColPos ) = 0;
    virtual tools::Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex)= 0;
    virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint)= 0;
    virtual void FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const= 0;
    virtual OUString GetRowDescription( sal_Int32 _nRow ) const = 0;
    virtual OUString GetRowName(sal_Int32 _nIndex) const = 0;
    virtual OUString GetColumnDescription( sal_uInt16 _nColumnPos ) const = 0;
    virtual OUString GetColumnName( sal_Int32 _nIndex ) const = 0;
    virtual OUString GetAccessibleCellText(sal_Int32 _nRowPos, sal_Int32 _nColPos) const = 0;

    virtual sal_Int32 GetSelectedRowCount() const = 0;
    virtual sal_Int32 GetSelectedRowIndex( sal_Int32 const i_selectionIndex ) const = 0;
    virtual bool IsRowSelected( sal_Int32 const i_rowIndex ) const = 0;
    virtual void SelectRow( sal_Int32 const i_rowIndex, bool const i_select ) = 0;
    virtual void SelectAllRows( bool const i_select ) = 0;

protected:
    ~IAccessibleTable() {}
};


/** interface for an implementation of a table control's Accessible component
*/
class IAccessibleTableControl
{
public:
    /** returns the XAccessible object itself

        The reference returned here can be used to control the life time of the
        IAccessibleTableImplementation object.

        The returned reference is guaranteed to not be <NULL/>.
    */
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getMyself() = 0;

    /** disposes the accessible implementation, so that it becomes defunc
    */
    virtual void DisposeAccessImpl() = 0;

    /** checks whether the accessible implementation, and its context, are still alive
        @return  <TRUE/>, if the object is not disposed or disposing.
    */
    virtual bool isAlive() const = 0;

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
         const css::uno::Any& rNewValue,
         const css::uno::Any& rOldValue
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
         const css::uno::Any& rNewValue,
         const css::uno::Any& rOldValue
     ) = 0;

    ///** Commits an event to all listeners. */
    virtual void commitEvent(
        sal_Int16 nEventId,
        const css::uno::Any& rNewValue
    ) = 0;

protected:
    ~IAccessibleTableControl() {}
};

} // namespace table
} // namespace vcl

#endif // INCLUDED_VCL_ACCESSIBLETABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
