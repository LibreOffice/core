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

#include <hintids.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <EnhancedPDFExportHelper.hxx>
#include <viewopt.hxx>  // SwViewOptions
#include <viewsh.hxx>
#include <SwPortionHandler.hxx>
#include <porhyph.hxx>  //
#include <inftxt.hxx>
#include <itrform2.hxx> //
#include <guess.hxx>    //
#include <splargs.hxx>  // SwInterHyphInfo

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;

/*************************************************************************
 *                      SwTxtFormatInfo::HyphWord()
 *************************************************************************/

Reference< XHyphenatedWord >  SwTxtFormatInfo::HyphWord(
                                const OUString &rTxt, const MSHORT nMinTrail )
{
    if( rTxt.getLength() < 4 || m_pFnt->IsSymbol(m_pVsh) )
        return 0;
    Reference< XHyphenator >  xHyph = ::GetHyphenator();
    Reference< XHyphenatedWord > xHyphWord;

    if( xHyph.is() )
        xHyphWord = xHyph->hyphenate( OUString(rTxt),
                            g_pBreakIt->GetLocale( m_pFnt->GetLanguage() ),
                            rTxt.getLength() - nMinTrail, GetHyphValues() );
    return xHyphWord;

}

/*************************************************************************
 *                      SwTxtFrm::Hyphenate
 *
 * Wir formatieren eine Zeile fuer die interaktive Trennung
 *************************************************************************/

sal_Bool SwTxtFrm::Hyphenate( SwInterHyphInfo &rHyphInf )
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),"swapped frame at SwTxtFrm::Hyphenate" );

    if( !g_pBreakIt->GetBreakIter().is() )
        return sal_False;
    // Wir machen den Laden erstmal dicht:
    OSL_ENSURE( !IsLocked(), "SwTxtFrm::Hyphenate: this is locked" );
    // 4935: Der frame::Frame muss eine gueltige SSize haben!
    Calc();
    GetFormatted();

    sal_Bool bRet = sal_False;
    if( !IsEmpty() )
    {
        // Wir muessen die Trennung immer einschalten.
        // Keine Angst, der SwTxtIter sichert im Hyphenate die alte Zeile.
        SwTxtFrmLocker aLock( this );

        if ( IsVertical() )
            SwapWidthAndHeight();

        SwTxtFormatInfo aInf( this, sal_True );     // sal_True fuer interactive hyph!
        SwTxtFormatter aLine( this, &aInf );
        aLine.CharToLine( rHyphInf.nStart );
        // Wenn wir innerhalb des ersten Wortes einer Zeile stehen, so koennte
        // dieses in der vorherigen getrennt werden, deshalb gehen wir ein Zeile
        // zurueck.
        if( aLine.Prev() )
        {
            SwLinePortion *pPor = aLine.GetCurr()->GetFirstPortion();
            while( pPor->GetPortion() )
                pPor = pPor->GetPortion();
            if( pPor->GetWhichPor() == POR_SOFTHYPH ||
                pPor->GetWhichPor() == POR_SOFTHYPHSTR )
                aLine.Next();
        }

        const xub_StrLen nEnd = rHyphInf.GetEnd();
        while( !bRet && aLine.GetStart() < nEnd )
        {
            bRet = aLine.Hyphenate( rHyphInf );
            if( !aLine.Next() )
                break;
        }

        if ( IsVertical() )
            SwapWidthAndHeight();
    }
    return bRet;
}

/*************************************************************************
 *                      SwTxtFormatter::Hyphenate
 *
 * Wir formatieren eine Zeile fuer die interaktive Trennung
 *************************************************************************/
// Wir koennen davon ausgehen, dass bereits formatiert wurde.
// Fuer die CeBIT'93 gehen wir den einfachen, sicheren Weg:
// Die Zeile wird einfach neu formatiert, der Hyphenator wird dann
// so vorbereitet, wie ihn die UI erwartet.
// Hier stehen natuerlich enorme Optimierungsmoeglichkeiten offen.

void SetParaPortion( SwTxtInfo *pInf, SwParaPortion *pRoot )
{
    OSL_ENSURE( pRoot, "SetParaPortion: no root anymore" );
    pInf->pPara = pRoot;
}

sal_Bool SwTxtFormatter::Hyphenate( SwInterHyphInfo &rHyphInf )
{
    SwTxtFormatInfo &rInf = GetInfo();
    sal_Bool bRet = sal_False;

    // In der letzten Zeile gibt es nie etwas zu trennen.
    // Es sei denn, es befindet sich eine FlyPortion darin,
    // oder es ist die letzte Zeile des Masters
    if( !GetNext() && !rInf.GetTxtFly()->IsOn() && !pFrm->GetFollow() )
        return bRet;

    xub_StrLen nWrdStart = nStart;

    // Wir muessen die alte Zeile erhalten. Ein Beispiel:
    // Das Attribut fuer Trennung wurde nicht gesetzt,
    // in SwTxtFrm::Hyphenate wird es jedoch immer gesetzt,
    // weil wir Trennpositionen im Hyphenator einstellen wollen.
    SwLineLayout *pOldCurr = pCurr;

    InitCntHyph();

    // 5298: IsParaLine() (ex.IsFirstLine) fragt auf GetParaPortion() ab.
    // wir muessen gleiche Bedingungen schaffen: in der ersten
    // Zeile formatieren wir SwParaPortions...
    if( pOldCurr->IsParaPortion() )
    {
        SwParaPortion *pPara = new SwParaPortion();
        SetParaPortion( &rInf, pPara );
        pCurr = pPara;
        OSL_ENSURE( IsParaLine(), "SwTxtFormatter::Hyphenate: not the first" );
    }
    else
        pCurr = new SwLineLayout();

    nWrdStart = FormatLine( nWrdStart );

    // Man muss immer im Hinterkopf behalten, dass es z.B.
    // Felder gibt, die aufgetrennt werden koennen ...
    if( pCurr->PrtWidth() && pCurr->GetLen() )
    {
        // Wir muessen uns darauf einstellen, dass in der Zeile
        // FlyFrms haengen, an denen auch umgebrochen werden darf.
        // Wir suchen also die erste HyphPortion in dem angegebenen
        // Bereich.

        SwLinePortion *pPos = pCurr->GetPortion();
        const xub_StrLen nPamStart = rHyphInf.nStart;
        nWrdStart = nStart;
        const xub_StrLen nEnd = rHyphInf.GetEnd();
        while( pPos )
        {
            // Entweder wir liegen drueber oder wir laufen gerade auf eine
            // Hyphportion die am Ende der Zeile oder vor einem Flys steht.
            if( nWrdStart >= nEnd )
            {
                nWrdStart = 0;
                break;
            }

            if( nWrdStart >= nPamStart && pPos->InHyphGrp()
                && ( !pPos->IsSoftHyphPortion()
                     || ((SwSoftHyphPortion*)pPos)->IsExpand() ) )
            {
                nWrdStart = nWrdStart + pPos->GetLen();
                break;
            }

            nWrdStart = nWrdStart + pPos->GetLen();
            pPos = pPos->GetPortion();
        }
        // Wenn pPos 0 ist, wurde keine Trennstelle ermittelt.
        if( !pPos )
            nWrdStart = 0;
    }

    // Das alte LineLayout wird wieder eingestellt ...
    delete pCurr;
    pCurr = pOldCurr;

    if( pOldCurr->IsParaPortion() )
    {
        SetParaPortion( &rInf, (SwParaPortion*)pOldCurr );
        OSL_ENSURE( IsParaLine(), "SwTxtFormatter::Hyphenate: even not the first" );
    }

    if( nWrdStart )
    {
        // nWrdStart bezeichnet nun die Position im String, der
        // fuer eine Trennung zur Debatte steht.
        // Start() hangelt sich zum End()
        rHyphInf.nWordStart = nWrdStart;

        xub_StrLen nLen = 0;
        const xub_StrLen nEnd = nWrdStart;

        // Wir suchen vorwaerts
        Reference< XHyphenatedWord > xHyphWord;

        Boundary aBound =
            g_pBreakIt->GetBreakIter()->getWordBoundary( rInf.GetTxt(), nWrdStart,
            g_pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ), WordType::DICTIONARY_WORD, sal_True );
        nWrdStart = static_cast<xub_StrLen>(aBound.startPos);
        nLen = static_cast<xub_StrLen>(aBound.endPos - nWrdStart);
        bRet = 0 != nLen;
        if( bRet )
        {
            OUString aSelTxt( rInf.GetTxt().copy(nWrdStart, nLen) );

            {
                MSHORT nMinTrail = 0;
                if( nWrdStart + nLen > nEnd )
                    nMinTrail = nWrdStart + nLen - nEnd - 1;

                //!! rHyphInf.SetHyphWord( ... ) mu??? hier geschehen
                xHyphWord = rInf.HyphWord( aSelTxt, nMinTrail );
                bRet = xHyphWord.is();
                if ( !rHyphInf.IsCheck() && sal_False == bRet )
                    rHyphInf.SetNoLang( sal_True );
            }

            if( bRet )
            {
                rHyphInf.SetHyphWord( xHyphWord );
                rHyphInf.nWordStart = nWrdStart;
                rHyphInf.nWordLen = nLen;
                rHyphInf.SetNoLang( sal_False );
                rHyphInf.SetCheck( sal_True );
            }
        }
    }
    return bRet;
}

/*************************************************************************
 *                      SwTxtPortion::CreateHyphen()
 *************************************************************************/

sal_Bool SwTxtPortion::CreateHyphen( SwTxtFormatInfo &rInf, SwTxtGuess &rGuess )
{
    Reference< XHyphenatedWord >  xHyphWord = rGuess.HyphWord();

    OSL_ENSURE( !pPortion, "SwTxtPortion::CreateHyphen(): another portion, another planet..." );
    OSL_ENSURE( xHyphWord.is(), "SwTxtPortion::CreateHyphen(): You are lucky! The code is robust here." );

    if( rInf.IsHyphForbud() ||
        pPortion || // robust
        !xHyphWord.is() || // more robust
        // Mehrzeilige Felder duerfen nicht interaktiv getrennt werden.
        ( rInf.IsInterHyph() && InFldGrp() ) )
        return sal_False;

    SwHyphPortion *pHyphPor;
    xub_StrLen nPorEnd;
    SwTxtSizeInfo aInf( rInf );

    // first case: hyphenated word has alternative spelling
    if ( xHyphWord->isAlternativeSpelling() )
    {
        SvxAlternativeSpelling aAltSpell;
        aAltSpell = SvxGetAltSpelling( xHyphWord );
        OSL_ENSURE( aAltSpell.bIsAltSpelling, "no alternatve spelling" );

        OUString aAltTxt = aAltSpell.aReplacement;
        nPorEnd = aAltSpell.nChangedPos + rGuess.BreakStart() - rGuess.FieldDiff();
        xub_StrLen nTmpLen = 0;

        // soft hyphen at alternative spelling position?
        if( rInf.GetTxt()[ rInf.GetSoftHyphPos() ] == CHAR_SOFTHYPHEN )
        {
            pHyphPor = new SwSoftHyphStrPortion( aAltTxt );
            nTmpLen = 1;
        }
        else {
            pHyphPor = new SwHyphStrPortion( aAltTxt );
        }

        // length of pHyphPor is adjusted
        pHyphPor->SetLen( aAltTxt.getLength() + 1 );
        (SwPosSize&)(*pHyphPor) = pHyphPor->GetTxtSize( rInf );
        pHyphPor->SetLen( aAltSpell.nChangedLength + nTmpLen );
    }
    else
    {
        // second case: no alternative spelling
        SwHyphPortion aHyphPor;
        aHyphPor.SetLen( 1 );

        static const void* pLastMagicNo = 0;
        static KSHORT aMiniCacheH = 0, aMiniCacheW = 0;
        const void* pTmpMagic;
        MSHORT nFntIdx;
        rInf.GetFont()->GetMagic( pTmpMagic, nFntIdx, rInf.GetFont()->GetActual() );
        if( !pLastMagicNo || pLastMagicNo != pTmpMagic ) {
            pLastMagicNo = pTmpMagic;
            (SwPosSize&)aHyphPor = aHyphPor.GetTxtSize( rInf );
            aMiniCacheH = aHyphPor.Height(), aMiniCacheW = aHyphPor.Width();
        } else {
            aHyphPor.Height( aMiniCacheH ), aHyphPor.Width( aMiniCacheW );
        }
        aHyphPor.SetLen( 0 );
        pHyphPor = new SwHyphPortion( aHyphPor );

        pHyphPor->SetWhichPor( POR_HYPH );

        // values required for this
        nPorEnd = xHyphWord->getHyphenPos() + 1 + rGuess.BreakStart()
                - rGuess.FieldDiff();
    }

    // portion end must be in front of us
    // we do not put hyphens at start of line
    if ( nPorEnd > rInf.GetIdx() ||
         ( nPorEnd == rInf.GetIdx() && rInf.GetLineStart() != rInf.GetIdx() ) )
    {
        aInf.SetLen( nPorEnd - rInf.GetIdx() );
        pHyphPor->SetAscent( GetAscent() );
        SetLen( aInf.GetLen() );
        CalcTxtSize( aInf );

        Insert( pHyphPor );

        short nKern = rInf.GetFont()->CheckKerning();
        if( nKern )
            new SwKernPortion( *this, nKern );

        return sal_True;
    }

    // last exit for the lost
    delete pHyphPor;
    BreakCut( rInf, rGuess );
    return sal_False;
}


/*************************************************************************
 *              virtual SwHyphPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwHyphPortion::GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const
{
    // #i16816# tagged pdf support
    const sal_Unicode cChar = rInf.GetVsh() &&
                              rInf.GetVsh()->GetViewOptions()->IsPDFExport() &&
                              SwTaggedPDFHelper::IsExportTaggedPDF( *rInf.GetOut() ) ?
                              0xad :
                              '-';

    rTxt = OUString(cChar);
    return sal_True;
}

/*************************************************************************
 *              virtual SwHyphPortion::HandlePortion()
 *************************************************************************/

void SwHyphPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    OUString aString( '-' );
    rPH.Special( GetLen(), aString, GetWhichPor() );
}

/*************************************************************************
 *                 virtual SwHyphPortion::Format()
 *************************************************************************/

sal_Bool SwHyphPortion::Format( SwTxtFormatInfo &rInf )
{
    const SwLinePortion *pLast = rInf.GetLast();
    Height( pLast->Height() );
    SetAscent( pLast->GetAscent() );
    OUString aTxt;

    if( !GetExpTxt( rInf, aTxt ) )
        return sal_False;

    PrtWidth( rInf.GetTxtSize( aTxt ).Width() );
    const sal_Bool bFull = rInf.Width() <= rInf.X() + PrtWidth();
    if( bFull && !rInf.IsUnderFlow() ) {
        Truncate();
        rInf.SetUnderFlow( this );
    }

    return bFull;
}

/*************************************************************************
 *              virtual SwHyphStrPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwHyphStrPortion::GetExpTxt( const SwTxtSizeInfo &, OUString &rTxt ) const
{
    rTxt = aExpand;
    return sal_True;
}

/*************************************************************************
 *              virtual SwHyphStrPortion::HandlePortion()
 *************************************************************************/

void SwHyphStrPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), aExpand, GetWhichPor() );
}

/*************************************************************************
 *                      class SwSoftHyphPortion
 *************************************************************************/

SwLinePortion *SwSoftHyphPortion::Compress() { return this; }

SwSoftHyphPortion::SwSoftHyphPortion() :
    bExpand(sal_False), nViewWidth(0), nHyphWidth(0)
{
    SetLen(1);
    SetWhichPor( POR_SOFTHYPH );
}

KSHORT SwSoftHyphPortion::GetViewWidth( const SwTxtSizeInfo &rInf ) const
{
    // Wir stehen zwar im const, aber nViewWidth sollte erst im letzten
    // Moment errechnet werden:
    if( !Width() && rInf.OnWin() && rInf.GetOpt().IsSoftHyph() && !IsExpand() )
    {
        if( !nViewWidth )
            ((SwSoftHyphPortion*)this)->nViewWidth
                = rInf.GetTxtSize(OUString('-')).Width();
    }
    else
        ((SwSoftHyphPortion*)this)->nViewWidth = 0;
    return nViewWidth;
}

/*  Faelle:
 *  1) SoftHyph steht in der Zeile, ViewOpt aus.
 *     -> unsichtbar, Nachbarn unveraendert
 *  2) SoftHyph steht in der Zeile, ViewOpt an.
 *     -> sichtbar, Nachbarn veraendert
 *  3) SoftHyph steht am Zeilenende, ViewOpt aus/an.
 *     -> immer sichtbar, Nachbarn unveraendert
 */

void SwSoftHyphPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( Width() )
    {
        rInf.DrawViewOpt( *this, POR_SOFTHYPH );
        SwExpandPortion::Paint( rInf );
    }
}

/*************************************************************************
 *                 virtual SwSoftHyphPortion::Format()
 *************************************************************************/

/* Die endgueltige Breite erhalten wir im FormatEOL().
 * In der Underflow-Phase stellen wir fest, ob ueberhaupt ein
 * alternatives Spelling vorliegt. Wenn ja ...
 *
 * Fall 1: "Au-to"
 * 1) {Au}{-}{to}, {to} passt nicht mehr => Underflow
 * 2) {-} ruft Hyphenate => keine Alternative
 * 3) FormatEOL() und bFull = sal_True
 *
 * Fall 2: "Zuc-ker"
 * 1) {Zuc}{-}{ker}, {ker} passt nicht mehr => Underflow
 * 2) {-} ruft Hyphenate => Alternative!
 * 3) Underflow() und bFull = sal_True
 * 4) {Zuc} ruft Hyphenate => {Zuk}{-}{ker}
 */

sal_Bool SwSoftHyphPortion::Format( SwTxtFormatInfo &rInf )
{
    sal_Bool bFull = sal_True;

    // special case for old german spelling
    if( rInf.IsUnderFlow()  )
    {
        if( rInf.GetSoftHyphPos() )
            return sal_True;

        const sal_Bool bHyph = rInf.ChgHyph( sal_True );
        if( rInf.IsHyphenate() )
        {
            rInf.SetSoftHyphPos( rInf.GetIdx() );
            Width(0);
            // if the soft hyphend word has an alternative spelling
            // when hyphenated (old german spelling), the soft hyphen
            // portion has to trigger an underflow
            SwTxtGuess aGuess;
            bFull = rInf.IsInterHyph() ||
                    !aGuess.AlternativeSpelling( rInf, rInf.GetIdx() - 1 );
        }
        rInf.ChgHyph( bHyph );

        if( bFull && !rInf.IsHyphForbud() )
        {
            rInf.SetSoftHyphPos(0);
            FormatEOL( rInf );
            if ( rInf.GetFly() )
                rInf.GetRoot()->SetMidHyph( sal_True );
            else
                rInf.GetRoot()->SetEndHyph( sal_True );
        }
        else
        {
            rInf.SetSoftHyphPos( rInf.GetIdx() );
            Truncate();
            rInf.SetUnderFlow( this );
        }
        return sal_True;
    }

    rInf.SetSoftHyphPos(0);
    SetExpand( sal_True );
    bFull = SwHyphPortion::Format( rInf );
    SetExpand( sal_False );
    if( !bFull )
    {
        // default-maessig besitzen wir keine Breite, aber eine Hoehe
        nHyphWidth = Width();
        Width(0);
    }
    return bFull;
}

/*************************************************************************
 *                 virtual SwSoftHyphPortion::FormatEOL()
 *************************************************************************/
// Format end of Line

void SwSoftHyphPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
    if( !IsExpand() )
    {
        SetExpand( sal_True );
        if( rInf.GetLast() == this )
            rInf.SetLast( FindPrevPortion( rInf.GetRoot() ) );

        // 5964: alte Werte muessen wieder zurueckgesetzt werden.
        const SwTwips nOldX  = rInf.X();
        const xub_StrLen nOldIdx = rInf.GetIdx();
        rInf.X( rInf.X() - PrtWidth() );
        rInf.SetIdx( rInf.GetIdx() - GetLen() );
        const sal_Bool bFull = SwHyphPortion::Format( rInf );
        nHyphWidth = Width();

        // 6976: Eine truebe Sache: Wir werden erlaubterweise breiter,
        // aber gleich wird noch ein Fly verarbeitet, der eine korrekte
        // X-Position braucht.
        if( bFull || !rInf.GetFly() )
            rInf.X( nOldX );
        else
            rInf.X( nOldX + Width() );
        rInf.SetIdx( nOldIdx );
    }
}

/*************************************************************************
 *              virtual SwSoftHyphPortion::GetExpTxt()
 *
 * Wir expandieren:
 * - wenn die Sonderzeichen sichtbar sein sollen
 * - wenn wir am Ende der Zeile stehen.
 * - wenn wir vor einem (echten/emuliertem) Zeilenumbruch stehen
 *************************************************************************/

sal_Bool SwSoftHyphPortion::GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const
{
    if( IsExpand() || ( rInf.OnWin() && rInf.GetOpt().IsSoftHyph() ) ||
        ( GetPortion() && ( GetPortion()->InFixGrp() ||
          GetPortion()->IsDropPortion() || GetPortion()->IsLayPortion() ||
          GetPortion()->IsParaPortion() || GetPortion()->IsBreakPortion() ) ) )
    {
        return SwHyphPortion::GetExpTxt( rInf, rTxt );
    }
    return sal_False;
}

/*************************************************************************
 *              virtual SwSoftHyphPortion::HandlePortion()
 *************************************************************************/

void SwSoftHyphPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    const OUString aString( '-' );
    const sal_uInt16 nWhich = ! Width() ?
                          POR_SOFTHYPH_COMP :
                          GetWhichPor();
    rPH.Special( GetLen(), aString, nWhich );
}

/*************************************************************************
 *                      SwSoftHyphStrPortion::Paint
 *************************************************************************/

void SwSoftHyphStrPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    // Bug oder feature?:
    // {Zu}{k-}{ker}, {k-} wird grau statt {-}
    rInf.DrawViewOpt( *this, POR_SOFTHYPH );
    SwHyphStrPortion::Paint( rInf );
}

SwSoftHyphStrPortion::SwSoftHyphStrPortion( const OUString &rStr )
    : SwHyphStrPortion( rStr )
{
    SetLen( 1 );
    SetWhichPor( POR_SOFTHYPHSTR );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
