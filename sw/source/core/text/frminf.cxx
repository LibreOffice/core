/*************************************************************************
 *
 *  $RCSfile: frminf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _PAM_HXX
#include <pam.hxx>          // GetSpaces
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>
#endif
#ifndef _FRMINF_HXX
#include <frminf.hxx>       // SwTxtFrminfo
#endif
#ifndef _ITRTXT_HXX
#include <itrtxt.hxx>       // SwTxtMargin
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>       // IsBullet()
#endif


/*************************************************************************
 *                   SwTxtMargin::GetTxtStart()
 *************************************************************************/

xub_StrLen SwTxtMargin::GetTxtStart() const
{
    const XubString &rTxt = GetInfo().GetTxt();
    const xub_StrLen nPos = nStart;
    const xub_StrLen nEnd = nPos + pCurr->GetLen();
    for( xub_StrLen i = nPos; i < nEnd; ++i )
    {
        const xub_Unicode aChar = rTxt.GetChar( i );
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
    const xub_StrLen nPos = nStart;
    const xub_StrLen nEnd = nPos + pCurr->GetLen();
    for( long i = nEnd - 1; i >= nPos; --i )
    {
        xub_Unicode aChar = rTxt.GetChar( i );
        if( CH_TAB != aChar && CH_BREAK != aChar && ' ' != aChar )
            return i + 1;
    }
    return i + 1;
}

/*************************************************************************
 *                   SwTxtFrmInfo::IsOneLine()
 *************************************************************************/

// Passt der Absatz in eine Zeile?
sal_Bool SwTxtFrmInfo::IsOneLine() const
{
    const SwLineLayout *pLay = pFrm->GetPara();
    if( !pLay )
        return sal_False;
    else
    {
        // 6575: bei Follows natuerlich sal_False
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

// Ist die Zeile zu X% gefuellt?
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

// Wo beginnt der Text (ohne whitespaces)? ( Dokument global )
SwTwips SwTxtFrmInfo::GetLineStart( const SwTxtCursor &rLine ) const
{
    SwTwips nTxtStart = rLine.GetTxtStart();
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

// Wo beginnt der Text (ohne whitespaces)? (rel. im Frame)
SwTwips SwTxtFrmInfo::GetLineStart() const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtCursor aLine( (SwTxtFrm*)pFrm, &aInf );
    return GetLineStart( aLine ) - pFrm->Frm().Left() - pFrm->Prt().Left();
}

// errechne die Position des Zeichens und gebe die Mittelposition zurueck
SwTwips SwTxtFrmInfo::GetCharPos( xub_StrLen nChar, sal_Bool bCenter ) const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtCursor aLine( (SwTxtFrm*)pFrm, &aInf );

    SwTwips nStt, nNext;
    SwRect aRect;
    if( ((SwTxtCursor&)aLine).GetCharRect( &aRect, nChar ) )
        nStt = aRect.Left();
    else
        nStt = aLine.GetLineStart();

    if( !bCenter )
        return nStt - pFrm->Frm().Left();

    if( ((SwTxtCursor&)aLine).GetCharRect( &aRect, nChar+1 ) )
        nNext = aRect.Left();
    else
        nNext = aLine.GetLineStart();

    return (( nNext + nStt ) / 2 ) - pFrm->Frm().Left();
}

/*************************************************************************
 *                   SwTxtFrmInfo::GetSpaces()
 *************************************************************************/

SwPaM *AddPam( SwPaM *pPam, const SwTxtFrm* pTxtFrm,
                const xub_StrLen nPos, const xub_StrLen nLen )
{
    if( nLen )
    {
        // Es koennte auch der erste sein.
        if( pPam->HasMark() )
        {
            // liegt die neue Position genau hinter der aktuellen, dann
            // erweiter den Pam einfach
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

// Sammelt die whitespaces am Zeilenbeginn und -ende im Pam
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
            // Bug 49649: von der ersten Line die Blanks/Tabs NICHT
            //              mit selektieren
            if( !bFirstLine && nPos > aLine.GetStart() )
                pPam = AddPam( pPam, pFrm, aLine.GetStart(),
                                nPos - aLine.GetStart() );

            // Bug 49649: von der letzten Line die Blanks/Tabs NICHT
            //              mit selektieren
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

// Ist an der Textposition ein Bullet/Symbol etc?
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

// Ermittelt Erstzeileneinzug
// Voraussetzung fuer pos. oder neg. EZE ist, dass alle
// Zeilen ausser der ersten Zeile den selben linken Rand haben.
// Wir wollen nicht so knauserig sein und arbeiten mit einer Toleranz
// von TOLERANCE Twips.

#define TOLERANCE 20

SwTwips SwTxtFrmInfo::GetFirstIndent() const
{
    SwTxtSizeInfo aInf( (SwTxtFrm*)pFrm );
    SwTxtCursor aLine( (SwTxtFrm*)pFrm, &aInf );
    const SwTwips nFirst = GetLineStart( aLine );
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

    // Vorerst wird nur +1, -1 und 0 returnt.
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
        // ich bin einzeilig
        SwTxtSizeInfo aNxtInf( (SwTxtFrm*)pNextFrm );
        SwTxtCursor aNxtLine( (SwTxtFrm*)pNextFrm, &aNxtInf );
        nNextIndent = GetLineStart( aNxtLine );
    }
    else
    {
        // ich bin mehrzeilig
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

    // steht vor einem "nicht Space"
    const XubString& rTxt = aInf.GetTxt();
    xub_Unicode aChar = rTxt.GetChar( rFndPos );
    if( CH_TAB == aChar || CH_BREAK == aChar || ' ' == aChar ||
        (( CH_TXTATR_BREAKWORD == aChar || CH_TXTATR_INWORD == aChar ) &&
            aInf.HasHint( rFndPos ) ) )
        return 0;

    // und hinter einem "Space"
    aChar = rTxt.GetChar( rFndPos - 1 );
    if( CH_TAB != aChar && CH_BREAK != aChar &&
        ( ( CH_TXTATR_BREAKWORD != aChar && CH_TXTATR_INWORD != aChar ) ||
            !aInf.HasHint( rFndPos - 1 ) ) &&
        // mehr als 2 Blanks !!
        ( ' ' != aChar || ' ' != rTxt.GetChar( rFndPos - 2 ) ) )
        return 0;

    SwRect aRect;
    return aLine.GetCharRect( &aRect, rFndPos )
            ? KSHORT( aRect.Left() - pFrm->Frm().Left() - pFrm->Prt().Left())
            : 0;
}



