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

#include "RegressionCurveHelper.hxx"
#include "RegressionCurveItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "GraphicPropertyItemConverter.hxx"

#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <osl/diagnose.h>

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>

#include <functional>
#include <algorithm>

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

void lclConvertToItemSetDouble(SfxItemSet& rItemSet, sal_uInt16 nWhichId, const uno::Reference<beans::XPropertySet>& xProperties, const OUString& aPropertyID)
{
    OSL_ASSERT(xProperties.is());
    if( xProperties.is() )
    {
        double aValue = static_cast<double>(static_cast<const SvxDoubleItem&>(rItemSet.Get( nWhichId )).GetValue());
        if(xProperties->getPropertyValue( aPropertyID ) >>= aValue)
        {
            rItemSet.Put(SvxDoubleItem( aValue, nWhichId ));
        }
    }
}

} // anonymous namespace

namespace chart
{
namespace wrapper
{

RegressionCurveItemConverter::RegressionCurveItemConverter(
    const uno::Reference< beans::XPropertySet >& rPropertySet,
    const uno::Reference< chart2::XRegressionCurveContainer >& xContainer,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_spGraphicConverter( new GraphicPropertyItemConverter(
                                  rPropertySet, rItemPool, rDrawModel,
                                  xNamedPropertyContainerFactory,
                                  GraphicPropertyItemConverter::LINE_PROPERTIES )),
        m_xCurveContainer( xContainer )
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

const sal_uInt16 * RegressionCurveItemConverter::GetWhichPairs() const
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
                            xCurve,
                            uno::Reference< uno::XComponentContext >());
                uno::Reference<beans::XPropertySet> xProperties( xCurve, uno::UNO_QUERY );
                resetPropertySet( xProperties );
                bChanged = true;
            }
        }
        break;

        case SCHATTR_REGRESSION_DEGREE:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<sal_Int32, SfxInt32Item>(rItemSet, nWhichId, xProperties, "PolynomialDegree");
        }
        break;

        case SCHATTR_REGRESSION_PERIOD:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<sal_Int32, SfxInt32Item>(rItemSet, nWhichId, xProperties, "MovingAveragePeriod");
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<double, SvxDoubleItem>(rItemSet, nWhichId, xProperties, "ExtrapolateForward");
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<double, SvxDoubleItem>(rItemSet, nWhichId, xProperties, "ExtrapolateBackward");
        }
        break;

        case SCHATTR_REGRESSION_SET_INTERCEPT:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<bool, SfxBoolItem>(rItemSet, nWhichId, xProperties, "ForceIntercept");
        }
        break;

        case SCHATTR_REGRESSION_INTERCEPT_VALUE:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<double, SvxDoubleItem>(rItemSet, nWhichId, xProperties, "InterceptValue");
        }
        break;

        case SCHATTR_REGRESSION_CURVE_NAME:
        {
            uno::Reference< beans::XPropertySet > xProperties( xCurve, uno::UNO_QUERY );
            bChanged = lclConvertToPropertySet<OUString, SfxStringItem>(rItemSet, nWhichId, xProperties, "CurveName");
        }
        break;

        case SCHATTR_REGRESSION_SHOW_EQUATION:
        {
            uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
            bChanged = lclConvertToPropertySet<bool, SfxBoolItem>(rItemSet, nWhichId, xEqProp, "ShowEquation");
        }
        break;

        case SCHATTR_REGRESSION_XNAME:
        {
            uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
            bChanged = lclConvertToPropertySet<OUString, SfxStringItem>(rItemSet, nWhichId, xEqProp, "XName");
        }
        break;

        case SCHATTR_REGRESSION_YNAME:
        {
            uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
            bChanged = lclConvertToPropertySet<OUString, SfxStringItem>(rItemSet, nWhichId, xEqProp, "YName");
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
            bChanged = lclConvertToPropertySet<bool, SfxBoolItem>(rItemSet, nWhichId, xEqProp, "ShowCorrelationCoefficient");
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
            lclConvertToItemSet<sal_Int32, SfxInt32Item>(rOutItemSet, nWhichId, xProperties, "PolynomialDegree");
        }
        break;

        case SCHATTR_REGRESSION_PERIOD:
        {
            lclConvertToItemSet<sal_Int32, SfxInt32Item>(rOutItemSet, nWhichId, xProperties, "MovingAveragePeriod");
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD:
        {
            lclConvertToItemSetDouble(rOutItemSet, nWhichId, xProperties, "ExtrapolateForward");
        }
        break;

        case SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD:
        {
            lclConvertToItemSetDouble(rOutItemSet, nWhichId, xProperties, "ExtrapolateBackward");
        }
        break;

        case SCHATTR_REGRESSION_SET_INTERCEPT:
        {
            lclConvertToItemSet<bool, SfxBoolItem>(rOutItemSet, nWhichId, xProperties, "ForceIntercept");
        }
        break;

        case SCHATTR_REGRESSION_INTERCEPT_VALUE:
        {
            lclConvertToItemSetDouble(rOutItemSet, nWhichId, xProperties, "InterceptValue");
        }
        break;

        case SCHATTR_REGRESSION_CURVE_NAME:
        {
            lclConvertToItemSet<OUString, SfxStringItem>(rOutItemSet, nWhichId, xProperties, "CurveName");
        }
        break;

        case SCHATTR_REGRESSION_SHOW_EQUATION:
        {
            lclConvertToItemSet<bool, SfxBoolItem>(rOutItemSet, nWhichId, xCurve->getEquationProperties(), "ShowEquation");
        }
        break;

        case SCHATTR_REGRESSION_XNAME:
        {
            lclConvertToItemSet<OUString, SfxStringItem>(rOutItemSet, nWhichId, xCurve->getEquationProperties(), "XName");
        }
        break;

        case SCHATTR_REGRESSION_YNAME:
        {
            lclConvertToItemSet<OUString, SfxStringItem>(rOutItemSet, nWhichId, xCurve->getEquationProperties(), "YName");
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            lclConvertToItemSet<bool, SfxBoolItem>(rOutItemSet, nWhichId, xCurve->getEquationProperties(), "ShowCorrelationCoefficient");
        }
        break;
    }
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
