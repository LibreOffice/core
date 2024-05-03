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
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <toolkit/helper/convert.hxx>
#include <utility>
#include <vcl/accessibletable.hxx>
#include <vcl/svapp.hxx>

namespace accessibility
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::vcl;
using namespace ::vcl::table;

AccessibleGridControl::AccessibleGridControl(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent, const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator,
            ::vcl::table::IAccessibleTable& _rTable )
    : AccessibleGridControlBase(_rxParent, _rTable, AccessibleTableControlObjType::GRIDCONTROL),
      m_aCreator(_rxCreator)
{
}


void SAL_CALL AccessibleGridControl::disposing()
{
    SolarMutexGuard g;

    m_aCreator.clear();

    if ( m_xTable.is() )
    {
        m_xTable->dispose();
        m_xTable.clear();
    }
    if ( m_xRowHeaderBar.is() )
    {
        m_xRowHeaderBar->dispose();
        m_xRowHeaderBar.clear();
    }
    if ( m_xColumnHeaderBar.is() )
    {
        m_xColumnHeaderBar->dispose();
        m_xColumnHeaderBar.clear();
    }
    AccessibleGridControlBase::disposing();
}

sal_Int64 AccessibleGridControl::implGetAccessibleChildCount()
{
    return m_aTable.GetAccessibleControlCount();
}

// css::accessibility::XAccessibleContext ---------------------------------------------------------


sal_Int64 SAL_CALL AccessibleGridControl::getAccessibleChildCount()
{
    SolarMutexGuard aSolarGuard;
    ensureIsAlive();
    return implGetAccessibleChildCount();
}


css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
AccessibleGridControl::getAccessibleChild( sal_Int64 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    if (nChildIndex<0 || nChildIndex>=implGetAccessibleChildCount())
        throw IndexOutOfBoundsException();

    css::uno::Reference< css::accessibility::XAccessible > xChild;
    if (isAlive())
    {
        if(nChildIndex == 0 && m_aTable.HasColHeader())
        {
            if(!m_xColumnHeaderBar.is())
            {
                m_xColumnHeaderBar = new AccessibleGridControlHeader(m_aCreator, m_aTable,
                                                                     vcl::table::AccessibleTableControlObjType::COLUMNHEADERBAR);
            }
            xChild = m_xColumnHeaderBar.get();
        }
        else if(m_aTable.HasRowHeader() && (nChildIndex == 1 || nChildIndex == 0))
        {
            if(!m_xRowHeaderBar.is())
            {
                m_xRowHeaderBar = new AccessibleGridControlHeader(m_aCreator, m_aTable,
                                                                  vcl::table::AccessibleTableControlObjType::ROWHEADERBAR);
            }
            xChild = m_xRowHeaderBar.get();
        }
        else
        {
            if(!m_xTable.is())
            {
                m_xTable = new AccessibleGridControlTable(m_aCreator, m_aTable);
            }
            xChild = m_xTable.get();
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
                VCLRectangle( xCurrChildComp->getBounds() ).Contains( aPoint ) )
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
    return u"com.sun.star.accessibility.AccessibleGridControl"_ustr;
}

// internal virtual methods ---------------------------------------------------

tools::Rectangle AccessibleGridControl::implGetBoundingBox()
{
    vcl::Window* pParent = m_aTable.GetAccessibleParentWindow();
    assert(pParent && "implGetBoundingBox - missing parent window");
    return m_aTable.GetWindowExtentsRelative( *pParent );
}

AbsoluteScreenPixelRectangle AccessibleGridControl::implGetBoundingBoxOnScreen()
{
    return m_aTable.GetWindowExtentsAbsolute();
}

// internal helper methods ----------------------------------------------------

css::uno::Reference< css::accessibility::XAccessible > AccessibleGridControl::implGetTable()
{
    if( !m_xTable.is() )
    {
        m_xTable = createAccessibleTable();
    }
    return m_xTable;
}

css::uno::Reference< css::accessibility::XAccessible >
AccessibleGridControl::implGetHeaderBar( AccessibleTableControlObjType eObjType )
{
    css::uno::Reference< css::accessibility::XAccessible > xRet;
    rtl::Reference< AccessibleGridControlHeader >* pxMember = nullptr;

    if (eObjType == AccessibleTableControlObjType::ROWHEADERBAR)
        pxMember = &m_xRowHeaderBar;
    else if (eObjType ==  AccessibleTableControlObjType::COLUMNHEADERBAR)
        pxMember = &m_xColumnHeaderBar;

    if( pxMember )
    {
        if( !pxMember->is() )
        {
            *pxMember = new AccessibleGridControlHeader(
                m_aCreator, m_aTable, eObjType );
        }
        xRet = pxMember->get();
    }
    return xRet;
}

css::uno::Reference< css::accessibility::XAccessible >
AccessibleGridControl::implGetFixedChild( sal_Int64 nChildIndex )
{
    css::uno::Reference< css::accessibility::XAccessible > xRet;
    switch( nChildIndex )
    {
        /** Child index of the column header bar (first row). */
        case 0:
            xRet = implGetHeaderBar(AccessibleTableControlObjType::COLUMNHEADERBAR);
        break;
        /** Child index of the row header bar ("handle column"). */
        case 1:
            xRet = implGetHeaderBar(AccessibleTableControlObjType::ROWHEADERBAR);
        break;
        /** Child index of the data table. */
        case 2:
            xRet = implGetTable();
        break;
    }
    return xRet;
}

rtl::Reference<AccessibleGridControlTable> AccessibleGridControl::createAccessibleTable()
{
    css::uno::Reference< css::accessibility::XAccessible > xCreator(m_aCreator);
    OSL_ENSURE( xCreator.is(), "extended/AccessibleGridControl::createAccessibleTable: my creator died - how this?" );
    return new AccessibleGridControlTable( xCreator, m_aTable );
}

void AccessibleGridControl::commitCellEvent(sal_Int16 _nEventId,const Any& _rNewValue,const Any& _rOldValue)
{
    sal_Int64 nChildCount = implGetAccessibleChildCount();
    if(nChildCount != 0)
    {
        for(sal_Int64 i=0;i<nChildCount;i++)
        {
            css::uno::Reference< css::accessibility::XAccessible > xAccessible = getAccessibleChild(i);
            if(css::uno::Reference< css::accessibility::XAccessible >(m_xTable) == xAccessible)
            {
                Reference<XAccessible> xCell = m_xTable->getAccessibleCellAt(
                    m_aTable.GetCurrentRow(), m_aTable.GetCurrentColumn());
                AccessibleGridControlTableCell* pCell = static_cast<AccessibleGridControlTableCell*>(xCell.get());
                pCell->commitEvent(_nEventId, _rNewValue, _rOldValue);
            }
        }
    }
    else
    {
        if ( m_xTable.is() )
            m_xTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
    }
}

void AccessibleGridControl::commitTableEvent(sal_Int16 _nEventId,const Any& _rNewValue,const Any& _rOldValue)
{
    if ( !m_xTable.is() )
        return;

    if(_nEventId == AccessibleEventId::ACTIVE_DESCENDANT_CHANGED)
    {
        const sal_Int32 nCurrentRow = m_aTable.GetCurrentRow();
        const sal_Int32 nCurrentCol = m_aTable.GetCurrentColumn();
        css::uno::Reference< css::accessibility::XAccessible > xChild;
        if (nCurrentRow > -1 && nCurrentCol > -1)
            xChild = m_xTable->getAccessibleCellAt(nCurrentRow, nCurrentCol);

        m_xTable->commitEvent(_nEventId, Any(xChild),_rOldValue);
    }
    else
        m_xTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
}

// = AccessibleGridControlAccess


AccessibleGridControlAccess::AccessibleGridControlAccess(
        css::uno::Reference< css::accessibility::XAccessible > xParent, ::vcl::table::IAccessibleTable& rTable )
    : m_xParent(std::move( xParent ))
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
    // then reset and re-create.
    if ( m_xContext.is() && !m_xContext->isAlive() )
        m_xContext = nullptr;

    if (!m_xContext.is() && m_pTable)
        m_xContext = new AccessibleGridControl(m_xParent, this, *m_pTable);

    return m_xContext;
}


}   // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
