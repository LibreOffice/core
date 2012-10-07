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

// for SfxBoolItem
#include <svl/eitem.hxx>
#include <svx/chrtitem.hxx>

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;

namespace
{

::chart::RegressionCurveHelper::tRegressionType lcl_convertRegressionType( SvxChartRegress eRegress )
{
    ::chart::RegressionCurveHelper::tRegressionType eType = ::chart::RegressionCurveHelper::REGRESSION_TYPE_NONE;
    switch( eRegress )
    {
        case CHREGRESS_LINEAR:
            eType = ::chart::RegressionCurveHelper::REGRESSION_TYPE_LINEAR;
            break;
        case CHREGRESS_LOG:
            eType = ::chart::RegressionCurveHelper::REGRESSION_TYPE_LOG;
            break;
        case CHREGRESS_EXP:
            eType = ::chart::RegressionCurveHelper::REGRESSION_TYPE_EXP;
            break;
        case CHREGRESS_POWER:
            eType = ::chart::RegressionCurveHelper::REGRESSION_TYPE_POWER;
            break;
        case CHREGRESS_NONE:
            break;
    }
    return eType;
}

} // anonymous namespace

namespace chart
{
namespace wrapper
{

RegressionCurveItemConverter::RegressionCurveItemConverter(
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    const uno::Reference< chart2::XRegressionCurveContainer > & xRegCurveCnt,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_spGraphicConverter( new GraphicPropertyItemConverter(
                                  rPropertySet, rItemPool, rDrawModel,
                                  xNamedPropertyContainerFactory,
                                  GraphicPropertyItemConverter::LINE_PROPERTIES )),
        m_xCurveContainer( xRegCurveCnt )
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
    throw( uno::Exception )
{
    uno::Reference< chart2::XRegressionCurve > xCurve( GetPropertySet(), uno::UNO_QUERY );
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_REGRESSION_TYPE:
        {
            OSL_ASSERT( xCurve.is());
            if( xCurve.is())
            {
                SvxChartRegress eRegress = static_cast< SvxChartRegress >(
                    static_cast< sal_Int32 >( RegressionCurveHelper::getRegressionType( xCurve )));
                SvxChartRegress eNewRegress = static_cast< const SvxChartRegressItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
                if( eRegress != eNewRegress )
                {
                    // note that changing the regression type changes the object
                    // for which this converter was created. Not optimal, but
                    // currently the only way to handle the type in the
                    // regression curve properties dialog
                    RegressionCurveHelper::replaceOrAddCurveAndReduceToOne(
                        lcl_convertRegressionType( eNewRegress ), m_xCurveContainer,
                        uno::Reference< uno::XComponentContext >());
                    uno::Reference< beans::XPropertySet > xNewPropSet(
                        RegressionCurveHelper::getFirstCurveNotMeanValueLine( m_xCurveContainer ),
                        uno::UNO_QUERY );
                    OSL_ASSERT( xNewPropSet.is());
                    if( xNewPropSet.is())
                    {
                        resetPropertySet( xNewPropSet );
                        bChanged = true;
                    }
                }
            }
        }
        break;

        case SCHATTR_REGRESSION_SHOW_EQUATION:
        {
            OSL_ASSERT( xCurve.is());
            if( xCurve.is())
            {
                bool bNewShow = static_cast< sal_Bool >(
                    static_cast< const SfxBoolItem & >(
                        rItemSet.Get( nWhichId )).GetValue());

                uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
                OSL_ASSERT( xEqProp.is());
                bool bOldShow = false;
                if( xEqProp.is() &&
                    (xEqProp->getPropertyValue( "ShowEquation" ) >>= bOldShow) &&
                    bOldShow != bNewShow )
                {
                    xEqProp->setPropertyValue( "ShowEquation" , uno::makeAny( bNewShow ));
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            OSL_ASSERT( xCurve.is());
            if( xCurve.is())
            {
                bool bNewShow = static_cast< sal_Bool >(
                    static_cast< const SfxBoolItem & >(
                        rItemSet.Get( nWhichId )).GetValue());

                uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
                OSL_ASSERT( xEqProp.is());
                bool bOldShow = false;
                if( xEqProp.is() &&
                    (xEqProp->getPropertyValue( "ShowCorrelationCoefficient" ) >>= bOldShow) &&
                    bOldShow != bNewShow )
                {
                    xEqProp->setPropertyValue( "ShowCorrelationCoefficient" , uno::makeAny( bNewShow ));
                    bChanged = true;
                }
            }
        }
        break;
    }

    return bChanged;
}

void RegressionCurveItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    uno::Reference< chart2::XRegressionCurve > xCurve( GetPropertySet(), uno::UNO_QUERY );

    switch( nWhichId )
    {
        case SCHATTR_REGRESSION_TYPE:
        {
            OSL_ASSERT( xCurve.is());
            if( xCurve.is())
            {
                SvxChartRegress eRegress = static_cast< SvxChartRegress >(
                    static_cast< sal_Int32 >( RegressionCurveHelper::getRegressionType( xCurve )));
                rOutItemSet.Put( SvxChartRegressItem( eRegress, SCHATTR_REGRESSION_TYPE ));
            }
        }
        break;

        case SCHATTR_REGRESSION_SHOW_EQUATION:
        {
            OSL_ASSERT( xCurve.is());
            if( xCurve.is())
            {
                uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
                OSL_ASSERT( xEqProp.is());
                bool bShow = false;
                if( xEqProp.is() &&
                    (xEqProp->getPropertyValue( "ShowEquation" ) >>= bShow))
                {
                    rOutItemSet.Put( SfxBoolItem( nWhichId, bShow ));
                }
            }
        }
        break;

        case SCHATTR_REGRESSION_SHOW_COEFF:
        {
            OSL_ASSERT( xCurve.is());
            if( xCurve.is())
            {
                uno::Reference< beans::XPropertySet > xEqProp( xCurve->getEquationProperties());
                OSL_ASSERT( xEqProp.is());
                bool bShow = false;
                if( xEqProp.is() &&
                    (xEqProp->getPropertyValue( "ShowCorrelationCoefficient" ) >>= bShow))
                {
                    rOutItemSet.Put( SfxBoolItem( nWhichId, bShow ));
                }
            }
        }
        break;
    }
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
