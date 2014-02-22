/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <pam.hxx>
#include <frminf.hxx>
#include <itrtxt.hxx>

/*************************************************************************
 *                   SwTxtMargin::GetTxtStart()
 *************************************************************************/

sal_Int32 SwTxtMargin::GetTxtStart() const
{
    const OUString &rTxt = GetInfo().GetTxt();
    const sal_Int32 nTmpPos = nStart;
    const sal_Int32 nEnd = nTmpPos + pCurr->GetLen();
    sal_Int32 i;

    for( i = nTmpPos; i < nEnd; ++i )
    {
        const sal_Unicode aChar = rTxt[i];
        if( CH_TAB != aChar && ' ' != aChar )
            return i;
    }
    return i;
}

/*************************************************************************
 *                   SwTxtMargin::GetTxtEnd()
 *************************************************************************/

sal_Int32 SwTxtMargin::GetTxtEnd() const
{
    const OUString &rTxt = GetInfo().GetTxt();
    const sal_Int32 nTmpPos = nStart;
    const sal_Int32 nEnd = nTmpPos + pCurr->GetLen();
    sal_Int32 i;
    for( i = nEnd - 1; i >= nTmpPos; --i )
    {
        sal_Unicode aChar = rTxt[i];
        if( CH_TAB != aChar && CH_BREAK != aChar && ' ' != aChar )
            return static_cast<sal_Int32>(i + 1);
    }
    return static_cast<sal_Int32>(i + 1);
}

/*************************************************************************
 *                   SwTxtFrmInfo::IsOneLine()
 *************************************************************************/


sal_Bool SwTxtFrmInfo::IsOneLine() const
{
    const SwLineLayout *pLay = pFrm->GetPara();
    if( !pLay )
        return sal_False;
    else
    {
        
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


SwTwips SwTxtFrmInfo::GetLineStart( const SwTxtCursor &rLine ) const
{
    sal_Int32 nTxtStart = rLine.GetTxtStart();
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


SwTwips SwTxtFrmInfo::GetLineStart() const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtCursor aLine( (SwTxtFrm*)pFrm, &aInf );
    return GetLineStart( aLine ) - pFrm->Frm().Left() - pFrm->Prt().Left();
}


SwTwips SwTxtFrmInfo::GetCharPos( sal_Int32 nChar, sal_Bool bCenter ) const
{
    SWRECTFN( pFrm )
    SwFrmSwapper aSwapper( pFrm, true );

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
                const sal_Int32 nPos, const sal_Int32 nLen )
{
    if( nLen )
    {
        
        if( pPam->HasMark() )
        {
            
            
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


void SwTxtFrmInfo::GetSpaces( SwPaM &rPam, sal_Bool bWithLineBreak ) const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtMargin aLine( (SwTxtFrm*)pFrm, &aInf );
    SwPaM *pPam = &rPam;
    bool bFirstLine = true;
    do {

        if( aLine.GetCurr()->GetLen() )
        {
            sal_Int32 nPos = aLine.GetTxtStart();
            
            
            if( !bFirstLine && nPos > aLine.GetStart() )
                pPam = AddPam( pPam, pFrm, aLine.GetStart(),
                                nPos - aLine.GetStart() );

            
            
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
        bFirstLine = false;
    }
    while( aLine.Next() );
}

/*************************************************************************
 *                   SwTxtFrmInfo::IsBullet()
 *************************************************************************/



sal_Bool SwTxtFrmInfo::IsBullet( sal_Int32 nTxtStart ) const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtMargin aLine( (SwTxtFrm*)pFrm, &aInf );
    aInf.SetIdx( nTxtStart );
    return aLine.IsSymbol( nTxtStart );
}

/*************************************************************************
 *                   SwTxtFrmInfo::GetFirstIndent()
 *************************************************************************/






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

sal_Int32 SwTxtFrmInfo::GetBigIndent( sal_Int32& rFndPos,
                                    const SwTxtFrm *pNextFrm ) const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtCursor aLine( (SwTxtFrm*)pFrm, &aInf );
    SwTwips nNextIndent = 0;

    if( pNextFrm )
    {
        
        SwTxtSizeInfo aNxtInf( (SwTxtFrm*)pNextFrm );
        SwTxtCursor aNxtLine( (SwTxtFrm*)pNextFrm, &aNxtInf );
        nNextIndent = GetLineStart( aNxtLine );
    }
    else
    {
        
        if( aLine.Next() )
        {
            nNextIndent = GetLineStart( aLine );
            aLine.Prev();
        }
    }

    if( nNextIndent <= GetLineStart( aLine ) )
        return 0;

    const Point aPoint( nNextIndent, aLine.Y() );
    rFndPos = aLine.GetCrsrOfst( 0, aPoint, false );
    if( 1 >= rFndPos )
        return 0;

    
    const OUString& rTxt = aInf.GetTxt();
    sal_Unicode aChar = rTxt[rFndPos];
    if( CH_TAB == aChar || CH_BREAK == aChar || ' ' == aChar ||
        (( CH_TXTATR_BREAKWORD == aChar || CH_TXTATR_INWORD == aChar ) &&
            aInf.HasHint( rFndPos ) ) )
        return 0;

    
    aChar = rTxt[rFndPos - 1];
    if( CH_TAB != aChar && CH_BREAK != aChar &&
        ( ( CH_TXTATR_BREAKWORD != aChar && CH_TXTATR_INWORD != aChar ) ||
            !aInf.HasHint( rFndPos - 1 ) ) &&
        
        ( ' ' != aChar || ' ' != rTxt[rFndPos - 2] ) )
        return 0;

    SwRect aRect;
    return aLine.GetCharRect( &aRect, rFndPos )
            ? static_cast<sal_Int32>(aRect.Left() - pFrm->Frm().Left() - pFrm->Prt().Left())
            : 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
