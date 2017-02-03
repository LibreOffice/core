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

#include "LegendWrapper.hxx"
#include "macros.hxx"
#include "Chart2ModelContact.hxx"
#include "LegendHelper.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

#include "CharacterProperties.hxx"
#include "LinePropertiesHelper.hxx"
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

namespace chart
{
class WrappedLegendAlignmentProperty : public WrappedProperty
{
public:
    WrappedLegendAlignmentProperty();

    virtual void setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const override;
    virtual Any getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const override;

protected:
    virtual Any convertInnerToOuterValue( const Any& rInnerValue ) const override;
    virtual Any convertOuterToInnerValue( const Any& rOuterValue ) const override;
};

WrappedLegendAlignmentProperty::WrappedLegendAlignmentProperty()
    : ::chart::WrappedProperty( "Alignment", "AnchorPosition" )
{
}

Any WrappedLegendAlignmentProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Any aRet;
    if( xInnerPropertySet.is() )
    {
        bool bShowLegend = true;
        xInnerPropertySet->getPropertyValue( "Show" ) >>= bShowLegend;
        if(!bShowLegend)
        {
            aRet = uno::Any( css::chart::ChartLegendPosition_NONE );
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
{
    if(xInnerPropertySet.is())
    {
        bool bNewShowLegend = true;
        bool bOldShowLegend = true;
        {
            css::chart::ChartLegendPosition eOuterPos(css::chart::ChartLegendPosition_NONE);
            if( (rOuterValue >>= eOuterPos)  && eOuterPos == css::chart::ChartLegendPosition_NONE )
                bNewShowLegend = false;
            xInnerPropertySet->getPropertyValue( "Show" ) >>= bOldShowLegend;
        }
        if(bNewShowLegend!=bOldShowLegend)
        {
            xInnerPropertySet->setPropertyValue( "Show", uno::Any(bNewShowLegend) );
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
            css::chart::ChartLegendExpansion eNewExpansion =
                ( eNewInnerPos == chart2::LegendPosition_LINE_END ||
                  eNewInnerPos == chart2::LegendPosition_LINE_START )
                ? css::chart::ChartLegendExpansion_HIGH
                : css::chart::ChartLegendExpansion_WIDE;

            css::chart::ChartLegendExpansion eOldExpansion( css::chart::ChartLegendExpansion_HIGH );
            bool bExpansionWasSet(
                xInnerPropertySet->getPropertyValue( "Expansion" ) >>= eOldExpansion );

            if( !bExpansionWasSet || (eOldExpansion != eNewExpansion))
                xInnerPropertySet->setPropertyValue( "Expansion", uno::Any( eNewExpansion ));
        }

        //correct RelativePosition
        Any aRelativePosition( xInnerPropertySet->getPropertyValue("RelativePosition") );
        if(aRelativePosition.hasValue())
        {
            xInnerPropertySet->setPropertyValue( "RelativePosition", Any() );
        }
    }
}

Any WrappedLegendAlignmentProperty::convertInnerToOuterValue( const Any& rInnerValue ) const
{
    css::chart::ChartLegendPosition ePos = css::chart::ChartLegendPosition_NONE;

    chart2::LegendPosition eNewPos;
    if( rInnerValue >>= eNewPos )
    {
        switch( eNewPos )
        {
            case chart2::LegendPosition_LINE_START:
                ePos = css::chart::ChartLegendPosition_LEFT;
                break;
            case chart2::LegendPosition_LINE_END:
                ePos = css::chart::ChartLegendPosition_RIGHT;
                break;
            case chart2::LegendPosition_PAGE_START:
                ePos = css::chart::ChartLegendPosition_TOP;
                break;
            case chart2::LegendPosition_PAGE_END:
                ePos = css::chart::ChartLegendPosition_BOTTOM;
                break;

            default:
                ePos = css::chart::ChartLegendPosition_NONE;
                break;
        }
    }
    return uno::Any( ePos );
}
Any WrappedLegendAlignmentProperty::convertOuterToInnerValue( const Any& rOuterValue ) const
{
    chart2::LegendPosition eNewPos = chart2::LegendPosition_LINE_END;

    css::chart::ChartLegendPosition ePos;
    if( rOuterValue >>= ePos )
    {
        switch( ePos )
        {
            case css::chart::ChartLegendPosition_LEFT:
                eNewPos = chart2::LegendPosition_LINE_START;
                break;
            case css::chart::ChartLegendPosition_RIGHT:
                eNewPos = chart2::LegendPosition_LINE_END;
                break;
            case css::chart::ChartLegendPosition_TOP:
                eNewPos = chart2::LegendPosition_PAGE_START;
                break;
            case css::chart::ChartLegendPosition_BOTTOM:
                eNewPos = chart2::LegendPosition_PAGE_END;
                break;
            default: // NONE
                break;
        }
    }

    return uno::Any( eNewPos );
}
}

namespace
{

enum
{
    PROP_LEGEND_ALIGNMENT,
    PROP_LEGEND_EXPANSION
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "Alignment",
                  PROP_LEGEND_ALIGNMENT,
                  cppu::UnoType<css::chart::ChartLegendPosition>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "Expansion",
                  PROP_LEGEND_EXPANSION,
                  cppu::UnoType<css::chart::ChartLegendExpansion>::get(),
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
    static Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );
        ::chart::wrapper::WrappedAutomaticPositionProperties::addProperties( aProperties );
        ::chart::wrapper::WrappedScaleTextProperties::addProperties( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }
};

struct StaticLegendWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticLegendWrapperPropertyArray_Initializer >
{
};

} // anonymous namespace

namespace chart
{
namespace wrapper
{

LegendWrapper::LegendWrapper(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
    : m_spChart2ModelContact(spChart2ModelContact)
    , m_aEventListenerContainer(m_aMutex)
{
}

LegendWrapper::~LegendWrapper()
{
}

// ____ XShape ____
awt::Point SAL_CALL LegendWrapper::getPosition()
{
    return m_spChart2ModelContact->GetLegendPosition();
}

void SAL_CALL LegendWrapper::setPosition( const awt::Point& aPosition )
{
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet() );
    if( xProp.is() )
    {
        awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );

        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_TOP_LEFT;
        aRelativePosition.Primary = aPageSize.Width == 0 ? 0 : double(aPosition.X)/double(aPageSize.Width);
        aRelativePosition.Secondary = aPageSize.Height == 0 ? 0 : double(aPosition.Y)/double(aPageSize.Height);
        xProp->setPropertyValue( "RelativePosition", uno::Any(aRelativePosition) );
    }
}

awt::Size SAL_CALL LegendWrapper::getSize()
{
    return m_spChart2ModelContact->GetLegendSize();
}

void SAL_CALL LegendWrapper::setSize( const awt::Size& aSize )
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
OUString SAL_CALL LegendWrapper::getShapeType()
{
    return OUString( "com.sun.star.chart.ChartLegend" );
}

// ____ XComponent ____
void SAL_CALL LegendWrapper::dispose()
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    MutexGuard aGuard( GetMutex());
    clearWrappedPropertySet();
}

void SAL_CALL LegendWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL LegendWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    m_aEventListenerContainer.removeInterface( aListener );
}

//ReferenceSizePropertyProvider
void LegendWrapper::updateReferenceSize()
{
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        if( xProp->getPropertyValue( "ReferencePageSize" ).hasValue() )
            xProp->setPropertyValue( "ReferencePageSize", uno::Any(
                m_spChart2ModelContact->GetPageSize() ));
    }
}
Any LegendWrapper::getReferenceSize()
{
    Any aRet;
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet(), uno::UNO_QUERY );
    if( xProp.is() )
        aRet = xProp->getPropertyValue( "ReferencePageSize" );

    return aRet;
}
awt::Size LegendWrapper::getCurrentSizeForReference()
{
    return m_spChart2ModelContact->GetPageSize();
}

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
    aWrappedProperties.push_back( new WrappedProperty( "Expansion", "Expansion"));
    WrappedCharacterHeightProperty::addWrappedProperties( aWrappedProperties, this );
    //same problem as for wall: the defaults in the old chart are different for different charttypes, so we need to export explicitly
    aWrappedProperties.push_back( new WrappedDirectStateProperty("FillStyle", "FillStyle"));
    aWrappedProperties.push_back( new WrappedDirectStateProperty("FillColor", "FillColor"));
    WrappedAutomaticPositionProperties::addWrappedProperties( aWrappedProperties );
    WrappedScaleTextProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    return aWrappedProperties;
}

OUString SAL_CALL LegendWrapper::getImplementationName()
{
    return OUString("com.sun.star.comp.chart.Legend");
}

sal_Bool SAL_CALL LegendWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LegendWrapper::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart.ChartLegend",
        "com.sun.star.drawing.Shape",
        "com.sun.star.xml.UserDefinedAttributesSupplier",
        "com.sun.star.style.CharacterProperties"
    };
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
