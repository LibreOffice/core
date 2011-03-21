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
#include "LegendWrapper.hxx"
#include "macros.hxx"
#include "Chart2ModelContact.hxx"
#include "LegendHelper.hxx"
#include "ContainerHelper.hxx"
#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

#include "CharacterProperties.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "WrappedCharacterHeightProperty.hxx"
#include "PositionAndSizeHelper.hxx"
#include "WrappedDirectStateProperty.hxx"
#include "WrappedAutomaticPositionProperties.hxx"
#include "WrappedScaleTextProperties.hxx"

#include <algorithm>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace chart
{
class WrappedLegendAlignmentProperty : public WrappedProperty
{
public:
    WrappedLegendAlignmentProperty();
    virtual ~WrappedLegendAlignmentProperty();

    virtual void setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                                    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException);
    virtual Any getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

protected:
    virtual Any convertInnerToOuterValue( const Any& rInnerValue ) const;
    virtual Any convertOuterToInnerValue( const Any& rOuterValue ) const;
};

WrappedLegendAlignmentProperty::WrappedLegendAlignmentProperty()
    : ::chart::WrappedProperty( C2U( "Alignment" ), C2U( "AnchorPosition" ) )
{
}
WrappedLegendAlignmentProperty::~WrappedLegendAlignmentProperty()
{
}

Any WrappedLegendAlignmentProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    if( xInnerPropertySet.is() )
    {
        sal_Bool bShowLegend = sal_True;
        xInnerPropertySet->getPropertyValue( C2U( "Show" ) ) >>= bShowLegend;
        if(!bShowLegend)
        {
            aRet = uno::makeAny( ::com::sun::star::chart::ChartLegendPosition_NONE );
        }
        else
        {
            aRet = xInnerPropertySet->getPropertyValue( m_aInnerName );
            aRet = this->convertInnerToOuterValue( aRet );
        }
    }
    return aRet;
}

void WrappedLegendAlignmentProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if(xInnerPropertySet.is())
    {
        sal_Bool bNewShowLegend = sal_True;
        sal_Bool bOldShowLegend = sal_True;
        {
            ::com::sun::star::chart::ChartLegendPosition eOuterPos(::com::sun::star::chart::ChartLegendPosition_NONE);
            if( (rOuterValue >>= eOuterPos)  && eOuterPos == ::com::sun::star::chart::ChartLegendPosition_NONE )
                bNewShowLegend = sal_False;
            xInnerPropertySet->getPropertyValue( C2U( "Show" ) ) >>= bOldShowLegend;
        }
        if(bNewShowLegend!=bOldShowLegend)
        {
            xInnerPropertySet->setPropertyValue( C2U( "Show" ), uno::makeAny(bNewShowLegend) );
        }
        if(!bNewShowLegend)
            return;

        //set corresponding LegendPosition
        Any aInnerValue = this->convertOuterToInnerValue( rOuterValue );
        xInnerPropertySet->setPropertyValue( m_aInnerName, aInnerValue );

        //correct LegendExpansion
        chart2::LegendPosition eNewInnerPos(chart2::LegendPosition_LINE_END);
        if( aInnerValue >>= eNewInnerPos )
        {
            ::com::sun::star::chart::ChartLegendExpansion eNewExpansion =
                ( eNewInnerPos == chart2::LegendPosition_LINE_END ||
                  eNewInnerPos == chart2::LegendPosition_LINE_START )
                ? ::com::sun::star::chart::ChartLegendExpansion_HIGH
                : ::com::sun::star::chart::ChartLegendExpansion_WIDE;

            ::com::sun::star::chart::ChartLegendExpansion eOldExpansion( ::com::sun::star::chart::ChartLegendExpansion_HIGH );
            bool bExpansionWasSet(
                xInnerPropertySet->getPropertyValue( C2U( "Expansion" ) ) >>= eOldExpansion );

            if( !bExpansionWasSet || (eOldExpansion != eNewExpansion))
                xInnerPropertySet->setPropertyValue( C2U( "Expansion" ), uno::makeAny( eNewExpansion ));
        }

        //correct RelativePosition
        Any aRelativePosition( xInnerPropertySet->getPropertyValue( C2U( "RelativePosition" ) ) );
        if(aRelativePosition.hasValue())
        {
            xInnerPropertySet->setPropertyValue( C2U( "RelativePosition" ), Any() );
        }
    }
}

Any WrappedLegendAlignmentProperty::convertInnerToOuterValue( const Any& rInnerValue ) const
{
    ::com::sun::star::chart::ChartLegendPosition ePos = ::com::sun::star::chart::ChartLegendPosition_NONE;

    chart2::LegendPosition eNewPos;
    if( rInnerValue >>= eNewPos )
    {
        switch( eNewPos )
        {
            case chart2::LegendPosition_LINE_START:
                ePos = ::com::sun::star::chart::ChartLegendPosition_LEFT;
                break;
            case chart2::LegendPosition_LINE_END:
                ePos = ::com::sun::star::chart::ChartLegendPosition_RIGHT;
                break;
            case chart2::LegendPosition_PAGE_START:
                ePos = ::com::sun::star::chart::ChartLegendPosition_TOP;
                break;
            case chart2::LegendPosition_PAGE_END:
                ePos = ::com::sun::star::chart::ChartLegendPosition_BOTTOM;
                break;

            default:
                ePos = ::com::sun::star::chart::ChartLegendPosition_NONE;
                break;
        }
    }
    return uno::makeAny( ePos );
}
Any WrappedLegendAlignmentProperty::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    chart2::LegendPosition eNewPos = chart2::LegendPosition_LINE_END;

    ::com::sun::star::chart::ChartLegendPosition ePos;
    if( rOuterValue >>= ePos )
    {
        switch( ePos )
        {
            case ::com::sun::star::chart::ChartLegendPosition_LEFT:
                eNewPos = chart2::LegendPosition_LINE_START;
                break;
            case ::com::sun::star::chart::ChartLegendPosition_RIGHT:
                eNewPos = chart2::LegendPosition_LINE_END;
                break;
            case ::com::sun::star::chart::ChartLegendPosition_TOP:
                eNewPos = chart2::LegendPosition_PAGE_START;
                break;
            case ::com::sun::star::chart::ChartLegendPosition_BOTTOM:
                eNewPos = chart2::LegendPosition_PAGE_END;
                break;
            default: // NONE
                break;
        }
    }

    return uno::makeAny( eNewPos );
}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Legend" ));

enum
{
    PROP_LEGEND_ALIGNMENT,
    PROP_LEGEND_EXPANSION
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Alignment" ),
                  PROP_LEGEND_ALIGNMENT,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartLegendPosition * >(0)),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Expansion" ),
                  PROP_LEGEND_EXPANSION,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartLegendExpansion * >(0)),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticLegendWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence() );
        return &aPropSeq;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        //::chart::NamedProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );
        ::chart::wrapper::WrappedAutomaticPositionProperties::addProperties( aProperties );
        ::chart::wrapper::WrappedScaleTextProperties::addProperties( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }
};

struct StaticLegendWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticLegendWrapperPropertyArray_Initializer >
{
};

} // anonymous namespace

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

LegendWrapper::LegendWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex )
{
}

LegendWrapper::~LegendWrapper()
{
}

// ____ XShape ____
awt::Point SAL_CALL LegendWrapper::getPosition()
    throw (uno::RuntimeException)
{
    return m_spChart2ModelContact->GetLegendPosition();
}

void SAL_CALL LegendWrapper::setPosition( const awt::Point& aPosition )
    throw (uno::RuntimeException)
{
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet() );
    if( xProp.is() )
    {
        awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );

        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_TOP_LEFT;
        aRelativePosition.Primary = double(aPosition.X)/double(aPageSize.Width);
        aRelativePosition.Secondary = double(aPosition.Y)/double(aPageSize.Height);
        xProp->setPropertyValue( C2U( "RelativePosition" ), uno::makeAny(aRelativePosition) );
    }
}

awt::Size SAL_CALL LegendWrapper::getSize()
    throw (uno::RuntimeException)
{
    return m_spChart2ModelContact->GetLegendSize();
}

void SAL_CALL LegendWrapper::setSize( const awt::Size& aSize )
    throw (beans::PropertyVetoException,
           uno::RuntimeException)
{
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet() );
    if( xProp.is() )
    {
        awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );
        awt::Rectangle aPageRectangle( 0,0,aPageSize.Width,aPageSize.Height);

        awt::Point aPos( this->getPosition() );
        awt::Rectangle aNewPositionAndSize(aPos.X,aPos.Y,aSize.Width,aSize.Height);

        PositionAndSizeHelper::moveObject( OBJECTTYPE_LEGEND
                , xProp, aNewPositionAndSize, aPageRectangle );
    }
}

// ____ XShapeDescriptor (base of XShape) ____
::rtl::OUString SAL_CALL LegendWrapper::getShapeType()
    throw (uno::RuntimeException)
{
    return C2U( "com.sun.star.chart.ChartLegend" );
}

// ____ XComponent ____
void SAL_CALL LegendWrapper::dispose()
    throw (uno::RuntimeException)
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    // /--
    MutexGuard aGuard( GetMutex());
    clearWrappedPropertySet();
    // \--
}

void SAL_CALL LegendWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL LegendWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ================================================================================

//ReferenceSizePropertyProvider
void LegendWrapper::updateReferenceSize()
{
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        if( xProp->getPropertyValue( C2U("ReferencePageSize") ).hasValue() )
            xProp->setPropertyValue( C2U("ReferencePageSize"), uno::makeAny(
                m_spChart2ModelContact->GetPageSize() ));
    }
}
Any LegendWrapper::getReferenceSize()
{
    Any aRet;
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet(), uno::UNO_QUERY );
    if( xProp.is() )
        aRet = xProp->getPropertyValue( C2U("ReferencePageSize") );

    return aRet;
}
awt::Size LegendWrapper::getCurrentSizeForReference()
{
    return m_spChart2ModelContact->GetPageSize();
}

// ================================================================================

// WrappedPropertySet
Reference< beans::XPropertySet > LegendWrapper::getInnerPropertySet()
{
    Reference< beans::XPropertySet > xRet;
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() )
        xRet.set( xDiagram->getLegend(), uno::UNO_QUERY );
    OSL_ENSURE(xRet.is(),"LegendWrapper::getInnerPropertySet() is NULL");
    return xRet;
}

const Sequence< beans::Property >& LegendWrapper::getPropertySequence()
{
    return *StaticLegendWrapperPropertyArray::get();
}

const std::vector< WrappedProperty* > LegendWrapper::createWrappedProperties()
{
    ::std::vector< ::chart::WrappedProperty* > aWrappedProperties;

    aWrappedProperties.push_back( new WrappedLegendAlignmentProperty() );
    aWrappedProperties.push_back( new WrappedProperty( C2U("Expansion"), C2U("Expansion") ));
    WrappedCharacterHeightProperty::addWrappedProperties( aWrappedProperties, this );
    //same problem as for wall: thje defaults ion the old chart are different for different charttypes, so we need to export explicitly
    aWrappedProperties.push_back( new WrappedDirectStateProperty( C2U("FillStyle"), C2U("FillStyle") ) );
    aWrappedProperties.push_back( new WrappedDirectStateProperty( C2U("FillColor"), C2U("FillColor") ));
    WrappedAutomaticPositionProperties::addWrappedProperties( aWrappedProperties );
    WrappedScaleTextProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    return aWrappedProperties;
}

// ================================================================================

Sequence< ::rtl::OUString > LegendWrapper::getSupportedServiceNames_Static()
{
    Sequence< ::rtl::OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartLegend" );
    aServices[ 1 ] = C2U( "com.sun.star.drawing.Shape" );
    aServices[ 2 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 3 ] = C2U( "com.sun.star.style.CharacterProperties" );
//     aServices[ 4 ] = C2U( "com.sun.star.beans.PropertySet" );
//     aServices[ 5 ] = C2U( "com.sun.star.drawing.FillProperties" );
//     aServices[ 6 ] = C2U( "com.sun.star.drawing.LineProperties" );

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LegendWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart
