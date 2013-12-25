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
#include "res_ErrorBar_IDs.hrc"
#include "ResId.hxx"
#include "Strings.hrc"
#include "Bitmaps.hrc"
#include "RangeSelectionHelper.hxx"
// for RANGE_SELECTION_INVALID_RANGE_BACKGROUND_COLOR
#include "TabPageNotifiable.hxx"
#include "macros.hxx"

#include <rtl/math.hxx>
#include <vcl/dialog.hxx>
#include <svl/stritem.hxx>

using namespace ::com::sun::star;

namespace
{
void lcl_enableRangeChoosing( bool bEnable, Dialog * pDialog )
{
    if( pDialog )
    {
        pDialog->Show( bEnable ? sal_False : sal_True );
        pDialog->SetModalInputMode( bEnable ? sal_False : sal_True );
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

ErrorBarResources::ErrorBarResources( SfxTabPage* pParent, Dialog * pParentDialog,
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
        m_pParentWindow( pParent ),
        m_pParentDialog( pParentDialog ),
        m_pCurrentRangeChoosingField( 0 ),
        m_bHasInternalDataProvider( true ),
        m_bDisableDataTableDialog( false )
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

//TODO: ohallot: Remove these strings from .src files
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
    m_pLbFunction->SetSelectHdl( LINK( this, ErrorBarResources, CategoryChosen ));

    m_pCbSyncPosNeg->Check( sal_False );
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
                xProps->getPropertyValue( "DisableDataTableDialog" ) >>= m_bDisableDataTableDialog;
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
    m_pRbRange->Enable( !m_bHasInternalDataProvider || !m_bDisableDataTableDialog );
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

IMPL_LINK( ErrorBarResources, CategoryChosen, void *,  )
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
    return 0;
}

IMPL_LINK_NOARG(ErrorBarResources, SynchronizePosAndNeg)
{
    UpdateControlStates();
    PosValueChanged( 0 );
    return 0;
}

IMPL_LINK_NOARG(ErrorBarResources, PosValueChanged)
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

    return 0;
}

IMPL_LINK_NOARG(ErrorBarResources, IndicatorChanged)
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
    return 0;
}

IMPL_LINK( ErrorBarResources, ChooseRange, RangeSelectionButton *, pButton )
{
    OSL_ASSERT( m_apRangeSelectionHelper.get());
    if( ! m_apRangeSelectionHelper.get())
        return 0;
    OSL_ASSERT( m_pCurrentRangeChoosingField == 0 );

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
        m_pCurrentRangeChoosingField = 0;

    return 0;
}

IMPL_LINK( ErrorBarResources, RangeChanged, Edit *, pEdit )
{
    if( pEdit == m_pEdRangePositive )
    {
        m_bRangePosUnique = true;
        PosValueChanged( 0 );
    }
    else
    {
        m_bRangeNegUnique = true;
    }

    isRangeFieldContentValid( *pEdit );

    return 0;
}

void ErrorBarResources::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;
    SfxItemState aState = SFX_ITEM_UNKNOWN;

    // category
     m_eErrorKind = CHERROR_NONE;
    aState = rInAttrs.GetItemState( SCHATTR_STAT_KIND_ERROR, sal_True, &pPoolItem );
    m_bErrorKindUnique = ( aState != SFX_ITEM_DONTCARE );

    if( aState == SFX_ITEM_SET )
        m_eErrorKind = ((const SvxChartKindErrorItem*) pPoolItem)->GetValue();

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
        m_pRbNone->Check( sal_False );
        m_pRbConst->Check( sal_False );
        m_pRbPercent->Check( sal_False );
        m_pRbFunction->Check( sal_False );
    }

    // parameters
    aState = rInAttrs.GetItemState( SCHATTR_STAT_CONSTPLUS, sal_True, &pPoolItem );
    m_bPlusUnique = ( aState != SFX_ITEM_DONTCARE );
    double fPlusValue = 0.0;
    if( aState == SFX_ITEM_SET )
    {
        fPlusValue = ((const SvxDoubleItem*) pPoolItem)->GetValue();
        sal_Int32 nPlusValue = static_cast< sal_Int32 >( fPlusValue * pow(10.0,m_pMfPositive->GetDecimalDigits()) );
        m_pMfPositive->SetValue( nPlusValue );
    }

    aState = rInAttrs.GetItemState( SCHATTR_STAT_CONSTMINUS, sal_True, &pPoolItem );
    m_bMinusUnique = ( aState != SFX_ITEM_DONTCARE );
    if( aState == SFX_ITEM_SET )
    {
        double fMinusValue = ((const SvxDoubleItem*) pPoolItem)->GetValue();
        sal_Int32 nMinusValue = static_cast< sal_Int32 >( fMinusValue * pow(10.0,m_pMfNegative->GetDecimalDigits()) );
        m_pMfNegative->SetValue( nMinusValue );

        if( m_eErrorKind != CHERROR_RANGE &&
            fPlusValue == fMinusValue )
            m_pCbSyncPosNeg->Check();
    }

    // indicator
    aState = rInAttrs.GetItemState( SCHATTR_STAT_INDICATE, sal_True, &pPoolItem );
    m_bIndicatorUnique = ( aState != SFX_ITEM_DONTCARE );
    if( aState == SFX_ITEM_SET)
        m_eIndicate = ((const SvxChartIndicateItem * ) pPoolItem)->GetValue();

    if( m_bIndicatorUnique )
    {
        switch( m_eIndicate )
        {
            case CHINDICATE_NONE :
                // no longer used, use both as default
                m_eIndicate = CHINDICATE_BOTH;
                // fall-through intended to BOTH
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
        m_pRbBoth->Check( sal_False );
        m_pRbPositive->Check( sal_False );
        m_pRbNegative->Check( sal_False );
    }

    // ranges
    aState = rInAttrs.GetItemState( SCHATTR_STAT_RANGE_POS, sal_True, &pPoolItem );
    m_bRangePosUnique = ( aState != SFX_ITEM_DONTCARE );
    if( aState == SFX_ITEM_SET )
    {
        OUString sRangePositive = (static_cast< const SfxStringItem * >( pPoolItem ))->GetValue();
        m_pEdRangePositive->SetText( sRangePositive );
    }

    aState = rInAttrs.GetItemState( SCHATTR_STAT_RANGE_NEG, sal_True, &pPoolItem );
    m_bRangeNegUnique = ( aState != SFX_ITEM_DONTCARE );
    if( aState == SFX_ITEM_SET )
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

sal_Bool ErrorBarResources::FillItemSet(SfxItemSet& rOutAttrs) const
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

    return sal_True;
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
    if( m_pParentWindow )
    {
        m_pParentWindow->ToTop();
        m_pParentWindow->GrabFocus();
    }

    if( m_pCurrentRangeChoosingField )
    {
        m_pCurrentRangeChoosingField->SetText( aRange );
        m_pCurrentRangeChoosingField->GrabFocus();
        PosValueChanged( 0 );
    }

    m_pCurrentRangeChoosingField = 0;

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

oldErrorBarResources::oldErrorBarResources( Window* pParent, Dialog * pParentDialog,
                                      const SfxItemSet& rInAttrs, bool bNoneAvailable,
                                      tErrorBarType eType /* = ERROR_BAR_Y */ ) :

        m_aFlErrorCategory( pParent, SchResId( FL_ERROR )),
        m_aRbNone( pParent, SchResId( RB_NONE )),
        m_aRbConst( pParent, SchResId( RB_CONST )),
        m_aRbPercent( pParent, SchResId( RB_PERCENT )),
        m_aRbFunction( pParent, SchResId( RB_FUNCTION )),
        m_aRbRange( pParent, SchResId( RB_RANGE )),
        m_aLbFunction( pParent, SchResId( LB_FUNCTION )),

        m_aFlParameters( pParent, SchResId( FL_PARAMETERS )),
        m_aFtPositive( pParent, SchResId( FT_POSITIVE )),
        m_aMfPositive( pParent, SchResId( MF_POSITIVE )),
        m_aEdRangePositive( pParent, SchResId( ED_RANGE_POSITIVE )),
        m_aIbRangePositive( pParent, SchResId( IB_RANGE_POSITIVE )),
        m_aFtNegative( pParent, SchResId( FT_NEGATIVE )),
        m_aMfNegative( pParent, SchResId( MF_NEGATIVE )),
        m_aEdRangeNegative( pParent, SchResId( ED_RANGE_NEGATIVE )),
        m_aIbRangeNegative( pParent, SchResId( IB_RANGE_NEGATIVE )),
        m_aCbSyncPosNeg( pParent, SchResId( CB_SYN_POS_NEG )),

        m_aFlIndicate( pParent, SchResId( FL_INDICATE )),
        m_aRbBoth( pParent, SchResId( RB_BOTH )),
        m_aRbPositive( pParent, SchResId( RB_POSITIVE )),
        m_aRbNegative( pParent, SchResId( RB_NEGATIVE )),
        m_aFiBoth( pParent, SchResId( FI_BOTH )),
        m_aFiPositive( pParent, SchResId( FI_POSITIVE )),
        m_aFiNegative( pParent, SchResId( FI_NEGATIVE )),

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
        m_pParentWindow( pParent ),
        m_pParentDialog( pParentDialog ),
        m_pCurrentRangeChoosingField( 0 ),
        m_bHasInternalDataProvider( true ),
        m_bDisableDataTableDialog( false )
{
    if( m_bNoneAvailable )
        m_aRbNone.SetClickHdl( LINK( this, oldErrorBarResources, CategoryChosen ));
    else
        m_aRbNone.Hide();

    m_aRbConst.SetClickHdl( LINK( this, oldErrorBarResources, CategoryChosen ));
    m_aRbPercent.SetClickHdl( LINK( this, oldErrorBarResources, CategoryChosen ));
    m_aRbFunction.SetClickHdl( LINK( this, oldErrorBarResources, CategoryChosen ));
    m_aRbRange.SetClickHdl( LINK( this, oldErrorBarResources, CategoryChosen ));
    m_aLbFunction.SetSelectHdl( LINK( this, oldErrorBarResources, CategoryChosen ));

    m_aCbSyncPosNeg.Check( sal_False );
    m_aCbSyncPosNeg.SetToggleHdl( LINK( this, oldErrorBarResources, SynchronizePosAndNeg ));

    m_aMfPositive.SetModifyHdl( LINK( this, oldErrorBarResources, PosValueChanged ));
    m_aEdRangePositive.SetModifyHdl( LINK( this, oldErrorBarResources, RangeChanged ));
    m_aEdRangeNegative.SetModifyHdl( LINK( this, oldErrorBarResources, RangeChanged ));

    m_aRbPositive.SetClickHdl( LINK( this, oldErrorBarResources, IndicatorChanged ));
    m_aRbNegative.SetClickHdl( LINK( this, oldErrorBarResources, IndicatorChanged ));
    m_aRbBoth.SetClickHdl( LINK( this, oldErrorBarResources, IndicatorChanged ));

    m_aIbRangePositive.SetClickHdl( LINK( this, oldErrorBarResources, ChooseRange ));
    m_aIbRangeNegative.SetClickHdl( LINK( this, oldErrorBarResources, ChooseRange ));
    m_aIbRangePositive.SetQuickHelpText(SCH_RESSTR(STR_TIP_SELECT_RANGE));
    m_aIbRangeNegative.SetQuickHelpText(SCH_RESSTR(STR_TIP_SELECT_RANGE));

    FillValueSets();
    Reset( rInAttrs );
}

oldErrorBarResources::~oldErrorBarResources()
{
}

void oldErrorBarResources::SetErrorBarType( tErrorBarType eNewType )
{
    if( m_eErrorBarType != eNewType )
    {
        m_eErrorBarType = eNewType;
        FillValueSets();
    }
}

void oldErrorBarResources::SetChartDocumentForRangeChoosing(
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
                xProps->getPropertyValue( "DisableDataTableDialog" ) >>= m_bDisableDataTableDialog;
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
        m_aRbRange.SetText(SCH_RESSTR(STR_CONTROLTEXT_ERROR_BARS_FROM_DATA));
    }

    if( m_aRbRange.IsChecked())
    {
        isRangeFieldContentValid( m_aEdRangePositive );
        isRangeFieldContentValid( m_aEdRangeNegative );
    }
}

void oldErrorBarResources::SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth )
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

void oldErrorBarResources::UpdateControlStates()
{
    // function
    bool bIsFunction = m_aRbFunction.IsChecked();
    m_aLbFunction.Enable( bIsFunction );

    // range buttons
    m_aRbRange.Enable( !m_bHasInternalDataProvider || !m_bDisableDataTableDialog );
    bool bShowRange = ( m_aRbRange.IsChecked());
    bool bCanChooseRange =
        ( bShowRange &&
          m_apRangeSelectionHelper.get() &&
          m_apRangeSelectionHelper->hasRangeSelection());

    m_aMfPositive.Show( ! bShowRange );
    m_aMfNegative.Show( ! bShowRange );

    // use range but without range chooser => hide controls
    m_aEdRangePositive.Show( bShowRange && ! m_bHasInternalDataProvider );
    m_aIbRangePositive.Show( bCanChooseRange );
    m_aEdRangeNegative.Show( bShowRange && ! m_bHasInternalDataProvider );
    m_aIbRangeNegative.Show( bCanChooseRange );

    bool bShowPosNegAndSync = ! (bShowRange && m_bHasInternalDataProvider);
    m_aFtPositive.Show( bShowPosNegAndSync );
    m_aFtNegative.Show( bShowPosNegAndSync );
    m_aCbSyncPosNeg.Show( bShowPosNegAndSync );
    m_aFlParameters.Show( bShowPosNegAndSync );

    // unit for metric fields
    bool bIsErrorMargin(
        ( m_aRbFunction.IsChecked()) &&
        ( m_aLbFunction.GetSelectEntryPos() == CHART_LB_FUNCTION_ERROR_MARGIN ));
    bool bIsPercentage( m_aRbPercent.IsChecked() || bIsErrorMargin );
    OUString aCustomUnit;

    if( bIsPercentage )
    {
        aCustomUnit = " %";
        m_aMfPositive.SetDecimalDigits( 1 );
        m_aMfPositive.SetSpinSize( 10 );
        m_aMfNegative.SetDecimalDigits( 1 );
        m_aMfNegative.SetSpinSize( 10 );
    }
    else
    {
        m_aMfPositive.SetDecimalDigits( m_nConstDecimalDigits );
        m_aMfPositive.SetSpinSize( m_nConstSpinSize );
        m_aMfNegative.SetDecimalDigits( m_nConstDecimalDigits );
        m_aMfNegative.SetSpinSize( m_nConstSpinSize );
    }
    m_aMfPositive.SetCustomUnitText( aCustomUnit );
    m_aMfNegative.SetCustomUnitText( aCustomUnit );

    // positive and negative value fields
    bool bPosEnabled = ( m_aRbPositive.IsChecked() || m_aRbBoth.IsChecked());
    bool bNegEnabled = ( m_aRbNegative.IsChecked() || m_aRbBoth.IsChecked());
    if( !( bPosEnabled || bNegEnabled ))
    {
        // all three controls are not checked -> ambiguous state
        bPosEnabled = true;
        bNegEnabled = true;
    }

    // functions with only one parameter
    bool bOneParameterCategory =
        bIsErrorMargin || m_aRbPercent.IsChecked();
    if( bOneParameterCategory )
    {
        m_aCbSyncPosNeg.Check();
    }

    if( m_aCbSyncPosNeg.IsChecked())
    {
        bPosEnabled = true;
        bNegEnabled = false;
    }

    // all functions except error margin take no arguments
    if( m_aRbFunction.IsChecked() &&
        ( m_aLbFunction.GetSelectEntryPos() != CHART_LB_FUNCTION_ERROR_MARGIN ))
    {
        bPosEnabled = false;
        bNegEnabled = false;
    }

    // enable/disable pos/neg fields
    m_aFtPositive.Enable( bPosEnabled );
    m_aFtNegative.Enable( bNegEnabled );
    if( bShowRange )
    {
        m_aEdRangePositive.Enable( bPosEnabled );
        m_aIbRangePositive.Enable( bPosEnabled );
        m_aEdRangeNegative.Enable( bNegEnabled );
        m_aIbRangeNegative.Enable( bNegEnabled );
    }
    else
    {
        m_aMfPositive.Enable( bPosEnabled );
        m_aMfNegative.Enable( bNegEnabled );
    }

    m_aCbSyncPosNeg.Enable(
        !bOneParameterCategory &&
        ( bPosEnabled || bNegEnabled ));

    // mark invalid entries in the range fields
    if( bShowRange && ! m_bHasInternalDataProvider )
    {
        isRangeFieldContentValid( m_aEdRangePositive );
        isRangeFieldContentValid( m_aEdRangeNegative );
    }
}

IMPL_LINK( oldErrorBarResources, CategoryChosen, void *,  )
{
    m_bErrorKindUnique = true;
    SvxChartKindError eOldError = m_eErrorKind;

    if( m_aRbNone.IsChecked())
        m_eErrorKind = CHERROR_NONE;
    else if( m_aRbConst.IsChecked())
        m_eErrorKind = CHERROR_CONST;
    else if( m_aRbPercent.IsChecked())
        m_eErrorKind = CHERROR_PERCENT;
    else if( m_aRbRange.IsChecked())
        m_eErrorKind = CHERROR_RANGE;
    else if( m_aRbFunction.IsChecked())
    {
        if( m_aLbFunction.GetSelectEntryCount() == 1 )
        {
            switch( m_aLbFunction.GetSelectEntryPos())
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
        m_aCbSyncPosNeg.Check(
            (!m_aEdRangePositive.GetText().isEmpty()) &&
            m_aEdRangePositive.GetText() == m_aEdRangeNegative.GetText());
    }
    // changed from range
    else if( m_eErrorKind != CHERROR_RANGE &&
        eOldError == CHERROR_RANGE )
    {
        m_aCbSyncPosNeg.Check(
            m_aMfPositive.GetValue() == m_aMfNegative.GetValue());
    }

    UpdateControlStates();
    return 0;
}

IMPL_LINK_NOARG(oldErrorBarResources, SynchronizePosAndNeg)
{
    UpdateControlStates();
    PosValueChanged( 0 );
    return 0;
}

IMPL_LINK_NOARG(oldErrorBarResources, PosValueChanged)
{
    if( m_aCbSyncPosNeg.IsChecked())
    {
        if( m_aRbRange.IsChecked())
        {
            m_aEdRangeNegative.SetText( m_aEdRangePositive.GetText());
            m_bRangeNegUnique = m_bRangePosUnique;
        }
        else
            m_aMfNegative.SetValue( m_aMfPositive.GetValue());
    }

    return 0;
}

IMPL_LINK_NOARG(oldErrorBarResources, IndicatorChanged)
{
    m_bIndicatorUnique = true;
    if( m_aRbBoth.IsChecked())
        m_eIndicate = CHINDICATE_BOTH;
    else if( m_aRbPositive.IsChecked())
        m_eIndicate = CHINDICATE_UP;
    else if( m_aRbNegative.IsChecked())
        m_eIndicate = CHINDICATE_DOWN;
    else
        m_bIndicatorUnique = false;

    UpdateControlStates();
    return 0;
}

IMPL_LINK( oldErrorBarResources, ChooseRange, RangeSelectionButton *, pButton )
{
    OSL_ASSERT( m_apRangeSelectionHelper.get());
    if( ! m_apRangeSelectionHelper.get())
        return 0;
    OSL_ASSERT( m_pCurrentRangeChoosingField == 0 );

    OUString aUIString;
    if( pButton == &m_aIbRangePositive )
    {
        m_pCurrentRangeChoosingField = &m_aEdRangePositive;
        aUIString = SCH_RESSTR( STR_DATA_SELECT_RANGE_FOR_POSITIVE_ERRORBARS );
    }
    else
    {
        m_pCurrentRangeChoosingField = &m_aEdRangeNegative;
        aUIString = SCH_RESSTR( STR_DATA_SELECT_RANGE_FOR_NEGATIVE_ERRORBARS );
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
        m_pCurrentRangeChoosingField = 0;

    return 0;
}

IMPL_LINK( oldErrorBarResources, RangeChanged, Edit *, pEdit )
{
    if( pEdit == & m_aEdRangePositive )
    {
        m_bRangePosUnique = true;
        PosValueChanged( 0 );
    }
    else
    {
        m_bRangeNegUnique = true;
    }

    isRangeFieldContentValid( *pEdit );

    return 0;
}

void oldErrorBarResources::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;
    SfxItemState aState = SFX_ITEM_UNKNOWN;

    // category
     m_eErrorKind = CHERROR_NONE;
    aState = rInAttrs.GetItemState( SCHATTR_STAT_KIND_ERROR, sal_True, &pPoolItem );
    m_bErrorKindUnique = ( aState != SFX_ITEM_DONTCARE );

    if( aState == SFX_ITEM_SET )
        m_eErrorKind = ((const SvxChartKindErrorItem*) pPoolItem)->GetValue();

    m_aLbFunction.SelectEntryPos( lcl_getLbEntryPosByErrorKind( m_eErrorKind ));

    if( m_bErrorKindUnique )
    {
        switch( m_eErrorKind )
        {
            case CHERROR_NONE:
                m_aRbNone.Check();
                break;
            case CHERROR_PERCENT:
                m_aRbPercent.Check();
                break;
            case CHERROR_CONST:
                m_aRbConst.Check();
                break;
            case CHERROR_STDERROR:
            case CHERROR_VARIANT:
            case CHERROR_SIGMA:
            case CHERROR_BIGERROR:
                m_aRbFunction.Check();
                break;
            case CHERROR_RANGE:
                m_aRbRange.Check();
                break;
        }
    }
    else
    {
        m_aRbNone.Check( sal_False );
        m_aRbConst.Check( sal_False );
        m_aRbPercent.Check( sal_False );
        m_aRbFunction.Check( sal_False );
    }

    // parameters
    aState = rInAttrs.GetItemState( SCHATTR_STAT_CONSTPLUS, sal_True, &pPoolItem );
    m_bPlusUnique = ( aState != SFX_ITEM_DONTCARE );
    double fPlusValue = 0.0;
    if( aState == SFX_ITEM_SET )
    {
        fPlusValue = ((const SvxDoubleItem*) pPoolItem)->GetValue();
        sal_Int32 nPlusValue = static_cast< sal_Int32 >( fPlusValue * pow(10.0,m_aMfPositive.GetDecimalDigits()) );
        m_aMfPositive.SetValue( nPlusValue );
    }

    aState = rInAttrs.GetItemState( SCHATTR_STAT_CONSTMINUS, sal_True, &pPoolItem );
    m_bMinusUnique = ( aState != SFX_ITEM_DONTCARE );
    if( aState == SFX_ITEM_SET )
    {
        double fMinusValue = ((const SvxDoubleItem*) pPoolItem)->GetValue();
        sal_Int32 nMinusValue = static_cast< sal_Int32 >( fMinusValue * pow(10.0,m_aMfNegative.GetDecimalDigits()) );
        m_aMfNegative.SetValue( nMinusValue );

        if( m_eErrorKind != CHERROR_RANGE &&
            fPlusValue == fMinusValue )
            m_aCbSyncPosNeg.Check();
    }

    // indicator
    aState = rInAttrs.GetItemState( SCHATTR_STAT_INDICATE, sal_True, &pPoolItem );
    m_bIndicatorUnique = ( aState != SFX_ITEM_DONTCARE );
    if( aState == SFX_ITEM_SET)
        m_eIndicate = ((const SvxChartIndicateItem * ) pPoolItem)->GetValue();

    if( m_bIndicatorUnique )
    {
        switch( m_eIndicate )
        {
            case CHINDICATE_NONE :
                // no longer used, use both as default
                m_eIndicate = CHINDICATE_BOTH;
                // fall-through intended to BOTH
            case CHINDICATE_BOTH :
                m_aRbBoth.Check(); break;
            case CHINDICATE_UP :
                m_aRbPositive.Check(); break;
            case CHINDICATE_DOWN :
                m_aRbNegative.Check(); break;
        }
    }
    else
    {
        m_aRbBoth.Check( sal_False );
        m_aRbPositive.Check( sal_False );
        m_aRbNegative.Check( sal_False );
    }

    // ranges
    aState = rInAttrs.GetItemState( SCHATTR_STAT_RANGE_POS, sal_True, &pPoolItem );
    m_bRangePosUnique = ( aState != SFX_ITEM_DONTCARE );
    if( aState == SFX_ITEM_SET )
    {
        OUString sRangePositive = (static_cast< const SfxStringItem * >( pPoolItem ))->GetValue();
        m_aEdRangePositive.SetText( sRangePositive );
    }

    aState = rInAttrs.GetItemState( SCHATTR_STAT_RANGE_NEG, sal_True, &pPoolItem );
    m_bRangeNegUnique = ( aState != SFX_ITEM_DONTCARE );
    if( aState == SFX_ITEM_SET )
    {
        OUString sRangeNegative = (static_cast< const SfxStringItem * >( pPoolItem ))->GetValue();
        m_aEdRangeNegative.SetText( sRangeNegative );
        if( m_eErrorKind == CHERROR_RANGE &&
            !sRangeNegative.isEmpty() &&
            sRangeNegative == m_aEdRangePositive.GetText() )
            m_aCbSyncPosNeg.Check();
    }

    UpdateControlStates();
}

sal_Bool oldErrorBarResources::FillItemSet(SfxItemSet& rOutAttrs) const
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
                aPosRange = m_aEdRangePositive.GetText();
                if( m_aCbSyncPosNeg.IsChecked())
                    aNegRange = aPosRange;
                else
                    aNegRange = m_aEdRangeNegative.GetText();
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
            double fPosValue = static_cast< double >( m_aMfPositive.GetValue()) /
                pow( 10.0, m_aMfPositive.GetDecimalDigits());
            double fNegValue = 0.0;

            if( m_aCbSyncPosNeg.IsChecked())
                fNegValue = fPosValue;
            else
                fNegValue = static_cast< double >( m_aMfNegative.GetValue()) /
                    pow( 10.0, m_aMfNegative.GetDecimalDigits());

            rOutAttrs.Put( SvxDoubleItem( fPosValue, SCHATTR_STAT_CONSTPLUS ));
            rOutAttrs.Put( SvxDoubleItem( fNegValue, SCHATTR_STAT_CONSTMINUS ));
        }
    }

    rOutAttrs.Put( SfxBoolItem( SCHATTR_STAT_ERRORBAR_TYPE , m_eErrorBarType == ERROR_BAR_Y ));

    return sal_True;
}

void oldErrorBarResources::FillValueSets()
{
    if( m_eErrorBarType == ERROR_BAR_Y )
    {
        m_aFiNegative.SetImage( Image( SchResId( BMP_INDICATE_DOWN       ) ) );
        m_aFiPositive.SetImage( Image( SchResId( BMP_INDICATE_UP         ) ) );
        m_aFiBoth.SetImage(     Image( SchResId( BMP_INDICATE_BOTH_VERTI ) ) );
    }
    else if( m_eErrorBarType == ERROR_BAR_X )
    {
        m_aFiNegative.SetImage( Image( SchResId( BMP_INDICATE_LEFT      ) ) );
        m_aFiPositive.SetImage( Image( SchResId( BMP_INDICATE_RIGHT     ) ) );
        m_aFiBoth.SetImage(     Image( SchResId( BMP_INDICATE_BOTH_HORI ) ) );
    }
}

void oldErrorBarResources::listeningFinished(
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
    if( m_pParentWindow )
    {
        m_pParentWindow->ToTop();
        m_pParentWindow->GrabFocus();
    }

    if( m_pCurrentRangeChoosingField )
    {
        m_pCurrentRangeChoosingField->SetText( aRange );
        m_pCurrentRangeChoosingField->GrabFocus();
        PosValueChanged( 0 );
    }

    m_pCurrentRangeChoosingField = 0;

    UpdateControlStates();
    OSL_ASSERT( m_pParentDialog );
    if( m_pParentDialog )
        lcl_enableRangeChoosing( false, m_pParentDialog );
}

void oldErrorBarResources::disposingRangeSelection()
{
    OSL_ASSERT( m_apRangeSelectionHelper.get());
    if( m_apRangeSelectionHelper.get())
        m_apRangeSelectionHelper->stopRangeListening( false );
}

bool oldErrorBarResources::isRangeFieldContentValid( Edit & rEdit )
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
