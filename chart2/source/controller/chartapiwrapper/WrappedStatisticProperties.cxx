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

#include "WrappedStatisticProperties.hxx"
#include "WrappedSeriesOrDiagramProperty.hxx"
#include <FastPropertyIdRanges.hxx>
#include <RegressionCurveHelper.hxx>
#include <ErrorBar.hxx>
#include <StatisticsHelper.hxx>
#include <unonames.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/ChartErrorCategory.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#include <com/sun/star/chart/ChartRegressionCurveType.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <utility>

namespace com { namespace sun { namespace star { namespace chart2 { namespace data { class XDataProvider; } } } } }

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;

namespace chart
{
namespace wrapper
{

namespace
{

Any lcl_getRegressionDefault()
{
    Any aRet;
    aRet <<= css::chart::ChartRegressionCurveType_NONE;
    return aRet;
}

css::chart::ChartRegressionCurveType lcl_getRegressionCurveType(SvxChartRegress eRegressionType)
{
    css::chart::ChartRegressionCurveType eRet = css::chart::ChartRegressionCurveType_NONE;
    switch(eRegressionType)
    {
        case SvxChartRegress::Linear:
            eRet = css::chart::ChartRegressionCurveType_LINEAR;
            break;
        case SvxChartRegress::Log:
            eRet = css::chart::ChartRegressionCurveType_LOGARITHM;
            break;
        case SvxChartRegress::Exp:
            eRet = css::chart::ChartRegressionCurveType_EXPONENTIAL;
            break;
        case SvxChartRegress::Power:
            eRet = css::chart::ChartRegressionCurveType_POWER;
            break;
        case SvxChartRegress::Polynomial:
            eRet = css::chart::ChartRegressionCurveType_POLYNOMIAL;
            break;
        /*case SvxChartRegress::MovingAverage:
            eRet = css::chart::ChartRegressionCurveType_MOVING_AVERAGE;
            break;*/
        default:
            eRet = css::chart::ChartRegressionCurveType_NONE;
            break;
    }
    return eRet;
}

SvxChartRegress lcl_getRegressionType( css::chart::ChartRegressionCurveType eRegressionCurveType )
{
    SvxChartRegress eRet;
    switch (eRegressionCurveType)
    {
        case css::chart::ChartRegressionCurveType_LINEAR:
            eRet = SvxChartRegress::Linear;
            break;
        case css::chart::ChartRegressionCurveType_LOGARITHM:
            eRet = SvxChartRegress::Log;
            break;
        case css::chart::ChartRegressionCurveType_EXPONENTIAL:
            eRet = SvxChartRegress::Exp;
            break;
        case css::chart::ChartRegressionCurveType_POLYNOMIAL:
        //case css::chart::ChartRegressionCurveType_MOVING_AVERAGE:
        case css::chart::ChartRegressionCurveType_POWER:
            eRet = SvxChartRegress::Power;
            break;
        default:
            eRet = SvxChartRegress::NONE;
            break;
    }
    return eRet;
}

sal_Int32 lcl_getErrorBarStyle( const uno::Reference< beans::XPropertySet >& xErrorBarProperties )
{
    sal_Int32 nStyle = css::chart::ErrorBarStyle::NONE;
    if(xErrorBarProperties.is())
        xErrorBarProperties->getPropertyValue( "ErrorBarStyle" ) >>= nStyle;
    return nStyle;
}

uno::Reference< chart2::data::XDataProvider > lcl_getDataProviderFromContact(
    const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    uno::Reference< chart2::data::XDataProvider > xResult;
    if( spChart2ModelContact.get())
    {
        uno::Reference< chart2::XChartDocument > xChartDoc(
            spChart2ModelContact->getChart2Document());
        if( xChartDoc.is())
            xResult.set( xChartDoc->getDataProvider());
    }
    return xResult;
}

void lcl_ConvertRangeFromXML(
    OUString & rInOutRange,
    const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    if( !rInOutRange.isEmpty())
    {
        uno::Reference< chart2::data::XRangeXMLConversion > xConverter(
            lcl_getDataProviderFromContact( spChart2ModelContact ), uno::UNO_QUERY );
        if( xConverter.is())
        {
            OUString aResult = xConverter->convertRangeFromXML( rInOutRange );
            rInOutRange = aResult;
        }
    }
}

void lcl_ConvertRangeToXML(
    OUString & rInOutRange,
    const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    if( !rInOutRange.isEmpty())
    {
        uno::Reference< chart2::data::XRangeXMLConversion > xConverter(
            lcl_getDataProviderFromContact( spChart2ModelContact ), uno::UNO_QUERY );
        if( xConverter.is())
        {
            OUString aResult = xConverter->convertRangeToXML( rInOutRange );
            rInOutRange = aResult;
        }
    }
}

}//anonymous namespace

template< typename PROPERTYTYPE >
class WrappedStatisticProperty : public WrappedSeriesOrDiagramProperty< PROPERTYTYPE >
{
public:
    explicit WrappedStatisticProperty(
        const OUString& rName, const Any& rDefaulValue,
        const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
        tSeriesOrDiagramPropertyType ePropertyType)
        : WrappedSeriesOrDiagramProperty<PROPERTYTYPE>(rName, rDefaulValue, spChart2ModelContact,
                                                       ePropertyType)
    {}

protected:
    static uno::Reference< beans::XPropertySet > getOrCreateErrorBarProperties( const Reference< beans::XPropertySet >& xSeriesPropertySet )
    {
        if(!xSeriesPropertySet.is())
            return nullptr;
        uno::Reference< beans::XPropertySet > xErrorBarProperties;
        xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xErrorBarProperties;
        if( !xErrorBarProperties.is() )
        {
            xErrorBarProperties = new ::chart::ErrorBar;
            //default in new and old api are different
            xErrorBarProperties->setPropertyValue( "ShowPositiveError" , uno::Any(false) );
            xErrorBarProperties->setPropertyValue( "ShowNegativeError" , uno::Any(false) );
            xErrorBarProperties->setPropertyValue( "ErrorBarStyle" , uno::Any(css::chart::ErrorBarStyle::NONE) );
            xSeriesPropertySet->setPropertyValue( CHART_UNONAME_ERRORBAR_Y , uno::Any( xErrorBarProperties ) );
        }
        return xErrorBarProperties;
    }

};

//PROP_CHART_STATISTIC_CONST_ERROR_LOW
class WrappedConstantErrorLowProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const double& aNewValue ) const override;

    explicit WrappedConstantErrorLowProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                              tSeriesOrDiagramPropertyType ePropertyType );

private:
    mutable Any m_aOuterValue;
};

WrappedConstantErrorLowProperty::WrappedConstantErrorLowProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< double >( "ConstantErrorLow"
            , uno::Any( 0.0 ), std::move(spChart2ModelContact), ePropertyType  )
{
}

double WrappedConstantErrorLowProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    double aRet = 0.0;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( lcl_getErrorBarStyle( xErrorBarProperties ) == css::chart::ErrorBarStyle::ABSOLUTE )
            xErrorBarProperties->getPropertyValue( "NegativeError" ) >>= aRet;
        else
            m_aOuterValue >>= aRet;
    }
    return aRet;
}

void WrappedConstantErrorLowProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const double& aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        m_aOuterValue <<= aNewValue;
        if( lcl_getErrorBarStyle( xErrorBarProperties ) == css::chart::ErrorBarStyle::ABSOLUTE )
        {
            xErrorBarProperties->setPropertyValue( "NegativeError", m_aOuterValue );
        }
    }
}

//PROP_CHART_STATISTIC_CONST_ERROR_HIGH
class WrappedConstantErrorHighProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const double& aNewValue ) const override;

    explicit WrappedConstantErrorHighProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                               tSeriesOrDiagramPropertyType ePropertyType );

private:
    mutable Any m_aOuterValue;
};

WrappedConstantErrorHighProperty::WrappedConstantErrorHighProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< double >( "ConstantErrorHigh"
            , uno::Any( 0.0 ), std::move(spChart2ModelContact), ePropertyType  )
{
}

double WrappedConstantErrorHighProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    double aRet = 0.0;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( lcl_getErrorBarStyle( xErrorBarProperties ) == css::chart::ErrorBarStyle::ABSOLUTE )
            xErrorBarProperties->getPropertyValue( "PositiveError" ) >>= aRet;
        else
            m_aOuterValue >>= aRet;
    }
    return aRet;
}

void WrappedConstantErrorHighProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const double& aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        m_aOuterValue <<= aNewValue;
        if( lcl_getErrorBarStyle( xErrorBarProperties ) == css::chart::ErrorBarStyle::ABSOLUTE )
        {
            xErrorBarProperties->setPropertyValue( "PositiveError" , m_aOuterValue );
        }
    }
}

//PROP_CHART_STATISTIC_MEAN_VALUE
class WrappedMeanValueProperty : public WrappedStatisticProperty< bool >
{
public:
    virtual bool getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const bool& aNewValue ) const override;

    explicit WrappedMeanValueProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                       tSeriesOrDiagramPropertyType ePropertyType );
};

WrappedMeanValueProperty::WrappedMeanValueProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< bool >( "MeanValue", uno::Any( false ), std::move(spChart2ModelContact), ePropertyType  )
{
}

bool WrappedMeanValueProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    bool bRet = false;
    uno::Reference< chart2::XRegressionCurveContainer > xRegCnt( xSeriesPropertySet, uno::UNO_QUERY );
    if( xRegCnt.is() )
        bRet = RegressionCurveHelper::hasMeanValueLine( xRegCnt );
    return bRet;
}

void WrappedMeanValueProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const bool& aNewValue ) const
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegCnt( xSeriesPropertySet, uno::UNO_QUERY );
    if( xRegCnt.is() )
    {
        if(aNewValue)
            RegressionCurveHelper::addMeanValueLine( xRegCnt, nullptr );
        else
            RegressionCurveHelper::removeMeanValueLine( xRegCnt );
    }
}

//PROP_CHART_STATISTIC_ERROR_CATEGORY
// deprecated, replaced by ErrorBarStyle
class WrappedErrorCategoryProperty : public WrappedStatisticProperty< css::chart::ChartErrorCategory >
{
public:
    virtual css::chart::ChartErrorCategory getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const css::chart::ChartErrorCategory& aNewValue ) const override;

    explicit WrappedErrorCategoryProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                           tSeriesOrDiagramPropertyType ePropertyType );
};

WrappedErrorCategoryProperty::WrappedErrorCategoryProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< css::chart::ChartErrorCategory >( "ErrorCategory"
            , uno::Any( css::chart::ChartErrorCategory_NONE ), std::move(spChart2ModelContact), ePropertyType  )
{
}

css::chart::ChartErrorCategory WrappedErrorCategoryProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    css::chart::ChartErrorCategory aRet = css::chart::ChartErrorCategory_NONE;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        sal_Int32 nStyle = css::chart::ErrorBarStyle::NONE;
        xErrorBarProperties->getPropertyValue( "ErrorBarStyle" ) >>= nStyle;
        switch(nStyle)
        {
            case css::chart::ErrorBarStyle::NONE:
                aRet = css::chart::ChartErrorCategory_NONE;
                break;
            case css::chart::ErrorBarStyle::VARIANCE:
                aRet = css::chart::ChartErrorCategory_VARIANCE;
                break;
            case css::chart::ErrorBarStyle::STANDARD_DEVIATION:
                aRet = css::chart::ChartErrorCategory_STANDARD_DEVIATION;
                break;
            case css::chart::ErrorBarStyle::ABSOLUTE:
                aRet = css::chart::ChartErrorCategory_CONSTANT_VALUE;
                break;
            case css::chart::ErrorBarStyle::RELATIVE:
                aRet = css::chart::ChartErrorCategory_PERCENT;
                break;
            case css::chart::ErrorBarStyle::ERROR_MARGIN:
                aRet = css::chart::ChartErrorCategory_ERROR_MARGIN;
                break;
            case css::chart::ErrorBarStyle::STANDARD_ERROR:
                break;
            case css::chart::ErrorBarStyle::FROM_DATA:
                break;
            default:
                break;
        }
    }
    return aRet;
}
void WrappedErrorCategoryProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const css::chart::ChartErrorCategory& aNewValue ) const
{
    if( !xSeriesPropertySet.is() )
        return;

    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        sal_Int32 nNewStyle = css::chart::ErrorBarStyle::NONE;
        switch(aNewValue)
        {
            case css::chart::ChartErrorCategory_NONE:
                nNewStyle = css::chart::ErrorBarStyle::NONE;
                break;
            case css::chart::ChartErrorCategory_VARIANCE:
                nNewStyle = css::chart::ErrorBarStyle::VARIANCE;
                break;
            case css::chart::ChartErrorCategory_STANDARD_DEVIATION:
                nNewStyle = css::chart::ErrorBarStyle::STANDARD_DEVIATION;
                break;
            case css::chart::ChartErrorCategory_CONSTANT_VALUE:
                nNewStyle = css::chart::ErrorBarStyle::ABSOLUTE;
                break;
            case css::chart::ChartErrorCategory_PERCENT:
                nNewStyle = css::chart::ErrorBarStyle::RELATIVE;
                break;
            case css::chart::ChartErrorCategory_ERROR_MARGIN:
                nNewStyle = css::chart::ErrorBarStyle::ERROR_MARGIN;
                break;
            default:
                break;
        }
        xErrorBarProperties->setPropertyValue( "ErrorBarStyle" , uno::Any(nNewStyle) );
    }
}

//PROP_CHART_STATISTIC_PERCENT_ERROR
class WrappedPercentageErrorProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const double& aNewValue ) const override;

    explicit WrappedPercentageErrorProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                             tSeriesOrDiagramPropertyType ePropertyType );

private:
    mutable Any m_aOuterValue;
};

WrappedPercentageErrorProperty::WrappedPercentageErrorProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< double >( "PercentageError"
            , uno::Any( 0.0 ), std::move(spChart2ModelContact), ePropertyType  )
{
}

double WrappedPercentageErrorProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    double aRet = 0.0;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( lcl_getErrorBarStyle( xErrorBarProperties ) == css::chart::ErrorBarStyle::RELATIVE )
            xErrorBarProperties->getPropertyValue( "PositiveError" ) >>= aRet;
        else
            m_aOuterValue >>= aRet;
    }
    return aRet;
}
void WrappedPercentageErrorProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const double& aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        m_aOuterValue <<= aNewValue;
        if( lcl_getErrorBarStyle( xErrorBarProperties ) == css::chart::ErrorBarStyle::RELATIVE )
        {
            xErrorBarProperties->setPropertyValue( "PositiveError" , m_aOuterValue );
            xErrorBarProperties->setPropertyValue( "NegativeError" , m_aOuterValue );
        }
    }
}

//PROP_CHART_STATISTIC_ERROR_MARGIN
class WrappedErrorMarginProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const double& aNewValue ) const override;

    explicit WrappedErrorMarginProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                         tSeriesOrDiagramPropertyType ePropertyType );

private:
    mutable Any m_aOuterValue;
};

WrappedErrorMarginProperty::WrappedErrorMarginProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< double >( "ErrorMargin"
            , uno::Any( 0.0 ), std::move(spChart2ModelContact), ePropertyType  )
{
}

double WrappedErrorMarginProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    double aRet = 0.0;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( lcl_getErrorBarStyle( xErrorBarProperties ) == css::chart::ErrorBarStyle::ERROR_MARGIN )
            xErrorBarProperties->getPropertyValue( "PositiveError" ) >>= aRet;
        else
            m_aOuterValue >>= aRet;
    }
    return aRet;
}
void WrappedErrorMarginProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const double& aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        m_aOuterValue <<= aNewValue;
        if( lcl_getErrorBarStyle( xErrorBarProperties ) == css::chart::ErrorBarStyle::ERROR_MARGIN )
        {
            xErrorBarProperties->setPropertyValue( "PositiveError" , m_aOuterValue );
            xErrorBarProperties->setPropertyValue( "NegativeError" , m_aOuterValue );
        }
    }
}

//PROP_CHART_STATISTIC_ERROR_INDICATOR
class WrappedErrorIndicatorProperty : public WrappedStatisticProperty< css::chart::ChartErrorIndicatorType >
{
public:
    virtual css::chart::ChartErrorIndicatorType getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const css::chart::ChartErrorIndicatorType& aNewValue ) const override;

    explicit WrappedErrorIndicatorProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                            tSeriesOrDiagramPropertyType ePropertyType );
};

WrappedErrorIndicatorProperty::WrappedErrorIndicatorProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< css::chart::ChartErrorIndicatorType >( "ErrorIndicator"
            , uno::Any( css::chart::ChartErrorIndicatorType_NONE ), std::move(spChart2ModelContact), ePropertyType  )
{
}

css::chart::ChartErrorIndicatorType WrappedErrorIndicatorProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    css::chart::ChartErrorIndicatorType aRet = css::chart::ChartErrorIndicatorType_NONE;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        bool bPositive = false;
        bool bNegative = false;
        xErrorBarProperties->getPropertyValue( "ShowPositiveError" ) >>= bPositive;
        xErrorBarProperties->getPropertyValue( "ShowNegativeError" ) >>= bNegative;

        if( bPositive && bNegative )
            aRet = css::chart::ChartErrorIndicatorType_TOP_AND_BOTTOM;
        else if( bPositive && !bNegative )
            aRet = css::chart::ChartErrorIndicatorType_UPPER;
        else if( !bPositive && bNegative )
            aRet = css::chart::ChartErrorIndicatorType_LOWER;
    }
    return aRet;
}
void WrappedErrorIndicatorProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const css::chart::ChartErrorIndicatorType& aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        bool bPositive = false;
        bool bNegative = false;
        switch( aNewValue )
        {
            case css::chart::ChartErrorIndicatorType_TOP_AND_BOTTOM:
                bPositive = true;
                bNegative = true;
                break;
            case css::chart::ChartErrorIndicatorType_UPPER:
                bPositive = true;
                break;
            case css::chart::ChartErrorIndicatorType_LOWER:
                bNegative = true;
                break;
            default:
                break;
        }

        xErrorBarProperties->setPropertyValue( "ShowPositiveError" , uno::Any(bPositive) );
        xErrorBarProperties->setPropertyValue( "ShowNegativeError" , uno::Any(bNegative) );
    }
}

//PROP_CHART_STATISTIC_ERROR_BAR_STYLE
// this is the new constant group that replaces the deprecated enum ChartErrorCategory
class WrappedErrorBarStyleProperty : public WrappedStatisticProperty< sal_Int32 >
{
public:
    virtual sal_Int32 getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const sal_Int32& nNewValue ) const override;

    explicit WrappedErrorBarStyleProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact1,
                                           tSeriesOrDiagramPropertyType ePropertyType );
};

WrappedErrorBarStyleProperty::WrappedErrorBarStyleProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< sal_Int32 >( "ErrorBarStyle"
            , uno::Any( css::chart::ErrorBarStyle::NONE ), std::move(spChart2ModelContact), ePropertyType  )
{
}

sal_Int32 WrappedErrorBarStyleProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    sal_Int32 nRet = css::chart::ErrorBarStyle::NONE;
    m_aDefaultValue >>= nRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        xErrorBarProperties->getPropertyValue( "ErrorBarStyle" ) >>= nRet;
    }
    return nRet;
}
void WrappedErrorBarStyleProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const sal_Int32& nNewValue ) const
{
    if( !xSeriesPropertySet.is() )
        return;

    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        xErrorBarProperties->setPropertyValue( "ErrorBarStyle" , uno::Any( nNewValue ));
    }
}

//PROP_CHART_STATISTIC_ERROR_RANGE_POSITIVE
class WrappedErrorBarRangePositiveProperty : public WrappedStatisticProperty< OUString >
{
public:
    virtual OUString getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const OUString& aNewValue ) const override;

    explicit WrappedErrorBarRangePositiveProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                                   tSeriesOrDiagramPropertyType ePropertyType );
};

WrappedErrorBarRangePositiveProperty::WrappedErrorBarRangePositiveProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< OUString >( "ErrorBarRangePositive"
            , uno::Any( OUString() ), std::move(spChart2ModelContact), ePropertyType  )
{
}

OUString WrappedErrorBarRangePositiveProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    OUString aRet;
    m_aDefaultValue >>= aRet;
    uno::Reference< chart2::data::XDataSource > xErrorBarDataSource;
    if( xSeriesPropertySet.is() &&
        ( xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xErrorBarDataSource ) &&
        xErrorBarDataSource.is())
    {
        uno::Reference< chart2::data::XDataSequence > xSeq(
            StatisticsHelper::getErrorDataSequenceFromDataSource(
                xErrorBarDataSource, true /* positive */ ));
        if( xSeq.is())
            aRet = xSeq->getSourceRangeRepresentation();
        else
            m_aOuterValue >>= aRet;
    }
    lcl_ConvertRangeToXML( aRet, m_spChart2ModelContact );
    return aRet;
}

void WrappedErrorBarRangePositiveProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const OUString& aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        uno::Reference< chart2::data::XDataProvider > xDataProvider(
            lcl_getDataProviderFromContact( m_spChart2ModelContact ));
        uno::Reference< chart2::data::XDataSource > xDataSource( xErrorBarProperties, uno::UNO_QUERY );
        if( xDataSource.is() && xDataProvider.is())
        {
            OUString aTmp( aNewValue );
            OUString aXMLRange( aNewValue );
            lcl_ConvertRangeFromXML( aTmp, m_spChart2ModelContact );
            StatisticsHelper::setErrorDataSequence(
                xDataSource, xDataProvider, aTmp, true /* positive */, true /* y-error */, &aXMLRange );
            m_aOuterValue <<= aTmp;
        }
    }
}

//PROP_CHART_STATISTIC_ERROR_RANGE_NEGATIVE
class WrappedErrorBarRangeNegativeProperty : public WrappedStatisticProperty< OUString >
{
public:
    virtual OUString getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const OUString& aNewValue ) const override;

    explicit WrappedErrorBarRangeNegativeProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                                   tSeriesOrDiagramPropertyType ePropertyType );
};

WrappedErrorBarRangeNegativeProperty::WrappedErrorBarRangeNegativeProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< OUString >( "ErrorBarRangeNegative"
            , uno::Any( OUString() ), std::move(spChart2ModelContact), ePropertyType  )
{
}

OUString WrappedErrorBarRangeNegativeProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    OUString aRet;
    m_aDefaultValue >>= aRet;
    uno::Reference< chart2::data::XDataSource > xErrorBarDataSource;
    if( xSeriesPropertySet.is() &&
        ( xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xErrorBarDataSource ) &&
        xErrorBarDataSource.is())
    {
        uno::Reference< chart2::data::XDataSequence > xSeq(
            StatisticsHelper::getErrorDataSequenceFromDataSource(
                xErrorBarDataSource, false /* positive */ ));
        if( xSeq.is())
            aRet = xSeq->getSourceRangeRepresentation();
        else
            m_aOuterValue >>= aRet;
    }
    lcl_ConvertRangeToXML( aRet, m_spChart2ModelContact );
    return aRet;
}

void WrappedErrorBarRangeNegativeProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const OUString& aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        uno::Reference< chart2::data::XDataProvider > xDataProvider(
            lcl_getDataProviderFromContact( m_spChart2ModelContact ));
        uno::Reference< chart2::data::XDataSource > xDataSource( xErrorBarProperties, uno::UNO_QUERY );
        if( xDataSource.is() && xDataProvider.is())
        {
            OUString aTmp( aNewValue );
            OUString aXMLRange( aNewValue );
            lcl_ConvertRangeFromXML( aTmp, m_spChart2ModelContact );
            StatisticsHelper::setErrorDataSequence(
                xDataSource, xDataProvider, aTmp, false /* positive */, true /* y-error */, &aXMLRange );
            m_aOuterValue <<= aTmp;
        }
    }
}

//PROP_CHART_STATISTIC_REGRESSION_CURVES
class WrappedRegressionCurvesProperty : public WrappedStatisticProperty< css::chart::ChartRegressionCurveType >
{
public:
    virtual css::chart::ChartRegressionCurveType getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const css::chart::ChartRegressionCurveType & aNewValue ) const override;

    explicit WrappedRegressionCurvesProperty( std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                              tSeriesOrDiagramPropertyType ePropertyType );
};

WrappedRegressionCurvesProperty::WrappedRegressionCurvesProperty(
    std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< css::chart::ChartRegressionCurveType >( "RegressionCurves"
        , lcl_getRegressionDefault(), std::move(spChart2ModelContact), ePropertyType  )
{
}

css::chart::ChartRegressionCurveType WrappedRegressionCurvesProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    css::chart::ChartRegressionCurveType aRet;
    m_aDefaultValue >>= aRet;
    uno::Reference< chart2::XRegressionCurveContainer > xRegCnt( xSeriesPropertySet, uno::UNO_QUERY );
    if( xRegCnt.is() )
    {
        aRet = lcl_getRegressionCurveType(
            RegressionCurveHelper::getFirstRegressTypeNotMeanValueLine( xRegCnt ) );
    }
    return aRet;
}
void WrappedRegressionCurvesProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const css::chart::ChartRegressionCurveType& aNewValue ) const
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer( xSeriesPropertySet, uno::UNO_QUERY );
    uno::Reference< chart2::XRegressionCurve > xRegressionCurve( xSeriesPropertySet, uno::UNO_QUERY );

    if( xRegressionCurveContainer.is() && xRegressionCurve.is() )
    {
        SvxChartRegress eNewRegressionType = lcl_getRegressionType( aNewValue );

        RegressionCurveHelper::changeRegressionCurveType(
                        eNewRegressionType,
                        xRegressionCurveContainer,
                        xRegressionCurve);
    }
}

//PROP_CHART_STATISTIC_REGRESSION_PROPERTIES
//PROP_CHART_STATISTIC_ERROR_PROPERTIES
//PROP_CHART_STATISTIC_MEAN_VALUE_PROPERTIES
class WrappedStatisticPropertySetProperty : public WrappedStatisticProperty< Reference< beans::XPropertySet > >
{
public:
    virtual Reference< beans::XPropertySet > getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    // properties are read-only, so this method should never be called
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const Reference< beans::XPropertySet > & xNewValue ) const override;

    enum PropertySetType
    {
        PROPERTY_SET_TYPE_REGRESSION,
        PROPERTY_SET_TYPE_ERROR_BAR,
        PROPERTY_SET_TYPE_MEAN_VALUE
    };

    explicit WrappedStatisticPropertySetProperty(
        PropertySetType ePropertySetType, std::shared_ptr< Chart2ModelContact > spChart2ModelContact,
        tSeriesOrDiagramPropertyType ePropertyType );

private:
    PropertySetType m_eType;
};

WrappedStatisticPropertySetProperty::WrappedStatisticPropertySetProperty(
    PropertySetType ePropertySetType
    , std::shared_ptr< Chart2ModelContact > spChart2ModelContact
    , tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< Reference< beans::XPropertySet > >(
            (ePropertySetType == PROPERTY_SET_TYPE_REGRESSION)
            ? OUString("DataRegressionProperties")
            : (ePropertySetType == PROPERTY_SET_TYPE_ERROR_BAR)
              ? OUString("DataErrorProperties")
              : OUString("DataMeanValueProperties")
            , uno::Any(), std::move(spChart2ModelContact), ePropertyType  )
        , m_eType( ePropertySetType )
{
}

Reference< beans::XPropertySet > WrappedStatisticPropertySetProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    Reference< beans::XPropertySet > xResult;
    uno::Reference< chart2::XRegressionCurveContainer > xRegCnt( xSeriesPropertySet, uno::UNO_QUERY );

    switch( m_eType )
    {
        case PROPERTY_SET_TYPE_REGRESSION:
            if( xRegCnt.is() )
                xResult.set( RegressionCurveHelper::getFirstCurveNotMeanValueLine( xRegCnt ), uno::UNO_QUERY );
            break;
        case PROPERTY_SET_TYPE_ERROR_BAR:
            if( xSeriesPropertySet.is())
                xSeriesPropertySet->getPropertyValue( CHART_UNONAME_ERRORBAR_Y ) >>= xResult;
            break;
        case PROPERTY_SET_TYPE_MEAN_VALUE:
            if( xRegCnt.is() )
                xResult.set( RegressionCurveHelper::getMeanValueLine( xRegCnt ), uno::UNO_QUERY );
            break;
    }

    return xResult;
}

void WrappedStatisticPropertySetProperty::setValueToSeries(
    const Reference< beans::XPropertySet >& /* xSeriesPropertySet */
    , const Reference< beans::XPropertySet >& /* xNewValue */ ) const
{
}

namespace
{
enum
{
    //statistic properties
    PROP_CHART_STATISTIC_CONST_ERROR_LOW = FAST_PROPERTY_ID_START_CHART_STATISTIC_PROP,
    PROP_CHART_STATISTIC_CONST_ERROR_HIGH,
    PROP_CHART_STATISTIC_MEAN_VALUE,
    PROP_CHART_STATISTIC_ERROR_CATEGORY,
    PROP_CHART_STATISTIC_ERROR_BAR_STYLE,
    PROP_CHART_STATISTIC_PERCENT_ERROR,
    PROP_CHART_STATISTIC_ERROR_MARGIN,
    PROP_CHART_STATISTIC_ERROR_INDICATOR,
    PROP_CHART_STATISTIC_ERROR_RANGE_POSITIVE,
    PROP_CHART_STATISTIC_ERROR_RANGE_NEGATIVE,
    PROP_CHART_STATISTIC_REGRESSION_CURVES,
    PROP_CHART_STATISTIC_REGRESSION_PROPERTIES,
    PROP_CHART_STATISTIC_ERROR_PROPERTIES,
    PROP_CHART_STATISTIC_MEAN_VALUE_PROPERTIES
};

/** @parameter bDataSeriesProperty if true, this property is for a single data
               series, if false, it is for the whole diagram, i.e. for all
               series
 */
void lcl_addWrappedProperties( std::vector< std::unique_ptr<WrappedProperty> >& rList
            , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact
            , tSeriesOrDiagramPropertyType ePropertyType )
{
    rList.emplace_back( new WrappedConstantErrorLowProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedConstantErrorHighProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedMeanValueProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedErrorCategoryProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedErrorBarStyleProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedPercentageErrorProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedErrorMarginProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedErrorIndicatorProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedErrorBarRangePositiveProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedErrorBarRangeNegativeProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedRegressionCurvesProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedStatisticPropertySetProperty(
                         WrappedStatisticPropertySetProperty::PROPERTY_SET_TYPE_REGRESSION, spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedStatisticPropertySetProperty(
                         WrappedStatisticPropertySetProperty::PROPERTY_SET_TYPE_ERROR_BAR,  spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedStatisticPropertySetProperty(
                         WrappedStatisticPropertySetProperty::PROPERTY_SET_TYPE_MEAN_VALUE, spChart2ModelContact, ePropertyType ) );
}

}//anonymous namespace

void WrappedStatisticProperties::addProperties( std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "ConstantErrorLow",
                  PROP_CHART_STATISTIC_CONST_ERROR_LOW,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "ConstantErrorHigh",
                  PROP_CHART_STATISTIC_CONST_ERROR_HIGH,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "MeanValue",
                  PROP_CHART_STATISTIC_MEAN_VALUE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "ErrorCategory",
                  PROP_CHART_STATISTIC_ERROR_CATEGORY,
                  cppu::UnoType<css::chart::ChartErrorCategory>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "ErrorBarStyle",
                  PROP_CHART_STATISTIC_ERROR_BAR_STYLE,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "PercentageError",
                  PROP_CHART_STATISTIC_PERCENT_ERROR,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "ErrorMargin",
                  PROP_CHART_STATISTIC_ERROR_MARGIN,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "ErrorIndicator",
                  PROP_CHART_STATISTIC_ERROR_INDICATOR,
                  cppu::UnoType<css::chart::ChartErrorIndicatorType>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "ErrorBarRangePositive",
                  PROP_CHART_STATISTIC_ERROR_RANGE_POSITIVE,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "ErrorBarRangeNegative",
                  PROP_CHART_STATISTIC_ERROR_RANGE_NEGATIVE,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "RegressionCurves",
                  PROP_CHART_STATISTIC_REGRESSION_CURVES,
                  cppu::UnoType<css::chart::ChartRegressionCurveType>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "DataRegressionProperties",
                  PROP_CHART_STATISTIC_REGRESSION_PROPERTIES,
                  cppu::UnoType<beans::XPropertySet>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::READONLY
                  | beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( "DataErrorProperties",
                  PROP_CHART_STATISTIC_ERROR_PROPERTIES,
                  cppu::UnoType<beans::XPropertySet>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::READONLY
                  | beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( "DataMeanValueProperties",
                  PROP_CHART_STATISTIC_MEAN_VALUE_PROPERTIES,
                  cppu::UnoType<beans::XPropertySet>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::READONLY
                  | beans::PropertyAttribute::MAYBEVOID );
}

void WrappedStatisticProperties::addWrappedPropertiesForSeries( std::vector< std::unique_ptr<WrappedProperty> >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact, DATA_SERIES );
}

void WrappedStatisticProperties::addWrappedPropertiesForDiagram( std::vector< std::unique_ptr<WrappedProperty> >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact, DIAGRAM  );
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
