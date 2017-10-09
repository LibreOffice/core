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

#include <extended/AccessibleGridControl.hxx>
#include <extended/AccessibleGridControlTable.hxx>
#include <extended/AccessibleGridControlHeader.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <svtools/accessibletable.hxx>
#include <comphelper/types.hxx>
#include <toolkit/helper/vclunohelper.hxx>


namespace accessibility
{


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::svt;
using namespace ::svt::table;


class AccessibleGridControl_Impl
{
public:
    /// the css::accessibility::XAccessible which created the AccessibleGridControl
    css::uno::WeakReference< css::accessibility::XAccessible >                    m_aCreator;

    /** The data table child. */
    rtl::Reference<AccessibleGridControlTable>                m_xTable;

    /** The header bar for rows. */
    rtl::Reference<AccessibleGridControlHeader>               m_xRowHeaderBar;

    /** The header bar for columns (first row of the table). */
    rtl::Reference<AccessibleGridControlHeader>               m_xColumnHeaderBar;

    /** The table cell child. */
    rtl::Reference<AccessibleGridControlTableCell>            m_xCell;

};

AccessibleGridControl::AccessibleGridControl(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent, const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator,
            ::svt::table::IAccessibleTable& _rTable )
    : AccessibleGridControlBase( _rxParent, _rTable, TCTYPE_GRIDCONTROL )
{
    m_xImpl.reset( new AccessibleGridControl_Impl );
    m_xImpl->m_aCreator = _rxCreator;
}


AccessibleGridControl::~AccessibleGridControl()
{
}


void SAL_CALL AccessibleGridControl::disposing()
{
    SolarMutexGuard g;

    m_xImpl->m_aCreator.clear();

    if ( m_xImpl->m_xTable.is() )
    {
        m_xImpl->m_xTable->dispose();
        m_xImpl->m_xTable.clear();
    }
    if ( m_xImpl->m_xCell.is() )
    {
        m_xImpl->m_xCell->dispose();
        m_xImpl->m_xCell.clear();
    }
    if ( m_xImpl->m_xRowHeaderBar.is() )
    {
        m_xImpl->m_xRowHeaderBar->dispose();
        m_xImpl->m_xRowHeaderBar.clear();
    }
    if ( m_xImpl->m_xColumnHeaderBar.is() )
    {
        m_xImpl->m_xColumnHeaderBar->dispose();
        m_xImpl->m_xColumnHeaderBar.clear();
    }
    AccessibleGridControlBase::disposing();
}


// css::accessibility::XAccessibleContext ---------------------------------------------------------

sal_Int32 SAL_CALL AccessibleGridControl::getAccessibleChildCount()
{
    SolarMutexGuard aSolarGuard;
    ensureIsAlive();
    return m_aTable.GetAccessibleControlCount();
}


css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
AccessibleGridControl::getAccessibleChild( sal_Int32 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    if (nChildIndex<0 || nChildIndex>=getAccessibleChildCount())
        throw IndexOutOfBoundsException();

    css::uno::Reference< css::accessibility::XAccessible > xChild;
    if (isAlive())
    {
        if(nChildIndex == 0 && m_aTable.HasColHeader())
        {
            if(!m_xImpl->m_xColumnHeaderBar.is())
            {
                m_xImpl->m_xColumnHeaderBar = new AccessibleGridControlHeader(m_xImpl->m_aCreator, m_aTable, svt::table::TCTYPE_COLUMNHEADERBAR);
            }
            xChild = m_xImpl->m_xColumnHeaderBar.get();
        }
        else if(m_aTable.HasRowHeader() && (nChildIndex == 1 || nChildIndex == 0))
        {
            if(!m_xImpl->m_xRowHeaderBar.is())
            {
                m_xImpl->m_xRowHeaderBar = new AccessibleGridControlHeader(m_xImpl->m_aCreator, m_aTable, svt::table::TCTYPE_ROWHEADERBAR);
            }
            xChild = m_xImpl->m_xRowHeaderBar.get();
        }
        else
        {
            if(!m_xImpl->m_xTable.is())
            {
                m_xImpl->m_xTable = new AccessibleGridControlTable(m_xImpl->m_aCreator, m_aTable);
            }
            xChild = m_xImpl->m_xTable.get();
        }
    }
    return xChild;
}


sal_Int16 SAL_CALL AccessibleGridControl::getAccessibleRole()
{
    SolarMutexGuard g;

    ensureIsAlive();
    return AccessibleRole::PANEL;
}


// css::accessibility::XAccessibleComponent -------------------------------------------------------

css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
AccessibleGridControl::getAccessibleAtPoint( const awt::Point& rPoint )
{
    SolarMutexGuard aSolarGuard;
    ensureIsAlive();

    css::uno::Reference< css::accessibility::XAccessible > xChild;
    sal_Int32 nIndex = 0;
    if( m_aTable.ConvertPointToControlIndex( nIndex, VCLPoint( rPoint ) ) )
        xChild = m_aTable.CreateAccessibleControl( nIndex );
    else
    {
        // try whether point is in one of the fixed children
        // (table, header bars, corner control)
        Point aPoint( VCLPoint( rPoint ) );
        for( nIndex = 0; (nIndex < 3) && !xChild.is(); ++nIndex )
        {
            css::uno::Reference< css::accessibility::XAccessible > xCurrChild( implGetFixedChild( nIndex ) );
            css::uno::Reference< css::accessibility::XAccessibleComponent >
            xCurrChildComp( xCurrChild, uno::UNO_QUERY );

            if( xCurrChildComp.is() &&
                VCLRectangle( xCurrChildComp->getBounds() ).IsInside( aPoint ) )
            xChild = xCurrChild;
        }
    }
    return xChild;
}


void SAL_CALL AccessibleGridControl::grabFocus()
{
    SolarMutexGuard aSolarGuard;
    ensureIsAlive();
        m_aTable.GrabFocus();
}

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleGridControl::getImplementationName()
{
    return OUString( "com.sun.star.accessibility.AccessibleGridControl" );
}


// internal virtual methods ---------------------------------------------------

tools::Rectangle AccessibleGridControl::implGetBoundingBox()
{
    vcl::Window* pParent = m_aTable.GetAccessibleParentWindow();
    OSL_ENSURE( pParent, "implGetBoundingBox - missing parent window" );
    return m_aTable.GetWindowExtentsRelative( pParent );
}


tools::Rectangle AccessibleGridControl::implGetBoundingBoxOnScreen()
{
    return m_aTable.GetWindowExtentsRelative( nullptr );
}
// internal helper methods ----------------------------------------------------

css::uno::Reference< css::accessibility::XAccessible > AccessibleGridControl::implGetTable()
{
    if( !m_xImpl->m_xTable.is() )
    {
        m_xImpl->m_xTable = createAccessibleTable();
    }
    return m_xImpl->m_xTable.get();
}


css::uno::Reference< css::accessibility::XAccessible >
AccessibleGridControl::implGetHeaderBar( AccessibleTableControlObjType eObjType )
{
    css::uno::Reference< css::accessibility::XAccessible > xRet;
    rtl::Reference< AccessibleGridControlHeader >* pxMember = nullptr;

    if( eObjType == TCTYPE_ROWHEADERBAR )
        pxMember = &m_xImpl->m_xRowHeaderBar;
    else if( eObjType ==  TCTYPE_COLUMNHEADERBAR )
        pxMember = &m_xImpl->m_xColumnHeaderBar;

    if( pxMember )
    {
        if( !pxMember->is() )
        {
            *pxMember = new AccessibleGridControlHeader(
                m_xImpl->m_aCreator, m_aTable, eObjType );
        }
        xRet = pxMember->get();
    }
    return xRet;
}

css::uno::Reference< css::accessibility::XAccessible >
AccessibleGridControl::implGetFixedChild( sal_Int32 nChildIndex )
{
    css::uno::Reference< css::accessibility::XAccessible > xRet;
    switch( nChildIndex )
    {
        /** Child index of the column header bar (first row). */
        case 0:
            xRet = implGetHeaderBar( TCTYPE_COLUMNHEADERBAR );
        break;
        /** Child index of the row header bar ("handle column"). */
        case 1:
            xRet = implGetHeaderBar( TCTYPE_ROWHEADERBAR );
        break;
        /** Child index of the data table. */
        case 2:
            xRet = implGetTable();
        break;
    }
    return xRet;
}

AccessibleGridControlTable* AccessibleGridControl::createAccessibleTable()
{
    css::uno::Reference< css::accessibility::XAccessible > xCreator(m_xImpl->m_aCreator);
    OSL_ENSURE( xCreator.is(), "extended/AccessibleGridControl::createAccessibleTable: my creator died - how this?" );
    return new AccessibleGridControlTable( xCreator, m_aTable );
}

void AccessibleGridControl::commitCellEvent(sal_Int16 _nEventId,const Any& _rNewValue,const Any& _rOldValue)
{
    sal_Int32 nChildCount = getAccessibleChildCount();
    if(nChildCount != 0)
    {
        for(sal_Int32 i=0;i<nChildCount;i++)
        {
            css::uno::Reference< css::accessibility::XAccessible > xAccessible = getAccessibleChild(i);
            if(css::uno::Reference< css::accessibility::XAccessible >(m_xImpl->m_xTable.get()) == xAccessible)
            {
                std::vector< AccessibleGridControlTableCell* >& rCells =
                    m_xImpl->m_xTable->getCellVector();
                size_t nIndex = m_aTable.GetCurrentRow() * m_aTable.GetColumnCount()
                              + m_aTable.GetCurrentColumn();
                if (nIndex < rCells.size() && rCells[nIndex])
                {
                    m_xImpl->m_xCell = rCells[nIndex];
                    m_xImpl->m_xCell->commitEvent( _nEventId, _rNewValue, _rOldValue );
                }
            }
        }
    }
    else
    {
        if ( m_xImpl->m_xTable.is() )
            m_xImpl->m_xTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
    }
}

void AccessibleGridControl::commitTableEvent(sal_Int16 _nEventId,const Any& _rNewValue,const Any& _rOldValue)
{
    if ( m_xImpl->m_xTable.is() )
    {
        if(_nEventId == AccessibleEventId::ACTIVE_DESCENDANT_CHANGED)
        {
            const sal_Int32 nCurrentRow = m_aTable.GetCurrentRow();
            const sal_Int32 nCurrentCol = m_aTable.GetCurrentColumn();
            css::uno::Reference< css::accessibility::XAccessible > xChild;
            if (nCurrentRow > -1 && nCurrentCol > -1)
            {
                sal_Int32 nColumnCount = m_aTable.GetColumnCount();
                xChild = m_xImpl->m_xTable->getAccessibleChild(nCurrentRow * nColumnCount + nCurrentCol);
            }
            m_xImpl->m_xTable->commitEvent(_nEventId, Any(xChild),_rOldValue);
        }
        else if(_nEventId == AccessibleEventId::TABLE_MODEL_CHANGED)
        {
            AccessibleTableModelChange aChange;
            if(_rNewValue >>= aChange)
            {
                if(aChange.Type == AccessibleTableModelChangeType::DELETE)
                {
                    std::vector< AccessibleGridControlTableCell* >& rCells =
                        m_xImpl->m_xTable->getCellVector();
                    std::vector< css::uno::Reference< css::accessibility::XAccessible > >& rAccCells =
                        m_xImpl->m_xTable->getAccessibleCellVector();
                    int nColCount = m_aTable.GetColumnCount();
                    // check valid index - entries are inserted lazily
                    size_t const nStart = nColCount * aChange.FirstRow;
                    size_t const nEnd   = nColCount * aChange.LastRow;
                    if (nStart < rCells.size())
                    {
                        m_xImpl->m_xTable->getCellVector().erase(
                            rCells.begin() + nStart,
                            rCells.begin() + std::min(rCells.size(), nEnd));
                    }
                    if (nStart < rAccCells.size())
                    {
                        m_xImpl->m_xTable->getAccessibleCellVector().erase(
                            rAccCells.begin() + nStart,
                            rAccCells.begin() + std::min(rAccCells.size(), nEnd));
                    }
                    m_xImpl->m_xTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
                }
                else
                    m_xImpl->m_xTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
            }
        }
        else
            m_xImpl->m_xTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
    }
}

// = AccessibleGridControlAccess


AccessibleGridControlAccess::AccessibleGridControlAccess(
        const css::uno::Reference< css::accessibility::XAccessible >& rxParent, ::svt::table::IAccessibleTable& rTable )
    : m_xParent( rxParent )
    , m_pTable( & rTable )
{
}


AccessibleGridControlAccess::~AccessibleGridControlAccess()
{
}


void AccessibleGridControlAccess::DisposeAccessImpl()
{
    SolarMutexGuard g;

    m_pTable = nullptr;
    if (m_xContext.is())
    {
        m_xContext->dispose();
        m_xContext.clear();
    }
}


css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL AccessibleGridControlAccess::getAccessibleContext()
{
    SolarMutexGuard g;

    // if the context died meanwhile (we're no listener, so it won't tell us explicitly when this happens),
    // then reset an re-create.
    if ( m_xContext.is() && !m_xContext->isAlive() )
        m_xContext = nullptr;

    if (!m_xContext.is() && m_pTable)
        m_xContext = new AccessibleGridControl(m_xParent, this, *m_pTable);

    return m_xContext.get();
}


}   // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
