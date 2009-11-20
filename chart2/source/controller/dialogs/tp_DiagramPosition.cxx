/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tp_DiagramPosition.cxx,v $
 * $Revision: 1.13 $
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
#include "tp_DiagramPosition.hxx"
#include "tp_DiagramPosition.hrc"

#include "DiagramHelper.hxx"
#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"

#include <svx/svxids.hrc>
#include <svx/svdview.hxx>
//GetModuleFieldUnit
#include <svx/dlgutil.hxx>
#include <svtools/rectitem.hxx>
#include <svtools/aeitem.hxx>
// header for class SvxDoubleItem
#include <svx/chrtitem.hxx>

using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

void lcl_ConvertRect(basegfx::B2DRange& rRange, const sal_uInt16 nDigits, const MapUnit ePoolUnit, const FieldUnit eDlgUnit)
{
    const basegfx::B2DPoint aTopLeft(
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMinX()), nDigits, ePoolUnit, eDlgUnit),
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMinY()), nDigits, ePoolUnit, eDlgUnit));
    const basegfx::B2DPoint aBottomRight(
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMaxX()), nDigits, ePoolUnit, eDlgUnit),
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMaxY()), nDigits, ePoolUnit, eDlgUnit));

    rRange = basegfx::B2DRange(aTopLeft, aBottomRight);
}

void lcl_ScaleRect(basegfx::B2DRange& rRange, const Fraction aUIScale)
{
    const double fFactor(1.0 / double(aUIScale));
    rRange = basegfx::B2DRange(rRange.getMinimum() * fFactor, rRange.getMaximum() * fFactor);
}

DiagramPositionTabPage::DiagramPositionTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :
    SvxTabPage      ( pParent, SchResId( TP_DIAGRAM_POSITION ), rInAttrs ),

    m_aFlPosMode( this, SchResId( FL_POSITIONING_MODE ) ),

    m_aRbPosMode_Auto( this, SchResId( RB_POSITIONING_MODE_AUTOMATIC ) ),
    m_aRbPosMode_Including( this, SchResId( RB_POSITIONING_MODE_INCLUDING ) ),
    m_aRbPosMode_Excluding( this, SchResId( RB_POSITIONING_MODE_EXCLUDING ) ),

    maFlPosition        ( this, SchResId( FL_POSITION ) ),
    maFtPosX            ( this, SchResId( FT_POS_X ) ),
    maMtrPosX           ( this, SchResId( MTR_FLD_POS_X ) ),
    maFtPosY            ( this, SchResId( FT_POS_Y ) ),
    maMtrPosY           ( this, SchResId( MTR_FLD_POS_Y ) ),
    maFtPosReference    ( this, SchResId( FT_POSREFERENCE ) ),
    maCtlPos            ( this, SchResId( CTL_POSRECT ), RP_LT ),

    maFlSize                         ( this, SchResId( FL_SIZE ) ),
    maFtWidth                        ( this, SchResId( FT_WIDTH ) ),
    maMtrWidth                       ( this, SchResId( MTR_FLD_WIDTH ) ),
    maFtHeight                       ( this, SchResId( FT_HEIGHT ) ),
    maMtrHeight                      ( this, SchResId( MTR_FLD_HEIGHT ) ),
    maCbxScale                       ( this, SchResId( CBX_SCALE ) ),
    maFtSizeReference                ( this, SchResId( FT_SIZEREFERENCE) ),
    maCtlSize                        ( this, SchResId( CTL_SIZERECT ), RP_LT ),
    m_aExcludingRect(1,2,3,4),
    m_aIncludingRect(1,2,3,4),
    m_aMaxRect(1,2,3,4),
    m_fUIScale( 1.0 ),
    m_aCurrentRect(1,2,3,4),
    m_bRectChangedByUser( false )
{
    FreeResource();

    // this pege needs ExchangeSupport
    // SetExchangeSupport();

    // evaluate PoolUnit
    SfxItemPool* pPool = rInAttrs.GetPool();
    DBG_ASSERT( pPool, "no pool (!)" );
    mePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );

    Construct();

    m_aRbPosMode_Auto.SetClickHdl( LINK( this, DiagramPositionTabPage, ChangeModeHdl ) );
    m_aRbPosMode_Including.SetClickHdl( LINK( this, DiagramPositionTabPage, ChangeModeHdl ) );
    m_aRbPosMode_Excluding.SetClickHdl( LINK( this, DiagramPositionTabPage, ChangeModeHdl ) );

    maMtrPosX.SetModifyHdl( LINK( this, DiagramPositionTabPage, ChangePosXHdl ) );
    maMtrPosY.SetModifyHdl( LINK( this, DiagramPositionTabPage, ChangePosYHdl ) );

    maMtrWidth.SetModifyHdl( LINK( this, DiagramPositionTabPage, ChangeWidthHdl ) );
    maMtrHeight.SetModifyHdl( LINK( this, DiagramPositionTabPage, ChangeHeightHdl ) );
    maCbxScale.SetClickHdl( LINK( this, DiagramPositionTabPage, ClickScaleHdl ) );
}

// -----------------------------------------------------------------------

void DiagramPositionTabPage::Construct()
{
    // get range and work area
    meDlgUnit = GetModuleFieldUnit( &GetItemSet() );
    SetFieldUnit( maMtrPosX, meDlgUnit, TRUE );
    SetFieldUnit( maMtrPosY, meDlgUnit, TRUE );
    SetFieldUnit( maMtrWidth, meDlgUnit, TRUE );
    SetFieldUnit( maMtrHeight, meDlgUnit, TRUE );

    if(FUNIT_MILE == meDlgUnit || FUNIT_KM == meDlgUnit)
    {
        maMtrPosX.SetDecimalDigits( 3 );
        maMtrPosY.SetDecimalDigits( 3 );
        maMtrWidth.SetDecimalDigits( 3 );
        maMtrHeight.SetDecimalDigits( 3 );
    }
}

// -----------------------------------------------------------------------

void DiagramPositionTabPage::Reset( const SfxItemSet& rInAttrs )
{
    //positioning mode
    DiagramPositioningMode ePos = DiagramPositioningMode_AUTO;
    const SfxPoolItem* pItem = NULL;
    if( rInAttrs.GetItemState( SCHATTR_DIAGRAM_POS_MODE, TRUE, &pItem ) == SFX_ITEM_SET )
        ePos = DiagramPositioningMode(((const SfxInt32Item*)pItem)->GetValue());
    if( ePos == DiagramPositioningMode_AUTO )
        m_aRbPosMode_Auto.Check();
    else if( ePos == DiagramPositioningMode_INCLUDING )
        m_aRbPosMode_Including.Check();
    else if( ePos == DiagramPositioningMode_EXCLUDING )
        m_aRbPosMode_Excluding.Check();

    ReleaseBorders();
    maCtlPos.Reset();
    maCtlSize.Reset();

    //rectangles
    if( rInAttrs.GetItemState( SCHATTR_DIAGRAM_RECT_MAX, TRUE, &pItem ) == SFX_ITEM_SET )
    {
        m_aMaxRect = ((const SfxRectangleItem*)pItem)->GetValue();
        SetRectIn100thmm( m_aMaxRect );
        m_aMaxRect = GetRectIn100thmm();
    }

    if( rInAttrs.GetItemState( SCHATTR_DIAGRAM_RECT_EXCLUDING, TRUE, &pItem ) == SFX_ITEM_SET )
    {
        m_aExcludingRect = ((const SfxRectangleItem*)pItem)->GetValue();
        SetRectIn100thmm( m_aExcludingRect );
        m_aExcludingRect = GetRectIn100thmm();
    }

    if( rInAttrs.GetItemState( SCHATTR_DIAGRAM_RECT_INCLUDING, TRUE, &pItem ) == SFX_ITEM_SET )
    {
        m_aIncludingRect = ((const SfxRectangleItem*)pItem)->GetValue();
        SetRectIn100thmm( m_aIncludingRect );
        m_aIncludingRect = GetRectIn100thmm();
    }

    if( rInAttrs.GetItemState( SCHATTR_DIAGRAM_UI_SCALE, TRUE, &pItem ) == SFX_ITEM_SET )
        m_fUIScale = ((const SvxDoubleItem*)pItem)->GetValue();

    m_aCurrentRect = m_aIncludingRect;
    if( rInAttrs.GetItemState( SCHATTR_DIAGRAM_RECT_TO_USE, TRUE, &pItem ) == SFX_ITEM_SET )
    {
        m_aCurrentRect = ((const SfxRectangleItem*)pItem)->GetValue();
        SetRectIn100thmm( m_aCurrentRect );
        m_aCurrentRect = GetRectIn100thmm();
    }

    // scale
    String aStr = GetUserData();
    maCbxScale.Check( (BOOL)aStr.ToInt32() );
    mfScaleSizeX  = std::max( (double)GetCoreValue( maMtrWidth,  mePoolUnit ), 1.0 );
    mfScaleSizeY = std::max( (double)GetCoreValue( maMtrHeight, mePoolUnit ), 1.0 );


    m_bRectChangedByUser = false;
    SetMinMaxPosition();
    UpdateControlStates();
}

// -----------------------------------------------------------------------

BOOL DiagramPositionTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    BOOL bModified(TRUE);

    //positioning mode
    sal_Int32 nMode = 0;
    if( m_aRbPosMode_Including.IsChecked() )
        nMode = 1;
    else if( m_aRbPosMode_Excluding.IsChecked() )
        nMode = 2;
    rOutAttrs.Put( SfxInt32Item( SCHATTR_DIAGRAM_POS_MODE, nMode ) );

    rOutAttrs.Put( SfxRectangleItem( SCHATTR_DIAGRAM_RECT_TO_USE, GetRectIn100thmm() ) );

    return bModified;
}

// -----------------------------------------------------------------------

SfxTabPage* DiagramPositionTabPage::Create( Window* pWindow, const SfxItemSet& rOutAttrs )
{
    return( new DiagramPositionTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

void DiagramPositionTabPage::ActivatePage( const SfxItemSet& /*rSet*/ )
{
}

// -----------------------------------------------------------------------

int DiagramPositionTabPage::DeactivatePage( SfxItemSet* pItemSet )
{
    if( pItemSet )
        FillItemSet( *pItemSet );
    return LEAVE_PAGE;
}

//------------------------------------------------------------------------

long DiagramPositionTabPage::get1oothMMPosValue( MetricField& rField )
{
    double fValue( GetCoreValue( rField, mePoolUnit ) * m_fUIScale);
    return basegfx::fround(fValue);
}

long DiagramPositionTabPage::get1oothMMSizeValue( MetricField& rField )
{
    double fValue = static_cast<double>(rField.GetValue( meDlgUnit ));
    fValue = MetricField::ConvertDoubleValue( fValue, rField.GetBaseValue(), rField.GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
    long nValue = long(fValue*m_fUIScale);
    nValue = OutputDevice::LogicToLogic( nValue, MAP_100TH_MM, (MapUnit)mePoolUnit );
    nValue = static_cast<long>(rField.Denormalize( nValue ));
    return nValue;
}

/*
const double fTmp((((const SfxInt32Item*)pItem)->GetValue() - maAnchor.getY()) / fUIScale);
SetMetricValue(maMtrPosY, basegfx::fround(fTmp), mePoolUnit);
    sal_Int64 nVal = OutputDevice::LogicToLogic( nCoreValue, (MapUnit)eUnit, MAP_100TH_MM );
    nVal = rField.Normalize( nVal );
    rField.SetValue( nVal, FUNIT_100TH_MM );
    */

void DiagramPositionTabPage::set1oothMMPosValue( MetricField& rField, long n100thMM )
{
    const double fTmp( n100thMM / m_fUIScale );
    SetMetricValue(rField, basegfx::fround(fTmp), mePoolUnit);
}
void DiagramPositionTabPage::set1oothMMSizeValue( MetricField& rField, long n100thMM )
{
    double fValue((OutputDevice::LogicToLogic( n100thMM, (MapUnit)mePoolUnit, MAP_100TH_MM)) );
    fValue /= m_fUIScale;
    if(rField.GetDecimalDigits())
        fValue *= pow(10.0, rField.GetDecimalDigits());
    fValue = MetricField::ConvertDoubleValue(fValue, rField.GetBaseValue(), rField.GetDecimalDigits(), FUNIT_100TH_MM, meDlgUnit);
    rField.SetValue(static_cast<sal_Int64>(fValue), meDlgUnit);
}

sal_Int64 DiagramPositionTabPage::convert1oothMMValueToFieldUnit( MetricField& rField, long n100thMM )
{
    double fValue((OutputDevice::LogicToLogic( n100thMM, (MapUnit)mePoolUnit, MAP_100TH_MM)) );
    fValue /= m_fUIScale;
    if(rField.GetDecimalDigits())
        fValue *= pow(10.0, rField.GetDecimalDigits());
    fValue = MetricField::ConvertDoubleValue(fValue, rField.GetBaseValue(), rField.GetDecimalDigits(), FUNIT_100TH_MM, meDlgUnit);
    return basegfx::fround64(fValue);
}

//------------------------------------------------------------------------

void DiagramPositionTabPage::SetRectIn100thmm( const Rectangle& rRect )
{
    ReleaseBorders();

    m_aCurrentRect = rRect;
    long nLeft = m_aCurrentRect.Left();
    long nTop = m_aCurrentRect.Top();
    long nWidth = m_aCurrentRect.getWidth();
    long nHeight = m_aCurrentRect.getHeight();

    switch ( maCtlPos.GetActualRP() )
    {
        case RP_LT:
        {
            break;
        }
        case RP_MT:
        {
            nLeft   += nWidth / 2;
            break;
        }
        case RP_RT:
        {
            nLeft   += nWidth;
            break;
        }
        case RP_LM:
        {
            nTop    += nHeight / 2;
            break;
        }
        case RP_MM:
        {
            nLeft   += nWidth / 2;
            nTop    += nHeight / 2;
            break;
        }
        case RP_RM:
        {
            nLeft   += nWidth;
            nTop    += nHeight / 2;
            break;
        }
        case RP_LB:
        {
            nTop    += nHeight;
            break;
        }
        case RP_MB:
        {
            nLeft   += nWidth / 2;
            nTop    += nHeight;
            break;
        }
        case RP_RB:
        {
            nLeft   += nWidth;
            nTop    += nHeight;
            break;
        }
    }

    set1oothMMPosValue( maMtrPosX, nLeft );
    set1oothMMPosValue( maMtrPosY, nTop );
    set1oothMMSizeValue( maMtrWidth, nWidth );
    set1oothMMSizeValue( maMtrHeight, nHeight );

    SetMinMaxPosition();
}

//------------------------------------------------------------------------

Rectangle DiagramPositionTabPage::GetRectIn100thmm()
{
    long nX = get1oothMMPosValue( maMtrPosX );
    long nY = get1oothMMPosValue( maMtrPosY );
    long nWidth = get1oothMMSizeValue( maMtrWidth );
    long nHeight = get1oothMMSizeValue( maMtrHeight );

    switch( maCtlPos.GetActualRP() )
    {
        case RP_LT:
        {
            break;
        }
        case RP_MT:
        {
            nX -= nWidth/2;
            break;
        }
        case RP_RT:
        {
            nX -= nWidth;
            break;
        }
        case RP_LM:
        {
            nY -= nHeight/2;
            break;
        }
        case RP_MM:
        {
            nX -= nWidth/2;
            nY -= nHeight/2;
            break;
        }
        case RP_RM:
        {
            nX -= nWidth;
            nY -= nHeight/2;
            break;
        }
        case RP_LB:
        {
            nY -= nHeight;
            break;
        }
        case RP_MB:
        {
            nX -= nWidth/2;
            nY -= nHeight;
            break;
        }
        case RP_RB:
        {
            nX -= nWidth;
            nY -= nHeight;
            break;
        }
    }

    return Rectangle(nX,nY,nX+nWidth,nY+nHeight);
}

//------------------------------------------------------------------------

void DiagramPositionTabPage::UpdateControlStates()
{
    bool bEnable = true;

    m_aRbPosMode_Auto.Enable( bEnable );
    m_aRbPosMode_Including.Enable( bEnable );
    m_aRbPosMode_Excluding.Enable( bEnable );

    if( m_aRbPosMode_Auto.IsChecked() )
        bEnable = false;

    maFlPosition.Enable( bEnable );
    maFtPosX.Enable( bEnable );
    maMtrPosX.Enable( bEnable );
    maFtPosY.Enable( bEnable );
    maMtrPosY.Enable( bEnable );
    maFtPosReference.Enable( bEnable );
    maCtlPos.Enable( bEnable );

    maFlSize.Enable( bEnable );
    maCtlSize.Enable( bEnable );
    maFtWidth.Enable( bEnable );
    maMtrWidth.Enable( bEnable );
    maFtHeight.Enable( bEnable );
    maMtrHeight.Enable( bEnable );
    maCbxScale.Enable( bEnable );
    maFtSizeReference.Enable( bEnable );

    maCtlSize.Invalidate();
    maCtlPos.Invalidate();
}

//------------------------------------------------------------------------

void DiagramPositionTabPage::ReleaseBorders()
{
    //make it possible to set new values that will lead to different min max values afterwards
    maMtrPosX.SetMin( convert1oothMMValueToFieldUnit( maMtrPosX, m_aMaxRect.Left() ), meDlgUnit);
    maMtrPosX.SetMax( convert1oothMMValueToFieldUnit( maMtrPosX, m_aMaxRect.Right() ), meDlgUnit);
    maMtrPosY.SetMin( convert1oothMMValueToFieldUnit( maMtrPosY, m_aMaxRect.Top() ), meDlgUnit);
    maMtrPosY.SetMax( convert1oothMMValueToFieldUnit( maMtrPosY, m_aMaxRect.Bottom() ), meDlgUnit);
    maMtrWidth.SetMax( convert1oothMMValueToFieldUnit( maMtrWidth, m_aMaxRect.GetWidth() ), meDlgUnit);
    maMtrHeight.SetMax( convert1oothMMValueToFieldUnit( maMtrHeight, m_aMaxRect.GetHeight() ), meDlgUnit);
}

//------------------------------------------------------------------------

void DiagramPositionTabPage::SetMinMaxPosition()
{
    // position

    long nLeft( m_aMaxRect.Left() );
    long nTop( m_aMaxRect.Top() );
    long nRight( m_aMaxRect.Right() );
    long nBottom( m_aMaxRect.Bottom() );

    const long nWidth( m_aCurrentRect.GetWidth() );
    const long nHeight( m_aCurrentRect.GetHeight() );

    switch ( maCtlPos.GetActualRP() )
    {
        case RP_LT:
        {
            nRight  -= nWidth;
            nBottom -= nHeight;
            break;
        }
        case RP_MT:
        {
            nLeft   += nWidth / 2;
            nRight  -= nWidth / 2;
            nBottom -= nHeight;
            break;
        }
        case RP_RT:
        {
            nLeft   += nWidth;
            nBottom -= nHeight;
            break;
        }
        case RP_LM:
        {
            nRight  -= nWidth;
            nTop    += nHeight / 2;
            nBottom -= nHeight / 2;
            break;
        }
        case RP_MM:
        {
            nLeft   += nWidth / 2;
            nRight  -= nWidth / 2;
            nTop    += nHeight / 2;
            nBottom -= nHeight / 2;
            break;
        }
        case RP_RM:
        {
            nLeft   += nWidth;
            nTop    += nHeight / 2;
            nBottom -= nHeight / 2;
            break;
        }
        case RP_LB:
        {
            nRight  -= nWidth;
            nTop    += nHeight;
            break;
        }
        case RP_MB:
        {
            nLeft   += nWidth / 2;
            nRight  -= nWidth / 2;
            nTop    += nHeight;
            break;
        }
        case RP_RB:
        {
            nLeft   += nWidth;
            nTop    += nHeight;
            break;
        }
    }

    maMtrPosX.SetMin( convert1oothMMValueToFieldUnit( maMtrPosX, nLeft ), meDlgUnit);
    maMtrPosX.SetFirst( convert1oothMMValueToFieldUnit( maMtrPosX, nLeft ), meDlgUnit);
    maMtrPosX.SetMax( convert1oothMMValueToFieldUnit( maMtrPosX, nRight ), meDlgUnit);
    maMtrPosX.SetLast( convert1oothMMValueToFieldUnit( maMtrPosX, nRight ), meDlgUnit);
    maMtrPosY.SetMin( convert1oothMMValueToFieldUnit( maMtrPosY, nTop ), meDlgUnit);
    maMtrPosY.SetFirst( convert1oothMMValueToFieldUnit( maMtrPosY, nTop ), meDlgUnit);
    maMtrPosY.SetMax( convert1oothMMValueToFieldUnit( maMtrPosY, nBottom ), meDlgUnit);
    maMtrPosY.SetLast( convert1oothMMValueToFieldUnit( maMtrPosY, nBottom ), meDlgUnit);

    // size

    nLeft = m_aMaxRect.Left();
    nTop = m_aMaxRect.Top();
    nRight = m_aMaxRect.Right();
    nBottom = m_aMaxRect.Bottom();

    long nMaxWidth( m_aMaxRect.GetWidth() );
    long nMaxHeight( m_aMaxRect.GetHeight() );

    switch ( maCtlSize.GetActualRP() )
    {
        case RP_LT:
        {
            nMaxWidth -= ( m_aCurrentRect.Left() - nLeft );
            nMaxHeight -= ( m_aCurrentRect.Top() - nTop );
            break;
        }
        case RP_MT:
        {
            nMaxWidth = std::min( m_aCurrentRect.Center().X() - nLeft, nRight - m_aCurrentRect.Center().X() ) * 2;
            nMaxHeight -= ( m_aCurrentRect.Top() - nTop );
            break;
        }
        case RP_RT:
        {
            nMaxWidth -= ( nRight - m_aCurrentRect.Right() );
            nMaxHeight -= ( m_aCurrentRect.Top() - nTop );
            break;
        }
        case RP_LM:
        {
            nMaxWidth -= ( m_aCurrentRect.Left() - nLeft );
            nMaxHeight = std::min( m_aCurrentRect.Center().Y() - nTop, nBottom - m_aCurrentRect.Center().Y() ) * 2;
            break;
        }
        case RP_MM:
        {
            nMaxWidth = std::min( m_aCurrentRect.Center().X() - nLeft, nRight - m_aCurrentRect.Center().X() ) * 2;
            nMaxHeight = std::min( m_aCurrentRect.Center().Y() - nTop, nBottom - m_aCurrentRect.Center().Y() ) * 2;
            break;
        }
        case RP_RM:
        {
            nMaxWidth -= ( nRight - m_aCurrentRect.Right() );
            nMaxHeight = std::min( m_aCurrentRect.Center().Y() - nTop, nBottom - m_aCurrentRect.Center().Y() ) * 2;
            break;
        }
        case RP_LB:
        {
            nMaxWidth -= ( m_aCurrentRect.Left() - nLeft );
            nMaxHeight -=( nBottom - m_aCurrentRect.Bottom() );
            break;
        }
        case RP_MB:
        {
            nMaxWidth = std::min( m_aCurrentRect.Center().X() - nLeft, nRight - m_aCurrentRect.Center().X() ) * 2;
            nMaxHeight -= ( m_aCurrentRect.Bottom() - nBottom );
            break;
        }
        case RP_RB:
        {
            nMaxWidth -= ( nRight - m_aCurrentRect.Right() );
            nMaxHeight -= ( nBottom - m_aCurrentRect.Bottom() );
            break;
        }
    }

    if( maCbxScale.IsChecked() && maCbxScale.IsEnabled() )
    {
        sal_Int64 nMaxHeightResultingFromScale(basegfx::fround64((mfScaleSizeY * (double)nMaxWidth) / mfScaleSizeX));
        if( nMaxHeightResultingFromScale > nMaxHeight )
            nMaxWidth = basegfx::fround64((mfScaleSizeX * (double)nMaxHeight / mfScaleSizeY));
        sal_Int64 nMaxWidthResultingFromScale(basegfx::fround64((mfScaleSizeX * (double)nMaxHeight) / mfScaleSizeY));
        if( nMaxWidthResultingFromScale > nMaxWidth )
            nMaxHeight = basegfx::fround64((mfScaleSizeY * (double)nMaxWidth / mfScaleSizeX));
    }

    maMtrWidth.SetMax( convert1oothMMValueToFieldUnit( maMtrWidth, nMaxWidth ), meDlgUnit);
    maMtrWidth.SetLast( convert1oothMMValueToFieldUnit( maMtrWidth, nMaxWidth ), meDlgUnit);
    maMtrHeight.SetMax( convert1oothMMValueToFieldUnit( maMtrHeight, nMaxHeight ), meDlgUnit);
    maMtrHeight.SetLast( convert1oothMMValueToFieldUnit( maMtrHeight, nMaxHeight ), meDlgUnit);
}

//------------------------------------------------------------------------

void DiagramPositionTabPage::PointChanged( Window* pWindow, RECT_POINT /*eRP*/ )
{
    if( pWindow == &maCtlPos )
        SetRectIn100thmm( m_aCurrentRect );
    else
        SetMinMaxPosition();
}

//------------------------------------------------------------------------

IMPL_LINK( DiagramPositionTabPage, ChangeModeHdl, RadioButton*, pButton )
{
    UpdateControlStates();
    if( pButton == &m_aRbPosMode_Including )
    {
        if( !m_bRectChangedByUser )
            SetRectIn100thmm( m_aIncludingRect );
    }
    else if( pButton == &m_aRbPosMode_Excluding )
    {
        if( !m_bRectChangedByUser )
            SetRectIn100thmm( m_aExcludingRect );
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( DiagramPositionTabPage, ChangePosXHdl, void *, EMPTYARG )
{
    m_aCurrentRect = GetRectIn100thmm();
    SetMinMaxPosition();
    m_bRectChangedByUser = true;
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( DiagramPositionTabPage, ChangePosYHdl, void *, EMPTYARG )
{
    m_aCurrentRect = GetRectIn100thmm();
    SetMinMaxPosition();
    m_bRectChangedByUser = true;
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( DiagramPositionTabPage, ChangeWidthHdl, void *, EMPTYARG )
{
    if( maCbxScale.IsChecked() && maCbxScale.IsEnabled() )
    {
        sal_Int64 nHeight(basegfx::fround64((mfScaleSizeY * (double)maMtrWidth.GetValue()) / mfScaleSizeX));

        if(nHeight <= maMtrHeight.GetMax(FUNIT_NONE))
        {
            maMtrHeight.SetUserValue(nHeight, FUNIT_NONE);
        }
        else
        {
            nHeight = maMtrHeight.GetMax(FUNIT_NONE);
            maMtrHeight.SetUserValue(nHeight);

            const sal_Int64 nWidth(basegfx::fround64((mfScaleSizeX * (double)nHeight) / mfScaleSizeY));
            maMtrWidth.SetUserValue(nWidth, FUNIT_NONE);
        }
    }
    m_bRectChangedByUser = true;
    m_aCurrentRect = GetRectIn100thmm();
    SetMinMaxPosition();
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( DiagramPositionTabPage, ChangeHeightHdl, void *, EMPTYARG )
{
    if( maCbxScale.IsChecked() && maCbxScale.IsEnabled() )
    {
        sal_Int64 nWidth(basegfx::fround64((mfScaleSizeX * (double)maMtrHeight.GetValue()) / mfScaleSizeY));

        if(nWidth <= maMtrWidth.GetMax(FUNIT_NONE))
        {
            maMtrWidth.SetUserValue(nWidth, FUNIT_NONE);
        }
        else
        {
            nWidth = maMtrWidth.GetMax(FUNIT_NONE);
            maMtrWidth.SetUserValue(nWidth);

            const sal_Int64 nHeight(basegfx::fround64((mfScaleSizeY * (double)nWidth) / mfScaleSizeX));
            maMtrHeight.SetUserValue(nHeight, FUNIT_NONE);
        }
    }
    m_bRectChangedByUser = true;
    m_aCurrentRect = GetRectIn100thmm();
    SetMinMaxPosition();
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( DiagramPositionTabPage, ClickScaleHdl, void *, EMPTYARG )
{
    if( maCbxScale.IsChecked() )
    {
        mfScaleSizeX  = std::max( (double)GetCoreValue( maMtrWidth,  mePoolUnit ), 1.0 );
        mfScaleSizeY = std::max( (double)GetCoreValue( maMtrHeight, mePoolUnit ), 1.0 );
    }
    SetMinMaxPosition();
    return( 0L );
}

//------------------------------------------------------------------------

void DiagramPositionTabPage::FillUserData()
{
    // Abgleich wird in der Ini-Datei festgehalten
    UniString aStr = UniString::CreateFromInt32( (sal_Int32) maCbxScale.IsChecked() );
    SetUserData( aStr );
}

//.............................................................................
} //namespace chart
//.............................................................................
