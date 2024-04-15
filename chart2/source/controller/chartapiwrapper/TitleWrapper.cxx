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

#include "TitleWrapper.hxx"
#include "Chart2ModelContact.hxx"
#include <ControllerLockGuard.hxx>

#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/XTitle.hpp>

#include <CharacterProperties.hxx>
#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <UserDefinedProperties.hxx>
#include "WrappedCharacterHeightProperty.hxx"
#include "WrappedTextRotationProperty.hxx"
#include "WrappedAutomaticPositionProperties.hxx"
#include "WrappedScaleTextProperties.hxx"

#include <algorithm>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/propshlp.hxx>
#include <utility>

using namespace ::com::sun::star;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{
namespace {

class WrappedTitleStringProperty : public WrappedProperty
{
public:
    explicit WrappedTitleStringProperty( const Reference< uno::XComponentContext >& xContext );

    virtual void setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const override;
    virtual Any getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const override;
    virtual Any getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const override;

protected:
    Reference< uno::XComponentContext > m_xContext;
};

}

WrappedTitleStringProperty::WrappedTitleStringProperty( const Reference< uno::XComponentContext >& xContext )
    : ::chart::WrappedProperty( "String", OUString() )
    , m_xContext( xContext )
{
}

void WrappedTitleStringProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Title* pTitle = dynamic_cast<Title*>(xInnerPropertySet.get());
    if(pTitle)
    {
        OUString aString;
        rOuterValue >>= aString;
        TitleHelper::setCompleteString( aString, pTitle, m_xContext );
    }
}
Any WrappedTitleStringProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Any aRet( getPropertyDefault( Reference< beans::XPropertyState >( xInnerPropertySet, uno::UNO_QUERY ) ) );
    Reference< chart2::XTitle > xTitle(xInnerPropertySet,uno::UNO_QUERY);
    if(xTitle.is())
    {
        const Sequence< Reference< chart2::XFormattedString > > aStrings( xTitle->getText());

        OUStringBuffer aBuf;
        for( Reference< chart2::XFormattedString > const & formattedStr : aStrings )
        {
            aBuf.append( formattedStr->getString());
        }
        aRet <<= aBuf.makeStringAndClear();
    }
    return aRet;
}
Any WrappedTitleStringProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    return uno::Any( OUString() );//default title is an empty String
}

namespace {

    class WrappedTitleFormStringsProperty : public WrappedProperty
    {
        public:
            explicit WrappedTitleFormStringsProperty();

            virtual void setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const override;
            virtual Any getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const override;
            virtual Any getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const override;

        protected:
            Reference< uno::XComponentContext > m_xContext;
    };

}

WrappedTitleFormStringsProperty::WrappedTitleFormStringsProperty()
    : ::chart::WrappedProperty( "FormattedStrings", OUString() )
{
}

void WrappedTitleFormStringsProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Title* pTitle = dynamic_cast<Title*>(xInnerPropertySet.get());
    if (pTitle)
    {
        Sequence< Reference< chart2::XFormattedString >> xFormattedStrings;
        rOuterValue >>= xFormattedStrings;
        TitleHelper::setFormattedString(pTitle, xFormattedStrings);
    }
}
Any WrappedTitleFormStringsProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Any aRet(getPropertyDefault(Reference< beans::XPropertyState >(xInnerPropertySet, uno::UNO_QUERY)));
    Reference< chart2::XTitle > xTitle(xInnerPropertySet, uno::UNO_QUERY);
    if (xTitle.is())
    {
        const Sequence< Reference< chart2::XFormattedString > > aStrings(xTitle->getText());
        aRet <<= aStrings;
    }
    return aRet;
}
Any WrappedTitleFormStringsProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    return uno::Any(Sequence< Reference< chart2::XFormattedString > >()); //default title is an empty Sequence of XFormattedStrings
}

namespace {

class WrappedStackedTextProperty : public WrappedProperty
{
public:
    WrappedStackedTextProperty();
};

}

WrappedStackedTextProperty::WrappedStackedTextProperty()
    : ::chart::WrappedProperty( "StackedText", "StackCharacters" )
{
}

}// end namespace chart

namespace
{

enum
{
    PROP_TITLE_STRING,
    PROP_TITLE_FORMATTED_STRINGS,
    PROP_TITLE_VISIBLE,
    PROP_TITLE_TEXT_ROTATION,
    PROP_TITLE_TEXT_STACKED
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "String",
                  PROP_TITLE_STRING,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "FormattedStrings",
                  PROP_TITLE_FORMATTED_STRINGS,
                  cppu::UnoType< Sequence< Reference< chart2::XFormattedString >>>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "Visible",
                  PROP_TITLE_VISIBLE,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "TextRotation",
                  PROP_TITLE_TEXT_ROTATION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "StackedText",
                  PROP_TITLE_TEXT_STACKED,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

const Sequence< Property > & StaticTitleWrapperPropertyArray()
{
    static Sequence< Property > aPropSeq = []()
        {
            std::vector< beans::Property > aProperties;
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

TitleWrapper::TitleWrapper( ::chart::TitleHelper::eTitleType eTitleType,
    std::shared_ptr<Chart2ModelContact> spChart2ModelContact ) :
        m_spChart2ModelContact(std::move( spChart2ModelContact )),
        m_eTitleType(eTitleType)
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getDocumentModel() );
    if( !getTitleObject().is() ) //#i83831# create an empty title at the model, thus references to properties can be mapped correctly
        TitleHelper::createTitle( m_eTitleType, OUString(), m_spChart2ModelContact->getDocumentModel(), m_spChart2ModelContact->m_xContext );
}

TitleWrapper::~TitleWrapper()
{
}

// ____ XShape ____
awt::Point SAL_CALL TitleWrapper::getPosition()
{
    return m_spChart2ModelContact->GetTitlePosition( getTitleObject() );
}

void SAL_CALL TitleWrapper::setPosition( const awt::Point& aPosition )
{
    Reference< beans::XPropertySet > xPropertySet( getInnerPropertySet() );
    if(xPropertySet.is())
    {
        awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );

        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_TOP_LEFT;
        aRelativePosition.Primary = double(aPosition.X)/double(aPageSize.Width);
        aRelativePosition.Secondary = double(aPosition.Y)/double(aPageSize.Height);
        xPropertySet->setPropertyValue( "RelativePosition", uno::Any(aRelativePosition) );
    }
}

awt::Size SAL_CALL TitleWrapper::getSize()
{
    return m_spChart2ModelContact->GetTitleSize( getTitleObject() );
}

void SAL_CALL TitleWrapper::setSize( const awt::Size& /*aSize*/ )
{
    OSL_FAIL( "trying to set size of title" );
}

// ____ XShapeDescriptor (base of XShape) ____
OUString SAL_CALL TitleWrapper::getShapeType()
{
    return "com.sun.star.chart.ChartTitle";
}

// ____ XComponent ____
void SAL_CALL TitleWrapper::dispose()
{
    std::unique_lock g(m_aMutex);
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( g, lang::EventObject( xSource ) );

    clearWrappedPropertySet();
}

void SAL_CALL TitleWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.addInterface( g, xListener );
}

void SAL_CALL TitleWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.removeInterface( g, aListener );
}

void TitleWrapper::getFastCharacterPropertyValue( sal_Int32 nHandle, Any& rValue )
{
    OSL_ASSERT( FAST_PROPERTY_ID_START_CHAR_PROP <= nHandle &&
                nHandle < CharacterProperties::FAST_PROPERTY_ID_END_CHAR_PROP );

    Reference< beans::XPropertySet > xProp = getInnerPropertySet();
    Reference< beans::XFastPropertySet > xFastProp( xProp, uno::UNO_QUERY );
    if(xProp.is())
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( nHandle );
        if( pWrappedProperty )
        {
            rValue = pWrappedProperty->getPropertyValue( xProp );
        }
        else if( xFastProp.is() )
        {
            rValue = xFastProp->getFastPropertyValue( nHandle );
        }
    }

}

void TitleWrapper::setFastCharacterPropertyValue(
    sal_Int32 nHandle, const Any& rValue )
{
    OSL_ASSERT( FAST_PROPERTY_ID_START_CHAR_PROP <= nHandle &&
                nHandle < CharacterProperties::FAST_PROPERTY_ID_END_CHAR_PROP );

    Reference< chart2::XTitle > xTitle( getTitleObject() );
    if( !xTitle.is())
        return;

    const Sequence< Reference< chart2::XFormattedString > > aStrings( xTitle->getText());
    const WrappedProperty* pWrappedProperty = getWrappedProperty( nHandle );

    for( Reference< chart2::XFormattedString > const & formattedStr : aStrings )
    {
        Reference< beans::XFastPropertySet > xFastPropertySet( formattedStr, uno::UNO_QUERY );
        Reference< beans::XPropertySet > xPropSet( xFastPropertySet, uno::UNO_QUERY );

        if( pWrappedProperty )
            pWrappedProperty->setPropertyValue( rValue, xPropSet );
        else if( xFastPropertySet.is() )
            xFastPropertySet->setFastPropertyValue( nHandle, rValue );
    }

    Reference< beans::XPropertySet > xInnerProp = getInnerPropertySet();
    Reference< beans::XFastPropertySet > xFastInnerProp( xInnerProp, uno::UNO_QUERY );
    if (xInnerProp.is())
    {
        if (pWrappedProperty)
            pWrappedProperty->setPropertyValue(rValue, xInnerProp);
        else if (xFastInnerProp.is())
            xFastInnerProp->setFastPropertyValue(nHandle, rValue);
    }
}

// WrappedPropertySet

void SAL_CALL TitleWrapper::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
{
    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
    {
        setFastCharacterPropertyValue( nHandle, rValue );
    }
    else
        WrappedPropertySet::setPropertyValue( rPropertyName, rValue );
}

Any SAL_CALL TitleWrapper::getPropertyValue( const OUString& rPropertyName )
{
    Any aRet;
    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
        getFastCharacterPropertyValue( nHandle, aRet );
    else
        aRet = WrappedPropertySet::getPropertyValue( rPropertyName );
    return aRet;
}

beans::PropertyState SAL_CALL TitleWrapper::getPropertyState( const OUString& rPropertyName )
{
    beans::PropertyState aState( beans::PropertyState_DIRECT_VALUE );

    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
    {
        Reference< beans::XPropertyState > xPropState( getInnerPropertySet(), uno::UNO_QUERY);
        if( xPropState.is() )
        {
            const WrappedProperty* pWrappedProperty = getWrappedProperty( rPropertyName );
            if( pWrappedProperty )
                aState = pWrappedProperty->getPropertyState( xPropState );
            else
                aState = xPropState->getPropertyState( rPropertyName );
        }
    }
    else
        aState = WrappedPropertySet::getPropertyState( rPropertyName );

    return aState;
}
void SAL_CALL TitleWrapper::setPropertyToDefault( const OUString& rPropertyName )
{
    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
    {
        Any aDefault = getPropertyDefault( rPropertyName );
        setFastCharacterPropertyValue( nHandle, aDefault );
    }
    else
        WrappedPropertySet::setPropertyToDefault( rPropertyName );
}
Any SAL_CALL TitleWrapper::getPropertyDefault( const OUString& rPropertyName )
{
    Any aRet;

    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
    {
        Reference< beans::XPropertyState > xPropState( getInnerPropertySet(), uno::UNO_QUERY );
        if( xPropState.is() )
        {
            const WrappedProperty* pWrappedProperty = getWrappedProperty( rPropertyName );
            if( pWrappedProperty )
                aRet = pWrappedProperty->getPropertyDefault(xPropState);
            else
                aRet = xPropState->getPropertyDefault( rPropertyName );
        }
    }
    else
        aRet = WrappedPropertySet::getPropertyDefault( rPropertyName );

    return aRet;
}

void SAL_CALL TitleWrapper::addPropertyChangeListener( const OUString& rPropertyName, const Reference< beans::XPropertyChangeListener >& xListener )
{
    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
    {
        Reference< beans::XPropertySet > xPropSet = getInnerPropertySet();
        if( xPropSet.is() )
            xPropSet->addPropertyChangeListener( rPropertyName, xListener );
    }
    else
        WrappedPropertySet::addPropertyChangeListener( rPropertyName, xListener );
}
void SAL_CALL TitleWrapper::removePropertyChangeListener( const OUString& rPropertyName, const Reference< beans::XPropertyChangeListener >& xListener )
{
    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
    {
        Reference< beans::XPropertySet > xPropSet = getInnerPropertySet();
        if( xPropSet.is() )
            xPropSet->removePropertyChangeListener( rPropertyName, xListener );
    }
    else
        WrappedPropertySet::removePropertyChangeListener( rPropertyName, xListener );
}

//ReferenceSizePropertyProvider
void TitleWrapper::updateReferenceSize()
{
    Reference< beans::XPropertySet > xProp( getTitleObject(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        if( xProp->getPropertyValue( "ReferencePageSize" ).hasValue() )
            xProp->setPropertyValue( "ReferencePageSize", uno::Any(
                            m_spChart2ModelContact->GetPageSize() ));
    }
}
Any TitleWrapper::getReferenceSize()
{
    Any aRet;
    Reference< beans::XPropertySet > xProp( getTitleObject(), uno::UNO_QUERY );
    if( xProp.is() )
        aRet = xProp->getPropertyValue( "ReferencePageSize" );

    return aRet;
}
awt::Size TitleWrapper::getCurrentSizeForReference()
{
    return m_spChart2ModelContact->GetPageSize();
}

Reference< chart2::XTitle > TitleWrapper::getTitleObject()
{
    return TitleHelper::getTitle( m_eTitleType, m_spChart2ModelContact->getDocumentModel() );
}

// WrappedPropertySet

Reference< beans::XPropertySet > TitleWrapper::getInnerPropertySet()
{
    return Reference< beans::XPropertySet >( getTitleObject(), uno::UNO_QUERY );
}

const Sequence< beans::Property >& TitleWrapper::getPropertySequence()
{
    return StaticTitleWrapperPropertyArray();
}

std::vector< std::unique_ptr<WrappedProperty> > TitleWrapper::createWrappedProperties()
{
    std::vector< std::unique_ptr<WrappedProperty> > aWrappedProperties;

    aWrappedProperties.emplace_back( new WrappedTitleStringProperty( m_spChart2ModelContact->m_xContext ) );
    aWrappedProperties.emplace_back( new WrappedTitleFormStringsProperty() );
    aWrappedProperties.emplace_back( new WrappedTextRotationProperty( true ) );
    aWrappedProperties.emplace_back( new WrappedStackedTextProperty() );
    WrappedCharacterHeightProperty::addWrappedProperties( aWrappedProperties, this );
    WrappedAutomaticPositionProperties::addWrappedProperties( aWrappedProperties );
    WrappedScaleTextProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    return aWrappedProperties;
}

OUString SAL_CALL TitleWrapper::getImplementationName()
{
    return "com.sun.star.comp.chart.Title";
}

sal_Bool SAL_CALL TitleWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL TitleWrapper::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart.ChartTitle",
        "com.sun.star.drawing.Shape",
        "com.sun.star.xml.UserDefinedAttributesSupplier",
         "com.sun.star.style.CharacterProperties"
    };
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
