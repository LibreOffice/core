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

#include <vcl/outdev.hxx>
#include <SwPortionHandler.hxx>

#include "porlin.hxx"
#include "inftxt.hxx"
#include "portxt.hxx"
#include "pormulti.hxx"
#include "porglue.hxx"
#include "blink.hxx"
#if OSL_DEBUG_LEVEL > 0

sal_Bool ChkChain( SwLinePortion *pStart )
{
    SwLinePortion *pPor = pStart->GetPortion();
    MSHORT nCount = 0;
    while( pPor )
    {
        ++nCount;
        OSL_ENSURE( nCount < 200 && pPor != pStart,
                "ChkChain(): lost in chains" );
        if( nCount >= 200 || pPor == pStart )
        {
            // der Lebensretter
            pPor = pStart->GetPortion();
            pStart->SetPortion(0);
            pPor->Truncate();
            pStart->SetPortion( pPor );
            return sal_False;
        }
        pPor = pPor->GetPortion();
    }
    return sal_True;
}
#endif


SwLinePortion::~SwLinePortion()
{
    if( pBlink )
        pBlink->Delete( this );
}

SwLinePortion *SwLinePortion::Compress()
{
    return GetLen() || Width() ? this : 0;
}

KSHORT SwLinePortion::GetViewWidth( const SwTxtSizeInfo & ) const
{
    return 0;
}

/*************************************************************************
 *               SwLinePortion::SwLinePortion( )
 *************************************************************************/

SwLinePortion::SwLinePortion( ) :
    pPortion( NULL ),
    nLineLength( 0 ),
    nAscent( 0 )
{
}

/*************************************************************************
 *               SwLinePortion::PrePaint()
 *************************************************************************/

void SwLinePortion::PrePaint( const SwTxtPaintInfo& rInf,
                              const SwLinePortion* pLast ) const
{
    OSL_ENSURE( rInf.OnWin(), "SwLinePortion::PrePaint: don't prepaint on a printer");
    OSL_ENSURE( !Width(), "SwLinePortion::PrePaint: For Width()==0 only!");

    const KSHORT nViewWidth = GetViewWidth( rInf );

    if( ! nViewWidth )
        return;

    const KSHORT nHalfView = nViewWidth / 2;
    sal_uInt16 nLastWidth = pLast->Width();

    if ( pLast->InSpaceGrp() && rInf.GetSpaceAdd() )
        nLastWidth = nLastWidth + (sal_uInt16)pLast->CalcSpacing( rInf.GetSpaceAdd(), rInf );

    KSHORT nPos;
    SwTxtPaintInfo aInf( rInf );

    const sal_Bool bBidiPor = ( rInf.GetTxtFrm()->IsRightToLeft() ) !=
                          ( 0 != ( TEXT_LAYOUT_BIDI_RTL & rInf.GetOut()->GetLayoutMode() ) );

    sal_uInt16 nDir = bBidiPor ?
                  1800 :
                  rInf.GetFont()->GetOrientation( rInf.GetTxtFrm()->IsVertical() );

    switch ( nDir )
    {
    case 0 :
        nPos = KSHORT( rInf.X() );
        if( nLastWidth > nHalfView )
            nPos += nLastWidth - nHalfView;
        aInf.X( nPos );
        break;
    case 900 :
        nPos = KSHORT( rInf.Y() );
        if( nLastWidth > nHalfView )
            nPos -= nLastWidth + nHalfView;
        aInf.Y( nPos );
        break;
    case 1800 :
        nPos = KSHORT( rInf.X() );
        if( nLastWidth > nHalfView )
            nPos -= nLastWidth + nHalfView;
        aInf.X( nPos );
        break;
    case 2700 :
        nPos = KSHORT( rInf.Y() );
        if( nLastWidth > nHalfView )
            nPos += nLastWidth - nHalfView;
        aInf.Y( nPos );
        break;
    }

    SwLinePortion *pThis = (SwLinePortion*)this;
    pThis->Width( nViewWidth );
    Paint( aInf );
    pThis->Width(0);
}

/*************************************************************************
 *                  SwLinePortion::CalcTxtSize()
 *************************************************************************/

void SwLinePortion::CalcTxtSize( const SwTxtSizeInfo &rInf )
{
    if( GetLen() == rInf.GetLen()  )
        *((SwPosSize*)this) = GetTxtSize( rInf );
    else
    {
        SwTxtSizeInfo aInf( rInf );
        aInf.SetLen( GetLen() );
        *((SwPosSize*)this) = GetTxtSize( aInf );
    }
}

/*************************************************************************
 *                  SwLinePortion::Truncate()
 *
 * Es werden alle nachfolgenden Portions geloescht.
 *************************************************************************/

void SwLinePortion::_Truncate()
{
    SwLinePortion *pPos = pPortion;
    do
    { OSL_ENSURE( pPos != this, "SwLinePortion::Truncate: loop" );
        SwLinePortion *pLast = pPos;
        pPos = pPos->GetPortion();
        pLast->SetPortion( 0 );
        delete pLast;

    } while( pPos );

    pPortion = 0;
}

/*************************************************************************
 *                virtual SwLinePortion::Insert()
 *
 * Es wird immer hinter uns eingefuegt.
 *************************************************************************/

SwLinePortion *SwLinePortion::Insert( SwLinePortion *pIns )
{
    pIns->FindLastPortion()->SetPortion( pPortion );
    SetPortion( pIns );
#if OSL_DEBUG_LEVEL > 0
    ChkChain( this );
#endif
    return pIns;
}

/*************************************************************************
 *                  SwLinePortion::FindLastPortion()
 *************************************************************************/

SwLinePortion *SwLinePortion::FindLastPortion()
{
    SwLinePortion *pPos = this;
    // An das Ende wandern und pLinPortion an den letzten haengen ...
    while( pPos->GetPortion() )
    {
        pPos = pPos->GetPortion();
    }
    return pPos;
}

/*************************************************************************
 *                virtual SwLinePortion::Append()
 *************************************************************************/

SwLinePortion *SwLinePortion::Append( SwLinePortion *pIns )
{
    SwLinePortion *pPos = FindLastPortion();
    pPos->SetPortion( pIns );
    pIns->SetPortion( 0 );
#if OSL_DEBUG_LEVEL > 0
    ChkChain( this );
#endif
    return pIns;
}

/*************************************************************************
 *                virtual SwLinePortion::Cut()
 *************************************************************************/

SwLinePortion *SwLinePortion::Cut( SwLinePortion *pVictim )
{
    SwLinePortion *pPrev = pVictim->FindPrevPortion( this );
    OSL_ENSURE( pPrev, "SwLinePortion::Cut(): can't cut" );
    pPrev->SetPortion( pVictim->GetPortion() );
    pVictim->SetPortion(0);
    return pVictim;
}

/*************************************************************************
 *                SwLinePortion::FindPrevPortion()
 *************************************************************************/

SwLinePortion *SwLinePortion::FindPrevPortion( const SwLinePortion *pRoot )
{
    OSL_ENSURE( pRoot != this, "SwLinePortion::FindPrevPortion(): invalid root" );
    SwLinePortion *pPos = (SwLinePortion*)pRoot;
    while( pPos->GetPortion() && pPos->GetPortion() != this )
    {
        pPos = pPos->GetPortion();
    }
    OSL_ENSURE( pPos->GetPortion(),
            "SwLinePortion::FindPrevPortion: blowing in the wind");
    return pPos;
}

/*************************************************************************
 *                virtual SwLinePortion::GetCrsrOfst()
 *************************************************************************/

xub_StrLen SwLinePortion::GetCrsrOfst( const KSHORT nOfst ) const
{
    if( nOfst > ( PrtWidth() / 2 ) )
        return GetLen();
    else
        return 0;
}

/*************************************************************************
 *                virtual SwLinePortion::GetTxtSize()
 *************************************************************************/

SwPosSize SwLinePortion::GetTxtSize( const SwTxtSizeInfo & ) const
{
    OSL_ENSURE( !this, "SwLinePortion::GetTxtSize: don't ask me about sizes, "
                   "I'm only a stupid SwLinePortion" );
    return SwPosSize();
}

/*************************************************************************
 *                 virtual SwLinePortion::Format()
 *************************************************************************/

sal_Bool SwLinePortion::Format( SwTxtFormatInfo &rInf )
{
    if( rInf.X() > rInf.Width() )
    {
        Truncate();
        rInf.SetUnderFlow( this );
        return sal_True;
    }

    const SwLinePortion *pLast = rInf.GetLast();
    Height( pLast->Height() );
    SetAscent( pLast->GetAscent() );
    const KSHORT nNewWidth = static_cast<sal_uInt16>(rInf.X() + PrtWidth());
    // Nur Portions mit echter Breite koennen ein sal_True zurueckliefern
    // Notizen beispielsweise setzen niemals bFull==sal_True
    if( rInf.Width() <= nNewWidth && PrtWidth() && ! IsKernPortion() )
    {
        Truncate();
        if( nNewWidth > rInf.Width() )
            PrtWidth( nNewWidth - rInf.Width() );
        rInf.GetLast()->FormatEOL( rInf );
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
 *                 virtual SwLinePortion::FormatEOL()
 *************************************************************************/

// Format end of line

void SwLinePortion::FormatEOL( SwTxtFormatInfo & )
{ }

/*************************************************************************
 *                      SwLinePortion::Move()
 *************************************************************************/

void SwLinePortion::Move( SwTxtPaintInfo &rInf )
{
    sal_Bool bB2T = rInf.GetDirection() == DIR_BOTTOM2TOP;
    const sal_Bool bFrmDir = rInf.GetTxtFrm()->IsRightToLeft();
    sal_Bool bCounterDir = ( ! bFrmDir && DIR_RIGHT2LEFT == rInf.GetDirection() ) ||
                       (   bFrmDir && DIR_LEFT2RIGHT == rInf.GetDirection() );

    if ( InSpaceGrp() && rInf.GetSpaceAdd() )
    {
        SwTwips nTmp = PrtWidth() + CalcSpacing( rInf.GetSpaceAdd(), rInf );
        if( rInf.IsRotated() )
            rInf.Y( rInf.Y() + ( bB2T ? -nTmp : nTmp ) );
        else if ( bCounterDir )
            rInf.X( rInf.X() - nTmp );
        else
            rInf.X( rInf.X() + nTmp );
    }
    else
    {
        if( InFixMargGrp() && !IsMarginPortion() )
        {
            rInf.IncSpaceIdx();
            rInf.IncKanaIdx();
        }
        if( rInf.IsRotated() )
            rInf.Y( rInf.Y() + ( bB2T ? -PrtWidth() : PrtWidth() ) );
        else if ( bCounterDir )
            rInf.X( rInf.X() - PrtWidth() );
        else
            rInf.X( rInf.X() + PrtWidth() );
    }
    if( IsMultiPortion() && ((SwMultiPortion*)this)->HasTabulator() )
        rInf.IncSpaceIdx();

    rInf.SetIdx( rInf.GetIdx() + GetLen() );
}

/*************************************************************************
 *              virtual SwLinePortion::CalcSpacing()
 *************************************************************************/

long SwLinePortion::CalcSpacing( long , const SwTxtSizeInfo & ) const
{
    return 0;
}

/*************************************************************************
 *              virtual SwLinePortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwLinePortion::GetExpTxt( const SwTxtSizeInfo &, XubString & ) const
{
    return sal_False;
}

/*************************************************************************
 *              virtual SwLinePortion::HandlePortion()
 *************************************************************************/

void SwLinePortion::HandlePortion( SwPortionHandler& rPH ) const
{
    String aString;
    rPH.Special( GetLen(), aString, GetWhichPor(), Height(), Width() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
