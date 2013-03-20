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

#include "swrect.hxx"
#include "paratr.hxx"   // pTabStop, ADJ*
#include "viewopt.hxx"  // SwViewOptions
#include <SwPortionHandler.hxx>
#include "porglue.hxx"
#include "inftxt.hxx"
#include "porlay.hxx"   // SwParaPortion, SetFull
#include "porfly.hxx"   // SwParaPortion, SetFull
#include <comphelper/string.hxx>

/*************************************************************************
 *                      class SwGluePortion
 *************************************************************************/

SwGluePortion::SwGluePortion( const KSHORT nInitFixWidth )
    : nFixWidth( nInitFixWidth )
{
    PrtWidth( nFixWidth );
    SetWhichPor( POR_GLUE );
}

/*************************************************************************
 *                virtual SwGluePortion::GetCrsrOfst()
 *************************************************************************/

xub_StrLen SwGluePortion::GetCrsrOfst( const KSHORT nOfst ) const
{
    if( !GetLen() || nOfst > GetLen() || !Width() )
        return SwLinePortion::GetCrsrOfst( nOfst );
    else
        return nOfst / (Width() / GetLen());
}

/*************************************************************************
 *                virtual SwGluePortion::GetTxtSize()
 *************************************************************************/

SwPosSize SwGluePortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
{
    if( 1 >= GetLen() || rInf.GetLen() > GetLen() || !Width() || !GetLen() )
        return SwPosSize(*this);
    else
        return SwPosSize( (Width() / GetLen()) * rInf.GetLen(), Height() );
}

/*************************************************************************
 *              virtual SwGluePortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwGluePortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
{
    if( GetLen() && rInf.OnWin() &&
        rInf.GetOpt().IsBlank() && rInf.IsNoSymbol() )
    {
        OUStringBuffer aBuf;
        comphelper::string::padToLength(aBuf, GetLen(), CH_BULLET);
        rTxt = aBuf.makeStringAndClear();
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
 *                virtual SwGluePortion::Paint()
 *************************************************************************/

void SwGluePortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( !GetLen() )
        return;

    if( rInf.GetFont()->IsPaintBlank() )
    {
        OUStringBuffer aBuf;
        comphelper::string::padToLength(aBuf, GetFixWidth() / GetLen(), ' ');
        String aTxt(aBuf.makeStringAndClear());
        SwTxtPaintInfo aInf( rInf, aTxt );
        aInf.DrawText( *this, aTxt.Len(), sal_True );
    }

    if( rInf.OnWin() && rInf.GetOpt().IsBlank() && rInf.IsNoSymbol() )
    {
#if OSL_DEBUG_LEVEL > 0
        const sal_Unicode cChar = rInf.GetChar( rInf.GetIdx() );
        OSL_ENSURE( CH_BLANK  == cChar || CH_BULLET == cChar,
                "SwGluePortion::Paint: blank expected" );
#endif
        if( 1 == GetLen() )
        {
            OUString aBullet( CH_BULLET );
            SwPosSize aBulletSize( rInf.GetTxtSize( aBullet ) );
            Point aPos( rInf.GetPos() );
            aPos.X() += (Width()/2) - (aBulletSize.Width()/2);
            SwTxtPaintInfo aInf( rInf, aBullet );
            aInf.SetPos( aPos );
            SwTxtPortion aBulletPor;
            aBulletPor.Width( aBulletSize.Width() );
            aBulletPor.Height( aBulletSize.Height() );
            aBulletPor.SetAscent( GetAscent() );
            aInf.DrawText( aBulletPor, aBullet.getLength(), sal_True );
        }
        else
        {
            SwTxtSlot aSlot( &rInf, this, true, false );
            rInf.DrawText( *this, rInf.GetLen(), sal_True );
        }
    }
}

/*************************************************************************
 *                      SwGluePortion::MoveGlue()
 *************************************************************************/

void SwGluePortion::MoveGlue( SwGluePortion *pTarget, const short nPrtGlue )
{
    short nPrt = Min( nPrtGlue, GetPrtGlue() );
    if( 0 < nPrt )
    {
        pTarget->AddPrtWidth( nPrt );
        SubPrtWidth( nPrt );
    }
}

/*************************************************************************
 *                void SwGluePortion::Join()
 *************************************************************************/

void SwGluePortion::Join( SwGluePortion *pVictim )
{
    // Die GluePortion wird ausgesogen und weggespuelt ...
    AddPrtWidth( pVictim->PrtWidth() );
    SetLen( pVictim->GetLen() + GetLen() );
    if( Height() < pVictim->Height() )
        Height( pVictim->Height() );

    AdjFixWidth();
    Cut( pVictim );
    delete pVictim;
}

/*************************************************************************
 *                class SwFixPortion
 *************************************************************************/

// Wir erwarten ein framelokales SwRect !
SwFixPortion::SwFixPortion( const SwRect &rRect )
       :SwGluePortion( KSHORT(rRect.Width()) ), nFix( KSHORT(rRect.Left()) )
{
    Height( KSHORT(rRect.Height()) );
    SetWhichPor( POR_FIX );
}

SwFixPortion::SwFixPortion(const KSHORT nFixedWidth, const KSHORT nFixedPos)
       : SwGluePortion(nFixedWidth), nFix(nFixedPos)
{
    SetWhichPor( POR_FIX );
}

/*************************************************************************
 *                class SwMarginPortion
 *************************************************************************/

SwMarginPortion::SwMarginPortion( const KSHORT nFixedWidth )
    :SwGluePortion( nFixedWidth )
{
    SetWhichPor( POR_MARGIN );
}

/*************************************************************************
 *                SwMarginPortion::AdjustRight()
 *
 * In the outer loop all portions are inspected - the GluePortions
 * at the end are processed first.
 * The end is shifted forwardly till no more GluePortions remain.
 * Always GluePortion-pairs (pLeft and pRight) are treated, where
 * textportions between pLeft and pRight are moved at the back of
 * pRight if pRight has enough Glue. With every move part of the
 * Glue is transferred from pRight to pLeft.
 * The next loop starts with the processed pLeft as pRight.
 *************************************************************************/

void SwMarginPortion::AdjustRight( const SwLineLayout *pCurr )
{
    SwGluePortion *pRight = 0;
    bool bNoMove = 0 != pCurr->GetpKanaComp();
    while( pRight != this )
    {

        // 1) We search for the left Glue
        SwLinePortion *pPos = (SwLinePortion*)this;
        SwGluePortion *pLeft = 0;
        while( pPos )
        {
            if( pPos->InFixMargGrp() )
                pLeft = (SwGluePortion*)pPos;
            pPos = pPos->GetPortion();
            if( pPos == pRight)
                pPos = 0;
        }

        // Two adjoining FlyPortions are merged
        if( pRight && pLeft && pLeft->GetPortion() == pRight )
        {
            pRight->MoveAllGlue( pLeft );
            pRight = 0;
        }
        KSHORT nRightGlue = pRight && 0 < pRight->GetPrtGlue()
                          ? KSHORT(pRight->GetPrtGlue()) : 0;
        // 2) balance left and right Glue
        //    But not for tabs ...
        if( pLeft && nRightGlue && !pRight->InTabGrp() )
        {
            // pPrev is the portion immediately before pRight
            SwLinePortion *pPrev = pRight->FindPrevPortion( pLeft );

            if ( pRight->IsFlyPortion() && pRight->GetLen() )
            {
                SwFlyPortion *pFly = (SwFlyPortion *)pRight;
                if ( pFly->GetBlankWidth() < nRightGlue )
                {
                    // Creating new TxtPortion, that takes over the
                    // Blank previously swallowed by the Fly.
                    nRightGlue = nRightGlue - pFly->GetBlankWidth();
                    pFly->SubPrtWidth( pFly->GetBlankWidth() );
                    pFly->SetLen( 0 );
                    SwTxtPortion *pNewPor = new SwTxtPortion;
                    pNewPor->SetLen( 1 );
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
                    pRight->MoveGlue( pLeft, short( pPrev->PrtWidth() ) );
                    // Now fix the linking of our portions.
                    SwLinePortion *pPrevPrev = pPrev->FindPrevPortion( pLeft );
                    pPrevPrev->SetPortion( pRight );
                    pPrev->SetPortion( pRight->GetPortion() );
                    pRight->SetPortion( pPrev );
                    if ( pPrev->GetPortion() && pPrev->InTxtGrp()
                         && pPrev->GetPortion()->IsHolePortion() )
                    {
                        SwHolePortion *pHolePor =
                            (SwHolePortion*)pPrev->GetPortion();
                        if ( !pHolePor->GetPortion() ||
                             !pHolePor->GetPortion()->InFixMargGrp() )
                        {
                            pPrev->AddPrtWidth( pHolePor->GetBlankWidth() );
                            pPrev->SetLen( pPrev->GetLen() + 1 );
                            pPrev->SetPortion( pHolePor->GetPortion() );
                            delete pHolePor;
                        }
                    }
                    pPrev = pPrevPrev;
                }
            }
        }
        // If no left Glue remaines we set the break condition.
        pRight = pLeft ? pLeft : (SwGluePortion*)this;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
