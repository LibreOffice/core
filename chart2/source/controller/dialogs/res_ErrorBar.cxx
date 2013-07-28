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

ErrorBarResources::ErrorBarResources( Window* pParent, Dialog * pParentDialog,
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
        m_aRbNone.SetClickHdl( LINK( this, ErrorBarResources, CategoryChosen ));
    else
        m_aRbNone.Hide();

    m_aRbConst.SetClickHdl( LINK( this, ErrorBarResources, CategoryChosen ));
    m_aRbPercent.SetClickHdl( LINK( this, ErrorBarResources, CategoryChosen ));
    m_aRbFunction.SetClickHdl( LINK( this, ErrorBarResources, CategoryChosen ));
    m_aRbRange.SetClickHdl( LINK( this, ErrorBarResources, CategoryChosen ));
    m_aLbFunction.SetSelectHdl( LINK( this, ErrorBarResources, CategoryChosen ));

    m_aCbSyncPosNeg.Check( sal_False );
    m_aCbSyncPosNeg.SetToggleHdl( LINK( this, ErrorBarResources, SynchronizePosAndNeg ));

    m_aMfPositive.SetModifyHdl( LINK( this, ErrorBarResources, PosValueChanged ));
    m_aEdRangePositive.SetModifyHdl( LINK( this, ErrorBarResources, RangeChanged ));
    m_aEdRangeNegative.SetModifyHdl( LINK( this, ErrorBarResources, RangeChanged ));

    m_aRbPositive.SetClickHdl( LINK( this, ErrorBarResources, IndicatorChanged ));
    m_aRbNegative.SetClickHdl( LINK( this, ErrorBarResources, IndicatorChanged ));
    m_aRbBoth.SetClickHdl( LINK( this, ErrorBarResources, IndicatorChanged ));

    m_aIbRangePositive.SetClickHdl( LINK( this, ErrorBarResources, ChooseRange ));
    m_aIbRangeNegative.SetClickHdl( LINK( this, ErrorBarResources, ChooseRange ));
    m_aIbRangePositive.SetQuickHelpText(SCH_RESSTR(STR_TIP_SELECT_RANGE));
    m_aIbRangeNegative.SetQuickHelpText(SCH_RESSTR(STR_TIP_SELECT_RANGE));

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
        m_aRbRange.SetText(SCH_RESSTR(STR_CONTROLTEXT_ERROR_BARS_FROM_DATA));
    }

    if( m_aRbRange.IsChecked())
    {
        isRangeFieldContentValid( m_aEdRangePositive );
        isRangeFieldContentValid( m_aEdRangeNegative );
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

IMPL_LINK( ErrorBarResources, CategoryChosen, void *,  )
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

IMPL_LINK_NOARG(ErrorBarResources, SynchronizePosAndNeg)
{
    UpdateControlStates();
    PosValueChanged( 0 );
    return 0;
}

IMPL_LINK_NOARG(ErrorBarResources, PosValueChanged)
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

IMPL_LINK_NOARG(ErrorBarResources, IndicatorChanged)
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

IMPL_LINK( ErrorBarResources, ChooseRange, RangeSelectionButton *, pButton )
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

IMPL_LINK( ErrorBarResources, RangeChanged, Edit *, pEdit )
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
                aPosRange = OUString("x");
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

void ErrorBarResources::FillValueSets()
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
        m_pCurrentRangeChoosingField->SetText( String( aRange ));
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

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
