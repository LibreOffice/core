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
#include <Function.hxx>
#include <strings.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace reportdesign
{

    using namespace com::sun::star;

uno::Reference< uno::XInterface > OFunction::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFunction(xContext));
}


OFunction::OFunction(uno::Reference< uno::XComponentContext > const & _xContext)
:FunctionBase(m_aMutex)
,FunctionPropertySet(_xContext,IMPLEMENTS_PROPERTY_SET,uno::Sequence< OUString >())
,m_bPreEvaluated(false)
,m_bDeepTraversing(false)
{
    m_sInitialFormula.IsPresent = false;
}

OFunction::~OFunction()
{
}

IMPLEMENT_FORWARD_XINTERFACE2(OFunction,FunctionBase,FunctionPropertySet)

void SAL_CALL OFunction::dispose()
{
    FunctionPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}

OUString OFunction::getImplementationName_Static(  )
{
    return u"com.sun.star.comp.report.OFunction"_ustr;
}


OUString SAL_CALL OFunction::getImplementationName(  )
{
    return getImplementationName_Static();
}

uno::Sequence< OUString > OFunction::getSupportedServiceNames_Static(  )
{
    uno::Sequence< OUString > aServices { SERVICE_FUNCTION };

    return aServices;
}

uno::Sequence< OUString > SAL_CALL OFunction::getSupportedServiceNames(  )
{
    return getSupportedServiceNames_Static();
}

sal_Bool SAL_CALL OFunction::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OFunction::getPropertySetInfo(  )
{
    return FunctionPropertySet::getPropertySetInfo();
}

void SAL_CALL OFunction::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    FunctionPropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OFunction::getPropertyValue( const OUString& PropertyName )
{
    return FunctionPropertySet::getPropertyValue( PropertyName);
}

void SAL_CALL OFunction::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
{
    FunctionPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OFunction::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
{
    FunctionPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OFunction::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    FunctionPropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OFunction::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    FunctionPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

// report::XFunction:
sal_Bool SAL_CALL OFunction::getPreEvaluated()
{
    osl::MutexGuard g(m_aMutex);
    return m_bPreEvaluated;
}


void SAL_CALL OFunction::setPreEvaluated(sal_Bool the_value)
{
    set(PROPERTY_PREEVALUATED,the_value,m_bPreEvaluated);
}

sal_Bool SAL_CALL OFunction::getDeepTraversing()
{
    osl::MutexGuard g(m_aMutex);
    return m_bDeepTraversing;
}


void SAL_CALL OFunction::setDeepTraversing(sal_Bool the_value)
{
    set(PROPERTY_DEEPTRAVERSING,the_value,m_bDeepTraversing);
}


OUString SAL_CALL OFunction::getName()
{
    osl::MutexGuard g(m_aMutex);
    return m_sName;
}


void SAL_CALL OFunction::setName(const OUString & the_value)
{
    set(PROPERTY_NAME,the_value,m_sName);
}

OUString SAL_CALL OFunction::getFormula()
{
    osl::MutexGuard g(m_aMutex);
    return m_sFormula;
}

void SAL_CALL OFunction::setFormula(const OUString & the_value)
{
    set(PROPERTY_FORMULA,the_value,m_sFormula);
}

beans::Optional< OUString> SAL_CALL OFunction::getInitialFormula()
{
    osl::MutexGuard g(m_aMutex);
    return m_sInitialFormula;
}

void SAL_CALL OFunction::setInitialFormula(const beans::Optional< OUString> & the_value)
{
    set(PROPERTY_INITIALFORMULA,the_value,m_sInitialFormula);
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL OFunction::getParent(  )
{
    osl::MutexGuard g(m_aMutex);
    return m_xParent;
}

void SAL_CALL OFunction::setParent( const uno::Reference< uno::XInterface >& Parent )
{
    osl::MutexGuard g(m_aMutex);
    if ( Parent.is() )
    {
        uno::Reference< report::XFunctions> xFunctions(Parent,uno::UNO_QUERY_THROW);
        m_xParent = xFunctions;
    }
    else
        m_xParent.clear();
}


} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
