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
#include "CharacterProperties.hxx"
#include "ObjectIdentifier.hxx"
#include "ObjectNameProvider.hxx"
#include "servicenames.hxx"
#include "macros.hxx"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;

namespace chart
{

AccessibleChartElement::AccessibleChartElement(
    const AccessibleElementInfo & rAccInfo,
    bool bMayHaveChildren ) :
        impl::AccessibleChartElement_Base( rAccInfo, bMayHaveChildren, false/*bAlwaysTransparent*/ ),
        m_bHasText( false )
{
    AddState( AccessibleStateType::TRANSIENT );
}

AccessibleChartElement::~AccessibleChartElement()
{
    OSL_ASSERT( CheckDisposeState( false /* don't throw exceptions */ ) );
}

// ________ protected ________

bool AccessibleChartElement::ImplUpdateChildren()
{
    bool bResult = false;
    Reference< chart2::XTitle > xTitle(
        ObjectIdentifier::getObjectPropertySet(
            GetInfo().m_aOID.getObjectCID(), Reference< chart2::XChartDocument >( GetInfo().m_xChartDocument )),
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
        Reference< view::XSelectionSupplier > xSelSupp( GetInfo().m_xSelectionSupplier );
        // get factory from selection supplier (controller)
        Reference< lang::XMultiServiceFactory > xFact( xSelSupp, uno::UNO_QUERY );
        if( xFact.is())
        {
            m_xTextHelper.set(
                xFact->createInstance( CHART_ACCESSIBLE_TEXT_SERVICE_NAME ), uno::UNO_QUERY );
        }
    }

    if( m_xTextHelper.is())
        try
        {
            Reference< lang::XInitialization > xInit( m_xTextHelper, uno::UNO_QUERY_THROW );
            Sequence< uno::Any > aArgs( 3 );
            aArgs[0] <<= GetInfo().m_aOID.getObjectCID();
            aArgs[1] <<= Reference< XAccessible >( this );
            aArgs[2] <<= Reference< awt::XWindow >( GetInfo().m_xWindow );
            xInit->initialize( aArgs );
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
}

//             Interfaces

// ________ AccessibleBase::XAccessibleContext ________
Reference< XAccessible > AccessibleChartElement::ImplGetAccessibleChildById( sal_Int32 i ) const
    throw (lang::IndexOutOfBoundsException, RuntimeException)
{
    Reference< XAccessible > xResult;

    if( m_bHasText )
        xResult.set( m_xTextHelper->getAccessibleChild( i ));
    else
        xResult.set( AccessibleBase::ImplGetAccessibleChildById( i ));

    return xResult;
}

sal_Int32 AccessibleChartElement::ImplGetAccessibleChildCount() const
    throw (RuntimeException)
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
    throw (RuntimeException, std::exception)
{
    return OUString( "AccessibleChartElement" );
}

// ________ AccessibleChartElement::XAccessibleContext (override) ________
OUString SAL_CALL AccessibleChartElement::getAccessibleName()
    throw (css::uno::RuntimeException, std::exception)
{
    return ObjectNameProvider::getNameForCID(
        GetInfo().m_aOID.getObjectCID(), GetInfo().m_xChartDocument );
}

// ________ AccessibleChartElement::XAccessibleContext (override) ________
OUString SAL_CALL AccessibleChartElement::getAccessibleDescription()
    throw (css::uno::RuntimeException, std::exception)
{
    return getToolTipText();
}

// ________ AccessibleChartElement::XAccessibleExtendedComponent ________
Reference< awt::XFont > SAL_CALL AccessibleChartElement::getFont()
    throw (uno::RuntimeException, std::exception)
{
    CheckDisposeState();

    Reference< awt::XFont > xFont;
    // using assignment for broken gcc 3.3
    Reference< awt::XDevice > xDevice( Reference< awt::XWindow >( GetInfo().m_xWindow ), uno::UNO_QUERY );

    if( xDevice.is())
    {
        Reference< beans::XMultiPropertySet > xObjProp(
            ObjectIdentifier::getObjectPropertySet(
                GetInfo().m_aOID.getObjectCID(), Reference< chart2::XChartDocument >( GetInfo().m_xChartDocument )), uno::UNO_QUERY );
        awt::FontDescriptor aDescr(
            CharacterProperties::createFontDescriptorFromPropertySet( xObjProp ));
        xFont = xDevice->getFont( aDescr );
    }

    return xFont;
}

OUString SAL_CALL AccessibleChartElement::getTitledBorderText()
    throw (uno::RuntimeException, std::exception)
{
    return OUString();
}

OUString SAL_CALL AccessibleChartElement::getToolTipText()
    throw (css::uno::RuntimeException, std::exception)
{
    CheckDisposeState();

    return ObjectNameProvider::getHelpText(
        GetInfo().m_aOID.getObjectCID(), Reference< chart2::XChartDocument >( GetInfo().m_xChartDocument ));
}

// ________ XAccessibleComponent ________
sal_Bool SAL_CALL AccessibleChartElement::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException, std::exception)
{
    return AccessibleBase::containsPoint( aPoint );
}

Reference< XAccessible > SAL_CALL AccessibleChartElement::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException, std::exception)
{
    return AccessibleBase::getAccessibleAtPoint( aPoint );
}

awt::Rectangle SAL_CALL AccessibleChartElement::getBounds()
    throw (uno::RuntimeException, std::exception)
{
    return AccessibleBase::getBounds();
}

awt::Point SAL_CALL AccessibleChartElement::getLocation()
    throw (uno::RuntimeException, std::exception)
{
    return AccessibleBase::getLocation();
}

awt::Point SAL_CALL AccessibleChartElement::getLocationOnScreen()
    throw (uno::RuntimeException, std::exception)
{
    return AccessibleBase::getLocationOnScreen();
}

awt::Size SAL_CALL AccessibleChartElement::getSize()
    throw (uno::RuntimeException, std::exception)
{
    return AccessibleBase::getSize();
}

void SAL_CALL AccessibleChartElement::grabFocus()
    throw (uno::RuntimeException, std::exception)
{
    return AccessibleBase::grabFocus();
}

sal_Int32 SAL_CALL AccessibleChartElement::getForeground()
    throw (uno::RuntimeException, std::exception)
{
    return AccessibleBase::getForeground();
}

sal_Int32 SAL_CALL AccessibleChartElement::getBackground()
    throw (uno::RuntimeException, std::exception)
{
    return AccessibleBase::getBackground();
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
