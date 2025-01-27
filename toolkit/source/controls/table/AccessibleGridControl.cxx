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

#include <controls/table/AccessibleGridControl.hxx>
#include <controls/table/AccessibleGridControlTable.hxx>
#include <controls/table/AccessibleGridControlHeader.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>

namespace accessibility
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::vcl;

AccessibleGridControl::AccessibleGridControl(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent, const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator,
            svt::table::TableControl& _rTable )
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
    ensureAlive();
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
                                                                     AccessibleTableControlObjType::COLUMNHEADERBAR);
            }
            xChild = m_xColumnHeaderBar.get();
        }
        else if(m_aTable.HasRowHeader() && (nChildIndex == 1 || nChildIndex == 0))
        {
            if(!m_xRowHeaderBar.is())
            {
                m_xRowHeaderBar = new AccessibleGridControlHeader(m_aCreator, m_aTable,
                                                                  AccessibleTableControlObjType::ROWHEADERBAR);
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

    ensureAlive();
    return AccessibleRole::PANEL;
}


// css::accessibility::XAccessibleComponent -------------------------------------------------------

css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
AccessibleGridControl::getAccessibleAtPoint( const awt::Point& rPoint )
{
    SolarMutexGuard aSolarGuard;
    ensureAlive();

    // try whether point is in one of the fixed children
    // (table, header bars, corner control)
    Point aPoint(vcl::unohelper::ConvertToVCLPoint(rPoint));
    const sal_Int64 nChildCount = implGetAccessibleChildCount();
    for (sal_Int64 nIndex = 0; nIndex < nChildCount; ++nIndex)
    {
        css::uno::Reference<css::accessibility::XAccessible> xCurrChild = getAccessibleChild(nIndex);
        css::uno::Reference<css::accessibility::XAccessibleComponent> xCurrChildComp(
            xCurrChild, uno::UNO_QUERY);

        if (xCurrChildComp.is()
            && vcl::unohelper::ConvertToVCLRect(xCurrChildComp->getBounds()).Contains(aPoint))
            return xCurrChild;
    }
    return nullptr;
}


void SAL_CALL AccessibleGridControl::grabFocus()
{
    SolarMutexGuard aSolarGuard;
    ensureAlive();
    m_aTable.GrabFocus();
}

// XServiceInfo ---------------------------------------------------------------
OUString SAL_CALL AccessibleGridControl::getImplementationName()
{
    return u"com.sun.star.accessibility.AccessibleGridControl"_ustr;
}

// internal virtual methods ---------------------------------------------------

AbsoluteScreenPixelRectangle AccessibleGridControl::implGetBoundingBoxOnScreen()
{
    return m_aTable.GetWindowExtentsAbsolute();
}

// internal helper methods ----------------------------------------------------

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
        css::uno::Reference<css::accessibility::XAccessible> xParent, svt::table::TableControl& rTable )
    : m_xParent(std::move( xParent ))
    , m_xTable(& rTable)
{
}


AccessibleGridControlAccess::~AccessibleGridControlAccess()
{
}


void AccessibleGridControlAccess::DisposeAccessImpl()
{
    SolarMutexGuard g;

    m_xTable.clear();
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

    if (!m_xContext.is() && m_xTable)
        m_xContext = new AccessibleGridControl(m_xParent, this, *m_xTable);

    return m_xContext;
}


}   // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
