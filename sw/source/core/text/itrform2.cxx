/*************************************************************************
 *
 *  $RCSfile: itrform2.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 16:27:07 $
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
#include <guess.hxx>        // Recycle()
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
    // restore hyphenation options if necessary
    if (GetInfo().IsRestoreHyphOptions())
    {
        GetInfo().RestoreHyphOptions();
    }

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
 *                      SwTxtFormatter::Recycle()
 *************************************************************************/

/* Recycle dient nicht nur der Optimierung, sondern soll gleichzeitig
 * unsere Probleme mit dem WYSIWYG (flackern und tanzen) loesen helfen.
 * Recycle betrachtet den Bereich einer Zeile _vor_ nReformat.
 * 4 Faelle koennen auftreten:
 * 1) pCurr ist hinter nReformat
 *    Init, sal_False
 * 2) pCurr wurde nagelneu angelegt
 *    -> pPos == 0, sal_False
 * 3) pCurr hatte Portions und nReformat liegt in der Zeile
 *    -> pPos != 0, sal_True
 * 4) pCurr hatte Portions und nReformat liegt aber _nicht_ in der Zeile
 *    -> pPos == 0, sal_True
 * Fall 4 sollte u.U. in ein ASSERT laufen.
 *
 * sal_True, wenn recyclet wurde ...
 */

void SwTxtFormatter::Recycle( SwTxtFormatInfo &rInf )
{
#ifndef PRODUCT
    // Durch das 0 setzen wird Recycle() ausgelassen.
    if( OPTLOW( rInf ) )
        pCurr->SetLen(0);
#endif
    // GetRest() liefert den (Feld-)Ueberhang der vorigen Zeile zurueck.
    // Dann gibt es natuerlich nichts zu recyclen ..
    if( rInf.GetRest() || HasChanges() )
    {
        FormatReset( rInf );
        return;
    }

    // Optimierung fuer Zeilen, die aus dem Rennen sind
    xub_StrLen nReformat = rInf.GetReformatStart();

    // Bei rechtsbuendig, zentriert und Blocksatz wird returnt ...
    sal_Bool bRecycle = IsFirstReformat() && pCurr->GetLen();
    if( bRecycle )
    {
        switch( GetAdjust() )
        {
            case SVX_ADJUST_BLOCK:
            {
                if( IsLastBlock() || IsLastCenter() )
                    bRecycle = sal_False;
                else
                {
                    // ????: Blank in der letzten Masterzeile (blocksat.sdw)
                    bRecycle = 0 == pCurr->GetNext() && !pFrm->GetFollow();
                    if ( bRecycle )
                    {
                        SwLinePortion *pPos = pCurr->GetFirstPortion();
                        while ( pPos && !pPos->IsFlyPortion() )
                            pPos = pPos->GetPortion();
                        bRecycle = !pPos;
                    }
                }
            }
            break;
            case SVX_ADJUST_CENTER:
            case SVX_ADJUST_RIGHT:
            {
                bRecycle = sal_False;
            }
            break;
            default: ;
        }
    }

    // Schon wieder ein Sonderfall: unsichtbare SoftHyphs
    if( bRecycle && STRING_LEN != nReformat )
    {
        const xub_Unicode cCh = rInf.GetTxt().GetChar( nReformat );
        bRecycle = ( CH_TXTATR_BREAKWORD != cCh && CH_TXTATR_INWORD != cCh )
                    || !rInf.HasHint( nReformat );
    }

    if( !bRecycle )
    {
        FormatReset( rInf );
        return;
    }

    // alle Portions retten, die vor nReformat liegen.
    // pPos kann nie 0 werden.
    SwLinePortion *pPos = pCurr->GetFirstPortion();
    SwLinePortion *pLast = pPos;

    // bTabFlag warnt uns vor rechtsbdg. Tabs
    sal_Bool bTabFlag = pPos->InTabGrp() && !pPos->IsTabLeftPortion();

    while( pPos->GetPortion() && rInf.GetIdx() + pPos->GetLen() < nReformat )
    {
        DBG_LOOP;
        pPos->Move( rInf );
        pLast = pPos;
        pPos = pPos->GetPortion();

        if ( bTabFlag )
            bTabFlag = !( pLast->IsTabLeftPortion() || pLast->IsFlyPortion() );

        bTabFlag = bTabFlag || (pPos->InTabGrp() && !pPos->IsTabLeftPortion());
        if( pPos->IsFtnPortion() )
            rInf.SetFtnInside( sal_True );
    }

    // bBrkBefore ist sal_True, wenn die BrkPos vor oder auf nReformat liegt,
    // d.h. dass die pPos in jedem Fall neuformatiert werden muss.
    SwTxtGuess aGuess;
    rInf.SetLen( nReformat );

    // 6736: Worte hinter Flys, Blank einfuegen.
    // 6820: Rechtstabs und Blanks
    xub_StrLen nPrevEnd = nReformat ?
        aGuess.GetPrevEnd( rInf, nReformat - 1 ) + 1: 0;
    if ( 1 == nPrevEnd )
        --nPrevEnd;

    const sal_Bool bBrkBefore = bTabFlag || pLast->InTabnLftGrp() ||
        pPos->IsQuoVadisPortion() ||
        ( pLast->IsFlyPortion() && (!nReformat || rInf.GetIdx() >= nPrevEnd) );
    // Wenn pLast nicht recyclebar ist (R/Z/D-Tabs und SoftHyphs etc)
    // dann muss diese Portion mit in die Neuformatierung aufgenommen
    // werden. D.h: pLast wandert um einen zurueck und rInf muss
    // um den entsprechenden Betrag zurueckgestellt werden.

    if( bBrkBefore )
    {
        // 13713: Bei nicht recyclebaren Tabs (rechtsbdg. etc.) muss man
        // bis zur letzten Nichttextportion zurueckgehen!
        if( pPos == pLast )
            pLast = pLast->FindPrevPortion( pCurr );
        while( pLast != pCurr &&
              ( !pLast->MayRecycle() || pPos->IsFlyPortion() || bTabFlag ) )
        {
            if ( bTabFlag )
                bTabFlag = pLast->InTxtGrp();
            else
                bTabFlag = pLast->InTabGrp();

            rInf.SetIdx( rInf.GetIdx() - pLast->GetLen() );
            rInf.X( rInf.X() - pLast->Width() );
            pPos = pLast;
            pLast = pLast->FindPrevPortion( pCurr );
        }
    }

    // Wunder der Technik: der PaintOfst ist die Position in der
    // Zeile, wo es gleich weiter geht.
    long nPOfst = 0;

    if ( pPos->InTxtGrp() && rInf.GetIdx() < nPrevEnd &&
         rInf.GetIdx() + pPos->GetLen() > nPrevEnd )
    {
        SwRect aRect;
        GetCharRect( &aRect, nPrevEnd );
        nPOfst = aRect.Left();
    }
    else
        nPOfst = rInf.X() + GetLeftMargin();

// AMA-Test: Problem, wenn diese Portion in die naechste Zeile rutscht,
// behauptet IsFirstReformat, dass die Zeile recycled werden kann, weil
// Reformat-Start in der Zeile liegt ( aber erst durch das Rutschen! ).
// Loesung: wir drehen am Reformat-Start
    rInf.GetParaPortion()->GetReformat()->LeftMove( rInf.GetIdx() );
#ifdef DEBUG
    SwTwips nOffset =  rInf.X() + GetLeftMargin();
#endif
    pPos = pLast == pPos ? pCurr : pLast;

    // nach pPos die Sintflut
    pPos->Truncate();

    // 9118: pLast ist eine NumPortion, SetNumDone ist nicht sal_True
    // Alternative: alle Flags setzen...
    if( pPos == pCurr || !rInf.GetIdx() || pPos->IsErgoSumPortion() )
    {
        if( pPos->IsGrfNumPortion() )
        {
            if( nReformat && !((SwGrfNumPortion*)pPos)->IsHide() )
                rInf.SetNumDone( sal_True );
            else
            {
                nPOfst = 0;
                FormatReset( rInf );
            }
        }
        else
            FormatReset( rInf );
    }

    if ( nPOfst )
        rInf.SetPaintOfst( nPOfst );
    rInf.SetLen(0);

    // Wir muessen die pCurr-Daten, die sonst waehrend der Fahrt
    // in NewTxtPortion() ermittelt werden, auf Vordermann bringen.
    pLast = pCurr->GetPortion();
    if( pLast )
    {
        pCurr->Init( pLast );
        while( pLast )
        {
            DBG_LOOP;
            if( pCurr->Height() < pLast->Height() )
                pCurr->Height( pLast->Height() );
            if( pCurr->GetAscent() < pLast->GetAscent() )
                pCurr->SetAscent( pLast->GetAscent() );
            pLast = pLast->GetPortion();
        }
    }
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
            if( pPor->Width() || pPor->IsSoftHyphPortion() )
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
        // Der PaintOfst muss zurueckgesetzt werden.
        // Unterlaufsituation in Kombination mit Recycle()
        if( pCurr->Height() < pPor->Height() )
        {
            pCurr->Height( pPor->Height() );
            if( IsFirstReformat() )
                rInf.SetPaintOfst(0);
        }
        if( pCurr->GetAscent() < pPor->GetAscent() )
        {
            pCurr->SetAscent( pPor->GetAscent() );
            if( IsFirstReformat() )
                rInf.SetPaintOfst(0);
        }
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

    // Es durchaus sein, dass pCurr durch Recycle() Daten enthaelt.
    // Erst NewTxtPortion() entscheidet, ob pCurr in pPor landet.
    // Wir muessen in jedem Fall dafuer sorgen, dass der Font eingestellt
    // wird. In CalcAscent geschieht dies automatisch.
    rInf.SetLast( pCurr->FindLastPortion() );
    rInf.ForcedLeftMargin( 0 );
    if( rInf.GetLast() == pCurr )
    {
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
        BYTE nNxtActual;
        if( pPor->InFldGrp() && !pPor->InNumberGrp() )
        {
            ((SwFldPortion*)pPor)->CheckScript( rInf );
            const SwFont* pTmpFnt = ((SwFldPortion*)pPor)->GetFont();
            if( !pTmpFnt )
                pTmpFnt = rInf.GetFont();
            nNxtActual = pTmpFnt->GetActual();
        }
        else
            nNxtActual = rInf.GetFont()->GetActual();
        if( rInf.GetLast() && rInf.GetLast()->InTxtGrp() &&
            rInf.GetLast()->Width() && !rInf.GetLast()->InNumberGrp() )
        {
            BYTE nLstActual;
            if( rInf.GetLast()->InFldGrp() &&
                ((SwFldPortion*)rInf.GetLast())->GetFont() )
                nLstActual = ((SwFldPortion*)rInf.GetLast())->GetFont()->
                             GetActual();
            else
                nLstActual = rInf.GetFont()->GetActual();
            if( nNxtActual != nLstActual )
            {
                const SwDoc *pDoc = GetTxtFrm()->GetTxtNode()->GetDoc();
                USHORT nDist;
                if( SW_LATIN == nNxtActual || SW_LATIN == nLstActual )
                {
                    if( SW_CJK == nNxtActual || SW_CJK == nLstActual )
                        nDist = pDoc->GetLatin_CJK();
                    else
                        nDist = pDoc->GetLatin_CTL();
                }
                else
                    nDist = pDoc->GetCJK_CTL();
                if( nDist )
                {
                    SwKernPortion* pKrn =
                        new SwKernPortion(*rInf.GetLast(), nDist);
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

        // Vorsicht: ein Fly im Blocksatz, dann kann das Repaint nur komplett
        // hinter ihm oder vom Zeilenbeginn sein.
#ifdef DEBUG
        KSHORT nWhere = rInf.X();
        long nLeft = GetLeftMargin();
        SwTwips nPaintOfs = rInf.GetPaintOfst();
#endif
        if ( pPor->IsFlyPortion() && ( SVX_ADJUST_BLOCK == GetAdjust() )
             && ( rInf.X() + GetLeftMargin() >= rInf.GetPaintOfst() ) )
            rInf.SetPaintOfst( 0 );

        if ( pPor->IsFlyCntPortion() )
            SetFlyInCntBase();

        rInf.SetFull( bFull );
        // 5964: bUnderFlow muss zurueckgesetzt werden, sonst wird beim
        //       naechsten Softhyphen wieder umgebrochen!
        if ( !bFull )
        {
            rInf.ClrUnderFlow();
            if( pPor->InTxtGrp() && pPor->GetLen() && !pPor->InFldGrp() )
            {
                xub_StrLen nTmp = rInf.GetIdx() + pPor->GetLen();
                // For the moment I insert a distance from 1/2 cm between
                // two different scripts. This value must be replaced by
                // the right document setting, perhaps depending on both scripts
                if( nTmp == NextScriptChg( nTmp - 1 ) )
                {
                    const SwDoc *pDoc = GetTxtFrm()->GetTxtNode()->GetDoc();
                    USHORT nDist;
                    USHORT nScript = ScriptType( nTmp );
                    if( SW_LATIN == nNxtActual || ScriptType::LATIN == nScript )
                    {
                        if( SW_CJK == nNxtActual || ScriptType::ASIAN==nScript )
                            nDist = pDoc->GetLatin_CJK();
                        else
                            nDist = pDoc->GetLatin_CTL();
                    }
                    else
                        nDist = pDoc->GetCJK_CTL();
                    if( nDist )
                        new SwKernPortion( *pPor, nDist );
                }
            }
        }

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
        else if( rInf.GetLast() && rInf.GetLast()->IsKernPortion() )
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
            UpdatePos( pCurr, sal_True );
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

    // At the end of a multi-line part we've to break.
    if( GetMulti() && nNextChg > GetMulti()->GetLen() )
        nNextChg = GetMulti()->GetLen();

    nNextAttr = NextScriptChg( rInf.GetIdx() );
    if( nNextChg > nNextAttr )
        nNextChg = nNextAttr;
    if ( nNextChg > 1 + rInf.GetIdx() && ' ' == rInf.GetChar( nNextChg-1 ) )
        --nNextChg;

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
            const KSHORT nWidth =
                  rInf.GetTxtSize(rInf.GetIdx(), MAX_TXTPORLEN ).Width();
            if( nWidth > rInf.Width() )
                nNextChg = Min( nMaxChg, rInf.GetTxt().Len() );
        }
    }
    nNextChg = rInf.ScanPortionEnd( nNextChg );
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
    else
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
        // 3) Die DropCaps
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

        // Wenn die Zeile voll ist und ein Blank am Zeilenende
        // steht, dann muss es in einer HolePortion verborgen werden
        // (trailing blank).
        // 8518: keine HolePortions bei mehrzeiligen Feldern.
        if( rInf.GetIdx() < rInf.GetTxt().Len() &&
            rInf.GetIdx() && ' ' == rInf.GetChar( rInf.GetIdx() )
            && !rInf.GetLast()->IsHolePortion() && !rInf.GetRest()
            && CH_BREAK != rInf.GetChar( rInf.GetIdx() - 1 ) )
        {
            SwHolePortion *pHole = new SwHolePortion( *rInf.GetLast() );
            xub_StrLen nCnt = rInf.GetIdx();
            xub_StrLen nLen = rInf.GetTxt().Len();
            while ( nCnt < nLen && ' ' == rInf.GetChar( nCnt ) )
                nCnt++;
            pHole->SetLen( nCnt - rInf.GetIdx() );
            return pHole;
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
            const SwTxtAttr* pTwoLines = rInf.GetMultiAttr( nEnd );
            if( pTwoLines )
            {
                SwMultiPortion* pTmp = NULL;
                if( RES_TXTATR_CJK_RUBY == pTwoLines->Which() )
                    pTmp = new SwRubyPortion( *pTwoLines,*rInf.GetFont(),nEnd );
                else
                    pTmp = new SwDoubleLinePortion( *pTwoLines, nEnd );
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
    GetInfo().SetPaintOfst( 0 );

    // Recycling muss bei geaenderter Zeilenhoehe unterdrueckt werden
    // und auch bei geaendertem Ascent (Absenken der Grundlinie).
    const KSHORT nOldHeight = pCurr->Height();
    const KSHORT nOldAscent = pCurr->GetAscent();

    pCurr->SetEndHyph( sal_False );
    pCurr->SetMidHyph( sal_False );

    // Hier folgt bald die Unterlaufpruefung.
    while( bBuild )
    {
        sal_Bool bOldNumDone = GetInfo().IsNumDone();
        sal_Bool bOldArrowDone = GetInfo().IsArrowDone();
        GetInfo().SetFtnInside( sal_False );
        FeedInf( GetInfo() );
        Recycle( GetInfo() );   // initialisiert sich oder rettet Portions
        if( bOldNumDone )
            GetInfo().SetNumDone( sal_True );
        if( bOldArrowDone )
            GetInfo().SetArrowDone( sal_True );
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

        pCurr->CalcLine( *this );
        CalcRealHeight( GetInfo().IsNewLine() );

        if ( IsFlyInCntBase() && !IsQuick() )
        {
            KSHORT nTmpAscent, nTmpHeight;
            CalcAscentAndHeight( nTmpAscent, nTmpHeight );
            AlignFlyInCntBase( Y() + long( nTmpAscent ) );
            pCurr->CalcLine( *this );
            CalcRealHeight();
        }

        // bBuild entscheidet, ob noch eine Ehrenrunde gedreht wird
        bBuild = !GetInfo().GetLineHeight() &&
                ( GetInfo().GetTxtFly()->IsOn() && ChkFlyUnderflow( GetInfo() )
                || GetInfo().CheckFtnPortion( pCurr ) );
        if( bBuild )
        {
            GetInfo().SetNumDone( bOldNumDone );
            pCurr->SetLen( 0 );
            pCurr->Width(0);
            pCurr->Truncate();
        }
    }

    xub_StrLen nNewStart = nStart + pCurr->GetLen();
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
        UpdatePos( pCurr );

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

