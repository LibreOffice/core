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

#include <ErrorBarItemConverter.hxx>
#include "SchWhichPairs.hxx"
#include <StatisticsHelper.hxx>

#include <GraphicPropertyItemConverter.hxx>

#include <svl/stritem.hxx>
#include <svx/chrtitem.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;

namespace
{

void lcl_getErrorValues( const uno::Reference< beans::XPropertySet > & xErrorBarProp,
                    double & rOutPosError, double & rOutNegError )
{
    if( ! xErrorBarProp.is())
        return;

    try
    {
        xErrorBarProp->getPropertyValue( u"PositiveError"_ustr ) >>= rOutPosError;
        xErrorBarProp->getPropertyValue( u"NegativeError"_ustr ) >>= rOutNegError;
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void lcl_getErrorIndicatorValues(
    const uno::Reference< beans::XPropertySet > & xErrorBarProp,
    bool & rOutShowPosError, bool & rOutShowNegError )
{
    if( ! xErrorBarProp.is())
        return;

    try
    {
        xErrorBarProp->getPropertyValue( u"ShowPositiveError"_ustr ) >>= rOutShowPosError;
        xErrorBarProp->getPropertyValue( u"ShowNegativeError"_ustr ) >>= rOutShowNegError;
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

} // anonymous namespace

namespace chart::wrapper
{

ErrorBarItemConverter::ErrorBarItemConverter(
    uno::Reference< frame::XModel > xModel,
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_spGraphicConverter( std::make_shared<GraphicPropertyItemConverter>(
                                  rPropertySet, rItemPool, rDrawModel,
                                  xNamedPropertyContainerFactory,
                                  GraphicObjectType::LineProperties )),
        m_xModel(std::move( xModel ))
{}

ErrorBarItemConverter::~ErrorBarItemConverter()
{}

void ErrorBarItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    m_spGraphicConverter->FillItemSet( rOutItemSet );

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool ErrorBarItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = m_spGraphicConverter->ApplyItemSet( rItemSet );

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const WhichRangesContainer& ErrorBarItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nErrorBarWhichPairs;
}

bool ErrorBarItemConverter::GetItemProperty(
    tWhichIdType /* nWhichId */,
    tPropertyNameWithMemberId & /* rOutProperty */ ) const
{
    return false;
}

bool ErrorBarItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
{
    bool bChanged = false;

    switch( nWhichId )
    {
        // Attention !!! This case must be passed before SCHATTR_STAT_PERCENT,
        // SCHATTR_STAT_BIGERROR, SCHATTR_STAT_CONSTPLUS,
        // SCHATTR_STAT_CONSTMINUS and SCHATTR_STAT_INDICATE
        case SCHATTR_STAT_KIND_ERROR:
        {
            uno::Reference< beans::XPropertySet > xErrorBarProp( GetPropertySet());

            SvxChartKindError eErrorKind =
                static_cast< const SvxChartKindErrorItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            if( !xErrorBarProp.is() && eErrorKind == SvxChartKindError::NONE)
            {
                //nothing to do
            }
            else
            {
                sal_Int32 nStyle = css::chart::ErrorBarStyle::NONE;

                switch( eErrorKind )
                {
                    case SvxChartKindError::NONE:
                        nStyle = css::chart::ErrorBarStyle::NONE; break;
                    case SvxChartKindError::Variant:
                        nStyle = css::chart::ErrorBarStyle::VARIANCE; break;
                    case SvxChartKindError::Sigma:
                        nStyle = css::chart::ErrorBarStyle::STANDARD_DEVIATION; break;
                    case SvxChartKindError::Percent:
                        nStyle = css::chart::ErrorBarStyle::RELATIVE; break;
                    case SvxChartKindError::BigError:
                        nStyle = css::chart::ErrorBarStyle::ERROR_MARGIN; break;
                    case SvxChartKindError::Const:
                        nStyle = css::chart::ErrorBarStyle::ABSOLUTE; break;
                    case SvxChartKindError::StdError:
                        nStyle = css::chart::ErrorBarStyle::STANDARD_ERROR; break;
                    case SvxChartKindError::Range:
                        nStyle = css::chart::ErrorBarStyle::FROM_DATA; break;
                }

                xErrorBarProp->setPropertyValue( u"ErrorBarStyle"_ustr , uno::Any( nStyle ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_PERCENT:
        case SCHATTR_STAT_BIGERROR:
        {
            OSL_FAIL( "Deprecated item" );
            uno::Reference< beans::XPropertySet > xErrorBarProp( GetPropertySet());

            double fValue =
                static_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos(0.0), fNeg(0.0);
            lcl_getErrorValues( xErrorBarProp, fPos, fNeg );

            if( ! ( ::rtl::math::approxEqual( fPos, fValue ) &&
                    ::rtl::math::approxEqual( fNeg, fValue )))
            {
                xErrorBarProp->setPropertyValue( u"PositiveError"_ustr , uno::Any( fValue ));
                xErrorBarProp->setPropertyValue( u"NegativeError"_ustr , uno::Any( fValue ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_CONSTPLUS:
        {
            double fValue =
                static_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos(0.0), fNeg(0.0);
            lcl_getErrorValues( GetPropertySet(), fPos, fNeg );

            if( ! ::rtl::math::approxEqual( fPos, fValue ))
            {
                GetPropertySet()->setPropertyValue( u"PositiveError"_ustr , uno::Any( fValue ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_CONSTMINUS:
        {
            uno::Reference< beans::XPropertySet > xErrorBarProp( GetPropertySet());

            double fValue =
                static_cast< const SvxDoubleItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            double fPos(0.0), fNeg(0.0);
            lcl_getErrorValues( xErrorBarProp, fPos, fNeg );

            if( ! ::rtl::math::approxEqual( fNeg, fValue ))
            {
                xErrorBarProp->setPropertyValue( u"NegativeError"_ustr , uno::Any( fValue ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_INDICATE:
        {
            uno::Reference< beans::XPropertySet > xErrorBarProp( GetPropertySet());

            SvxChartIndicate eIndicate =
                static_cast< const SvxChartIndicateItem & >(
                    rItemSet.Get( nWhichId )).GetValue();

            bool bNewIndPos = (eIndicate == SvxChartIndicate::Both || eIndicate == SvxChartIndicate::Up );
            bool bNewIndNeg = (eIndicate == SvxChartIndicate::Both || eIndicate == SvxChartIndicate::Down );

            bool bShowPos(false), bShowNeg(false);
            lcl_getErrorIndicatorValues( xErrorBarProp, bShowPos, bShowNeg );

            if( bShowPos != bNewIndPos ||
                bShowNeg != bNewIndNeg )
            {
                xErrorBarProp->setPropertyValue( u"ShowPositiveError"_ustr , uno::Any( bNewIndPos ));
                xErrorBarProp->setPropertyValue( u"ShowNegativeError"_ustr , uno::Any( bNewIndNeg ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_STAT_RANGE_POS:
        case SCHATTR_STAT_RANGE_NEG:
        {
            // @todo: also be able to deal with x-error bars
            const bool bYError =
                rItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE).GetValue();

            uno::Reference< chart2::data::XDataSource > xErrorBarSource( GetPropertySet(), uno::UNO_QUERY );
            uno::Reference< chart2::XChartDocument > xChartDoc( m_xModel, uno::UNO_QUERY );
            uno::Reference< chart2::data::XDataProvider > xDataProvider;

            if( xChartDoc.is())
                xDataProvider.set( xChartDoc->getDataProvider());
            if( xErrorBarSource.is() && xDataProvider.is())
            {
                OUString aNewRange( static_cast< const SfxStringItem & >( rItemSet.Get( nWhichId )).GetValue());
                bool bApplyNewRange = false;

                bool bIsPositiveValue( nWhichId == SCHATTR_STAT_RANGE_POS );
                if( xChartDoc->hasInternalDataProvider())
                {
                    if( !aNewRange.isEmpty())
                    {
                        uno::Reference< chart2::data::XDataSequence > xSeq(
                            StatisticsHelper::getErrorDataSequenceFromDataSource(
                                xErrorBarSource, bIsPositiveValue, bYError ));
                        if( ! xSeq.is())
                        {
                            // no data range for error bars yet => create
                            uno::Reference< chart2::XInternalDataProvider > xIntDataProvider( xDataProvider, uno::UNO_QUERY );
                            OSL_ASSERT( xIntDataProvider.is());
                            if( xIntDataProvider.is())
                            {
                                xIntDataProvider->appendSequence();
                                aNewRange = "last";
                                bApplyNewRange = true;
                            }
                        }
                    }
                }
                else
                {
                    uno::Reference< chart2::data::XDataSequence > xSeq(
                        StatisticsHelper::getErrorDataSequenceFromDataSource(
                            xErrorBarSource, bIsPositiveValue, bYError ));
                    bApplyNewRange =
                        ! ( xSeq.is() && (aNewRange == xSeq->getSourceRangeRepresentation()));
                }

                if( bApplyNewRange )
                    StatisticsHelper::setErrorDataSequence(
                        xErrorBarSource, xDataProvider, aNewRange, bIsPositiveValue, bYError );
            }
        }
        break;
    }

    return bChanged;
}

void ErrorBarItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
{
    switch( nWhichId )
    {
        case SCHATTR_STAT_KIND_ERROR:
        {
            SvxChartKindError eErrorKind = SvxChartKindError::NONE;
            uno::Reference< beans::XPropertySet > xErrorBarProp( GetPropertySet());

            sal_Int32 nStyle = 0;
            if( xErrorBarProp->getPropertyValue( u"ErrorBarStyle"_ustr ) >>= nStyle )
            {
                switch( nStyle )
                {
                    case css::chart::ErrorBarStyle::NONE:
                        break;
                    case css::chart::ErrorBarStyle::VARIANCE:
                        eErrorKind = SvxChartKindError::Variant; break;
                    case css::chart::ErrorBarStyle::STANDARD_DEVIATION:
                        eErrorKind = SvxChartKindError::Sigma; break;
                    case css::chart::ErrorBarStyle::ABSOLUTE:
                        eErrorKind = SvxChartKindError::Const; break;
                    case css::chart::ErrorBarStyle::RELATIVE:
                        eErrorKind = SvxChartKindError::Percent; break;
                    case css::chart::ErrorBarStyle::ERROR_MARGIN:
                        eErrorKind = SvxChartKindError::BigError; break;
                    case css::chart::ErrorBarStyle::STANDARD_ERROR:
                        eErrorKind = SvxChartKindError::StdError; break;
                    case css::chart::ErrorBarStyle::FROM_DATA:
                        eErrorKind = SvxChartKindError::Range; break;
                }
            }
            rOutItemSet.Put( SvxChartKindErrorItem( eErrorKind, SCHATTR_STAT_KIND_ERROR ));
        }
        break;

        case SCHATTR_STAT_PERCENT:
        {
            double fPos(0.0), fNeg(0.0);
            lcl_getErrorValues( GetPropertySet(), fPos, fNeg );
            rOutItemSet.Put( SvxDoubleItem( ( fPos + fNeg ) / 2.0, SCHATTR_STAT_PERCENT ));
        }
        break;

        case SCHATTR_STAT_BIGERROR:
        {
            double fPos(0.0), fNeg(0.0);
            lcl_getErrorValues( GetPropertySet(), fPos, fNeg );
            rOutItemSet.Put( SvxDoubleItem( ( fPos + fNeg ) / 2.0, SCHATTR_STAT_BIGERROR ));
        }
        break;

        case SCHATTR_STAT_CONSTPLUS:
        {
            double fPos(0.0), fNeg(0.0);
            lcl_getErrorValues( GetPropertySet(), fPos, fNeg );
            rOutItemSet.Put( SvxDoubleItem( fPos, SCHATTR_STAT_CONSTPLUS ));
        }
        break;

        case SCHATTR_STAT_CONSTMINUS:
        {
            double fPos(0.0), fNeg(0.0);
            lcl_getErrorValues( GetPropertySet(), fPos, fNeg );
            rOutItemSet.Put( SvxDoubleItem( fNeg, SCHATTR_STAT_CONSTMINUS ));
        }
        break;

        case SCHATTR_STAT_INDICATE:
        {
            SvxChartIndicate eIndicate = SvxChartIndicate::Both;
            bool bShowPos(false), bShowNeg(false);
            lcl_getErrorIndicatorValues( GetPropertySet(), bShowPos, bShowNeg );

            if( bShowPos )
            {
                if( bShowNeg )
                    eIndicate = SvxChartIndicate::Both;
                else
                    eIndicate = SvxChartIndicate::Up;
            }
            else
            {
                if( bShowNeg )
                    eIndicate = SvxChartIndicate::Down;
                else
                    eIndicate = SvxChartIndicate::NONE;
            }
            rOutItemSet.Put( SvxChartIndicateItem( eIndicate, SCHATTR_STAT_INDICATE ));
        }
        break;

        case SCHATTR_STAT_RANGE_POS:
        case SCHATTR_STAT_RANGE_NEG:
        {
            const bool bYError =
                rOutItemSet.Get(SCHATTR_STAT_ERRORBAR_TYPE).GetValue();

            uno::Reference< chart2::data::XDataSource > xErrorBarSource( GetPropertySet(), uno::UNO_QUERY );
            if( xErrorBarSource.is())
            {
                uno::Reference< chart2::data::XDataSequence > xSeq(
                    StatisticsHelper::getErrorDataSequenceFromDataSource(
                        xErrorBarSource, (nWhichId == SCHATTR_STAT_RANGE_POS), bYError ));
                if( xSeq.is())
                    rOutItemSet.Put( SfxStringItem( nWhichId, xSeq->getSourceRangeRepresentation()));
            }
        }
        break;
   }
}

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
