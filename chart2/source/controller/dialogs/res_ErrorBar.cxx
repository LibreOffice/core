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

#include "res_ErrorBar.hxx"
#include "ResId.hxx"
#include "Bitmaps.hrc"
#include "RangeSelectionHelper.hxx"
#include "TabPageNotifiable.hxx"
#include "macros.hxx"

#include <rtl/math.hxx>
#include <vcl/dialog.hxx>
#include <svl/stritem.hxx>

#define CHART_LB_FUNCTION_STD_ERROR     0
#define CHART_LB_FUNCTION_STD_DEV       1
#define CHART_LB_FUNCTION_VARIANCE      2
#define CHART_LB_FUNCTION_ERROR_MARGIN  3

using namespace ::com::sun::star;

namespace
{
void lcl_enableRangeChoosing( bool bEnable, Dialog * pDialog )
{
    if( pDialog )
    {
        pDialog->Show( !bEnable );
        pDialog->SetModalInputMode( !bEnable );
    }
}

sal_uInt16 lcl_getLbEntryPosByErrorKind( SvxChartKindError eErrorKind )
{
    sal_uInt16 nResult = 0;
    switch( eErrorKind )
    {
        // for these cases select the default in the list box
        case CHERROR_NONE:
        case CHERROR_PERCENT:
        case CHERROR_CONST:
        case CHERROR_RANGE:
            nResult = CHART_LB_FUNCTION_STD_DEV;
            break;
        case CHERROR_VARIANT:
            nResult = CHART_LB_FUNCTION_VARIANCE;
            break;
        case CHERROR_SIGMA:
            nResult = CHART_LB_FUNCTION_STD_DEV;
            break;
        case CHERROR_BIGERROR:
            nResult = CHART_LB_FUNCTION_ERROR_MARGIN;
            break;
        case CHERROR_STDERROR:
            nResult = CHART_LB_FUNCTION_STD_ERROR;
            break;
    }
    return nResult;
}
} // anonymous namespace

namespace chart
{

ErrorBarResources::ErrorBarResources( VclBuilderContainer* pParent, Dialog * pParentDialog,
                                      const SfxItemSet& rInAttrs, bool bNoneAvailable,
                                      tErrorBarType eType /* = ERROR_BAR_Y */ ) :
        m_eErrorKind( CHERROR_NONE ),
        m_eIndicate( CHINDICATE_BOTH ),
        m_bErrorKindUnique( true ),
        m_bIndicatorUnique( true ),
        m_bPlusUnique( true ),
        m_bMinusUnique( true ),
        m_bRangePosUnique( true ),
        m_bRangeNegUnique( true ),
        m_bNoneAvailable( bNoneAvailable ),
        m_eErrorBarType( eType ),
        m_nConstDecimalDigits( 1 ),
        m_nConstSpinSize( 1 ),
        m_fPlusValue(0.0),
        m_fMinusValue(0.0),
        m_pParentDialog( pParentDialog ),
        m_pCurrentRangeChoosingField( nullptr ),
        m_bHasInternalDataProvider( true ),
        m_bEnableDataTableDialog( true )
{
        pParent->get(m_pRbNone,"RB_NONE");
        pParent->get(m_pRbConst, "RB_CONST");
        pParent->get(m_pRbPercent, "RB_PERCENT");
        pParent->get(m_pRbFunction, "RB_FUNCTION");
        pParent->get(m_pRbRange, "RB_RANGE");
        pParent->get(m_pLbFunction, "LB_FUNCTION");

        pParent->get(m_pFlParameters, "framePARAMETERS");
        pParent->get(m_pBxPositive, "boxPOSITIVE");
        pParent->get(m_pMfPositive, "MF_POSITIVE");
        pParent->get(m_pEdRangePositive, "ED_RANGE_POSITIVE");
        pParent->get(m_pIbRangePositive, "IB_RANGE_POSITIVE");
        pParent->get(m_pBxNegative, "boxNEGATIVE");
        pParent->get(m_pMfNegative, "MF_NEGATIVE");
        pParent->get(m_pEdRangeNegative, "ED_RANGE_NEGATIVE");
        pParent->get(m_pIbRangeNegative, "IB_RANGE_NEGATIVE");
        pParent->get(m_pCbSyncPosNeg, "CB_SYN_POS_NEG");

        pParent->get(m_pRbBoth, "RB_BOTH");
        pParent->get(m_pRbPositive, "RB_POSITIVE");
        pParent->get(m_pRbNegative, "RB_NEGATIVE");
        pParent->get(m_pFiBoth, "FI_BOTH");
        pParent->get(m_pFiPositive, "FI_POSITIVE");
        pParent->get(m_pFiNegative, "FI_NEGATIVE");

        pParent->get(m_pUIStringPos, "STR_DATA_SELECT_RANGE_FOR_POSITIVE_ERRORBARS");
        pParent->get(m_pUIStringNeg, "STR_DATA_SELECT_RANGE_FOR_NEGATIVE_ERRORBARS");
        pParent->get(m_pUIStringRbRange, "STR_CONTROLTEXT_ERROR_BARS_FROM_DATA");

    if( m_bNoneAvailable )
        m_pRbNone->SetClickHdl( LINK( this, ErrorBarResources, CategoryChosen ));
    else
        m_pRbNone->Hide();

    m_pRbConst->SetClickHdl( LINK( this, ErrorBarResources, CategoryChosen ));
    m_pRbPercent->SetClickHdl( LINK( this, ErrorBarResources, CategoryChosen ));
    m_pRbFunction->SetClickHdl( LINK( this, ErrorBarResources, CategoryChosen ));
    m_pRbRange->SetClickHdl( LINK( this, ErrorBarResources, CategoryChosen ));
    m_pLbFunction->SetSelectHdl( LINK( this, ErrorBarResources, CategoryChosen2 ));

    m_pCbSyncPosNeg->Check( false );
    m_pCbSyncPosNeg->SetToggleHdl( LINK( this, ErrorBarResources, SynchronizePosAndNeg ));

    m_pMfPositive->SetModifyHdl( LINK( this, ErrorBarResources, PosValueChanged ));
    m_pEdRangePositive->SetModifyHdl( LINK( this, ErrorBarResources, RangeChanged ));
    m_pEdRangeNegative->SetModifyHdl( LINK( this, ErrorBarResources, RangeChanged ));

    m_pRbPositive->SetClickHdl( LINK( this, ErrorBarResources, IndicatorChanged ));
    m_pRbNegative->SetClickHdl( LINK( this, ErrorBarResources, IndicatorChanged ));
    m_pRbBoth->SetClickHdl( LINK( this, ErrorBarResources, IndicatorChanged ));

    m_pIbRangePositive->SetClickHdl( LINK( this, ErrorBarResources, ChooseRange ));
    m_pIbRangeNegative->SetClickHdl( LINK( this, ErrorBarResources, ChooseRange ));

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
                ASSERT_EXCEPTION( e );
            }
        }
    }
    m_apRangeSelectionHelper.reset( new RangeSelectionHelper( xChartDocument ));

    // has internal data provider => rename "cell range" to "from data"
    OSL_ASSERT( m_apRangeSelectionHelper.get());
    if( m_bHasInternalDataProvider )
    {
        m_pRbRange->SetText(m_pUIStringRbRange->GetText());
    }

    if( m_pRbRange->IsChecked())
    {
        isRangeFieldContentValid( *m_pEdRangePositive );
        isRangeFieldContentValid( *m_pEdRangeNegative );
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
        m_nConstSpinSize = static_cast< sal_Int64 >( pow( 10.0, (int)nExponent ));
    }
}

void ErrorBarResources::UpdateControlStates()
{
    // function
    bool bIsFunction = m_pRbFunction->IsChecked();
    m_pLbFunction->Enable( bIsFunction );

    // range buttons
    m_pRbRange->Enable( !m_bHasInternalDataProvider || m_bEnableDataTableDialog );
    bool bShowRange = ( m_pRbRange->IsChecked());
    bool bCanChooseRange =
        ( bShowRange &&
          m_apRangeSelectionHelper.get() &&
          m_apRangeSelectionHelper->hasRangeSelection());

    m_pMfPositive->Show( ! bShowRange );
    m_pMfNegative->Show( ! bShowRange );

    // use range but without range chooser => hide controls
    m_pEdRangePositive->Show( bShowRange && ! m_bHasInternalDataProvider );
    m_pIbRangePositive->Show( bCanChooseRange );
    m_pEdRangeNegative->Show( bShowRange && ! m_bHasInternalDataProvider );
    m_pIbRangeNegative->Show( bCanChooseRange );

    bool bShowPosNegAndSync = ! (bShowRange && m_bHasInternalDataProvider);
    m_pFlParameters->Show( bShowPosNegAndSync );

    // unit for metric fields
    bool bIsErrorMargin(
        ( m_pRbFunction->IsChecked()) &&
        ( m_pLbFunction->GetSelectEntryPos() == CHART_LB_FUNCTION_ERROR_MARGIN ));
    bool bIsPercentage( m_pRbPercent->IsChecked() || bIsErrorMargin );
    OUString aCustomUnit;

    if( bIsPercentage )
    {
        aCustomUnit = " %";
        m_pMfPositive->SetDecimalDigits( 1 );
        m_pMfPositive->SetSpinSize( 10 );
        m_pMfNegative->SetDecimalDigits( 1 );
        m_pMfNegative->SetSpinSize( 10 );
    }
    else
    {
        m_pMfPositive->SetDecimalDigits( m_nConstDecimalDigits );
        m_pMfPositive->SetSpinSize( m_nConstSpinSize );
        m_pMfNegative->SetDecimalDigits( m_nConstDecimalDigits );
        m_pMfNegative->SetSpinSize( m_nConstSpinSize );
    }

    sal_Int32 nPlusValue = static_cast< sal_Int32 >( m_fPlusValue * pow(10.0,m_pMfPositive->GetDecimalDigits()) );
    sal_Int32 nMinusValue = static_cast< sal_Int32 >( m_fMinusValue * pow(10.0,m_pMfNegative->GetDecimalDigits()) );

    m_pMfPositive->SetValue( nPlusValue );
    m_pMfNegative->SetValue( nMinusValue );

    m_pMfPositive->SetCustomUnitText( aCustomUnit );
    m_pMfNegative->SetCustomUnitText( aCustomUnit );

    // positive and negative value fields
    bool bPosEnabled = ( m_pRbPositive->IsChecked() || m_pRbBoth->IsChecked());
    bool bNegEnabled = ( m_pRbNegative->IsChecked() || m_pRbBoth->IsChecked());
    if( !( bPosEnabled || bNegEnabled ))
    {
        // all three controls are not checked -> ambiguous state
        bPosEnabled = true;
        bNegEnabled = true;
    }

    // functions with only one parameter
    bool bOneParameterCategory =
        bIsErrorMargin || m_pRbPercent->IsChecked();
    if( bOneParameterCategory )
    {
        m_pCbSyncPosNeg->Check();
    }

    if( m_pCbSyncPosNeg->IsChecked())
    {
        bPosEnabled = true;
        bNegEnabled = false;
    }

    // all functions except error margin take no arguments
    if( m_pRbFunction->IsChecked() &&  ( m_pLbFunction->GetSelectEntryPos() != CHART_LB_FUNCTION_ERROR_MARGIN ))
    {
        bPosEnabled = false;
        bNegEnabled = false;
    }

    // enable/disable pos/neg fields
    m_pBxPositive->Enable( bPosEnabled );
    m_pBxNegative->Enable( bNegEnabled );
    if( bShowRange )
    {
        m_pEdRangePositive->Enable( bPosEnabled );
        m_pIbRangePositive->Enable( bPosEnabled );
        m_pEdRangeNegative->Enable( bNegEnabled );
        m_pIbRangeNegative->Enable( bNegEnabled );
    }
    else
    {
        m_pMfPositive->Enable( bPosEnabled );
        m_pMfNegative->Enable( bNegEnabled );
    }

    m_pCbSyncPosNeg->Enable( !bOneParameterCategory &&  ( bPosEnabled || bNegEnabled ));

    // mark invalid entries in the range fields
    if( bShowRange && ! m_bHasInternalDataProvider )
    {
        isRangeFieldContentValid( *m_pEdRangePositive );
        isRangeFieldContentValid( *m_pEdRangeNegative );
    }
}

IMPL_LINK_NOARG_TYPED( ErrorBarResources, CategoryChosen2, ListBox&, void )
{
   CategoryChosen(nullptr);
}

IMPL_LINK_NOARG_TYPED( ErrorBarResources, CategoryChosen, Button*, void )
{
    m_bErrorKindUnique = true;
    SvxChartKindError eOldError = m_eErrorKind;

    if( m_pRbNone->IsChecked())
        m_eErrorKind = CHERROR_NONE;
    else if( m_pRbConst->IsChecked())
        m_eErrorKind = CHERROR_CONST;
    else if( m_pRbPercent->IsChecked())
        m_eErrorKind = CHERROR_PERCENT;
    else if( m_pRbRange->IsChecked())
        m_eErrorKind = CHERROR_RANGE;
    else if( m_pRbFunction->IsChecked())
    {
        if( m_pLbFunction->GetSelectEntryCount() == 1 )
        {
            switch( m_pLbFunction->GetSelectEntryPos())
            {
                case CHART_LB_FUNCTION_STD_ERROR:
                    m_eErrorKind = CHERROR_STDERROR; break;
                case CHART_LB_FUNCTION_STD_DEV:
                    m_eErrorKind = CHERROR_SIGMA; break;
                case CHART_LB_FUNCTION_VARIANCE:
                    m_eErrorKind = CHERROR_VARIANT; break;
                case CHART_LB_FUNCTION_ERROR_MARGIN:
                    m_eErrorKind = CHERROR_BIGERROR; break;
                default:
                    m_bErrorKindUnique = false;
            }
        }
        else
            m_bErrorKindUnique = false;
    }
    else
    {
        OSL_FAIL( "Unknown category chosen" );
        m_bErrorKindUnique = false;
    }

    // changed to range
    if( m_eErrorKind == CHERROR_RANGE &&
        eOldError != CHERROR_RANGE )
    {
        m_pCbSyncPosNeg->Check(
            (!m_pEdRangePositive->GetText().isEmpty()) &&
            m_pEdRangePositive->GetText() == m_pEdRangeNegative->GetText());
    }
    // changed from range
    else if( m_eErrorKind != CHERROR_RANGE &&
        eOldError == CHERROR_RANGE )
    {
        m_pCbSyncPosNeg->Check( m_pMfPositive->GetValue() == m_pMfNegative->GetValue());
    }

    UpdateControlStates();
}

IMPL_LINK_NOARG_TYPED(ErrorBarResources, SynchronizePosAndNeg, CheckBox&, void)
{
    UpdateControlStates();
    PosValueChanged( *m_pMfPositive );
}

IMPL_LINK_NOARG_TYPED(ErrorBarResources, PosValueChanged, Edit&, void)
{
    if( m_pCbSyncPosNeg->IsChecked())
    {
        if( m_pRbRange->IsChecked())
        {
            m_pEdRangeNegative->SetText( m_pEdRangePositive->GetText());
            m_bRangeNegUnique = m_bRangePosUnique;
        }
        else
            m_pMfNegative->SetValue( m_pMfPositive->GetValue());
    }
}

IMPL_LINK_NOARG_TYPED(ErrorBarResources, IndicatorChanged, Button*, void)
{
    m_bIndicatorUnique = true;
    if( m_pRbBoth->IsChecked())
        m_eIndicate = CHINDICATE_BOTH;
    else if( m_pRbPositive->IsChecked())
        m_eIndicate = CHINDICATE_UP;
    else if( m_pRbNegative->IsChecked())
        m_eIndicate = CHINDICATE_DOWN;
    else
        m_bIndicatorUnique = false;

    UpdateControlStates();
}

IMPL_LINK_TYPED( ErrorBarResources, ChooseRange, Button*, pButton, void )
{
    OSL_ASSERT( m_apRangeSelectionHelper.get());
    if( ! m_apRangeSelectionHelper.get())
        return;
    OSL_ASSERT( m_pCurrentRangeChoosingField == nullptr );

    OUString aUIString;

    if( pButton == m_pIbRangePositive )
    {
        m_pCurrentRangeChoosingField = m_pEdRangePositive;
        aUIString = m_pUIStringPos->GetText();
    }
    else
    {
        m_pCurrentRangeChoosingField = m_pEdRangeNegative;
        aUIString = m_pUIStringNeg->GetText();
    }

    assert( m_pParentDialog );
    if( m_pParentDialog )
    {
        lcl_enableRangeChoosing( true, m_pParentDialog );
        m_apRangeSelectionHelper->chooseRange(
            m_pCurrentRangeChoosingField->GetText(),
            aUIString, *this );
    }
    else
        m_pCurrentRangeChoosingField = nullptr;
}

IMPL_LINK_TYPED( ErrorBarResources, RangeChanged, Edit&, rEdit, void )
{
    if( &rEdit == m_pEdRangePositive )
    {
        m_bRangePosUnique = true;
        PosValueChanged( *m_pMfPositive );
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
    m_eErrorKind = CHERROR_NONE;
    SfxItemState aState = rInAttrs.GetItemState( SCHATTR_STAT_KIND_ERROR, true, &pPoolItem );
    m_bErrorKindUnique = ( aState != SfxItemState::DONTCARE );

    if( aState == SfxItemState::SET )
        m_eErrorKind = static_cast<const SvxChartKindErrorItem*>(pPoolItem)->GetValue();

    m_pLbFunction->SelectEntryPos( lcl_getLbEntryPosByErrorKind( m_eErrorKind ));

    if( m_bErrorKindUnique )
    {
        switch( m_eErrorKind )
        {
            case CHERROR_NONE:
                m_pRbNone->Check();
                break;
            case CHERROR_PERCENT:
                m_pRbPercent->Check();
                break;
            case CHERROR_CONST:
                m_pRbConst->Check();
                break;
            case CHERROR_STDERROR:
            case CHERROR_VARIANT:
            case CHERROR_SIGMA:
            case CHERROR_BIGERROR:
                m_pRbFunction->Check();
                break;
            case CHERROR_RANGE:
                m_pRbRange->Check();
                break;
        }
    }
    else
    {
        m_pRbNone->Check( false );
        m_pRbConst->Check( false );
        m_pRbPercent->Check( false );
        m_pRbFunction->Check( false );
    }

    // parameters
    aState = rInAttrs.GetItemState( SCHATTR_STAT_CONSTPLUS, true, &pPoolItem );
    m_bPlusUnique = ( aState != SfxItemState::DONTCARE );
    if( aState == SfxItemState::SET )
    {
        m_fPlusValue = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
    }

    aState = rInAttrs.GetItemState( SCHATTR_STAT_CONSTMINUS, true, &pPoolItem );
    m_bMinusUnique = ( aState != SfxItemState::DONTCARE );
    if( aState == SfxItemState::SET )
    {
        m_fMinusValue = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();

        if( m_eErrorKind != CHERROR_RANGE &&
            m_fPlusValue == m_fMinusValue )
            m_pCbSyncPosNeg->Check();
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
            case CHINDICATE_NONE :
                // no longer used, use both as default
                m_eIndicate = CHINDICATE_BOTH;
                SAL_FALLTHROUGH; // to BOTH
            case CHINDICATE_BOTH :
                m_pRbBoth->Check(); break;
            case CHINDICATE_UP :
                m_pRbPositive->Check(); break;
            case CHINDICATE_DOWN :
                m_pRbNegative->Check(); break;
        }
    }
    else
    {
        m_pRbBoth->Check( false );
        m_pRbPositive->Check( false );
        m_pRbNegative->Check( false );
    }

    // ranges
    aState = rInAttrs.GetItemState( SCHATTR_STAT_RANGE_POS, true, &pPoolItem );
    m_bRangePosUnique = ( aState != SfxItemState::DONTCARE );
    if( aState == SfxItemState::SET )
    {
        OUString sRangePositive = (static_cast< const SfxStringItem * >( pPoolItem ))->GetValue();
        m_pEdRangePositive->SetText( sRangePositive );
    }

    aState = rInAttrs.GetItemState( SCHATTR_STAT_RANGE_NEG, true, &pPoolItem );
    m_bRangeNegUnique = ( aState != SfxItemState::DONTCARE );
    if( aState == SfxItemState::SET )
    {
        OUString sRangeNegative = (static_cast< const SfxStringItem * >( pPoolItem ))->GetValue();
        m_pEdRangeNegative->SetText( sRangeNegative );
        if( m_eErrorKind == CHERROR_RANGE &&
            !sRangeNegative.isEmpty() &&
            sRangeNegative == m_pEdRangePositive->GetText() )
            m_pCbSyncPosNeg->Check();
    }

    UpdateControlStates();
}

bool ErrorBarResources::FillItemSet(SfxItemSet& rOutAttrs) const
{
    if( m_bErrorKindUnique )
        rOutAttrs.Put( SvxChartKindErrorItem( m_eErrorKind, SCHATTR_STAT_KIND_ERROR ));
    if( m_bIndicatorUnique )
        rOutAttrs.Put( SvxChartIndicateItem( m_eIndicate, SCHATTR_STAT_INDICATE ));

    if( m_bErrorKindUnique )
    {
        if( m_eErrorKind == CHERROR_RANGE )
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
                aPosRange = m_pEdRangePositive->GetText();
                if( m_pCbSyncPosNeg->IsChecked())
                    aNegRange = aPosRange;
                else
                    aNegRange = m_pEdRangeNegative->GetText();
            }

            if( m_bRangePosUnique )
                rOutAttrs.Put( SfxStringItem( SCHATTR_STAT_RANGE_POS, aPosRange ));
            if( m_bRangeNegUnique )
                rOutAttrs.Put( SfxStringItem( SCHATTR_STAT_RANGE_NEG, aNegRange ));
        }
        else if( m_eErrorKind == CHERROR_CONST ||
                 m_eErrorKind == CHERROR_PERCENT ||
                 m_eErrorKind == CHERROR_BIGERROR )
        {
            double fPosValue = static_cast< double >( m_pMfPositive->GetValue()) /
                pow( 10.0, m_pMfPositive->GetDecimalDigits());
            double fNegValue = 0.0;

            if( m_pCbSyncPosNeg->IsChecked())
                fNegValue = fPosValue;
            else
                fNegValue = static_cast< double >( m_pMfNegative->GetValue()) /
                    pow( 10.0, m_pMfNegative->GetDecimalDigits());

            rOutAttrs.Put( SvxDoubleItem( fPosValue, SCHATTR_STAT_CONSTPLUS ));
            rOutAttrs.Put( SvxDoubleItem( fNegValue, SCHATTR_STAT_CONSTMINUS ));
        }
    }

    rOutAttrs.Put( SfxBoolItem( SCHATTR_STAT_ERRORBAR_TYPE , m_eErrorBarType == ERROR_BAR_Y ));

    return true;
}

void ErrorBarResources::FillValueSets()
{
    if( m_eErrorBarType == ERROR_BAR_Y )
    {
        m_pFiNegative->SetImage( Image( SchResId( BMP_INDICATE_DOWN       ) ) );
        m_pFiPositive->SetImage( Image( SchResId( BMP_INDICATE_UP         ) ) );
        m_pFiBoth->SetImage(     Image( SchResId( BMP_INDICATE_BOTH_VERTI ) ) );
    }
    else if( m_eErrorBarType == ERROR_BAR_X )
    {
        m_pFiNegative->SetImage( Image( SchResId( BMP_INDICATE_LEFT      ) ) );
        m_pFiPositive->SetImage( Image( SchResId( BMP_INDICATE_RIGHT     ) ) );
        m_pFiBoth->SetImage(     Image( SchResId( BMP_INDICATE_BOTH_HORI ) ) );
    }
}

void ErrorBarResources::listeningFinished(
    const OUString & rNewRange )
{
    OSL_ASSERT( m_apRangeSelectionHelper.get());
    if( ! m_apRangeSelectionHelper.get())
        return;

    // rNewRange becomes invalid after removing the listener
    OUString aRange( rNewRange );

    // stop listening
    m_apRangeSelectionHelper->stopRangeListening();

    // change edit field
//     if( m_pParentWindow )
//     {
//         m_pParentWindow->ToTop();
//         m_pParentWindow->GrabFocus();
//     }

    if( m_pCurrentRangeChoosingField )
    {
        m_pCurrentRangeChoosingField->SetText( aRange );
        m_pCurrentRangeChoosingField->GrabFocus();
        PosValueChanged( *m_pMfPositive );
    }

    m_pCurrentRangeChoosingField = nullptr;

    UpdateControlStates();
    OSL_ASSERT( m_pParentDialog );
    if( m_pParentDialog )
        lcl_enableRangeChoosing( false, m_pParentDialog );
}

void ErrorBarResources::disposingRangeSelection()
{
    OSL_ASSERT( m_apRangeSelectionHelper.get());
    if( m_apRangeSelectionHelper.get())
        m_apRangeSelectionHelper->stopRangeListening( false );
}

bool ErrorBarResources::isRangeFieldContentValid( Edit & rEdit )
{
    OUString aRange( rEdit.GetText());
    bool bIsValid = ( aRange.isEmpty() ) ||
        ( m_apRangeSelectionHelper.get() &&
          m_apRangeSelectionHelper->verifyCellRange( aRange ));

    if( bIsValid || !rEdit.IsEnabled())
    {
        rEdit.SetControlForeground();
        rEdit.SetControlBackground();
    }
    else
    {
        rEdit.SetControlBackground( RANGE_SELECTION_INVALID_RANGE_BACKGROUND_COLOR );
        rEdit.SetControlForeground( RANGE_SELECTION_INVALID_RANGE_FOREGROUND_COLOR );
    }

    return bIsValid;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
