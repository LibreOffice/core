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

#include <vcl/outdev.hxx>
#include <SwPortionHandler.hxx>

#include "porlin.hxx"
#include "inftxt.hxx"
#include "portxt.hxx"
#include "pormulti.hxx"
#include "porglue.hxx"
#include "blink.hxx"
#if OSL_DEBUG_LEVEL > 0

bool ChkChain( SwLinePortion *pStart )
{
    SwLinePortion *pPor = pStart->GetPortion();
    sal_uInt16 nCount = 0;
    while( pPor )
    {
        ++nCount;
        OSL_ENSURE( nCount < 200 && pPor != pStart,
                "ChkChain(): lost in chains" );
        if( nCount >= 200 || pPor == pStart )
        {
            // der Lebensretter
            pPor = pStart->GetPortion();
            pStart->SetPortion(nullptr);
            pPor->Truncate();
            pStart->SetPortion( pPor );
            return false;
        }
        pPor = pPor->GetPortion();
    }
    return true;
}
#endif

SwLinePortion::~SwLinePortion()
{
    if( pBlink )
        pBlink->Delete( this );
}

SwLinePortion *SwLinePortion::Compress()
{
    return GetLen() || Width() ? this : nullptr;
}

sal_uInt16 SwLinePortion::GetViewWidth( const SwTextSizeInfo & ) const
{
    return 0;
}

SwLinePortion::SwLinePortion( ) :
    pPortion( nullptr ),
    nLineLength( 0 ),
    nAscent( 0 ),
    nWhichPor( POR_LIN ),
    m_bJoinBorderWithPrev(false),
    m_bJoinBorderWithNext(false)
{
}

void SwLinePortion::PrePaint( const SwTextPaintInfo& rInf,
                              const SwLinePortion* pLast ) const
{
    OSL_ENSURE( rInf.OnWin(), "SwLinePortion::PrePaint: don't prepaint on a printer");
    OSL_ENSURE( !Width(), "SwLinePortion::PrePaint: For Width()==0 only!");

    const sal_uInt16 nViewWidth = GetViewWidth( rInf );

    if( ! nViewWidth )
        return;

    const sal_uInt16 nHalfView = nViewWidth / 2;
    sal_uInt16 nLastWidth = pLast->Width();

    if ( pLast->InSpaceGrp() && rInf.GetSpaceAdd() )
        nLastWidth = nLastWidth + (sal_uInt16)pLast->CalcSpacing( rInf.GetSpaceAdd(), rInf );

    sal_uInt16 nPos;
    SwTextPaintInfo aInf( rInf );

    const bool bBidiPor = rInf.GetTextFrame()->IsRightToLeft() !=
                          bool( TEXT_LAYOUT_BIDI_RTL & rInf.GetOut()->GetLayoutMode() );

    sal_uInt16 nDir = bBidiPor ?
                  1800 :
                  rInf.GetFont()->GetOrientation( rInf.GetTextFrame()->IsVertical() );

    switch ( nDir )
    {
    case 0 :
        nPos = sal_uInt16( rInf.X() );
        if( nLastWidth > nHalfView )
            nPos += nLastWidth - nHalfView;
        aInf.X( nPos );
        break;
    case 900 :
        nPos = sal_uInt16( rInf.Y() );
        if( nLastWidth > nHalfView )
            nPos -= nLastWidth + nHalfView;
        aInf.Y( nPos );
        break;
    case 1800 :
        nPos = sal_uInt16( rInf.X() );
        if( nLastWidth > nHalfView )
            nPos -= nLastWidth + nHalfView;
        aInf.X( nPos );
        break;
    case 2700 :
        nPos = sal_uInt16( rInf.Y() );
        if( nLastWidth > nHalfView )
            nPos += nLastWidth - nHalfView;
        aInf.Y( nPos );
        break;
    }

    SwLinePortion *pThis = const_cast<SwLinePortion*>(this);
    pThis->Width( nViewWidth );
    Paint( aInf );
    pThis->Width(0);
}

void SwLinePortion::CalcTextSize( const SwTextSizeInfo &rInf )
{
    if( GetLen() == rInf.GetLen()  )
        *static_cast<SwPosSize*>(this) = GetTextSize( rInf );
    else
    {
        SwTextSizeInfo aInf( rInf );
        aInf.SetLen( GetLen() );
        *static_cast<SwPosSize*>(this) = GetTextSize( aInf );
    }
}

// Es werden alle nachfolgenden Portions geloescht.
void SwLinePortion::_Truncate()
{
    SwLinePortion *pPos = pPortion;
    do
    { OSL_ENSURE( pPos != this, "SwLinePortion::Truncate: loop" );
        SwLinePortion *pLast = pPos;
        pPos = pPos->GetPortion();
        pLast->SetPortion( nullptr );
        delete pLast;

    } while( pPos );

    pPortion = nullptr;
}

// Es wird immer hinter uns eingefuegt.
SwLinePortion *SwLinePortion::Insert( SwLinePortion *pIns )
{
    pIns->FindLastPortion()->SetPortion( pPortion );
    SetPortion( pIns );
#if OSL_DEBUG_LEVEL > 0
    ChkChain( this );
#endif
    return pIns;
}

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

SwLinePortion *SwLinePortion::Append( SwLinePortion *pIns )
{
    SwLinePortion *pPos = FindLastPortion();
    pPos->SetPortion( pIns );
    pIns->SetPortion( nullptr );
#if OSL_DEBUG_LEVEL > 0
    ChkChain( this );
#endif
    return pIns;
}

SwLinePortion *SwLinePortion::Cut( SwLinePortion *pVictim )
{
    SwLinePortion *pPrev = pVictim->FindPrevPortion( this );
    OSL_ENSURE( pPrev, "SwLinePortion::Cut(): can't cut" );
    pPrev->SetPortion( pVictim->GetPortion() );
    pVictim->SetPortion(nullptr);
    return pVictim;
}

SwLinePortion *SwLinePortion::FindPrevPortion( const SwLinePortion *pRoot )
{
    OSL_ENSURE( pRoot != this, "SwLinePortion::FindPrevPortion(): invalid root" );
    SwLinePortion *pPos = const_cast<SwLinePortion*>(pRoot);
    while( pPos->GetPortion() && pPos->GetPortion() != this )
    {
        pPos = pPos->GetPortion();
    }
    OSL_ENSURE( pPos->GetPortion(),
            "SwLinePortion::FindPrevPortion: blowing in the wind");
    return pPos;
}

sal_Int32 SwLinePortion::GetCursorOfst( const sal_uInt16 nOfst ) const
{
    if( nOfst > ( PrtWidth() / 2 ) )
        return GetLen();
    else
        return 0;
}

SwPosSize SwLinePortion::GetTextSize( const SwTextSizeInfo & ) const
{
    OSL_ENSURE( false, "SwLinePortion::GetTextSize: don't ask me about sizes, "
                   "I'm only a stupid SwLinePortion" );
    return SwPosSize();
}

bool SwLinePortion::Format( SwTextFormatInfo &rInf )
{
    if( rInf.X() > rInf.Width() )
    {
        Truncate();
        rInf.SetUnderflow( this );
        return true;
    }

    const SwLinePortion *pLast = rInf.GetLast();
    Height( pLast->Height() );
    SetAscent( pLast->GetAscent() );
    const sal_uInt16 nNewWidth = static_cast<sal_uInt16>(rInf.X() + PrtWidth());
    // Nur Portions mit echter Breite koennen ein true zurueckliefern
    // Notizen beispielsweise setzen niemals bFull==true
    if( rInf.Width() <= nNewWidth && PrtWidth() && ! IsKernPortion() )
    {
        Truncate();
        if( nNewWidth > rInf.Width() )
            PrtWidth( nNewWidth - rInf.Width() );
        rInf.GetLast()->FormatEOL( rInf );
        return true;
    }
    return false;
}

// Format end of line

void SwLinePortion::FormatEOL( SwTextFormatInfo & )
{ }

void SwLinePortion::Move( SwTextPaintInfo &rInf )
{
    bool bB2T = rInf.GetDirection() == DIR_BOTTOM2TOP;
    const bool bFrameDir = rInf.GetTextFrame()->IsRightToLeft();
    bool bCounterDir = ( ! bFrameDir && DIR_RIGHT2LEFT == rInf.GetDirection() ) ||
                       (   bFrameDir && DIR_LEFT2RIGHT == rInf.GetDirection() );

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
    if( IsMultiPortion() && static_cast<SwMultiPortion*>(this)->HasTabulator() )
        rInf.IncSpaceIdx();

    rInf.SetIdx( rInf.GetIdx() + GetLen() );
}

long SwLinePortion::CalcSpacing( long , const SwTextSizeInfo & ) const
{
    return 0;
}

bool SwLinePortion::GetExpText( const SwTextSizeInfo &, OUString & ) const
{
    return false;
}

void SwLinePortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), OUString(), GetWhichPor(), Height(), Width() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
