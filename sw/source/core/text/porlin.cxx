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
#include "portxt.hxx"
#include "inftxt.hxx"
#include "pormulti.hxx"
#if OSL_DEBUG_LEVEL > 0

static bool ChkChain( SwLinePortion *pStart )
{
    SwLinePortion *pPor = pStart->GetNextPortion();
    sal_uInt16 nCount = 0;
    while( pPor )
    {
        ++nCount;
        OSL_ENSURE( nCount < 200 && pPor != pStart,
                "ChkChain(): lost in chains" );
        if( nCount >= 200 || pPor == pStart )
        {
            // the lifesaver
            pPor = pStart->GetNextPortion();
            pStart->SetNextPortion(nullptr);
            pPor->Truncate();
            pStart->SetNextPortion( pPor );
            return false;
        }
        pPor = pPor->GetNextPortion();
    }
    return true;
}
#endif

SwLinePortion::~SwLinePortion()
{
}

SwLinePortion *SwLinePortion::Compress()
{
    return GetLen() || Width() ? this : nullptr;
}

SwTwips SwLinePortion::GetViewWidth(const SwTextSizeInfo&) const { return 0; }

SwLinePortion::SwLinePortion( ) :
    mpNextPortion( nullptr ),
    mnLineLength( 0 ),
    mnAscent( 0 ),
    mnHangingBaseline( 0 ),
    mnWhichPor( PortionType::NONE ),
    m_bJoinBorderWithPrev(false),
    m_bJoinBorderWithNext(false)
{
}

void SwLinePortion::PrePaint( const SwTextPaintInfo& rInf,
                              const SwLinePortion* pLast ) const
{
    OSL_ENSURE( rInf.OnWin(), "SwLinePortion::PrePaint: don't prepaint on a printer");
    OSL_ENSURE( !Width(), "SwLinePortion::PrePaint: For Width()==0 only!");

    const SwTwips nViewWidth = GetViewWidth(rInf);

    if( ! nViewWidth )
        return;

    const SwTwips nHalfView = nViewWidth / 2;
    SwTwips nLastWidth = pLast->Width() + pLast->ExtraBlankWidth();

    if ( pLast->InSpaceGrp() && rInf.GetSpaceAdd(/*bShrink=*/true) )
        nLastWidth += pLast->CalcSpacing( rInf.GetSpaceAdd(/*bShrink=*/true), rInf );

    SwTwips nPos;
    SwTextPaintInfo aInf( rInf );

    const bool bBidiPor = rInf.GetTextFrame()->IsRightToLeft() !=
                          bool( vcl::text::ComplexTextLayoutFlags::BiDiRtl & rInf.GetOut()->GetLayoutMode() );

    Degree10 nDir = bBidiPor ?
                  1800_deg10 :
                  rInf.GetFont()->GetOrientation( rInf.GetTextFrame()->IsVertical() );

    // pLast == this *only* for the 1st portion in the line so nLastWidth is 0;
    // allow this too, will paint outside the frame but might look better...
    if (nLastWidth > nHalfView || pLast == this)
    {
        switch (nDir.get())
        {
        case 0:
            nPos = rInf.X();
            nPos += nLastWidth - nHalfView;
            aInf.X( nPos );
            break;
        case 900:
            nPos = rInf.Y();
            nPos -= nLastWidth - nHalfView;
            aInf.Y( nPos );
            break;
        case 1800:
            nPos = rInf.X();
            nPos -= nLastWidth - nHalfView;
            aInf.X( nPos );
            break;
        case 2700:
            nPos = rInf.Y();
            nPos += nLastWidth - nHalfView;
            aInf.Y( nPos );
            break;
        }
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

// all following portions will be deleted
void SwLinePortion::Truncate_()
{
    SwLinePortion *pPos = mpNextPortion;
    do
    {
        OSL_ENSURE( pPos != this, "SwLinePortion::Truncate: loop" );
        SwLinePortion *pLast = pPos;
        pPos = pPos->GetNextPortion();
        pLast->SetNextPortion( nullptr );
        delete pLast;

    } while( pPos );

    mpNextPortion = nullptr;
}

// It always will be inserted after us.
SwLinePortion *SwLinePortion::Insert( SwLinePortion *pIns )
{
    pIns->FindLastPortion()->SetNextPortion( mpNextPortion );
    SetNextPortion( pIns );
#if OSL_DEBUG_LEVEL > 0
    ChkChain( this );
#endif
    return pIns;
}

SwLinePortion *SwLinePortion::FindLastPortion()
{
    SwLinePortion *pPos = this;
    // Find the end and link pLinPortion to the last one...
    while( pPos->GetNextPortion() )
    {
        pPos = pPos->GetNextPortion();
    }
    return pPos;
}

SwLinePortion *SwLinePortion::Append( SwLinePortion *pIns )
{
    SwLinePortion *pPos = FindLastPortion();
    pPos->SetNextPortion( pIns );
    pIns->SetNextPortion( nullptr );
#if OSL_DEBUG_LEVEL > 0
    ChkChain( this );
#endif
    return pIns;
}

SwLinePortion *SwLinePortion::Cut( SwLinePortion *pVictim )
{
    SwLinePortion *pPrev = pVictim->FindPrevPortion( this );
    OSL_ENSURE( pPrev, "SwLinePortion::Cut(): can't cut" );
    // note: if pVictim is a follow then clearing pPrev's m_bHasFollow here is
    // tricky because it could be that the HookChar inserted a tab portion
    // between 2 field portions
    pPrev->SetNextPortion( pVictim->GetNextPortion() );
    pVictim->SetNextPortion(nullptr);
    return pVictim;
}

SwLinePortion *SwLinePortion::FindPrevPortion( const SwLinePortion *pRoot )
{
    OSL_ENSURE( pRoot != this, "SwLinePortion::FindPrevPortion(): invalid root" );
    SwLinePortion *pPos = const_cast<SwLinePortion*>(pRoot);
    while( pPos->GetNextPortion() && pPos->GetNextPortion() != this )
    {
        pPos = pPos->GetNextPortion();
    }
    OSL_ENSURE( pPos->GetNextPortion(),
            "SwLinePortion::FindPrevPortion: blowing in the wind");
    return pPos;
}

TextFrameIndex SwLinePortion::GetModelPositionForViewPoint(const SwTwips nOfst) const
{
    if( nOfst > ( PrtWidth() / 2 ) )
        return GetLen();
    else
        return TextFrameIndex(0);
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
    const SwTwips nNewWidth = rInf.X() + PrtWidth();
    // Only portions with true width can return true
    // Notes for example never set bFull==true
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

void SwLinePortion::Move(SwTextPaintInfo & rInf) const
{
    bool bB2T = rInf.GetDirection() == DIR_BOTTOM2TOP;
    const bool bFrameDir = rInf.GetTextFrame()->IsRightToLeft();
    bool bCounterDir = ( ! bFrameDir && DIR_RIGHT2LEFT == rInf.GetDirection() ) ||
                       (   bFrameDir && DIR_LEFT2RIGHT == rInf.GetDirection() );

    SwTwips nTmp = PrtWidth() + ExtraBlankWidth();
    if ( InSpaceGrp() && rInf.GetSpaceAdd(/*bShrink=*/true) )
    {
        nTmp += CalcSpacing( rInf.GetSpaceAdd(/*bShrink=*/true), rInf );
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
            rInf.Y(rInf.Y() + (bB2T ? -nTmp : nTmp));
        else if ( bCounterDir )
            rInf.X(rInf.X() - nTmp);
        else
            rInf.X(rInf.X() + nTmp);
    }
    if (IsMultiPortion() && static_cast<SwMultiPortion const*>(this)->HasTabulator())
        rInf.IncSpaceIdx();

    rInf.SetIdx( rInf.GetIdx() + GetLen() );
}

SwTwips SwLinePortion::CalcSpacing( tools::Long , const SwTextSizeInfo & ) const
{
    return 0;
}

bool SwLinePortion::GetExpText( const SwTextSizeInfo &, OUString & ) const
{
    return false;
}

void SwLinePortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), OUString(), GetWhichPor() );
}

void SwLinePortion::dumpAsXml(xmlTextWriterPtr pWriter, const OUString& rText, TextFrameIndex& nOffset) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwLinePortion"));
    dumpAsXmlAttributes(pWriter, rText, nOffset);
    nOffset += GetLen();

    (void)xmlTextWriterEndElement(pWriter);
}

void SwLinePortion::dumpAsXmlAttributes(xmlTextWriterPtr pWriter, std::u16string_view rText, TextFrameIndex nOffset) const
{
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("symbol"), BAD_CAST(typeid(*this).name()));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("width"),
        BAD_CAST(OString::number(Width()).getStr()));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("height"),
        BAD_CAST(OString::number(Height()).getStr()));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("length"),
        BAD_CAST(OString::number(static_cast<sal_Int32>(mnLineLength)).getStr()));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("type"),
        BAD_CAST(sw::PortionTypeToString(GetWhichPor())));
    OUString aText( rText.substr(sal_Int32(nOffset), sal_Int32(GetLen())) );
    for (int i = 0; i < 32; ++i)
        aText = aText.replace(i, '*');
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("portion"),
                                      BAD_CAST(aText.toUtf8().getStr()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
