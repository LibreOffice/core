/*************************************************************************
 *
 *  $RCSfile: widorp.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hjs $ $Date: 2003-09-25 10:49:59 $
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


#pragma hdrstop

#include "hintids.hxx"

#include "frmsh.hxx"
#include "layfrm.hxx"
#include "ftnboss.hxx"
#include "ndtxt.hxx"
#include "paratr.hxx"
#ifndef PRODUCT
#include "viewsh.hxx"   // ViewShell
#include "viewopt.hxx"  // SwViewOption
#endif

#ifndef _SVX_ORPHITEM_HXX //autogen
#include <svx/orphitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif

#include "txtcfg.hxx"
#include "widorp.hxx"
#include "txtfrm.hxx"
#include "itrtxt.hxx"
#include "sectfrm.hxx"  //SwSectionFrm
#include "ftnfrm.hxx"

#undef WIDOWTWIPS


/*************************************************************************
 *                  inline IsNastyFollow()
 *************************************************************************/
// Ein Follow, der auf der selben Seite steht, wie sein Master ist nasty.
inline sal_Bool IsNastyFollow( const SwTxtFrm *pFrm )
{
    ASSERT( !pFrm->IsFollow() || !pFrm->GetPrev() ||
            ((const SwTxtFrm*)pFrm->GetPrev())->GetFollow() == pFrm,
            "IsNastyFollow: Was ist denn hier los?" );
    return  pFrm->IsFollow() && pFrm->GetPrev();
}

/*************************************************************************
 *                  SwTxtFrmBreak::SwTxtFrmBreak()
 *************************************************************************/

SwTxtFrmBreak::SwTxtFrmBreak( SwTxtFrm *pFrm, const SwTwips nRst )
    : pFrm(pFrm), nRstHeight(nRst)
{
    SWAP_IF_SWAPPED( pFrm )
    SWRECTFN( pFrm )
    nOrigin = (pFrm->*fnRect->fnGetPrtTop)();
    SwSectionFrm* pSct;
    bKeep = !pFrm->IsMoveable() || IsNastyFollow( pFrm ) ||
            ( pFrm->IsInSct() && (pSct=pFrm->FindSctFrm())->Lower()->IsColumnFrm()
              && !pSct->MoveAllowed( pFrm ) ) ||
            !pFrm->GetTxtNode()->GetSwAttrSet().GetSplit().GetValue() ||
            pFrm->GetTxtNode()->GetSwAttrSet().GetKeep().GetValue();
    bBreak = sal_False;

    if( !nRstHeight && !pFrm->IsFollow() && pFrm->IsInFtn() && pFrm->HasPara() )
    {
        nRstHeight = pFrm->GetFtnFrmHeight();
        nRstHeight += (pFrm->Prt().*fnRect->fnGetHeight)() -
                      (pFrm->Frm().*fnRect->fnGetHeight)();
        if( nRstHeight < 0 )
            nRstHeight = 0;
    }

    UNDO_SWAP( pFrm )
}

/* BP 18.6.93: Widows.
 * Im Gegensatz zur ersten Implementierung werden die Widows nicht
 * mehr vorausschauend berechnet, sondern erst beim Formatieren des
 * gesplitteten Follows festgestellt. Im Master faellt die Widows-
 * Berechnung also generell weg (nWidows wird manipuliert).
 * Wenn der Follow feststellt, dass die Widowsregel zutrifft,
 * verschickt er an seinen Vorgaenger ein Prepare.
 * Ein besonderes Problem ergibt sich, wenn die Widows zuschlagen,
 * aber im Master noch ein paar Zeilen zur Verfuegung stehen.
 *
 */

/*************************************************************************
 *                  SwTxtFrmBreak::IsInside()
 *************************************************************************/

/* BP(22.07.92): Berechnung von Witwen und Waisen.
 * Die Methode liefert sal_True zurueck, wenn eine dieser Regelung zutrifft.
 *
 * Eine Schwierigkeit gibt es im Zusammenhang mit Widows und
 * unterschiedlichen Formaten zwischen Master- und Folgeframes:
 * Beispiel: Wenn die erste Spalte 3cm und die zweite 4cm breit ist
 * und Widows auf sagen wir 3 gesetzt ist, so ist erst bei der Formatierung
 * des Follows entscheidbar, ob die Widowsbedingung einhaltbar ist oder
 * nicht. Leider ist davon abhaengig, ob der Absatz als Ganzes auf die
 * naechste Seite rutscht.
 */

const sal_Bool SwTxtFrmBreak::IsInside( SwTxtMargin &rLine ) const
{
    register sal_Bool bFit = sal_False;

    SWAP_IF_SWAPPED( pFrm )
    SWRECTFN( pFrm )
    // nOrigin is an absolut value, rLine referes to the swapped situation.

    SwTwips nTmpY;
    if ( pFrm->IsVertical() )
        nTmpY = pFrm->SwitchHorizontalToVertical( rLine.Y() + rLine.GetLineHeight() );
    else
        nTmpY = rLine.Y() + rLine.GetLineHeight();

    SwTwips nLineHeight = (*fnRect->fnYDiff)( nTmpY , nOrigin );

    // 7455 und 6114: Raum fuer die Umrandung unten einkalkulieren.
    nLineHeight += (pFrm->*fnRect->fnGetBottomMargin)();


    if( nRstHeight )
        bFit = nRstHeight >= nLineHeight;
    else
    {
        // Der Frm besitzt eine Hoehe, mit der er auf die Seite passt.
        SwTwips nHeight =
            (*fnRect->fnYDiff)( (pFrm->GetUpper()->*fnRect->fnGetPrtBottom)(), nOrigin );

        // Wenn sich alles innerhalb des bestehenden Frames abspielt,
        // ist das Ergebnis sal_True;
        bFit = nHeight >= nLineHeight;
        if( !bFit )
        {
            // Die LineHeight sprengt die aktuelle Frm-Hoehe.
            // Nun rufen wir ein Probe-Grow, um zu ermitteln, ob der
            // Frame um den gewuenschten Bereich wachsen wuerde.
            nHeight += pFrm->GrowTst( LONG_MAX );

            // Das Grow() returnt die Hoehe, um die der Upper des TxtFrm
            // den TxtFrm wachsen lassen wuerde.
            // Der TxtFrm selbst darf wachsen wie er will.
            bFit = nHeight >= nLineHeight;
        }
    }

    UNDO_SWAP( pFrm );

    return bFit;
}

/*************************************************************************
 *                  SwTxtFrmBreak::IsBreakNow()
 *************************************************************************/

sal_Bool SwTxtFrmBreak::IsBreakNow( SwTxtMargin &rLine )
{
    SWAP_IF_SWAPPED( pFrm )

    // bKeep ist staerker als IsBreakNow()
    // Ist noch genug Platz ?
    if( bKeep || IsInside( rLine ) )
        bBreak = sal_False;
    else
    {
        /* Diese Klasse geht davon aus, dass der SwTxtMargin von Top nach Bottom
         * durchgearbeitet wird. Aus Performancegruenden wird in folgenden
         * Faellen der Laden fuer das weitere Aufspalten dicht gemacht:
         * Wenn eine einzige Zeile nicht mehr passt.
         * Sonderfall: bei DummyPortions ist LineNr == 1, obwohl wir splitten
         * wollen.
         */
        // 6010: DropLines mit einbeziehen

        sal_Bool bFirstLine = 1 == rLine.GetLineNr() && !rLine.GetPrev();
        bBreak = sal_True;
        if( ( bFirstLine && pFrm->GetIndPrev() )
            || ( rLine.GetLineNr() <= rLine.GetDropLines() ) )
        {
            bKeep = sal_True;
            bBreak = sal_False;
        }
        else if(bFirstLine && pFrm->IsInFtn() && !pFrm->FindFtnFrm()->GetPrev())
        {
            SwLayoutFrm* pTmp = pFrm->FindFtnBossFrm()->FindBodyCont();
            if( !pTmp || !pTmp->Lower() )
                bBreak = sal_False;
        }
    }

    UNDO_SWAP( pFrm )

    return bBreak;
}


/*MA ehemals fuer COMPACT
// WouldFit() liefert sal_True, wenn der Absatz ganz oder teilweise passen wuerde

sal_Bool SwTxtFrmBreak::WouldFit( SwTxtMargin &rLine )
{
    rLine.Bottom();
    if( IsInside( rLine ) )
        return sal_True;

    rLine.Top();
    // Suche die erste Trennmoeglichkeit ...
    while( !IsBreakNow( rLine ) )
    {
        DBG_LOOP;
        if( !rLine.NextLine() )
            return sal_False;
    }
    return sal_True;
}
*/

/*************************************************************************
 *                  WidowsAndOrphans::WidowsAndOrphans()
 *************************************************************************/

WidowsAndOrphans::WidowsAndOrphans( SwTxtFrm *pFrm, const SwTwips nRst,
    sal_Bool bChkKeep   )
    : SwTxtFrmBreak( pFrm, nRst ), nOrphLines( 0 ), nWidLines( 0 )
{
    SWAP_IF_SWAPPED( pFrm )
    SWRECTFN( pFrm )

    if( bKeep )
    {
        // 5652: bei Absaetzen, die zusammengehalten werden sollen und
        // groesser sind als die Seite wird bKeep aufgehoben.
        if( bChkKeep && !pFrm->GetPrev() && !pFrm->IsInFtn() &&
            pFrm->IsMoveable() &&
            ( !pFrm->IsInSct() || pFrm->FindSctFrm()->MoveAllowed(pFrm) ) )
            bKeep = sal_False;
        //Auch bei gesetztem Keep muessen Orphans beachtet werden,
        //z.B. bei verketteten Rahmen erhaelt ein Follow im letzten Rahmen ein Keep,
        //da er nicht (vorwaerts) Moveable ist,
        //er darf aber trotzdem vom Master Zeilen anfordern wg. der Orphanregel.
        if( pFrm->IsFollow() )
            nWidLines = pFrm->GetTxtNode()->GetSwAttrSet().GetWidows().GetValue();
    }
    else
    {
        const SwAttrSet& rSet = pFrm->GetTxtNode()->GetSwAttrSet();
        const SvxOrphansItem  &rOrph = rSet.GetOrphans();
        if ( rOrph.GetValue() > 1 )
            nOrphLines = rOrph.GetValue();
        if ( pFrm->IsFollow() )
            nWidLines = rSet.GetWidows().GetValue();

    }
    if( pFrm->IsInFtn() && !pFrm->GetIndPrev() &&
        ( bKeep || nWidLines || nOrphLines ) )
    {
        // Innerhalb von Fussnoten gibt es gute Gruende, das Keep-Attribut und
        // die Widows/Orphans abzuschalten.
        SwFtnFrm *pFtn = pFrm->FindFtnFrm();
        sal_Bool bFt = !pFtn->GetAttr()->GetFtn().IsEndNote();
        if( !pFtn->GetPrev() &&
            pFtn->FindFtnBossFrm( bFt ) != pFtn->GetRef()->FindFtnBossFrm( bFt )
            && ( !pFrm->IsInSct() || pFrm->FindSctFrm()->MoveAllowed(pFrm) ) )
        {
            bKeep = sal_False;
            nOrphLines = 0;
            nWidLines = 0;
        }
    }

    UNDO_SWAP( pFrm )
}

/*************************************************************************
 *                  WidowsAndOrphans::FindBreak()
 *************************************************************************/

/* Die Find*-Methoden suchen nicht nur, sondern stellen den SwTxtMargin auf
 * die Zeile ein, wo der Absatz gebrochen werden soll und kuerzen ihn dort.
 * FindBreak()
 */

sal_Bool WidowsAndOrphans::FindBreak( SwTxtFrm *pFrame, SwTxtMargin &rLine,
    sal_Bool bHasToFit )
{
    SWAP_IF_SWAPPED( pFrm )

    sal_Bool bRet = sal_True;
    MSHORT nOldOrphans = nOrphLines;
    if( bHasToFit )
        nOrphLines = 0;
    rLine.Bottom();
    if( !IsBreakNow( rLine ) )
        bRet = sal_False;
    if( !FindWidows( pFrame, rLine ) )
    {
        sal_Bool bBack = sal_False;
        while( IsBreakNow( rLine ) )
        {
            if( rLine.PrevLine() )
                bBack = sal_True;
            else
                break;
        }
        // Eigentlich werden bei HasToFit Schusterjungen (Orphans) nicht
        // beruecksichtigt, wenn allerdings Dummy-Lines im Spiel sind und
        // die Orphansregel verletzt wird, machen wir mal eine Ausnahme:
        // Wir lassen einfach eine Dummyline zurueck und wandern mit dem Text
        // komplett auf die naechste Seite/Spalte.
        if( rLine.GetLineNr() <= nOldOrphans &&
            rLine.GetInfo().GetParaPortion()->IsDummy() &&
            ( ( bHasToFit && bRet ) || SwTxtFrmBreak::IsBreakNow( rLine ) ) )
            rLine.Top();

        rLine.TruncLines( sal_True );
        bRet = bBack;
    }
    nOrphLines = nOldOrphans;

    UNDO_SWAP( pFrm )

    return bRet;
}

/*************************************************************************
 *                  WidowsAndOrphans::FindWidows()
 *************************************************************************/

/*  FindWidows positioniert den SwTxtMargin des Masters auf die umzubrechende
 *  Zeile, indem der Follow formatiert und untersucht wird.
 *  Liefert sal_True zurueck, wenn die Widows-Regelung in Kraft tritt,
 *  d.h. der Absatz _zusammengehalten_ werden soll !
 */

sal_Bool WidowsAndOrphans::FindWidows( SwTxtFrm *pFrm, SwTxtMargin &rLine )
{
    ASSERT( ! pFrm->IsVertical() || ! pFrm->IsSwapped(),
            "WidowsAndOrphans::FindWidows with swapped frame" )

    if( !nWidLines || !pFrm->IsFollow() )
        return sal_False;

    rLine.Bottom();

    // Wir koennen noch was abzwacken
    SwTxtFrm *pMaster = pFrm->FindMaster();
    ASSERT(pMaster, "+WidowsAndOrphans::FindWidows: Widows in a master?");
    if( !pMaster )
        return sal_False;

    // 5156: Wenn die erste Zeile des Follows nicht passt, wird der Master
    // wohl voll mit Dummies sein. In diesem Fall waere ein PREP_WIDOWS fatal.
    if( pMaster->GetOfst() == pFrm->GetOfst() )
        return sal_False;

    // Resthoehe des Masters
    SWRECTFN( pFrm )

    const SwTwips nDocPrtTop = (pFrm->*fnRect->fnGetPrtTop)();
    SwTwips nOldHeight;
    SwTwips nTmpY = rLine.Y() + rLine.GetLineHeight();

    if ( bVert )
    {
        nTmpY = pFrm->SwitchHorizontalToVertical( nTmpY );
        nOldHeight = -(pFrm->Prt().*fnRect->fnGetHeight)();
    }
    else
        nOldHeight = (pFrm->Prt().*fnRect->fnGetHeight)();

    const SwTwips nChg = (*fnRect->fnYDiff)( nTmpY, nDocPrtTop + nOldHeight );

    // Unterhalb der Widows-Schwelle...
    if( rLine.GetLineNr() >= nWidLines )
    {
        // 8575: Follow to Master I
        // Wenn der Follow *waechst*, so besteht fuer den Master die Chance,
        // Zeilen entgegenzunehmen, die er vor Kurzem gezwungen war an den
        // Follow abzugeben: Prepare(Need); diese Abfrage unterhalb von nChg!
        // (0W, 2O, 2M, 2F) + 1F = 3M, 2F
        if( rLine.GetLineNr() > nWidLines && pFrm->IsJustWidow() )
        {
            // Wenn der Master gelockt ist, so hat er vermutlich gerade erst
            // eine Zeile an uns abgegeben, diese geben nicht zurueck, nur
            // weil bei uns daraus mehrere geworden sind (z.B. durch Rahmen).
            if( !pMaster->IsLocked() && pMaster->GetUpper() )
            {
                const SwTwips nRstHeight = (pMaster->Frm().*fnRect->fnBottomDist)
                            ( (pMaster->GetUpper()->*fnRect->fnGetPrtBottom)() );
                if ( nRstHeight >=
                     SwTwips(rLine.GetInfo().GetParaPortion()->Height() ) )
                {
                    pMaster->Prepare( PREP_ADJUST_FRM );
                    pMaster->_InvalidateSize();
                    pMaster->InvalidatePage();
                }
            }

            pFrm->SetJustWidow( sal_False );
        }
        return sal_False;
    }

    // 8575: Follow to Master II
    // Wenn der Follow *schrumpft*, so besteht fuer den Master die Chance,
    // den kompletten Orphan zu inhalieren.
    // (0W, 2O, 2M, 1F) - 1F = 3M, 0F     -> PREP_ADJUST_FRM
    // (0W, 2O, 3M, 2F) - 1F = 2M, 2F     -> PREP_WIDOWS

    if( 0 > nChg && !pMaster->IsLocked() && pMaster->GetUpper() )
    {
        SwTwips nRstHeight = (pMaster->Frm().*fnRect->fnBottomDist)
                             ( (pMaster->GetUpper()->*fnRect->fnGetPrtBottom)() );
        if( nRstHeight >= SwTwips(rLine.GetInfo().GetParaPortion()->Height() ) )
        {
            pMaster->Prepare( PREP_ADJUST_FRM );
            pMaster->_InvalidateSize();
            pMaster->InvalidatePage();
            pFrm->SetJustWidow( sal_False );
            return sal_False;
        }
    }

    // Master to Follow
    // Wenn der Follow nach seiner Formatierung weniger Zeilen enthaelt
    // als Widows, so besteht noch die Chance, einige Zeilen des Masters
    // abzuzwacken. Wenn dadurch die Orphans-Regel des Masters in Kraft
    // tritt muss im CalcPrep() des Master-Frame der Frame so vergroessert
    // werden, dass er nicht mehr auf seine urspruengliche Seite passt.
    // Wenn er noch ein paar Zeilen entbehren kann, dann muss im CalcPrep()
    // ein Shrink() erfolgen, der Follow mit dem Widows rutscht dann auf
    // die Seite des Masters, haelt sich aber zusammen, so dass er (endlich)
    // auf die naechste Seite rutscht. - So die Theorie!


    // Wir fordern nur noch ein Zeile zur Zeit an, weil eine Zeile des Masters
    // bei uns durchaus mehrere Zeilen ergeben koennten.
    // Dafuer behaelt CalcFollow solange die Kontrolle, bis der Follow alle
    // notwendigen Zeilen bekommen hat.
    MSHORT nNeed = 1; // frueher: nWidLines - rLine.GetLineNr();

    // Special case: Master cannot give lines to follow
    if ( ! pMaster->GetIndPrev() && pMaster->GetThisLines() <= nNeed )
        return sal_False;

    pMaster->Prepare( PREP_WIDOWS, (void*)&nNeed );
    return sal_True;
}

/*************************************************************************
 *                  WidowsAndOrphans::WouldFit()
 *************************************************************************/

sal_Bool WidowsAndOrphans::WouldFit( SwTxtMargin &rLine, SwTwips &rMaxHeight, sal_Bool bTst )
{
    // Here it does not matter, if pFrm is swapped or not.
    // IsInside() takes care for itself

    // Wir erwarten, dass rLine auf der letzten Zeile steht!!
    ASSERT( !rLine.GetNext(), "WouldFit: aLine::Bottom missed!" );
    MSHORT nLineCnt = rLine.GetLineNr();

    // Erstmal die Orphansregel und den Initialenwunsch erfuellen ...
#ifndef USED
    const MSHORT nMinLines = Max( GetOrphansLines(), rLine.GetDropLines() );
#else
    const MSHORT nMinLines = rLine.GetDropLines();
#endif
    if ( nLineCnt < nMinLines )
        return sal_False;

    rLine.Top();
    SwTwips nLineSum = rLine.GetLineHeight();

    while( nMinLines > rLine.GetLineNr() )
    {
        DBG_LOOP;
        if( !rLine.NextLine() )
            return sal_False;
        nLineSum += rLine.GetLineHeight();
    }

    // Wenn wir jetzt schon nicht mehr passen ...
    if( !IsInside( rLine ) )
        return sal_False;

    // Jetzt noch die Widows-Regel ueberpruefen
    if( !nWidLines && !pFrm->IsFollow() )
    {
        // I.A. brauchen Widows nur ueberprueft werden, wenn wir ein Follow
        // sind. Bei WouldFit muss aber auch fuer den Master die Regel ueber-
        // prueft werden, weil wir ja gerade erst die Trennstelle ermitteln.
        // Im Ctor von WidowsAndOrphans wurde nWidLines aber nur fuer Follows
        // aus dem AttrSet ermittelt, deshalb holen wir es hier nach:
        const SwAttrSet& rSet = pFrm->GetTxtNode()->GetSwAttrSet();
        nWidLines = rSet.GetWidows().GetValue();
    }

    // Sind nach Orphans/Initialen noch genug Zeilen fuer die Widows uebrig?
    if( bTst || nLineCnt - nMinLines >= GetWidowsLines() )
    {
        if( rMaxHeight >= nLineSum )
        {
            rMaxHeight -= nLineSum;
            return sal_True;
        }
    }
    return sal_False;
}

