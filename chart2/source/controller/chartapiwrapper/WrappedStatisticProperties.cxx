/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedStatisticProperties.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:24:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedStatisticProperties.hxx"
#include "WrappedSeriesOrDiagramProperty.hxx"
#include "macros.hxx"
#include "FastPropertyIdRanges.hxx"
#include "RegressionCurveHelper.hxx"
#include "DiagramHelper.hxx"
#include "ErrorBar.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTERRORCATEGORY_HPP_
#include <com/sun/star/chart/ChartErrorCategory.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTERRORINDICATORTYPE_HPP_
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTREGRESSIONCURVETYPE_HPP_
#include <com/sun/star/chart/ChartRegressionCurveType.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_ERRORBARSTYLE_HPP_
#include <com/sun/star/chart2/ErrorBarStyle.hpp>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::rtl::OUString;

//.............................................................................
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

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
        default:
            eRet = ::com::sun::star::chart::ChartRegressionCurveType_NONE;
            break;
    }
    return eRet;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

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
        case ::com::sun::star::chart::ChartRegressionCurveType_POWER:
            eRet = RegressionCurveHelper::REGRESSION_TYPE_POWER;
            break;
        default:
            eRet = RegressionCurveHelper::REGRESSION_TYPE_NONE;
            break;
    }
    return eRet;
}

chart2::ErrorBarStyle lcl_getErrorBarStyle( const uno::Reference< beans::XPropertySet >& xErrorBarProperties )
{
    chart2::ErrorBarStyle eStyle = chart2::ErrorBarStyle_NONE;
    if(xErrorBarProperties.is())
        xErrorBarProperties->getPropertyValue( C2U( "ErrorBarStyle" )) >>= eStyle;
    return eStyle;
}

}//anonymous namespace

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< typename PROPERTYTYPE >
class WrappedStatisticProperty : public WrappedSeriesOrDiagramProperty< PROPERTYTYPE >
{
public:
    explicit WrappedStatisticProperty( const ::rtl::OUString& rName, const Any& rDefaulValue
                              , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )//if !spChart2ModelContact.get() this property does belong to a single series and not to the whole diagram
            : WrappedSeriesOrDiagramProperty< PROPERTYTYPE >(rName,rDefaulValue,spChart2ModelContact)
    {}
    virtual ~WrappedStatisticProperty() {};


protected:
    uno::Reference< beans::XPropertySet > getOrCreateErrorBarProperties( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
    {
        if(!xSeriesPropertySet.is())
            return 0;
        uno::Reference< beans::XPropertySet > xErrorBarProperties;
        xSeriesPropertySet->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProperties;
        if( !xErrorBarProperties.is() )
        {
            // todo: use a valid context
            xErrorBarProperties = new ::chart::ErrorBar( uno::Reference< uno::XComponentContext >() );
            //default in new and old api are different
            xErrorBarProperties->setPropertyValue( C2U( "ShowPositiveError" ), uno::makeAny(sal_Bool(sal_False)) );
            xErrorBarProperties->setPropertyValue( C2U( "ShowNegativeError" ), uno::makeAny(sal_Bool(sal_False)) );
            xErrorBarProperties->setPropertyValue( C2U( "ErrorBarStyle" ), uno::makeAny(chart2::ErrorBarStyle_NONE) );
            xSeriesPropertySet->setPropertyValue( C2U( "ErrorBarY" ), uno::makeAny( xErrorBarProperties ) );
        }
        return xErrorBarProperties;
    }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//PROP_CHART_STATISTIC_CONST_ERROR_LOW
class WrappedConstantErrorLowProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const;

    explicit WrappedConstantErrorLowProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );//if !spChart2ModelContact.get() this property does belong to a single series and not to the whole diagram
    virtual ~WrappedConstantErrorLowProperty();

private:
    mutable Any m_aOuterValue;
};

WrappedConstantErrorLowProperty::WrappedConstantErrorLowProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStatisticProperty< double >( C2U("ConstantErrorLow")
            , uno::makeAny( double(0.0) ), spChart2ModelContact )
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
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( chart2::ErrorBarStyle_ABSOLUTE == lcl_getErrorBarStyle( xErrorBarProperties ) )
            xErrorBarProperties->getPropertyValue( C2U( "NegativeError" )) >>= aRet;
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
        if( chart2::ErrorBarStyle_ABSOLUTE == lcl_getErrorBarStyle( xErrorBarProperties ) )
        {
            xErrorBarProperties->setPropertyValue( C2U( "NegativeError" ), m_aOuterValue );
        }
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//PROP_CHART_STATISTIC_CONST_ERROR_HIGH
class WrappedConstantErrorHighProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const;

    explicit WrappedConstantErrorHighProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );//if !spChart2ModelContact.get() this property does belong to a single series and not to the whole diagram
    virtual ~WrappedConstantErrorHighProperty();

private:
    mutable Any m_aOuterValue;
};

WrappedConstantErrorHighProperty::WrappedConstantErrorHighProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStatisticProperty< double >( C2U("ConstantErrorHigh")
            , uno::makeAny( double(0.0) ), spChart2ModelContact )
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
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( chart2::ErrorBarStyle_ABSOLUTE == lcl_getErrorBarStyle( xErrorBarProperties ) )
            xErrorBarProperties->getPropertyValue( C2U( "PositiveError" )) >>= aRet;
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
        if( chart2::ErrorBarStyle_ABSOLUTE == lcl_getErrorBarStyle( xErrorBarProperties ) )
        {
            xErrorBarProperties->setPropertyValue( C2U( "PositiveError" ), m_aOuterValue );
        }
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//PROP_CHART_STATISTIC_MEAN_VALUE
class WrappedMeanValueProperty : public WrappedStatisticProperty< sal_Bool >
{
public:
    virtual sal_Bool getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, sal_Bool aNewValue ) const;

    explicit WrappedMeanValueProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );//if !spChart2ModelContact.get() this property does belong to a single series and not to the whole diagram
    virtual ~WrappedMeanValueProperty();
};

WrappedMeanValueProperty::WrappedMeanValueProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStatisticProperty< sal_Bool >( C2U("MeanValue"), uno::makeAny( sal_False ), spChart2ModelContact )
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
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//PROP_CHART_STATISTIC_ERROR_CATEGORY
class WrappedErrorCategoryProperty : public WrappedStatisticProperty< ::com::sun::star::chart::ChartErrorCategory >
{
public:
    virtual ::com::sun::star::chart::ChartErrorCategory getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, ::com::sun::star::chart::ChartErrorCategory aNewValue ) const;

    explicit WrappedErrorCategoryProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );//if !spChart2ModelContact.get() this property does belong to a single series and not to the whole diagram
    virtual ~WrappedErrorCategoryProperty();
};

WrappedErrorCategoryProperty::WrappedErrorCategoryProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStatisticProperty< ::com::sun::star::chart::ChartErrorCategory >( C2U("ErrorCategory")
            , uno::makeAny( ::com::sun::star::chart::ChartErrorCategory_NONE ), spChart2ModelContact )
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
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        chart2::ErrorBarStyle eStyle = chart2::ErrorBarStyle_NONE;
        xErrorBarProperties->getPropertyValue( C2U( "ErrorBarStyle" )) >>= eStyle;
        switch(eStyle)
        {
            case chart2::ErrorBarStyle_NONE:
                aRet = ::com::sun::star::chart::ChartErrorCategory_NONE;
                break;
            case chart2::ErrorBarStyle_VARIANCE:
                aRet = ::com::sun::star::chart::ChartErrorCategory_VARIANCE;
                break;
            case chart2::ErrorBarStyle_STANDARD_DEVIATION:
                aRet = ::com::sun::star::chart::ChartErrorCategory_STANDARD_DEVIATION;
                break;
            case chart2::ErrorBarStyle_ABSOLUTE:
                aRet = ::com::sun::star::chart::ChartErrorCategory_CONSTANT_VALUE;
                break;
            case chart2::ErrorBarStyle_RELATIVE:
                aRet = ::com::sun::star::chart::ChartErrorCategory_PERCENT;
                break;
            case chart2::ErrorBarStyle_ERROR_MARGIN:
                aRet = ::com::sun::star::chart::ChartErrorCategory_ERROR_MARGIN;
                break;
            case chart2::ErrorBarStyle_STANDARD_ERROR:
                break;
            case chart2::ErrorBarStyle_FROM_DATA:
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
        chart2::ErrorBarStyle eNewStyle = chart2::ErrorBarStyle_NONE;
        switch(aNewValue)
        {
            case ::com::sun::star::chart::ChartErrorCategory_NONE:
                eNewStyle = chart2::ErrorBarStyle_NONE;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_VARIANCE:
                eNewStyle = chart2::ErrorBarStyle_VARIANCE;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_STANDARD_DEVIATION:
                eNewStyle = chart2::ErrorBarStyle_STANDARD_DEVIATION;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_CONSTANT_VALUE:
                eNewStyle = chart2::ErrorBarStyle_ABSOLUTE;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_PERCENT:
                eNewStyle = chart2::ErrorBarStyle_RELATIVE;
                break;
            case ::com::sun::star::chart::ChartErrorCategory_ERROR_MARGIN:
                eNewStyle = chart2::ErrorBarStyle_ERROR_MARGIN;
                break;
            default:
                break;
        }
        xErrorBarProperties->setPropertyValue( C2U( "ErrorBarStyle" ), uno::makeAny(eNewStyle) );
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//PROP_CHART_STATISTIC_PERCENT_ERROR
class WrappedPercentageErrorProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const;

    explicit WrappedPercentageErrorProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );//if !spChart2ModelContact.get() this property does belong to a single series and not to the whole diagram
    virtual ~WrappedPercentageErrorProperty();

private:
    mutable Any m_aOuterValue;
};

WrappedPercentageErrorProperty::WrappedPercentageErrorProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStatisticProperty< double >( C2U("PercentageError")
            , uno::makeAny( double(0.0) ), spChart2ModelContact )
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
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( chart2::ErrorBarStyle_RELATIVE == lcl_getErrorBarStyle( xErrorBarProperties ) )
            xErrorBarProperties->getPropertyValue( C2U( "PositiveError" )) >>= aRet;
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
        if( chart2::ErrorBarStyle_RELATIVE == lcl_getErrorBarStyle( xErrorBarProperties ) )
        {
            xErrorBarProperties->setPropertyValue( C2U( "PositiveError" ), m_aOuterValue );
            xErrorBarProperties->setPropertyValue( C2U( "NegativeError" ), m_aOuterValue );
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//PROP_CHART_STATISTIC_ERROR_MARGIN
class WrappedErrorMarginProperty : public WrappedStatisticProperty< double >
{
public:
    virtual double getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, double aNewValue ) const;

    explicit WrappedErrorMarginProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );//if !spChart2ModelContact.get() this property does belong to a single series and not to the whole diagram
    virtual ~WrappedErrorMarginProperty();

private:
    mutable Any m_aOuterValue;
};

WrappedErrorMarginProperty::WrappedErrorMarginProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStatisticProperty< double >( C2U("ErrorMargin")
            , uno::makeAny( double(0.0) ), spChart2ModelContact )
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
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        if( chart2::ErrorBarStyle_ERROR_MARGIN == lcl_getErrorBarStyle( xErrorBarProperties ) )
            xErrorBarProperties->getPropertyValue( C2U( "PositiveError" )) >>= aRet;
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
        if( chart2::ErrorBarStyle_ERROR_MARGIN == lcl_getErrorBarStyle( xErrorBarProperties ) )
        {
            xErrorBarProperties->setPropertyValue( C2U( "PositiveError" ), m_aOuterValue );
            xErrorBarProperties->setPropertyValue( C2U( "NegativeError" ), m_aOuterValue );
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//PROP_CHART_STATISTIC_ERROR_INDICATOR
class WrappedErrorIndicatorProperty : public WrappedStatisticProperty< ::com::sun::star::chart::ChartErrorIndicatorType >
{
public:
    virtual ::com::sun::star::chart::ChartErrorIndicatorType getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, ::com::sun::star::chart::ChartErrorIndicatorType aNewValue ) const;

    explicit WrappedErrorIndicatorProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );//if !spChart2ModelContact.get() this property does belong to a single series and not to the whole diagram
    virtual ~WrappedErrorIndicatorProperty();
};

WrappedErrorIndicatorProperty::WrappedErrorIndicatorProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStatisticProperty< ::com::sun::star::chart::ChartErrorIndicatorType >( C2U("ErrorIndicator")
            , uno::makeAny( ::com::sun::star::chart::ChartErrorIndicatorType_NONE ), spChart2ModelContact )
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
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue( C2U( "ErrorBarY" )) >>= xErrorBarProperties ) && xErrorBarProperties.is())
    {
        sal_Bool bPositive = sal_False;
        sal_Bool bNegative = sal_False;
        xErrorBarProperties->getPropertyValue( C2U( "ShowPositiveError" )) >>= bPositive;
        xErrorBarProperties->getPropertyValue( C2U( "ShowNegativeError" )) >>= bNegative;

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

        xErrorBarProperties->setPropertyValue( C2U( "ShowPositiveError" ), uno::makeAny(bPositive) );
        xErrorBarProperties->setPropertyValue( C2U( "ShowNegativeError" ), uno::makeAny(bNegative) );
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//PROP_CHART_STATISTIC_REGRESSION_CURVES
class WrappedRegressionCurvesProperty : public WrappedStatisticProperty< ::com::sun::star::chart::ChartRegressionCurveType >
{
public:
    virtual ::com::sun::star::chart::ChartRegressionCurveType getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, ::com::sun::star::chart::ChartRegressionCurveType aNewValue ) const;

    explicit WrappedRegressionCurvesProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );//if !spChart2ModelContact.get() this property does belong to a single series and not to the whole diagram
    virtual ~WrappedRegressionCurvesProperty();
};

WrappedRegressionCurvesProperty::WrappedRegressionCurvesProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStatisticProperty< ::com::sun::star::chart::ChartRegressionCurveType >( C2U("RegressionCurves")
        , lcl_getRegressionDefault(), spChart2ModelContact )
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
    uno::Reference< chart2::XRegressionCurveContainer > xRegCnt( xSeriesPropertySet, uno::UNO_QUERY );
    if( xRegCnt.is() )
    {
        RegressionCurveHelper::tRegressionType eNewRegressionType = lcl_getRegressionType( aNewValue );
        RegressionCurveHelper::removeAllExceptMeanValueLine( xRegCnt );
        if( eNewRegressionType != RegressionCurveHelper::REGRESSION_TYPE_NONE )
            RegressionCurveHelper::addRegressionCurve( eNewRegressionType, xRegCnt, 0, 0 );
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
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
        PropertySetType ePropertySetType, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedStatisticPropertySetProperty();

private:
    PropertySetType m_eType;
};

WrappedStatisticPropertySetProperty::WrappedStatisticPropertySetProperty(
    PropertySetType ePropertySetType
    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStatisticProperty< Reference< beans::XPropertySet > >(
            (ePropertySetType == PROPERTY_SET_TYPE_REGRESSION)
            ? C2U("DataRegressionProperties")
            : (ePropertySetType == PROPERTY_SET_TYPE_ERROR_BAR)
            ? C2U("DataErrorProperties")
            : C2U("DataMeanValueProperties")
            , uno::Any(), spChart2ModelContact )
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
                xSeriesPropertySet->getPropertyValue( C2U( "ErrorBarY" )) >>= xResult;
            break;
        case PROPERTY_SET_TYPE_MEAN_VALUE:
            if( xRegCnt.is() )
                xResult.set( RegressionCurveHelper::getMeanValueLine( xRegCnt ), uno::UNO_QUERY );
            break;
    }

    return xResult;
}

void WrappedStatisticPropertySetProperty::setValueToSeries(
    const Reference< beans::XPropertySet >& xSeriesPropertySet
    , Reference< beans::XPropertySet > xNewValue ) const
{
    OSL_ENSURE( false, "Trying to set a read-only property" );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace
{
enum
{
    //statistic properties
    PROP_CHART_STATISTIC_CONST_ERROR_LOW = FAST_PROPERTY_ID_START_CHART_STATISTIC_PROP,
    PROP_CHART_STATISTIC_CONST_ERROR_HIGH,
    PROP_CHART_STATISTIC_MEAN_VALUE,
    PROP_CHART_STATISTIC_ERROR_CATEGORY,
    PROP_CHART_STATISTIC_PERCENT_ERROR,
    PROP_CHART_STATISTIC_ERROR_MARGIN,
    PROP_CHART_STATISTIC_ERROR_INDICATOR,
    PROP_CHART_STATISTIC_REGRESSION_CURVES,
    PROP_CHART_STATISTIC_REGRESSION_PROPERTIES,
    PROP_CHART_STATISTIC_ERROR_PROPERTIES,
    PROP_CHART_STATISTIC_MEAN_VALUE_PROPERTIES
};

void lcl_addWrappedProperties( std::vector< WrappedProperty* >& rList
            , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    //if !spChart2ModelContact.get() then the created properties do belong to a single series
    //otherwise they do belong to the whole diagram

    rList.push_back( new WrappedConstantErrorLowProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedConstantErrorHighProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedMeanValueProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedErrorCategoryProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedPercentageErrorProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedErrorMarginProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedErrorIndicatorProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedRegressionCurvesProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedStatisticPropertySetProperty(
                         WrappedStatisticPropertySetProperty::PROPERTY_SET_TYPE_REGRESSION, spChart2ModelContact ) );
    rList.push_back( new WrappedStatisticPropertySetProperty(
                         WrappedStatisticPropertySetProperty::PROPERTY_SET_TYPE_ERROR_BAR,  spChart2ModelContact ) );
    rList.push_back( new WrappedStatisticPropertySetProperty(
                         WrappedStatisticPropertySetProperty::PROPERTY_SET_TYPE_MEAN_VALUE, spChart2ModelContact ) );
}

}//anonymous namespace

//static
void WrappedStatisticProperties::addProperties( ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "ConstantErrorLow" ),
                  PROP_CHART_STATISTIC_CONST_ERROR_LOW,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ConstantErrorHigh" ),
                  PROP_CHART_STATISTIC_CONST_ERROR_HIGH,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "MeanValue" ),
                  PROP_CHART_STATISTIC_MEAN_VALUE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ErrorCategory" ),
                  PROP_CHART_STATISTIC_ERROR_CATEGORY,
                  ::getCppuType( reinterpret_cast< ::com::sun::star::chart::ChartErrorCategory * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "PercentageError" ),
                  PROP_CHART_STATISTIC_PERCENT_ERROR,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ErrorMargin" ),
                  PROP_CHART_STATISTIC_ERROR_MARGIN,
                  ::getCppuType( reinterpret_cast< double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ErrorIndicator" ),
                  PROP_CHART_STATISTIC_ERROR_INDICATOR,
                  ::getCppuType( reinterpret_cast< ::com::sun::star::chart::ChartErrorIndicatorType * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "RegressionCurves" ),
                  PROP_CHART_STATISTIC_REGRESSION_CURVES,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartRegressionCurveType * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "DataRegressionProperties" ),
                  PROP_CHART_STATISTIC_REGRESSION_PROPERTIES,
                  ::getCppuType( reinterpret_cast< const Reference< beans::XPropertySet > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::READONLY
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "DataErrorProperties" ),
                  PROP_CHART_STATISTIC_ERROR_PROPERTIES,
                  ::getCppuType( reinterpret_cast< const Reference< beans::XPropertySet > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::READONLY
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "DataMeanValueProperties" ),
                  PROP_CHART_STATISTIC_MEAN_VALUE_PROPERTIES,
                  ::getCppuType( reinterpret_cast< const Reference< beans::XPropertySet > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::READONLY
                  | beans::PropertyAttribute::MAYBEVOID ));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//static
void WrappedStatisticProperties::addWrappedPropertiesForSeries( std::vector< WrappedProperty* >& rList )
{
    lcl_addWrappedProperties( rList, ::boost::shared_ptr< Chart2ModelContact >() );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//static
void WrappedStatisticProperties::addWrappedPropertiesForDiagram( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

} //namespace wrapper
} //namespace chart
//.............................................................................
