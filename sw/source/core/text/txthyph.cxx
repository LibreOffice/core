/*************************************************************************
 *
 *  $RCSfile: txthyph.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: tl $ $Date: 2001-03-29 08:07:29 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>  // SwViewOptions
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>
#endif
#ifndef _PORHYPH_HXX
#include <porhyph.hxx>  //
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _ITRFORM2_HXX
#include <itrform2.hxx> //
#endif
#ifndef _GUESS_HXX
#include <guess.hxx>    //
#endif
#ifndef _SPLARGS_HXX
#include <splargs.hxx>  // SwInterHyphInfo
#endif
#ifndef _PORRST_HXX
#include <porrst.hxx>   // SwKernPortion
#endif

#ifndef PRODUCT
extern const sal_Char *GetLangName( const MSHORT nLang );
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;

/*************************************************************************
 *                      SwTxtFormatInfo::HyphWord()
 *************************************************************************/

Reference< XHyphenatedWord >  SwTxtFormatInfo::HyphWord(
                                const XubString &rTxt, const MSHORT nMinTrail )
{
    if( rTxt.Len() < 4 || pFnt->IsSymbol(pVsh) )
        return 0;
//  ASSERT( IsHyphenate(), "SwTxtFormatter::HyphWord: why?" );
    Reference< XHyphenator >  xHyph = ::GetHyphenator();
    Reference< XHyphenatedWord > xHyphWord;

    if( xHyph.is() )
        xHyphWord = xHyph->hyphenate( OUString(rTxt),
                            pBreakIt->GetLocale( pFnt->GetLanguage() ),
                            rTxt.Len() - nMinTrail, GetHyphValues() );
    return xHyphWord;

}

/*************************************************************************
 *                      SwTxtFrm::Hyphenate
 *
 * Wir formatieren eine Zeile fuer die interaktive Trennung
 *************************************************************************/

sal_Bool SwTxtFrm::Hyphenate( SwInterHyphInfo &rHyphInf )
{
    if( !pBreakIt->xBreak.is() )
        return sal_False;;
    // Wir machen den Laden erstmal dicht:
    ASSERT( !IsLocked(), "SwTxtFrm::Hyphenate: this is locked" );
    // 4935: Der ::com::sun::star::frame::Frame muss eine gueltige SSize haben!
    Calc();
    GetFormatted();

    sal_Bool bRet = sal_False;
    if( !IsEmpty() )
    {
        // Wir muessen die Trennung immer einschalten.
        // Keine Angst, der SwTxtIter sichert im Hyphenate die alte Zeile.
        SwTxtFrmLocker aLock( this );
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
            DBG_LOOP;
            bRet = aLine.Hyphenate( rHyphInf );
            if( !aLine.Next() )
                break;
        }
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
    ASSERT( pRoot, "SetParaPortion: no root anymore" );
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
        ASSERT( IsParaLine(), "SwTxtFormatter::Hyphenate: not the first" );
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
                break;
            }

            nWrdStart += pPos->GetLen();
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
        ASSERT( IsParaLine(), "SwTxtFormatter::Hyphenate: even not the first" );
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
            pBreakIt->xBreak->getWordBoundary( rInf.GetTxt(), nWrdStart,
            pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ), WordType::DICTIONARY_WORD, sal_True );
        nWrdStart = aBound.startPos;
        nLen = aBound.endPos - nWrdStart;
        bRet = 0 != nLen;
        if( bRet )
        {
            XubString aSelTxt( rInf.GetTxt().Copy(nWrdStart, nLen) );
            xub_StrLen nCnt = 0;
            for( xub_StrLen i = 0; i < nLen; ++i )
            {
                sal_Unicode cCh = aSelTxt.GetChar(i);
                if( (CH_TXTATR_BREAKWORD == cCh || CH_TXTATR_INWORD == cCh )
                     && rInf.HasHint( nWrdStart + i ) )
                {
                    aSelTxt.Erase( i , 1 );
                    nCnt++;
                    --nLen;
                    if( i )
                        --i;
                }
            }
            {
                MSHORT nMinTrail = 0;
                if( nWrdStart + nLen > nEnd )
                    nMinTrail = nWrdStart + nLen - nEnd - 1;

                //!! rHyphInf.SetHyphWord( ... ) muﬂ hier geschehen
                xHyphWord = rInf.HyphWord( aSelTxt, nMinTrail );
                bRet = xHyphWord.is();
                if ( !rHyphInf.IsCheck() && sal_False == bRet )
                    rHyphInf.SetNoLang( sal_True );
            }

            if( bRet )
            {
                rHyphInf.SetHyphWord( xHyphWord );
                rHyphInf.nWordStart = nWrdStart;
                rHyphInf.nWordLen   = nLen+nCnt;
                rHyphInf.SetNoLang( sal_False );
                rHyphInf.SetCheck( sal_True );
            }
#ifdef DEBUGGY
            if( OPTDBG( rInf ) )
            {
                ASSERT( aSelTxt == aHyphWord,
                        "!SwTxtFormatter::Hyphenate: different words, different planets" );
                aDbstream << "Diff: \"" << aSelTxt.GetStr() << "\" != \""
                          << aHyphWord.GetStr() << "\"" << endl;
                ASSERT( bRet, "!SwTxtFormatter::Hyphenate: three of a perfect pair" );
                aDbstream << "Hyphenate: ";
            }
#endif
        }
    }
    return bRet;
}

/*************************************************************************
 *                      SwTxtPortion::CreateHyphen()
 *************************************************************************/

sal_Bool SwTxtPortion::CreateHyphen( SwTxtFormatInfo &rInf, SwTxtGuess &rGuess )
{
    ASSERT( !pPortion, "SwTxtPortion::Hyphenate: another portion, another planet..." );
    if( rInf.IsHyphForbud() ||
        pPortion || // robust
        // Mehrzeilige Felder duerfen nicht interaktiv getrennt werden.
        ( rInf.IsInterHyph() && InFldGrp() ) )
        return sal_False;

    Reference< XHyphenatedWord >  xHyphWord = rGuess.HyphWord();
    SwHyphPortion *pHyphPor;
    xub_StrLen nPorEnd;
    SwTxtSizeInfo aInf( rInf );

    // first case: hyphenated word has alternative spelling
    if ( xHyphWord.is() && xHyphWord->isAlternativeSpelling() ) {
        SvxAlternativeSpelling aAltSpell;
        aAltSpell = SvxGetAltSpelling( xHyphWord );
        ASSERT( aAltSpell.bIsAltSpelling, "no alternatve spelling" );

        XubString  aAltTxt   = aAltSpell.aReplacement;
        nPorEnd = aAltSpell.nChangedPos + rGuess.BreakStart();
        xub_StrLen nTmpLen = 0;

        // soft hyphen at alternative spelling position?
        if( rInf.GetTxt().GetChar( rInf.GetSoftHyphPos() ) == CHAR_SOFTHYPHEN )
        {
            pHyphPor = new SwSoftHyphStrPortion( aAltTxt );
            nTmpLen = 1;
        }
        else {
            pHyphPor = new SwHyphStrPortion( aAltTxt );
        }

        // length of pHyphPor is adjusted
        pHyphPor->SetLen( aAltTxt.Len() + 1 );
        (SwPosSize&)(*pHyphPor) = pHyphPor->GetTxtSize( rInf );
        pHyphPor->SetLen( aAltSpell.nChangedLength + nTmpLen );
    } else {
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
        nPorEnd = xHyphWord->getHyphenPos() + 1 + rGuess.BreakStart();
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

sal_Bool SwHyphPortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
{
    rTxt = '-';
    return sal_True;
}

/*************************************************************************
 *                 virtual SwHyphPortion::Format()
 *************************************************************************/

sal_Bool SwHyphPortion::Format( SwTxtFormatInfo &rInf )
{
    register const SwLinePortion *pLast = rInf.GetLast();
    Height( pLast->Height() );
    SetAscent( pLast->GetAscent() );
    XubString aTxt;

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

sal_Bool SwHyphStrPortion::GetExpTxt( const SwTxtSizeInfo &, XubString &rTxt ) const
{
    rTxt = aExpand;
    return sal_True;
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
                = rInf.GetTxtSize( '-' ).Width();
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
        const KSHORT nOldX   = rInf.X();
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

sal_Bool SwSoftHyphPortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
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
 *                      SwSoftHyphStrPortion::Paint
 *************************************************************************/

void SwSoftHyphStrPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    // Bug oder feature?:
    // {Zu}{k-}{ker}, {k-} wird grau statt {-}
    rInf.DrawViewOpt( *this, POR_SOFTHYPH );
    SwHyphStrPortion::Paint( rInf );
}

SwSoftHyphStrPortion::SwSoftHyphStrPortion( const XubString &rStr )
    : SwHyphStrPortion( rStr )
{
    SetLen( 1 );
    SetWhichPor( POR_SOFTHYPHSTR );
}



