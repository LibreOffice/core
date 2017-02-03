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
#include "ContainerHelper.hxx"
#include "EventListenerHelper.hxx"
#include "PropertyHelper.hxx"
#include "CloneHelper.hxx"

#include <cppuhelper/supportsservice.hxx>
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

static const char lcl_aServiceName[] = "com.sun.star.comp.chart2.ErrorBar";

bool lcl_isInternalData( const uno::Reference< chart2::data::XLabeledDataSequence > & xLSeq )
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( xLSeq, uno::UNO_QUERY );
    return ( xServiceInfo.is() && xServiceInfo->getImplementationName() == "com.sun.star.comp.chart2.LabeledDataSequence" );
}

const SfxItemPropertySet* GetErrorBarPropertySet()
{
    static const SfxItemPropertyMapEntry aErrorBarPropertyMap_Impl[] =
    {
        {OUString("ShowPositiveError"),0,cppu::UnoType<bool>::get(), 0, 0},
        {OUString("ShowNegativeError"),1,cppu::UnoType<bool>::get(), 0, 0},
        {OUString("PositiveError"),2,cppu::UnoType<double>::get(),0,0},
        {OUString("NegativeError"),3,cppu::UnoType<double>::get(), 0, 0},
        {OUString("PercentageError"),4,cppu::UnoType<double>::get(), 0, 0},
        {OUString("ErrorBarStyle"),5,cppu::UnoType<sal_Int32>::get(),0,0},
        {OUString("ErrorBarRangePositive"),6,cppu::UnoType<OUString>::get(),0,0}, // read-only for export
        {OUString("ErrorBarRangeNegative"),7,cppu::UnoType<OUString>::get(),0,0}, // read-only for export
        {OUString("Weight"),8,cppu::UnoType<double>::get(),0,0},
        {OUString("LineStyle"),9,cppu::UnoType<css::drawing::LineStyle>::get(),0,0},
        {OUString("LineDash"),10,cppu::UnoType<drawing::LineDash>::get(),0,0},
        {OUString("LineWidth"),11,cppu::UnoType<sal_Int32>::get(),0,0},
        {OUString("LineColor"),12,cppu::UnoType<css::util::Color>::get(),0,0},
        {OUString("LineTransparence"),13,cppu::UnoType<sal_Int16>::get(),0,0},
        {OUString("LineJoint"),14,cppu::UnoType<css::drawing::LineJoint>::get(),0,0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SfxItemPropertySet aPropSet( aErrorBarPropertyMap_Impl );
    return &aPropSet;
}

} // anonymous namespace

namespace chart
{

ErrorBar::ErrorBar() :
    mnLineWidth(0),
    meLineStyle(drawing::LineStyle_SOLID),
    maLineColor(0),
    mnLineTransparence(0),
    meLineJoint(drawing::LineJoint_ROUND),
    mbShowPositiveError(true),
    mbShowNegativeError(true),
    mfPositiveError(0),
    mfNegativeError(0),
    mfWeight(1),
    meStyle(css::chart::ErrorBarStyle::NONE),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

ErrorBar::ErrorBar( const ErrorBar & rOther ) :
    MutexContainer(),
    impl::ErrorBar_Base(),
    maDashName(rOther.maDashName),
    maLineDash(rOther.maLineDash),
    mnLineWidth(rOther.mnLineWidth),
    meLineStyle(rOther.meLineStyle),
    maLineColor(rOther.maLineColor),
    mnLineTransparence(rOther.mnLineTransparence),
    meLineJoint(rOther.meLineJoint),
    mbShowPositiveError(rOther.mbShowPositiveError),
    mbShowNegativeError(rOther.mbShowNegativeError),
    mfPositiveError(rOther.mfPositiveError),
    mfNegativeError(rOther.mfNegativeError),
    mfWeight(rOther.mfWeight),
    meStyle(rOther.meStyle),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    if( ! rOther.m_aDataSequences.empty())
    {
        if( lcl_isInternalData( rOther.m_aDataSequences.front()))
            CloneHelper::CloneRefVector< css::chart2::data::XLabeledDataSequence >(
                rOther.m_aDataSequences, m_aDataSequences );
        else
            m_aDataSequences = rOther.m_aDataSequences;
        ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
    }
}

ErrorBar::~ErrorBar()
{}

uno::Reference< util::XCloneable > SAL_CALL ErrorBar::createClone()
{
    return uno::Reference< util::XCloneable >( new ErrorBar( *this ));
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ErrorBar::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > aRef (
            new SfxItemPropertySetInfo( GetErrorBarPropertySet()->getPropertyMap() ) );
    return aRef;
}

void ErrorBar::setPropertyValue( const OUString& rPropName, const uno::Any& rAny )
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
        throw beans::UnknownPropertyException("read-only property", static_cast< uno::XWeak*>(this));
    else if(rPropName == "LineDashName")
        rAny >>= maDashName;
    else if(rPropName == "LineDash")
        rAny >>= maLineDash;
    else if(rPropName == "LineWidth")
        rAny >>= mnLineWidth;
    else if(rPropName == "LineStyle")
        rAny >>= meLineStyle;
    else if(rPropName == "LineColor")
        rAny >>= maLineColor;
    else if(rPropName == "LineTransparence")
        rAny >>= mnLineTransparence;
    else if(rPropName == "LineJoint")
        rAny >>= meLineJoint;

    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

namespace {

OUString getSourceRangeStrFromLabeledSequences( const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > >& aSequences, bool bPositive )
{
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
                if( ( xSeqProp->getPropertyValue( "Role" ) >>= aRole ) &&
                        aRole.match( "error-bars" ) && aRole.indexOf(aDirection) >= 0 )
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
        if(meStyle == css::chart::ErrorBarStyle::FROM_DATA)
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
        if(meStyle == css::chart::ErrorBarStyle::FROM_DATA)
        {
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences =
                getDataSequences();

            aRange = getSourceRangeStrFromLabeledSequences( aSequences, false );
        }

        aRet <<= aRange;
    }
    else if(rPropName == "LineDashName")
        aRet <<= maDashName;
    else if(rPropName == "LineDash")
        aRet <<= maLineDash;
    else if(rPropName == "LineWidth")
        aRet <<= mnLineWidth;
    else if(rPropName == "LineStyle")
        aRet <<= meLineStyle;
    else if(rPropName == "LineColor")
        aRet <<= maLineColor;
    else if(rPropName == "LineTransparence")
        aRet <<= mnLineTransparence;
    else if(rPropName == "LineJoint")
        aRet <<= meLineJoint;

    SAL_WARN_IF(!aRet.hasValue(), "chart2", "asked for property value: " << rPropName);
    return aRet;
}

beans::PropertyState ErrorBar::getPropertyState( const OUString& rPropName )
{
    if(rPropName == "ErrorBarStyle")
    {
        if(meStyle == css::chart::ErrorBarStyle::NONE)
            return beans::PropertyState_DEFAULT_VALUE;
        return beans::PropertyState_DIRECT_VALUE;
    }
    else if(rPropName == "PositiveError")
    {
        if(mbShowPositiveError)
        {
            switch(meStyle)
            {
                case css::chart::ErrorBarStyle::ABSOLUTE:
                case css::chart::ErrorBarStyle::ERROR_MARGIN:
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
                case css::chart::ErrorBarStyle::ABSOLUTE:
                case css::chart::ErrorBarStyle::ERROR_MARGIN:
                    return beans::PropertyState_DIRECT_VALUE;
                default:
                    break;
            }
        }
        return beans::PropertyState_DEFAULT_VALUE;
    }
    else if(rPropName == "PercentageError")
    {
        if(meStyle != css::chart::ErrorBarStyle::RELATIVE)
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
        if(meStyle == css::chart::ErrorBarStyle::FROM_DATA && mbShowPositiveError)
            return beans::PropertyState_DIRECT_VALUE;
        return beans::PropertyState_DEFAULT_VALUE;
    }
    else if(rPropName == "ErrorBarRangeNegative")
    {
        if(meStyle == css::chart::ErrorBarStyle::FROM_DATA && mbShowNegativeError)
            return beans::PropertyState_DIRECT_VALUE;
        return beans::PropertyState_DEFAULT_VALUE;
    }
    else
        return beans::PropertyState_DIRECT_VALUE;
}

uno::Sequence< beans::PropertyState > ErrorBar::getPropertyStates( const uno::Sequence< OUString >& rPropNames )
{
    uno::Sequence< beans::PropertyState > aRet( rPropNames.getLength() );
    for(sal_Int32 i = 0; i < rPropNames.getLength(); ++i)
    {
        aRet[i] = getPropertyState(rPropNames[i]);
    }
    return aRet;
}

void ErrorBar::setPropertyToDefault( const OUString& )
{
    //keep them unimplemented for now
}

uno::Any ErrorBar::getPropertyDefault( const OUString& )
{
    //keep them unimplemented for now
    return uno::Any();
}

void ErrorBar::addPropertyChangeListener( const OUString&, const css::uno::Reference< css::beans::XPropertyChangeListener >& )
{
}

void ErrorBar::removePropertyChangeListener( const OUString&, const css::uno::Reference< css::beans::XPropertyChangeListener >& )
{
}

void ErrorBar::addVetoableChangeListener( const OUString&, const css::uno::Reference< css::beans::XVetoableChangeListener >& )
{
}

void ErrorBar::removeVetoableChangeListener( const OUString&, const css::uno::Reference< css::beans::XVetoableChangeListener >& )
{
}

// ____ XModifyBroadcaster ____
void SAL_CALL ErrorBar::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ErrorBar::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ XDataSink ____
void SAL_CALL ErrorBar::setData( const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > >& aData )
{
    ModifyListenerHelper::removeListenerFromAllElements( m_aDataSequences, m_xModifyEventForwarder );
    EventListenerHelper::removeListenerFromAllElements( m_aDataSequences, this );
    m_aDataSequences = ContainerHelper::SequenceToVector( aData );
    EventListenerHelper::addListenerToAllElements( m_aDataSequences, this );
    ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
}

// ____ XDataSource ____
uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > SAL_CALL ErrorBar::getDataSequences()
{
    return comphelper::containerToSequence( m_aDataSequences );
}

OUString SAL_CALL ErrorBar::getImplementationName()
{
    return OUString(lcl_aServiceName);
}

sal_Bool SAL_CALL ErrorBar::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ErrorBar::getSupportedServiceNames()
{
    return {
        lcl_aServiceName,
        "com.sun.star.chart2.ErrorBar"
    };
}

// needed by MSC compiler
using impl::ErrorBar_Base;

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_ErrorBar_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::ErrorBar);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
