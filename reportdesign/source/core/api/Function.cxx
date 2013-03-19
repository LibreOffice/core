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
#include "Function.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "corestrings.hrc"
#include <tools/debug.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/property.hxx>
#include "Tools.hxx"
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
//------------------------------------------------------------------------------
uno::Reference< uno::XInterface > OFunction::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFunction(xContext));
}

DBG_NAME( rpt_OFunction )
// -----------------------------------------------------------------------------
OFunction::OFunction(uno::Reference< uno::XComponentContext > const & _xContext)
:FunctionBase(m_aMutex)
,FunctionPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< ::rtl::OUString >())
,m_xContext(_xContext)
,m_bPreEvaluated(sal_False)
,m_bDeepTraversing(sal_False)
{
    m_sInitialFormula.IsPresent = sal_False;
    DBG_CTOR( rpt_OFunction,NULL);
}
// -----------------------------------------------------------------------------
OFunction::~OFunction()
{
    DBG_DTOR( rpt_OFunction,NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(OFunction,FunctionBase,FunctionPropertySet)
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::dispose() throw(uno::RuntimeException)
{
    FunctionPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}
// -----------------------------------------------------------------------------
::rtl::OUString OFunction::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString("com.sun.star.comp.report.OFunction");
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFunction::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > OFunction::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = SERVICE_FUNCTION;

    return aServices;
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OFunction::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OFunction::supportsService(const ::rtl::OUString& ServiceName) throw( uno::RuntimeException )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OFunction::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return FunctionPropertySet::getPropertySetInfo();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    FunctionPropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OFunction::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return FunctionPropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FunctionPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FunctionPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FunctionPropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FunctionPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
// report::XFunction:
::sal_Bool SAL_CALL OFunction::getPreEvaluated() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_bPreEvaluated;
}
// -----------------------------------------------------------------------------

void SAL_CALL OFunction::setPreEvaluated(::sal_Bool the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_PREEVALUATED,the_value,m_bPreEvaluated);
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OFunction::getDeepTraversing() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_bDeepTraversing;
}
// -----------------------------------------------------------------------------

void SAL_CALL OFunction::setDeepTraversing(::sal_Bool the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_DEEPTRAVERSING,the_value,m_bDeepTraversing);
}
// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL OFunction::getName() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_sName;
}
// -----------------------------------------------------------------------------

void SAL_CALL OFunction::setName(const ::rtl::OUString & the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_NAME,the_value,m_sName);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFunction::getFormula() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_sFormula;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::setFormula(const ::rtl::OUString & the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_FORMULA,the_value,m_sFormula);
}
// -----------------------------------------------------------------------------
beans::Optional< ::rtl::OUString> SAL_CALL OFunction::getInitialFormula() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_sInitialFormula;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::setInitialFormula(const beans::Optional< ::rtl::OUString> & the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_INITIALFORMULA,the_value,m_sInitialFormula);
}
// -----------------------------------------------------------------------------
// XChild
uno::Reference< uno::XInterface > SAL_CALL OFunction::getParent(  ) throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_xParent;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunction::setParent( const uno::Reference< uno::XInterface >& Parent ) throw (lang::NoSupportException, uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    if ( Parent.is() )
    {
        uno::Reference< report::XFunctions> xFunctions(Parent,uno::UNO_QUERY_THROW);
        m_xParent = xFunctions;
    }
    else
        m_xParent = uno::WeakReference< report::XFunctions >();
}
// -----------------------------------------------------------------------------

// =============================================================================
} // namespace reportdesign
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
