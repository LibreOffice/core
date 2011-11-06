/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
//.............................................................................
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
//.............................................................................
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
//.............................................................................
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

}

//.............................................................................
} //namespace chart
//.............................................................................
