/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedAddInProperty.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:20:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

void WrappedAddInProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< util::XRefreshable > xAddIn;
    if( ! (rOuterValue >>= xAddIn) )
        throw lang::IllegalArgumentException( C2U("AddIn properties require type XRefreshable"), 0, 0 );

    m_rChartDocumentWrapper.setAddIn( xAddIn );
}

Any WrappedAddInProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
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

void WrappedBaseDiagramProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    rtl::OUString aBaseDiagram;
    if( ! (rOuterValue >>= aBaseDiagram) )
        throw lang::IllegalArgumentException( C2U("BaseDiagram properties require type OUString"), 0, 0 );

    m_rChartDocumentWrapper.setBaseDiagram( aBaseDiagram );
}

Any WrappedBaseDiagramProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
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

void WrappedAdditionalShapesProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    throw lang::IllegalArgumentException( C2U("AdditionalShapes is a read only property"), 0, 0 );
}

Any WrappedAdditionalShapesProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
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

void WrappedRefreshAddInAllowedProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bUpdateAddIn = sal_True;
    if( ! (rOuterValue >>= bUpdateAddIn) )
        throw lang::IllegalArgumentException( C2U("The property RefreshAddInAllowed requires type boolean"), 0, 0 );

    m_rChartDocumentWrapper.setUpdateAddIn( bUpdateAddIn );
}

Any WrappedRefreshAddInAllowedProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( m_rChartDocumentWrapper.getUpdateAddIn() );
}

}

//.............................................................................
} //namespace chart
//.............................................................................
