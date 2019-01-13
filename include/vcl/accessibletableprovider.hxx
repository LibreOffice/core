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

#ifndef INCLUDED_VCL_ACCESSIBLETABLEPROVIDER_HXX
#define INCLUDED_VCL_ACCESSIBLETABLEPROVIDER_HXX

#include <vcl/AccessibleBrowseBoxObjType.hxx>
#include <vcl/outdev.hxx>

namespace com { namespace sun { namespace star { namespace accessibility { class XAccessible; } } } }
namespace utl { class AccessibleStateSetHelper; }

namespace vcl
{

#define OFFSET_DEFAULT  (sal_Int32(-1))
#define OFFSET_NONE     (sal_Int32(0))

enum AccessibleTableChildIndex
{
    /** Child index of the column header bar (first row). Exists always. */
    BBINDEX_COLUMNHEADERBAR = 0,
    /** Child index of the row header bar ("handle column"). Exists always. */
    BBINDEX_ROWHEADERBAR    = 1,
    /** Child index of the data table. */
    BBINDEX_TABLE           = 2,
    /** Child index of the first additional control. */
    BBINDEX_FIRSTCONTROL    = 3
};


/** This abstract class provides methods to implement an accessible table object.
*/
class IAccessibleTableProvider
{
public:
    /** @return  The count of the rows. */
    virtual long                    GetRowCount() const = 0;
    /** @return  The count of the columns. */
    virtual sal_uInt16              GetColumnCount() const = 0;

    /** @return  The position of the current row. */
    virtual sal_Int32               GetCurrRow() const = 0;
    /** @return  The position of the current column. */
    virtual sal_uInt16              GetCurrColumn() const = 0;

    /** @return  The description of a row.
        @param _nRow The row which description is in demand. */
    virtual OUString         GetRowDescription( sal_Int32 _nRow ) const = 0;
    /** @return  The description of a column.
        @param _nColumn The column which description is in demand. */
    virtual OUString         GetColumnDescription( sal_uInt16 _nColumnPos ) const = 0;

    /** @return  <TRUE/>, if the object has a row header. */
    virtual bool                    HasRowHeader() const = 0; //GetColumnId
    virtual bool                    GoToCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos ) = 0;

    virtual void                    SetNoSelection() = 0;
    virtual void                    SelectAll() = 0;
    virtual void                    SelectRow( long _nRow, bool _bSelect = true, bool bExpand = true ) = 0;
    virtual void                    SelectColumn( sal_uInt16 _nColumnPos, bool _bSelect = true ) = 0;
    virtual sal_Int32               GetSelectedRowCount() const = 0;
    virtual sal_Int32               GetSelectedColumnCount() const = 0;
    /** @return  <TRUE/>, if the row is selected. */
    virtual bool                    IsRowSelected( long _nRow ) const = 0;
    virtual bool                    IsColumnSelected( long _nColumnPos ) const = 0;
    virtual void                    GetAllSelectedRows( css::uno::Sequence< sal_Int32 >& _rRows ) const = 0;
    virtual void                    GetAllSelectedColumns( css::uno::Sequence< sal_Int32 >& _rColumns ) const = 0;

    /** @return  <TRUE/>, if the cell is visible. */
    virtual bool                    IsCellVisible( sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const = 0;
    virtual OUString                GetAccessibleCellText( long _nRow, sal_uInt16 _nColumnPos ) const = 0;

    virtual tools::Rectangle               calcHeaderRect( bool _bIsColumnBar, bool _bOnScreen = true ) = 0;
    virtual tools::Rectangle               calcTableRect( bool _bOnScreen = true ) = 0;
    virtual tools::Rectangle               GetFieldRectPixelAbs( sal_Int32 _nRow, sal_uInt16 _nColumnPos, bool _bIsHeader, bool _bOnScreen = true ) = 0;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos ) = 0;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleRowHeader( sal_Int32 _nRow ) = 0;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleColumnHeader( sal_uInt16 _nColumnPos ) = 0;

    virtual sal_Int32               GetAccessibleControlCount() const = 0;
    virtual css::uno::Reference< css::accessibility::XAccessible >                    CreateAccessibleControl( sal_Int32 _nIndex ) = 0;
    virtual bool                    ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint ) = 0;

    virtual bool                    ConvertPointToCellAddress( sal_Int32& _rnRow, sal_uInt16& _rnColPos, const Point& _rPoint ) = 0;
    virtual bool                    ConvertPointToRowHeader( sal_Int32& _rnRow, const Point& _rPoint ) = 0;
    virtual bool                    ConvertPointToColumnHeader( sal_uInt16& _rnColPos, const Point& _rPoint ) = 0;

    virtual OUString                GetAccessibleObjectName( ::vcl::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const = 0;
    virtual OUString                GetAccessibleObjectDescription( ::vcl::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const = 0;

    virtual void                    FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& _rStateSet, ::vcl::AccessibleBrowseBoxObjType _eType ) const = 0;
    virtual void                    FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const = 0;
    virtual void                    GrabTableFocus() = 0;

    // OutputDevice
    virtual bool                    GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex, int nLen, MetricVector& rVector ) = 0;

    // Window
    virtual tools::Rectangle               GetWindowExtentsRelative( vcl::Window *pRelativeWindow ) const = 0;
    virtual void                    GrabFocus() = 0;
    virtual css::uno::Reference< css::accessibility::XAccessible > GetAccessible() = 0;
    virtual vcl::Window*                 GetAccessibleParentWindow() const = 0;
    virtual vcl::Window*                 GetWindowInstance() = 0;

    virtual tools::Rectangle               GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex) = 0;
    virtual sal_Int32               GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint) = 0;

protected:
    ~IAccessibleTableProvider() {}
};


/** interface for an implementation of a table control's Accessible component
*/
class IAccessibleTabListBox
{
public:
    /** returns the XAccessible object itself

        The reference returned here can be used to control the life time of the
        IAccessibleTableImplementation object.

        The returned reference is guaranteed to not be <NULL/>.
    */
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getMyself() = 0;

    /** returns the accessible object for the column header bar
    */
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getHeaderBar() = 0;

protected:
    ~IAccessibleTabListBox() {}
};

/** interface for an implementation of a browse box's Accessible component
*/
class IAccessibleBrowseBox
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
    virtual void dispose() = 0;

    /** checks whether the accessible implementation, and its context, are still alive
        @return  <TRUE/>, if the object is not disposed or disposing.
    */
    virtual bool isAlive() const = 0;

    /** returns the accessible object for the row or the column header bar
    */
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getHeaderBar( ::vcl::AccessibleBrowseBoxObjType _eObjType ) = 0;

    /** returns the accessible object for the table representation
    */
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getTable() = 0;

    /** commits the event at all listeners of the column/row header bar
        @param nEventId
            the event id
        @param rNewValue
            the new value
        @param rOldValue
            the old value
    */
    virtual void commitHeaderBarEvent(
        sal_Int16 nEventId,
        const css::uno::Any& rNewValue,
        const css::uno::Any& rOldValue,
        bool _bColumnHeaderBar
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

    /** Commits an event to all listeners. */
    virtual void commitEvent(
        sal_Int16 nEventId,
        const css::uno::Any& rNewValue,
        const css::uno::Any& rOldValue
    ) = 0;

protected:
    ~IAccessibleBrowseBox() {}
};

} // namespace vcl

#endif // INCLUDED_VCL_ACCESSIBLETABLEPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
