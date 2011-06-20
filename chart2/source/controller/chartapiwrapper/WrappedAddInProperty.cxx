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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedAddInProperty.hxx"
#include "macros.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;
using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................
namespace wrapper
{

WrappedAddInProperty::WrappedAddInProperty( ChartDocumentWrapper& rChartDocumentWrapper )
    : ::chart::WrappedProperty( C2U( "AddIn" ), OUString() )
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
        throw lang::IllegalArgumentException( C2U("AddIn properties require type XRefreshable"), 0, 0 );

    m_rChartDocumentWrapper.setAddIn( xAddIn );
}

Any WrappedAddInProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( m_rChartDocumentWrapper.getAddIn() );
}

//.............................................................................

WrappedBaseDiagramProperty::WrappedBaseDiagramProperty( ChartDocumentWrapper& rChartDocumentWrapper )
    : ::chart::WrappedProperty( C2U( "BaseDiagram" ), OUString() )
    , m_rChartDocumentWrapper( rChartDocumentWrapper )
{
}
WrappedBaseDiagramProperty::~WrappedBaseDiagramProperty()
{
}

void WrappedBaseDiagramProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    rtl::OUString aBaseDiagram;
    if( ! (rOuterValue >>= aBaseDiagram) )
        throw lang::IllegalArgumentException( C2U("BaseDiagram properties require type OUString"), 0, 0 );

    m_rChartDocumentWrapper.setBaseDiagram( aBaseDiagram );
}

Any WrappedBaseDiagramProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( m_rChartDocumentWrapper.getBaseDiagram() );
}

//.............................................................................

WrappedAdditionalShapesProperty::WrappedAdditionalShapesProperty( ChartDocumentWrapper& rChartDocumentWrapper )
    : ::chart::WrappedProperty( C2U( "AdditionalShapes" ), OUString() )
    , m_rChartDocumentWrapper( rChartDocumentWrapper )
{
}
WrappedAdditionalShapesProperty::~WrappedAdditionalShapesProperty()
{
}

void WrappedAdditionalShapesProperty::setPropertyValue( const Any& /*rOuterValue*/, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    throw lang::IllegalArgumentException( C2U("AdditionalShapes is a read only property"), 0, 0 );
}

Any WrappedAdditionalShapesProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( m_rChartDocumentWrapper.getAdditionalShapes() );
}

//.............................................................................

WrappedRefreshAddInAllowedProperty::WrappedRefreshAddInAllowedProperty( ChartDocumentWrapper& rChartDocumentWrapper )
    : ::chart::WrappedProperty( C2U( "RefreshAddInAllowed" ), OUString() )
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
        throw lang::IllegalArgumentException( C2U("The property RefreshAddInAllowed requires type boolean"), 0, 0 );

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
