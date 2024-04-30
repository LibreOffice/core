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

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <extended/AccessibleGridControlTable.hxx>
#include <extended/AccessibleGridControlTableCell.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/accessibletable.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::vcl;
using namespace ::vcl::table;


namespace accessibility {


AccessibleGridControlTable::AccessibleGridControlTable(
        const Reference< XAccessible >& rxParent,
        IAccessibleTable& rTable) :
    AccessibleGridControlTableBase(rxParent, rTable, AccessibleTableControlObjType::TABLE)
{
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlTable::getAccessibleChild( sal_Int64 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    sal_Int64 nCount = getAccessibleChildCount();
    if(m_aCellVector.empty() || m_aCellVector.size() != static_cast<unsigned>(nCount))
    {
        assert(o3tl::make_unsigned(nCount) < m_aCellVector.max_size());
        m_aCellVector.resize(nCount);
    }
    if(!m_aCellVector[nChildIndex].is())
    {
        m_aCellVector[nChildIndex].set(new AccessibleGridControlTableCell(this, m_aTable, nChildIndex/m_aTable.GetColumnCount(), nChildIndex%m_aTable.GetColumnCount()));
    }
    return m_aCellVector[nChildIndex];
}

sal_Int64 SAL_CALL AccessibleGridControlTable::getAccessibleIndexInParent()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    if(m_aTable.HasRowHeader() && m_aTable.HasColHeader())
        return 0;
    else if((!m_aTable.HasRowHeader() && m_aTable.HasColHeader()) || (m_aTable.HasRowHeader() && !m_aTable.HasColHeader()) )
        return 1;
    else
        return 2;
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlTable::getAccessibleAtPoint( const awt::Point& rPoint )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();

    Reference< XAccessible > xChild;
    sal_Int32 nRow = 0;
    sal_Int32 nColumnPos = 0;
    if( m_aTable.ConvertPointToCellAddress( nRow, nColumnPos, VCLPoint( rPoint ) ) )
        xChild = new AccessibleGridControlTableCell(this, m_aTable, nRow, nColumnPos);
    return xChild;
}

void SAL_CALL AccessibleGridControlTable::grabFocus()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    m_aTable.GrabFocus();
}

// XAccessibleTable -----------------------------------------------------------

OUString SAL_CALL AccessibleGridControlTable::getAccessibleRowDescription( sal_Int32 nRow )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidRow( nRow );
    return u"row description"_ustr;
}

OUString SAL_CALL AccessibleGridControlTable::getAccessibleColumnDescription( sal_Int32 nColumn )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return u"col description"_ustr;
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlTable::getAccessibleRowHeaders()
{
    SolarMutexGuard g;

    ensureIsAlive();
    if(m_aTable.HasColHeader())
        return implGetHeaderBar( 1 );
    else
        return implGetHeaderBar( 0 );
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlTable::getAccessibleColumnHeaders()
{
    SolarMutexGuard g;

    ensureIsAlive();
    return implGetHeaderBar( 0 );
}

Sequence< sal_Int32 > SAL_CALL AccessibleGridControlTable::getSelectedAccessibleRows()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    Sequence< sal_Int32 > aSelSeq;
    implGetSelectedRows( aSelSeq );
    return aSelSeq;
}

//columns aren't selectable
Sequence< sal_Int32 > SAL_CALL AccessibleGridControlTable::getSelectedAccessibleColumns()
{
    return {};
}

sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleRowSelected( sal_Int32 nRow )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidRow( nRow );
    Sequence< sal_Int32 > selectedRows = getSelectedAccessibleRows();
    return comphelper::findValue(selectedRows, nRow) != -1;
}

//columns aren't selectable
sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleColumnSelected( sal_Int32 )
{
    return false;
}

Reference< XAccessible > SAL_CALL AccessibleGridControlTable::getAccessibleCellAt(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    sal_Int64 nChildIndex = static_cast<sal_Int64>(nRow) * static_cast<sal_Int64>(m_aTable.GetColumnCount()) + nColumn;
    return getAccessibleChild(nChildIndex);
}

sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleSelected(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    //selection of single cells not possible, so if row is selected, the cell will be selected too
    return isAccessibleRowSelected(nRow);
}
void SAL_CALL AccessibleGridControlTable::selectAccessibleChild( sal_Int64 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    sal_Int32 nRow = nChildIndex / nColumns;
    m_aTable.SelectRow( nRow, true );
}
sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    sal_Int32 nRow = nChildIndex / nColumns;
    return isAccessibleRowSelected(nRow);
}
void SAL_CALL AccessibleGridControlTable::clearAccessibleSelection()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    m_aTable.SelectAllRows( false );
}
void SAL_CALL AccessibleGridControlTable::selectAllAccessibleChildren()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    Sequence< sal_Int32 > selectedRows = getSelectedAccessibleRows();
    auto selectedRowsRange = asNonConstRange(selectedRows);
    for(tools::Long i=0; i<m_aTable.GetRowCount(); i++)
        selectedRowsRange[i]=i;
}
sal_Int64 SAL_CALL AccessibleGridControlTable::getSelectedAccessibleChildCount()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    Sequence< sal_Int32 > selectedRows = getSelectedAccessibleRows();
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    return static_cast<sal_Int64>(selectedRows.getLength()) * static_cast<sal_Int64>(nColumns);
}
Reference< XAccessible > SAL_CALL
AccessibleGridControlTable::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    if (nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount())
        throw lang::IndexOutOfBoundsException(u"Invalid index into selection"_ustr, *this);

    const sal_Int32 nColCount = getAccessibleColumnCount();
    assert(nColCount > 0 && "Column count non-positive, but child count > 0");
    const sal_Int32 nIndexInSelectedRowsSequence = nSelectedChildIndex / nColCount;
    const Sequence<sal_Int32> aSelectedRows = getSelectedAccessibleRows();
    const sal_Int32 nRowIndex = aSelectedRows[nIndexInSelectedRowsSequence];
    const sal_Int32 nColIndex = nSelectedChildIndex % nColCount;
    return getAccessibleCellAt(nRowIndex, nColIndex);
}
//not implemented yet, because only row selection possible
void SAL_CALL AccessibleGridControlTable::deselectAccessibleChild(
        sal_Int64 )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
}
// XInterface -----------------------------------------------------------------

Any SAL_CALL AccessibleGridControlTable::queryInterface( const uno::Type& rType )
{
    Any aAny( AccessibleGridControlTableBase::queryInterface( rType ) );
    return aAny.hasValue() ?
        aAny : AccessibleGridControlTableSelectionImplHelper::queryInterface( rType );
}

void SAL_CALL AccessibleGridControlTable::acquire() noexcept
{
    AccessibleGridControlTableBase::acquire();
}

void SAL_CALL AccessibleGridControlTable::release() noexcept
{
    AccessibleGridControlTableBase::release();
}
// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleGridControlTable::getImplementationName()
{
    return u"com.sun.star.accessibility.AccessibleGridControlTable"_ustr;
}

void AccessibleGridControlTable::dispose()
{
    for (rtl::Reference<AccessibleGridControlTableCell>& rxCell : m_aCellVector)
    {
        if (rxCell.is())
        {
            rxCell->dispose();
            rxCell.clear();
        }
    }

    AccessibleGridControlTableBase::dispose();
}

void AccessibleGridControlTable::commitEvent(sal_Int16 nEventId, const css::uno::Any& rNewValue,
                                             const css::uno::Any& rOldValue)
{
    if (nEventId == AccessibleEventId::TABLE_MODEL_CHANGED)
    {
        AccessibleTableModelChange aChange;
        if (rNewValue >>= aChange)
        {
            assert(aChange.Type != AccessibleTableModelChangeType::COLUMNS_REMOVED);

            if (aChange.Type == AccessibleTableModelChangeType::ROWS_REMOVED)
            {
                int nColCount = m_aTable.GetColumnCount();
                // check valid index - entries are inserted lazily
                size_t const nStart = nColCount * aChange.FirstRow;
                size_t const nEnd = nColCount * aChange.LastRow;
                if (nStart < m_aCellVector.size())
                {
                    m_aCellVector.erase(
                        m_aCellVector.begin() + nStart,
                        m_aCellVector.begin() + std::min(m_aCellVector.size(), nEnd));
                }
            }
        }
    }

    AccessibleGridControlBase::commitEvent(nEventId, rNewValue, rOldValue);
}

// internal virtual methods ---------------------------------------------------

tools::Rectangle AccessibleGridControlTable::implGetBoundingBox()
{
    vcl::Window* pParent = m_aTable.GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "implGetBoundingBox - missing parent window" );
    tools::Rectangle aGridRect( m_aTable.GetWindowExtentsRelative( *pParent ));
    tools::Rectangle aTableRect( m_aTable.calcTableRect() );
    tools::Long nX = aGridRect.Left() + aTableRect.Left();
    tools::Long nY = aGridRect.Top() + aTableRect.Top();
    tools::Long nWidth = aGridRect.GetSize().Width()-aTableRect.Left();
    tools::Long nHeight = aGridRect.GetSize().Height()-aTableRect.Top();
    tools::Rectangle aTable( Point( nX, nY ), Size( nWidth, nHeight ));
    return aTable;
}

AbsoluteScreenPixelRectangle AccessibleGridControlTable::implGetBoundingBoxOnScreen()
{
    tools::Rectangle aGridRect( m_aTable.GetWindowExtentsAbsolute());
    tools::Rectangle aTableRect( m_aTable.calcTableRect() );
    tools::Long nX = aGridRect.Left() + aTableRect.Left();
    tools::Long nY = aGridRect.Top() + aTableRect.Top();
    tools::Long nWidth = aGridRect.GetSize().Width()-aTableRect.Left();
    tools::Long nHeight = aGridRect.GetSize().Height()-aTableRect.Top();
    AbsoluteScreenPixelRectangle aTable( AbsoluteScreenPixelPoint( nX, nY ), AbsoluteScreenPixelSize( nWidth, nHeight ));
    return aTable;
}
// internal helper methods ----------------------------------------------------
Reference< XAccessibleTable > AccessibleGridControlTable::implGetHeaderBar(
        sal_Int32 nChildIndex )
{
    Reference< XAccessible > xRet;

    if (!m_xParent.is())
        return nullptr;

    Reference<XAccessibleContext> xContext = m_xParent->getAccessibleContext();
    if( xContext.is() )
    {
        try
        {
            xRet = xContext->getAccessibleChild( nChildIndex );
        }
        catch (const lang::IndexOutOfBoundsException&)
        {
            OSL_FAIL( "implGetHeaderBar - wrong child index" );
        }
        // RuntimeException goes to caller
    }
    return Reference< XAccessibleTable >( xRet, uno::UNO_QUERY );
}


} // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
