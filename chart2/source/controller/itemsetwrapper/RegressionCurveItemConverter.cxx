/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
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
                    (xEqProp->getPropertyValue( C2U( "ShowEquation" )) >>= bOldShow) &&
                    bOldShow != bNewShow )
                {
                    xEqProp->setPropertyValue( C2U( "ShowEquation" ), uno::makeAny( bNewShow ));
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
                    (xEqProp->getPropertyValue( C2U( "ShowCorrelationCoefficient" )) >>= bOldShow) &&
                    bOldShow != bNewShow )
                {
                    xEqProp->setPropertyValue( C2U( "ShowCorrelationCoefficient" ), uno::makeAny( bNewShow ));
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
                    (xEqProp->getPropertyValue( C2U( "ShowEquation" )) >>= bShow))
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
                    (xEqProp->getPropertyValue( C2U( "ShowCorrelationCoefficient" )) >>= bShow))
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
