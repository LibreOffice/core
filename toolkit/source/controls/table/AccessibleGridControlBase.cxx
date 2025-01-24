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

#include <controls/table/AccessibleGridControlBase.hxx>

#include <utility>
#include <vcl/accessibletable.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/window.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/types.h>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <unotools/accessiblerelationsethelper.hxx>
#include <sal/log.hxx>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;
using namespace ::vcl;
using namespace ::vcl::table;


namespace accessibility {

using namespace com::sun::star::accessibility::AccessibleStateType;

AccessibleGridControlBase::AccessibleGridControlBase(
    css::uno::Reference<css::accessibility::XAccessible> xParent, svt::table::TableControl& rTable,
    ::vcl::table::AccessibleTableControlObjType eObjType)
    : m_xParent(std::move(xParent))
    , m_aTable(rTable)
    , m_eObjType(eObjType)
{
}

void SAL_CALL AccessibleGridControlBase::disposing()
{
    SolarMutexGuard g;

    OAccessibleComponentHelper::disposing();

    m_xParent = nullptr;
    //m_aTable = NULL;
}

// css::accessibility::XAccessibleContext

css::uno::Reference< css::accessibility::XAccessible > SAL_CALL AccessibleGridControlBase::getAccessibleParent()
{
    SolarMutexGuard g;

    ensureAlive();
    return m_xParent;
}

sal_Int64 SAL_CALL AccessibleGridControlBase::getAccessibleIndexInParent()
{
    SolarMutexGuard g;

    ensureAlive();

    // -1 for child not found/no parent (according to specification)
    sal_Int64 nRet = -1;

    css::uno::Reference< uno::XInterface > xMeMyselfAndI( static_cast< css::accessibility::XAccessibleContext* >( this ), uno::UNO_QUERY );

    //  iterate over parent's children and search for this object
    if( m_xParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext >
            xParentContext( m_xParent->getAccessibleContext() );
        if( xParentContext.is() )
        {
            css::uno::Reference< uno::XInterface > xChild;

            sal_Int64 nChildCount = xParentContext->getAccessibleChildCount();
            for( sal_Int64 nChild = 0; nChild < nChildCount; ++nChild )
            {
                xChild.set(xParentContext->getAccessibleChild( nChild ), css::uno::UNO_QUERY);
                if ( xMeMyselfAndI.get() == xChild.get() )
                {
                    nRet = nChild;
                    break;
                }
            }
        }
    }
    return nRet;
}

OUString SAL_CALL AccessibleGridControlBase::getAccessibleDescription()
{
    SolarMutexGuard g;

    ensureAlive();
    return m_aTable.GetAccessibleObjectDescription(m_eObjType);
}

OUString SAL_CALL AccessibleGridControlBase::getAccessibleName()
{
    SolarMutexGuard g;

    ensureAlive();
    return m_aTable.GetAccessibleObjectName(m_eObjType, 0, 0);
}

css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
AccessibleGridControlBase::getAccessibleRelationSet()
{
   SolarMutexGuard g;

   ensureAlive();
   // GridControl does not have relations.
   return new utl::AccessibleRelationSetHelper;
}

sal_Int64 SAL_CALL
AccessibleGridControlBase::getAccessibleStateSet()
{
    SolarMutexGuard aSolarGuard;

    // don't check whether alive -> StateSet may contain DEFUNC
    return implCreateStateSet();
}

lang::Locale SAL_CALL AccessibleGridControlBase::getLocale()
{
    SolarMutexGuard g;

    ensureAlive();
    if( m_xParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext >
            xParentContext( m_xParent->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }
    throw IllegalAccessibleComponentStateException();
}

// XTypeProvider

Sequence< sal_Int8 > SAL_CALL AccessibleGridControlBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo

sal_Bool SAL_CALL AccessibleGridControlBase::supportsService(
        const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL AccessibleGridControlBase::getSupportedServiceNames()
{
    return { u"com.sun.star.accessibility.AccessibleContext"_ustr };
}
// internal virtual methods

bool AccessibleGridControlBase::implIsShowing()
{
    bool bShowing = false;
    if( m_xParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleComponent >
            xParentComp( m_xParent->getAccessibleContext(), uno::UNO_QUERY );
        if( xParentComp.is() )
            bShowing = implGetBoundingBox().Overlaps(
                vcl::unohelper::ConvertToVCLRect(xParentComp->getBounds()));
    }
    return bShowing;
}

tools::Rectangle AccessibleGridControlBase::implGetBoundingBox()
{
    // calculate parent-relative position from own and parent's absolute position
    tools::Rectangle aBound(implGetBoundingBoxOnScreen());
    if (!m_xParent.is())
        return aBound;

    uno::Reference<css::accessibility::XAccessibleComponent> xParentComponent(m_xParent->getAccessibleContext(), uno::UNO_QUERY);
    if (!xParentComponent.is())
        return aBound;

    awt::Point aParentPos = xParentComponent->getLocationOnScreen();
    aBound.Move(-aParentPos.X, -aParentPos.Y);
    return aBound;
}

sal_Int64 AccessibleGridControlBase::implCreateStateSet()
{
    sal_Int64 nStateSet = 0;

    if( isAlive() )
    {
        // SHOWING done with m_xParent
        if( implIsShowing() )
            nStateSet |= AccessibleStateType::SHOWING;
        // GridControl fills StateSet with states depending on object type
        m_aTable.FillAccessibleStateSet( nStateSet, getType() );
    }
    else
        nStateSet |= AccessibleStateType::DEFUNC;
    return nStateSet;
}

css::awt::Rectangle AccessibleGridControlBase::implGetBounds()
{
    return vcl::unohelper::ConvertToAWTRect(implGetBoundingBox());
}

void AccessibleGridControlBase::commitEvent(
        sal_Int16 _nEventId, const Any& _rNewValue, const Any& _rOldValue )
{
    NotifyAccessibleEvent(_nEventId, _rOldValue, _rNewValue);
}

sal_Int16 SAL_CALL AccessibleGridControlBase::getAccessibleRole()
{
    ensureAlive();
    sal_Int16 nRole = AccessibleRole::UNKNOWN;
    switch ( m_eObjType )
    {
        case AccessibleTableControlObjType::ROWHEADERCELL:
        nRole = AccessibleRole::ROW_HEADER;
        break;
    case AccessibleTableControlObjType::COLUMNHEADERCELL:
        nRole = AccessibleRole::COLUMN_HEADER;
        break;
    case AccessibleTableControlObjType::COLUMNHEADERBAR:
    case AccessibleTableControlObjType::ROWHEADERBAR:
    case AccessibleTableControlObjType::TABLE:
        nRole = AccessibleRole::TABLE;
        break;
    case AccessibleTableControlObjType::TABLECELL:
        nRole = AccessibleRole::TABLE_CELL;
        break;
    case AccessibleTableControlObjType::GRIDCONTROL:
        nRole = AccessibleRole::PANEL;
        break;
    }
    return nRole;
}

css::uno::Reference<css::accessibility::XAccessible > SAL_CALL AccessibleGridControlBase::getAccessibleAtPoint( const css::awt::Point& )
{
    return nullptr;
}

sal_Int32 SAL_CALL AccessibleGridControlBase::getForeground(  )
{
    SolarMutexGuard aSolarGuard;

    ensureAlive();

    Color nColor;
    if (m_aTable.IsControlForeground())
        nColor = m_aTable.GetControlForeground();
    else
    {
        vcl::Font aFont;
        if (m_aTable.IsControlFont() )
            aFont = m_aTable.GetControlFont();
        else
            aFont = m_aTable.GetFont();
        nColor = aFont.GetColor();
    }
    return sal_Int32(nColor);
}

sal_Int32 SAL_CALL AccessibleGridControlBase::getBackground(  )
{
    SolarMutexGuard aSolarGuard;

    ensureAlive();
    Color nColor;
    if (m_aTable.IsControlBackground())
        nColor = m_aTable.GetControlBackground();
    else
        nColor = m_aTable.GetBackground().GetColor();
    return sal_Int32(nColor);
}


GridControlAccessibleElement::GridControlAccessibleElement( const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
                        svt::table::TableControl& rTable,
                        ::vcl::table::AccessibleTableControlObjType  eObjType )
    : GridControlAccessibleElement_Base(rxParent, rTable, eObjType)
{
}

// css::accessibility::XAccessible

css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL GridControlAccessibleElement::getAccessibleContext()
{
    SolarMutexGuard g;

    ensureAlive();
    return this;
}

GridControlAccessibleElement::~GridControlAccessibleElement( )
{
}


} // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
