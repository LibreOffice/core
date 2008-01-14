/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedScaleProperty.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:56:19 $
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

#include "WrappedScaleProperty.hxx"
#include "macros.hxx"
#include "Scaling.hxx"
#include "CommonConverters.hxx"
#include "AxisHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_EXPLICITINCREMENTDATA_HPP_
#include <com/sun/star/chart2/ExplicitIncrementData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_AXISORIENTATION_HPP_
#include <com/sun/star/chart2/AxisOrientation.hpp>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

//.............................................................................
namespace chart
{
namespace wrapper
{

WrappedScaleProperty::WrappedScaleProperty( tScaleProperty eScaleProperty
                , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(OUString(),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_eScaleProperty( eScaleProperty )
{
    switch( m_eScaleProperty )
    {
        case SCALE_PROP_MAX:
            m_aOuterName = C2U("Max");
            break;
        case SCALE_PROP_MIN:
            m_aOuterName = C2U("Min");
            break;
        case SCALE_PROP_ORIGIN:
            m_aOuterName = C2U("Origin");
            break;
        case SCALE_PROP_STEPMAIN:
            m_aOuterName = C2U("StepMain");
            break;
        case SCALE_PROP_STEPHELP:
            m_aOuterName = C2U("StepHelp");
            break;
        case SCALE_PROP_AUTO_MAX:
            m_aOuterName = C2U("AutoMax");
            break;
        case SCALE_PROP_AUTO_MIN:
            m_aOuterName = C2U("AutoMin");
            break;
        case SCALE_PROP_AUTO_ORIGIN:
            m_aOuterName = C2U("AutoOrigin");
            break;
        case SCALE_PROP_AUTO_STEPMAIN:
            m_aOuterName = C2U("AutoStepMain");
            break;
        case SCALE_PROP_AUTO_STEPHELP:
            m_aOuterName = C2U("AutoStepHelp");
            break;
        case SCALE_PROP_LOGARITHMIC:
            m_aOuterName = C2U("Logarithmic");
            break;
        case SCALE_PROP_REVERSEDIRECTION:
            m_aOuterName = C2U("ReverseDirection");
            break;
        default:
            OSL_ENSURE(false,"unknown scale property");
            break;
    }
}

WrappedScaleProperty::~WrappedScaleProperty()
{
}

//static
void WrappedScaleProperty::addWrappedProperties( std::vector< WrappedProperty* >& rList
            , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_MAX, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_MIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_ORIGIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_STEPMAIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_STEPHELP, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AUTO_MAX, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AUTO_MIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AUTO_ORIGIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AUTO_STEPMAIN, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_AUTO_STEPHELP, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_LOGARITHMIC, spChart2ModelContact ) );
    rList.push_back( new WrappedScaleProperty( SCALE_PROP_REVERSEDIRECTION, spChart2ModelContact ) );
}

void WrappedScaleProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    setPropertyValue( m_eScaleProperty, rOuterValue, xInnerPropertySet );
}

Any WrappedScaleProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return getPropertyValue( m_eScaleProperty, xInnerPropertySet );
}

void WrappedScaleProperty::setPropertyValue( tScaleProperty eScaleProperty, const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aOuterValue = rOuterValue;

    Reference< chart2::XAxis > xAxis( xInnerPropertySet, uno::UNO_QUERY );
    OSL_ENSURE(xAxis.is(),"need an XAxis");
    if(!xAxis.is())
        return;

    bool bSetScaleData     = false;

    chart2::ScaleData aScaleData( xAxis->getScaleData() );

    sal_Bool bBool = false;
    switch( eScaleProperty )
    {
        case SCALE_PROP_MAX:
        {
            aScaleData.Maximum = rOuterValue;
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_MIN:
        {
            aScaleData.Minimum = rOuterValue;
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_STEPMAIN:
        {
            aScaleData.IncrementData.Distance = rOuterValue;
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_STEPHELP:
        {
            Sequence< chart2::SubIncrement >& rSubIncrements( aScaleData.IncrementData.SubIncrements );
            if( rSubIncrements.getLength() == 0 )
                rSubIncrements.realloc( 1 );

            double fStepMain = 0, fStepHelp = 0;
            if( (rOuterValue >>= fStepHelp) )
            {
                if( AxisHelper::isLogarithmic(aScaleData.Scaling) )
                {
                    sal_Int32 nIntervalCount = static_cast< sal_Int32 >(fStepHelp);
                    rSubIncrements[ 0 ].IntervalCount <<= nIntervalCount;
                }
                else if( (fStepHelp != 0.0) &&
                    (aScaleData.IncrementData.Distance >>= fStepMain) )
                {
                    // approximate interval count
                    sal_Int32 nIntervalCount = static_cast< sal_Int32 >(fStepMain / fStepHelp);
                    rSubIncrements[ 0 ].IntervalCount <<= nIntervalCount;
                }
            }
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AUTO_MAX:
        {
            if( (rOuterValue >>= bBool) && bBool )
                aScaleData.Maximum = Any();
            else
                aScaleData.Maximum = getPropertyValue( SCALE_PROP_MAX, xInnerPropertySet );
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AUTO_MIN:
        {
            if( (rOuterValue >>= bBool) && bBool )
                aScaleData.Minimum = Any();
            else
                aScaleData.Minimum = getPropertyValue( SCALE_PROP_MIN, xInnerPropertySet );
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AUTO_STEPMAIN:
        {
            if( (rOuterValue >>= bBool) && bBool )
                aScaleData.IncrementData.Distance = Any();
            else
                aScaleData.IncrementData.Distance = getPropertyValue( SCALE_PROP_STEPMAIN, xInnerPropertySet );
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AUTO_STEPHELP:
        {
            Sequence< chart2::SubIncrement >& rSubIncrements( aScaleData.IncrementData.SubIncrements );
            if( rSubIncrements.getLength() == 0 )
                rSubIncrements.realloc( 1 );

            if( (rOuterValue >>= bBool) && bBool )
                rSubIncrements[ 0 ].IntervalCount = Any();
            else
                rSubIncrements[ 0 ].IntervalCount = getPropertyValue( SCALE_PROP_STEPHELP, xInnerPropertySet );
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_ORIGIN:
        {
            aScaleData.Origin = rOuterValue;
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_AUTO_ORIGIN:
        {
            if( (rOuterValue >>= bBool) && bBool )
                aScaleData.Origin = Any();
            else
                aScaleData.Origin = getPropertyValue( SCALE_PROP_ORIGIN, xInnerPropertySet );
            bSetScaleData = true;
            break;
        }
        case SCALE_PROP_LOGARITHMIC:
        {
            if( rOuterValue >>= bBool )
            {
                bool bWasLogarithm = AxisHelper::isLogarithmic( aScaleData.Scaling );

                // safe comparison between sal_Bool and bool
                if( (!bBool) != (!bWasLogarithm) )
                {
                    if( bBool )
                        aScaleData.Scaling = new LogarithmicScaling( 10.0 );
                    else
                        aScaleData.Scaling = 0;
                    bSetScaleData = true;
                }
            }
            break;
        }
        case SCALE_PROP_REVERSEDIRECTION:
        {
            if( rOuterValue >>= bBool )
            {
                bool bWasReverse = ( AxisOrientation_REVERSE == aScaleData.Orientation );
                if( (!bBool) != (!bWasReverse) ) // safe comparison between sal_Bool and bool
                {
                    aScaleData.Orientation = bBool ? AxisOrientation_REVERSE : AxisOrientation_MATHEMATICAL;
                    bSetScaleData = true;
                }
            }
            break;
        }
        default:
        {
            OSL_ENSURE(false,"unknown scale property");
            break;
        }
    }

    if( bSetScaleData )
        xAxis->setScaleData( aScaleData );
}

Any WrappedScaleProperty::getPropertyValue( tScaleProperty eScaleProperty, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet( m_aOuterValue );

    Reference< chart2::XAxis > xAxis( xInnerPropertySet, uno::UNO_QUERY );
    OSL_ENSURE(xAxis.is(),"need an XAxis");
    if(!xAxis.is())
        return aRet;

    chart2::ScaleData aScaleData( xAxis->getScaleData() );

    chart2::ExplicitScaleData aExplicitScale;
    chart2::ExplicitIncrementData aExplicitIncrement;

    switch( eScaleProperty )
    {
        case SCALE_PROP_MAX:
        {
            aRet = aScaleData.Maximum;
            if( !aRet.hasValue() )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis(
                    xAxis, aExplicitScale, aExplicitIncrement );
                aRet <<= aExplicitScale.Maximum;
            }
            break;
        }
        case SCALE_PROP_MIN:
        {
            aRet = aScaleData.Minimum;
            if( !aRet.hasValue() )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis(
                    xAxis, aExplicitScale, aExplicitIncrement );
                aRet <<= aExplicitScale.Minimum;
            }
            break;
        }

        case SCALE_PROP_STEPMAIN:
        {
            aRet = aScaleData.IncrementData.Distance;
            if( !aRet.hasValue() )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis(
                    xAxis, aExplicitScale, aExplicitIncrement );
                aRet <<= aExplicitIncrement.Distance;
            }
            break;
        }
        case SCALE_PROP_STEPHELP:
        {
            // todo: evaluate PostEquidistant
            bool bNeedToCalculateExplicitValues = true;

            bool bLogarithmic( AxisHelper::isLogarithmic(aScaleData.Scaling) );
            Sequence< chart2::SubIncrement >& rSubIncrements( aScaleData.IncrementData.SubIncrements );
            if( bLogarithmic )
            {
                if( rSubIncrements.getLength() > 0 )
                {
                    sal_Int32 nIntervalCount = 0;
                    rSubIncrements[ 0 ].IntervalCount >>= nIntervalCount;
                    aRet = uno::makeAny( double(nIntervalCount) );
                    bNeedToCalculateExplicitValues = false;
                }
            }
            else if( aScaleData.IncrementData.Distance.hasValue() )
            {
                if( rSubIncrements.getLength() > 0 )
                {
                    double fStepMain = 0;
                    sal_Int32 nIntervalCount = 0;
                    if( (aScaleData.IncrementData.Distance >>= fStepMain) &&
                        (rSubIncrements[ 0 ].IntervalCount >>= nIntervalCount) &&
                        nIntervalCount > 0 )
                    {
                        aRet <<= ( fStepMain / static_cast< double >( nIntervalCount ) );
                        bNeedToCalculateExplicitValues = false;
                    }
                }
                else
                {
                    aRet = aScaleData.IncrementData.Distance;
                    bNeedToCalculateExplicitValues = false;
                }
            }

            if( bNeedToCalculateExplicitValues )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis(
                    xAxis, aExplicitScale, aExplicitIncrement );
                if( aExplicitIncrement.SubIncrements.getLength() > 0 &&
                     aExplicitIncrement.SubIncrements[ 0 ].IntervalCount > 0 )
                {
                    if( bLogarithmic )
                    {
                        if( rSubIncrements.getLength() > 0 )
                        {
                            sal_Int32 nIntervalCount = aExplicitIncrement.SubIncrements[ 0 ].IntervalCount;
                            aRet = uno::makeAny( double(nIntervalCount) );
                        }
                    }
                    else
                        aRet <<= ( aExplicitIncrement.Distance /
                                static_cast< double >(
                                    aExplicitIncrement.SubIncrements[ 0 ].IntervalCount ));
                }
                else
                {
                    if( bLogarithmic )
                        aRet <<= 5.0;
                    else
                        aRet <<= aExplicitIncrement.Distance;
                }
            }
            break;
        }
        case SCALE_PROP_AUTO_MAX:
        {
            aRet <<= (sal_Bool)( !aScaleData.Maximum.hasValue() );
            break;
        }
        case SCALE_PROP_AUTO_MIN:
        {
            aRet <<= (sal_Bool)( !aScaleData.Minimum.hasValue() );
            break;
        }
        case SCALE_PROP_AUTO_STEPMAIN:
        {
            aRet <<= (sal_Bool)( !aScaleData.IncrementData.Distance.hasValue() );
            break;
        }
        case SCALE_PROP_AUTO_STEPHELP:
        {
            Sequence< chart2::SubIncrement >& rSubIncrements( aScaleData.IncrementData.SubIncrements );
            if( rSubIncrements.getLength() > 0 )
                aRet <<= (sal_Bool)( !rSubIncrements[ 0 ].IntervalCount.hasValue() );
            else
                aRet <<= sal_True;
            break;
        }
        case SCALE_PROP_ORIGIN:
        {
            aRet = aScaleData.Origin;
            if( !aRet.hasValue() )
            {
                m_spChart2ModelContact->getExplicitValuesForAxis(
                    xAxis, aExplicitScale, aExplicitIncrement );
                aRet <<= aExplicitScale.Origin;
            }
            break;
        }
        case SCALE_PROP_AUTO_ORIGIN:
        {
            aRet <<= !hasDoubleValue(aScaleData.Origin);
            break;
        }
        case SCALE_PROP_LOGARITHMIC:
        {
            aRet <<= static_cast< sal_Bool >( AxisHelper::isLogarithmic(aScaleData.Scaling) );
            break;
        }
        case SCALE_PROP_REVERSEDIRECTION:
        {
            aRet <<= static_cast< sal_Bool >( AxisOrientation_REVERSE == aScaleData.Orientation );
            break;
        }
        default:
        {
            OSL_ENSURE(false,"unknown scale property");
            break;
        }
    }

    return aRet;
}

} //  namespace wrapper
} //namespace chart
//.............................................................................
