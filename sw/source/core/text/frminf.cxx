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

#include <pam.hxx>          // GetSpaces
#include <frminf.hxx>       // SwTxtFrminfo
#include <itrtxt.hxx>       // SwTxtMargin

/*************************************************************************
 *                   SwTxtMargin::GetTxtStart()
 *************************************************************************/

xub_StrLen SwTxtMargin::GetTxtStart() const
{
    const XubString &rTxt = GetInfo().GetTxt();
    const xub_StrLen nTmpPos = nStart;
    const xub_StrLen nEnd = nTmpPos + pCurr->GetLen();
    xub_StrLen i;

    for( i = nTmpPos; i < nEnd; ++i )
    {
        const sal_Unicode aChar = rTxt.GetChar( i );
        if( CH_TAB != aChar && ' ' != aChar )
            return i;
    }
    return i;
}

/*************************************************************************
 *                   SwTxtMargin::GetTxtEnd()
 *************************************************************************/

xub_StrLen SwTxtMargin::GetTxtEnd() const
{
    const XubString &rTxt = GetInfo().GetTxt();
    const xub_StrLen nTmpPos = nStart;
    const xub_StrLen nEnd = nTmpPos + pCurr->GetLen();
    long i;
    for( i = nEnd - 1; i >= nTmpPos; --i )
    {
        sal_Unicode aChar = rTxt.GetChar( static_cast<xub_StrLen>(i) );
        if( CH_TAB != aChar && CH_BREAK != aChar && ' ' != aChar )
            return static_cast<xub_StrLen>(i + 1);
    }
    return static_cast<xub_StrLen>(i + 1);
}

/*************************************************************************
 *                   SwTxtFrmInfo::IsOneLine()
 *************************************************************************/

// Does the paragraph fit into one line?
sal_Bool SwTxtFrmInfo::IsOneLine() const
{
    const SwLineLayout *pLay = pFrm->GetPara();
    if( !pLay )
        return sal_False;
    else
    {
        // For follows sal_False of course
        if( pFrm->GetFollow() )
            return sal_False;
        pLay = pLay->GetNext();
        while( pLay )
        {
            if( pLay->GetLen() )
                return sal_False;
            pLay = pLay->GetNext();
        }
        return sal_True;
    }
}

/*************************************************************************
 *                   SwTxtFrmInfo::IsFilled()
 *************************************************************************/

// Is the line filled for X percent?
sal_Bool SwTxtFrmInfo::IsFilled( const sal_uInt8 nPercent ) const
{
    const SwLineLayout *pLay = pFrm->GetPara();
    if( !pLay )
        return sal_False;
    else
    {
        long nWidth = pFrm->Prt().Width();
        nWidth *= nPercent;
        nWidth /= 100;
        return KSHORT(nWidth) <= pLay->Width();
    }
}

/*************************************************************************
 *                   SwTxtFrmInfo::GetLineStart()
 *************************************************************************/

// Where does the text start (without whitespace)? (document global)
SwTwips SwTxtFrmInfo::GetLineStart( const SwTxtCursor &rLine ) const
{
    xub_StrLen nTxtStart = rLine.GetTxtStart();
    SwTwips nStart;
    if( rLine.GetStart() == nTxtStart )
        nStart = rLine.GetLineStart();
    else
    {
        SwRect aRect;
        if( ((SwTxtCursor&)rLine).GetCharRect( &aRect, nTxtStart ) )
            nStart = aRect.Left();
        else
            nStart = rLine.GetLineStart();
    }
    return nStart;
}


/*************************************************************************
 *                   SwTxtFrmInfo::GetLineStart()
 *************************************************************************/

// Where does the text start (without whitespace)? (relative in the Frame)
SwTwips SwTxtFrmInfo::GetLineStart() const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtCursor aLine( (SwTxtFrm*)pFrm, &aInf );
    return GetLineStart( aLine ) - pFrm->Frm().Left() - pFrm->Prt().Left();
}

// Calculates the character's position and returns the middle position
SwTwips SwTxtFrmInfo::GetCharPos( xub_StrLen nChar, sal_Bool bCenter ) const
{
    SWRECTFN( pFrm )
    SwFrmSwapper aSwapper( pFrm, sal_True );

    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtCursor aLine( (SwTxtFrm*)pFrm, &aInf );

    SwTwips nStt, nNext;
    SwRect aRect;
    if( ((SwTxtCursor&)aLine).GetCharRect( &aRect, nChar ) )
    {
        if ( bVert )
            pFrm->SwitchHorizontalToVertical( aRect );

        nStt = (aRect.*fnRect->fnGetLeft)();
    }
    else
        nStt = aLine.GetLineStart();

    if( !bCenter )
        return nStt - (pFrm->Frm().*fnRect->fnGetLeft)();

    if( ((SwTxtCursor&)aLine).GetCharRect( &aRect, nChar+1 ) )
    {
        if ( bVert )
            pFrm->SwitchHorizontalToVertical( aRect );

        nNext = (aRect.*fnRect->fnGetLeft)();
    }
    else
        nNext = aLine.GetLineStart();

    return (( nNext + nStt ) / 2 ) - (pFrm->Frm().*fnRect->fnGetLeft)();
}

/*************************************************************************
 *                   SwTxtFrmInfo::GetSpaces()
 *************************************************************************/

SwPaM *AddPam( SwPaM *pPam, const SwTxtFrm* pTxtFrm,
                const xub_StrLen nPos, const xub_StrLen nLen )
{
    if( nLen )
    {
        // It could be the first
        if( pPam->HasMark() )
        {
            // If the new position is right after the current one, then
            // simply extend the Pam
            if( nPos == pPam->GetPoint()->nContent.GetIndex() )
            {
                pPam->GetPoint()->nContent += nLen;
                return pPam;
            }
            pPam = new SwPaM( *pPam );
        }

        SwIndex &rContent = pPam->GetPoint()->nContent;
        rContent.Assign( (SwTxtNode*)pTxtFrm->GetTxtNode(), nPos );
        pPam->SetMark();
        rContent += nLen;
    }
    return pPam;
}

// Accumulates the whitespace at line start and end in the Pam
void SwTxtFrmInfo::GetSpaces( SwPaM &rPam, sal_Bool bWithLineBreak ) const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtMargin aLine( (SwTxtFrm*)pFrm, &aInf );
    SwPaM *pPam = &rPam;
    sal_Bool bFirstLine = sal_True;
    do {

        if( aLine.GetCurr()->GetLen() )
        {
            xub_StrLen nPos = aLine.GetTxtStart();
            // Do NOT include the blanks/tabs from the first line
            // in the selection
            if( !bFirstLine && nPos > aLine.GetStart() )
                pPam = AddPam( pPam, pFrm, aLine.GetStart(),
                                nPos - aLine.GetStart() );

            // Do NOT include the blanks/tabs from the last line
            // in the selection
            if( aLine.GetNext() )
            {
                nPos = aLine.GetTxtEnd();

                if( nPos < aLine.GetEnd() )
                {
                    MSHORT nOff = !bWithLineBreak && CH_BREAK ==
                                aLine.GetInfo().GetChar( aLine.GetEnd() - 1 )
                                ? 1 : 0;
                    pPam = AddPam( pPam, pFrm, nPos, aLine.GetEnd() - nPos - nOff );
                }
            }
        }
        bFirstLine = sal_False;
    }
    while( aLine.Next() );
}

/*************************************************************************
 *                   SwTxtFrmInfo::IsBullet()
 *************************************************************************/

// Is there a bullet/symbol etc. at the text position?
// Fonts: CharSet, SYMBOL und DONTKNOW
sal_Bool SwTxtFrmInfo::IsBullet( xub_StrLen nTxtStart ) const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtMargin aLine( (SwTxtFrm*)pFrm, &aInf );
    aInf.SetIdx( nTxtStart );
    return aLine.IsSymbol( nTxtStart );
}

/*************************************************************************
 *                   SwTxtFrmInfo::GetFirstIndent()
 *************************************************************************/

// Get first line indent
// The precondition for a positive or negative first line indent:
// All lines (except for the first one) have the same left margin.
// We do not want to be so picky and work with a tolerance of TOLERANCE twips.

SwTwips SwTxtFrmInfo::GetFirstIndent() const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtCursor aLine( (SwTxtFrm*)pFrm, &aInf );
    const SwTwips nFirst = GetLineStart( aLine );
    const SwTwips TOLERANCE = 20;

    if( !aLine.Next() )
        return 0;

    SwTwips nLeft = GetLineStart( aLine );
    while( aLine.Next() )
    {
        if( aLine.GetCurr()->GetLen() )
        {
            const SwTwips nCurrLeft = GetLineStart( aLine );
            if( nLeft + TOLERANCE < nCurrLeft ||
                nLeft - TOLERANCE > nCurrLeft )
                return 0;
        }
    }

    // At first we only return +1, -1 and 0
    if( nLeft == nFirst )
        return 0;
    else
        if( nLeft > nFirst )
            return -1;
        else
            return +1;
}

/*************************************************************************
 *                   SwTxtFrmInfo::GetBigIndent()
 *************************************************************************/

KSHORT SwTxtFrmInfo::GetBigIndent( xub_StrLen& rFndPos,
                                    const SwTxtFrm *pNextFrm ) const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtCursor aLine( (SwTxtFrm*)pFrm, &aInf );
    SwTwips nNextIndent = 0;

    if( pNextFrm )
    {
        // I'm a single line
        SwTxtSizeInfo aNxtInf( (SwTxtFrm*)pNextFrm );
        SwTxtCursor aNxtLine( (SwTxtFrm*)pNextFrm, &aNxtInf );
        nNextIndent = GetLineStart( aNxtLine );
    }
    else
    {
        // I'm multi-line
        if( aLine.Next() )
        {
            nNextIndent = GetLineStart( aLine );
            aLine.Prev();
        }
    }

    if( nNextIndent <= GetLineStart( aLine ) )
        return 0;

    const Point aPoint( nNextIndent, aLine.Y() );
    rFndPos = aLine.GetCrsrOfst( 0, aPoint, sal_False );
    if( 1 >= rFndPos )
        return 0;

    // Is on front of a non-space
    const XubString& rTxt = aInf.GetTxt();
    sal_Unicode aChar = rTxt.GetChar( rFndPos );
    if( CH_TAB == aChar || CH_BREAK == aChar || ' ' == aChar ||
        (( CH_TXTATR_BREAKWORD == aChar || CH_TXTATR_INWORD == aChar ) &&
            aInf.HasHint( rFndPos ) ) )
        return 0;

    // and after a space
    aChar = rTxt.GetChar( rFndPos - 1 );
    if( CH_TAB != aChar && CH_BREAK != aChar &&
        ( ( CH_TXTATR_BREAKWORD != aChar && CH_TXTATR_INWORD != aChar ) ||
            !aInf.HasHint( rFndPos - 1 ) ) &&
        // More than two Blanks!
        ( ' ' != aChar || ' ' != rTxt.GetChar( rFndPos - 2 ) ) )
        return 0;

    SwRect aRect;
    return aLine.GetCharRect( &aRect, rFndPos )
            ? KSHORT( aRect.Left() - pFrm->Frm().Left() - pFrm->Prt().Left())
            : 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
