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
#include "ReportComponentHandler.hxx"
#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/sequence.hxx>
#include "uistrings.hrc"
#include <comphelper/types.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/syslocale.hxx>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/report/XSection.hpp>
#include <com/sun/star/inspection/XNumericControl.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <tools/fldunit.hxx>
#include "metadata.hxx"

//........................................................................
namespace rptui
{
//........................................................................
using namespace ::com::sun::star;

ReportComponentHandler::ReportComponentHandler(uno::Reference< uno::XComponentContext > const & context)
    :ReportComponentHandler_Base(m_aMutex)
    ,m_xContext(context)
    ,m_pInfoService( new OPropertyInfoService() )
{
    try
    {
        m_xFormComponentHandler.set(m_xContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.inspection.FormComponentPropertyHandler")),m_xContext),uno::UNO_QUERY_THROW);

    }catch(const uno::Exception &)
    {
    }
}

//------------------------------------------------------------------------
::rtl::OUString SAL_CALL ReportComponentHandler::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------
sal_Bool SAL_CALL ReportComponentHandler::supportsService( const ::rtl::OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_static());
}

//------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL ReportComponentHandler::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

//------------------------------------------------------------------------
::rtl::OUString ReportComponentHandler::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.ReportComponentHandler"));
}

//------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > ReportComponentHandler::getSupportedServiceNames_static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.inspection.ReportComponentHandler"));
    return aSupported;
}

//------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL ReportComponentHandler::create( const uno::Reference< uno::XComponentContext >& _rxContext )
{
    return *(new ReportComponentHandler( _rxContext ));
}
// overload WeakComponentImplHelperBase::disposing()
// This function is called upon disposing the component,
// if your component needs special work when it becomes
// disposed, do it here.
void SAL_CALL ReportComponentHandler::disposing()
{
    ::comphelper::disposeComponent(m_xFormComponentHandler);
}
void SAL_CALL ReportComponentHandler::addEventListener(const uno::Reference< lang::XEventListener > & xListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->addEventListener(xListener);
}

void SAL_CALL ReportComponentHandler::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->removeEventListener(aListener);
}

// inspection::XPropertyHandler:

/********************************************************************************/
void SAL_CALL ReportComponentHandler::inspect(const uno::Reference< uno::XInterface > & Component) throw (uno::RuntimeException, lang::NullPointerException)
{
    try
    {
        uno::Reference< container::XNameContainer > xNameCont(Component,uno::UNO_QUERY);
        const ::rtl::OUString sFormComponent(RTL_CONSTASCII_USTRINGPARAM("FormComponent"));
        if ( xNameCont->hasByName(sFormComponent) )
            xNameCont->getByName(sFormComponent) >>= m_xFormComponent;
        const ::rtl::OUString sRowSet(RTL_CONSTASCII_USTRINGPARAM("RowSet"));
        if ( xNameCont->hasByName(sRowSet) )
        {
            uno::Reference<beans::XPropertySet> xProp(m_xFormComponentHandler,uno::UNO_QUERY);
            xProp->setPropertyValue(sRowSet,xNameCont->getByName(sRowSet));
        }
    }
    catch(const uno::Exception &)
    {
        throw lang::NullPointerException();
    }
    if ( m_xFormComponent.is() )
    {
        m_xFormComponentHandler->inspect(m_xFormComponent);
    }
}

uno::Any SAL_CALL ReportComponentHandler::getPropertyValue(const ::rtl::OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_xFormComponentHandler->getPropertyValue(PropertyName);
}

void SAL_CALL ReportComponentHandler::setPropertyValue(const ::rtl::OUString & PropertyName, const uno::Any & Value) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    m_xFormComponentHandler->setPropertyValue(PropertyName, Value);
}

beans::PropertyState SAL_CALL ReportComponentHandler::getPropertyState(const ::rtl::OUString & PropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_xFormComponentHandler->getPropertyState(PropertyName);
}

inspection::LineDescriptor SAL_CALL ReportComponentHandler::describePropertyLine(const ::rtl::OUString & PropertyName,  const uno::Reference< inspection::XPropertyControlFactory > & ControlFactory) throw (beans::UnknownPropertyException, lang::NullPointerException,uno::RuntimeException)
{
    return m_xFormComponentHandler->describePropertyLine(PropertyName, ControlFactory);
}

uno::Any SAL_CALL ReportComponentHandler::convertToPropertyValue(const ::rtl::OUString & PropertyName, const uno::Any & ControlValue) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_xFormComponentHandler->convertToPropertyValue(PropertyName, ControlValue);
}

uno::Any SAL_CALL ReportComponentHandler::convertToControlValue(const ::rtl::OUString & PropertyName, const uno::Any & PropertyValue, const uno::Type & ControlValueType) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_xFormComponentHandler->convertToControlValue(PropertyName, PropertyValue, ControlValueType);
}

void SAL_CALL ReportComponentHandler::addPropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & Listener) throw (uno::RuntimeException, lang::NullPointerException)
{
    m_xFormComponentHandler->addPropertyChangeListener(Listener);
}

void SAL_CALL ReportComponentHandler::removePropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & _rxListener) throw (uno::RuntimeException)
{
    m_xFormComponentHandler->removePropertyChangeListener(_rxListener);
}

uno::Sequence< beans::Property > SAL_CALL ReportComponentHandler::getSupportedProperties() throw (uno::RuntimeException)
{
    ::std::vector< beans::Property > aNewProps;
    m_pInfoService->getExcludeProperties( aNewProps, m_xFormComponentHandler );

    return aNewProps.empty() ? uno::Sequence< beans::Property > () : uno::Sequence< beans::Property > (&(*aNewProps.begin()),aNewProps.size());
}

uno::Sequence< ::rtl::OUString > SAL_CALL ReportComponentHandler::getSupersededProperties() throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet;
    return aRet;
}

uno::Sequence< ::rtl::OUString > SAL_CALL ReportComponentHandler::getActuatingProperties() throw (uno::RuntimeException)
{
    return m_xFormComponentHandler->getActuatingProperties();
}

::sal_Bool SAL_CALL ReportComponentHandler::isComposable( const ::rtl::OUString& _rPropertyName ) throw (uno::RuntimeException, beans::UnknownPropertyException)
{
    return m_pInfoService->isComposable( _rPropertyName, m_xFormComponentHandler );
}

inspection::InteractiveSelectionResult SAL_CALL ReportComponentHandler::onInteractivePropertySelection(const ::rtl::OUString & PropertyName, ::sal_Bool Primary, uno::Any & out_Data, const uno::Reference< inspection::XObjectInspectorUI > & InspectorUI) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::NullPointerException)
{
    return m_xFormComponentHandler->onInteractivePropertySelection(PropertyName, Primary, out_Data, InspectorUI);
}

void SAL_CALL ReportComponentHandler::actuatingPropertyChanged(const ::rtl::OUString & ActuatingPropertyName, const uno::Any & NewValue, const uno::Any & OldValue, const uno::Reference< inspection::XObjectInspectorUI > & InspectorUI, ::sal_Bool FirstTimeInit) throw (uno::RuntimeException, lang::NullPointerException)
{
    m_xFormComponentHandler->actuatingPropertyChanged(ActuatingPropertyName, NewValue, OldValue, InspectorUI, FirstTimeInit);
}

::sal_Bool SAL_CALL ReportComponentHandler::suspend(::sal_Bool Suspend) throw (uno::RuntimeException)
{
    return m_xFormComponentHandler->suspend(Suspend);
}

//........................................................................
} // namespace rptui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
