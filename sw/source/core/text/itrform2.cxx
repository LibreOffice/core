/*************************************************************************
 *
 *  $RCSfile: itrform2.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: fme $ $Date: 2001-07-24 07:56:42 $
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

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _LAYFRM_HXX
#include <layfrm.hxx>       // GetFrmRstHeight, etc
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>      // SwViewOptions
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>       // SwFmtDrop
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>      // CH_TXTATR
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>
#endif
#ifndef _ITRFORM2_HXX
#include <itrform2.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>       // IsTox, IsRef, SetLingu
#endif
#ifndef _PORRST_HXX
#include <porrst.hxx>
#endif
#ifndef _PORTAB_HXX
#include <portab.hxx>       // pLastTab->
#endif
#ifndef _PORFLY_HXX
#include <porfly.hxx>       // CalcFlyWidth
#endif
#ifndef _PORTOX_HXX
#include <portox.hxx>       // WhichTxtPortion
#endif
#ifndef _PORREF_HXX
#include <porref.hxx>       // WhichTxtPortion
#endif
#ifndef _PORFLD_HXX
#include <porfld.hxx>       // SwNumberPortion fuer CalcAscent()
#endif
#ifndef _PORFTN_HXX
#include <porftn.hxx>       // SwFtnPortion
#endif
#ifndef _POREXP_HXX
#include <porexp.hxx>
#endif
#ifndef _PORHYPH_HXX
#include <porhyph.hxx>
#endif
#ifndef _GUESS_HXX
#include <guess.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>       // GetFrmRstHeight, etc
#endif
#ifndef _BLINK_HXX
#include <blink.hxx>        // pBlink
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>       // WhichFirstPortion() -> mal Verlagern.
#endif
#ifndef _REDLNITR_HXX
#include <redlnitr.hxx>     // SwRedlineItr
#endif
#ifndef _DOC_HXX
#include <doc.hxx>          // SwDoc
#endif
#ifndef _PORMULTI_HXX
#include <pormulti.hxx>     // SwMultiPortion
#endif
#define _SVSTDARR_LONGS
#include <svtools/svstdarr.hxx>
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifdef DEBUG
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // pSwpHints, Ausgabeoperator
#endif
#endif

using namespace ::com::sun::star::i18n;

#define MAX_TXTPORLEN 300

inline void ClearFly( SwTxtFormatInfo &rInf )
{
    if( rInf.GetFly() )
    {
        delete rInf.GetFly();
        rInf.SetFly(0);
    }
}

/*************************************************************************
 *                  SwTxtFormatter::CtorInit()
 *************************************************************************/

void SwTxtFormatter::CtorInit( SwTxtFrm *pFrm, SwTxtFormatInfo *pNewInf )
{
    SwTxtPainter::CtorInit( pFrm, pNewInf );
    pInf = pNewInf;
    pDropFmt = GetInfo().GetDropFmt();
    pMulti = NULL;

    bOnceMore = sal_False;
    bChanges = sal_False;
    bTruncLines = sal_False;
    nCntEndHyph = 0;
    nCntMidHyph = 0;
    nLeftScanIdx = STRING_LEN;
    nRightScanIdx = 0;

    if( nStart > GetInfo().GetTxt().Len() )
    {
        ASSERT( !this, "+SwTxtFormatter::CTOR: bad offset" );
        nStart = GetInfo().GetTxt().Len();
    }

}

/*************************************************************************
 *                      SwTxtFormatter::DTOR
 *************************************************************************/

SwTxtFormatter::~SwTxtFormatter()
{
    // Auesserst unwahrscheinlich aber denkbar.
    // z.B.: Feld spaltet sich auf, Widows schlagen zu
    if( GetInfo().GetRest() )
    {
        delete GetInfo().GetRest();
        GetInfo().SetRest(0);
    }
}

/*************************************************************************
 *                      SwTxtFormatter::Insert()
 *************************************************************************/

void SwTxtFormatter::Insert( SwLineLayout *pLay )
{
    // Einfuegen heute mal ausnahmsweise hinter dem aktuellen Element.
    if ( pCurr )
    {
        pLay->SetNext( pCurr->GetNext() );
        pCurr->SetNext( pLay );
    }
    else
        pCurr = pLay;
}

/*************************************************************************
 *                  SwTxtFormatter::GetFrmRstHeight()
 *************************************************************************/

KSHORT SwTxtFormatter::GetFrmRstHeight() const
{
    // 8725: Uns interessiert die Resthoehe bezogen auf die Seite.
    // Wenn wir in einer Tabelle stehen, dann ist pFrm->GetUpper() nicht
    // die Seite. GetFrmRstHeight() wird im Zusammenhang mit den Ftn
    // gerufen.
    // Falsch: const SwFrm *pUpper = pFrm->GetUpper();
    const SwFrm *pPage = (const SwFrm*)pFrm->FindPageFrm();
    const SwTwips nHeight = pPage->Frm().Top()
                          + pPage->Prt().Top()
                          + pPage->Prt().Height() - Y();
    if( 0 > nHeight )
        return pCurr->Height();
    else
        return KSHORT( nHeight );
}

/*************************************************************************
 *                  SwTxtFormatter::UnderFlow()
 *************************************************************************/

SwLinePortion *SwTxtFormatter::UnderFlow( SwTxtFormatInfo &rInf )
{
    // Werte sichern und rInf initialisieren.
    SwLinePortion *pUnderFlow = rInf.GetUnderFlow();
    if( !pUnderFlow )
        return 0;

    // Wir formatieren rueckwaerts, d.h. dass Attributwechsel in der
    // naechsten Zeile durchaus noch einmal drankommen koennen.
    // Zu beobachten in 8081.sdw, wenn man in der ersten Zeile Text eingibt.

    const xub_StrLen nSoftHyphPos = rInf.GetSoftHyphPos();

    // 8358, 8359: Flys sichern und auf 0 setzen, sonst GPF
    // 3983: Nicht ClearFly(rInf) !
    SwFlyPortion *pFly = rInf.GetFly();
    rInf.SetFly( 0 );

    FeedInf( rInf );
    rInf.SetLast( pCurr );
    // pUnderFlow braucht nicht deletet werden, weil es im folgenden
    // Truncate() untergehen wird.
    rInf.SetUnderFlow(0);
    rInf.SetSoftHyphPos( nSoftHyphPos );

    // Wir suchen die Portion mit der Unterlaufposition
    SwLinePortion *pPor = pCurr->GetFirstPortion();
    if( pPor != pUnderFlow )
    {
        // pPrev wird die letzte Portion vor pUnderFlow,
        // die noch eine echte Breite hat.
        // Ausnahme: SoftHyphPortions duerfen dabei natuerlich
        // nicht vergessen werden, obwohl sie keine Breite haben.
        SwLinePortion *pPrev = pPor;
        while( pPor && pPor != pUnderFlow )
        {
            DBG_LOOP;
            if( !pPor->IsKernPortion() &&
                ( pPor->Width() || pPor->IsSoftHyphPortion() ) )
            {
                while( pPrev != pPor )
                {
                    pPrev->Move( rInf );
                    rInf.SetLast( pPrev );
                    pPrev = pPrev->GetPortion();
                    ASSERT( pPrev, "UnderFlow: Loosing control!" );
                };
            }
            pPor = pPor->GetPortion();
        }
        pPor = pPrev;
        if( pPor && // Flies + Initialen werden nicht beim UnderFlow mitgenommen
            ( pPor->IsFlyPortion() || pPor->IsDropPortion() ||
              pPor->IsFlyCntPortion() ) )
        {
            pPor->Move( rInf );
            rInf.SetLast( pPor );
            rInf.SetStopUnderFlow( sal_True );
            pPor = pUnderFlow;
        }
    }

    // Was? Die Unterlaufsituation ist nicht in der Portion-Kette ?
    ASSERT( pPor, "SwTxtFormatter::UnderFlow: overflow but underflow" );

    if( rInf.IsFtnInside() && pPor && !rInf.IsQuick() )
    {
        SwLinePortion *pTmp = pPor->GetPortion();
        while( pTmp )
        {
            if( pTmp->IsFtnPortion() )
                ((SwFtnPortion*)pTmp)->ClearFtn();
            pTmp = pTmp->GetPortion();
        }
    }

    /*-----------------14.12.94 09:45-------------------
     * 9849: Schnellschuss
     * --------------------------------------------------*/
    if ( pPor==rInf.GetLast() )
    {
        // Hier landen wir, wenn die UnderFlow-ausloesende Portion sich
        // ueber die ganze Zeile erstreckt, z. B. wenn ein Wort ueber
        // mehrere Zeilen geht und in der zweiten Zeile in einen Fly
        // hineinlaeuft!
        rInf.SetFly( pFly ); // wg. 28300
        pPor->Truncate();
        return pPor; // Reicht das?
    }
    /*---------------------------------------------------
     * Ende des Schnellschusses wg. 9849
     * --------------------------------------------------*/

    // 4656: X + Width == 0 bei SoftHyph > Zeile ?!
    if( !pPor || !(rInf.X() + pPor->Width()) )
    {
        delete pFly;
        return 0;
    }

    // Vorbereitungen auf's Format()
    // Wir muessen die Kette hinter pLast abknipsen, weil
    // nach dem Format() ein Insert erfolgt.
    SeekAndChg( rInf );

    // line width is adjusted, so that pPor does not fit to current
    // line anymore
    rInf.Width( rInf.X() + (pPor->Width() ? pPor->Width() - 1 : 0) );
    rInf.SetLen( pPor->GetLen() );
    rInf.SetFull( sal_False );
    if( pFly )
    {
        // Aus folgendem Grund muss die FlyPortion neu berechnet werden:
        // Wenn durch einen grossen Font in der Mitte der Zeile die Grundlinie
        // abgesenkt wird und dadurch eine Ueberlappung mit eine Fly entsteht,
        // so hat die FlyPortion eine falsche Groesse/Fixsize.
        rInf.SetFly( pFly );
        CalcFlyWidth( rInf );
    }
    rInf.GetLast()->SetPortion(0);

    // Eine Ausnahme bildet das SwLineLayout, dass sich beim
    // ersten Portionwechsel aufspaltet. Hier nun der umgekehrte Weg:
    if( rInf.GetLast() == pCurr )
    {
        if( pPor->InTxtGrp() && !pPor->InExpGrp() )
        {
            MSHORT nOldWhich = pCurr->GetWhichPor();
            *(SwLinePortion*)pCurr = *pPor;
            pCurr->SetPortion( pPor->GetPortion() );
            pCurr->SetWhichPor( nOldWhich );
            pPor->SetPortion( 0 );
            delete pPor;
            pPor = pCurr;
        }
    }
    pPor->Truncate();
    delete rInf.GetRest();
    rInf.SetRest(0);
    return pPor;
}

/*************************************************************************
 *                      SwTxtFormatter::InsertPortion()
 *************************************************************************/

void SwTxtFormatter::InsertPortion( SwTxtFormatInfo &rInf,
                                    SwLinePortion *pPor ) const
{
    // Die neue Portion wird eingefuegt,
    // bei dem LineLayout ist allerdings alles anders...
    if( pPor == pCurr )
    {
        if( pCurr->GetPortion() )
            pPor = pCurr->GetPortion();
    }
    else
    {
        SwLinePortion *pLast = rInf.GetLast();
        if( pLast->GetPortion() )
        {
            while( pLast->GetPortion() )
                pLast = pLast->GetPortion();
            rInf.SetLast( pLast );
        }
        pLast->Insert( pPor );

        // Maxima anpassen:
        if( pCurr->Height() < pPor->Height() )
            pCurr->Height( pPor->Height() );
        if( pCurr->GetAscent() < pPor->GetAscent() )
            pCurr->SetAscent( pPor->GetAscent() );
    }

    // manchmal werden ganze Ketten erzeugt (z.B. durch Hyphenate)
    rInf.SetLast( pPor );
    while( pPor )
    {
        DBG_LOOP;
        pPor->Move( rInf );
        rInf.SetLast( pPor );
        pPor = pPor->GetPortion();
    }
}

/*************************************************************************
 *                      SwTxtFormatter::BuildPortion()
 *************************************************************************/

void SwTxtFormatter::BuildPortions( SwTxtFormatInfo &rInf )
{
    ASSERT( rInf.GetTxt().Len() < STRING_LEN,
            "SwTxtFormatter::BuildPortions: bad text length in info" );

    rInf.ChkNoHyph( CntEndHyph(), CntMidHyph() );

    // Erst NewTxtPortion() entscheidet, ob pCurr in pPor landet.
    // Wir muessen in jedem Fall dafuer sorgen, dass der Font eingestellt
    // wird. In CalcAscent geschieht dies automatisch.
    rInf.SetLast( pCurr );
    rInf.ForcedLeftMargin( 0 );

    ASSERT( pCurr->FindLastPortion() == pCurr, "pLast supposed to equal pCurr" );

    if( !pCurr->GetAscent() && !pCurr->Height() )
        CalcAscent( rInf, pCurr );

    SeekAndChg( rInf );

    // In CalcFlyWidth wird Width() verkuerzt, wenn eine FlyPortion vorliegt.
    ASSERT( !rInf.X() || pMulti, "SwTxtFormatter::BuildPortion X=0?" );
    CalcFlyWidth( rInf );
    SwFlyPortion *pFly = rInf.GetFly();
    if( pFly )
    {
        if ( 0 < pFly->Fix() )
            ClearFly( rInf );
        else
            rInf.SetFull(sal_True);
    }

    SwLinePortion *pPor = NewPortion( rInf );
    sal_Bool bFull;
    rInf.Y( Y() );

    while( pPor && !rInf.IsStop() )
    {
        ASSERT( rInf.GetLen() < STRING_LEN &&
                rInf.GetIdx() <= rInf.GetTxt().Len(),
                "SwTxtFormatter::BuildPortions: bad length in info" );
        DBG_LOOP;

        if( pPor->InFldGrp() && !pPor->InNumberGrp() )
            ((SwFldPortion*)pPor)->CheckScript( rInf );

        if( rInf.HasScriptSpace() && rInf.GetLast() && rInf.GetLast()->InTxtGrp()
            && rInf.GetLast()->Width() && !rInf.GetLast()->InNumberGrp() )
        {
            BYTE nNxtActual = rInf.GetFont()->GetActual();
            BYTE nLstActual = nNxtActual;
            USHORT nLstHeight = (USHORT)rInf.GetFont()->GetHeight();
            sal_Bool bAllowBefore = sal_False;
            sal_Bool bAllowBehind = sal_False;
            const CharClass& rCC = GetAppCharClass();

            // are there any punctuation characters on both sides
            // of the kerning portion?
            if ( pPor->InFldGrp() )
            {
                XubString aAltTxt;
                if ( ((SwFldPortion*)pPor)->GetExpTxt( rInf, aAltTxt ) &&
                        aAltTxt.Len() )
                {
                    bAllowBehind = rCC.isLetterNumeric( aAltTxt, 0 );

                    const SwFont* pTmpFnt = ((SwFldPortion*)pPor)->GetFont();
                    if ( pTmpFnt )
                        nNxtActual = pTmpFnt->GetActual();
                }
            }
            else
                bAllowBehind = rCC.isLetterNumeric( rInf.GetTxt(), rInf.GetIdx() );

            const SwLinePortion* pLast = rInf.GetLast();
            if ( bAllowBehind && pLast )
            {
                if ( pLast->InFldGrp() )
                {
                    XubString aAltTxt;
                    if ( ((SwFldPortion*)pLast)->GetExpTxt( rInf, aAltTxt ) &&
                         aAltTxt.Len() )
                    {
                        bAllowBefore = rCC.isLetterNumeric( aAltTxt, aAltTxt.Len() - 1 );

                        const SwFont* pTmpFnt = ((SwFldPortion*)pLast)->GetFont();
                        if ( pTmpFnt )
                        {
                            nLstActual = pTmpFnt->GetActual();
                            nLstHeight = (USHORT)pTmpFnt->GetHeight();
                        }
                    }
                }
                else if ( rInf.GetIdx() )
                {
                    bAllowBefore = rCC.isLetterNumeric( rInf.GetTxt(), rInf.GetIdx() - 1 );
                    // Note: ScriptType returns values in [1,4]
                    if ( bAllowBefore )
                        nLstActual = pScriptInfo->ScriptType( rInf.GetIdx() - 1 ) - 1;
                }

                nLstHeight /= 5;
                // does the kerning portion still fit into the line?
                if( bAllowBefore && ( nLstActual != nNxtActual ) &&
                    nLstHeight && rInf.X() + nLstHeight <= rInf.Width() )
                {
                    SwKernPortion* pKrn =
                        new SwKernPortion( *rInf.GetLast(), nLstHeight );
                    rInf.GetLast()->SetPortion( NULL );
                    InsertPortion( rInf, pKrn );
                }
            }
        }

        // the multi-portion has it's own format function
        if( pPor->IsMultiPortion() && !pMulti )
            bFull = BuildMultiPortion( rInf, *((SwMultiPortion*)pPor) );
        else
            bFull = pPor->Format( rInf );

        if( rInf.IsRuby() && !rInf.GetRest() )
            bFull = sal_True;

        if ( pPor->IsFlyPortion() )
            pCurr->SetFly( sal_True );
        // some special cases, where we have to take care for the repaint
        // offset:
        // 1. Right Tab
        // 2. Multiportions
        else if (  ! rInf.GetPaintOfst() &&
                   // 1. Right Tab
                   ( ( pPor->InTabGrp() && !pPor->IsTabLeftPortion() ) ||
                   // 2. Multi Portion
                     ( pPor->IsMultiPortion() &&
                       rInf.GetReformatStart() >= rInf.GetIdx() &&
                       rInf.GetReformatStart() <= rInf.GetIdx() + pPor->GetLen() )
                   )
                )
            // we store the beginning of the critical portion as our
            // paint offset
            rInf.SetPaintOfst( GetLeftMargin() + rInf.X() );

        if( pPor->IsFlyCntPortion() || ( pPor->IsMultiPortion() &&
            ((SwMultiPortion*)pPor)->HasFlyInCntnt() ) )
            SetFlyInCntBase();

        // 5964: bUnderFlow muss zurueckgesetzt werden, sonst wird beim
        //       naechsten Softhyphen wieder umgebrochen!
        if ( !bFull )
        {
            rInf.ClrUnderFlow();
            if( rInf.HasScriptSpace() && pPor->InTxtGrp() &&
                pPor->GetLen() && !pPor->InFldGrp() )
            {
                // The distance between two different scripts is set
                // to 20% of the fontheight.
                xub_StrLen nTmp = rInf.GetIdx() + pPor->GetLen();
                if( nTmp == pScriptInfo->NextScriptChg( nTmp - 1 ) &&
                    nTmp != rInf.GetTxt().Len() )
                {
                    USHORT nDist = rInf.GetFont()->GetHeight()/5;

                    if( nDist )
                    {
                        // we do not want a kerning portion if any end
                        // would be a punctuation character
                        const CharClass& rCC = GetAppCharClass();
                        if ( rCC.isLetterNumeric( rInf.GetTxt(), nTmp - 1 ) &&
                             rCC.isLetterNumeric( rInf.GetTxt(), nTmp ) )
                        {
                            // does the kerning portion still fit into the line?
                            if ( rInf.X() + pPor->Width() + nDist <= rInf.Width() )
                                new SwKernPortion( *pPor, nDist );
                            else
                                bFull = sal_True;
                        }
                    }
                }
            }
        }

        rInf.SetFull( bFull );

        // Restportions von mehrzeiligen Feldern haben bisher noch
        // nicht den richtigen Ascent.
        if ( !pPor->GetLen() && !pPor->IsFlyPortion()
            && !pPor->IsGrfNumPortion() && !pPor->IsMultiPortion() )
            CalcAscent( rInf, pPor );

        InsertPortion( rInf, pPor );
        pPor = NewPortion( rInf );
    }

    if( !rInf.IsStop() )
    {
        // der letzte rechte, zentrierte, dezimale Tab
        SwTabPortion *pLastTab = rInf.GetLastTab();
        if( pLastTab )
            pLastTab->FormatEOL( rInf );
        else if( rInf.GetLast() && rInf.LastKernPortion() )
            rInf.GetLast()->FormatEOL( rInf );
    }
    if( pCurr->GetPortion() && pCurr->GetPortion()->InNumberGrp()
        && ((SwNumberPortion*)pCurr->GetPortion())->IsHide() )
        rInf.SetNumDone( sal_False );

    // 3260, 3860: Fly auf jeden Fall loeschen!
    ClearFly( rInf );
}

/*************************************************************************
 *                 SwTxtFormatter::CalcAdjustLine()
 *************************************************************************/

void SwTxtFormatter::CalcAdjustLine( SwLineLayout *pCurr )
{
    if( SVX_ADJUST_LEFT != GetAdjust() && !pMulti)
    {
        pCurr->SetFormatAdj(sal_True);
        if( IsFlyInCntBase() )
        {
            CalcAdjLine( pCurr );
            // 23348: z.B. bei zentrierten Flys muessen wir den RefPoint
            // auf jeden Fall umsetzen, deshalb bAllWays = sal_True
            UpdatePos( pCurr, GetTopLeft(), GetStart(), sal_True );
        }
    }
}

/*************************************************************************
 *                      SwTxtFormatter::CalcAscent()
 *************************************************************************/

void SwTxtFormatter::CalcAscent( SwTxtFormatInfo &rInf, SwLinePortion *pPor )
{
    if ( pPor->InFldGrp() && ((SwFldPortion*)pPor)->GetFont() )
    {
        // Numerierungen + InterNetFlds koennen einen eigenen Font beinhalten,
        // dann ist ihre Groesse unabhaengig von harten Attributierungen.
        SwFont* pFldFnt = ((SwFldPortion*)pPor)->pFnt;
        SwFontSave aSave( rInf, pFldFnt );
        ((SwFldPortion*)pPor)->Height( pFldFnt->GetHeight( rInf.GetVsh(), rInf.GetOut() ) );
        ((SwFldPortion*)pPor)->SetAscent( pFldFnt->GetAscent( rInf.GetVsh(), rInf.GetOut() ) );
    }
    else
    {
        const SwLinePortion *pLast = rInf.GetLast();
        sal_Bool bChg;

        // Fallunterscheidung: in leeren Zeilen werden die Attribute
        // per SeekStart angeschaltet.
        const sal_Bool bFirstPor = rInf.GetLineStart() == rInf.GetIdx();
        if ( pPor->IsQuoVadisPortion() )
            bChg = SeekStartAndChg( rInf, sal_True );
        else
        {
            if( bFirstPor )
            {
                if( rInf.GetTxt().Len() )
                {
                    if ( pPor->GetLen() || !rInf.GetIdx()
                         || ( pCurr != pLast && !pLast->IsFlyPortion() )
                         || !pCurr->IsRest() ) // statt !rInf.GetRest()
                        bChg = SeekAndChg( rInf );
                    else
                        bChg = SeekAndChgBefore( rInf );
                }
                else
                    bChg = SeekStartAndChg( rInf );
            }
            else
                bChg = SeekAndChg( rInf );
        }
        if( bChg || bFirstPor || !pPor->GetAscent()
            || !rInf.GetLast()->InTxtGrp() )
        {
            pPor->SetAscent( rInf.GetAscent()  );
            pPor->Height( rInf.GetTxtHeight() );
        }
        else
        {
            pPor->Height( pLast->Height() );
            pPor->SetAscent( pLast->GetAscent() );
        }
    }
}

/*************************************************************************
 *                      SwTxtFormatter::WhichTxtPor()
 *************************************************************************/

SwTxtPortion *SwTxtFormatter::WhichTxtPor( SwTxtFormatInfo &rInf ) const
{
    SwTxtPortion *pPor = 0;
    if( GetFnt()->IsTox() )
        pPor = new SwToxPortion;
    else
    {
        if( GetFnt()->IsRef() )
            pPor = new SwRefPortion;
        else
        {
            // Erst zum Schluss !
            // Wenn pCurr keine Breite hat, kann sie trotzdem schon Inhalt haben,
            // z.B. bei nicht darstellbaren Zeichen.
            if( !rInf.X() && !pCurr->GetPortion() && !pCurr->GetLen() &&
                !GetFnt()->IsURL() )
                pPor = pCurr;
            else
            {
                pPor = new SwTxtPortion;
                if( GetFnt()->IsURL() )
                    pPor->SetWhichPor( POR_URL );
            }
        }
    }
    return pPor;
}

/*************************************************************************
 *                      SwTxtFormatter::NewTxtPortion()
 *************************************************************************/
// Die Laenge wird ermittelt, folgende Portion-Grenzen sind definiert:
// 1) Tabs
// 2) Linebreaks
// 3) CH_TXTATR_BREAKWORD / CH_TXTATR_INWORD
// 4) naechster Attributwechsel

SwTxtPortion *SwTxtFormatter::NewTxtPortion( SwTxtFormatInfo &rInf )
{
    // Wenn wir am Zeilenbeginn stehen, nehmen wir pCurr
    // Wenn pCurr nicht von SwTxtPortion abgeleitet ist,
    // muessen wir duplizieren ...
    Seek( rInf.GetIdx() );
    SwTxtPortion *pPor = WhichTxtPor( rInf );

    // maximal bis zum naechsten Attributwchsel.
    xub_StrLen nNextAttr = GetNextAttr();
    xub_StrLen nNextChg = Min( nNextAttr, rInf.GetTxt().Len() );

    nNextAttr = pScriptInfo->NextScriptChg( rInf.GetIdx() );
    if( nNextChg > nNextAttr )
        nNextChg = nNextAttr;

    // 7515, 7516, 3470, 6441 : Turbo-Boost
    // Es wird unterstellt, dass die Buchstaben eines Fonts nicht
    // groesser als doppelt so breit wie hoch sind.
    // 7659: Ganz verrueckt: man muss sich auf den Ascent beziehen.
    // Falle: GetSize() enthaelt die Wunschhoehe, die reale Hoehe
    // ergibt sich erst im CalcAscent!
    // 7697: Das Verhaeltnis ist noch krasser: ein Blank im Times
    // New Roman besitzt einen Ascent von 182, eine Hoehe von 200
    // und eine Breite von 53! Daraus folgt, dass eine Zeile mit
    // vielen Blanks falsch eingeschaetzt wird. Wir erhoehen von
    // Faktor 2 auf 8 (wg. negativen Kernings).

    pPor->SetLen(1);
    CalcAscent( rInf, pPor );

    const SwFont* pFnt = rInf.GetFont();
    KSHORT nExpect = Min( KSHORT( ((Font *)pFnt)->GetSize().Height() ),
                          KSHORT( pPor->GetAscent() ) ) / 8;
    if ( !nExpect )
        nExpect = 1;
    nExpect = rInf.GetIdx() + ((rInf.Width() - rInf.X()) / nExpect);
    if( nExpect > rInf.GetIdx() && nNextChg > nExpect )
        nNextChg = Min( nExpect, rInf.GetTxt().Len() );

    // 4294: Vorsicht vor STRING_LEN-Ueberrundungen !
    if( MAX_TXTPORLEN < nNextChg && STRING_LEN - MAX_TXTPORLEN > rInf.GetIdx() )
    {
        const xub_StrLen nMaxChg = rInf.GetIdx() + MAX_TXTPORLEN;

        if( nMaxChg < nNextChg )
        {
            // 6441: uebel ist, wenn die Portion passt...
            const SwScriptInfo& rSI =
                ((SwParaPortion*)rInf.GetParaPortion())->GetScriptInfo();

            USHORT nMaxComp = ( SW_CJK == rInf.GetFont()->GetActual() ) &&
                                rSI.CountCompChg() &&
                                ! rInf.IsMulti() &&
                                ! pPor->InFldGrp() &&
                                ! pPor->IsDropPortion() ?
                                10000 :
                                    0 ;

            const KSHORT nWidth =
                rInf.GetTxtSize(
                     &rInf.GetParaPortion()->GetScriptInfo(),
                     rInf.GetIdx(), MAX_TXTPORLEN, nMaxComp ).Width();
            if( nWidth > rInf.Width() )
                nNextChg = Min( nMaxChg, rInf.GetTxt().Len() );
        }
    }

    // we keep an invariant during method calls:
    // there are no portion ending characters like hard spaces
    // or tabs in [ nLeftScanIdx, nRightScanIdx ]
    if ( nLeftScanIdx <= rInf.GetIdx() && rInf.GetIdx() <= nRightScanIdx )
    {
        if ( nNextChg > nRightScanIdx )
            nNextChg = nRightScanIdx = rInf.ScanPortionEnd( nRightScanIdx, nNextChg );
    }
    else
    {
        nLeftScanIdx = rInf.GetIdx();
        nNextChg = nRightScanIdx = rInf.ScanPortionEnd( rInf.GetIdx(), nNextChg );
    }

    pPor->SetLen( nNextChg - rInf.GetIdx() );
    rInf.SetLen( pPor->GetLen() );
    return pPor;
}


/*************************************************************************
 *                 SwTxtFormatter::WhichFirstPortion()
 *************************************************************************/

SwLinePortion *SwTxtFormatter::WhichFirstPortion(SwTxtFormatInfo &rInf) const
{
    SwLinePortion *pPor = 0;

    if( rInf.GetRest() )
    {
        // 5010: Tabs und Felder
        if( '\0' != rInf.GetHookChar() )
            return 0;

        pPor = rInf.GetRest();
        if( pPor->IsErgoSumPortion() )
            rInf.SetErgoDone(sal_True);
        else
            if( pPor->IsFtnNumPortion() )
                rInf.SetFtnDone(sal_True);
            else
                if( pPor->InNumberGrp() )
                    rInf.SetNumDone(sal_True);
        if( pPor )
        {
            rInf.SetRest(0);
            pCurr->SetRest( sal_True );
            return pPor;
        }
    }

    // ???? und ????: im Follow duerfen wir schon stehen,
    // entscheidend ist, ob pFrm->GetOfst() == 0 ist!
    if( rInf.GetIdx() )
    {
        // Nun koennen auch FtnPortions und ErgoSumPortions
        // verlaengert werden.

        // 1) Die ErgoSumTexte
        if( !rInf.IsErgoDone() )
        {
            if( pFrm->IsInFtn() && !pFrm->GetIndPrev() )
                pPor = (SwLinePortion*)NewErgoSumPortion( rInf );
            rInf.SetErgoDone( sal_True );
        }
        if( !pPor && !rInf.IsArrowDone() )
        {
            if( pFrm->GetOfst() && !pFrm->IsFollow() &&
                rInf.GetIdx() == pFrm->GetOfst() )
                pPor = new SwArrowPortion( *pCurr );
            rInf.SetArrowDone( sal_True );
        }

        // 2) Die Zeilenreste (mehrzeilige Felder)
        if( !pPor )
        {
            pPor = rInf.GetRest();
            // 6922: Nur bei pPor natuerlich.
            if( pPor )
            {
                pCurr->SetRest( sal_True );
                rInf.SetRest(0);
            }
        }
    }
    else if ( ! rInf.IsMulti() )
    {
        // 1) Die Fussnotenzahlen
        if( !rInf.IsFtnDone() )
        {
            sal_Bool bFtnNum = pFrm->IsFtnNumFrm();
            rInf.GetParaPortion()->SetFtnNum( bFtnNum );
            if( bFtnNum )
                pPor = (SwLinePortion*)NewFtnNumPortion( rInf );
            rInf.SetFtnDone( sal_True );
        }

        // 2) Die ErgoSumTexte gibt es natuerlich auch im TextMaster,
        // entscheidend ist, ob der SwFtnFrm ein Follow ist.
        if( !rInf.IsErgoDone() && !pPor )
        {
            if( pFrm->IsInFtn() && !pFrm->GetIndPrev() )
                pPor = (SwLinePortion*)NewErgoSumPortion( rInf );
            rInf.SetErgoDone( sal_True );
        }

        // 3) Die Numerierungen
        if( !rInf.IsNumDone() && !pPor )
        {
            // Wenn wir im Follow stehen, dann natuerlich nicht.
            if( GetTxtFrm()->GetTxtNode()->GetNum() ||
                GetTxtFrm()->GetTxtNode()->GetOutlineNum() )
            {
                pPor = (SwLinePortion*)NewNumberPortion( rInf );
            }
            rInf.SetNumDone( sal_True );
        }
        // 4) Die DropCaps
        if( !pPor && GetDropFmt() )
            pPor = (SwLinePortion*)NewDropPortion( rInf );
    }
    return pPor;
}

sal_Bool lcl_OldFieldRest( const SwLineLayout* pCurr )
{
    if( !pCurr->GetNext() )
        return sal_False;
    const SwLinePortion *pPor = pCurr->GetNext()->GetPortion();
    sal_Bool bRet = sal_False;
    while( pPor && !bRet )
    {
        bRet = (pPor->InFldGrp() && ((SwFldPortion*)pPor)->IsFollow()) ||
            (pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsFollowFld());
        if( !pPor->GetLen() )
            break;
        pPor = pPor->GetPortion();
    }
    return bRet;
}

/*************************************************************************
 *                      SwTxtFormatter::NewPortion()
 *************************************************************************/

/* NewPortion stellt rInf.nLen ein.
 * Eine SwTxtPortion wird begrenzt durch ein tab, break, txtatr,
 * attrwechsel.
 * Drei Faelle koennen eintreten:
 * 1) Die Zeile ist voll und der Umbruch wurde nicht emuliert
 *    -> return 0;
 * 2) Die Zeile ist voll und es wurde ein Umbruch emuliert
 *    -> Breite neu einstellen und return new FlyPortion
 * 3) Es muss eine neue Portion gebaut werden.
 *    -> CalcFlyWidth emuliert ggf. die Breite und return Portion
 */

SwLinePortion *SwTxtFormatter::NewPortion( SwTxtFormatInfo &rInf )
{
    // Underflow hat Vorrang
    rInf.SetStopUnderFlow( sal_False );
    if( rInf.GetUnderFlow() )
    {
        ASSERT( rInf.IsFull(), "SwTxtFormatter::NewPortion: underflow but not full" );
        return UnderFlow( rInf );
    }

    // Wenn die Zeile voll ist, koennten noch Flys oder
    // UnderFlow-LinePortions warten ...
    if( rInf.IsFull() )
    {
        // ????: LineBreaks und Flys (bug05.sdw)
        // 8450: IsDummy()
        if( rInf.IsNewLine() && (!rInf.GetFly() || !pCurr->IsDummy()) )
            return 0;

        // Wenn der Text an den Fly gestossen ist, oder wenn
        // der Fly als erstes drankommt, weil er ueber dem linken
        // Rand haengt, wird GetFly() returnt.
        // Wenn IsFull() und kein GetFly() vorhanden ist, gibt's
        // naturgemaesz eine 0.
        if( rInf.GetFly() )
        {
            if( rInf.GetLast()->IsBreakPortion() )
            {
                delete rInf.GetFly();
                rInf.SetFly( 0 );
            }

            return rInf.GetFly();
        }
        // Ein fieser Sonderfall: ein Rahmen ohne Umlauf kreuzt den
        // Ftn-Bereich. Wir muessen die Ftn-Portion als Zeilenrest
        // bekanntgeben, damit SwTxtFrm::Format nicht abbricht
        // (die Textmasse wurde ja durchformatiert).
        if( rInf.GetRest() )
            rInf.SetNewLine( sal_True );
        else
        {
            // Wenn die naechste Zeile mit einem Rest eines Feldes beginnt,
            // jetzt aber kein Rest mehr anliegt,
            // muss sie auf jeden Fall neu formatiert werden!
            if( lcl_OldFieldRest( GetCurr() ) )
                rInf.SetNewLine( sal_True );
            else
            {
                SwLinePortion *pFirst = WhichFirstPortion( rInf );
                if( pFirst )
                {
                    rInf.SetNewLine( sal_True );
                    if( pFirst->InNumberGrp() )
                        rInf.SetNumDone( sal_False) ;
                    delete pFirst;
                }
            }
        }

        return 0;
    }

    SwLinePortion *pPor = WhichFirstPortion( rInf );

    if( !pPor )
    {
        if( !pMulti )
        {   // We open a multiportion part, if we enter a multi-line part
            // of the paragraph.
            xub_StrLen nEnd = rInf.GetIdx();
            SwMultiCreator* pCreate = rInf.GetMultiCreator( nEnd );
            if( pCreate )
            {
                SwMultiPortion* pTmp = NULL;
                if( SW_MC_RUBY == pCreate->nId )
                {
                    Seek( rInf.GetIdx() );
                    pTmp = new SwRubyPortion( *pCreate, *rInf.GetFont(),
                                              *rInf.GetDoc(), nEnd );
                }
                else if( SW_MC_ROTATE == pCreate->nId )
                    pTmp = new SwRotatedPortion( *pCreate, nEnd );
                else
                    pTmp = new SwDoubleLinePortion( *pCreate, nEnd );

                delete pCreate;
                CalcFlyWidth( rInf );

                return pTmp;
            }
        }
        // 5010: Tabs und Felder
        xub_Unicode cChar = rInf.GetHookChar();

        if( cChar )
        {
            /* Wir holen uns nocheinmal cChar, um sicherzustellen, dass das
             * Tab jetzt wirklich ansteht und nicht auf die naechste Zeile
             * gewandert ist ( so geschehen hinter Rahmen ).
             * Wenn allerdings eine FldPortion im Rest wartet, muessen wir
             * das cChar natuerlich aus dem Feldinhalt holen, z.B. bei
             * DezimalTabs und Feldern (22615)
            */
            if( !rInf.GetRest() || !rInf.GetRest()->InFldGrp() )
                cChar = rInf.GetChar( rInf.GetIdx() );
            rInf.SetHookChar(0);
        }
        else
        {
            if( rInf.GetIdx() >= rInf.GetTxt().Len() )
            {
                rInf.SetFull(sal_True);
                CalcFlyWidth( rInf );
                return pPor;
            }
            cChar = rInf.GetChar( rInf.GetIdx() );
        }

        switch( cChar )
        {
            case CH_TAB    : pPor = NewTabPortion( rInf );  break;
            case CH_BREAK  : pPor = new SwBreakPortion( *rInf.GetLast() ); break;

            case CHAR_SOFTHYPHEN:                   // soft hyphen
                pPor = new SwSoftHyphPortion; break;

            case CHAR_HARDBLANK:                    // no-break space
                pPor = new SwBlankPortion( ' ' ); break;
            case CHAR_HARDHYPHEN:               // non-breaking hyphen
                pPor = new SwBlankPortion( '-' ); break;

            case CH_TXTATR_BREAKWORD:
            case CH_TXTATR_INWORD:
                            if( rInf.HasHint( rInf.GetIdx() ) )
                            {
                                pPor = NewExtraPortion( rInf );
                                break;
                            }
                            // No break
            default        :
            {
                if( rInf.GetLastTab() && cChar == rInf.GetTabDecimal() )
                    rInf.SetFull( rInf.GetLastTab()->Format( rInf ) );

                if( rInf.GetRest() )
                {
                    if( rInf.IsFull() )
                    {
                        rInf.SetNewLine(sal_True);
                        return 0;
                    }
                    pPor = rInf.GetRest();
                    rInf.SetRest(0);
                }
                else
                {
                    if( rInf.IsFull() )
                        return 0;
                    pPor = NewTxtPortion( rInf );
                }
                break;
            }
        }

        // Wenn eine Portion erzeugt wird, obwohl eine RestPortion ansteht,
        // dann haben wir es mit einem Feld zu tun, das sich aufgesplittet
        // hat, weil z.B. ein Tab enthalten ist.
        if( pPor && rInf.GetRest() )
            pPor->SetLen( 0 );

        // robust:
        if( !pPor || rInf.IsStop() )
        {
            delete pPor;
            return 0;
        }
    }

    // Der Font wird im Outputdevice eingestellt,
    // der Ascent und die Hoehe werden berechnet.
    if( !pPor->GetAscent() && !pPor->Height() )
        CalcAscent( rInf, pPor );
    rInf.SetLen( pPor->GetLen() );

    // In CalcFlyWidth wird Width() verkuerzt, wenn eine FlyPortion vorliegt.
    CalcFlyWidth( rInf );

    // Man darf nicht vergessen, dass pCurr als GetLast() vernuenftige
    // Werte bereithalten muss:
    if( !pCurr->Height() )
    {
        ASSERT( pCurr->Height(), "SwTxtFormatter::NewPortion: limbo dance" );
        pCurr->Height( pPor->Height() );
        pCurr->SetAscent( pPor->GetAscent() );
    }

    ASSERT( !pPor || pPor->Height(),
            "SwTxtFormatter::NewPortion: something went wrong");
    if( pPor->IsPostItsPortion() && rInf.X() >= rInf.Width() && rInf.GetFly() )
    {
        delete pPor;
        pPor = rInf.GetFly();
    }
    return pPor;
}

/*************************************************************************
 *                      SwTxtFormatter::FormatLine()
 *************************************************************************/

xub_StrLen SwTxtFormatter::FormatLine( const xub_StrLen nStart )
{
    SwHookOut aHook( &GetInfo() );
    if( GetInfo().GetLen() < GetInfo().GetTxt().Len() )
        GetInfo().SetLen( GetInfo().GetTxt().Len() );

    sal_Bool bBuild = sal_True;
    SetFlyInCntBase( sal_False );
    GetInfo().SetLineHeight( 0 );
    GetInfo().SetLineNettoHeight( 0 );

    // Recycling muss bei geaenderter Zeilenhoehe unterdrueckt werden
    // und auch bei geaendertem Ascent (Absenken der Grundlinie).
    const KSHORT nOldHeight = pCurr->Height();
    const KSHORT nOldAscent = pCurr->GetAscent();

    pCurr->SetEndHyph( sal_False );
    pCurr->SetMidHyph( sal_False );

    // fly positioning can make it necessary format a line several times
    // for this, we have to keep a copy of our rest portion
    SwLinePortion* pFld = GetInfo().GetRest();
    SwFldPortion* pSaveFld = 0;

    if ( pFld && pFld->InFldGrp() )
        pSaveFld = new SwFldPortion( *((SwFldPortion*)pFld) );

    // for an optimal repaint rectangle, we want to compare fly portions
    // before and after the BuildPortions call
    const sal_Bool bOptimizeRepaint = AllowRepaintOpt( GetInfo() );
    SvLongs* pFlyStart = 0;

    // these are the conditions for a fly position comparison
    if ( bOptimizeRepaint && pCurr->IsFly() )
    {
        pFlyStart = new SvLongs;
        SwLinePortion* pPor = pCurr->GetFirstPortion();
        long nPOfst = 0;
        USHORT nCnt = 0;

        while ( pPor )
        {
            if ( pPor->IsFlyPortion() )
                // insert start value of fly portion
                pFlyStart->Insert( nPOfst, nCnt++ );

            nPOfst += pPor->Width();
            pPor = pPor->GetPortion();
        }
    }

    // Hier folgt bald die Unterlaufpruefung.
    while( bBuild )
    {
        sal_Bool bOldNumDone = GetInfo().IsNumDone();
        sal_Bool bOldArrowDone = GetInfo().IsArrowDone();
        GetInfo().SetFtnInside( sal_False );

        // besides other things, this sets the repaint offset to 0
        FormatReset( GetInfo() );

        if( bOldNumDone )
            GetInfo().SetNumDone( sal_True );
        if( bOldArrowDone )
            GetInfo().SetArrowDone( sal_True );

        // build new portions for this line
        BuildPortions( GetInfo() );

        if( GetInfo().IsStop() )
        {
            pCurr->SetLen( 0 );
            pCurr->Height( GetFrmRstHeight() + 1 );
            pCurr->SetRealHeight( GetFrmRstHeight() + 1 );
            pCurr->Width(0);
            pCurr->Truncate();
            return nStart;
        }
        else if( GetInfo().IsDropInit() )
        {
            DropInit();
            GetInfo().SetDropInit( sal_False );
        }

        pCurr->CalcLine( *this, GetInfo() );
        CalcRealHeight( GetInfo().IsNewLine() );

        if ( IsFlyInCntBase() && !IsQuick() )
        {
            KSHORT nTmpAscent, nTmpHeight;
            CalcAscentAndHeight( nTmpAscent, nTmpHeight );
            AlignFlyInCntBase( Y() + long( nTmpAscent ) );
            pCurr->CalcLine( *this, GetInfo() );
            CalcRealHeight();
        }

        // bBuild entscheidet, ob noch eine Ehrenrunde gedreht wird
        if ( pCurr->GetRealHeight() <= GetInfo().GetLineHeight() )
        {
            pCurr->SetRealHeight( GetInfo().GetLineHeight() );
            bBuild = sal_False;
        }
        else
        {
            bBuild = ( GetInfo().GetTxtFly()->IsOn() && ChkFlyUnderflow( GetInfo() )
                     || GetInfo().CheckFtnPortion( pCurr ) );
            if( bBuild )
            {
                GetInfo().SetNumDone( bOldNumDone );
                GetInfo().ResetMaxWidthDiff();

                // delete old rest
                if ( GetInfo().GetRest() )
                {
                    delete GetInfo().GetRest();
                    GetInfo().SetRest( 0 );
                }

                // set original rest portion
                if ( pSaveFld )
                    GetInfo().SetRest( new SwFldPortion( *pSaveFld ) );

                pCurr->SetLen( 0 );
                pCurr->Width(0);
                pCurr->Truncate();
            }
        }
    }

    // calculate optimal repaint rectangle
    if ( bOptimizeRepaint )
    {
        GetInfo().SetPaintOfst( CalcOptRepaint( GetInfo(), pFlyStart ) );
        if ( pFlyStart )
            delete pFlyStart;
    }
    else
        // Special case: We do not allow an optimitation of the repaint
        // area, but during formatting the repaint offset is set to indicate
        // a maximum value for the offset. This value has to be reset:
        GetInfo().SetPaintOfst( 0 );

    // This corrects the start of the reformat range if something has
    // moved to the next line. Otherwise IsFirstReformat in AllowRepaintOpt
    // will give us a wrong result if we have to reformat another line
    GetInfo().GetParaPortion()->GetReformat()->LeftMove( GetInfo().GetIdx() );

    // delete master copy of rest portion
    if ( pSaveFld )
        delete pSaveFld;

    xub_StrLen nNewStart = nStart + pCurr->GetLen();

    // adjust text if kana compression is enabled
    const SwScriptInfo& rSI = GetInfo().GetParaPortion()->GetScriptInfo();

    if ( GetInfo().CompressLine() )
    {
        USHORT nRepaintOfst = CalcKanaAdj( pCurr );

        // adjust repaint offset
        if ( nRepaintOfst < GetInfo().GetPaintOfst() )
            GetInfo().SetPaintOfst( nRepaintOfst );
    }

    CalcAdjustLine( pCurr );

    if( nOldHeight != pCurr->Height() || nOldAscent != pCurr->GetAscent() )
    {
        SetFlyInCntBase();
        GetInfo().SetPaintOfst( 0 ); //geaenderte Zeilenhoehe => kein Recycling
        // alle weiteren Zeilen muessen gepaintet und, wenn Flys im Spiel sind
        // auch formatiert werden.
        GetInfo().SetShift( sal_True );
    }

    if ( IsFlyInCntBase() && !IsQuick() )
        UpdatePos( pCurr, GetTopLeft(), GetStart() );

    return nNewStart;
}

/*************************************************************************
 *                      SwTxtFormatter::FeedInf()
 *************************************************************************/

void SwTxtFormatter::FeedInf( SwTxtFormatInfo &rInf ) const
{
    // 3260, 3860: Fly auf jeden Fall loeschen!
    ClearFly( rInf );
    rInf.Init();

    rInf.ChkNoHyph( CntEndHyph(), CntMidHyph() );
    rInf.SetRoot( pCurr );
    rInf.SetLineStart( nStart );
    rInf.SetIdx( nStart );
    rInf.Left( KSHORT(Left()) );
    rInf.Right( KSHORT(Right()) );
    rInf.First( short(FirstLeft()) );
    rInf.RealWidth( KSHORT(rInf.Right()) - KSHORT(GetLeftMargin()) );
    rInf.Width( rInf.RealWidth() );
    if( ((SwTxtFormatter*)this)->GetRedln() )
    {
        ((SwTxtFormatter*)this)->GetRedln()->Clear( ((SwTxtFormatter*)this)->GetFnt() );
        ((SwTxtFormatter*)this)->GetRedln()->Reset();
    }
}

/*************************************************************************
 *                      SwTxtFormatter::FormatReset()
 *************************************************************************/

void SwTxtFormatter::FormatReset( SwTxtFormatInfo &rInf )
{
    pCurr->Truncate();
    pCurr->Init();
    if( pBlink && pCurr->IsBlinking() )
        pBlink->Delete( pCurr );

    // delete pSpaceAdd und pKanaComp
    pCurr->FinishSpaceAdd();
    pCurr->FinishKanaComp();
    pCurr->ResetFlags();
    FeedInf( rInf );
}

/*************************************************************************
 *                SwTxtFormatter::CalcOnceMore()
 *************************************************************************/

sal_Bool SwTxtFormatter::CalcOnceMore()
{
    if( pDropFmt )
    {
        const KSHORT nOldDrop = GetDropHeight();
        CalcDropHeight( pDropFmt->GetLines() );
        bOnceMore = nOldDrop != GetDropHeight();
    }
    else
        bOnceMore = sal_False;
    return bOnceMore;
}

/*************************************************************************
 *                SwTxtFormatter::CalcBottomLine()
 *************************************************************************/

SwTwips SwTxtFormatter::CalcBottomLine() const
{
    SwTwips nRet = Y() + GetLineHeight();
    SwTwips nMin = GetInfo().GetTxtFly()->GetMinBottom();
    if( nMin && ++nMin > nRet )
    {
        SwTwips nDist = pFrm->Frm().Height() - pFrm->Prt().Height()
                        - pFrm->Prt().Top();
        if( nRet + nDist < nMin )
        {
            sal_Bool bRepaint = HasTruncLines() &&
                GetInfo().GetParaPortion()->GetRepaint()->Bottom() == nRet-1;
            nRet = nMin - nDist;
            if( bRepaint )
            {
                ((SwRepaint*)GetInfo().GetParaPortion()
                    ->GetRepaint())->Bottom( nRet-1 );
                ((SwTxtFormatInfo&)GetInfo()).SetPaintOfst( 0 );
            }
        }
    }
    return nRet;
}

/*************************************************************************
 *                SwTxtFormatter::_CalcFitToContent()
 *************************************************************************/

KSHORT SwTxtFormatter::_CalcFitToContent()
{
    GetInfo().SetRoot( pCurr );

    GetInfo().First( KSHORT(FirstLeft()) );
    GetInfo().Left( KSHORT(Left()) );

    SeekAndChg( GetInfo() );
    GetInfo().SetLast( GetInfo().GetRoot() );

    SwLinePortion *pPor = NewPortion( GetInfo() );

    long nMaxWidth = 0;
    long nWidth = 0;
    long nMargin = FirstLeft();
    sal_Bool bFull = sal_False;
    while( pPor && !IsStop() && !bFull)
    {
        bFull = pPor->Format( GetInfo() );
        GetInfo().SetLast( pPor );
        while( pPor )
        {
            nWidth += pPor->Width();
            pPor->Move( GetInfo() );
            GetInfo().SetLast( pPor );
            pPor = pPor->GetPortion();
        }
        if( bFull && 0 != ( pPor = GetInfo().GetLast() ) && pPor->IsBreakPortion() )
        {
            if ( nWidth && (nMaxWidth < nWidth+nMargin ) )
                nMaxWidth = nWidth + nMargin;
            nWidth = 0;
            nMargin = Left();
            bFull = sal_False;
            GetInfo().X( KSHORT(nMargin) );
        }
        pPor = NewPortion( GetInfo() );
    }
    if ( nWidth && ( nMaxWidth < nWidth + nMargin ) )
        nMaxWidth = nWidth + nMargin;
    return KSHORT(nMaxWidth);
}

/*************************************************************************
 *                      SwTxtFormatter::AllowRepaintOpt()
 *
 * determines if the calculation of a repaint offset is allowed
 * otherwise each line is painted from 0 (this is a copy of the beginning
 * of the former SwTxtFormatter::Recycle() function
 *************************************************************************/
sal_Bool SwTxtFormatter::AllowRepaintOpt( const SwTxtFormatInfo& rInf ) const
{
    // reformat position in front of current line? Only in this case
    // we want to set the repaint offset
    sal_Bool bOptimizeRepaint = IsFirstReformat() && pCurr->GetLen();

    // a special case is the last line of a block adjusted paragraph:
    if ( bOptimizeRepaint )
    {
        switch( GetAdjust() )
        {
        case SVX_ADJUST_BLOCK:
        {
            if( IsLastBlock() || IsLastCenter() )
                bOptimizeRepaint = sal_False;
            else
            {
                // ????: Blank in der letzten Masterzeile (blocksat.sdw)
                bOptimizeRepaint = 0 == pCurr->GetNext() && !pFrm->GetFollow();
                if ( bOptimizeRepaint )
                {
                    SwLinePortion *pPos = pCurr->GetFirstPortion();
                    while ( pPos && !pPos->IsFlyPortion() )
                        pPos = pPos->GetPortion();
                    bOptimizeRepaint = !pPos;
                }
            }
            break;
        }
        case SVX_ADJUST_CENTER:
        case SVX_ADJUST_RIGHT:
            bOptimizeRepaint = sal_False;
            break;
        default: ;
        }
    }

    // Schon wieder ein Sonderfall: unsichtbare SoftHyphs
    const xub_StrLen nReformat = rInf.GetReformatStart();
    if( bOptimizeRepaint && STRING_LEN != nReformat )
    {
        const xub_Unicode cCh = rInf.GetTxt().GetChar( nReformat );
        bOptimizeRepaint = ( CH_TXTATR_BREAKWORD != cCh && CH_TXTATR_INWORD != cCh )
                            || !rInf.HasHint( nReformat );
    }

    return bOptimizeRepaint;
}

/*************************************************************************
 *                      SwTxtFormatter::CalcOptRepaint()
 *
 * calculates an optimal repaint offset for the current line
 *************************************************************************/
long SwTxtFormatter::CalcOptRepaint( SwTxtFormatInfo& rInf,
                                     const SvLongs* pFlyStart )
{
    if ( IsFirstReformat() )
    // the reformat position is behind our new line, that means
    // something of our text has moved to the next line
        return 0;

    xub_StrLen nReformat = rInf.GetReformatStart();

    // in case we do not have any fly in our line, our repaint position
    // is the changed position - 1
    if ( ! pFlyStart && ! pCurr->IsFly() )
    {
        // this is the maximum repaint offset determined during formatting
        // for example: the beginning of the first right tab stop
        // if this value is 0, this means that we do not have an upper
        // limit for the repaint offset
        const long nFormatRepaint = rInf.GetPaintOfst();

        if ( nReformat <= rInf.GetLineStart() )
            return 0;

        // Weird situation: Our line used to end with a hole portion
        // and we delete some characters at the end of our line. We have
        // to take care for repainting the blanks which are not anymore
        // covered by the hole portion
        while ( --nReformat > rInf.GetLineStart() &&
                CH_BLANK == rInf.GetChar( nReformat ) )
            ;

        ASSERT( nReformat < rInf.GetIdx(), "Reformat too small for me!" );
        SwRect aRect;

        // Note: GetChareRect is not const. It definitely changes the
        // bMulti flag. We have to save and resore the old value.
        sal_Bool bOldMulti = rInf.IsMulti();
        GetCharRect( &aRect, nReformat );
        rInf.SetMulti( bOldMulti );

        return nFormatRepaint ? Min( aRect.Left(), nFormatRepaint ) :
                                aRect.Left();
    }
    else
    {
        // nReformat may be wrong, if something around flys has changed:
        // we compare the former and the new fly positions in this line
        // if anything has changed, we carefully have to adjust the right
        // repaint position
        long nPOfst = 0;
        USHORT nCnt = 0;
        USHORT nX = 0;
        USHORT nIdx = rInf.GetLineStart();
        SwLinePortion* pPor = pCurr->GetFirstPortion();

        while ( pPor )
        {
            if ( pPor->IsFlyPortion() )
            {
                // compare start of fly with former start of fly
                if ( pFlyStart &&
                     nCnt < pFlyStart->Count() &&
                     nX == (*pFlyStart)[ nCnt ] &&
                     nIdx < nReformat
                   )
                    // found fix position, nothing has changed left from nX
                    nPOfst = nX + pPor->Width();
                else
                    break;

                nCnt++;
            }
            nX += pPor->Width();
            nIdx += pPor->GetLen();
            pPor = pPor->GetPortion();
        }

        return nPOfst + GetLeftMargin();
    }
}


