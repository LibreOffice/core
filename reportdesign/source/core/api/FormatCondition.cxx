/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"
#include "FormatCondition.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "corestrings.hrc"
#include <tools/debug.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/property.hxx>
#include "Tools.hxx"
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include "ReportHelperImpl.hxx"
#include "corestrings.hrc"
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
//------------------------------------------------------------------------------
uno::Reference< uno::XInterface > OFormatCondition::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OFormatCondition(xContext));
}

DBG_NAME( rpt_OFormatCondition )
// -----------------------------------------------------------------------------
OFormatCondition::OFormatCondition(uno::Reference< uno::XComponentContext > const & _xContext)
:FormatConditionBase(m_aMutex)
,FormatConditionPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< ::rtl::OUString >())
,m_bEnabled(sal_True)
{
    DBG_CTOR( rpt_OFormatCondition,NULL);
}
// -----------------------------------------------------------------------------
OFormatCondition::~OFormatCondition()
{
    DBG_DTOR( rpt_OFormatCondition,NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(OFormatCondition,FormatConditionBase,FormatConditionPropertySet)
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::dispose() throw(uno::RuntimeException)
{
    FormatConditionPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}
// -----------------------------------------------------------------------------
::rtl::OUString OFormatCondition::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.OFormatCondition"));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormatCondition::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > OFormatCondition::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = SERVICE_FORMATCONDITION;

    return aServices;
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OFormatCondition::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OFormatCondition::supportsService(const ::rtl::OUString& ServiceName) throw( uno::RuntimeException )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OFormatCondition::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return FormatConditionPropertySet::getPropertySetInfo();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    FormatConditionPropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OFormatCondition::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return FormatConditionPropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FormatConditionPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FormatConditionPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FormatConditionPropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    FormatConditionPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
// XFormatCondition
::sal_Bool SAL_CALL OFormatCondition::getEnabled() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_bEnabled;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::setEnabled( ::sal_Bool _enabled ) throw (uno::RuntimeException)
{
    set(PROPERTY_ENABLED,_enabled,m_bEnabled);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormatCondition::getFormula() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_sFormula;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFormatCondition::setFormula( const ::rtl::OUString& _formula ) throw (uno::RuntimeException)
{
    set(PROPERTY_FORMULA,_formula,m_sFormula);
}
// -----------------------------------------------------------------------------
// XReportControlFormat
REPORTCONTROLFORMAT_IMPL(OFormatCondition,m_aFormatProperties)
// =============================================================================
} // namespace reportdesign
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
