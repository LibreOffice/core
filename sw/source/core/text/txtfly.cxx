/*************************************************************************
 *
 *  $RCSfile: txtfly.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-17 10:21:27 $
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

#include "frmsh.hxx"
#include "doc.hxx"
#include "viewsh.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "viewimp.hxx"      // SwViewImp
#include "pam.hxx"          // SwPosition
#include "swregion.hxx"     // SwRegionRects
#include "dcontact.hxx"     // SwContact
#include "dflyobj.hxx"      // SdrObject
#include "flyfrm.hxx"     // SwFlyFrm
#include "frmtool.hxx"    // ::DrawGraphic
#include "porfld.hxx"       // SwGrfNumPortion

#ifdef VERT_DISTANCE
#include <math.h>
#endif

#ifndef _XPOLY_HXX //autogen
#include <svx/xpoly.hxx>
#endif

#ifndef _E3D_OBJ3D_HXX //autogen
#include <svx/obj3d.hxx>
#endif

#ifndef _TXTRANGE_HXX //autogen
#include <svx/txtrange.hxx>
#endif

#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif
#include "txtfrm.hxx"     // SwTxtFrm
#include "itrform2.hxx"   // SwTxtFormatter
#include "porfly.hxx"     // NewFlyCntPortion
#include "porfld.hxx"     // SwGrfNumPortion

#include "txtfly.hxx"     // SwTxtFly
#include "txtpaint.hxx"   // SwSaveClip

#include "txtatr.hxx"     // SwTxtFlyCnt
#include "txtcfg.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "drawfont.hxx" // SwDrawTextInfo
#include "fmtcnct.hxx"  // SwFmtChain

#ifndef PRODUCT
#include "viewopt.hxx"  // SwViewOptions, nur zum Testen (Test2)
#endif

/*****************************************************************************
 * Beschreibung:
 * Die Klasse SwTxtFly soll die Universalschnittstelle zwischen der
 * Formatierung/Textausgabe und den u.U. ueberlappenden freifliegenden
 * Frames sein.
 * Waehrend der Formatierung erkundigt sich der Formatierer beim SwTxtFly,
 * ob ein bestimmter Bereich durch die Attribute eines ueberlappenden
 * Frames vorliegt. Solche Bereiche werden in Form von Dummy-Portions
 * abgebildet.
 * Die gesamte Textausgabe und Retusche wird ebenfalls an ein SwTxtFly
 * weitergeleitet. Dieser entscheidet, ob Textteile geclippt werden muessen
 * und zerteilt z.B. die Bereiche bei einem DrawRect.
 * Zu beachten ist, dass alle freifliegenden Frames in einem nach TopLeft
 * sortiertem PtrArray an der Seite zu finden sind. Intern wird immer nur
 * in dokumentglobalen Werten gerechnet. Die IN- und OUT-Parameter sind
 * jedoch in den meisten Faellen an die Beduerfnisse des LineIters
 * zugeschnitten, d.h. sie werden in frame- oder windowlokalen Koordinaten
 * konvertiert.
 * Wenn mehrere Frames mit Umlaufattributen in einer Zeile liegen,
 * ergeben sich unterschiedliche Auswirkungen fuer den Textfluss:
 *
 *      L/R    P     L     R     K
 *       P   -P-P- -P-L  -P R- -P K
 *       L   -L P- -L L  -L R- -L K
 *       R    R-P-  R-L   R R-  R K
 *       K    K P-  K L   K R-  K K
 *
 * (P=parallel, L=links, R=rechts, K=kein Umlauf)
 *
 * Das Verhalten so beschreiben:
 * Jeder Rahmen kann Text verdraengen, wobei der Einfluss allerdings nur
 * bis zum naechsten Rahmen reicht.
 *****************************************************************************/

/*****************************************************************************
 *
 * lcl_TheAnchor liefert den SwFrm, an dem das Objekt verankert ist.
 *
 *****************************************************************************/

const SwFrm* lcl_TheAnchor( const SdrObject* pObj )
{
    SwFrm* pRet = pObj->IsWriterFlyFrame() ?
        ( (SwVirtFlyDrawObj*)pObj )->GetFlyFrm()->GetAnchor()
        : ( (SwDrawContact*)GetUserCall(pObj) )->GetAnchor();
    return pRet;
}

/*****************************************************************************
 * lcl_MaxAscDescent liefert die max. Ascents und Descents in der Zeile ohne
 * FlyPortions (abs. und seitengeb. Objekte), einmal mit und einmal ohne
 * Beruecksichtigung der zeichengeb. Objekte.
 * Diese Werte sind fuer das SetBase der zeichengebundenen Objekte wichtig,
 * wenn diese an den Zeichen oder an der Zeile ausgerichtet werden sollen.
 *****************************************************************************/

void lcl_MaxAscDescent( SwLinePortion* pPos, long &rAscent, long &rDescent,
     long &rFlyAscent, long &rFlyDescent, SwLinePortion* pNot = NULL )
{
    rAscent = 0;
    rDescent = 0;
    rFlyAscent = 0;
    rFlyDescent = 0;

    if( !pPos->GetLen() && ( pPos->IsParaPortion() || pPos->IsLayPortion() ) )
        pPos = pPos->GetPortion();

    while ( pPos )
    {
        if( !pPos->IsBreakPortion() && !pPos->IsFlyPortion() )
        {
            sal_Bool bFlyCmp = pPos->IsFlyCntPortion() ?
                           ((SwFlyCntPortion*)pPos)->IsMax() :  pPos != pNot;
            if( bFlyCmp )
            {
                rFlyAscent = Max( rFlyAscent, (long)pPos->GetAscent() );
                rFlyDescent = Max( rFlyDescent,
                        (long)( pPos->Height() - pPos->GetAscent() ) );
            }
            if( !pPos->IsFlyCntPortion() && !pPos->IsGrfNumPortion() )
            {
                rAscent = Max( rAscent, (long)pPos->GetAscent() );
                rDescent = Max( rDescent,
                    (long)( pPos->Height() - pPos->GetAscent() ) );
            }
        }
        pPos = pPos->GetPortion();
    }
}

void SwTxtFormatter::CalcUnclipped( SwTwips& rTop, SwTwips& rBottom )
{
    long nFlyAsc, nFlyDesc;
    lcl_MaxAscDescent( pCurr, rTop, rBottom, nFlyAsc, nFlyDesc );
    rTop = Y() + GetCurr()->GetAscent();
    rBottom = rTop + nFlyDesc;
    rTop -= nFlyAsc;
}

/*************************************************************************
 * SwTxtFormatter::UpdatePos() aktualisiert die Referenzpunkte der zeichengeb.
 * Objekte, z. B. nach Adjustierung ( rechtsbuendig, Blocksatz etc. )
 * ( hauptsaechlich Korrrektur der X-Position )
 *************************************************************************/

void SwTxtFormatter::UpdatePos( SwLineLayout *pCurr, sal_Bool bAllWays ) const
{
    if( GetInfo().IsTest() )
        return;
    SwLinePortion *pFirst = pCurr->GetFirstPortion();
    SwLinePortion *pPos = pFirst;
    SwTxtPaintInfo aTmpInf( GetInfo() );
    aTmpInf.SetSpaceAdd( pCurr->GetpSpaceAdd() );
    aTmpInf.ResetSpaceIdx();
    // Die Groesse des Frames
    aTmpInf.SetIdx( GetStart() );
    aTmpInf.SetPos( GetTopLeft() );

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
    lcl_MaxAscDescent( pPos, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );
    KSHORT nAscent, nTmpHeight;
    CalcAscentAndHeight( nAscent, nTmpHeight );
    long nTmpY = Y() + nAscent;

    while( pPos )
    {
        // bislang ist mir nur ein Fall bekannt, wo die Positionsaenderung
        // (verursacht durch das Adjustment) fuer eine Portion wichtig
        // sein koennte: Bei FlyCntPortions muss ein SetRefPoint erfolgen.
        if( ( pPos->IsFlyCntPortion() || pPos->IsGrfNumPortion() )
            && ( bAllWays || !IsQuick() ) )
        {
            lcl_MaxAscDescent( pFirst, nTmpAscent, nTmpDescent,
                               nFlyAsc, nFlyDesc, pPos );
            if( pPos->IsGrfNumPortion() )
            {
                if( !nFlyAsc && !nFlyDesc )
                {
                    nTmpAscent = nAscent;
                    nFlyAsc = nAscent;
                    nTmpDescent = nTmpHeight - nAscent;
                    nFlyDesc = nTmpDescent;
                }
                ((SwGrfNumPortion*)pPos)->SetBase( nTmpAscent, nTmpDescent,
                                                   nFlyAsc, nFlyDesc );
            }
            else
            {
                const Point aBase( aTmpInf.X(), nTmpY );
                ((SwFlyCntPortion*)pPos)->SetBase( aBase, nTmpAscent,
                    nTmpDescent, nFlyAsc, nFlyDesc, SETBASE_ULSPACE );
            }
        }
        pPos->Move( aTmpInf );
        pPos = pPos->GetPortion();
    }
}

/*************************************************************************
 * SwTxtFormatter::AlignFlyInCntBase()
 * richtet die zeichengeb. Objekte in Y-Richtung ggf. neu aus.
 *************************************************************************/

void SwTxtFormatter::AlignFlyInCntBase( long nBaseLine ) const
{
    if( GetInfo().IsTest() )
        return;
    SwLinePortion *pFirst = pCurr->GetFirstPortion();
    SwLinePortion *pPos = pFirst;

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;

    while( pPos )
    {
        if( pPos->IsFlyCntPortion() || pPos->IsGrfNumPortion() )
        {
            lcl_MaxAscDescent( pFirst, nTmpAscent, nTmpDescent,
                               nFlyAsc, nFlyDesc, pPos );
            if( pPos->IsGrfNumPortion() )
                ((SwGrfNumPortion*)pPos)->SetBase( nTmpAscent, nTmpDescent,
                                                   nFlyAsc, nFlyDesc );
            else
            {
                const Point aBase( ( (SwFlyCntPortion*)pPos)->GetRefPoint().X(),
                                   nBaseLine );
                ((SwFlyCntPortion*)pPos)->SetBase( aBase, nTmpAscent, nTmpDescent,
                    nFlyAsc, nFlyDesc, SETBASE_NOFLAG );
            }
        }
        pPos = pPos->GetPortion();
    }
}

/*************************************************************************
 * SwTxtFly::ChkFlyUnderflow()
 * ueberprueft, ob Textportions der aktuellen Zeile mit Objekten
 * (absatz/seitengeb.) ueberlappen.
 * Dazu wird SwTxtFly.GetFrm(..) benutzt.
 *************************************************************************/

sal_Bool SwTxtFormatter::ChkFlyUnderflow( SwTxtFormatInfo &rInf ) const
{
    ASSERT( rInf.GetTxtFly()->IsOn(), "SwTxtFormatter::ChkFlyUnderflow: why?" );
    if( GetCurr() )
    {
        // Erst pruefen wir, ob ueberhaupt ein Fly mit der Zeile ueberlappt.
        const long nHeight = GetLineHeight();
        SwRect aLine( GetLeftMargin(), Y(), rInf.RealWidth(), nHeight );
        SwRect aInter( rInf.GetTxtFly()->GetFrm( aLine ) );
        if( !aInter.HasArea() )
            return sal_False;

        // Nun ueberpruefen wir jede Portion, die sich haette senken koennen,
        // ob sie mit dem Fly ueberlappt.
        const SwLinePortion *pPos = GetCurr()->GetFirstPortion();
        while( pPos )
        {
            if( !pPos->IsFlyPortion() )
            {
                aLine.Width( pPos->Width() );
                aInter = rInf.GetTxtFly()->GetFrm( aLine );
                if( aInter.IsOver( aLine ) )
                {
                    aInter._Intersection( aLine );
                    if( aInter.HasArea() )
                    {
                        rInf.SetLineHeight( KSHORT(nHeight) );
                        return sal_True;
                    }
                }
            }
            aLine.Left( aLine.Left() + pPos->Width() );
            pPos = pPos->GetPortion();
        }
    }
    return sal_False;
}

/*************************************************************************
 * SwTxtFormatter::CalcFlyWidth()
 * ermittelt das naechste Objekt, das in die restliche Zeile ragt und
 * konstruiert die zugehoerige FlyPortion.
 * Dazu wird SwTxtFly.GetFrm(..) benutzt.
 *************************************************************************/

// Durch Flys kann sich der rechte Rand verkuerzen.

void SwTxtFormatter::CalcFlyWidth( SwTxtFormatInfo &rInf ) const
{
    if( GetMulti() )
        return;
    SwTxtFly *pTxtFly = rInf.GetTxtFly();
    if( !pTxtFly->IsOn() || rInf.IsIgnoreFly() )
        return;

    register const SwLinePortion *pLast = rInf.GetLast();

    long nHeight;
    long nAscent;
    sal_Bool bOldFly = 0 != rInf.GetFly();
    // aLine wird dokumentglobal
    if( bOldFly )
    {
        // Wenn CalcFlyWidth aus Underflow() gerufen wird, kann eine FlyPortion
        // gesetzt sein, die Breite muss neu berechnet werden, die Hoehe ist
        // richtig und muss als Zeilenhoehe genutzt werden
        nHeight = rInf.GetFly()->Height();
        nAscent = rInf.GetFly()->GetAscent();
        delete rInf.GetFly(); // Die alte FlyPortion hat ihre Schuldigkeit getan
        rInf.SetFly( 0 );
    }
    else if( rInf.GetLineHeight() )
    {
        nHeight = rInf.GetLineHeight();
        nAscent = pCurr->GetAscent();
    }
    else
    {
        nHeight = pLast->Height();
        nAscent = pLast->GetAscent();
    }
    const long nLeftMar = GetLeftMargin();
    const long nLeftMin = (rInf.X() || GetDropLeft()) ? nLeftMar : GetLeftMin();
    SwRect aLine( rInf.X() + nLeftMin, Y(),  rInf.RealWidth() - rInf.X()
                  + nLeftMar - nLeftMin , nHeight );

    SwRect aInter( pTxtFly->GetFrm( aLine ) );

    if( aInter.IsOver( aLine ) )
    {
        aLine.Left( rInf.X() + nLeftMar );
        sal_Bool bForced = sal_False;
        if( aInter.Left() <= nLeftMin )
        {
            SwTwips nFrmLeft = GetTxtFrm()->Frm().Left();
            if( GetTxtFrm()->Prt().Left() < 0 )
                nFrmLeft += GetTxtFrm()->Prt().Left();
            if( aInter.Left() < nFrmLeft )
                aInter.Left( nFrmLeft );
            aInter.Width( aInter.Width() + nLeftMar - nFrmLeft );
            // Bei negativem Erstzeileneinzug setzen wir das Flag,
            // um anzuzeigen, dass der Einzug/Rand verschoben wurde
            // Dies muss beim DefaultTab an der Nullposition beruecksichtigt
            // werden.
            if( IsFirstTxtLine() && HasNegFirst() )
                bForced = sal_True;
        }
        aInter.Intersection( aLine );
        if( !aInter.HasArea() )
            return;

        const sal_Bool bFullLine =  aLine.Left()  == aInter.Left() &&
                                aLine.Right() == aInter.Right();

        // Obwohl kein Text mehr da ist, muss eine weitere Zeile
        // formatiert werden, weil auch leere Zeilen einem Fly
        // ohne Umlauf ausweichen muessen.
        if( bFullLine && rInf.GetIdx() == rInf.GetTxt().Len() )
        {
            rInf.SetNewLine( sal_True );
            // 8221: Dummies erkennt man an Ascent == Height
            pCurr->SetDummy(sal_True);
        }

        // aInter wird framelokal
        aInter.Pos().X() -= nLeftMar;
        SwFlyPortion *pFly = new SwFlyPortion( aInter );
        if( bForced )
        {
            pCurr->SetForcedLeftMargin( sal_True );
            rInf.ForcedLeftMargin( aInter.Width() );
        }

        if( bFullLine )
        {
            // 8110: wir muessen um Einheiten von Zeilenhoehen anwachsen,
            // um nebeneinanderliegende Flys mit unterschiedlichen
            // Umlaufattributen angemessen zu umfliessen.
            // Die letzte ausweichende Zeile, sollte in der Hoehe angepasst
            // sein, damit nicht der Eindruck von "Rahmenabstaenden" aufkommt.
            // 8221: Wichtig ist, dass Ascent == Height ist, weil die FlyPortionWerte
            // im CalcLine in pCurr uebertragen werden und IsDummy() darauf
            // angewiesen ist.
            // Es gibt meines Wissens nur zwei Stellen, in denen DummyLines
            // entstehen koennen: hier und in MakeFlyDummies.
            // Ausgewertet wird IsDummy() in IsFirstTxtLine() und
            // beim Zeilenwandern und im Zusammenhang mit DropCaps.
            pFly->Height( KSHORT(aInter.Height()) );

            // In nNextTop steckt jetzt die Unterkante des Rahmens, dem wir
            // ausweichen oder die Oberkante des naechsten Rahmens, den wir
            // beachten muessen. Wir koennen also jetzt getrost bis zu diesem
            // Wert anwachsen, so sparen wir einige Leerzeilen.
            if( pTxtFly->GetNextTop() > aInter.Bottom() )
            {
                SwTwips nH = pTxtFly->GetNextTop() - aInter.Top();
                if( nH < KSHRT_MAX )
                    pFly->Height( KSHORT( nH ) );
            }
            if( nAscent < pFly->Height() )
                pFly->SetAscent( KSHORT(nAscent) );
            else
                pFly->SetAscent( pFly->Height() );
        }
        else
        {
            if( rInf.GetIdx() == rInf.GetTxt().Len() )
            {
                // Nicht nHeight nehmen, sonst haben wir einen Riesendescent
                pFly->Height( pLast->Height() );
                pFly->SetAscent( pLast->GetAscent() );
            }
            else
            {
                pFly->Height( KSHORT(aInter.Height()) );
                if( nAscent < pFly->Height() )
                    pFly->SetAscent( KSHORT(nAscent) );
                else
                    pFly->SetAscent( pFly->Height() );
            }
        }

        rInf.SetFly( pFly );
        if( !bOldFly || pFly->Fix() < rInf.Width() )
            rInf.Width( pFly->Fix() );
    }
}

/*****************************************************************************
 * SwTxtFormatter::NewFlyCntPortion
 * legt eine neue Portion fuer ein zeichengebundenes Objekt an.
 *****************************************************************************/

SwFlyCntPortion *SwTxtFormatter::NewFlyCntPortion( SwTxtFormatInfo &rInf,
                                                   SwTxtAttr *pHint ) const
{
    SwFlyCntPortion *pRet = 0;
    const SwFrm *pFrame = (SwFrm*)pFrm;

    SwFlyInCntFrm *pFly;
    SwFrmFmt* pFrmFmt = ((SwTxtFlyCnt*)pHint)->GetFlyCnt().GetFrmFmt();
    if( RES_FLYFRMFMT == pFrmFmt->Which() )
        pFly = ((SwTxtFlyCnt*)pHint)->GetFlyFrm(pFrame);
    else
        pFly = NULL;
    // aBase bezeichnet die dokumentglobale Position,
    // ab der die neue Extraportion plaziert wird.
    // aBase.X() = Offset in der Zeile,
    //             hinter der aktuellen Portion
    // aBase.Y() = LineIter.Y() + Ascent der aktuellen Portion

    SwLinePortion *pPos = pCurr->GetFirstPortion();

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
    lcl_MaxAscDescent( pPos, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );

    // Wenn der Ascent des Rahmens groesser als der Ascent der akt. Portion
    // ist, wird dieser bei der Base-Berechnung verwendet, sonst wuerde
    // der Rahmen zunaechst zu weit nach oben gesetzt, um dann doch wieder
    // nach unten zu rutschen und dabei ein Repaint in einem Bereich ausloesen,
    // indem er niemals wirklich war.
    KSHORT nAscent;
    if ( IsQuick() || !pFly || !pFly->GetValidPosFlag() ||
            !pFly->GetRefPoint().Y() ||
            ( nAscent = Abs( int( pFly->GetRelPos().Y() ) ) )
            < rInf.GetLast()->GetAscent() )
        nAscent = rInf.GetLast()->GetAscent();
    else if( nAscent > nFlyAsc )
        nFlyAsc = nAscent;

    Point aBase( GetLeftMargin() + rInf.X(), Y() + nAscent );

    if( pFly )
    {
        pRet = new SwFlyCntPortion( pFly, aBase, nTmpAscent, nTmpDescent,
                                    nFlyAsc, nFlyDesc, IsQuick() );
        // Wir muessen sicherstellen, dass unser Font wieder im OutputDevice
        // steht. Es koennte sein, dass der FlyInCnt frisch eingefuegt wurde,
        // dann hat GetFlyFrm dazu gefuehrt, dass er neu angelegt wird.
        // Dessen Frames werden sofort formatiert, die verstellen den Font
        // und schon haben wir den Salat (3322).
        rInf.SelectFont();
        if( pRet->GetAscent() > nAscent )
        {
            aBase.Y() = Y() + pRet->GetAscent();
            sal_uInt8 nMode = SETBASE_ULSPACE;
            if( IsQuick() )
                nMode |= SETBASE_QUICK;
            if( !rInf.IsTest() )
                pRet->SetBase( aBase, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc,
                               nMode );
        }
    }
    else
    {
        pRet = new SwFlyCntPortion( (SwDrawContact*)pFrmFmt->FindContactObj(),
           aBase, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, IsQuick() );
    }
    return pRet;
}



/*************************************************************************
 *                      SwTxtFly::SwTxtFly()
 *************************************************************************/

SwTxtFly::SwTxtFly( const SwTxtFly& rTxtFly )
{
    pPage = rTxtFly.pPage;
    pCurrFly = rTxtFly.pCurrFly;
    pCurrFrm = rTxtFly.pCurrFrm;
    pMaster = rTxtFly.pMaster;
    if( rTxtFly.pFlyList )
    {
        pFlyList = new SwFlyList( rTxtFly.pFlyList->Count(), 10 );
        pFlyList->Insert( rTxtFly.pFlyList, 0 );
    }
    else
        pFlyList = NULL;
    bOn = rTxtFly.bOn;
    bLeftSide = rTxtFly.bLeftSide;
    bTopRule = rTxtFly.bTopRule;
}

void SwTxtFly::CtorInit( const SwCntntFrm *pFrm )
{
    pPage = pFrm->FindPageFrm();
    const SwFlyFrm* pTmp = pFrm->FindFlyFrm();
    pCurrFly = pTmp ? pTmp->GetVirtDrawObj() : NULL;
    pCurrFrm = pFrm;
    pMaster = pCurrFrm->IsFollow() ? NULL : pCurrFrm;
    pFlyList = NULL;
    // Wenn wir nicht von einem Frame ueberlappt werden, oder wenn
    // es gar keine FlyCollection gibt, dann schaltet wir uns fuer immer ab.
    // Aber es koennte sein, dass waehrend der Formatierung eine Zeile
    // hinzukommt, die in einen Frame hineinragt. Deswegen keine Optimierung
    // per bOn = pSortedFlys && IsAnyFrm();
    bOn = pPage->GetSortedObjs() != 0;
    bTopRule = sal_True;
    bLeftSide = sal_False;
    nMinBottom = 0;
    nIndex = ULONG_MAX;
}

/*************************************************************************
 *                      SwTxtFly::_GetFrm()
 *
 * IN:  dokumentglobal  (rRect)
 * OUT: framelokal      (return-Wert)
 * Diese Methode wird waehrend der Formatierung vom LineIter gerufen.
 * 1. um die naechste FlyPortion vorzubereiten
 * 2. um nach Aenderung der Zeilenhoehe neue Ueberlappungen festzustellen
 *************************************************************************/

SwRect SwTxtFly::_GetFrm( const SwRect &rRect, sal_Bool bTop ) const
{
    SwRect aRet;
    if( ForEach( rRect, &aRet, sal_True ) )
    {
        if( bTop )
            aRet.Top( rRect.Top() );

        // 8110: Bottom nicht immer anpassen.
        if( aRet.Bottom() > rRect.Bottom() || 0 > aRet.Height() )
            aRet.Bottom( rRect.Bottom() );
    }
    return aRet;
}

/*************************************************************************
 *                      SwTxtFly::IsAnyFrm()
 *
 * IN: dokumentglobal
 * fuer die Printarea des aktuellen Frame
 *
 * dient zum Abschalten des SwTxtFly, wenn keine Objekte ueberlappen (Relax)
 *
 *************************************************************************/

sal_Bool SwTxtFly::IsAnyFrm() const
{
    ASSERT( bOn, "IsAnyFrm: Why?" );
    SwRect aRect( pCurrFrm->Frm().Pos() + pCurrFrm->Prt().Pos(),
        pCurrFrm->Prt().SSize() );
    return ForEach( aRect, NULL, sal_False );
}

/*************************************************************************
 *                      SwTxtFly::IsAnyObj()
 *
 * IN: dokumentglobal
 * OUT: sal_True Wenn ein Rahmen oder DrawObj beruecksichtigt werden muss
 * Nur wenn IsAnyObj sal_False liefert, koennen Optimierungen benutzt werden
 * wie Paint/FormatEmpty fuer leere Absaetze
 * und auch das virtuelle Outputdevice.
 *************************************************************************/

sal_Bool SwTxtFly::IsAnyObj( const SwRect &rRect ) const
{
    ASSERT ( bOn, "SwTxtFly::IsAnyObj: Who's knocking?" );

    SwRect aRect( rRect );
    if ( aRect.IsEmpty() )
        aRect = SwRect( pCurrFrm->Frm().Pos() + pCurrFrm->Prt().Pos(),
                        pCurrFrm->Prt().SSize() );

    const SwSortDrawObjs *pSorted = pPage->GetSortedObjs();
    if( pSorted ) // Eigentlich ist durch bOn sichergestellt, dass es an der
    // Seite Objekte gibt, aber wer weiss, wer inzwischen etwas geloescht hat.
    {
        for ( MSHORT i = 0; i < pSorted->Count(); ++i )
        {
            const SdrObject *pObj = (*pSorted)[i];

            const SwRect aBound( GetBoundRect( pObj ) );

            // Optimierung
            if( pObj->GetBoundRect().Left() > aRect.Right() )
                continue;

            if( pCurrFly != pObj && aBound.IsOver( aRect ) )
                return sal_True;
        }
    }
    return sal_False;
}

const SwCntntFrm* SwTxtFly::_GetMaster()
{
    pMaster = pCurrFrm;
    while( pMaster->IsFollow() )
        pMaster = (SwCntntFrm*)pMaster->FindMaster();
    return pMaster;
}

/*************************************************************************
 *                      SwTxtFly::DrawTextOpaque()
 *
 * IN: dokumentglobal
 * DrawTextOpaque() wird von DrawText() gerufen.
 * Die Clipregions werden so gesetzt, dass nur die Teile ausgegeben werden,
 * die nicht in den Bereichen von FlyFrms liegen, die undurchsichtig und
 * ueber dem aktuellen Frame liegen.
 * Die On-Optimierung uebernimmt DrawText()!
 *************************************************************************/

#define UINT32_MAX 0xFFFFFFFF

sal_Bool SwTxtFly::DrawTextOpaque( SwDrawTextInfo &rInf )
{
    SwSaveClip aClipSave( rInf.GetpOut() );
    SwRect aRect( rInf.GetPos(), rInf.GetSize() );
    if( rInf.GetSpace() )
    {
        xub_StrLen nTmpLen = STRING_LEN == rInf.GetLen() ? rInf.GetText().Len() :
                                                      rInf.GetLen();
        if( rInf.GetSpace() > 0 )
        {
            xub_StrLen nSpaceCnt = 0;
            const xub_StrLen nEndPos = rInf.GetIdx() + nTmpLen;
            for( xub_StrLen nPos = rInf.GetIdx(); nPos < nEndPos; ++nPos )
            {
                if( CH_BLANK == rInf.GetText().GetChar( nPos ) )
                    ++nSpaceCnt;
            }
            if( nSpaceCnt )
                aRect.Width( aRect.Width() + nSpaceCnt * rInf.GetSpace() );
        }
        else
            aRect.Width( aRect.Width() - nTmpLen * rInf.GetSpace() );
    }

    if( aClipSave.IsOn() && rInf.GetOut().IsClipRegion() )
    {
        SwRect aClipRect( rInf.GetOut().GetClipRegion().GetBoundRect() );
        aRect.Intersection( aClipRect );
    }

    SwRegionRects aRegion( aRect );

    sal_Bool bOpaque = sal_False;
    const UINT32 nCurrOrd = pCurrFly ? pCurrFly->GetOrdNum() : UINT32_MAX;
    ASSERT( !bTopRule, "DrawTextOpaque: Wrong TopRule" );

    MSHORT nCount;
    if( bOn && ( 0 != ( nCount = GetFlyList()->Count() ) ) )
    {
        MSHORT nHellId = pPage->GetShell()->GetDoc()->GetHellId();
        for( MSHORT i = 0; i < nCount; ++i )
        {
            const SdrObject *pTmp = (*pFlyList)[ i ];
            if( pTmp->IsWriterFlyFrame() && pCurrFly != pTmp )
            {
                SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pTmp)->GetFlyFrm();
                if( aRegion.GetOrigin().IsOver( pFly->Frm() ) )
                {
                    const SwFrmFmt *pFmt = pFly->GetFmt();
                    const SwFmtSurround &rSur = pFmt->GetSurround();
                    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                        //Nur undurchsichtige und weiter oben liegende.
                    if( ( SURROUND_THROUGHT == rSur.GetSurround() &&
                          ( !rSur.IsAnchorOnly() ||
                            GetMaster() == lcl_TheAnchor( pTmp ) ||
                            ( FLY_AT_CNTNT != rAnchor.GetAnchorId() &&
                              FLY_AUTO_CNTNT != rAnchor.GetAnchorId() ) ) &&
                          pTmp->GetLayer() != nHellId &&
                          nCurrOrd < pTmp->GetOrdNum() ) )
                    {
                        //Ausser der Inhalt ist Transparent
                        const SwNoTxtFrm *pNoTxt =
                                pFly->Lower() && pFly->Lower()->IsNoTxtFrm()
                                                   ? (SwNoTxtFrm*)pFly->Lower()
                                                   : 0;
                        if ( !pNoTxt ||
                             (!pNoTxt->IsTransparent() && !rSur.IsContour()) )
                        {
                            bOpaque = sal_True;
                            aRegion -= pFly->Frm();
                        }
                    }
                }
            }
        }
    }

    Point aPos( rInf.GetPos().X(), rInf.GetPos().Y() + rInf.GetAscent() );
    const Point &rOld = rInf.GetPos();
    rInf.SetPos( aPos );

    if( !bOpaque )
    {
        if( rInf.GetKern() )
            rInf.GetFont()->_DrawStretchText( rInf );
        else
            rInf.GetFont()->_DrawText( rInf );
        rInf.SetPos( rOld );
        return sal_False;
    }
    else if( aRegion.Count() )
    {
        // Was fuer ein Aufwand ...
        SwSaveClip aClipVout( rInf.GetpOut() );
        for( MSHORT i = 0; i < aRegion.Count(); ++i )
        {
            SwRect &rRect = aRegion[i];
            if( rRect != aRegion.GetOrigin() )
                aClipVout.ChgClip( rRect );
            if( rInf.GetKern() )
                rInf.GetFont()->_DrawStretchText( rInf );
            else
                rInf.GetFont()->_DrawText( rInf );
        }
    }
    rInf.SetPos( rOld );
    return sal_True;
}

/*************************************************************************
 *                      SwTxtFly::DrawFlyRect()
 *
 * IN: windowlokal
 * Zwei Feinheiten gilt es zu beachten:
 * 1) DrawRect() oberhalb des ClipRects sind erlaubt !
 * 2) FlyToRect() liefert groessere Werte als die Framedaten !
 *************************************************************************/

void SwTxtFly::DrawFlyRect( OutputDevice *pOut, const SwRect &rRect,
        const SwTxtPaintInfo &rInf, sal_Bool bNoGraphic )
{
    SwRegionRects aRegion( rRect );
    ASSERT( !bTopRule, "DrawFlyRect: Wrong TopRule" );
    MSHORT nCount;
    if( bOn && ( 0 != ( nCount = GetFlyList()->Count() ) ) )
    {
        MSHORT nHellId = pPage->GetShell()->GetDoc()->GetHellId();
        Size aPixelSz = Size( 1, 1 );
        aPixelSz = pOut->PixelToLogic( aPixelSz );
        for( MSHORT i = 0; i < nCount; ++i )
        {
            const SdrObject *pTmp = (*pFlyList)[ i ];
            if( pCurrFly != pTmp && pTmp->IsWriterFlyFrame() )
            {
                const SwFrmFmt *pFmt =
                    ((SwContact*)GetUserCall(pTmp))->GetFmt();
                const SwFmtSurround &rSur = pFmt->GetSurround();

                if( ( SURROUND_THROUGHT == rSur.GetSurround() ) ?
                    pTmp->GetLayer() != nHellId : !rSur.IsContour() )
                {
                    SwRect aFly( pTmp->GetBoundRect() );
                    aFly.Left( aFly.Left() - aPixelSz.Width() );
                    aFly.Top( aFly.Top() - aPixelSz.Height() );
                    if( aFly.Width() > 0 && aFly.Height() > 0 )
                        aRegion -= aFly;
                }
            }
        }
    }

    for( MSHORT i = 0; i < aRegion.Count(); ++i )
    {
        if ( bNoGraphic )
            pOut->DrawRect( aRegion[i].SVRect() );
        else
        {
            ASSERT( ((SvxBrushItem*)-1) != rInf.GetBrushItem(),
                    "DrawRect: Uninitialized BrushItem!" );
            ::DrawGraphic( rInf.GetBrushItem(), pOut, rInf.GetBrushRect(),
                       aRegion[i] );
        }
    }
}

/*************************************************************************
 *                      SwTxtFly::GetTop()
 *
 * GetTop() ueberprueft, ob pNew ueber pCurrFly liegt (Z-Order).
 * Es gilt, dass die unten liegenden die obenliegenden beachten nicht
 * umgekehrt !
 * Returnwert: pNew, wenn pNew ueber pCurrFly liegt, ansonsten 0.
 * wird nur von InitFlyList benutzt, um die in Frage kommenden Objekte
 * einzusammeln.
 *************************************************************************/

sal_Bool SwTxtFly::GetTop( const SdrObject *pNew, const sal_Bool bFooter )
{
    if( pNew != pCurrFly )
    {
        if( bFooter && bTopRule )
        {
            const SwFmtAnchor& rNewA =
                ((SwContact*)GetUserCall(pNew))->GetFmt()->GetAnchor();
            if( FLY_PAGE == rNewA.GetAnchorId() )
                return sal_False; // Im Footer wird seitengeb. nicht ausgewichen
        }
        sal_Bool bEvade = !pCurrFly  //Selbst nicht im Fly -> allen ausweichen.
                 //Den Lowern ist auszuweichen.
            || Is_Lower_Of(((SwVirtFlyDrawObj*)pCurrFly)->GetFlyFrm(), pNew);
        if ( !bEvade )
        {
            if ( !bTopRule )
                bEvade = sal_True; // nur an der Paint-Ordnung interessiert
            else
            {
                // innerhalb von verketteten Flys wird nur Lowern ausgewichen
                const SwFmtChain &rChain = ((SwContact*)GetUserCall(pCurrFly))->GetFmt()->GetChain();
                if ( !rChain.GetPrev() && !rChain.GetNext() )
                {
                    // Ausweichregel fuer Text:
                    const SwFmtAnchor& rNewA =
                        ((SwContact*)GetUserCall(pNew))->GetFmt()->GetAnchor();
                    const SwFmtAnchor& rCurrA =
                        ((SwContact*)GetUserCall(pCurrFly))->GetFmt()->GetAnchor();
                    if( FLY_IN_CNTNT == rCurrA.GetAnchorId() )
                        return sal_False; // Zeichengebundene weichen nur Lowern aus.
                    if( FLY_PAGE == rNewA.GetAnchorId() )
                    {   //Chg: Seitengebundenen wird nur noch von anderen
                        // seitengebundenen ausgewichen!
                        if( FLY_PAGE == rCurrA.GetAnchorId() )
                            bEvade = sal_True;
                        else
                            return sal_False;
                    }
                    else if( FLY_PAGE == rCurrA.GetAnchorId() )
                        return sal_False; // Seitengebundene weichen nur seitengeb. aus
                    else if( FLY_AT_FLY == rNewA.GetAnchorId() )
                        bEvade = sal_True; // Nicht seitengeb. weichen Rahmengeb. aus
                    else if( FLY_AT_FLY == rCurrA.GetAnchorId() )
                        return sal_False; // Rahmengebundene weichen abs.geb. nicht aus
                    else // Zwei Flies mit (auto-)absatzgebunder Verankerung ...
                    // ... entscheiden nach der Reihenfolge ihrer Anker im Dok.
                        bEvade = rNewA.GetCntntAnchor()->nNode.GetIndex() <=
                                rCurrA.GetCntntAnchor()->nNode.GetIndex();
                }
            }
            // aber: es wird niemals einem hierarchisch untergeordnetem
            // ausgewichen und ausserdem braucht nur bei Ueberlappung
            // ausgewichen werden.
            bEvade &= ( pCurrFly->GetOrdNum() < pNew->GetOrdNum() );
            if( bEvade )
            {
                SwRect aTmp( GetBoundRect( pNew ) );
                if( !aTmp.IsOver( pCurrFly->GetBoundRect() ) )
                    bEvade = sal_False;
            }
        }
        if ( bEvade )
        {
            const SwFmtAnchor& rNewA =
                ((SwContact*)GetUserCall(pNew))->GetFmt()->GetAnchor();
            ASSERT( FLY_IN_CNTNT != rNewA.GetAnchorId(), "Don't call GetTop with a FlyInCntFrm" );
            if( FLY_PAGE == rNewA.GetAnchorId() )
                return sal_True;  // Seitengebundenen wird immer ausgewichen.

            // Wenn absatzgebundene Flys in einem FlyCnt gefangen sind, so
            // endet deren Einflussbereich an den Grenzen des FlyCnt!
            // Wenn wir aber gerade den Text des FlyCnt formatieren, dann
            // muss er natuerlich dem absatzgebundenen Frm ausweichen!
            // pCurrFrm ist der Anker von pNew?
            const SwFrm* pTmp = lcl_TheAnchor( pNew );
            if( pTmp == pCurrFrm )
                return sal_True;
            if( pTmp->IsTxtFrm() && ( pTmp->IsInFly() || pTmp->IsInFtn() ) )
            {
                Point aPos;
                if( pNew->IsWriterFlyFrame() )
                    aPos = ( (SwVirtFlyDrawObj*)pNew )->GetFlyFrm()->Frm().Pos();
                else
                    aPos = pNew->GetBoundRect().TopLeft();
                pTmp = GetVirtualUpper( pTmp, aPos );
            }
            if( pCurrFrm->GetNext() != pTmp &&
                IsFrmInSameKontext( pTmp, pCurrFrm ) )
            {
                if( FLY_AT_FLY == rNewA.GetAnchorId() ) // LAYER_IMPL
                    return sal_True;  // Rahmengebundenen ausweichen.
                // Den Index des anderen erhalten wir immer ueber das Ankerattr.
                ULONG nTmpIndex = rNewA.GetCntntAnchor()->nNode.GetIndex();
                // Jetzt wird noch ueberprueft, ob der aktuelle Absatz vor dem
                // Anker des verdraengenden Objekts im Text steht, dann wird
                // nicht ausgewichen.
                // Der Index wird moeglichst ueber einen SwFmtAnchor ermittelt,
                // da sonst recht teuer.
                if( ULONG_MAX == nIndex )
                    nIndex = pCurrFrm->GetNode()->GetIndex();

                if( nIndex >= nTmpIndex )
                    return sal_True;
            }
        }
    }
    return 0;
}

/*************************************************************************
 * SwTxtFly::InitFlyList()
 *
 * fuellt die FlyList mit den Objekten, denen ggf. ausgwichen werden muss
 *
 *************************************************************************/

SwFlyList *SwTxtFly::InitFlyList()
{
    ASSERT( pCurrFrm, "InitFlyList: No Frame, no FlyList" );
    ASSERT( !pFlyList, "InitFlyList: FlyList already initialized" );

    const SwSortDrawObjs *pSorted = pPage->GetSortedObjs();
    const MSHORT nCount = pSorted ? pSorted->Count() : 0;
    bOn = sal_False;
    if( nCount )
    {
        pFlyList = new SwFlyList( 10, 10 );

        SwRect aRect( pCurrFrm->Prt() );
        aRect += pCurrFrm->Frm().Pos();
        // Wir machen uns etwas kleiner als wir sind,
        // damit Ein-Twip-Ueberlappungen ignoriert werden. (#49532)
        const long nRight = aRect.Right() - 1;
        const long nLeft = aRect.Left() + 1;
        const sal_Bool bFooter = pCurrFrm->IsInFtn();

        for( MSHORT i = 0; i < nCount; i++ )
        {
            SdrObject *pO = (*pSorted)[ i ];
            const SwRect aBound( GetBoundRect( pO ) );
            if( nRight < aBound.Left() || aRect.Top() > aBound.Bottom() ||
                nLeft > aBound.Right() )
                continue;
            if( GetTop( pO, bFooter ) )
            {
                MSHORT nPos = pFlyList->Count();
                while( nPos )
                {
                    SdrObject *pTmp = (*pFlyList)[ --nPos ];
                    const SwRect aTmpBound( GetBoundRect( pTmp ) );
                    if( aTmpBound.Left() <= aBound.Left() )
                    {
                        ++nPos;
                        break;
                    }
                }
                pFlyList->C40_INSERT( SdrObject, pO, nPos );

                SwContact *pContact = (SwContact*)GetUserCall(pO);
                const SwFmtSurround &rFlyFmt = pContact->GetFmt()->GetSurround();
                if( rFlyFmt.IsAnchorOnly() && lcl_TheAnchor( pO ) == GetMaster() )
                {
                    const SwFmtVertOrient &rTmpFmt = pContact->GetFmt()->GetVertOrient();
                    if( VERT_BOTTOM != rTmpFmt.GetVertOrient() )
                        nMinBottom = Max( nMinBottom, aBound.Bottom() );
                }

                bOn = sal_True;
            }
        }
        if( nMinBottom )
        {
            SwTwips nMax = pCurrFrm->GetUpper()->Frm().Top() +
                           pCurrFrm->GetUpper()->Prt().Bottom();
            if( nMinBottom > nMax )
                nMinBottom = nMax;
        }
    }
    else
        pFlyList = new SwFlyList( 0, 10 );
    return pFlyList;
}

SwTwips SwTxtFly::CalcMinBottom() const
{
    SwTwips nRet = 0;
    const SwDrawObjs *pDrawObj = GetMaster()->GetDrawObjs();
    const MSHORT nCount = pDrawObj ? pDrawObj->Count() : 0;
    if( nCount )
    {
        SwTwips nEndOfFrm = pCurrFrm->Frm().Bottom();
        for( MSHORT i = 0; i < nCount; i++ )
        {
            SdrObject *pO = (*pDrawObj)[ i ];
            SwContact *pContact = (SwContact*)GetUserCall(pO);
            const SwFmtSurround &rFlyFmt = pContact->GetFmt()->GetSurround();
            if( rFlyFmt.IsAnchorOnly() )
            {
                const SwFmtVertOrient &rTmpFmt = pContact->GetFmt()->GetVertOrient();
                if( VERT_BOTTOM != rTmpFmt.GetVertOrient() )
                {
                    const SwRect aBound( GetBoundRect( pO ) );
                    if( aBound.Top() < nEndOfFrm )
                        nRet = Max( nRet, aBound.Bottom() );
                }
            }
        }
        SwTwips nMax = pCurrFrm->GetUpper()->Frm().Top() +
                       pCurrFrm->GetUpper()->Prt().Bottom();
        if( nRet > nMax )
            nRet = nMax;
    }
    return nRet;
}

/*************************************************************************
 * Hier erfolgt die Berechnung der Kontur ...
 * CalcBoundRect(..) und andere
 *************************************************************************/

/*************************************************************************
 * class SwContourCache
 *************************************************************************/

SwContourCache::SwContourCache() :
    nObjCnt( 0 ), nPntCnt( 0 )
{
    memset( (SdrObject**)pSdrObj, 0, sizeof(pSdrObj) );
    memset( pTextRanger, 0, sizeof(pTextRanger) );
}

SwContourCache::~SwContourCache()
{
    for( MSHORT i = 0; i < nObjCnt; delete pTextRanger[ i++ ] )
        ;
}

void SwContourCache::ClrObject( MSHORT nPos )
{
    ASSERT( pTextRanger[ nPos ], "ClrObject: Allready cleared. Good Bye!" );
    nPntCnt -= pTextRanger[ nPos ]->GetPointCount();
    delete pTextRanger[ nPos ];
    --nObjCnt;
    memmove( (SdrObject**)pSdrObj + nPos, pSdrObj + nPos + 1,
             ( nObjCnt - nPos ) * sizeof( SdrObject* ) );
    memmove( pTextRanger + nPos, pTextRanger + nPos + 1,
             ( nObjCnt - nPos ) * sizeof( TextRanger* ) );
}

void ClrContourCache( const SdrObject *pObj )
{
    if( pContourCache && pObj )
        for( MSHORT i = 0; i < pContourCache->GetCount(); ++i )
            if( pObj == pContourCache->GetObject( i ) )
            {
                pContourCache->ClrObject( i );
                break;
            }
}

void ClrContourCache()
{
    if( pContourCache )
    {
        for( MSHORT i = 0; i < pContourCache->GetCount();
             delete pContourCache->pTextRanger[ i++ ] )
             ;
        pContourCache->nObjCnt = 0;
        pContourCache->nPntCnt = 0;
    }
}

/*************************************************************************
 * SwContourCache::CalcBoundRect
 * berechnet das Rechteck, welches vom Objekt in der angegebenen Zeile
 * ueberdeckt wird.
 * Bei _nicht_ konturumflossenen Objekten ist dies einfach die Ueber-
 * lappung von BoundRect (inkl. Abstand!) und Zeile,
 * bei Konturumfluss wird das Polypolygon des Objekts abgeklappert
 *************************************************************************/

const SwRect SwContourCache::CalcBoundRect( const SdrObject* pObj,
    const SwRect &rLine, const long nXPos, const sal_Bool bRight )
{
    SwRect aRet;
    const SwFmt *pFmt =
        ((SwContact*)GetUserCall(pObj))->GetFmt();
    if( pFmt->GetSurround().IsContour() &&
        ( !pObj->IsWriterFlyFrame() ||
          ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Lower() &&
          ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Lower()->IsNoTxtFrm() ) )
    {
        const SvxLRSpaceItem &rLRSpace = pFmt->GetLRSpace();
        const SvxULSpaceItem &rULSpace = pFmt->GetULSpace();
        aRet = pObj->GetBoundRect();
        aRet.Left( aRet.Left() - rLRSpace.GetLeft() );
        aRet.Width( aRet.Width() + rLRSpace.GetRight() );
        aRet.Top( aRet.Top() - rULSpace.GetUpper() );
        aRet.Height( aRet.Height() + rULSpace.GetLower() );
        if( aRet.IsOver( rLine ) )
        {
            if( !pContourCache )
                pContourCache = new SwContourCache;
            aRet = pContourCache->ContourRect( pFmt, pObj, rLine, nXPos, bRight );
        }
        else
            aRet.Width( 0 );
    }
    else
    {
        const SvxLRSpaceItem &rLRSpace = pFmt->GetLRSpace();
        aRet = pObj->GetBoundRect();
        aRet.Left( aRet.Left() - rLRSpace.GetLeft() );
        aRet.Width( aRet.Width() + rLRSpace.GetRight() );
    }
    const SvxULSpaceItem &rULSpace = pFmt->GetULSpace();
    aRet.Top( aRet.Top() - rULSpace.GetUpper() );
    aRet.Height( aRet.Height() + rULSpace.GetLower() );
    return aRet;
}

const SwRect SwContourCache::ContourRect( const SwFmt* pFmt,
    const SdrObject* pObj, const SwRect &rLine,
    const long nXPos, const sal_Bool bRight )
{
    SwRect aRet;
    MSHORT nPos = 0; // Suche im Cache ...
    while( nPos < GetCount() && pObj != pSdrObj[ nPos ] )
        ++nPos;
    if( GetCount() == nPos ) // nicht gefunden
    {
        if( nObjCnt == POLY_CNT )
        {
            nPntCnt -= pTextRanger[ --nObjCnt ]->GetPointCount();
            delete pTextRanger[ nObjCnt ];
        }
        XPolyPolygon aXPoly;
        XPolyPolygon *pXPoly = NULL;
        if ( pObj->IsWriterFlyFrame() )
        {
            // Vorsicht #37347: Das GetContour() fuehrt zum Laden der Grafik,
            // diese aendert dadurch ggf. ihre Groesse, ruft deshalb ein
            // ClrObject() auf.
            PolyPolygon aPoly;
            if( !((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetContour( aPoly ) )
                aPoly = PolyPolygon( ((SwVirtFlyDrawObj*)pObj)->
                                     GetFlyFrm()->Frm().SVRect() );
            aXPoly = XPolyPolygon( aPoly );
        }
        else
        {
            if( !pObj->ISA( E3dObject ) )
                pObj->TakeXorPoly( aXPoly, sal_True );
            pXPoly = new XPolyPolygon();
            pObj->TakeContour( *pXPoly );
        }
        const SvxLRSpaceItem &rLRSpace = pFmt->GetLRSpace();
        const SvxULSpaceItem &rULSpace = pFmt->GetULSpace();
        memmove( pTextRanger + 1, pTextRanger, nObjCnt * sizeof( TextRanger* ) );
        memmove( (SdrObject**)pSdrObj + 1, pSdrObj, nObjCnt++ * sizeof( SdrObject* ) );
        pSdrObj[ 0 ] = pObj; // Wg. #37347 darf das Object erst nach dem
                             // GetContour() eingetragen werden.
        pTextRanger[ 0 ] = new TextRanger( aXPoly, pXPoly, 20,
            rLRSpace.GetLeft(), rLRSpace.GetRight(),
            pFmt->GetSurround().IsOutside(), sal_False );
        pTextRanger[ 0 ]->SetUpper( rULSpace.GetUpper() );
        pTextRanger[ 0 ]->SetLower( rULSpace.GetLower() );
        delete pXPoly;
        // UPPER_LOWER_TEST
#ifndef PRODUCT
        if( pFmt->GetDoc()->GetRootFrm()->GetCurrShell() )
        {
            sal_Bool bT2 =  pFmt->GetDoc()->GetRootFrm()->GetCurrShell()->GetViewOptions()->IsTest2();
            sal_Bool bT6 = pFmt->GetDoc()->GetRootFrm()->GetCurrShell()->GetViewOptions()->IsTest6();
            if( bT2 || bT6 )
            {
                if( bT2 )
                    pTextRanger[ 0 ]->SetFlag7( sal_True );
                else
                    pTextRanger[ 0 ]->SetFlag6( sal_True );
            }
        }
#endif
        nPntCnt += pTextRanger[ 0 ]->GetPointCount();
        while( nPntCnt > POLY_MAX && nObjCnt > POLY_MIN )
        {
            nPntCnt -= pTextRanger[ --nObjCnt ]->GetPointCount();
            delete pTextRanger[ nObjCnt ];
        }
    }
    else if( nPos )
    {
        const SdrObject* pTmpObj = pSdrObj[ nPos ];
        TextRanger* pTmpRanger = pTextRanger[ nPos ];
        memmove( (SdrObject**)pSdrObj + 1, pSdrObj, nPos * sizeof( SdrObject* ) );
        memmove( pTextRanger + 1, pTextRanger, nPos * sizeof( TextRanger* ) );
        pSdrObj[ 0 ] = pTmpObj;
        pTextRanger[ 0 ] = pTmpRanger;
    }
    Range aRange( rLine.Top(), rLine.Bottom() );
    SvLongs *pTmp = pTextRanger[ 0 ]->GetTextRanges( aRange );

    MSHORT nCount;
    if( 0 != ( nCount = pTmp->Count() ) )
    {
        MSHORT nIdx = 0;
        while( nIdx < nCount && (*pTmp)[ nIdx ] < nXPos )
            ++nIdx;
        sal_Bool bOdd = nIdx % 2 ? sal_True : sal_False;
        sal_Bool bSet = sal_True;
        if( bOdd )
            --nIdx; // innerhalb eines Intervalls
        else if( !bRight && ( nIdx >= nCount || (*pTmp)[ nIdx ] != nXPos ) )
        {
            if( nIdx )
                nIdx -= 2; // ein Intervall nach links gehen
            else
                bSet = sal_False; // vor dem erstem Intervall
        }

        if( bSet && nIdx < nCount )
        {
            aRet.Top( rLine.Top() );
            aRet.Height( rLine.Height() );
            aRet.Left( (*pTmp)[ nIdx ] );
            aRet.Right( (*pTmp)[ nIdx + 1 ] );
        }
    }
    return aRet;
}

/*************************************************************************
 *                      SwContourCache::ShowContour()
 * zeichnet die PolyPolygone des Caches zu Debugzwecken.
 *************************************************************************/
#ifndef PRODUCT

void SwContourCache::ShowContour( OutputDevice* pOut, const SdrObject* pObj,
    const Color& rClosedColor, const Color& rOpenColor )
{
    MSHORT nPos = 0; // Suche im Cache ...
    while( nPos < POLY_CNT && pObj != pSdrObj[ nPos ] )
        ++nPos;
    if( POLY_CNT != nPos )
    {
        const PolyPolygon* pPol = pTextRanger[ nPos ]->GetLinePolygon();
        if( !pPol )
            pPol = &(pTextRanger[ nPos ]->GetPolyPolygon());
        for( MSHORT i = 0; i < pPol->Count(); ++i )
        {
            pOut->SetLineColor( rOpenColor );
            const Polygon& rPol = (*pPol)[ i ];
            MSHORT nCount = rPol.GetSize();
            if( nCount > 1 && rPol[ 0 ] == rPol[ nCount - 1 ] )
                pOut->SetLineColor( rClosedColor );
            pOut->DrawPolygon( rPol );
        }
#ifdef DEBUG
        static KSHORT nRadius = 0;
        if( nRadius )
        {
            KSHORT nHalf = nRadius / 2;
            Size aSz( nRadius, nRadius );
            for( MSHORT i = 0; i < pPol->Count(); ++i )
            {
                const Polygon& rPol = (*pPol)[ i ];
                MSHORT nCount = rPol.GetSize();
                for( MSHORT k = 0; k < nCount; ++k )
                {
                    Point aPt( rPol[ k ] );
                    aPt.X() -= nHalf;
                    aPt.Y() -= nHalf;
                    Rectangle aTmp( aPt, aSz );
                    pOut->DrawEllipse( aTmp );
                }
            }
        }
#endif
    }
}
#endif

/*************************************************************************
 *                      SwTxtFly::ShowContour()
 * zeichnet die PolyPolygone des Caches zu Debugzwecken.
 *************************************************************************/
#ifndef PRODUCT

void SwTxtFly::ShowContour( OutputDevice* pOut )
{
    MSHORT nFlyCount;
    if( bOn && ( 0 != ( nFlyCount = GetFlyList()->Count() ) ) )
    {
        static ULONG nWidth = 20;
        Color aRedColor( COL_LIGHTRED );
        Color aGreenColor( COL_LIGHTGREEN );
        Color aSaveColor( pOut->GetLineColor() );
        for( MSHORT j = 0; j < nFlyCount; ++j )
        {
            const SdrObject *pObj = (*pFlyList)[ j ];
            if( !((SwContact*)GetUserCall(pObj))->GetFmt()->GetSurround().IsContour() )
            {
                Rectangle aRect = pObj->GetBoundRect();
                pOut->DrawRect( aRect );
                continue;
            }
            pContourCache->ShowContour( pOut, pObj, aRedColor, aGreenColor );
        }
        pOut->SetLineColor( aSaveColor );
    }
}
#endif

/*************************************************************************
 *                      SwTxtFly::ForEach()
 *
 * sucht nach dem ersten Objekt, welches mit dem Rechteck ueberlappt
 *
 *************************************************************************/

sal_Bool SwTxtFly::ForEach( const SwRect &rRect, SwRect* pRect, sal_Bool bAvoid ) const
{
    sal_Bool bRet = sal_False;
    MSHORT nCount;
    if( bOn && ( 0 != ( nCount = GetFlyList()->Count() ) ) )
    {
        for( MSHORT i = 0; i < nCount; ++i )
        {
            const SdrObject *pObj = (*pFlyList)[ i ];
            SwRect aRect( GetBoundRect( pObj ) );
            // Optimierung
            if( aRect.Left() > rRect.Right() )
                break;
            if( pCurrFly != pObj && aRect.IsOver( rRect ) )
            {
                const SwFmt *pFmt = ((SwContact*)GetUserCall(pObj))->GetFmt();
                const SwFmtSurround &rSur = pFmt->GetSurround();
                if( bAvoid )
                {
                    // Wenn der Text drunter durchlaeuft, bleibt die
                    // Formatierung unbeeinflusst. Im LineIter::DrawText()
                    // muessen "nur" geschickt die ClippingRegions gesetzt werden ...
                    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                    if( ( SURROUND_THROUGHT == rSur.GetSurround() &&
                          ( !rSur.IsAnchorOnly() ||
                            GetMaster() == lcl_TheAnchor( pObj ) ||
                            ( FLY_AT_CNTNT != rAnchor.GetAnchorId() &&
                              FLY_AUTO_CNTNT != rAnchor.GetAnchorId() ) ) )
                        || aRect.Top() == WEIT_WECH )
                        continue;
                }
                if( pRect )
                {
                    SwRect aFly = FlyToRect( pObj, rRect );
                    if( aFly.IsEmpty() || !aFly.IsOver( rRect ) )
                        continue;
                    if( !bRet || aFly.Left() < pRect->Left() )
                        *pRect = aFly;
                    if( rSur.IsContour() )
                    {
                        bRet = sal_True;
                        continue;
                    }
                }
                bRet = sal_True;
                break;
            }
        }
    }
    return bRet;
}

/*************************************************************************
 *                      SwTxtFly::GetPos()
 *
 * liefert die Position im sorted Array zurueck
 *************************************************************************/

MSHORT SwTxtFly::GetPos( const SdrObject *pObj ) const
{
    MSHORT nCount = GetFlyList()->Count();
    MSHORT nRet = 0;
    while( nRet < nCount && pObj != (*pFlyList)[ nRet ] )
        ++nRet;
    return nRet;
}

/*************************************************************************
 *                      SwTxtFly::CalcRightMargin()
 *
 * pObj ist das Object, der uns gerade ueberlappt.
 * pCurrFrm ist der aktuelle Textframe, der ueberlappt wird.
 * Der rechte Rand ist der rechte Rand oder
 * er wird durch das naechste Object, welches in die Zeile ragt, bestimmt.
 *************************************************************************/

void SwTxtFly::CalcRightMargin( SwRect &rFly, MSHORT nFlyPos,
                                  const SwRect &rLine ) const
{
    // Normalerweise ist der rechte Rand der rechte Rand der Printarea.
    SwTwips nRight = pCurrFrm->Frm().Left() + pCurrFrm->Prt().Right() + 1;
    SwTwips nFlyRight = rFly.Right();
    SwRect aLine( rLine );
    aLine.Right( nRight );
    aLine.Left( rFly.Left() );

    // Es koennte aber sein, dass in die gleiche Zeile noch ein anderes
    // Object hineinragt, welches _ueber_ uns liegt.
    // Wunder der Technik: Flys mit Durchlauf sind fuer die darunterliegenden
    // unsichtbar, das heisst, dass sie bei der Berechnung der Raender
    // anderer Flys ebenfalls nicht auffallen.
    // 3301: pNext->Frm().IsOver( rLine ) ist noetig
    _FlyCntnt eOrder;

    sal_Bool bStop = sal_False;
    MSHORT nPos = 0;

    while( nPos < pFlyList->Count() && !bStop )
    {
        if( nPos == nFlyPos )
        {
            ++nPos;
            continue;
        }
        const SdrObject *pNext = (*pFlyList)[ nPos++ ];
        if( pNext == pCurrFly )
            continue;
        eOrder = GetOrder( pNext );
        if( SURROUND_THROUGHT == eOrder )
            continue;
        const SwRect aTmp( SwContourCache::CalcBoundRect
            ( pNext, aLine, nFlyRight, sal_True ) );
        SwTwips nTmpRight = aTmp.Right();

        // Optimierung:
        // In nNextTop wird notiert, an welcher Y-Positon mit Aenderung der
        // Rahmenverhaeltnisse gerechnet werden muss. Dies dient dazu, dass,
        // obwohl nur die Rahmen in der aktuellen Zeilenhoehe betrachtet werden,
        // bei Rahmen ohne Umlauf die Zeilenhoehe so erhoeht wird, dass mit einer
        // einzigen Zeile die Unterkante das Rahmens oder ggf. die Oberkante des
        // naechsten Rahmen erreicht wird.
        // Insbesondere bei HTML-Dokumenten kommen oft (Dummy-)Absaetze in einer
        // 2-Pt.-Schrift vor, bis diese einem groesseren Rahmen ausgewichen sind,
        // erforderte es frueher Unmengen von Leerzeilen.
        if( aLine.Top() < aTmp.Top() )
        {
            if( aTmp.Top() < nNextTop )
                SetNextTop( aTmp.Top() ); // Die Oberkante des "naechsten" Rahmens
        }
        else if( !aTmp.Width() ) // Typisch fuer Objekte mit Konturumlauf
        {   // Bei Objekten mit Konturumlauf, die vor der aktuellen Zeile beginnen
            // und hinter ihr enden, trotzdem aber nicht mit ihr ueberlappen,
            // muss die Optimierung ausgeschaltet werden, denn bereits in der
            // naechsten Zeile kann sich dies aendern.
            if( !aTmp.Height() || aTmp.Bottom() > aLine.Top() )
                SetNextTop( 0 );
        }

        if( aTmp.IsOver( aLine ) && nTmpRight > nFlyRight )
        {
            nFlyRight = nTmpRight;
            switch( eOrder )
            {
                case SURROUND_RIGHT :
                case SURROUND_PARALLEL :
                {
                    // der FlyFrm wird ueberstimmt.
                    if( nRight > nFlyRight )
                        nRight = nFlyRight;
                    bStop = sal_True;
                    break;
                }
            }
        }
    }
    rFly.Right( nRight );
}

/*************************************************************************
 *                      SwTxtFly::CalcLeftMargin()
 *
 * pFly ist der FlyFrm, der uns gerade ueberlappt.
 * pCurrFrm ist der aktuelle Textframe, der ueberlappt wird.
 * Der linke Rand ist der linke Rand der aktuellen PrintArea oder
 * er wird durch den vorigen FlyFrm, der in die Zeile ragt, bestimmt.
 *************************************************************************/

void SwTxtFly::CalcLeftMargin( SwRect &rFly, MSHORT nFlyPos,
                                  const SwRect &rLine ) const
{
    // Normalerweise ist der linke Rand der linke Rand der Printarea.
    SwTwips nLeft = pCurrFrm->Frm().Left() + pCurrFrm->Prt().Left();
    if( nLeft > rFly.Left() )
        nLeft = rFly.Left();
    SwRect aLine( rLine );
    aLine.Left( nLeft );

    // Es koennte aber sein, dass in die gleiche Zeile noch ein anderes
    // Object hineinragt, welches _ueber_ uns liegt.
    // Wunder der Technik: Flys mit Durchlauf sind fuer die darunterliegenden
    // unsichtbar, das heisst, dass sie bei der Berechnung der Raender
    // anderer Flys ebenfalls nicht auffallen.
    // 3301: pNext->Frm().IsOver( rLine ) ist noetig

    MSHORT nMyPos = nFlyPos;
    while( ++nFlyPos < pFlyList->Count() )
    {
        const SdrObject *pNext = (*pFlyList)[ nFlyPos ];
        const SwRect aTmp( GetBoundRect( pNext ) );
        if( aTmp.Left() >= rFly.Left() )
            break;
    }

    while( nFlyPos )
    {
        if( --nFlyPos == nMyPos )
            continue;
        const SdrObject *pNext = (*pFlyList)[ nFlyPos ];
        if( pNext == pCurrFly )
            continue;
        _FlyCntnt eOrder = GetOrder( pNext );
        if( SURROUND_THROUGHT == eOrder )
            continue;
        const SwRect aTmp( SwContourCache::CalcBoundRect
            ( pNext, aLine, rFly.Left(), sal_False ) );
        if( aTmp.Left() < rFly.Left() && aTmp.IsOver( aLine ) )
        {
            SwTwips nTmpRight = aTmp.Right();
            if( nLeft <= nTmpRight )
                nLeft = nTmpRight + 1;
            break;
        }
    }
    rFly.Left( nLeft );
}

/*************************************************************************
 *                      SwTxtFly::FlyToRect()
 *
 * IN:  dokumentglobal  (rRect)
 * OUT: dokumentglobal  (return-Wert)
 * Liefert zu einem SwFlyFrm das von ihm in Anspruch genommene Rechteck
 * unter Beruecksichtigung der eingestellten Attribute fuer den Abstand
 * zum Text zurueck.
 *************************************************************************/

SwRect SwTxtFly::FlyToRect( const SdrObject *pObj, const SwRect &rLine ) const
{
    SwRect aFly = SwContourCache::CalcBoundRect( pObj, rLine,
        rLine.Left(), sal_True );
    if( !aFly.Width() )
        return aFly;

    SetNextTop( aFly.Bottom() ); // Damit die Zeile ggf. bis zur Unterkante
                                 // des Rahmens waechst.
    MSHORT nFlyPos = GetPos( pObj );

    // Bei LEFT und RIGHT vergroessern wir das Rechteck.
    // Hier gibt es einige Probleme, wenn mehrere Frames zu sehen sind.
    // Zur Zeit wird nur der einfachste Fall angenommen:
    // LEFT bedeutet, dass der Text links vom Frame fliessen soll,
    // d.h. der Frame blaeht sich bis zum rechten Rand der Printarea
    // oder bis zum naechsten Frame auf.
    // Bei RIGHT ist es umgekehrt.
    // Ansonsten wird immer der eingestellte Abstand zwischen Text
    // und Frame aufaddiert.
    switch( GetOrder( pObj ) )
    {
        case SURROUND_LEFT :
        {
            CalcRightMargin( aFly, nFlyPos, rLine );
            break;
        }
        case SURROUND_RIGHT :
        {
            CalcLeftMargin( aFly, nFlyPos, rLine );
            break;
        }
        case SURROUND_NONE :
        {
            CalcRightMargin( aFly, nFlyPos, rLine );
            CalcLeftMargin( aFly, nFlyPos, rLine );
            break;
        }
    }
    return aFly;
}


/*************************************************************************
 *                      SwTxtFly::CalcSmart()
 *
 * CalcSmart() liefert die Umlaufform zurueck.
 *
 * Auf beiden Seiten ist weniger als 2 cm Platz fuer den Text
 *   => kein Umlauf ( SURROUND_NONE )
 * Auf genau einer Seite ist mehr als 2 cm Platz
 *   => Umlauf auf dieser Seite ( SURROUND_LEFT / SURROUND_RIGHT )
 * Auf beiden Seiten ist mehr als 2 cm Platz, das Objekt ist breiter als 1,5 cm
 *   => Umlauf auf der breiteren Seite ( SURROUND_LEFT / SURROUND_RIGHT )
 * Auf beiden Seiten ist mehr als 2 cm Platz, das Objekt ist schmaler als 1,5 cm
 *   => beidseitiger Umlauf ( SURROUND_PARALLEL )
 *
 *************************************************************************/

// Umfluss nur auf Seiten mit mindestens 2 cm Platz fuer den Text
#define TEXT_MIN 1134
// Beidseitiger Umfluss bis zu einer Rahmenbreite von maximal 1,5 cm
#define FRAME_MAX 850

_FlyCntnt SwTxtFly::CalcSmart( const SdrObject *pObj ) const
{
    _FlyCntnt eOrder;

    // 11839: Nur die X-Positionen sind interessant, die Y-Positionen des
    // CurrentFrames koennen sich noch aendern (wachsen).

    const long nCurrLeft = pCurrFrm->Prt().Left() + pCurrFrm->Frm().Left();
    const long nCurrRight = pCurrFrm->Prt().Right() + pCurrFrm->Frm().Left();
    const SwRect aRect( GetBoundRect( pObj ) );
    long nFlyLeft = aRect.Left();
    long nFlyRight = aRect.Right();

    if ( nFlyRight < nCurrLeft || nFlyLeft > nCurrRight )
        eOrder = SURROUND_PARALLEL;
    else
    {
#ifndef USED
        long nLeft = nFlyLeft - nCurrLeft;
        long nRight = nCurrRight - nFlyRight;
        if( nFlyRight - nFlyLeft > FRAME_MAX )
        {
            if( nLeft < nRight )
                nLeft = 0;
            else
                nRight = 0;
        }
        if( nLeft < TEXT_MIN )
            nLeft = 0;
        if( nRight < TEXT_MIN )
            nRight = 0;
        if( nLeft )
            eOrder = nRight ? SURROUND_PARALLEL : SURROUND_LEFT;
        else
            eOrder = nRight ? SURROUND_RIGHT: SURROUND_NONE;
#else
        if ( nFlyRight > nCurrRight )
            nFlyRight = nCurrRight;
        if ( nFlyLeft < nCurrLeft )
            nFlyLeft = nCurrLeft;
        const long nCurrPart = ( nCurrRight - nCurrLeft )/3;
        const long nFlyWidth = nFlyRight - nFlyLeft;

        if( nFlyWidth < nCurrPart )
            eOrder = SURROUND_PARALLEL;
        else
        {
            if( nFlyWidth > (nCurrPart * 2) )
                eOrder = SURROUND_NONE;
            else
            {
                const long nHalfCurr = ( nCurrRight + nCurrLeft ) / 2;
                const long nHalfFly  = ( nFlyRight + nFlyLeft ) / 2 ;
                if ( nHalfFly == nHalfCurr )
                    eOrder = SURROUND_COLUMN;
                else
                    eOrder = nHalfFly < nHalfCurr ?
                             SURROUND_RIGHT : SURROUND_LEFT;
            }
        }
#endif
    }

    return eOrder;
}

/*************************************************************************
 *                      SwTxtFly::GetOrder()
 *************************************************************************/

_FlyCntnt SwTxtFly::GetOrder( const SdrObject *pObj ) const
{
    const SwFrmFmt *pFmt = ((SwContact*)GetUserCall(pObj))->GetFmt();
    const SwFmtSurround &rFlyFmt = pFmt->GetSurround();
    _FlyCntnt eOrder = rFlyFmt.GetSurround();

    if( rFlyFmt.IsAnchorOnly() && lcl_TheAnchor( pObj ) != GetMaster() )
    {
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        if( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
            FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
            return SURROUND_NONE;
    }

    // Beim Durchlauf und Nowrap wird smart ignoriert.
    if( SURROUND_THROUGHT == eOrder || SURROUND_NONE == eOrder )
        return eOrder;

    // "idealer Seitenumlauf":
    if( SURROUND_IDEAL == eOrder )
        eOrder = CalcSmart( pObj ); //Bei SMART wird die Order automatisch berechnet:

    return eOrder;
}

/*************************************************************************
 *                      SwTxtFly::IsAnyFrm( SwRect )
 *
 * IN: dokumentglobal
 *
 * dient zum Abschalten des SwTxtFly, wenn keine Objekte ueberlappen (Relax)
 *
 *************************************************************************/

sal_Bool SwTxtFly::IsAnyFrm( const SwRect &rLine ) const
{
    ASSERT( bOn, "IsAnyFrm: Why?" );
    return ForEach( rLine, NULL, sal_False );
}

const SwFrmFmt* SwTxtFrm::IsFirstBullet()
{
    GetFormatted();
    const SwLineLayout *pLayout = GetPara();
    if( !pLayout ||
        ( !pLayout->GetLen() && !pLayout->GetPortion() && !pLayout->GetNext() ) )
        return NULL;

    SwLinePortion* pPor = pLayout->GetFirstPortion();
    while( pPor->IsFlyPortion() && pPor->GetPortion() )
        pPor = pPor->GetPortion();
    SwLinePortion* pTmp;
    do
    {
        pTmp = pLayout->GetFirstPortion();
        while( pTmp && !pTmp->InTxtGrp() )
            pTmp = pTmp->GetPortion();
        pLayout = pLayout->GetNext();
    } while( !pTmp && pLayout );
    long nMaxHeight = pTmp ? ( pTmp->Height() * 15 ) / 10 : 0;
    if( !nMaxHeight )
        return NULL;

    long nMaxWidth = 2*pTmp->Height();
    if( pPor->IsFlyCntPortion() &&
        ( pPor->Height() < nMaxHeight && pPor->Width() < nMaxWidth ) )
        return ((SwFlyCntPortion*)pPor)->GetFrmFmt();

    const SwFrmFmt* pRet = NULL;

    SwPageFrm* pPage = FindPageFrm();
    const SwSortDrawObjs *pSorted = pPage->GetSortedObjs();
    if( pSorted )
    {
        for ( MSHORT i = 0; i < pSorted->Count(); ++i )
        {
            const SdrObject *pObj = (*pSorted)[i];
            if( this == lcl_TheAnchor( pObj ) )
            {
                SwRect aBound( GetBoundRect( pObj ) );
                if( aBound.Top() > Frm().Top() + Prt().Top() )
                    aBound.Top( Frm().Top() + Prt().Top() );
                if( aBound.Left() > Frm().Left() + Prt().Left() )
                    aBound.Left( Frm().Left() + Prt().Left() );
                if( aBound.Height() < nMaxHeight && aBound.Width() < nMaxWidth )
                {
                    pRet = ((SwContact*)GetUserCall(pObj))->GetFmt();
                    _FlyCntnt eOrder = pRet->GetSurround().GetSurround();
                    if( SURROUND_THROUGHT == eOrder || SURROUND_NONE == eOrder )
                        pRet = NULL;
                    else
                        break;
                }
            }
        }
    }
    return pRet;
}


