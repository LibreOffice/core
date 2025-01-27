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
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <cppuhelper/implbase.hxx>

namespace vcl { class Window; }

namespace vcl::table
{

enum class AccessibleTableControlObjType
{
    GRIDCONTROL,         /// The GridControl itself.
    TABLE,               /// The data table.
    ROWHEADERBAR,        /// The row header bar.
    COLUMNHEADERBAR,     /// The horizontal column header bar.
    TABLECELL,           /// A cell of the data table.
    ROWHEADERCELL,       /// A cell of the row header bar.
    COLUMNHEADERCELL,    /// A cell of the column header bar.
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
    virtual void FillAccessibleStateSet( sal_Int64& rStateSet,
            AccessibleTableControlObjType eObjType ) const= 0;

    // Window
    virtual sal_Int32 GetAccessibleControlCount() const = 0;
    virtual sal_Int32 GetRowCount() const= 0;
    virtual sal_Int32 GetColumnCount() const= 0;
    virtual bool ConvertPointToCellAddress( sal_Int32& _rnRow, sal_Int32& _rnColPos, const Point& _rPoint )= 0;
    virtual tools::Rectangle calcHeaderRect( bool _bIsColumnBar ) = 0;
    virtual tools::Rectangle calcHeaderCellRect( bool _bColHeader, sal_Int32 _nPos ) = 0;
    virtual tools::Rectangle calcTableRect() = 0;
    virtual tools::Rectangle calcCellRect( sal_Int32 _nRowPos, sal_Int32 _nColPos ) = 0;
    virtual void FillAccessibleStateSetForCell( sal_Int64& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const= 0;
    virtual OUString GetRowName(sal_Int32 _nIndex) const = 0;
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

} // namespace vcl::table

#endif // INCLUDED_VCL_ACCESSIBLETABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
