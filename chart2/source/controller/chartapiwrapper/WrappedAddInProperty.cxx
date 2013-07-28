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

#include "WrappedAddInProperty.hxx"
#include "macros.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using namespace ::com::sun::star;

namespace chart
{
namespace wrapper
{

WrappedAddInProperty::WrappedAddInProperty( ChartDocumentWrapper& rChartDocumentWrapper )
    : ::chart::WrappedProperty( "AddIn", OUString() )
    , m_rChartDocumentWrapper( rChartDocumentWrapper )
{
}
WrappedAddInProperty::~WrappedAddInProperty()
{
}

void WrappedAddInProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< util::XRefreshable > xAddIn;
    if( ! (rOuterValue >>= xAddIn) )
        throw lang::IllegalArgumentException( "AddIn properties require type XRefreshable", 0, 0 );

    m_rChartDocumentWrapper.setAddIn( xAddIn );
}

Any WrappedAddInProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( m_rChartDocumentWrapper.getAddIn() );
}

WrappedBaseDiagramProperty::WrappedBaseDiagramProperty( ChartDocumentWrapper& rChartDocumentWrapper )
    : ::chart::WrappedProperty( "BaseDiagram" , OUString() )
    , m_rChartDocumentWrapper( rChartDocumentWrapper )
{
}
WrappedBaseDiagramProperty::~WrappedBaseDiagramProperty()
{
}

void WrappedBaseDiagramProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OUString aBaseDiagram;
    if( ! (rOuterValue >>= aBaseDiagram) )
        throw lang::IllegalArgumentException( "BaseDiagram properties require type OUString", 0, 0 );

    m_rChartDocumentWrapper.setBaseDiagram( aBaseDiagram );
}

Any WrappedBaseDiagramProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( m_rChartDocumentWrapper.getBaseDiagram() );
}

WrappedAdditionalShapesProperty::WrappedAdditionalShapesProperty( ChartDocumentWrapper& rChartDocumentWrapper )
    : ::chart::WrappedProperty( "AdditionalShapes" , OUString() )
    , m_rChartDocumentWrapper( rChartDocumentWrapper )
{
}
WrappedAdditionalShapesProperty::~WrappedAdditionalShapesProperty()
{
}

void WrappedAdditionalShapesProperty::setPropertyValue( const Any& /*rOuterValue*/, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    throw lang::IllegalArgumentException( "AdditionalShapes is a read only property", 0, 0 );
}

Any WrappedAdditionalShapesProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( m_rChartDocumentWrapper.getAdditionalShapes() );
}

WrappedRefreshAddInAllowedProperty::WrappedRefreshAddInAllowedProperty( ChartDocumentWrapper& rChartDocumentWrapper )
    : ::chart::WrappedProperty( "RefreshAddInAllowed" , OUString() )
    , m_rChartDocumentWrapper( rChartDocumentWrapper )
{
}
WrappedRefreshAddInAllowedProperty::~WrappedRefreshAddInAllowedProperty()
{
}

void WrappedRefreshAddInAllowedProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /* xInnerPropertySet */ ) const
                        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bUpdateAddIn = sal_True;
    if( ! (rOuterValue >>= bUpdateAddIn) )
        throw lang::IllegalArgumentException( "The property RefreshAddInAllowed requires type boolean", 0, 0 );

    m_rChartDocumentWrapper.setUpdateAddIn( bUpdateAddIn );
}

Any WrappedRefreshAddInAllowedProperty::getPropertyValue( const Reference< beans::XPropertySet >& /* xInnerPropertySet */ ) const
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( m_rChartDocumentWrapper.getUpdateAddIn() );
}

} //namespace wrapper

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
