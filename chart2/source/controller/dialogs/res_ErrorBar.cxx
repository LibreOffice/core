/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_ErrorBar.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:45:55 $
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

#include "res_ErrorBar.hxx"
#include "res_ErrorBar_IDs.hrc"
#include "ResId.hxx"
#include "Strings.hrc"
#include "Bitmaps.hrc"
#include "Bitmaps_HC.hrc"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

// macro for selecting a normal or high contrast bitmap the stack variable
// bIsHighContrast must exist and reflect the correct state
#define SELECT_BITMAP(name) Bitmap( SchResId( bIsHighContrast ? name ## _HC : name ))

enum StatIndicator
{
    INDICATE_BOTH,
    INDICATE_UP,
    INDICATE_DOWN
};

ErrorBarResources::ErrorBarResources( Window* pWindow, const SfxItemSet& rInAttrs,
                                      tErrorBarType eType /* = ERROR_BAR_Y */ ) :
    m_aFlErrorCategory (pWindow, SchResId (FL_ERROR)),
    m_aRbtNone (pWindow, SchResId (RBT_NONE)),
    m_aRbtVariant (pWindow, SchResId (RBT_VARIANT)),
    m_aRbtSigma (pWindow, SchResId (RBT_SIGMA)),
    m_aRbtPercent (pWindow, SchResId (RBT_PERCENT)),
    m_aRbtBigError (pWindow, SchResId (RBT_BIGERROR)),
    m_aRbtConst (pWindow, SchResId (RBT_CONST)),
//     m_aRbtRange (pWindow, SchResId (RBT_RANGE)),
    m_aMtrFldPercent (pWindow, SchResId (MTR_FLD_PERCENT)),
    m_aMtrFldBigError (pWindow, SchResId (MTR_FLD_BIGERROR)),
    m_aFTConstPlus (pWindow, SchResId (FT_PERCENT_PLUS)),
    m_aMtrFldConstPlus (pWindow, SchResId (MTR_FLD_PLUS)),
    m_aFTConstMinus (pWindow, SchResId (FT_PERCENT_MINUS)),
    m_aMtrFldConstMinus (pWindow, SchResId (MTR_FLD_MINUS)),
    m_aFLIndicate (pWindow, SchResId (FL_INDICATE)),
    m_aIndicatorSet (pWindow, SchResId (CT_INDICATE)),
    m_eErrorKind( CHERROR_NONE ),
    m_eIndicate( CHINDICATE_BOTH ),
    m_bErrorKindUnique( true ),
    m_bIndicatorUnique( true ),
    m_bPlusUnique( true ),
    m_bMinusUnique( true ),
    m_eErrorBarType( eType )
{
    m_aRbtNone.SetClickHdl(LINK(this, ErrorBarResources, RBtnClick));
    m_aRbtVariant.SetClickHdl(LINK(this, ErrorBarResources, RBtnClick));
    m_aRbtSigma.SetClickHdl(LINK(this, ErrorBarResources, RBtnClick));
    m_aRbtPercent.SetClickHdl(LINK(this, ErrorBarResources, RBtnClick));
    m_aRbtBigError.SetClickHdl(LINK(this, ErrorBarResources, RBtnClick));
    m_aRbtConst.SetClickHdl(LINK(this, ErrorBarResources, RBtnClick));

    FillValueSets();

    m_aIndicatorSet.SetStyle (m_aIndicatorSet.GetStyle () /*| WB_ITEMBORDER | WB_DOUBLEBORDER*/ | WB_NAMEFIELD /*| WB_VSCROLL*/ );
    m_aIndicatorSet.SetColCount(5);
    m_aIndicatorSet.SetLineCount(1);
    m_aIndicatorSet.SetExtraSpacing(2);
    m_aIndicatorSet.SetSelectHdl(LINK(this, ErrorBarResources, SelectIndicate));

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

void ErrorBarResources::SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth )
{
    USHORT nDecimalDigits = 1;
    if( fMinorStepWidth < 0 )
        fMinorStepWidth *= -1.0;
    if( !::rtl::math::approxEqual(fMinorStepWidth,0.0) )
    {
        sal_Int32 nExponent = static_cast< sal_Int32 >( ::rtl::math::approxFloor( log10( fMinorStepWidth ) ) );
        if(nExponent<0)
            nDecimalDigits = static_cast< sal_uInt16 >( -nExponent );
    }
    m_aMtrFldConstPlus.SetDecimalDigits(nDecimalDigits);
    m_aMtrFldConstMinus.SetDecimalDigits(nDecimalDigits);
}

IMPL_LINK( ErrorBarResources, RBtnClick, Button *, pBtn )
{
    m_aMtrFldPercent.Enable (pBtn == &m_aRbtPercent);
    m_aMtrFldBigError.Enable (pBtn == &m_aRbtBigError);
    m_aMtrFldConstPlus.Enable (pBtn == &m_aRbtConst);
    m_aMtrFldConstMinus.Enable (pBtn == &m_aRbtConst);
    m_aFTConstPlus.Enable (pBtn == &m_aRbtConst);
    m_aFTConstMinus.Enable (pBtn == &m_aRbtConst);

    m_aIndicatorSet.Show (pBtn != &m_aRbtNone);
//     m_aFLIndicate.Show (pBtn != &m_aRbtNone);

    if (pBtn == &m_aRbtPercent) m_eErrorKind = CHERROR_PERCENT;
    else if (pBtn == &m_aRbtBigError) m_eErrorKind = CHERROR_BIGERROR;
    else if (pBtn == &m_aRbtConst) m_eErrorKind = CHERROR_CONST;
    else if (pBtn == &m_aRbtNone) m_eErrorKind = CHERROR_NONE;
    else if (pBtn == &m_aRbtVariant) m_eErrorKind = CHERROR_VARIANT;
    else if (pBtn == &m_aRbtSigma) m_eErrorKind = CHERROR_SIGMA;

    if( pBtn != &m_aRbtNone )
    {
        if( m_eIndicate == CHINDICATE_NONE )
        {
            m_bIndicatorUnique = true;
            m_eIndicate = CHINDICATE_BOTH;
            m_aIndicatorSet.SelectItem(INDICATE_BOTH + 1);
        }
    }

    m_bErrorKindUnique = true;

    return 0;
}

IMPL_LINK( ErrorBarResources, SelectIndicate, void *, EMPTYARG )
{
    StatIndicator eSelection = (StatIndicator) m_aIndicatorSet.GetSelectItemId();

    switch (eSelection - 1)
    {
//      case INDICATE_NONE :
//          m_eIndicate = CHINDICATE_NONE;
//          break;

        case INDICATE_BOTH :
            m_eIndicate = CHINDICATE_BOTH;
            break;

        case INDICATE_UP :
            m_eIndicate = CHINDICATE_UP;
            break;

        case INDICATE_DOWN :
            m_eIndicate = CHINDICATE_DOWN;
            break;
    }

    m_aIndicatorSet.SelectItem( static_cast< sal_uInt16 >(eSelection));
    m_aIndicatorSet.Show();
//     m_aFLIndicate.Show();

    m_bIndicatorUnique = true;

    return 0;
}

void ErrorBarResources::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;
    SfxItemState aState = SFX_ITEM_UNKNOWN;

     m_eErrorKind = CHERROR_NONE;
    aState = rInAttrs.GetItemState( SCHATTR_STAT_KIND_ERROR, TRUE, &pPoolItem );
    m_bErrorKindUnique = ( aState != SFX_ITEM_DONTCARE );

    if( aState == SFX_ITEM_SET )
        m_eErrorKind = ((const SvxChartKindErrorItem*) pPoolItem)->GetValue();

    if( m_bErrorKindUnique )
    {
        m_aRbtNone.Check    ( m_eErrorKind == CHERROR_NONE );
        m_aRbtVariant.Check ( m_eErrorKind == CHERROR_VARIANT );
        m_aRbtSigma.Check   ( m_eErrorKind == CHERROR_SIGMA );
        m_aRbtPercent.Check ( m_eErrorKind == CHERROR_PERCENT );
        m_aRbtBigError.Check( m_eErrorKind == CHERROR_BIGERROR );
        m_aRbtConst.Check   ( m_eErrorKind == CHERROR_CONST );
    }
    else
    {
        m_aRbtNone.Check    ( FALSE );
        m_aRbtVariant.Check ( FALSE );
        m_aRbtSigma.Check   ( FALSE );
        m_aRbtPercent.Check ( FALSE );
        m_aRbtBigError.Check( FALSE );
        m_aRbtConst.Check   ( FALSE );
    }

    m_aIndicatorSet.Show(m_eErrorKind != CHERROR_NONE);
//     m_aFLIndicate.Show(m_eErrorKind != CHERROR_NONE);

    //-----

    aState = rInAttrs.GetItemState( SCHATTR_STAT_CONSTPLUS, TRUE, &pPoolItem );
    m_bPlusUnique = ( aState != SFX_ITEM_DONTCARE );
    double fPlusValue = 0.0;
    if( aState == SFX_ITEM_SET )
    {
        fPlusValue = ((const SvxDoubleItem*) pPoolItem)->GetValue();
        sal_Int32 nPlusValue = static_cast< sal_Int32 >( fPlusValue * pow(10.0,m_aMtrFldConstPlus.GetDecimalDigits()) );
        m_aMtrFldConstPlus.SetValue( nPlusValue );
    }

    aState = rInAttrs.GetItemState( SCHATTR_STAT_CONSTMINUS, TRUE, &pPoolItem );
    m_bMinusUnique = ( aState != SFX_ITEM_DONTCARE );
    double fMinusValue = 0.0;
    if( aState == SFX_ITEM_SET )
    {
        fMinusValue = ((const SvxDoubleItem*) pPoolItem)->GetValue();
        sal_Int32 nMinusValue = static_cast< sal_Int32 >( fMinusValue * pow(10.0,m_aMtrFldConstMinus.GetDecimalDigits()) );
        m_aMtrFldConstMinus.SetValue( nMinusValue );
    }

    if( m_bPlusUnique && m_bMinusUnique )
    {
        sal_Int32 nAvg = static_cast< sal_Int32 >( ((fPlusValue + fMinusValue)/ 2.0) * pow(10.0,m_aMtrFldPercent.GetDecimalDigits()) );
        m_aMtrFldBigError.SetDecimalDigits( m_aMtrFldPercent.GetDecimalDigits() );
        m_aMtrFldPercent.SetValue(  nAvg );
        m_aMtrFldBigError.SetValue( nAvg );
    }

    m_aMtrFldPercent.Enable (m_eErrorKind == CHERROR_PERCENT);
    m_aMtrFldBigError.Enable (m_eErrorKind == CHERROR_BIGERROR);
    m_aMtrFldConstPlus.Enable (m_eErrorKind == CHERROR_CONST);
    m_aMtrFldConstMinus.Enable (m_eErrorKind == CHERROR_CONST);
    m_aFTConstPlus.Enable (m_eErrorKind == CHERROR_CONST);
    m_aFTConstMinus.Enable (m_eErrorKind == CHERROR_CONST);


    aState = rInAttrs.GetItemState( SCHATTR_STAT_INDICATE, TRUE, &pPoolItem );
    m_bIndicatorUnique = ( aState != SFX_ITEM_DONTCARE );
    if( aState == SFX_ITEM_SET)
        m_eIndicate = ((const SvxChartIndicateItem * ) pPoolItem)->GetValue();

    if( m_bIndicatorUnique )
    {
        switch( m_eIndicate )
        {
            case CHINDICATE_NONE :
                // no longer used
                OSL_ENSURE( false, "CHINDICATE_NONE no longer supported" );
                break;
            case CHINDICATE_BOTH :
                m_aIndicatorSet.SelectItem(INDICATE_BOTH + 1);
                break;
            case CHINDICATE_UP :
                m_aIndicatorSet.SelectItem(INDICATE_UP + 1);
                break;
            case CHINDICATE_DOWN :
                m_aIndicatorSet.SelectItem(INDICATE_DOWN + 1);
                break;
        }                                                             \
    }
    else
        m_aIndicatorSet.SetNoSelection();
}

BOOL ErrorBarResources::FillItemSet(SfxItemSet& rOutAttrs) const
{
    if( m_bErrorKindUnique )
        rOutAttrs.Put( SvxChartKindErrorItem( m_eErrorKind, SCHATTR_STAT_KIND_ERROR ));
    if( m_bIndicatorUnique )
        rOutAttrs.Put( SvxChartIndicateItem( m_eIndicate, SCHATTR_STAT_INDICATE ));

    if( m_bErrorKindUnique )
    {
        switch( m_eErrorKind )
        {
            case CHERROR_PERCENT :
                rOutAttrs.Put (SvxDoubleItem ((double) m_aMtrFldPercent.GetValue () / double(pow(10.0,m_aMtrFldPercent.GetDecimalDigits())), SCHATTR_STAT_PERCENT));
                break;

            case CHERROR_BIGERROR :
                rOutAttrs.Put (SvxDoubleItem ((double) m_aMtrFldBigError.GetValue () / double(pow(10.0,m_aMtrFldBigError.GetDecimalDigits())), SCHATTR_STAT_BIGERROR));
                break;

            case CHERROR_CONST :
                rOutAttrs.Put (SvxDoubleItem ((double) m_aMtrFldConstPlus.GetValue () / double(pow(10.0,m_aMtrFldConstPlus.GetDecimalDigits())), SCHATTR_STAT_CONSTPLUS));
                rOutAttrs.Put (SvxDoubleItem ((double) m_aMtrFldConstMinus.GetValue () / double(pow(10.0,m_aMtrFldConstMinus.GetDecimalDigits())), SCHATTR_STAT_CONSTMINUS));
                break;

            case CHERROR_NONE:
            case CHERROR_VARIANT:
            case CHERROR_SIGMA:
                // nothing
                break;
        }
    }

    return TRUE;
}

void ErrorBarResources::FillValueSets()
{
    bool bIsHighContrast = ( true && m_aRbtConst.GetDisplayBackground().GetColor().IsDark() );

    if( m_aIndicatorSet.GetItemCount() == 0 )
    {
        if( m_eErrorBarType == ERROR_BAR_Y )
        {
            m_aIndicatorSet.InsertItem( INDICATE_BOTH + 1, SELECT_BITMAP( BMP_INDICATE_BOTH_VERTI ),
                                        String(SchResId(STR_INDICATE_BOTH)));
            m_aIndicatorSet.InsertItem( INDICATE_DOWN + 1, SELECT_BITMAP( BMP_INDICATE_DOWN ),
                                        String(SchResId(STR_INDICATE_DOWN)));
            m_aIndicatorSet.InsertItem( INDICATE_UP + 1, SELECT_BITMAP( BMP_INDICATE_UP ),
                                        String(SchResId(STR_INDICATE_UP)));
        }
        else if( m_eErrorBarType == ERROR_BAR_X )
        {
            m_aIndicatorSet.InsertItem( INDICATE_BOTH + 1, SELECT_BITMAP( BMP_INDICATE_BOTH_HORI ),
                                        String(SchResId(STR_INDICATE_BOTH)));
            m_aIndicatorSet.InsertItem( INDICATE_DOWN + 1, SELECT_BITMAP( BMP_INDICATE_LEFT ),
                                        String(SchResId(STR_INDICATE_DOWN)));
            m_aIndicatorSet.InsertItem( INDICATE_UP + 1, SELECT_BITMAP( BMP_INDICATE_RIGHT ),
                                        String(SchResId(STR_INDICATE_UP)));
        }
        else
        {
            OSL_ENSURE( false,  "error bar type not handled" );
        }
    }
    else
    {
        if( m_eErrorBarType == ERROR_BAR_Y )
        {
            m_aIndicatorSet.SetItemImage( INDICATE_BOTH + 1, SELECT_BITMAP( BMP_INDICATE_BOTH_VERTI ));
            m_aIndicatorSet.SetItemImage( INDICATE_DOWN + 1, SELECT_BITMAP( BMP_INDICATE_DOWN ));
            m_aIndicatorSet.SetItemImage( INDICATE_UP + 1, SELECT_BITMAP( BMP_INDICATE_UP ));
        }
        else if( m_eErrorBarType == ERROR_BAR_X )
        {
            m_aIndicatorSet.SetItemImage( INDICATE_BOTH + 1, SELECT_BITMAP( BMP_INDICATE_BOTH_HORI ));
            m_aIndicatorSet.SetItemImage( INDICATE_DOWN + 1, SELECT_BITMAP( BMP_INDICATE_LEFT ));
            m_aIndicatorSet.SetItemImage( INDICATE_UP + 1, SELECT_BITMAP( BMP_INDICATE_RIGHT ));
        }
        else
        {
            OSL_ENSURE( false,  "error bar type not handled" );
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
