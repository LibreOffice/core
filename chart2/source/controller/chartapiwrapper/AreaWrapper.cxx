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

#include "AreaWrapper.hxx"
#include "Chart2ModelContact.hxx"
#include <WrappedDirectStateProperty.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <UserDefinedProperties.hxx>

#include <algorithm>
#include <utility>

using namespace ::com::sun::star;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart::wrapper
{

AreaWrapper::AreaWrapper(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
    : m_spChart2ModelContact(std::move(spChart2ModelContact))
{
}

AreaWrapper::~AreaWrapper()
{}

// ____ XShape ____
awt::Point SAL_CALL AreaWrapper::getPosition()
{
    return awt::Point(0,0);
}

void SAL_CALL AreaWrapper::setPosition( const awt::Point& /*aPosition*/ )
{
    OSL_FAIL( "trying to set position of chart area" );
}

awt::Size SAL_CALL AreaWrapper::getSize()
{
    return m_spChart2ModelContact->GetPageSize();
}

void SAL_CALL AreaWrapper::setSize( const awt::Size& /*aSize*/ )
{
    OSL_FAIL( "trying to set size of chart area" );
}

// ____ XShapeDescriptor (base of XShape) ____
OUString SAL_CALL AreaWrapper::getShapeType()
{
    return u"com.sun.star.chart.ChartArea"_ustr;
}

// ____ XComponent ____
void SAL_CALL AreaWrapper::dispose()
{
    std::unique_lock g(m_aMutex);
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( g, lang::EventObject( xSource ) );

    clearWrappedPropertySet();
}

void SAL_CALL AreaWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.addInterface( g, xListener );
}

void SAL_CALL AreaWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.removeInterface( g, aListener );
}

// WrappedPropertySet
Reference< beans::XPropertySet > AreaWrapper::getInnerPropertySet()
{
    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    if( xChartDoc.is() )
        return xChartDoc->getPageBackground();
    OSL_FAIL("AreaWrapper::getInnerPropertySet() is NULL");
    return nullptr;
}

const Sequence< beans::Property >& AreaWrapper::getPropertySequence()
{
    static Sequence< Property > aPropSeq = []()
        {
            std::vector< css::beans::Property > aProperties;
            ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
            ::chart::FillProperties::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropSeq;
}

std::vector< std::unique_ptr<WrappedProperty> > AreaWrapper::createWrappedProperties()
{
    std::vector< std::unique_ptr<WrappedProperty> > aWrappedProperties;

    aWrappedProperties.emplace_back( new WrappedDirectStateProperty(u"LineStyle"_ustr,u"LineStyle"_ustr) );

    return aWrappedProperties;
}

OUString SAL_CALL AreaWrapper::getImplementationName()
{
    return u"com.sun.star.comp.chart.Area"_ustr;
}

sal_Bool SAL_CALL AreaWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL AreaWrapper::getSupportedServiceNames()
{
    return {
        u"com.sun.star.xml.UserDefinedAttributesSupplier"_ustr,
        u"com.sun.star.beans.PropertySet"_ustr,
        u"com.sun.star.drawing.FillProperties"_ustr,
        u"com.sun.star.drawing.LineProperties"_ustr };
}

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
