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

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

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
    static SfxItemPropertyMapEntry aErrorBarPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN("ShowPositiveError"),0,&getBooleanCppuType(), 0, 0},
        {MAP_CHAR_LEN("ShowNegativeError"),1,&getBooleanCppuType(), 0, 0},
        {MAP_CHAR_LEN("PositiveError"),2,&getCppuType((const double*)0),0,0},
        {MAP_CHAR_LEN("NegativeError"),3,&getCppuType((const double*)0), 0, 0},
        {MAP_CHAR_LEN("ErrorBarStyle"),4,&getCppuType((sal_Int32*)0),0,0},
        {MAP_CHAR_LEN("Weight"),5,&getCppuType((const double*)0),0,0},
        {MAP_CHAR_LEN(""),6,&getBooleanCppuType(), 0, 0},
        {MAP_CHAR_LEN(""),7,&getCppuType((OUString*)0),0,0},
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
    if(rPropName == "ErrorBarStyle")
        rAny >>= meStyle;
    else if(rPropName == "PositiveError")
        rAny >>= mfPositiveError;
    else if(rPropName == "NegativeError")
        rAny >>= mfNegativeError;
    else if(rPropName == "ShowPositiveError")
        rAny >>= mbShowPositiveError;
    else if(rPropName == "ShowNegativeError")
        rAny >>= mbShowNegativeError;
    else
        SAL_WARN("chart2", "asked for property value: " << rPropName);
}

uno::Any ErrorBar::getPropertyValue(const OUString& rPropName)
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    if(rPropName == "ErrorBarStyle")
        aRet <<= meStyle;
    else if(rPropName == "PositiveError")
        aRet <<= mfPositiveError;
    else if(rPropName == "NegativeError")
        aRet <<= mfNegativeError;
    else if(rPropName == "ShowPositiveError")
        aRet <<= mbShowPositiveError;
    else if(rPropName == "ShowNegativeError")
        aRet <<= mbShowNegativeError;
    else
        SAL_WARN("chart2", "asked for property value: " << rPropName);
    return aRet;
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

// ================================================================================

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
