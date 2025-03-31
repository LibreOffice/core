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

#include "AccessibleChartElement.hxx"
#include <AccessibleTextHelper.hxx>
#include <CharacterProperties.hxx>
#include <ChartModel.hxx>
#include <ChartController.hxx>
#include <ObjectIdentifier.hxx>
#include <ObjectNameProvider.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;

namespace chart
{

AccessibleChartElement::AccessibleChartElement(
    const AccessibleElementInfo & rAccInfo,
    bool bMayHaveChildren ) :
        AccessibleBase(rAccInfo, bMayHaveChildren, false/*bAlwaysTransparent*/),
        m_bHasText( false )
{
    AddState( AccessibleStateType::TRANSIENT );
}

AccessibleChartElement::~AccessibleChartElement()
{
    OSL_ASSERT(!isAlive());
}

// ________ protected ________

bool AccessibleChartElement::ImplUpdateChildren()
{
    bool bResult = false;
    Reference< chart2::XTitle > xTitle(
        ObjectIdentifier::getObjectPropertySet(
            GetInfo().m_aOID.getObjectCID(), GetInfo().m_xChartDocument ),
        uno::UNO_QUERY );
    m_bHasText = xTitle.is();

    if( m_bHasText )
    {
        InitTextEdit();
        bResult = true;
    }
    else
        bResult = AccessibleBase::ImplUpdateChildren();

    return bResult;
}

void AccessibleChartElement::InitTextEdit()
{
    if( ! m_xTextHelper.is())
    {
        // get hard reference
        rtl::Reference< ::chart::ChartController > xChartController( GetInfo().m_xChartController );
        if( xChartController.is())
            m_xTextHelper = xChartController->createAccessibleTextContext();
    }

    if( !m_xTextHelper.is())
        return;

    m_xTextHelper->initialize(GetInfo().m_aOID.getObjectCID(), this, GetInfo().m_pWindow);
}

//             Interfaces

// ________ AccessibleBase::XAccessibleContext ________
Reference< XAccessible > AccessibleChartElement::ImplGetAccessibleChildById( sal_Int64 i ) const
{
    Reference< XAccessible > xResult;

    if( m_bHasText )
        xResult.set( m_xTextHelper->getAccessibleChild( i ));
    else
        xResult.set( AccessibleBase::ImplGetAccessibleChildById( i ));

    return xResult;
}

sal_Int64 AccessibleChartElement::ImplGetAccessibleChildCount() const
{
    if( m_bHasText )
    {
        if( m_xTextHelper.is())
            return m_xTextHelper->getAccessibleChildCount();
        return 0;
    }

    return AccessibleBase::ImplGetAccessibleChildCount();
}

// ________ XServiceInfo ________
OUString SAL_CALL AccessibleChartElement::getImplementationName()
{
    return u"AccessibleChartElement"_ustr;
}

// ________ AccessibleChartElement::XAccessibleContext (override) ________
OUString SAL_CALL AccessibleChartElement::getAccessibleName()
{
    return ObjectNameProvider::getNameForCID(
        GetInfo().m_aOID.getObjectCID(), GetInfo().m_xChartDocument );
}

// ________ AccessibleChartElement::XAccessibleContext (override) ________
OUString SAL_CALL AccessibleChartElement::getAccessibleDescription()
{
    return getToolTipText();
}

// ________ AccessibleChartElement::XAccessibleExtendedComponent ________

OUString SAL_CALL AccessibleChartElement::getToolTipText()
{
    ensureAlive();

    return ObjectNameProvider::getHelpText(
        GetInfo().m_aOID.getObjectCID(), GetInfo().m_xChartDocument );
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
