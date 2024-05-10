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
#include <ReportComponentHandler.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/form/inspection/FormComponentPropertyHandler.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <metadata.hxx>
#include <utility>


namespace rptui
{

using namespace ::com::sun::star;

ReportComponentHandler::ReportComponentHandler(uno::Reference< uno::XComponentContext > context)
    :ReportComponentHandler_Base(m_aMutex)
    ,m_xContext(std::move(context))
{
    try
    {
        m_xFormComponentHandler = form::inspection::FormComponentPropertyHandler::create(m_xContext);

    }catch(const uno::Exception &)
    {
    }
}

OUString SAL_CALL ReportComponentHandler::getImplementationName(  )
{
    return u"com.sun.star.comp.report.ReportComponentHandler"_ustr;
}

sal_Bool SAL_CALL ReportComponentHandler::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL ReportComponentHandler::getSupportedServiceNames(  )
{
    return { u"com.sun.star.report.inspection.ReportComponentHandler"_ustr };
}

// override WeakComponentImplHelperBase::disposing()
// This function is called upon disposing the component,
// if your component needs special work when it becomes
// disposed, do it here.
void SAL_CALL ReportComponentHandler::disposing()
{
    ::comphelper::disposeComponent(m_xFormComponentHandler);
}
void SAL_CALL ReportComponentHandler::addEventListener(const uno::Reference< lang::XEventListener > & xListener)
{
    m_xFormComponentHandler->addEventListener(xListener);
}

void SAL_CALL ReportComponentHandler::removeEventListener(const uno::Reference< lang::XEventListener > & aListener)
{
    m_xFormComponentHandler->removeEventListener(aListener);
}

// inspection::XPropertyHandler:

/********************************************************************************/
void SAL_CALL ReportComponentHandler::inspect(const uno::Reference< uno::XInterface > & Component)
{
    try
    {
        uno::Reference< container::XNameContainer > xNameCont(Component,uno::UNO_QUERY);
        static constexpr OUString sFormComponent(u"FormComponent"_ustr);
        if ( xNameCont->hasByName(sFormComponent) )
            xNameCont->getByName(sFormComponent) >>= m_xFormComponent;
        static constexpr OUString sRowSet(u"RowSet"_ustr);
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

uno::Any SAL_CALL ReportComponentHandler::getPropertyValue(const OUString & PropertyName)
{
    return m_xFormComponentHandler->getPropertyValue(PropertyName);
}

void SAL_CALL ReportComponentHandler::setPropertyValue(const OUString & PropertyName, const uno::Any & Value)
{
    m_xFormComponentHandler->setPropertyValue(PropertyName, Value);
}

beans::PropertyState SAL_CALL ReportComponentHandler::getPropertyState(const OUString & PropertyName)
{
    return m_xFormComponentHandler->getPropertyState(PropertyName);
}

inspection::LineDescriptor SAL_CALL ReportComponentHandler::describePropertyLine(const OUString & PropertyName,  const uno::Reference< inspection::XPropertyControlFactory > & ControlFactory)
{
    return m_xFormComponentHandler->describePropertyLine(PropertyName, ControlFactory);
}

uno::Any SAL_CALL ReportComponentHandler::convertToPropertyValue(const OUString & PropertyName, const uno::Any & ControlValue)
{
    return m_xFormComponentHandler->convertToPropertyValue(PropertyName, ControlValue);
}

uno::Any SAL_CALL ReportComponentHandler::convertToControlValue(const OUString & PropertyName, const uno::Any & PropertyValue, const uno::Type & ControlValueType)
{
    return m_xFormComponentHandler->convertToControlValue(PropertyName, PropertyValue, ControlValueType);
}

void SAL_CALL ReportComponentHandler::addPropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & Listener)
{
    m_xFormComponentHandler->addPropertyChangeListener(Listener);
}

void SAL_CALL ReportComponentHandler::removePropertyChangeListener(const uno::Reference< beans::XPropertyChangeListener > & _rxListener)
{
    m_xFormComponentHandler->removePropertyChangeListener(_rxListener);
}

uno::Sequence< beans::Property > SAL_CALL ReportComponentHandler::getSupportedProperties()
{
    ::std::vector< beans::Property > aNewProps;
    rptui::OPropertyInfoService::getExcludeProperties( aNewProps, m_xFormComponentHandler );

    return uno::Sequence< beans::Property >(aNewProps.data(), aNewProps.size());
}

uno::Sequence< OUString > SAL_CALL ReportComponentHandler::getSupersededProperties()
{
    uno::Sequence< OUString > aRet;
    return aRet;
}

uno::Sequence< OUString > SAL_CALL ReportComponentHandler::getActuatingProperties()
{
    return m_xFormComponentHandler->getActuatingProperties();
}

sal_Bool SAL_CALL ReportComponentHandler::isComposable( const OUString& _rPropertyName )
{
    return OPropertyInfoService::isComposable( _rPropertyName, m_xFormComponentHandler );
}

inspection::InteractiveSelectionResult SAL_CALL ReportComponentHandler::onInteractivePropertySelection(const OUString & PropertyName, sal_Bool Primary, uno::Any & out_Data, const uno::Reference< inspection::XObjectInspectorUI > & InspectorUI)
{
    return m_xFormComponentHandler->onInteractivePropertySelection(PropertyName, Primary, out_Data, InspectorUI);
}

void SAL_CALL ReportComponentHandler::actuatingPropertyChanged(const OUString & ActuatingPropertyName, const uno::Any & NewValue, const uno::Any & OldValue, const uno::Reference< inspection::XObjectInspectorUI > & InspectorUI, sal_Bool FirstTimeInit)
{
    m_xFormComponentHandler->actuatingPropertyChanged(ActuatingPropertyName, NewValue, OldValue, InspectorUI, FirstTimeInit);
}

sal_Bool SAL_CALL ReportComponentHandler::suspend(sal_Bool Suspend)
{
    return m_xFormComponentHandler->suspend(Suspend);
}


} // namespace rptui


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_ReportComponentHandler_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new rptui::ReportComponentHandler(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
