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

#include <swrect.hxx>
#include <paratr.hxx>
#include <viewopt.hxx>
#include <SwPortionHandler.hxx>
#include "porglue.hxx"
#include "inftxt.hxx"
#include "porlay.hxx"
#include "porfly.hxx"
#include <comphelper/string.hxx>

SwGluePortion::SwGluePortion( const sal_uInt16 nInitFixWidth )
    : nFixWidth( nInitFixWidth )
{
    PrtWidth( nFixWidth );
    SetWhichPor( PortionType::Glue );
}

TextFrameIndex SwGluePortion::GetCursorOfst(const sal_uInt16 nOfst) const
{
    // FIXME why nOfst > GetLen() ? is that supposed to be > Width() ?
    if( !GetLen() || nOfst > sal_Int32(GetLen()) || !Width() )
        return SwLinePortion::GetCursorOfst( nOfst );
    else
        return TextFrameIndex(nOfst / (Width() / sal_Int32(GetLen())));
}

SwPosSize SwGluePortion::GetTextSize( const SwTextSizeInfo &rInf ) const
{
    if (TextFrameIndex(1) >= GetLen() || rInf.GetLen() > GetLen() || !Width() || !GetLen())
        return SwPosSize(*this);
    else
        return SwPosSize((Width() / sal_Int32(GetLen())) * sal_Int32(rInf.GetLen()), Height());
}

bool SwGluePortion::GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const
{
    if( GetLen() && rInf.OnWin() &&
        rInf.GetOpt().IsBlank() && rInf.IsNoSymbol() )
    {
        OUStringBuffer aBuf;
        comphelper::string::padToLength(aBuf, sal_Int32(GetLen()), CH_BULLET);
        rText = aBuf.makeStringAndClear();
        return true;
    }
    return false;
}

void SwGluePortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( !GetLen() )
        return;

    if( rInf.GetFont()->IsPaintBlank() )
    {
        OUStringBuffer aBuf;
        comphelper::string::padToLength(aBuf, GetFixWidth() / sal_Int32(GetLen()), ' ');
        OUString aText(aBuf.makeStringAndClear());
        SwTextPaintInfo aInf( rInf, &aText );
        aInf.DrawText(*this, TextFrameIndex(aText.getLength()), true);
    }

    if( rInf.OnWin() && rInf.GetOpt().IsBlank() && rInf.IsNoSymbol() )
    {
#if OSL_DEBUG_LEVEL > 0
        const sal_Unicode cChar = rInf.GetChar( rInf.GetIdx() );
        OSL_ENSURE( CH_BLANK  == cChar || CH_BULLET == cChar,
                "SwGluePortion::Paint: blank expected" );
#endif
        if (TextFrameIndex(1) == GetLen())
        {
            OUString aBullet( CH_BULLET );
            SwPosSize aBulletSize( rInf.GetTextSize( aBullet ) );
            Point aPos( rInf.GetPos() );
            aPos.AdjustX((Width()/2) - (aBulletSize.Width()/2) );
            SwTextPaintInfo aInf( rInf, &aBullet );
            aInf.SetPos( aPos );
            SwTextPortion aBulletPor;
            aBulletPor.Width( aBulletSize.Width() );
            aBulletPor.Height( aBulletSize.Height() );
            aBulletPor.SetAscent( GetAscent() );
            aInf.DrawText(aBulletPor, TextFrameIndex(aBullet.getLength()), true);
        }
        else
        {
            SwTextSlot aSlot( &rInf, this, true, false );
            rInf.DrawText( *this, rInf.GetLen(), true );
        }
    }
}

void SwGluePortion::MoveGlue( SwGluePortion *pTarget, const long nPrtGlue )
{
    auto nPrt = std::min( nPrtGlue, GetPrtGlue() );
    if( 0 < nPrt )
    {
        pTarget->AddPrtWidth( nPrt ); //TODO: overflow
        SubPrtWidth( nPrt ); //TODO: overflow
    }
}

void SwGluePortion::Join( SwGluePortion *pVictim )
{
    // The GluePortion is extracted and flushed away ...
    AddPrtWidth( pVictim->PrtWidth() );
    SetLen( pVictim->GetLen() + GetLen() );
    if( Height() < pVictim->Height() )
        Height( pVictim->Height() );

    AdjFixWidth();
    Cut( pVictim );
    delete pVictim;
}

/**
 * We're expecting a frame-local SwRect!
 */
SwFixPortion::SwFixPortion( const SwRect &rRect )
       :SwGluePortion( sal_uInt16(rRect.Width()) ), nFix( sal_uInt16(rRect.Left()) )
{
    Height( sal_uInt16(rRect.Height()) );
    SetWhichPor( PortionType::Fix );
}

SwFixPortion::SwFixPortion()
       : SwGluePortion(0), nFix(0)
{
    SetWhichPor( PortionType::Fix );
}

SwMarginPortion::SwMarginPortion()
    :SwGluePortion( 0 )
{
    SetWhichPor( PortionType::Margin );
}

/**
 * In the outer loop all portions are inspected - the GluePortions
 * at the end are processed first.
 * The end is shifted forwardly till no more GluePortions remain.
 * Always GluePortion-pairs (pLeft and pRight) are treated, where
 * textportions between pLeft and pRight are moved at the back of
 * pRight if pRight has enough Glue. With every move part of the
 * Glue is transferred from pRight to pLeft.
 * The next loop starts with the processed pLeft as pRight.
 */
void SwMarginPortion::AdjustRight( const SwLineLayout *pCurr )
{
    SwGluePortion *pRight = nullptr;
    bool bNoMove = nullptr != pCurr->GetpKanaComp();
    while( pRight != this )
    {

        // 1) We search for the left Glue
        SwLinePortion *pPos = this;
        SwGluePortion *pLeft = nullptr;
        while( pPos )
        {
            if( pPos->InFixMargGrp() )
                pLeft = static_cast<SwGluePortion*>(pPos);
            pPos = pPos->GetNextPortion();
            if( pPos == pRight)
                pPos = nullptr;
        }

        // Two adjoining FlyPortions are merged
        if( pRight && pLeft && pLeft->GetNextPortion() == pRight )
        {
            pRight->MoveAllGlue( pLeft );
            pRight = nullptr;
        }
        auto nRightGlue = pRight && 0 < pRight->GetPrtGlue()
                          ? pRight->GetPrtGlue() : 0;
        // 2) balance left and right Glue
        //    But not for tabs ...
        if( pLeft && nRightGlue && !pRight->InTabGrp() )
        {
            // pPrev is the portion immediately before pRight
            SwLinePortion *pPrev = pRight->FindPrevPortion( pLeft );

            if ( pRight->IsFlyPortion() && pRight->GetLen() )
            {
                SwFlyPortion *pFly = static_cast<SwFlyPortion *>(pRight);
                if ( pFly->GetBlankWidth() < nRightGlue )
                {
                    // Creating new TextPortion, that takes over the
                    // Blank previously swallowed by the Fly.
                    nRightGlue = nRightGlue - pFly->GetBlankWidth();
                    pFly->SubPrtWidth( pFly->GetBlankWidth() );
                    pFly->SetLen(TextFrameIndex(0));
                    SwTextPortion *pNewPor = new SwTextPortion;
                    pNewPor->SetLen(TextFrameIndex(1));
                    pNewPor->Height( pFly->Height() );
                    pNewPor->Width( pFly->GetBlankWidth() );
                    pFly->Insert( pNewPor );
                }
                else
                    pPrev = pLeft;
            }
            while( pPrev != pLeft )
            {
                if( bNoMove || pPrev->PrtWidth() >= nRightGlue ||
                    pPrev->InHyphGrp() || pPrev->IsKernPortion() )
                {
                    // The portion before the pRight cannot be moved
                    // because no Glue is remaining.
                    // We set the break condition:
                    pPrev = pLeft;
                }
                else
                {
                    nRightGlue = nRightGlue - pPrev->PrtWidth();
                    // pPrev is moved behind pRight. For this the
                    // Glue value between pRight and pLeft gets balanced.
                    pRight->MoveGlue( pLeft, pPrev->PrtWidth() );
                    // Now fix the linking of our portions.
                    SwLinePortion *pPrevPrev = pPrev->FindPrevPortion( pLeft );
                    pPrevPrev->SetNextPortion( pRight );
                    pPrev->SetNextPortion( pRight->GetNextPortion() );
                    pRight->SetNextPortion( pPrev );
                    if ( pPrev->GetNextPortion() && pPrev->InTextGrp()
                         && pPrev->GetNextPortion()->IsHolePortion() )
                    {
                        SwHolePortion *pHolePor =
                            static_cast<SwHolePortion*>(pPrev->GetNextPortion());
                        if ( !pHolePor->GetNextPortion() ||
                             !pHolePor->GetNextPortion()->InFixMargGrp() )
                        {
                            pPrev->AddPrtWidth( pHolePor->GetBlankWidth() );
                            pPrev->SetLen(pPrev->GetLen() + TextFrameIndex(1));
                            pPrev->SetNextPortion( pHolePor->GetNextPortion() );
                            delete pHolePor;
                        }
                    }
                    pPrev = pPrevPrev;
                }
            }
        }
        // If no left Glue remains, we set the break condition.
        pRight = pLeft ? pLeft : this;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
