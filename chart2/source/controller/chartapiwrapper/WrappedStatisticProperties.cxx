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
#include "macros.hxx"
#include "FastPropertyIdRanges.hxx"
#include "RegressionCurveHelper.hxx"
#include "DiagramHelper.hxx"
#include "ErrorBar.hxx"
#include "StatisticsHelper.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ChartErrorCategory.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#include <com/sun/star/chart/ChartRegressionCurveType.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
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
    aRet <<= ::com::sun::star::chart::ChartRegressionCurveType_NONE;
    return aRet;
}

::com::sun::star::chart::ChartRegressionCurveType lcl_getRegressionCurveType( RegressionCurveHelper::tRegressionType eRegressionType )
{
    ::com::sun::star::chart::ChartRegressionCurveType eRet = ::com::sun::star::chart::ChartRegressionCurveType_NONE;
    switch(eRegressionType)
    {
        case RegressionCurveHelper::REGRESSION_TYPE_LINEAR:
            eRet = ::com::sun::star::chart::ChartRegressionCurveType_LINEAR;
            break;
        case RegressionCurveHelper::REGRESSION_TYPE_LOG:
            eRet = ::com::sun::star::chart::ChartRegressionCurveType_LOGARITHM;
            break;
        case RegressionCurveHelper::REGRESSION_TYPE_EXP:
            eRet = ::com::sun::star::chart::ChartRegressionCurveType_EXPONENTIAL;
            break;
        case RegressionCurveHelper::REGRESSION_TYPE_POWER:
            eRet = ::com::sun::star::chart::ChartRegressionCurveType_POWER;
            break;
        case RegressionCurveHelper::REGRESSION_TYPE_POLYNOMIAL:
            eRet = ::com::sun::star::chart::ChartRegressionCurveType_POLYNOMIAL;
            break;
        /*case RegressionCurveHelper::REGRESSION_TYPE_MOVING_AVERAGE:
            eRet = ::com::sun::star::chart::ChartRegressionCurveType_MOVING_AVERAGE;
            break;*/
        default:
            eRet = ::com::sun::star::chart::ChartRegressionCurveType_NONE;
            break;
    }
    return eRet;
}

RegressionCurveHelper::tRegressionType lcl_getRegressionType( ::com::sun::star::chart::ChartRegressionCurveType eRegressionCurveType )
{
    RegressionCurveHelper::tRegressionType eRet;
    switch(eRegressionCurveType)
    {
        case ::com::sun::star::chart::ChartRegressionCurveType_LINEAR:
            eRet = RegressionCurveHelper::REGRESSION_TYPE_LINEAR;
            break;
        case ::com::sun::star::chart::ChartRegressionCurveType_LOGARITHM:
            eRet = RegressionCurveHelper::REGRESSION_TYPE_LOG;
            break;
        case ::com::sun::star::chart::ChartRegressionCurveType_EXPONENTIAL:
            eRet = RegressionCurveHelper::REGRESSION_TYPE_EXP;
            break;
        case ::com::sun::star::chart::ChartRegressionCurveType_POLYNOMIAL:
        //case ::com::sun::star::chart::ChartRegressionCurveType_MOVING_AVERAGE:
        case ::com::sun::star::chart::ChartRegressionCurveType_POWER:
            eRet = RegressionCurveHelper::REGRESSION_TYPE_POWER;
            break;
        default:
            eRet = RegressionCurveHelper::REGRESSION_TYPE_NONE;
            break;
    }
    return eRet;
}

sal_Int32 lcl_getErrorBarStyle( const uno::Reference< beans::XPropertySet >& xErrorBarProperties )
{
    sal_Int32 nStyle = ::com::sun::star::chart::ErrorBarStyle::NONE;
    if(xErrorBarProperties.is())
        xErrorBarProperties->getPropertyValue( "ErrorBarStyle" ) >>= nStyle;
    return nStyle;
}

uno::Reference< chart2::data::XDataProvider > lcl_getDataProviderFromContact(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
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
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
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
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
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
    explicit WrappedStatisticProperty( const OUString& rName, const Any& rDefaulValue
                              , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact
                              , tSeriesOrDiagramPropertyType ePropertyType )
            : WrappedSeriesOrDiagramProperty< PROPERTYTYPE >(rName,rDefaulValue,spChart2ModelContact,ePropertyType)
    {}
    virtual ~WrappedStatisticProperty() {};

protected:
    uno::Reference< beans::XPropertySet > getOrCreateErrorBarProperties( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
    {
        if(!xSeriesPropertySet.is())
            return 0;
        uno::Reference< beans::XPropertySet > xErrorBarProperties;
        xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xErrorBarProperties;
        if( !xErrorBarProperties.is() )
        {
            // todo: use a valid context
            xErrorBarProperties = ::chart::createErrorBar( uno::Reference< uno::XComponentContext >() );
            //default in new and old api are different
            xErrorBarProperties->setPropertyValue( "ShowPositiveError" , uno::makeAny(sal_Bool(sal_False)) );
            xErrorBarProperties->setPropertyValue( "ShowNegativeError" , uno::makeAny(sal_Bool(sal_False)) );
            xErrorBarProperties->setPropertyValue( "ErrorBarStyle" , uno::makeAny(::com::sun::star::chart::ErrorBarStyle::NONE) );
            xSeriesPropertySet->setPropertyValue( "ErrorBarY" , uno::makeAny( xErrorBarProperties ) );
        }
        return xErrorBarProperties;
    }

};

//PROP_CHART_STATISTIC_CONST_ERROR_LOW
class WrappedConstantErrorLowProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const;

    explicit WrappedConstantErrorLowProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                              tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedConstantErrorLowProperty();

private:
    mutable Any m_aOuterValue;
};

WrappedConstantErrorLowProperty::WrappedConstantErrorLowProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< double >( "ConstantErrorLow"
            , uno::makeAny( double(0.0) ), spChart2ModelContact, ePropertyType  )
{
}
WrappedConstantErrorLowProperty::~WrappedConstantErrorLowProperty()
{
}

double WrappedConstantErrorLowProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    double aRet = 0.0;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( ::com::sun::star::chart::ErrorBarStyle::ABSOLUTE == lcl_getErrorBarStyle( xErrorBarProperties ) )
            xErrorBarProperties->getPropertyValue( "NegativeError" ) >>= aRet;
        else
            m_aOuterValue >>= aRet;
    }
    return aRet;
}

void WrappedConstantErrorLowProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        m_aOuterValue = uno::makeAny( aNewValue );
        if( ::com::sun::star::chart::ErrorBarStyle::ABSOLUTE == lcl_getErrorBarStyle( xErrorBarProperties ) )
        {
            xErrorBarProperties->setPropertyValue( "NegativeError", m_aOuterValue );
        }
    }
}

//PROP_CHART_STATISTIC_CONST_ERROR_HIGH
class WrappedConstantErrorHighProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const;

    explicit WrappedConstantErrorHighProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                               tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedConstantErrorHighProperty();

private:
    mutable Any m_aOuterValue;
};

WrappedConstantErrorHighProperty::WrappedConstantErrorHighProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< double >( "ConstantErrorHigh"
            , uno::makeAny( double(0.0) ), spChart2ModelContact, ePropertyType  )
{
}
WrappedConstantErrorHighProperty::~WrappedConstantErrorHighProperty()
{
}

double WrappedConstantErrorHighProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    double aRet = 0.0;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( ::com::sun::star::chart::ErrorBarStyle::ABSOLUTE == lcl_getErrorBarStyle( xErrorBarProperties ) )
            xErrorBarProperties->getPropertyValue( "PositiveError" ) >>= aRet;
        else
            m_aOuterValue >>= aRet;
    }
    return aRet;
}

void WrappedConstantErrorHighProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        m_aOuterValue = uno::makeAny( aNewValue );
        if( ::com::sun::star::chart::ErrorBarStyle::ABSOLUTE == lcl_getErrorBarStyle( xErrorBarProperties ) )
        {
            xErrorBarProperties->setPropertyValue( "PositiveError" , m_aOuterValue );
        }
    }
}

//PROP_CHART_STATISTIC_MEAN_VALUE
class WrappedMeanValueProperty : public WrappedStatisticProperty< sal_Bool >
{
public:
    virtual sal_Bool getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, sal_Bool aNewValue ) const;

    explicit WrappedMeanValueProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                       tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedMeanValueProperty();
};

WrappedMeanValueProperty::WrappedMeanValueProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< sal_Bool >( "MeanValue", uno::makeAny( sal_False ), spChart2ModelContact, ePropertyType  )
{
}
WrappedMeanValueProperty::~WrappedMeanValueProperty()
{
}

sal_Bool WrappedMeanValueProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    sal_Bool bRet = sal_False;
    uno::Reference< chart2::XRegressionCurveContainer > xRegCnt( xSeriesPropertySet, uno::UNO_QUERY );
    if( xRegCnt.is() )
        bRet = RegressionCurveHelper::hasMeanValueLine( xRegCnt );
    return bRet;
}

void WrappedMeanValueProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, sal_Bool aNewValue ) const
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegCnt( xSeriesPropertySet, uno::UNO_QUERY );
    if( xRegCnt.is() )
    {
        if(aNewValue)
            RegressionCurveHelper::addMeanValueLine( xRegCnt, 0, 0 );
        else
            RegressionCurveHelper::removeMeanValueLine( xRegCnt );
    }
}

//PROP_CHART_STATISTIC_ERROR_CATEGORY
// deprecated, replaced by ErrorBarStyle
class WrappedErrorCategoryProperty : public WrappedStatisticProperty< ::com::sun::star::chart::ChartErrorCategory >
{
public:
    virtual ::com::sun::star::chart::ChartErrorCategory getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, ::com::sun::star::chart::ChartErrorCategory aNewValue ) const;

    explicit WrappedErrorCategoryProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                           tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedErrorCategoryProperty();
};

WrappedErrorCategoryProperty::WrappedErrorCategoryProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< ::com::sun::star::chart::ChartErrorCategory >( "ErrorCategory"
            , uno::makeAny( ::com::sun::star::chart::ChartErrorCategory_NONE ), spChart2ModelContact, ePropertyType  )
{
}
WrappedErrorCategoryProperty::~WrappedErrorCategoryProperty()
{
}

::com::sun::star::chart::ChartErrorCategory WrappedErrorCategoryProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    ::com::sun::star::chart::ChartErrorCategory aRet = ::com::sun::star::chart::ChartErrorCategory_NONE;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        sal_Int32 nStyle = ::com::sun::star::chart::ErrorBarStyle::NONE;
        xErrorBarProperties->getPropertyValue( "ErrorBarStyle" ) >>= nStyle;
        switch(nStyle)
        {
            case ::com::sun::star::chart::ErrorBarStyle::NONE:
                aRet = ::com::sun::star::chart::ChartErrorCategory_NONE;
                break;
            case ::com::sun::star::chart::ErrorBarStyle::VARIANCE:
                aRet = ::com::sun::star::chart::ChartErrorCategory_VARIANCE;
                break;
            case ::com::sun::star::chart::ErrorBarStyle::STANDARD_DEVIATION:
                aRet = ::com::sun::star::chart::ChartErrorCategory_STANDARD_DEVIATION;
                break;
            case ::com::sun::star::chart::ErrorBarStyle::ABSOLUTE:
                aRet = ::com::sun::star::chart::ChartErrorCategory_CONSTANT_VALUE;
                break;
            case ::com::sun::star::chart::ErrorBarStyle::RELATIVE:
                aRet = ::com::sun::star::chart::ChartErrorCategory_PERCENT;
                break;
            case ::com::sun::star::chart::ErrorBarStyle::ERROR_MARGIN:
                aRet = ::com::sun::star::chart::ChartErrorCategory_ERROR_MARGIN;
                break;
            case ::com::sun::star::chart::ErrorBarStyle::STANDARD_ERROR:
                break;
            case ::com::sun::star::chart::ErrorBarStyle::FROM_DATA:
                break;
            default:
                break;
        }
    }
    return aRet;
}
void WrappedErrorCategoryProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, ::com::sun::star::chart::ChartErrorCategory aNewValue ) const
{
    if( !xSeriesPropertySet.is() )
        return;

    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        sal_Int32 nNewStyle = ::com::sun::star::chart::ErrorBarStyle::NONE;
        switch(aNewValue)
        {
            case ::com::sun::star::chart::ChartErrorCategory_NONE:
                nNewStyle = ::com::sun::star::chart::ErrorBarStyle::NONE;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_VARIANCE:
                nNewStyle = ::com::sun::star::chart::ErrorBarStyle::VARIANCE;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_STANDARD_DEVIATION:
                nNewStyle = ::com::sun::star::chart::ErrorBarStyle::STANDARD_DEVIATION;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_CONSTANT_VALUE:
                nNewStyle = ::com::sun::star::chart::ErrorBarStyle::ABSOLUTE;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_PERCENT:
                nNewStyle = ::com::sun::star::chart::ErrorBarStyle::RELATIVE;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_ERROR_MARGIN:
                nNewStyle = ::com::sun::star::chart::ErrorBarStyle::ERROR_MARGIN;
                break;
            default:
                break;
        }
        xErrorBarProperties->setPropertyValue( "ErrorBarStyle" , uno::makeAny(nNewStyle) );
    }
}

//PROP_CHART_STATISTIC_PERCENT_ERROR
class WrappedPercentageErrorProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const;

    explicit WrappedPercentageErrorProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                             tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedPercentageErrorProperty();

private:
    mutable Any m_aOuterValue;
};

WrappedPercentageErrorProperty::WrappedPercentageErrorProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< double >( "PercentageError"
            , uno::makeAny( double(0.0) ), spChart2ModelContact, ePropertyType  )
{
}
WrappedPercentageErrorProperty::~WrappedPercentageErrorProperty()
{
}

double WrappedPercentageErrorProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    double aRet = 0.0;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( ::com::sun::star::chart::ErrorBarStyle::RELATIVE == lcl_getErrorBarStyle( xErrorBarProperties ) )
            xErrorBarProperties->getPropertyValue( "PositiveError" ) >>= aRet;
        else
            m_aOuterValue >>= aRet;
    }
    return aRet;
}
void WrappedPercentageErrorProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        m_aOuterValue = uno::makeAny( aNewValue );
        if( ::com::sun::star::chart::ErrorBarStyle::RELATIVE == lcl_getErrorBarStyle( xErrorBarProperties ) )
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
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const;

    explicit WrappedErrorMarginProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                         tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedErrorMarginProperty();

private:
    mutable Any m_aOuterValue;
};

WrappedErrorMarginProperty::WrappedErrorMarginProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< double >( "ErrorMargin"
            , uno::makeAny( double(0.0) ), spChart2ModelContact, ePropertyType  )
{
}
WrappedErrorMarginProperty::~WrappedErrorMarginProperty()
{
}

double WrappedErrorMarginProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    double aRet = 0.0;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( ::com::sun::star::chart::ErrorBarStyle::ERROR_MARGIN == lcl_getErrorBarStyle( xErrorBarProperties ) )
            xErrorBarProperties->getPropertyValue( "PositiveError" ) >>= aRet;
        else
            m_aOuterValue >>= aRet;
    }
    return aRet;
}
void WrappedErrorMarginProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        m_aOuterValue = uno::makeAny( aNewValue );
        if( ::com::sun::star::chart::ErrorBarStyle::ERROR_MARGIN == lcl_getErrorBarStyle( xErrorBarProperties ) )
        {
            xErrorBarProperties->setPropertyValue( "PositiveError" , m_aOuterValue );
            xErrorBarProperties->setPropertyValue( "NegativeError" , m_aOuterValue );
        }
    }
}

//PROP_CHART_STATISTIC_ERROR_INDICATOR
class WrappedErrorIndicatorProperty : public WrappedStatisticProperty< ::com::sun::star::chart::ChartErrorIndicatorType >
{
public:
    virtual ::com::sun::star::chart::ChartErrorIndicatorType getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, ::com::sun::star::chart::ChartErrorIndicatorType aNewValue ) const;

    explicit WrappedErrorIndicatorProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                            tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedErrorIndicatorProperty();
};

WrappedErrorIndicatorProperty::WrappedErrorIndicatorProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< ::com::sun::star::chart::ChartErrorIndicatorType >( "ErrorIndicator"
            , uno::makeAny( ::com::sun::star::chart::ChartErrorIndicatorType_NONE ), spChart2ModelContact, ePropertyType  )
{
}
WrappedErrorIndicatorProperty::~WrappedErrorIndicatorProperty()
{
}

::com::sun::star::chart::ChartErrorIndicatorType WrappedErrorIndicatorProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    ::com::sun::star::chart::ChartErrorIndicatorType aRet = ::com::sun::star::chart::ChartErrorIndicatorType_NONE;
    m_aDefaultValue >>= aRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        sal_Bool bPositive = sal_False;
        sal_Bool bNegative = sal_False;
        xErrorBarProperties->getPropertyValue( "ShowPositiveError" ) >>= bPositive;
        xErrorBarProperties->getPropertyValue( "ShowNegativeError" ) >>= bNegative;

        if( bPositive && bNegative )
            aRet = ::com::sun::star::chart::ChartErrorIndicatorType_TOP_AND_BOTTOM;
        else if( bPositive && !bNegative )
            aRet = ::com::sun::star::chart::ChartErrorIndicatorType_UPPER;
        else if( !bPositive && bNegative )
            aRet = ::com::sun::star::chart::ChartErrorIndicatorType_LOWER;
    }
    return aRet;
}
void WrappedErrorIndicatorProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, ::com::sun::star::chart::ChartErrorIndicatorType aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        sal_Bool bPositive = sal_False;
        sal_Bool bNegative = sal_False;
        switch( aNewValue )
        {
            case ::com::sun::star::chart::ChartErrorIndicatorType_TOP_AND_BOTTOM:
                bPositive = sal_True;
                bNegative = sal_True;
                break;
            case ::com::sun::star::chart::ChartErrorIndicatorType_UPPER:
                bPositive = sal_True;
                break;
            case ::com::sun::star::chart::ChartErrorIndicatorType_LOWER:
                bNegative = sal_True;
                break;
            default:
                break;
        }

        xErrorBarProperties->setPropertyValue( "ShowPositiveError" , uno::makeAny(bPositive) );
        xErrorBarProperties->setPropertyValue( "ShowNegativeError" , uno::makeAny(bNegative) );
    }
}

//PROP_CHART_STATISTIC_ERROR_BAR_STYLE
// this is the new constant group that replaces the deprecated enum ChartErrorCategory
class WrappedErrorBarStyleProperty : public WrappedStatisticProperty< sal_Int32 >
{
public:
    virtual sal_Int32 getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, sal_Int32 nNewValue ) const;

    explicit WrappedErrorBarStyleProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact1,
                                           tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedErrorBarStyleProperty();
};

WrappedErrorBarStyleProperty::WrappedErrorBarStyleProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< sal_Int32 >( "ErrorBarStyle"
            , uno::makeAny( ::com::sun::star::chart::ErrorBarStyle::NONE ), spChart2ModelContact, ePropertyType  )
{
}
WrappedErrorBarStyleProperty::~WrappedErrorBarStyleProperty()
{
}

sal_Int32 WrappedErrorBarStyleProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    sal_Int32 nRet = ::com::sun::star::chart::ErrorBarStyle::NONE;
    m_aDefaultValue >>= nRet;
    uno::Reference< beans::XPropertySet > xErrorBarProperties;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        xErrorBarProperties->getPropertyValue( "ErrorBarStyle" ) >>= nRet;
    }
    return nRet;
}
void WrappedErrorBarStyleProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, sal_Int32 nNewValue ) const
{
    if( !xSeriesPropertySet.is() )
        return;

    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        xErrorBarProperties->setPropertyValue( "ErrorBarStyle" , uno::makeAny( nNewValue ));
    }
}

//PROP_CHART_STATISTIC_ERROR_RANGE_POSITIVE
class WrappedErrorBarRangePositiveProperty : public WrappedStatisticProperty< OUString >
{
public:
    virtual OUString getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, OUString aNewValue ) const;

    explicit WrappedErrorBarRangePositiveProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                                   tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedErrorBarRangePositiveProperty();

private:
    mutable Any m_aOuterValue;
};

WrappedErrorBarRangePositiveProperty::WrappedErrorBarRangePositiveProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< OUString >( "ErrorBarRangePositive"
            , uno::makeAny( OUString() ), spChart2ModelContact, ePropertyType  )
{
}
WrappedErrorBarRangePositiveProperty::~WrappedErrorBarRangePositiveProperty()
{
}

OUString WrappedErrorBarRangePositiveProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    OUString aRet;
    m_aDefaultValue >>= aRet;
    uno::Reference< chart2::data::XDataSource > xErrorBarDataSource;
    if( xSeriesPropertySet.is() &&
        ( xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xErrorBarDataSource ) &&
        xErrorBarDataSource.is())
    {
        uno::Reference< chart2::data::XDataSequence > xSeq(
            StatisticsHelper::getErrorDataSequenceFromDataSource(
                xErrorBarDataSource, true /* positive */, true /* y-error */ ));
        if( xSeq.is())
            aRet = xSeq->getSourceRangeRepresentation();
        else
            m_aOuterValue >>= aRet;
    }
    lcl_ConvertRangeToXML( aRet, m_spChart2ModelContact );
    return aRet;
}

void WrappedErrorBarRangePositiveProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, OUString aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        uno::Reference< chart2::data::XDataProvider > xDataProvider(
            lcl_getDataProviderFromContact( m_spChart2ModelContact ));
        uno::Reference< chart2::data::XDataSource > xDataSource( xErrorBarProperties, uno::UNO_QUERY );
        if( xDataSource.is() && xDataProvider.is())
        {
            OUString aXMLRange( aNewValue );
            lcl_ConvertRangeFromXML( aNewValue, m_spChart2ModelContact );
            StatisticsHelper::setErrorDataSequence(
                xDataSource, xDataProvider, aNewValue, true /* positive */, true /* y-error */, &aXMLRange );
            m_aOuterValue <<= aNewValue;
        }
    }
}

//PROP_CHART_STATISTIC_ERROR_RANGE_NEGATIVE
class WrappedErrorBarRangeNegativeProperty : public WrappedStatisticProperty< OUString >
{
public:
    virtual OUString getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, OUString aNewValue ) const;

    explicit WrappedErrorBarRangeNegativeProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                                   tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedErrorBarRangeNegativeProperty();

private:
    mutable Any m_aOuterValue;
};

WrappedErrorBarRangeNegativeProperty::WrappedErrorBarRangeNegativeProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< OUString >( "ErrorBarRangeNegative"
            , uno::makeAny( OUString() ), spChart2ModelContact, ePropertyType  )
{
}
WrappedErrorBarRangeNegativeProperty::~WrappedErrorBarRangeNegativeProperty()
{
}

OUString WrappedErrorBarRangeNegativeProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    OUString aRet;
    m_aDefaultValue >>= aRet;
    uno::Reference< chart2::data::XDataSource > xErrorBarDataSource;
    if( xSeriesPropertySet.is() &&
        ( xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xErrorBarDataSource ) &&
        xErrorBarDataSource.is())
    {
        uno::Reference< chart2::data::XDataSequence > xSeq(
            StatisticsHelper::getErrorDataSequenceFromDataSource(
                xErrorBarDataSource, false /* positive */, true /* y-error */ ));
        if( xSeq.is())
            aRet = xSeq->getSourceRangeRepresentation();
        else
            m_aOuterValue >>= aRet;
    }
    lcl_ConvertRangeToXML( aRet, m_spChart2ModelContact );
    return aRet;
}

void WrappedErrorBarRangeNegativeProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, OUString aNewValue ) const
{
    uno::Reference< beans::XPropertySet > xErrorBarProperties( getOrCreateErrorBarProperties(xSeriesPropertySet) );
    if( xErrorBarProperties.is() )
    {
        uno::Reference< chart2::data::XDataProvider > xDataProvider(
            lcl_getDataProviderFromContact( m_spChart2ModelContact ));
        uno::Reference< chart2::data::XDataSource > xDataSource( xErrorBarProperties, uno::UNO_QUERY );
        if( xDataSource.is() && xDataProvider.is())
        {
            OUString aXMLRange( aNewValue );
            lcl_ConvertRangeFromXML( aNewValue, m_spChart2ModelContact );
            StatisticsHelper::setErrorDataSequence(
                xDataSource, xDataProvider, aNewValue, false /* positive */, true /* y-error */, &aXMLRange );
            m_aOuterValue <<= aNewValue;
        }
    }
}

//PROP_CHART_STATISTIC_REGRESSION_CURVES
class WrappedRegressionCurvesProperty : public WrappedStatisticProperty< ::com::sun::star::chart::ChartRegressionCurveType >
{
public:
    virtual ::com::sun::star::chart::ChartRegressionCurveType getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, ::com::sun::star::chart::ChartRegressionCurveType aNewValue ) const;

    explicit WrappedRegressionCurvesProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                              tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedRegressionCurvesProperty();
};

WrappedRegressionCurvesProperty::WrappedRegressionCurvesProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< ::com::sun::star::chart::ChartRegressionCurveType >( "RegressionCurves"
        , lcl_getRegressionDefault(), spChart2ModelContact, ePropertyType  )
{
}
WrappedRegressionCurvesProperty::~WrappedRegressionCurvesProperty()
{
}

::com::sun::star::chart::ChartRegressionCurveType WrappedRegressionCurvesProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    ::com::sun::star::chart::ChartRegressionCurveType aRet;
    m_aDefaultValue >>= aRet;
    uno::Reference< chart2::XRegressionCurveContainer > xRegCnt( xSeriesPropertySet, uno::UNO_QUERY );
    if( xRegCnt.is() )
    {
        aRet = lcl_getRegressionCurveType(
            RegressionCurveHelper::getFirstRegressTypeNotMeanValueLine( xRegCnt ) );
    }
    return aRet;
}
void WrappedRegressionCurvesProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, ::com::sun::star::chart::ChartRegressionCurveType aNewValue ) const
{
    uno::Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer( xSeriesPropertySet, uno::UNO_QUERY );
    uno::Reference< chart2::XRegressionCurve > xRegressionCurve( xSeriesPropertySet, uno::UNO_QUERY );

    if( xRegressionCurveContainer.is() && xRegressionCurve.is() )
    {
        RegressionCurveHelper::tRegressionType eNewRegressionType = lcl_getRegressionType( aNewValue );

        RegressionCurveHelper::changeRegressionCurveType(
                        eNewRegressionType,
                        xRegressionCurveContainer,
                        xRegressionCurve,
                        uno::Reference< uno::XComponentContext >());
    }
}

//PROP_CHART_STATISTIC_REGRESSION_PROPERTIES
//PROP_CHART_STATISTIC_ERROR_PROPERTIES
//PROP_CHART_STATISTIC_MEAN_VALUE_PROPERTIES
class WrappedStatisticPropertySetProperty : public WrappedStatisticProperty< Reference< beans::XPropertySet > >
{
public:
    virtual Reference< beans::XPropertySet > getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    // properties are read-only, so this method should never be called
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, Reference< beans::XPropertySet > xNewValue ) const;

    enum PropertySetType
    {
        PROPERTY_SET_TYPE_REGRESSION,
        PROPERTY_SET_TYPE_ERROR_BAR,
        PROPERTY_SET_TYPE_MEAN_VALUE
    };

    explicit WrappedStatisticPropertySetProperty(
        PropertySetType ePropertySetType, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
        tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedStatisticPropertySetProperty();

private:
    PropertySetType m_eType;
};

WrappedStatisticPropertySetProperty::WrappedStatisticPropertySetProperty(
    PropertySetType ePropertySetType
    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact
    , tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedStatisticProperty< Reference< beans::XPropertySet > >(
            (ePropertySetType == PROPERTY_SET_TYPE_REGRESSION)
            ? OUString("DataRegressionProperties")
            : (ePropertySetType == PROPERTY_SET_TYPE_ERROR_BAR)
              ? OUString("DataErrorProperties")
              : OUString("DataMeanValueProperties")
            , uno::Any(), spChart2ModelContact, ePropertyType  )
        , m_eType( ePropertySetType )
{
}
WrappedStatisticPropertySetProperty::~WrappedStatisticPropertySetProperty()
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
                xSeriesPropertySet->getPropertyValue( "ErrorBarY" ) >>= xResult;
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
    , Reference< beans::XPropertySet > /* xNewValue */ ) const
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
void lcl_addWrappedProperties( std::vector< WrappedProperty* >& rList
            , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact
            , tSeriesOrDiagramPropertyType ePropertyType )
{
    rList.push_back( new WrappedConstantErrorLowProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedConstantErrorHighProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedMeanValueProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedErrorCategoryProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedErrorBarStyleProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedPercentageErrorProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedErrorMarginProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedErrorIndicatorProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedErrorBarRangePositiveProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedErrorBarRangeNegativeProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedRegressionCurvesProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedStatisticPropertySetProperty(
                         WrappedStatisticPropertySetProperty::PROPERTY_SET_TYPE_REGRESSION, spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedStatisticPropertySetProperty(
                         WrappedStatisticPropertySetProperty::PROPERTY_SET_TYPE_ERROR_BAR,  spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedStatisticPropertySetProperty(
                         WrappedStatisticPropertySetProperty::PROPERTY_SET_TYPE_MEAN_VALUE, spChart2ModelContact, ePropertyType ) );
}

}//anonymous namespace

void WrappedStatisticProperties::addProperties( ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "ConstantErrorLow",
                  PROP_CHART_STATISTIC_CONST_ERROR_LOW,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "ConstantErrorHigh",
                  PROP_CHART_STATISTIC_CONST_ERROR_HIGH,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "MeanValue",
                  PROP_CHART_STATISTIC_MEAN_VALUE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "ErrorCategory",
                  PROP_CHART_STATISTIC_ERROR_CATEGORY,
                  ::getCppuType( reinterpret_cast< ::com::sun::star::chart::ChartErrorCategory * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "ErrorBarStyle",
                  PROP_CHART_STATISTIC_ERROR_BAR_STYLE,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "PercentageError",
                  PROP_CHART_STATISTIC_PERCENT_ERROR,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "ErrorMargin",
                  PROP_CHART_STATISTIC_ERROR_MARGIN,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "ErrorIndicator",
                  PROP_CHART_STATISTIC_ERROR_INDICATOR,
                  ::getCppuType( reinterpret_cast< ::com::sun::star::chart::ChartErrorIndicatorType * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "ErrorBarRangePositive",
                  PROP_CHART_STATISTIC_ERROR_RANGE_POSITIVE,
                  ::getCppuType( reinterpret_cast< OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "ErrorBarRangeNegative",
                  PROP_CHART_STATISTIC_ERROR_RANGE_NEGATIVE,
                  ::getCppuType( reinterpret_cast< OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "RegressionCurves",
                  PROP_CHART_STATISTIC_REGRESSION_CURVES,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartRegressionCurveType * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "DataRegressionProperties",
                  PROP_CHART_STATISTIC_REGRESSION_PROPERTIES,
                  ::getCppuType( reinterpret_cast< const Reference< beans::XPropertySet > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::READONLY
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "DataErrorProperties",
                  PROP_CHART_STATISTIC_ERROR_PROPERTIES,
                  ::getCppuType( reinterpret_cast< const Reference< beans::XPropertySet > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::READONLY
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "DataMeanValueProperties",
                  PROP_CHART_STATISTIC_MEAN_VALUE_PROPERTIES,
                  ::getCppuType( reinterpret_cast< const Reference< beans::XPropertySet > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::READONLY
                  | beans::PropertyAttribute::MAYBEVOID ));
}

void WrappedStatisticProperties::addWrappedPropertiesForSeries( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact, DATA_SERIES );
}

void WrappedStatisticProperties::addWrappedPropertiesForDiagram( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact, DIAGRAM  );
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
