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

#include <res_ErrorBar.hxx>
#include <bitmaps.hlst>
#include <RangeSelectionHelper.hxx>
#include <helpids.h>
#include <chartview/ChartSfxItemIds.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <vcl/dialog.hxx>

#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <svl/stritem.hxx>

#define CHART_LB_FUNCTION_STD_ERROR     0
#define CHART_LB_FUNCTION_STD_DEV       1
#define CHART_LB_FUNCTION_VARIANCE      2
#define CHART_LB_FUNCTION_ERROR_MARGIN  3

using namespace ::com::sun::star;

namespace
{
void lcl_enableRangeChoosing(bool bEnable, TabPageParent pParent)
{
    if (weld::DialogController* pController = pParent.pController)
    {
        weld::Window* pWeldDialog = pController->getDialog();
        pWeldDialog->set_modal(!bEnable);
        pWeldDialog->set_visible(!bEnable);
    }
    else if (::Dialog* pVCLDialog = pParent.pParent ? pParent.pParent->GetParentDialog() : nullptr)
    {
        pVCLDialog->Show(!bEnable);
        pVCLDialog->SetModalInputMode(!bEnable);
    }
}

sal_uInt16 lcl_getLbEntryPosByErrorKind( SvxChartKindError eErrorKind )
{
    sal_uInt16 nResult = 0;
    switch( eErrorKind )
    {
        // for these cases select the default in the list box
        case SvxChartKindError::NONE:
        case SvxChartKindError::Percent:
        case SvxChartKindError::Const:
        case SvxChartKindError::Range:
            nResult = CHART_LB_FUNCTION_STD_DEV;
            break;
        case SvxChartKindError::Variant:
            nResult = CHART_LB_FUNCTION_VARIANCE;
            break;
        case SvxChartKindError::Sigma:
            nResult = CHART_LB_FUNCTION_STD_DEV;
            break;
        case SvxChartKindError::BigError:
            nResult = CHART_LB_FUNCTION_ERROR_MARGIN;
            break;
        case SvxChartKindError::StdError:
            nResult = CHART_LB_FUNCTION_STD_ERROR;
            break;
    }
    return nResult;
}
} // anonymous namespace

namespace chart
{

ErrorBarResources::ErrorBarResources(weld::Builder* pParent, TabPageParent pParentDialog,
                                     const SfxItemSet& rInAttrs, bool bNoneAvailable,
                                     tErrorBarType eType /* = ERROR_BAR_Y */ )
    : m_eErrorKind( SvxChartKindError::NONE )
    , m_eIndicate( SvxChartIndicate::Both )
    , m_bErrorKindUnique( true )
    , m_bIndicatorUnique( true )
    , m_bRangePosUnique( true )
    , m_bRangeNegUnique( true )
    , m_eErrorBarType( eType )
    , m_nConstDecimalDigits( 1 )
    , m_nConstSpinSize( 1 )
    , m_fPlusValue(0.0)
    , m_fMinusValue(0.0)
    , m_pParentDialog( pParentDialog )
    , m_pCurrentRangeChoosingField( nullptr )
    , m_bHasInternalDataProvider( true )
    , m_bEnableDataTableDialog( true )
    , m_xRbNone(pParent->weld_radio_button("RB_NONE"))
    , m_xRbConst(pParent->weld_radio_button("RB_CONST"))
    , m_xRbPercent(pParent->weld_radio_button("RB_PERCENT"))
    , m_xRbFunction(pParent->weld_radio_button("RB_FUNCTION"))
    , m_xRbRange(pParent->weld_radio_button("RB_RANGE"))
    , m_xLbFunction(pParent->weld_combo_box("LB_FUNCTION"))
    , m_xFlParameters(pParent->weld_frame("framePARAMETERS"))
    , m_xBxPositive(pParent->weld_widget("boxPOSITIVE"))
    , m_xMfPositive(pParent->weld_metric_spin_button("MF_POSITIVE", FieldUnit::NONE))
    , m_xEdRangePositive(pParent->weld_entry("ED_RANGE_POSITIVE"))
    , m_xIbRangePositive(pParent->weld_button("IB_RANGE_POSITIVE"))
    , m_xBxNegative(pParent->weld_widget("boxNEGATIVE"))
    , m_xMfNegative(pParent->weld_metric_spin_button("MF_NEGATIVE", FieldUnit::NONE))
    , m_xEdRangeNegative(pParent->weld_entry("ED_RANGE_NEGATIVE"))
    , m_xIbRangeNegative(pParent->weld_button("IB_RANGE_NEGATIVE"))
    , m_xCbSyncPosNeg(pParent->weld_check_button("CB_SYN_POS_NEG"))
    , m_xRbBoth(pParent->weld_radio_button("RB_BOTH"))
    , m_xRbPositive(pParent->weld_radio_button("RB_POSITIVE"))
    , m_xRbNegative(pParent->weld_radio_button("RB_NEGATIVE"))
    , m_xFiBoth(pParent->weld_image("FI_BOTH"))
    , m_xFiPositive(pParent->weld_image("FI_POSITIVE"))
    , m_xFiNegative(pParent->weld_image("FI_NEGATIVE"))
    , m_xUIStringPos(pParent->weld_label("STR_DATA_SELECT_RANGE_FOR_POSITIVE_ERRORBARS"))
    , m_xUIStringNeg(pParent->weld_label("STR_DATA_SELECT_RANGE_FOR_NEGATIVE_ERRORBARS"))
    , m_xUIStringRbRange(pParent->weld_label("STR_CONTROLTEXT_ERROR_BARS_FROM_DATA"))
{
    if( bNoneAvailable )
        m_xRbNone->connect_toggled(LINK(this, ErrorBarResources, CategoryChosen));
    else
        m_xRbNone->hide();

    m_xRbConst->connect_toggled( LINK( this, ErrorBarResources, CategoryChosen ));
    m_xRbPercent->connect_toggled( LINK( this, ErrorBarResources, CategoryChosen ));
    m_xRbFunction->connect_toggled( LINK( this, ErrorBarResources, CategoryChosen ));
    m_xRbRange->connect_toggled( LINK( this, ErrorBarResources, CategoryChosen ));
    m_xLbFunction->connect_changed( LINK( this, ErrorBarResources, CategoryChosen2 ));

    m_xCbSyncPosNeg->set_active( false );
    m_xCbSyncPosNeg->connect_toggled( LINK( this, ErrorBarResources, SynchronizePosAndNeg ));

    m_xMfPositive->connect_value_changed( LINK( this, ErrorBarResources, PosValueChanged ));
    m_xEdRangePositive->connect_changed( LINK( this, ErrorBarResources, RangeChanged ));
    m_xEdRangeNegative->connect_changed( LINK( this, ErrorBarResources, RangeChanged ));

    m_xRbPositive->connect_toggled( LINK( this, ErrorBarResources, IndicatorChanged ));
    m_xRbNegative->connect_toggled( LINK( this, ErrorBarResources, IndicatorChanged ));
    m_xRbBoth->connect_toggled( LINK( this, ErrorBarResources, IndicatorChanged ));

    m_xIbRangePositive->connect_clicked( LINK( this, ErrorBarResources, ChooseRange ));
    m_xIbRangeNegative->connect_clicked( LINK( this, ErrorBarResources, ChooseRange ));

    FillValueSets();
    Reset( rInAttrs );
}

ErrorBarResources::~ErrorBarResources()
{
}

void ErrorBarResources::SetErrorBarType( tErrorBarType eNewType )
{
    if( m_eErrorBarType != eNewType )
    {
        m_eErrorBarType = eNewType;
        FillValueSets();
    }
}

void ErrorBarResources::SetChartDocumentForRangeChoosing(
    const uno::Reference< chart2::XChartDocument > & xChartDocument )
{
    if( xChartDocument.is())
    {
        m_bHasInternalDataProvider = xChartDocument->hasInternalDataProvider();
        uno::Reference< beans::XPropertySet > xProps( xChartDocument, uno::UNO_QUERY );
        if ( xProps.is() )
        {
            try
            {
                xProps->getPropertyValue("EnableDataTableDialog") >>= m_bEnableDataTableDialog;
            }
            catch( const uno::Exception& e )
            {
                SAL_WARN("chart2", "Exception caught. " << e );
            }
        }
    }
    m_apRangeSelectionHelper.reset( new RangeSelectionHelper( xChartDocument ));

    // has internal data provider => rename "cell range" to "from data"
    OSL_ASSERT(m_apRangeSelectionHelper);
    if( m_bHasInternalDataProvider )
    {
        m_xRbRange->set_label(m_xUIStringRbRange->get_label());
        m_xRbRange->set_help_id(HID_SCH_ERROR_BARS_FROM_DATA);
    }

    if( m_xRbRange->get_active())
    {
        isRangeFieldContentValid( *m_xEdRangePositive );
        isRangeFieldContentValid( *m_xEdRangeNegative );
    }
}

void ErrorBarResources::SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth )
{
    if( fMinorStepWidth < 0 )
        fMinorStepWidth = -fMinorStepWidth;

    sal_Int32 nExponent = static_cast< sal_Int32 >( ::rtl::math::approxFloor( log10( fMinorStepWidth )));
    if( nExponent <= 0 )
    {
        // one digit precision more
        m_nConstDecimalDigits = static_cast< sal_uInt16 >( (-nExponent) + 1 );
        m_nConstSpinSize = 10;
    }
    else
    {
        m_nConstDecimalDigits = 0;
        m_nConstSpinSize = static_cast< sal_Int64 >( pow( 10.0, static_cast<int>(nExponent) ));
    }
}

void ErrorBarResources::UpdateControlStates()
{
    // function
    bool bIsFunction = m_xRbFunction->get_active();
    m_xLbFunction->set_sensitive( bIsFunction );

    // range buttons
    m_xRbRange->set_sensitive( !m_bHasInternalDataProvider || m_bEnableDataTableDialog );
    bool bShowRange = m_xRbRange->get_active();
    bool bCanChooseRange =
        ( bShowRange &&
          m_apRangeSelectionHelper.get() &&
          m_apRangeSelectionHelper->hasRangeSelection());

    m_xMfPositive->set_visible( ! bShowRange );
    m_xMfNegative->set_visible( ! bShowRange );

    // use range but without range chooser => hide controls
    m_xEdRangePositive->set_visible( bShowRange && ! m_bHasInternalDataProvider );
    m_xIbRangePositive->set_visible( bCanChooseRange );
    m_xEdRangeNegative->set_visible( bShowRange && ! m_bHasInternalDataProvider );
    m_xIbRangeNegative->set_visible( bCanChooseRange );

    bool bShowPosNegAndSync = ! (bShowRange && m_bHasInternalDataProvider);
    m_xFlParameters->set_visible( bShowPosNegAndSync );

    // unit for metric fields
    bool bIsErrorMargin(
        ( m_xRbFunction->get_active()) &&
        ( m_xLbFunction->get_active() == CHART_LB_FUNCTION_ERROR_MARGIN ));
    bool bIsPercentage( m_xRbPercent->get_active() || bIsErrorMargin );
    FieldUnit eFieldUnit = FieldUnit::NONE;

    if( bIsPercentage )
    {
        eFieldUnit = FieldUnit::PERCENT;
        m_xMfPositive->set_digits( 1 );
        m_xMfPositive->set_increments(10, 100, FieldUnit::NONE);
        m_xMfNegative->set_digits( 1 );
        m_xMfNegative->set_increments(10, 100, FieldUnit::NONE);
    }
    else
    {
        m_xMfPositive->set_digits( m_nConstDecimalDigits );
        m_xMfPositive->set_increments(m_nConstSpinSize, m_nConstSpinSize * 10, FieldUnit::NONE);
        m_xMfNegative->set_digits( m_nConstDecimalDigits );
        m_xMfNegative->set_increments(m_nConstSpinSize, m_nConstSpinSize * 10, FieldUnit::NONE);
    }

    sal_Int32 nPlusValue = static_cast< sal_Int32 >( m_fPlusValue * pow(10.0,m_xMfPositive->get_digits()) );
    sal_Int32 nMinusValue = static_cast< sal_Int32 >( m_fMinusValue * pow(10.0,m_xMfNegative->get_digits()) );

    m_xMfPositive->set_value(nPlusValue, FieldUnit::NONE);
    m_xMfNegative->set_value(nMinusValue, FieldUnit::NONE);

    m_xMfPositive->set_unit(eFieldUnit);
    m_xMfNegative->set_unit(eFieldUnit);

    // positive and negative value fields
    bool bPosEnabled = ( m_xRbPositive->get_active() || m_xRbBoth->get_active());
    bool bNegEnabled = ( m_xRbNegative->get_active() || m_xRbBoth->get_active());
    if( !( bPosEnabled || bNegEnabled ))
    {
        // all three controls are not checked -> ambiguous state
        bPosEnabled = true;
        bNegEnabled = true;
    }

    // functions with only one parameter
    bool bOneParameterCategory =
        bIsErrorMargin || m_xRbPercent->get_active();
    if( bOneParameterCategory )
    {
        m_xCbSyncPosNeg->set_active(true);
    }

    if( m_xCbSyncPosNeg->get_active())
    {
        bPosEnabled = true;
        bNegEnabled = false;
    }

    // all functions except error margin take no arguments
    if( m_xRbFunction->get_active() &&  ( m_xLbFunction->get_active() != CHART_LB_FUNCTION_ERROR_MARGIN ))
    {
        bPosEnabled = false;
        bNegEnabled = false;
    }

    // enable/disable pos/neg fields
    m_xBxPositive->set_sensitive( bPosEnabled );
    m_xBxNegative->set_sensitive( bNegEnabled );
    if( bShowRange )
    {
        m_xEdRangePositive->set_sensitive( bPosEnabled );
        m_xIbRangePositive->set_sensitive( bPosEnabled );
        m_xEdRangeNegative->set_sensitive( bNegEnabled );
        m_xIbRangeNegative->set_sensitive( bNegEnabled );
    }
    else
    {
        m_xMfPositive->set_sensitive( bPosEnabled );
        m_xMfNegative->set_sensitive( bNegEnabled );
    }

    m_xCbSyncPosNeg->set_sensitive( !bOneParameterCategory &&  ( bPosEnabled || bNegEnabled ));

    // mark invalid entries in the range fields
    if( bShowRange && ! m_bHasInternalDataProvider )
    {
        isRangeFieldContentValid( *m_xEdRangePositive );
        isRangeFieldContentValid( *m_xEdRangeNegative );
    }
}

IMPL_LINK_NOARG( ErrorBarResources, CategoryChosen2, weld::ComboBox&, void )
{
   CategoryChosen(*m_xRbConst);
}

IMPL_LINK_NOARG( ErrorBarResources, CategoryChosen, weld::ToggleButton&, void )
{
    m_bErrorKindUnique = true;
    SvxChartKindError eOldError = m_eErrorKind;

    if( m_xRbNone->get_active())
        m_eErrorKind = SvxChartKindError::NONE;
    else if( m_xRbConst->get_active())
        m_eErrorKind = SvxChartKindError::Const;
    else if( m_xRbPercent->get_active())
        m_eErrorKind = SvxChartKindError::Percent;
    else if( m_xRbRange->get_active())
        m_eErrorKind = SvxChartKindError::Range;
    else if( m_xRbFunction->get_active())
    {
        switch( m_xLbFunction->get_active())
        {
            case CHART_LB_FUNCTION_STD_ERROR:
                m_eErrorKind = SvxChartKindError::StdError; break;
            case CHART_LB_FUNCTION_STD_DEV:
                m_eErrorKind = SvxChartKindError::Sigma; break;
            case CHART_LB_FUNCTION_VARIANCE:
                m_eErrorKind = SvxChartKindError::Variant; break;
            case CHART_LB_FUNCTION_ERROR_MARGIN:
                m_eErrorKind = SvxChartKindError::BigError; break;
            default:
                m_bErrorKindUnique = false;
        }
    }
    else
    {
        OSL_FAIL( "Unknown category chosen" );
        m_bErrorKindUnique = false;
    }

    // changed to range
    if( m_eErrorKind == SvxChartKindError::Range &&
        eOldError != SvxChartKindError::Range )
    {
        m_xCbSyncPosNeg->set_active(
            (!m_xEdRangePositive->get_text().isEmpty()) &&
            m_xEdRangePositive->get_text() == m_xEdRangeNegative->get_text());
    }
    // changed from range
    else if( m_eErrorKind != SvxChartKindError::Range &&
        eOldError == SvxChartKindError::Range )
    {
        m_xCbSyncPosNeg->set_active( m_xMfPositive->get_value(FieldUnit::NONE) == m_xMfNegative->get_value(FieldUnit::NONE));
    }

    UpdateControlStates();
}

IMPL_LINK_NOARG(ErrorBarResources, SynchronizePosAndNeg, weld::ToggleButton&, void)
{
    UpdateControlStates();
    PosValueChanged( *m_xMfPositive );
}

IMPL_LINK_NOARG(ErrorBarResources, PosValueChanged, weld::MetricSpinButton&, void)
{
    if( m_xCbSyncPosNeg->get_active())
    {
        if( m_xRbRange->get_active())
        {
            m_xEdRangeNegative->set_text( m_xEdRangePositive->get_text());
            m_bRangeNegUnique = m_bRangePosUnique;
        }
        else
            m_xMfNegative->set_value(m_xMfPositive->get_value(FieldUnit::NONE), FieldUnit::NONE);
    }
}

IMPL_LINK_NOARG(ErrorBarResources, IndicatorChanged, weld::ToggleButton&, void)
{
    m_bIndicatorUnique = true;
    if( m_xRbBoth->get_active())
        m_eIndicate = SvxChartIndicate::Both;
    else if( m_xRbPositive->get_active())
        m_eIndicate = SvxChartIndicate::Up;
    else if( m_xRbNegative->get_active())
        m_eIndicate = SvxChartIndicate::Down;
    else
        m_bIndicatorUnique = false;

    UpdateControlStates();
}

IMPL_LINK(ErrorBarResources, ChooseRange, weld::Button&, rButton, void)
{
    OSL_ASSERT(m_apRangeSelectionHelper);
    if (!m_apRangeSelectionHelper)
        return;
    OSL_ASSERT( m_pCurrentRangeChoosingField == nullptr );

    OUString aUIString;

    if (&rButton == m_xIbRangePositive.get())
    {
        m_pCurrentRangeChoosingField = m_xEdRangePositive.get();
        aUIString = m_xUIStringPos->get_label();
    }
    else
    {
        m_pCurrentRangeChoosingField = m_xEdRangeNegative.get();
        aUIString = m_xUIStringNeg->get_label();
    }

    lcl_enableRangeChoosing( true, m_pParentDialog );
    m_apRangeSelectionHelper->chooseRange(
        m_pCurrentRangeChoosingField->get_text(),
        aUIString, *this );
}

IMPL_LINK( ErrorBarResources, RangeChanged, weld::Entry&, rEdit, void )
{
    if( &rEdit == m_xEdRangePositive.get() )
    {
        m_bRangePosUnique = true;
        PosValueChanged( *m_xMfPositive );
    }
    else
    {
        m_bRangeNegUnique = true;
    }

    isRangeFieldContentValid( rEdit );
}

void ErrorBarResources::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = nullptr;

    // category
    m_eErrorKind = SvxChartKindError::NONE;
    SfxItemState aState = rInAttrs.GetItemState( SCHATTR_STAT_KIND_ERROR, true, &pPoolItem );
    m_bErrorKindUnique = ( aState != SfxItemState::DONTCARE );

    if( aState == SfxItemState::SET )
        m_eErrorKind = static_cast<const SvxChartKindErrorItem*>(pPoolItem)->GetValue();

    m_xLbFunction->set_active( lcl_getLbEntryPosByErrorKind( m_eErrorKind ));

    if( m_bErrorKindUnique )
    {
        switch( m_eErrorKind )
        {
            case SvxChartKindError::NONE:
                m_xRbNone->set_active(true);
                break;
            case SvxChartKindError::Percent:
                m_xRbPercent->set_active(true);
                break;
            case SvxChartKindError::Const:
                m_xRbConst->set_active(true);
                break;
            case SvxChartKindError::StdError:
            case SvxChartKindError::Variant:
            case SvxChartKindError::Sigma:
            case SvxChartKindError::BigError:
                m_xRbFunction->set_active(true);
                break;
            case SvxChartKindError::Range:
                m_xRbRange->set_active(true);
                break;
        }
    }
    else
    {
        m_xRbNone->set_active( false );
        m_xRbConst->set_active( false );
        m_xRbPercent->set_active( false );
        m_xRbFunction->set_active( false );
    }

    // parameters
    aState = rInAttrs.GetItemState( SCHATTR_STAT_CONSTPLUS, true, &pPoolItem );
    if( aState == SfxItemState::SET )
    {
        m_fPlusValue = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
    }

    aState = rInAttrs.GetItemState( SCHATTR_STAT_CONSTMINUS, true, &pPoolItem );
    if( aState == SfxItemState::SET )
    {
        m_fMinusValue = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();

        if( m_eErrorKind != SvxChartKindError::Range &&
            m_fPlusValue == m_fMinusValue )
            m_xCbSyncPosNeg->set_active(true);
    }

    // indicator
    aState = rInAttrs.GetItemState( SCHATTR_STAT_INDICATE, true, &pPoolItem );
    m_bIndicatorUnique = ( aState != SfxItemState::DONTCARE );
    if( aState == SfxItemState::SET)
        m_eIndicate = static_cast<const SvxChartIndicateItem *>(pPoolItem)->GetValue();

    if( m_bIndicatorUnique )
    {
        switch( m_eIndicate )
        {
            case SvxChartIndicate::NONE :
                // no longer used, use both as default
                m_eIndicate = SvxChartIndicate::Both;
                [[fallthrough]]; // to BOTH
            case SvxChartIndicate::Both :
                m_xRbBoth->set_active(true); break;
            case SvxChartIndicate::Up :
                m_xRbPositive->set_active(true); break;
            case SvxChartIndicate::Down :
                m_xRbNegative->set_active(true); break;
        }
    }
    else
    {
        m_xRbBoth->set_active( false );
        m_xRbPositive->set_active( false );
        m_xRbNegative->set_active( false );
    }

    // ranges
    aState = rInAttrs.GetItemState( SCHATTR_STAT_RANGE_POS, true, &pPoolItem );
    m_bRangePosUnique = ( aState != SfxItemState::DONTCARE );
    if( aState == SfxItemState::SET )
    {
        OUString sRangePositive = static_cast< const SfxStringItem * >( pPoolItem )->GetValue();
        m_xEdRangePositive->set_text( sRangePositive );
    }

    aState = rInAttrs.GetItemState( SCHATTR_STAT_RANGE_NEG, true, &pPoolItem );
    m_bRangeNegUnique = ( aState != SfxItemState::DONTCARE );
    if( aState == SfxItemState::SET )
    {
        OUString sRangeNegative = static_cast< const SfxStringItem * >( pPoolItem )->GetValue();
        m_xEdRangeNegative->set_text( sRangeNegative );
        if( m_eErrorKind == SvxChartKindError::Range &&
            !sRangeNegative.isEmpty() &&
            sRangeNegative == m_xEdRangePositive->get_text() )
            m_xCbSyncPosNeg->set_active(true);
    }

    UpdateControlStates();
}

void ErrorBarResources::FillItemSet(SfxItemSet& rOutAttrs) const
{
    if( m_bErrorKindUnique )
        rOutAttrs.Put( SvxChartKindErrorItem( m_eErrorKind, SCHATTR_STAT_KIND_ERROR ));
    if( m_bIndicatorUnique )
        rOutAttrs.Put( SvxChartIndicateItem( m_eIndicate, SCHATTR_STAT_INDICATE ));

    if( m_bErrorKindUnique )
    {
        if( m_eErrorKind == SvxChartKindError::Range )
        {
            OUString aPosRange;
            OUString aNegRange;
            if( m_bHasInternalDataProvider )
            {
                // the strings aPosRange/aNegRange have to be set to a non-empty
                // arbitrary string to generate error-bar sequences
                aPosRange = "x";
                aNegRange = aPosRange;
            }
            else
            {
                aPosRange = m_xEdRangePositive->get_text();
                if( m_xCbSyncPosNeg->get_active())
                    aNegRange = aPosRange;
                else
                    aNegRange = m_xEdRangeNegative->get_text();
            }

            if( m_bRangePosUnique )
                rOutAttrs.Put( SfxStringItem( SCHATTR_STAT_RANGE_POS, aPosRange ));
            if( m_bRangeNegUnique )
                rOutAttrs.Put( SfxStringItem( SCHATTR_STAT_RANGE_NEG, aNegRange ));
        }
        else if( m_eErrorKind == SvxChartKindError::Const ||
                 m_eErrorKind == SvxChartKindError::Percent ||
                 m_eErrorKind == SvxChartKindError::BigError )
        {
            double fPosValue = static_cast< double >( m_xMfPositive->get_value(FieldUnit::NONE)) /
                pow( 10.0, m_xMfPositive->get_digits());
            double fNegValue = 0.0;

            if( m_xCbSyncPosNeg->get_active())
                fNegValue = fPosValue;
            else
                fNegValue = static_cast< double >( m_xMfNegative->get_value(FieldUnit::NONE)) /
                    pow( 10.0, m_xMfNegative->get_digits());

            rOutAttrs.Put( SvxDoubleItem( fPosValue, SCHATTR_STAT_CONSTPLUS ));
            rOutAttrs.Put( SvxDoubleItem( fNegValue, SCHATTR_STAT_CONSTMINUS ));
        }
    }

    rOutAttrs.Put( SfxBoolItem( SCHATTR_STAT_ERRORBAR_TYPE , m_eErrorBarType == ERROR_BAR_Y ));
}

void ErrorBarResources::FillValueSets()
{
    if( m_eErrorBarType == ERROR_BAR_Y )
    {
        m_xFiNegative->set_from_icon_name(BMP_INDICATE_DOWN);
        m_xFiPositive->set_from_icon_name(BMP_INDICATE_UP);
        m_xFiBoth->set_from_icon_name(BMP_INDICATE_BOTH_VERTI);
    }
    else if( m_eErrorBarType == ERROR_BAR_X )
    {
        m_xFiNegative->set_from_icon_name(BMP_INDICATE_LEFT);
        m_xFiPositive->set_from_icon_name(BMP_INDICATE_RIGHT);
        m_xFiBoth->set_from_icon_name(BMP_INDICATE_BOTH_HORI);
    }
}

void ErrorBarResources::listeningFinished(
    const OUString & rNewRange )
{
    OSL_ASSERT(m_apRangeSelectionHelper);
    if (!m_apRangeSelectionHelper)
        return;

    // rNewRange becomes invalid after removing the listener
    OUString aRange( rNewRange );

    // stop listening
    m_apRangeSelectionHelper->stopRangeListening();

    // change edit field
//     if( m_pParentWindow )
//     {
//         m_pParentWindow->ToTop();
//         m_pParentWindow->grab_focus();
//     }

    if( m_pCurrentRangeChoosingField )
    {
        m_pCurrentRangeChoosingField->set_text( aRange );
        m_pCurrentRangeChoosingField->grab_focus();
        PosValueChanged( *m_xMfPositive );
    }

    m_pCurrentRangeChoosingField = nullptr;

    UpdateControlStates();
    lcl_enableRangeChoosing( false, m_pParentDialog );
}

void ErrorBarResources::disposingRangeSelection()
{
    OSL_ASSERT(m_apRangeSelectionHelper);
    if (m_apRangeSelectionHelper)
        m_apRangeSelectionHelper->stopRangeListening( false );
}

void ErrorBarResources::isRangeFieldContentValid(weld::Entry& rEdit)
{
    OUString aRange( rEdit.get_text());
    bool bIsValid = ( aRange.isEmpty() ) ||
        ( m_apRangeSelectionHelper.get() &&
          m_apRangeSelectionHelper->verifyCellRange( aRange ));

    if( bIsValid || !rEdit.get_sensitive())
    {
        rEdit.set_error(false);
    }
    else
    {
        rEdit.set_error(true);
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
