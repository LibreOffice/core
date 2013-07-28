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

#include "ErrorBar.hxx"
#include "macros.hxx"
#include "LineProperties.hxx"
#include "ContainerHelper.hxx"
#include "EventListenerHelper.hxx"
#include "PropertyHelper.hxx"
#include "CloneHelper.hxx"

#include <svl/itemprop.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

namespace
{

static const OUString lcl_aServiceName( "com.sun.star.comp.chart2.ErrorBar" );

bool lcl_isInternalData( const uno::Reference< chart2::data::XLabeledDataSequence > & xLSeq )
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( xLSeq, uno::UNO_QUERY );
    return ( xServiceInfo.is() && xServiceInfo->getImplementationName() == "com.sun.star.comp.chart2.LabeledDataSequence" );
}

const SfxItemPropertySet* GetErrorBarPropertySet()
{
    static const SfxItemPropertyMapEntry aErrorBarPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN("ShowPositiveError"),0,&getBooleanCppuType(), 0, 0},
        {MAP_CHAR_LEN("ShowNegativeError"),1,&getBooleanCppuType(), 0, 0},
        {MAP_CHAR_LEN("PositiveError"),2,&getCppuType((const double*)0),0,0},
        {MAP_CHAR_LEN("NegativeError"),3,&getCppuType((const double*)0), 0, 0},
        {MAP_CHAR_LEN("PercentageError"),4,&getCppuType((const double*)0), 0, 0},
        {MAP_CHAR_LEN("ErrorBarStyle"),5,&getCppuType((sal_Int32*)0),0,0},
        {MAP_CHAR_LEN("ErrorBarRangePositive"),6,&getCppuType((OUString*)0),0,0}, // read-only for export
        {MAP_CHAR_LEN("ErrorBarRangeNegative"),7,&getCppuType((OUString*)0),0,0}, // read-only for export
        {MAP_CHAR_LEN("Weight"),8,&getCppuType((const double*)0),0,0},
        {MAP_CHAR_LEN("LineStyle"),9,&getCppuType((com::sun::star::drawing::LineStyle*)0),0,0},
        {MAP_CHAR_LEN("LineDash"),10,&getCppuType((drawing::LineDash*)0),0,0},
        {MAP_CHAR_LEN("LineWidth"),11,&getCppuType((sal_Int32*)0),0,0},
        {MAP_CHAR_LEN("LineColor"),12,&getCppuType((com::sun::star::util::Color*)0),0,0},
        {MAP_CHAR_LEN("LineTransparence"),13,&getCppuType((sal_Int16*)0),0,0},
        {MAP_CHAR_LEN("LineJoint"),14,&getCppuType((com::sun::star::drawing::LineJoint*)0),0,0},
        {0,0,0,0,0,0}
    };
    static SfxItemPropertySet aPropSet( aErrorBarPropertyMap_Impl );
    return &aPropSet;
}

} // anonymous namespace

namespace chart
{

uno::Reference< beans::XPropertySet > createErrorBar( const uno::Reference< uno::XComponentContext > & xContext )
{
    return new ErrorBar( xContext );
}

ErrorBar::ErrorBar(
    uno::Reference< uno::XComponentContext > const & xContext ) :
    LineProperties(),
    mbShowPositiveError(true),
    mbShowNegativeError(true),
    mfPositiveError(0),
    mfNegativeError(0),
    mfWeight(1),
    m_xContext( xContext ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

ErrorBar::ErrorBar( const ErrorBar & rOther ) :
    MutexContainer(),
    impl::ErrorBar_Base(),
    LineProperties(rOther),
    mbShowPositiveError(rOther.mbShowPositiveError),
    mbShowNegativeError(rOther.mbShowNegativeError),
    mfPositiveError(rOther.mfPositiveError),
    mfNegativeError(rOther.mfNegativeError),
    mfWeight(rOther.mfWeight),
    m_xContext( rOther.m_xContext ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    if( ! rOther.m_aDataSequences.empty())
    {
        if( lcl_isInternalData( rOther.m_aDataSequences.front()))
            CloneHelper::CloneRefVector< tDataSequenceContainer::value_type >(
                rOther.m_aDataSequences, m_aDataSequences );
        else
            m_aDataSequences = rOther.m_aDataSequences;
        ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
    }
}

ErrorBar::~ErrorBar()
{}

uno::Reference< util::XCloneable > SAL_CALL ErrorBar::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new ErrorBar( *this ));
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ErrorBar::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > aRef (
            new SfxItemPropertySetInfo( GetErrorBarPropertySet()->getPropertyMap() ) );
    return aRef;
}

void ErrorBar::setPropertyValue( const OUString& rPropName, const uno::Any& rAny )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if(rPropName == "ErrorBarStyle")
        rAny >>= meStyle;
    else if(rPropName == "PositiveError")
        rAny >>= mfPositiveError;
    else if(rPropName == "PercentageError")
    {
        rAny >>= mfPositiveError;
        rAny >>= mfNegativeError;
    }
    else if(rPropName == "Weight")
    {
        rAny >>= mfWeight;
    }
    else if(rPropName == "NegativeError")
        rAny >>= mfNegativeError;
    else if(rPropName == "ShowPositiveError")
        rAny >>= mbShowPositiveError;
    else if(rPropName == "ShowNegativeError")
        rAny >>= mbShowNegativeError;
    else if(rPropName == "ErrorBarRangePositive" || rPropName == "ErrorBarRangeNegative")
        throw uno::RuntimeException("read-only property", static_cast< uno::XWeak*>(this));
    else
        LineProperties::setPropertyValue(rPropName, rAny);

    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

namespace {

OUString getSourceRangeStrFromLabeledSequences( uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences, bool bPositive )
{
    const OUString aRolePrefix( "error-bars" );
    OUString aDirection;
    if(bPositive)
        aDirection = "positive";
    else
        aDirection = "negative";

    for( sal_Int32 nI=0; nI< aSequences.getLength(); ++nI )
    {
        try
        {
            if( aSequences[nI].is())
            {
                uno::Reference< chart2::data::XDataSequence > xSequence( aSequences[nI]->getValues());
                uno::Reference< beans::XPropertySet > xSeqProp( xSequence, uno::UNO_QUERY_THROW );
                OUString aRole;
                if( ( xSeqProp->getPropertyValue(
                                OUString( "Role" )) >>= aRole ) &&
                        aRole.match( aRolePrefix ) && aRole.indexOf(aDirection) >= 0 )
                {
                    return xSequence->getSourceRangeRepresentation();
                }
            }
        }
        catch (...)
        {
            // we can't be sure that this is 100% safe and we don't want to kill the export
            // we should at least check why the exception is thrown
            SAL_WARN("chart2", "unexpected exception!");
        }
    }

    return OUString();
}

}

uno::Any ErrorBar::getPropertyValue(const OUString& rPropName)
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;
    if(rPropName == "ErrorBarStyle")
        aRet <<= meStyle;
    else if(rPropName == "PositiveError")
        aRet <<= mfPositiveError;
    else if(rPropName == "NegativeError")
        aRet <<= mfNegativeError;
    else if(rPropName == "PercentageError")
        aRet <<= mfPositiveError;
    else if(rPropName == "ShowPositiveError")
        aRet <<= mbShowPositiveError;
    else if(rPropName == "ShowNegativeError")
        aRet <<= mbShowNegativeError;
    else if(rPropName == "Weight")
        aRet <<= mfWeight;
    else if(rPropName == "ErrorBarRangePositive")
    {
        OUString aRange;
        if(meStyle == com::sun::star::chart::ErrorBarStyle::FROM_DATA)
        {
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences =
                getDataSequences();

            aRange = getSourceRangeStrFromLabeledSequences( aSequences, true );
        }

        aRet <<= aRange;
    }
    else if(rPropName == "ErrorBarRangeNegative")
    {
        OUString aRange;
        if(meStyle == com::sun::star::chart::ErrorBarStyle::FROM_DATA)
        {
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences =
                getDataSequences();

            aRange = getSourceRangeStrFromLabeledSequences( aSequences, true );
        }

        aRet <<= aRange;
    }
    else
        aRet = LineProperties::getPropertyValue(rPropName);

    SAL_WARN_IF(!aRet.hasValue(), "chart2", "asked for property value: " << rPropName);
    return aRet;
}

beans::PropertyState ErrorBar::getPropertyState( const OUString& rPropName )
        throw (com::sun::star::beans::UnknownPropertyException)
{
    if(rPropName == "ErrorBarStyle")
    {
        if(meStyle == com::sun::star::chart::ErrorBarStyle::NONE)
            return beans::PropertyState_DEFAULT_VALUE;
        return beans::PropertyState_DIRECT_VALUE;
    }
    else if(rPropName == "PositiveError")
    {
        if(mbShowPositiveError)
        {
            switch(meStyle)
            {
                case com::sun::star::chart::ErrorBarStyle::ABSOLUTE:
                case com::sun::star::chart::ErrorBarStyle::ERROR_MARGIN:
                    return beans::PropertyState_DIRECT_VALUE;
                default:
                    break;
            }
        }
        return beans::PropertyState_DEFAULT_VALUE;
    }
    else if(rPropName == "NegativeError")
    {
        if(mbShowNegativeError)
        {
            switch(meStyle)
            {
                case com::sun::star::chart::ErrorBarStyle::ABSOLUTE:
                case com::sun::star::chart::ErrorBarStyle::ERROR_MARGIN:
                    return beans::PropertyState_DIRECT_VALUE;
                default:
                    break;
            }
        }
        return beans::PropertyState_DEFAULT_VALUE;
    }
    else if(rPropName == "PercentageError")
    {
        if(meStyle != com::sun::star::chart::ErrorBarStyle::RELATIVE)
            return beans::PropertyState_DEFAULT_VALUE;
        return beans::PropertyState_DIRECT_VALUE;
    }
    else if(rPropName == "ShowPositiveError")
    {
        // this value should be never default
        return beans::PropertyState_DIRECT_VALUE;
    }
    else if(rPropName == "ShowNegativeError")
    {
        // this value should be never default
        return beans::PropertyState_DIRECT_VALUE;
    }
    else if(rPropName == "ErrorBarRangePositive")
    {
        if(meStyle == com::sun::star::chart::ErrorBarStyle::FROM_DATA && mbShowPositiveError)
            return beans::PropertyState_DIRECT_VALUE;
        return beans::PropertyState_DEFAULT_VALUE;
    }
    else if(rPropName == "ErrorBarRangeNegative")
    {
        if(meStyle == com::sun::star::chart::ErrorBarStyle::FROM_DATA && mbShowNegativeError)
            return beans::PropertyState_DIRECT_VALUE;
        return beans::PropertyState_DEFAULT_VALUE;
    }
    else
        return beans::PropertyState_DIRECT_VALUE;
}

uno::Sequence< beans::PropertyState > ErrorBar::getPropertyStates( const uno::Sequence< OUString >& rPropNames )
        throw (com::sun::star::beans::UnknownPropertyException)
{
    uno::Sequence< beans::PropertyState > aRet( rPropNames.getLength() );
    for(sal_Int32 i = 0; i < rPropNames.getLength(); ++i)
    {
        aRet[i] = getPropertyState(rPropNames[i]);
    }
    return aRet;
}

void ErrorBar::setPropertyToDefault( const OUString& )
        throw (beans::UnknownPropertyException)
{
    //keep them unimplemented for now
}

uno::Any ErrorBar::getPropertyDefault( const OUString& )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException)
{
    //keep them unimplemented for now
    return uno::Any();
}

void ErrorBar::addPropertyChangeListener( const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& )
    throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
}

void ErrorBar::removePropertyChangeListener( const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& )
    throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
}

void ErrorBar::addVetoableChangeListener( const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& )
    throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
}

void ErrorBar::removeVetoableChangeListener( const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& )
    throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
}

// ____ XModifyBroadcaster ____
void SAL_CALL ErrorBar::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL ErrorBar::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XModifyListener ____
void SAL_CALL ErrorBar::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ErrorBar::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ XDataSink ____
void SAL_CALL ErrorBar::setData( const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > >& aData )
    throw (uno::RuntimeException)
{
    ModifyListenerHelper::removeListenerFromAllElements( m_aDataSequences, m_xModifyEventForwarder );
    EventListenerHelper::removeListenerFromAllElements( m_aDataSequences, this );
    m_aDataSequences = ContainerHelper::SequenceToVector( aData );
    EventListenerHelper::addListenerToAllElements( m_aDataSequences, this );
    ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
}

// ____ XDataSource ____
uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > SAL_CALL ErrorBar::getDataSequences()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( m_aDataSequences );
}

// ____ XChild ____
uno::Reference< uno::XInterface > SAL_CALL ErrorBar::getParent()
    throw (uno::RuntimeException)
{
    return m_xParent;
}

void SAL_CALL ErrorBar::setParent(
    const uno::Reference< uno::XInterface >& Parent )
    throw (lang::NoSupportException,
           uno::RuntimeException)
{
    m_xParent.set( Parent );
}

uno::Sequence< OUString > ErrorBar::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = "com.sun.star.chart2.ErrorBar";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ErrorBar, lcl_aServiceName );

// needed by MSC compiler
using impl::ErrorBar_Base;

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
