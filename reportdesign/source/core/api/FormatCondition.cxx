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
#include "FormatCondition.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "corestrings.hrc"
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "Tools.hxx"
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include "ReportHelperImpl.hxx"

namespace reportdesign
{

    using namespace com::sun::star;
    using namespace comphelper;

uno::Reference< uno::XInterface > OFormatCondition::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFormatCondition(xContext));
}


OFormatCondition::OFormatCondition(uno::Reference< uno::XComponentContext > const & _xContext)
:FormatConditionBase(m_aMutex)
,FormatConditionPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< OUString >())
,m_bEnabled(true)
{
}

OFormatCondition::~OFormatCondition()
{
}

IMPLEMENT_FORWARD_XINTERFACE2(OFormatCondition,FormatConditionBase,FormatConditionPropertySet)

void SAL_CALL OFormatCondition::dispose() throw(uno::RuntimeException, std::exception)
{
    FormatConditionPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}

OUString OFormatCondition::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return OUString("com.sun.star.comp.report.OFormatCondition");
}


OUString SAL_CALL OFormatCondition::getImplementationName(  ) throw(uno::RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

uno::Sequence< OUString > OFormatCondition::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aServices { SERVICE_FORMATCONDITION };

    return aServices;
}

uno::Sequence< OUString > SAL_CALL OFormatCondition::getSupportedServiceNames(  ) throw(uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

sal_Bool SAL_CALL OFormatCondition::supportsService(const OUString& ServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OFormatCondition::getPropertySetInfo(  ) throw(uno::RuntimeException, std::exception)
{
    return FormatConditionPropertySet::getPropertySetInfo();
}

void SAL_CALL OFormatCondition::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FormatConditionPropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OFormatCondition::getPropertyValue( const OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    return FormatConditionPropertySet::getPropertyValue( PropertyName);
}

void SAL_CALL OFormatCondition::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FormatConditionPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OFormatCondition::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FormatConditionPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OFormatCondition::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FormatConditionPropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OFormatCondition::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    FormatConditionPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

// XFormatCondition
sal_Bool SAL_CALL OFormatCondition::getEnabled() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bEnabled;
}

void SAL_CALL OFormatCondition::setEnabled( sal_Bool _enabled ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_ENABLED,_enabled,m_bEnabled);
}

OUString SAL_CALL OFormatCondition::getFormula() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_sFormula;
}

void SAL_CALL OFormatCondition::setFormula( const OUString& _formula ) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_FORMULA,_formula,m_sFormula);
}

// XReportControlFormat
REPORTCONTROLFORMAT_IMPL(OFormatCondition,m_aFormatProperties)

} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
