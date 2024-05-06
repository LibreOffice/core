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

#include <RegressionCurveHelper.hxx>
#include <RegressionCurveModel.hxx>
#include <RegressionCurveItemConverter.hxx>
#include "SchWhichPairs.hxx"
#include <GraphicPropertyItemConverter.hxx>
#include <DataSeries.hxx>

#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <osl/diagnose.h>

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <utility>

using namespace ::com::sun::star;

namespace
{
template <class T, class D>
bool lclConvertToPropertySet(const SfxItemSet& rItemSet, sal_uInt16 nWhichId, const uno::Reference<beans::XPropertySet>& xProperties, const OUString& aPropertyID)
{
    OSL_ASSERT(xProperties.is());
    if( xProperties.is() )
    {
        T aValue = static_cast<T>(static_cast<const D&>(rItemSet.Get( nWhichId )).GetValue());
        T aOldValue = aValue;
        bool aSuccess = xProperties->getPropertyValue( aPropertyID ) >>= aOldValue;
        if (!aSuccess || aOldValue != aValue)
        {
            xProperties->setPropertyValue( aPropertyID , uno::Any( aValue ));
            return true;
        }
    }
    return false;
}

template <class T, class D>
void lclConvertToItemSet(SfxItemSet& rItemSet, sal_uInt16 nWhichId, const uno::Reference<beans::XPropertySet>& xProperties, const OUString& aPropertyID)
{
    OSL_ASSERT(xProperties.is());
    if( xProperties.is() )
    {
        T aValue = static_cast<T>(static_cast<const D&>(rItemSet.Get( nWhichId )).GetValue());
        if(xProperties->getPropertyValue( aPropertyID ) >>= aValue)
        {
            rItemSet.Put(D( nWhichId, aValue ));
        }
    }
}

void lclConvertToItemSetDouble(SfxItemSet& rItemSet, TypedWhichId<SvxDoubleItem> nWhichId, const uno::Reference<beans::XPropertySet>& xProperties, const OUString& aPropertyID)
{
    OSL_ASSERT(xProperties.is());
    if( xProperties.is() )
    {
        double aValue = rItemSet.Get( nWhichId ).GetValue();
        if(xProperties->getPropertyValue( aPropertyID ) >>= aValue)
        {
            rItemSet.Put(SvxDoubleItem( aValue, nWhichId ));
        }
    }
}

} // anonymous namespace

namespace chart::wrapper
{

RegressionCurveItemConverter::RegressionCurveItemConverter(
    const uno::Reference< beans::XPropertySet >& rPropertySet,
    rtl::Reference< DataSeries > xContainer,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_spGraphicConverter( std::make_shared<GraphicPropertyItemConverter>(
                                  rPropertySet, rItemPool, rDrawModel,
                                  xNamedPropertyContainerFactory,
                                  GraphicObjectType::LineProperties )),
        m_xCurveContainer(std::move( xContainer ))
{}

RegressionCurveItemConverter::~RegressionCurveItemConverter()
{}

void RegressionCurveItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    m_spGraphicConverter->FillItemSet( rOutItemSet );

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool RegressionCurveItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = m_spGraphicConverter->ApplyItemSet( rItemSet );

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const WhichRangesContainer& RegressionCurveItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nRegressionCurveWhichPairs;
}

bool RegressionCurveItemConverter::GetItemProperty(
    tWhichIdType /* nWhichId */, tPropertyNameWithMemberId & /* rOutProperty */ ) const
{
    // No own (non-special) properties
    return false;
}

bool RegressionCurveItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
{
    uno::Reference< chart2::XRegressionCurve > xCurve( GetPropertySet(), uno::UNO_QUERY );
    bool bChanged = false;

    OSL_ASSERT(xCurve.is());
    if(!xCurve.is())
        return false;

    switch( nWhichId )
    {
        case SCHATTR_REGRESSION_TYPE:
        {
            SvxChartRegress eRegress = RegressionCurveHelper::getRegressionType(xCurve);
            SvxChartRegress eNewRegress = static_cast< const SvxChartRegressItem & >(
                rItemSet.Get( nWhichId )).GetValue();
            if( eRegress != eNewRegress )
            {
                // note that changing the regression type changes the object
                // for which this converter was created. Not optimal, but
                // currently the only way to handle the type in the
                // regression curve properties dialog
                xCurve = RegressionCurveHelper::changeRegressionCurveType(
                            eNewRegress,
                            m_xCurveContainer,
                            xCurve);
                uno::Reference<beans::XPropertySet> xProperties( xCurve, uno::UNO_QUERY );
                resetPropertySet( xProperties );
                bChanged = true;
            }
        }
        break;

        case SCHATTR_REGRESSION_DEGREE:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<sal_Int32, SfxInt32Item>(rItemSet, nWhichId, xProperties, u"PolynomialDegree"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_PERIOD:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<sal_Int32, SfxInt32Item>(rItemSet, nWhichId, xProperties, u"MovingAveragePeriod"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<double, SvxDoubleItem>(rItemSet, nWhichId, xProperties, u"ExtrapolateForward"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<double, SvxDoubleItem>(rItemSet, nWhichId, xProperties, u"ExtrapolateBackward"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_SET_INTERCEPT:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<bool, SfxBoolItem>(rItemSet, nWhichId, xProperties, u"ForceIntercept"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_INTERCEPT_VALUE:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<double, SvxDoubleItem>(rItemSet, nWhichId, xProperties, u"InterceptValue"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_CURVE_NAME:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<OUString, SfxStringItem>(rItemSet, nWhichId, xProperties, u"CurveName"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_MOVING_TYPE:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<sal_Int32, SfxInt32Item>(rItemSet, nWhichId, xProperties, u"MovingAverageType"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_SHOW_EQUATION:
        {
            uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
            bChanged = lclConvertToPropertySet<bool, SfxBoolItem>(rItemSet, nWhichId, xEqProp, u"ShowEquation"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_XNAME:
        {
            uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
            bChanged = lclConvertToPropertySet<OUString, SfxStringItem>(rItemSet, nWhichId, xEqProp, u"XName"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_YNAME:
        {
            uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
            bChanged = lclConvertToPropertySet<OUString, SfxStringItem>(rItemSet, nWhichId, xEqProp, u"YName"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
            bChanged = lclConvertToPropertySet<bool, SfxBoolItem>(rItemSet, nWhichId, xEqProp, u"ShowCorrelationCoefficient"_ustr);
        }
        break;

    }
    return bChanged;
}

void RegressionCurveItemConverter::FillSpecialItem(sal_uInt16 nWhichId, SfxItemSet& rOutItemSet ) const
{
    uno::Reference<chart2::XRegressionCurve> xCurve(GetPropertySet(), uno::UNO_QUERY);
    OSL_ASSERT(xCurve.is());
    if(!xCurve.is())
        return;

    uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );

    switch( nWhichId )
    {
        case SCHATTR_REGRESSION_TYPE:
        {
            SvxChartRegress eRegress = RegressionCurveHelper::getRegressionType(xCurve);
            rOutItemSet.Put( SvxChartRegressItem( eRegress, SCHATTR_REGRESSION_TYPE ));
        }
        break;

        case SCHATTR_REGRESSION_DEGREE:
        {
            lclConvertToItemSet<sal_Int32, SfxInt32Item>(rOutItemSet, nWhichId, xProperties, u"PolynomialDegree"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_PERIOD:
        {
            lclConvertToItemSet<sal_Int32, SfxInt32Item>(rOutItemSet, nWhichId, xProperties, u"MovingAveragePeriod"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD:
        {
            lclConvertToItemSetDouble(rOutItemSet, SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD, xProperties, u"ExtrapolateForward"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD:
        {
            lclConvertToItemSetDouble(rOutItemSet, SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD, xProperties, u"ExtrapolateBackward"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_SET_INTERCEPT:
        {
            lclConvertToItemSet<bool, SfxBoolItem>(rOutItemSet, nWhichId, xProperties, u"ForceIntercept"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_INTERCEPT_VALUE:
        {
            lclConvertToItemSetDouble(rOutItemSet, SCHATTR_REGRESSION_INTERCEPT_VALUE, xProperties, u"InterceptValue"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_CURVE_NAME:
        {
            lclConvertToItemSet<OUString, SfxStringItem>(rOutItemSet, nWhichId, xProperties, u"CurveName"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_MOVING_TYPE:
        {
            lclConvertToItemSet<sal_Int32, SfxInt32Item>(rOutItemSet, nWhichId, xProperties, u"MovingAverageType"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_SHOW_EQUATION:
        {
            lclConvertToItemSet<bool, SfxBoolItem>(rOutItemSet, nWhichId, xCurve->getEquationProperties(), u"ShowEquation"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_XNAME:
        {
            lclConvertToItemSet<OUString, SfxStringItem>(rOutItemSet, nWhichId, xCurve->getEquationProperties(), u"XName"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_YNAME:
        {
            lclConvertToItemSet<OUString, SfxStringItem>(rOutItemSet, nWhichId, xCurve->getEquationProperties(), u"YName"_ustr);
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            lclConvertToItemSet<bool, SfxBoolItem>(rOutItemSet, nWhichId, xCurve->getEquationProperties(), u"ShowCorrelationCoefficient"_ustr);
        }
        break;
    }
}

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
