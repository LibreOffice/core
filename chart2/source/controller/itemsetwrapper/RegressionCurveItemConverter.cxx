/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegressionCurveItemConverter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:56:00 $
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
#include "RegressionCurveHelper.hxx"
#include "RegressionCurveItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "GraphicPropertyItemConverter.hxx"

#include <com/sun/star/chart2/XRegressionCurve.hpp>

// for SfxBoolItem
#include <svtools/eitem.hxx>
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

const USHORT * RegressionCurveItemConverter::GetWhichPairs() const
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
    USHORT nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    uno::Reference< chart2::XRegressionCurve > xCurve( GetPropertySet(), uno::UNO_QUERY );
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_STAT_REGRESSTYPE:
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
                bool bNewShow = static_cast< BOOL >(
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
                bool bNewShow = static_cast< BOOL >(
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
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    uno::Reference< chart2::XRegressionCurve > xCurve( GetPropertySet(), uno::UNO_QUERY );

    switch( nWhichId )
    {
        case SCHATTR_STAT_REGRESSTYPE:
        {
            OSL_ASSERT( xCurve.is());
            if( xCurve.is())
            {
                SvxChartRegress eRegress = static_cast< SvxChartRegress >(
                    static_cast< sal_Int32 >( RegressionCurveHelper::getRegressionType( xCurve )));
                rOutItemSet.Put( SvxChartRegressItem( eRegress, SCHATTR_STAT_REGRESSTYPE ));
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
