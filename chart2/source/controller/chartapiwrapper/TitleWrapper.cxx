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
#include "macros.hxx"
#include "ContainerHelper.hxx"
#include "ControllerLockGuard.hxx"

#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>

#include "CharacterProperties.hxx"
#include "LinePropertiesHelper.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "WrappedCharacterHeightProperty.hxx"
#include "WrappedTextRotationProperty.hxx"
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
class WrappedTitleStringProperty : public WrappedProperty
{
public:
    WrappedTitleStringProperty( const Reference< uno::XComponentContext >& xContext );
    virtual ~WrappedTitleStringProperty();

    virtual void setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                                    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException);
    virtual Any getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
    virtual Any getPropertyDefault( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

protected:
    Reference< uno::XComponentContext > m_xContext;
};

WrappedTitleStringProperty::WrappedTitleStringProperty( const Reference< uno::XComponentContext >& xContext )
    : ::chart::WrappedProperty( "String", OUString() )
    , m_xContext( xContext )
{
}
WrappedTitleStringProperty::~WrappedTitleStringProperty()
{
}

void WrappedTitleStringProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< chart2::XTitle > xTitle(xInnerPropertySet,uno::UNO_QUERY);
    if(xTitle.is())
    {
        OUString aString;
        rOuterValue >>= aString;
        TitleHelper::setCompleteString( aString, xTitle, m_xContext );
    }
}
Any WrappedTitleStringProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet( getPropertyDefault( Reference< beans::XPropertyState >( xInnerPropertySet, uno::UNO_QUERY ) ) );
    Reference< chart2::XTitle > xTitle(xInnerPropertySet,uno::UNO_QUERY);
    if(xTitle.is())
    {
        Sequence< Reference< chart2::XFormattedString > > aStrings( xTitle->getText());

        OUStringBuffer aBuf;
        for( sal_Int32 i = 0; i < aStrings.getLength(); ++i )
        {
            aBuf.append( aStrings[ i ]->getString());
        }
        aRet <<= aBuf.makeStringAndClear();
    }
    return aRet;
}
Any WrappedTitleStringProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( OUString() );//default title is a empty String
}

class WrappedStackedTextProperty : public WrappedProperty
{
public:
    WrappedStackedTextProperty();
    virtual ~WrappedStackedTextProperty();
};

WrappedStackedTextProperty::WrappedStackedTextProperty()
    : ::chart::WrappedProperty( "StackedText", "StackCharacters" )
{
}
WrappedStackedTextProperty::~WrappedStackedTextProperty()
{
}

}// end namespace chart

namespace
{
static const OUString lcl_aServiceName("com.sun.star.comp.chart.Title");

enum
{
    PROP_TITLE_STRING,
    PROP_TITLE_TEXT_ROTATION,
    PROP_TITLE_TEXT_STACKED
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "String",
                  PROP_TITLE_STRING,
                  ::getCppuType( reinterpret_cast< const OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "TextRotation",
                  PROP_TITLE_TEXT_ROTATION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "StackedText",
                  PROP_TITLE_TEXT_STACKED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticTitleWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence() );
        return &aPropSeq;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );
        ::chart::wrapper::WrappedAutomaticPositionProperties::addProperties( aProperties );
        ::chart::wrapper::WrappedScaleTextProperties::addProperties( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }
};

struct StaticTitleWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticTitleWrapperPropertyArray_Initializer >
{
};

} // anonymous namespace

namespace chart
{
namespace wrapper
{

TitleWrapper::TitleWrapper( ::chart::TitleHelper::eTitleType eTitleType,
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex ),
        m_eTitleType(eTitleType)
{
    ControllerLockGuard aCtrlLockGuard( Reference< frame::XModel >( m_spChart2ModelContact->getChart2Document(), uno::UNO_QUERY ));
    if( !getTitleObject().is() ) //#i83831# create an empty title at the model, thus references to properties can be mapped mapped correctly
        TitleHelper::createTitle( m_eTitleType, OUString(), m_spChart2ModelContact->getChartModel(), m_spChart2ModelContact->m_xContext );
}

TitleWrapper::~TitleWrapper()
{
}

// ____ XShape ____
awt::Point SAL_CALL TitleWrapper::getPosition()
    throw (uno::RuntimeException)
{
    return m_spChart2ModelContact->GetTitlePosition( this->getTitleObject() );
}

void SAL_CALL TitleWrapper::setPosition( const awt::Point& aPosition )
    throw (uno::RuntimeException)
{
    Reference< beans::XPropertySet > xPropertySet( this->getInnerPropertySet() );
    if(xPropertySet.is())
    {
        awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );

        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_TOP_LEFT;
        aRelativePosition.Primary = double(aPosition.X)/double(aPageSize.Width);
        aRelativePosition.Secondary = double(aPosition.Y)/double(aPageSize.Height);
        xPropertySet->setPropertyValue( "RelativePosition", uno::makeAny(aRelativePosition) );
    }
}

awt::Size SAL_CALL TitleWrapper::getSize()
    throw (uno::RuntimeException)
{
    return m_spChart2ModelContact->GetTitleSize( this->getTitleObject() );
}

void SAL_CALL TitleWrapper::setSize( const awt::Size& /*aSize*/ )
    throw (beans::PropertyVetoException,
           uno::RuntimeException)
{
    OSL_FAIL( "trying to set size of title" );
}

// ____ XShapeDescriptor (base of XShape) ____
OUString SAL_CALL TitleWrapper::getShapeType()
    throw (uno::RuntimeException)
{
    return OUString( "com.sun.star.chart.ChartTitle" );
}

// ____ XComponent ____
void SAL_CALL TitleWrapper::dispose()
    throw (uno::RuntimeException)
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    MutexGuard aGuard( GetMutex());
    clearWrappedPropertySet();
}

void SAL_CALL TitleWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL TitleWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

Reference< beans::XPropertySet > TitleWrapper::getFirstCharacterPropertySet()
{
    Reference< beans::XPropertySet > xProp;

    Reference< chart2::XTitle > xTitle( this->getTitleObject() );
    if( xTitle.is())
    {
        Sequence< Reference< chart2::XFormattedString > > aStrings( xTitle->getText());
        if( aStrings.getLength() > 0 )
            xProp.set( aStrings[0], uno::UNO_QUERY );
    }

    return xProp;
}

void TitleWrapper::getFastCharacterPropertyValue( sal_Int32 nHandle, Any& rValue )
{
    OSL_ASSERT( FAST_PROPERTY_ID_START_CHAR_PROP <= nHandle &&
                nHandle < CharacterProperties::FAST_PROPERTY_ID_END_CHAR_PROP );

    Reference< beans::XPropertySet > xProp( getFirstCharacterPropertySet(), uno::UNO_QUERY );
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
    sal_Int32 nHandle, const Any& rValue ) throw (uno::Exception)
{
    OSL_ASSERT( FAST_PROPERTY_ID_START_CHAR_PROP <= nHandle &&
                nHandle < CharacterProperties::FAST_PROPERTY_ID_END_CHAR_PROP );

    Reference< chart2::XTitle > xTitle( this->getTitleObject() );
    if( xTitle.is())
    {
        Sequence< Reference< chart2::XFormattedString > > aStrings( xTitle->getText());
        const WrappedProperty* pWrappedProperty = getWrappedProperty( nHandle );

        for( sal_Int32 i = 0; i < aStrings.getLength(); ++i )
        {
            Reference< beans::XFastPropertySet > xFastPropertySet( aStrings[ i ], uno::UNO_QUERY );
            Reference< beans::XPropertySet > xPropSet( xFastPropertySet, uno::UNO_QUERY );

            if( pWrappedProperty )
                pWrappedProperty->setPropertyValue( rValue, xPropSet );
            else if( xFastPropertySet.is() )
                xFastPropertySet->setFastPropertyValue( nHandle, rValue );
        }
    }
}

// WrappedPropertySet

void SAL_CALL TitleWrapper::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
                                    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
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
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
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
                                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    beans::PropertyState aState( beans::PropertyState_DIRECT_VALUE );

    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
    {
        Reference< beans::XPropertyState > xPropState( getFirstCharacterPropertySet(), uno::UNO_QUERY );
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
                                    throw (beans::UnknownPropertyException, uno::RuntimeException)
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
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;

    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
    {
        Reference< beans::XPropertyState > xPropState( getFirstCharacterPropertySet(), uno::UNO_QUERY );
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
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
    {
        Reference< beans::XPropertySet > xPropSet( getFirstCharacterPropertySet(), uno::UNO_QUERY );
        if( xPropSet.is() )
            xPropSet->addPropertyChangeListener( rPropertyName, xListener );
    }
    else
        WrappedPropertySet::addPropertyChangeListener( rPropertyName, xListener );
}
void SAL_CALL TitleWrapper::removePropertyChangeListener( const OUString& rPropertyName, const Reference< beans::XPropertyChangeListener >& xListener )
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    if( CharacterProperties::IsCharacterPropertyHandle( nHandle ) )
    {
        Reference< beans::XPropertySet > xPropSet( getFirstCharacterPropertySet(), uno::UNO_QUERY );
        if( xPropSet.is() )
            xPropSet->removePropertyChangeListener( rPropertyName, xListener );
    }
    else
        WrappedPropertySet::removePropertyChangeListener( rPropertyName, xListener );
}

//ReferenceSizePropertyProvider
void TitleWrapper::updateReferenceSize()
{
    Reference< beans::XPropertySet > xProp( this->getTitleObject(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        if( xProp->getPropertyValue( "ReferencePageSize" ).hasValue() )
            xProp->setPropertyValue( "ReferencePageSize", uno::makeAny(
                            m_spChart2ModelContact->GetPageSize() ));
    }
}
Any TitleWrapper::getReferenceSize()
{
    Any aRet;
    Reference< beans::XPropertySet > xProp( this->getTitleObject(), uno::UNO_QUERY );
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
    return TitleHelper::getTitle( m_eTitleType, m_spChart2ModelContact->getChartModel() );
}

// WrappedPropertySet

Reference< beans::XPropertySet > TitleWrapper::getInnerPropertySet()
{
    return Reference< beans::XPropertySet >( this->getTitleObject(), uno::UNO_QUERY );
}

const Sequence< beans::Property >& TitleWrapper::getPropertySequence()
{
    return *StaticTitleWrapperPropertyArray::get();
}

const std::vector< WrappedProperty* > TitleWrapper::createWrappedProperties()
{
    ::std::vector< ::chart::WrappedProperty* > aWrappedProperties;

    aWrappedProperties.push_back( new WrappedTitleStringProperty( m_spChart2ModelContact->m_xContext ) );
    aWrappedProperties.push_back( new WrappedTextRotationProperty( m_eTitleType==TitleHelper::Y_AXIS_TITLE || m_eTitleType==TitleHelper::X_AXIS_TITLE ) );
    aWrappedProperties.push_back( new WrappedStackedTextProperty() );
    WrappedCharacterHeightProperty::addWrappedProperties( aWrappedProperties, this );
    WrappedAutomaticPositionProperties::addWrappedProperties( aWrappedProperties );
    WrappedScaleTextProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    return aWrappedProperties;
}

Sequence< OUString > TitleWrapper::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 4 );
    aServices[ 0 ] = "com.sun.star.chart.ChartTitle";
    aServices[ 1 ] = "com.sun.star.drawing.Shape";
    aServices[ 2 ] = "com.sun.star.xml.UserDefinedAttributesSupplier";
    aServices[ 3 ] = "com.sun.star.style.CharacterProperties";

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( TitleWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
