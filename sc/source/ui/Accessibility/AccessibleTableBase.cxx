/*************************************************************************
 *
 *  $RCSfile: AccessibleTableBase.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2002-01-18 09:54:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "AccessibleTableBase.hxx"
#ifndef SC_MISCUNO_HXX
#include "miscuno.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEADDRESSABLE_HPP_
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleTableBase::ScAccessibleTableBase (
        const uno::Reference<XAccessible>& rxParent,
        const uno::Reference<sheet::XSpreadsheetView >& rxSheetView,
        const table::CellRangeAddress& rRange)
    :
    SvAccessibleContextBase (rxParent, AccessibleRole::TABLE),
    mxSheetView(rxSheetView),
    maRange(rRange),
    mxSheet(rxSheetView->getActiveSheet())
{
}

ScAccessibleTableBase::~ScAccessibleTableBase ()
{
}

    //=====  XInterface  ======================================================

uno::Any SAL_CALL
    ScAccessibleTableBase::queryInterface(
    const uno::Type & rType )
    throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( XAccessibleTable )

    return SvAccessibleContextBase::queryInterface( rType );
}

/** Increase the reference count.
*/
void SAL_CALL
    ScAccessibleTableBase::acquire (void)
    throw ()
{
    OWeakObject::acquire ();
}

/** Decrease the reference count.
*/
void SAL_CALL
    ScAccessibleTableBase::release (void)
    throw ()
{
    OWeakObject::release ();
}

    //=====  XAccessibleTable  ================================================

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleRowCount(  )
                    throw (uno::RuntimeException)
{
    return maRange.EndRow - maRange.StartRow + 1;
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleColumnCount(  )
                    throw (uno::RuntimeException)
{
    return maRange.EndColumn - maRange.StartColumn + 1;
}

::rtl::OUString SAL_CALL ScAccessibleTableBase::getAccessibleRowDescription( sal_Int32 nRow )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("Here should be a implementation to fill the description");
    //setAccessibleRowDescription(nRow, xAccessible); // to remember the created Description
    return rtl::OUString();
}

::rtl::OUString SAL_CALL ScAccessibleTableBase::getAccessibleColumnDescription( sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("Here should be a implementation to fill the description");
    //setAccessibleColumnDescription(nColumn, xAccessible); // to remember the created Description
    return rtl::OUString();
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    uno::Reference<table::XCellRange> xFullCellRange = mxSheetView->getActiveSheet()->getCellRangeByPosition(
        maRange.StartColumn, maRange.StartRow, maRange.EndColumn, maRange.EndRow);
    if (xFullCellRange.is())
    {
        uno::Reference<table::XCellRange> xCellRange = xFullCellRange->getCellRangeByPosition(
            nColumn, nRow, nColumn, nRow);
        if (xCellRange.is())
        {
            uno::Reference<sheet::XSheetCellRange> xSheetCellRange(xCellRange, uno::UNO_QUERY);
            if (xSheetCellRange.is())
            {
                uno::Reference<sheet::XSheetCellCursor> xCursor = mxSheetView->getActiveSheet()->createCursorByRange(xSheetCellRange);
                if(xCursor.is())
                {
                    uno::Reference<sheet::XCellRangeAddressable> xCellAddress (xCursor, uno::UNO_QUERY);
                    xCursor->collapseToMergedArea();
                    table::CellRangeAddress aCellAddress = xCellAddress->getRangeAddress();
                    return aCellAddress.EndRow - aCellAddress.StartRow + 1;
                }
            }
        }
    }
    return 1; // the same cell
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    uno::Reference<table::XCellRange> xFullCellRange = mxSheetView->getActiveSheet()->getCellRangeByPosition(
        maRange.StartColumn, maRange.StartRow, maRange.EndColumn, maRange.EndRow);
    if (xFullCellRange.is())
    {
        uno::Reference<table::XCellRange> xCellRange = xFullCellRange->getCellRangeByPosition(
            nColumn, nRow, nColumn, nRow);
        if (xCellRange.is())
        {
            uno::Reference<sheet::XSheetCellRange> xSheetCellRange(xCellRange, uno::UNO_QUERY);
            if (xSheetCellRange.is())
            {
                uno::Reference<sheet::XSheetCellCursor> xCursor = mxSheetView->getActiveSheet()->createCursorByRange(xSheetCellRange);
                if(xCursor.is())
                {
                    uno::Reference<sheet::XCellRangeAddressable> xCellAddress (xCursor, uno::UNO_QUERY);
                    xCursor->collapseToMergedArea();
                    table::CellRangeAddress aCellAddress = xCellAddress->getRangeAddress();
                    return aCellAddress.EndColumn - aCellAddress.StartColumn + 1;
                }
            }
        }
    }
    return 1; // the same cell
}

uno::Reference< XAccessibleTable > SAL_CALL ScAccessibleTableBase::getAccessibleRowHeaders(  )
                    throw (uno::RuntimeException)
{
    uno::Reference< XAccessibleTable > xAccessibleTable;
    DBG_ERROR("Here should be a implementation to fill the row headers");
    //CommitChange
    return xAccessibleTable;
}

uno::Reference< XAccessibleTable > SAL_CALL ScAccessibleTableBase::getAccessibleColumnHeaders(  )
                    throw (uno::RuntimeException)
{
    uno::Reference< XAccessibleTable > xAccessibleTable;
    DBG_ERROR("Here should be a implementation to fill the column headers");
    //CommitChange
    return xAccessibleTable;
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessibleTableBase::getSelectedAccessibleRows(  )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Sequence< sal_Int32 > aSequence;
    return aSequence;
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessibleTableBase::getSelectedAccessibleColumns(  )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Sequence< sal_Int32 > aSequence;
    return aSequence;
}

sal_Bool SAL_CALL ScAccessibleTableBase::isAccessibleRowSelected( sal_Int32 nRow )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    return sal_False;
}

sal_Bool SAL_CALL ScAccessibleTableBase::isAccessibleColumnSelected( sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    return sal_False;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleTableBase::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleTableBase::getAccessibleCaption(  )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleTableBase::getAccessibleSummary(  )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

sal_Bool SAL_CALL ScAccessibleTableBase::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    return sal_False;
}

    //=====  XAccessibleExtendedTable  ========================================

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    return -1;
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleRow( sal_Int32 nChildIndex )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    return -1;
}

sal_Int32 SAL_CALL ScAccessibleTableBase::getAccessibleColumn( sal_Int32 nChildIndex )
                    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    return -1;
}

    //=====  XAccessibleContext  ==============================================

long SAL_CALL
    ScAccessibleTableBase::getAccessibleChildCount (void)
{
    DBG_ERROR("not implemented yet");
    return 0;
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessibleTableBase::getAccessibleChild (long nIndex)
        throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Reference< XAccessible > xAccessible;
    return xAccessible;
}

::rtl::OUString SAL_CALL
    ScAccessibleTableBase::getAccessibleDescription (void)
    throw (uno::RuntimeException)
{
    setAccessibleDescription(getAccessibleName());
    return SvAccessibleContextBase::getAccessibleDescription();
}

::rtl::OUString SAL_CALL
    ScAccessibleTableBase::getAccessibleName (void)
    throw (uno::RuntimeException)
{
    if (mxSheet.is())
    {
        uno::Reference<container::XNamed> xName (mxSheet, uno::UNO_QUERY );
        if ( xName.is() )
            setAccessibleName(xName->getName());
    }
    return SvAccessibleContextBase::getAccessibleName();
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
    ScAccessibleTableBase::getAccessibleRelationSet (void)
    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    return uno::Reference<XAccessibleRelationSet>();
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleTableBase::getAccessibleStateSet (void)
    throw (uno::RuntimeException)
{
    DBG_ERROR("not implemented yet");
    uno::Reference< XAccessibleStateSet > xAccessibleStateSet;
    return xAccessibleStateSet;
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleTableBase::getImplementationName (void)
        throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleTableBase"));
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScAccessibleTableBase::getSupportedServiceNames (void)
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = SvAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = getServiceName();

    return aSequence;
}

    //=====  XTypeProvider  ===================================================

uno::Sequence< uno::Type> SAL_CALL ScAccessibleTableBase::getTypes (void)
        throw (uno::RuntimeException)
{
    uno::Sequence< uno::Type>
        aTypeSequence = SvAccessibleContextBase::getTypes();
    sal_Int32 nOldSize(aTypeSequence.getLength());
    aTypeSequence.realloc(nOldSize + 1);
    uno::Type* pTypes = aTypeSequence.getArray();

    pTypes[nOldSize] = ::getCppuType((const uno::Reference<
            XAccessibleTable>*)0);

    return aTypeSequence;
}

uno::Sequence<sal_Int8> SAL_CALL ScAccessibleTableBase::getImplementationId (void)
    throw (uno::RuntimeException)
{
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        ::vos::OGuard aGuard (maMutex);
        aId.realloc (16);
        rtl_createUuid ((sal_uInt8 *)aId.getArray(), 0, sal_True);
    }
    return aId;
}

    //=====  XServiceName  ====================================================

::rtl::OUString SAL_CALL ScAccessibleTableBase::getServiceName (void)
        throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM (
        "drafts.com.sun.star.accessibility.AccessibleTable"));
}

