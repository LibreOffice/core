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

#include "WallFloorWrapper.hxx"
#include "Chart2ModelContact.hxx"
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <FillProperties.hxx>
#include <LinePropertiesHelper.hxx>
#include <UserDefinedProperties.hxx>
#include <WrappedDirectStateProperty.hxx>

#include <algorithm>
#include <utility>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart::wrapper
{

WallFloorWrapper::WallFloorWrapper( bool bWall,
    std::shared_ptr<Chart2ModelContact> spChart2ModelContact ) :
        m_spChart2ModelContact(std::move( spChart2ModelContact )),
        m_bWall( bWall )

{
}

WallFloorWrapper::~WallFloorWrapper()
{
}

// ____ XComponent ____
void SAL_CALL WallFloorWrapper::dispose()
{
    std::unique_lock g(m_aMutex);
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( g, lang::EventObject( xSource ) );

    clearWrappedPropertySet();
}

void SAL_CALL WallFloorWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.addInterface( g, xListener );
}

void SAL_CALL WallFloorWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.removeInterface( g, aListener );
}

// WrappedPropertySet
Reference< beans::XPropertySet > WallFloorWrapper::getInnerPropertySet()
{
    Reference< beans::XPropertySet > xRet;

    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( xDiagram.is() )
    {
        if( m_bWall )
            xRet.set( xDiagram->getWall() );
        else
            xRet.set( xDiagram->getFloor() );
    }

    return xRet;
}

const Sequence< beans::Property >& WallFloorWrapper::getPropertySequence()
{
    static Sequence< Property > aPropSeq = []()
        {
            std::vector< css::beans::Property > aProperties;
            ::chart::FillProperties::AddPropertiesToVector( aProperties );
            ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropSeq;
}

std::vector< std::unique_ptr<WrappedProperty> > WallFloorWrapper::createWrappedProperties()
{
    std::vector< std::unique_ptr<WrappedProperty> > aWrappedProperties;

    // use direct state always, so that in XML the value is always
    // exported. Because in the old chart the defaults is as follows:
    // Floor: SOLID (new and old model default), Wall: NONE, except for some chart types (line, scatter)
    if( m_bWall )
        aWrappedProperties.emplace_back( new WrappedDirectStateProperty( u"FillStyle"_ustr, u"FillStyle"_ustr ));
    aWrappedProperties.emplace_back( new WrappedDirectStateProperty( u"FillColor"_ustr, u"FillColor"_ustr ));

    return aWrappedProperties;
}

OUString SAL_CALL WallFloorWrapper::getImplementationName()
{
    return u"com.sun.star.comp.chart.WallOrFloor"_ustr;
}

sal_Bool SAL_CALL WallFloorWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL WallFloorWrapper::getSupportedServiceNames()
{
    return {
        u"com.sun.star.xml.UserDefinedAttributesSupplier"_ustr,
        u"com.sun.star.drawing.FillProperties"_ustr,
        u"com.sun.star.drawing.LineProperties"_ustr,
        u"com.sun.star.beans.PropertySet"_ustr
    };
}

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
