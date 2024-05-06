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
#include "Chart2ModelContact.hxx"
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>

#include <CharacterProperties.hxx>
#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <UserDefinedProperties.hxx>
#include "WrappedCharacterHeightProperty.hxx"
#include <PositionAndSizeHelper.hxx>
#include <WrappedDirectStateProperty.hxx>
#include "WrappedAutomaticPositionProperties.hxx"
#include "WrappedScaleTextProperties.hxx"

#include <algorithm>
#include <utility>

using namespace ::com::sun::star;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{
namespace {

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

}

WrappedLegendAlignmentProperty::WrappedLegendAlignmentProperty()
    : ::chart::WrappedProperty( u"Alignment"_ustr, u"AnchorPosition"_ustr )
{
}

Any WrappedLegendAlignmentProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Any aRet;
    if( xInnerPropertySet.is() )
    {
        bool bShowLegend = true;
        xInnerPropertySet->getPropertyValue( u"Show"_ustr ) >>= bShowLegend;
        if(!bShowLegend)
        {
            aRet <<= css::chart::ChartLegendPosition_NONE;
        }
        else
        {
            aRet = xInnerPropertySet->getPropertyValue( m_aInnerName );
            aRet = convertInnerToOuterValue( aRet );
        }
    }
    return aRet;
}

void WrappedLegendAlignmentProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    if(!xInnerPropertySet.is())
        return;

    bool bNewShowLegend = true;
    bool bOldShowLegend = true;
    {
        css::chart::ChartLegendPosition eOuterPos(css::chart::ChartLegendPosition_NONE);
        if( (rOuterValue >>= eOuterPos)  && eOuterPos == css::chart::ChartLegendPosition_NONE )
            bNewShowLegend = false;
        xInnerPropertySet->getPropertyValue( u"Show"_ustr ) >>= bOldShowLegend;
    }
    if(bNewShowLegend!=bOldShowLegend)
    {
        xInnerPropertySet->setPropertyValue( u"Show"_ustr, uno::Any(bNewShowLegend) );
    }
    if(!bNewShowLegend)
        return;

    //set corresponding LegendPosition
    Any aInnerValue = convertOuterToInnerValue( rOuterValue );
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
            xInnerPropertySet->getPropertyValue( u"Expansion"_ustr ) >>= eOldExpansion );

        if( !bExpansionWasSet || (eOldExpansion != eNewExpansion))
            xInnerPropertySet->setPropertyValue( u"Expansion"_ustr, uno::Any( eNewExpansion ));
    }

    //correct RelativePosition
    Any aRelativePosition( xInnerPropertySet->getPropertyValue(u"RelativePosition"_ustr) );
    if(aRelativePosition.hasValue())
    {
        xInnerPropertySet->setPropertyValue( u"RelativePosition"_ustr, Any() );
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
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "Alignment",
                  PROP_LEGEND_ALIGNMENT,
                  cppu::UnoType<css::chart::ChartLegendPosition>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "Expansion",
                  PROP_LEGEND_EXPANSION,
                  cppu::UnoType<css::chart::ChartLegendExpansion>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );
}

const Sequence< Property >& StaticLegendWrapperPropertyArray()
{
    static Sequence< Property > aPropSeq = []()
        {
            std::vector< css::beans::Property > aProperties;
            lcl_AddPropertiesToVector( aProperties );
            ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
            ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
            ::chart::FillProperties::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );
            ::chart::wrapper::WrappedAutomaticPositionProperties::addProperties( aProperties );
            ::chart::wrapper::WrappedScaleTextProperties::addProperties( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropSeq;
};

} // anonymous namespace

namespace chart::wrapper
{

LegendWrapper::LegendWrapper(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
    : m_spChart2ModelContact(std::move(spChart2ModelContact))
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
    Reference< beans::XPropertySet > xProp( getInnerPropertySet() );
    if( xProp.is() )
    {
        awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );

        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_TOP_LEFT;
        aRelativePosition.Primary = aPageSize.Width == 0 ? 0 : double(aPosition.X)/double(aPageSize.Width);
        aRelativePosition.Secondary = aPageSize.Height == 0 ? 0 : double(aPosition.Y)/double(aPageSize.Height);
        xProp->setPropertyValue( u"RelativePosition"_ustr, uno::Any(aRelativePosition) );
    }
}

awt::Size SAL_CALL LegendWrapper::getSize()
{
    return m_spChart2ModelContact->GetLegendSize();
}

void SAL_CALL LegendWrapper::setSize( const awt::Size& aSize )
{
    Reference< beans::XPropertySet > xProp( getInnerPropertySet() );
    if( xProp.is() )
    {
        awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );
        awt::Rectangle aPageRectangle( 0,0,aPageSize.Width,aPageSize.Height);

        awt::Point aPos( getPosition() );
        awt::Rectangle aNewPositionAndSize(aPos.X,aPos.Y,aSize.Width,aSize.Height);

        PositionAndSizeHelper::moveObject( OBJECTTYPE_LEGEND
                , xProp, aNewPositionAndSize, awt::Rectangle(), aPageRectangle );
    }
}

// ____ XShapeDescriptor (base of XShape) ____
OUString SAL_CALL LegendWrapper::getShapeType()
{
    return u"com.sun.star.chart.ChartLegend"_ustr;
}

// ____ XComponent ____
void SAL_CALL LegendWrapper::dispose()
{
    std::unique_lock g(m_aMutex);
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( g, lang::EventObject( xSource ) );

    clearWrappedPropertySet();
}

void SAL_CALL LegendWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.addInterface( g, xListener );
}

void SAL_CALL LegendWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.removeInterface( g, aListener );
}

//ReferenceSizePropertyProvider
void LegendWrapper::updateReferenceSize()
{
    Reference< beans::XPropertySet > xProp = getInnerPropertySet();
    if( xProp.is() )
    {
        if( xProp->getPropertyValue( u"ReferencePageSize"_ustr ).hasValue() )
            xProp->setPropertyValue( u"ReferencePageSize"_ustr, uno::Any(
                m_spChart2ModelContact->GetPageSize() ));
    }
}
Any LegendWrapper::getReferenceSize()
{
    Any aRet;
    Reference< beans::XPropertySet > xProp = getInnerPropertySet();
    if( xProp.is() )
        aRet = xProp->getPropertyValue( u"ReferencePageSize"_ustr );

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
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( xDiagram.is() )
        xRet.set( xDiagram->getLegend(), uno::UNO_QUERY );
    OSL_ENSURE(xRet.is(),"LegendWrapper::getInnerPropertySet() is NULL");
    return xRet;
}

const Sequence< beans::Property >& LegendWrapper::getPropertySequence()
{
    return StaticLegendWrapperPropertyArray();
}

std::vector< std::unique_ptr<WrappedProperty> > LegendWrapper::createWrappedProperties()
{
    std::vector< std::unique_ptr<WrappedProperty> > aWrappedProperties;

    aWrappedProperties.emplace_back( new WrappedLegendAlignmentProperty() );
    aWrappedProperties.emplace_back( new WrappedProperty( u"Expansion"_ustr, u"Expansion"_ustr));
    WrappedCharacterHeightProperty::addWrappedProperties( aWrappedProperties, this );
    //same problem as for wall: the defaults in the old chart are different for different charttypes, so we need to export explicitly
    aWrappedProperties.emplace_back( new WrappedDirectStateProperty(u"FillStyle"_ustr, u"FillStyle"_ustr));
    aWrappedProperties.emplace_back( new WrappedDirectStateProperty(u"FillColor"_ustr, u"FillColor"_ustr));
    WrappedAutomaticPositionProperties::addWrappedProperties( aWrappedProperties );
    WrappedScaleTextProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    return aWrappedProperties;
}

OUString SAL_CALL LegendWrapper::getImplementationName()
{
    return u"com.sun.star.comp.chart.Legend"_ustr;
}

sal_Bool SAL_CALL LegendWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LegendWrapper::getSupportedServiceNames()
{
    return {
        u"com.sun.star.chart.ChartLegend"_ustr,
        u"com.sun.star.drawing.Shape"_ustr,
        u"com.sun.star.xml.UserDefinedAttributesSupplier"_ustr,
        u"com.sun.star.style.CharacterProperties"_ustr
    };
}

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
