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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

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
#include "txtfrm.hxx"     // SwTxtFrm
#include "itrform2.hxx"   // SwTxtFormatter
#include "porfly.hxx"     // NewFlyCntPortion
#include "porfld.hxx"     // SwGrfNumPortion
#include "txtfly.hxx"     // SwTxtFly
#include "txtpaint.hxx"   // SwSaveClip
#include "txtatr.hxx"     // SwTxtFlyCnt
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "fmtcnct.hxx"  // SwFmtChain
#include <pormulti.hxx>     // SwMultiPortion
#include <svx/obj3d.hxx>
#include <editeng/txtrange.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
// --> OD 2004-06-16 #i28701#
#include <editeng/lspcitem.hxx>
// <--
#include <txtflcnt.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <fmtflcnt.hxx>
#include <frmfmt.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>
#include <sortedobjs.hxx>
#include <layouter.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <svx/svdoedge.hxx>
#include "doc.hxx"

#if OSL_DEBUG_LEVEL > 1
#include "viewopt.hxx"  // SwViewOptions, nur zum Testen (Test2)
#include "doc.hxx"
#endif


using namespace ::com::sun::star;

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

void SwTxtFormatter::CalcUnclipped( SwTwips& rTop, SwTwips& rBottom )
{
    OSL_ENSURE( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::CalcUnclipped with unswapped frame" );

    long nFlyAsc, nFlyDesc;
    // OD 08.01.2004 #i11859# - use new method <SwLineLayout::MaxAscentDescent(..)>
    //lcl_MaxAscDescent( pCurr, rTop, rBottom, nFlyAsc, nFlyDesc );
    pCurr->MaxAscentDescent( rTop, rBottom, nFlyAsc, nFlyDesc );
    rTop = Y() + GetCurr()->GetAscent();
    rBottom = rTop + nFlyDesc;
    rTop -= nFlyAsc;
}

/*************************************************************************
 * SwTxtFormatter::UpdatePos() aktualisiert die Referenzpunkte der zeichengeb.
 * Objekte, z. B. nach Adjustierung ( rechtsbuendig, Blocksatz etc. )
 * ( hauptsaechlich Korrrektur der X-Position )
 *************************************************************************/

void SwTxtFormatter::UpdatePos( SwLineLayout *pCurrent, Point aStart,
    xub_StrLen nStartIdx, sal_Bool bAllWays ) const
{
    OSL_ENSURE( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::UpdatePos with unswapped frame" );

    if( GetInfo().IsTest() )
        return;
    SwLinePortion *pFirst = pCurrent->GetFirstPortion();
    SwLinePortion *pPos = pFirst;
    SwTxtPaintInfo aTmpInf( GetInfo() );
    aTmpInf.SetpSpaceAdd( pCurrent->GetpLLSpaceAdd() );
    aTmpInf.ResetSpaceIdx();
    aTmpInf.SetKanaComp( pCurrent->GetpKanaComp() );
    aTmpInf.ResetKanaIdx();

    // Die Groesse des Frames
    aTmpInf.SetIdx( nStartIdx );
    aTmpInf.SetPos( aStart );

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
    // OD 08.01.2004 #i11859# - use new method <SwLineLayout::MaxAscentDescent(..)>
    //lcl_MaxAscDescent( pPos, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );
    pCurrent->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );

    KSHORT nTmpHeight = pCurrent->GetRealHeight();
    KSHORT nAscent = pCurrent->GetAscent() + nTmpHeight - pCurrent->Height();
    objectpositioning::AsCharFlags nFlags = AS_CHAR_ULSPACE;
    if( GetMulti() )
    {
        aTmpInf.SetDirection( GetMulti()->GetDirection() );
        if( GetMulti()->HasRotation() )
        {
            nFlags |= AS_CHAR_ROTATE;
            if( GetMulti()->IsRevers() )
            {
                nFlags |= AS_CHAR_REVERSE;
                aTmpInf.X( aTmpInf.X() - nAscent );
            }
            else
                aTmpInf.X( aTmpInf.X() + nAscent );
        }
        else
        {
            if ( GetMulti()->IsBidi() )
                nFlags |= AS_CHAR_BIDI;
            aTmpInf.Y( aTmpInf.Y() + nAscent );
        }
    }
    else
        aTmpInf.Y( aTmpInf.Y() + nAscent );

    while( pPos )
    {
        // bislang ist mir nur ein Fall bekannt, wo die Positionsaenderung
        // (verursacht durch das Adjustment) fuer eine Portion wichtig
        // sein koennte: Bei FlyCntPortions muss ein SetRefPoint erfolgen.
        if( ( pPos->IsFlyCntPortion() || pPos->IsGrfNumPortion() )
            && ( bAllWays || !IsQuick() ) )
        {
            // OD 08.01.2004 #i11859# - use new method <SwLineLayout::MaxAscentDescent(..)>
            //lcl_MaxAscDescent( pFirst, nTmpAscent, nTmpDescent,
            //                  nFlyAsc, nFlyDesc, pPos );
            pCurrent->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, pPos );

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
                Point aBase( aTmpInf.GetPos() );
                if ( GetInfo().GetTxtFrm()->IsVertical() )
                    GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( aBase );

                ((SwFlyCntPortion*)pPos)->SetBase( *aTmpInf.GetTxtFrm(),
                    aBase, nTmpAscent, nTmpDescent, nFlyAsc,
                    nFlyDesc, nFlags );
            }
        }
        if( pPos->IsMultiPortion() && ((SwMultiPortion*)pPos)->HasFlyInCntnt() )
        {
            OSL_ENSURE( !GetMulti(), "Too much multi" );
            ((SwTxtFormatter*)this)->pMulti = (SwMultiPortion*)pPos;
            SwLineLayout *pLay = &GetMulti()->GetRoot();
            Point aSt( aTmpInf.X(), aStart.Y() );

            if ( GetMulti()->HasBrackets() )
            {
                OSL_ENSURE( GetMulti()->IsDouble(), "Brackets only for doubles");
                aSt.X() += ((SwDoubleLinePortion*)GetMulti())->PreWidth();
            }
            else if( GetMulti()->HasRotation() )
            {
                aSt.Y() += pCurrent->GetAscent() - GetMulti()->GetAscent();
                if( GetMulti()->IsRevers() )
                    aSt.X() += GetMulti()->Width();
                else
                    aSt.Y() += GetMulti()->Height();
               }
            else if ( GetMulti()->IsBidi() )
                // jump to end of the bidi portion
                aSt.X() += pLay->Width();

            xub_StrLen nStIdx = aTmpInf.GetIdx();
            do
            {
                UpdatePos( pLay, aSt, nStIdx, bAllWays );
                nStIdx = nStIdx + pLay->GetLen();
                aSt.Y() += pLay->Height();
                pLay = pLay->GetNext();
            } while ( pLay );
            ((SwTxtFormatter*)this)->pMulti = NULL;
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
    OSL_ENSURE( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::AlignFlyInCntBase with unswapped frame" );

    if( GetInfo().IsTest() )
        return;
    SwLinePortion *pFirst = pCurr->GetFirstPortion();
    SwLinePortion *pPos = pFirst;
    objectpositioning::AsCharFlags nFlags = AS_CHAR_NOFLAG;
    if( GetMulti() && GetMulti()->HasRotation() )
    {
        nFlags |= AS_CHAR_ROTATE;
        if( GetMulti()->IsRevers() )
            nFlags |= AS_CHAR_REVERSE;
    }

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;

    while( pPos )
    {
        if( pPos->IsFlyCntPortion() || pPos->IsGrfNumPortion() )
        {
            // OD 08.01.2004 #i11859# - use new method <SwLineLayout::MaxAscentDescent(..)>
            //lcl_MaxAscDescent( pFirst, nTmpAscent, nTmpDescent,
            //                  nFlyAsc, nFlyDesc, pPos );
            pCurr->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, pPos );

            if( pPos->IsGrfNumPortion() )
                ((SwGrfNumPortion*)pPos)->SetBase( nTmpAscent, nTmpDescent,
                                                   nFlyAsc, nFlyDesc );
            else
            {
                Point aBase;
                if ( GetInfo().GetTxtFrm()->IsVertical() )
                {
                    nBaseLine = GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( nBaseLine );
                    aBase = Point( nBaseLine, ((SwFlyCntPortion*)pPos)->GetRefPoint().Y() );
                }
                else
                    aBase = Point( ((SwFlyCntPortion*)pPos)->GetRefPoint().X(), nBaseLine );

                ((SwFlyCntPortion*)pPos)->SetBase( *GetInfo().GetTxtFrm(), aBase, nTmpAscent, nTmpDescent,
                    nFlyAsc, nFlyDesc, nFlags );
            }
        }
        pPos = pPos->GetPortion();
    }
}

/*************************************************************************
 *                      SwTxtFly::ChkFlyUnderflow()
 * This is called after the real height of the line has been calculated
 * Therefore it is possible, that more flys from below intersect with the
 * line, or that flys from above do not intersect with the line anymore
 * We check this and return true if so, meaning that the line has to be
 * formatted again
 *************************************************************************/

sal_Bool SwTxtFormatter::ChkFlyUnderflow( SwTxtFormatInfo &rInf ) const
{
    OSL_ENSURE( rInf.GetTxtFly()->IsOn(), "SwTxtFormatter::ChkFlyUnderflow: why?" );
    if( GetCurr() )
    {
        // Erst pruefen wir, ob ueberhaupt ein Fly mit der Zeile ueberlappt.
        // = GetLineHeight()
        const long nHeight = GetCurr()->GetRealHeight();
        SwRect aLine( GetLeftMargin(), Y(), rInf.RealWidth(), nHeight );

        SwRect aLineVert( aLine );
        if ( pFrm->IsVertical() )
            pFrm->SwitchHorizontalToVertical( aLineVert );
        SwRect aInter( rInf.GetTxtFly()->GetFrm( aLineVert ) );
        if ( pFrm->IsVertical() )
            pFrm->SwitchVerticalToHorizontal( aInter );

        if( !aInter.HasArea() )
            return sal_False;

        // Nun ueberpruefen wir jede Portion, die sich haette senken koennen,
        // ob sie mit dem Fly ueberlappt.
        const SwLinePortion *pPos = GetCurr()->GetFirstPortion();
        aLine.Pos().Y() = Y() + GetCurr()->GetRealHeight() - GetCurr()->Height();
        aLine.Height( GetCurr()->Height() );

        while( pPos )
        {
            aLine.Width( pPos->Width() );

            aLineVert = aLine;
            if ( pFrm->IsVertical() )
                pFrm->SwitchHorizontalToVertical( aLineVert );
            aInter = rInf.GetTxtFly()->GetFrm( aLineVert );
            if ( pFrm->IsVertical() )
                pFrm->SwitchVerticalToHorizontal( aInter );

            // new flys from below?
            if( !pPos->IsFlyPortion() )
            {
                if( aInter.IsOver( aLine ) )
                {
                    aInter._Intersection( aLine );
                    if( aInter.HasArea() )
                    {
                        // to be evaluated during reformat of this line:
                        // RealHeight including spacing
                        rInf.SetLineHeight( KSHORT(nHeight) );
                        // Height without extra spacing
                        rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
                        return sal_True;
                    }
                }
            }
            else
            {
                // the fly portion is not anylonger intersected by a fly
                if ( ! aInter.IsOver( aLine ) )
                {
                    rInf.SetLineHeight( KSHORT(nHeight) );
                    rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
                    return sal_True;
                }
                else
                {
                    aInter._Intersection( aLine );

                    // no area means a fly has become invalid because of
                    // lowering the line => reformat the line
                    // we also have to reformat the line, if the fly size
                    // differs from the intersection intervals size
                    if( ! aInter.HasArea() ||
                        ((SwFlyPortion*)pPos)->GetFixWidth() != aInter.Width() )
                    {
                        rInf.SetLineHeight( KSHORT(nHeight) );
                        rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
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

void SwTxtFormatter::CalcFlyWidth( SwTxtFormatInfo &rInf )
{
    if( GetMulti() || rInf.GetFly() )
        return;

    SwTxtFly *pTxtFly = rInf.GetTxtFly();
    if( !pTxtFly->IsOn() || rInf.IsIgnoreFly() )
        return;

    const SwLinePortion *pLast = rInf.GetLast();

    long nAscent;
    long nTop = Y();
    long nHeight;

    if( rInf.GetLineHeight() )
    {
        // real line height has already been calculated, we only have to
        // search for intersections in the lower part of the strip
        nAscent = pCurr->GetAscent();
        nHeight = rInf.GetLineNettoHeight();
        nTop += rInf.GetLineHeight() - nHeight;
    }
    else
    {
        nAscent = pLast->GetAscent();
        nHeight = pLast->Height();

        // we make a first guess for the lines real height
        if ( ! pCurr->GetRealHeight() )
            CalcRealHeight();

        if ( pCurr->GetRealHeight() > nHeight )
            nTop += pCurr->GetRealHeight() - nHeight;
        else
            // important for fixed space between lines
            nHeight = pCurr->GetRealHeight();
    }

    const long nLeftMar = GetLeftMargin();
    const long nLeftMin = (rInf.X() || GetDropLeft()) ? nLeftMar : GetLeftMin();

    SwRect aLine( rInf.X() + nLeftMin, nTop, rInf.RealWidth() - rInf.X()
                  + nLeftMar - nLeftMin , nHeight );

    SwRect aLineVert( aLine );
    if ( pFrm->IsRightToLeft() )
        pFrm->SwitchLTRtoRTL( aLineVert );

    if ( pFrm->IsVertical() )
        pFrm->SwitchHorizontalToVertical( aLineVert );
    SwRect aInter( pTxtFly->GetFrm( aLineVert ) );

    if ( pFrm->IsRightToLeft() )
        pFrm->SwitchRTLtoLTR( aInter );

    if ( pFrm->IsVertical() )
        pFrm->SwitchVerticalToHorizontal( aInter );

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

            long nAddMar = 0;
            if ( pFrm->IsRightToLeft() )
            {
                nAddMar = pFrm->Frm().Right() - Right();
                if ( nAddMar < 0 )
                    nAddMar = 0;
            }
            else
                nAddMar = nLeftMar - nFrmLeft;

            aInter.Width( aInter.Width() + nAddMar );
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
            rInf.ForcedLeftMargin( (sal_uInt16)aInter.Width() );
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
            long nNextTop = pTxtFly->GetNextTop();
            if ( pFrm->IsVertical() )
                nNextTop = pFrm->SwitchVerticalToHorizontal( nNextTop );
            if( nNextTop > aInter.Bottom() )
            {
                SwTwips nH = nNextTop - aInter.Top();
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

        if( pFly->Fix() < rInf.Width() )
            rInf.Width( pFly->Fix() );

        GETGRID( pFrm->FindPageFrm() )
        if ( pGrid )
        {
            const SwPageFrm* pPageFrm = pFrm->FindPageFrm();
            const SwLayoutFrm* pBody = pPageFrm->FindBodyCont();

            SWRECTFN( pPageFrm )

            const long nGridOrigin = pBody ?
                                    (pBody->*fnRect->fnGetPrtLeft)() :
                                    (pPageFrm->*fnRect->fnGetPrtLeft)();

            const SwDoc *pDoc = rInf.GetTxtFrm()->GetNode()->GetDoc();
            const sal_uInt16 nGridWidth = GETGRIDWIDTH( pGrid, pDoc);   //for textgrid refactor

            SwTwips nStartX = GetLeftMargin();
            if ( bVert )
            {
                Point aPoint( nStartX, 0 );
                pFrm->SwitchHorizontalToVertical( aPoint );
                nStartX = aPoint.Y();
            }

            const SwTwips nOfst = nStartX - nGridOrigin;
            const SwTwips nTmpWidth = rInf.Width() + nOfst;

            const sal_uLong i = nTmpWidth / nGridWidth + 1;

            const long nNewWidth = ( i - 1 ) * nGridWidth - nOfst;
            if ( nNewWidth > 0 )
                rInf.Width( (sal_uInt16)nNewWidth );
            else
                rInf.Width( 0 );
        }
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

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
    // OD 08.01.2004 #i11859# - use new method <SwLineLayout::MaxAscentDescent(..)>
    //SwLinePortion *pPos = pCurr->GetFirstPortion();
    //lcl_MaxAscDescent( pPos, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );
    pCurr->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );

    // Wenn der Ascent des Rahmens groesser als der Ascent der akt. Portion
    // ist, wird dieser bei der Base-Berechnung verwendet, sonst wuerde
    // der Rahmen zunaechst zu weit nach oben gesetzt, um dann doch wieder
    // nach unten zu rutschen und dabei ein Repaint in einem Bereich ausloesen,
    // indem er niemals wirklich war.
    KSHORT nAscent = 0;

    const bool bTxtFrmVertical = GetInfo().GetTxtFrm()->IsVertical();

    const bool bUseFlyAscent = pFly && pFly->GetValidPosFlag() &&
                               0 != ( bTxtFrmVertical ?
                                      pFly->GetRefPoint().X() :
                                      pFly->GetRefPoint().Y() );

    if ( bUseFlyAscent )
         nAscent = static_cast<sal_uInt16>( Abs( int( bTxtFrmVertical ?
                                                  pFly->GetRelPos().X() :
                                                  pFly->GetRelPos().Y() ) ) );

    // check if be prefer to use the ascent of the last portion:
    if ( IsQuick() ||
         !bUseFlyAscent ||
         nAscent < rInf.GetLast()->GetAscent() )
    {
        nAscent = rInf.GetLast()->GetAscent();
    }
    else if( nAscent > nFlyAsc )
        nFlyAsc = nAscent;

    Point aBase( GetLeftMargin() + rInf.X(), Y() + nAscent );
    objectpositioning::AsCharFlags nMode = IsQuick() ? AS_CHAR_QUICK : 0;
    if( GetMulti() && GetMulti()->HasRotation() )
    {
        nMode |= AS_CHAR_ROTATE;
        if( GetMulti()->IsRevers() )
            nMode |= AS_CHAR_REVERSE;
    }

    Point aTmpBase( aBase );
    if ( GetInfo().GetTxtFrm()->IsVertical() )
        GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( aTmpBase );

    if( pFly )
    {
        pRet = new SwFlyCntPortion( *GetInfo().GetTxtFrm(), pFly, aTmpBase,
                                    nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, nMode );
        // Wir muessen sicherstellen, dass unser Font wieder im OutputDevice
        // steht. Es koennte sein, dass der FlyInCnt frisch eingefuegt wurde,
        // dann hat GetFlyFrm dazu gefuehrt, dass er neu angelegt wird.
        // Dessen Frames werden sofort formatiert, die verstellen den Font
        // und schon haben wir den Salat (3322).
        rInf.SelectFont();
        if( pRet->GetAscent() > nAscent )
        {
            aBase.Y() = Y() + pRet->GetAscent();
            nMode |= AS_CHAR_ULSPACE;
            if( !rInf.IsTest() )
                aTmpBase = aBase;
                if ( GetInfo().GetTxtFrm()->IsVertical() )
                    GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( aTmpBase );

                pRet->SetBase( *rInf.GetTxtFrm(), aTmpBase, nTmpAscent,
                               nTmpDescent, nFlyAsc, nFlyDesc, nMode );
        }
    }
    else
    {
        pRet = new SwFlyCntPortion( *rInf.GetTxtFrm(), (SwDrawContact*)pFrmFmt->FindContactObj(),
           aTmpBase, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, nMode );
    }
    return pRet;
}



/*************************************************************************
 *                      SwTxtFly::SwTxtFly()
 *************************************************************************/

SwTxtFly::SwTxtFly( const SwTxtFly& rTxtFly )
{
    pPage = rTxtFly.pPage;
    mpCurrAnchoredObj = rTxtFly.mpCurrAnchoredObj;
    pCurrFrm = rTxtFly.pCurrFrm;
    pMaster = rTxtFly.pMaster;
    if( rTxtFly.mpAnchoredObjList )
    {
        mpAnchoredObjList = new SwAnchoredObjList( *(rTxtFly.mpAnchoredObjList) );
    }
    else
    {
        mpAnchoredObjList = NULL;
    }

    bOn = rTxtFly.bOn;
    bLeftSide = rTxtFly.bLeftSide;
    bTopRule = rTxtFly.bTopRule;
    nMinBottom = rTxtFly.nMinBottom;
    nNextTop = rTxtFly.nNextTop;
    nIndex = rTxtFly.nIndex;
    mbIgnoreCurrentFrame = rTxtFly.mbIgnoreCurrentFrame;
    mbIgnoreContour = rTxtFly.mbIgnoreContour;
    mbIgnoreObjsInHeaderFooter = rTxtFly.mbIgnoreObjsInHeaderFooter;
}

void SwTxtFly::CtorInitTxtFly( const SwTxtFrm *pFrm )
{
    mbIgnoreCurrentFrame = sal_False;
    mbIgnoreContour = sal_False;
    // --> OD 2004-12-17 #118809#
    mbIgnoreObjsInHeaderFooter = sal_False;
    // <--
    pPage = pFrm->FindPageFrm();
    const SwFlyFrm* pTmp = pFrm->FindFlyFrm();
    // --> OD 2006-08-15 #i68520#
    mpCurrAnchoredObj = pTmp;
    // <--
    pCurrFrm = pFrm;
    pMaster = pCurrFrm->IsFollow() ? NULL : pCurrFrm;
    // --> OD 2006-08-15 #i68520#
    mpAnchoredObjList = NULL;
    // <--
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
        SWRECTFN( pCurrFrm )
        if( bTop )
            (aRet.*fnRect->fnSetTop)( (rRect.*fnRect->fnGetTop)() );

        // 8110: Bottom nicht immer anpassen.
        const SwTwips nRetBottom = (aRet.*fnRect->fnGetBottom)();
        const SwTwips nRectBottom = (rRect.*fnRect->fnGetBottom)();
        if ( (*fnRect->fnYDiff)( nRetBottom, nRectBottom ) > 0 ||
             (aRet.*fnRect->fnGetHeight)() < 0 )
            (aRet.*fnRect->fnSetBottom)( nRectBottom );
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
    SWAP_IF_SWAPPED( pCurrFrm )

    OSL_ENSURE( bOn, "IsAnyFrm: Why?" );
    SwRect aRect( pCurrFrm->Frm().Pos() + pCurrFrm->Prt().Pos(),
        pCurrFrm->Prt().SSize() );

    const sal_Bool bRet = ForEach( aRect, NULL, sal_False );
    UNDO_SWAP( pCurrFrm )
    return bRet;
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
   OSL_ENSURE( bOn, "SwTxtFly::IsAnyObj: Who's knocking?" );

    SwRect aRect( rRect );
    if ( aRect.IsEmpty() )
        aRect = SwRect( pCurrFrm->Frm().Pos() + pCurrFrm->Prt().Pos(),
                        pCurrFrm->Prt().SSize() );

    const SwSortedObjs *pSorted = pPage->GetSortedObjs();
    if( pSorted ) // Eigentlich ist durch bOn sichergestellt, dass es an der
    // Seite Objekte gibt, aber wer weiss, wer inzwischen etwas geloescht hat.
    {
        for ( MSHORT i = 0; i < pSorted->Count(); ++i )
        {
            const SwAnchoredObject* pObj = (*pSorted)[i];

            const SwRect aBound( pObj->GetObjRectWithSpaces() );

            // Optimierung
            if( pObj->GetObjRect().Left() > aRect.Right() )
                continue;

            // --> OD 2006-08-15 #i68520#
            if( mpCurrAnchoredObj != pObj && aBound.IsOver( aRect ) )
            // <--
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
    // --> OD 2006-08-15 #i68520#
    const sal_uInt32 nCurrOrd = mpCurrAnchoredObj
                            ? mpCurrAnchoredObj->GetDrawObj()->GetOrdNum()
                            : SAL_MAX_UINT32;
    // <--
    OSL_ENSURE( !bTopRule, "DrawTextOpaque: Wrong TopRule" );

    // --> OD 2006-08-15 #i68520#
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    // <--
    {
        MSHORT nHellId = pPage->getRootFrm()->GetCurrShell()->getIDocumentDrawModelAccess()->GetHellId();
        for( MSHORT i = 0; i < nCount; ++i )
        {
            // --> OD 2006-08-15 #i68520#
            const SwAnchoredObject* pTmpAnchoredObj = (*mpAnchoredObjList)[i];
            if( dynamic_cast<const SwFlyFrm*>(pTmpAnchoredObj) &&
                mpCurrAnchoredObj != pTmpAnchoredObj )
            // <--
            {
                // --> OD 2006-08-15 #i68520#
                const SwFlyFrm* pFly = dynamic_cast<const SwFlyFrm*>(pTmpAnchoredObj);
                // <--
                if( aRegion.GetOrigin().IsOver( pFly->Frm() ) )
                {
                    const SwFrmFmt *pFmt = pFly->GetFmt();
                    const SwFmtSurround &rSur = pFmt->GetSurround();
                    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                        //Nur undurchsichtige und weiter oben liegende.
                    /// OD 08.10.2002 #103898# - add condition
                    /// <!(pFly->IsBackgroundTransparent() || pFly->IsShadowTransparent())>
                    if( !( pFly->IsBackgroundTransparent()
                           || pFly->IsShadowTransparent() ) &&
                        SURROUND_THROUGHT == rSur.GetSurround() &&
                        ( !rSur.IsAnchorOnly() ||
                          // --> OD 2006-08-15 #i68520#
                          GetMaster() == pFly->GetAnchorFrm() ||
                          // <--
                          ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                           (FLY_AT_CHAR != rAnchor.GetAnchorId())
                          )
                        ) &&
                        // --> OD 2006-08-15 #i68520#
                        pTmpAnchoredObj->GetDrawObj()->GetLayer() != nHellId &&
                        nCurrOrd < pTmpAnchoredObj->GetDrawObj()->GetOrdNum()
                        // <--
                      )
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

void SwTxtFly::DrawFlyRect( OutputDevice* pOut, const SwRect &rRect,
        const SwTxtPaintInfo &rInf, sal_Bool bNoGraphic )
{
    SwRegionRects aRegion( rRect );
    OSL_ENSURE( !bTopRule, "DrawFlyRect: Wrong TopRule" );
    // --> OD 2006-08-15 #i68520#
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    // <--
    {
        MSHORT nHellId = pPage->getRootFrm()->GetCurrShell()->getIDocumentDrawModelAccess()->GetHellId();
        for( MSHORT i = 0; i < nCount; ++i )
        {
            // --> OD 2006-08-15 #i68520#
            const SwAnchoredObject* pAnchoredObjTmp = (*mpAnchoredObjList)[i];
            if( mpCurrAnchoredObj != pAnchoredObjTmp &&
                dynamic_cast<const SwFlyFrm*>(pAnchoredObjTmp) )
            // <--
            {
                // --> OD 2006-08-15 #i68520#
                const SwFmtSurround& rSur = pAnchoredObjTmp->GetFrmFmt().GetSurround();
                // <--

                // OD 24.01.2003 #106593# - correct clipping of fly frame area.
                // Consider that fly frame background/shadow can be transparent
                // and <SwAlignRect(..)> fly frame area
                // --> OD 2006-08-15 #i68520#
                const SwFlyFrm* pFly = dynamic_cast<const SwFlyFrm*>(pAnchoredObjTmp);
                // <--
                // --> OD 2005-06-08 #i47804# - consider transparent graphics
                // and OLE objects.
                bool bClipFlyArea =
                        ( ( SURROUND_THROUGHT == rSur.GetSurround() )
                          // --> OD 2006-08-15 #i68520#
                          ? (pAnchoredObjTmp->GetDrawObj()->GetLayer() != nHellId)
                          // <--
                          : !rSur.IsContour() ) &&
                        !pFly->IsBackgroundTransparent() &&
                        !pFly->IsShadowTransparent() &&
                        ( !pFly->Lower() ||
                          !pFly->Lower()->IsNoTxtFrm() ||
                          !static_cast<const SwNoTxtFrm*>(pFly->Lower())->IsTransparent() );
                // <--
                if ( bClipFlyArea )
                {
                    // --> OD 2006-08-15 #i68520#
                    SwRect aFly( pAnchoredObjTmp->GetObjRect() );
                    // <--
                    // OD 24.01.2003 #106593#
                    ::SwAlignRect( aFly, pPage->getRootFrm()->GetCurrShell() );
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
            OSL_ENSURE( ((SvxBrushItem*)-1) != rInf.GetBrushItem(),
                    "DrawRect: Uninitialized BrushItem!" );
            ::DrawGraphic( rInf.GetBrushItem(), pOut, rInf.GetBrushRect(),
                       aRegion[i] );
        }
    }
}

// --> OD 2004-10-06 #i26945# - change first parameter:
// Now it's the <SwAnchoredObject> instance of the floating screen object
sal_Bool SwTxtFly::GetTop( const SwAnchoredObject* _pAnchoredObj,
                           const sal_Bool bInFtn,
                           const sal_Bool bInFooterOrHeader )
// <--
{
    // --> OD 2006-08-15 #i68520#
    // <mpCurrAnchoredObj> is set, if <pCurrFrm> is inside a fly frame
    if( _pAnchoredObj != mpCurrAnchoredObj )
    // <--
    {
        // --> OD 2004-10-06 #i26945#
        const SdrObject* pNew = _pAnchoredObj->GetDrawObj();
        // <--
        // #102344# Ignore connectors which have one or more connections
        if(pNew && pNew->ISA(SdrEdgeObj))
        {
            if(((SdrEdgeObj*)pNew)->GetConnectedNode(sal_True)
                || ((SdrEdgeObj*)pNew)->GetConnectedNode(sal_False))
            {
                return sal_False;
            }
        }

        if( ( bInFtn || bInFooterOrHeader ) && bTopRule )
        {
            // --> OD 2004-10-06 #i26945#
            const SwFrmFmt& rFrmFmt = _pAnchoredObj->GetFrmFmt();
            const SwFmtAnchor& rNewA = rFrmFmt.GetAnchor();
            // <--
            if (FLY_AT_PAGE == rNewA.GetAnchorId())
            {
                if ( bInFtn )
                    return sal_False;

                if ( bInFooterOrHeader )
                {
                    SwFmtVertOrient aVert( rFrmFmt.GetVertOrient() );
                    sal_Bool bVertPrt = aVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA ||
                            aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA;
                    if( bVertPrt )
                        return sal_False;
                }
            }
        }

        // --> OD 2006-08-15 #i68520#
        // bEvade: consider pNew, if we are not inside a fly
        //         consider pNew, if pNew is lower of <mpCurrAnchoredObj>
        sal_Bool bEvade = !mpCurrAnchoredObj ||
                          Is_Lower_Of( dynamic_cast<const SwFlyFrm*>(mpCurrAnchoredObj), pNew);

        if ( !bEvade )
        {
            // We are currently inside a fly frame and pNew is not
            // inside this fly frame. We can do some more checks if
            // we have to consider pNew.

            // If bTopRule is not set, we ignore the frame types.
            // We directly check the z-order
            if ( !bTopRule )
                bEvade = sal_True;
            else
            {
                // innerhalb von verketteten Flys wird nur Lowern ausgewichen
                // --> OD 2006-08-15 #i68520#
                const SwFmtChain &rChain = mpCurrAnchoredObj->GetFrmFmt().GetChain();
                // <--
                if ( !rChain.GetPrev() && !rChain.GetNext() )
                {
                    // --> OD 2004-10-06 #i26945#
                    const SwFmtAnchor& rNewA = _pAnchoredObj->GetFrmFmt().GetAnchor();
                    // <--
                    // --> OD 2006-08-15 #i68520#
                    const SwFmtAnchor& rCurrA = mpCurrAnchoredObj->GetFrmFmt().GetAnchor();
                    // <--

                    // If <mpCurrAnchoredObj> is anchored as character, its content
                    // does not wrap around pNew
                    if (FLY_AS_CHAR == rCurrA.GetAnchorId())
                        return sal_False;

                    // If pNew is anchored to page and <mpCurrAnchoredObj is not anchored
                    // to page, the content of <mpCurrAnchoredObj> does not wrap around pNew
                    // If both pNew and <mpCurrAnchoredObj> are anchored to page, we can do
                    // some more checks
                    if (FLY_AT_PAGE == rNewA.GetAnchorId())
                    {
                        if (FLY_AT_PAGE == rCurrA.GetAnchorId())
                        {
                            bEvade = sal_True;
                        }
                        else
                            return sal_False;
                    }
                    else if (FLY_AT_PAGE == rCurrA.GetAnchorId())
                        return sal_False; // Seitengebundene weichen nur seitengeb. aus
                    else if (FLY_AT_FLY == rNewA.GetAnchorId())
                        bEvade = sal_True; // Nicht seitengeb. weichen Rahmengeb. aus
                    else if( FLY_AT_FLY == rCurrA.GetAnchorId() )
                        return sal_False; // Rahmengebundene weichen abs.geb. nicht aus
                    // --> OD 2006-01-30 #i57062#
                    // In order to avoid loop situation, it's decided to adjust
                    // the wrapping behaviour of content of at-paragraph/at-character
                    // anchored objects to one in the page header/footer and
                    // the document body --> content of at-paragraph/at-character
                    // anchored objects doesn't wrap around each other.
//                    else if( bInFooterOrHeader )
//                        return sal_False;  // In header or footer no wrapping
//                                           // if both bounded at paragraph
//                    else // Zwei Flies mit (auto-)absatzgebunder Verankerung ...
//                    // ... entscheiden nach der Reihenfolge ihrer Anker im Dok.
//                      bEvade = rNewA.GetCntntAnchor()->nNode.GetIndex() <=
//                              rCurrA.GetCntntAnchor()->nNode.GetIndex();
                    else
                        return sal_False;
                    // <--
                }
            }

            // aber: es wird niemals einem hierarchisch untergeordnetem
            // ausgewichen und ausserdem braucht nur bei Ueberlappung
            // ausgewichen werden.
            // --> OD 2006-08-15 #i68520#
            bEvade &= ( mpCurrAnchoredObj->GetDrawObj()->GetOrdNum() < pNew->GetOrdNum() );
            // <--
            if( bEvade )
            {
                // --> OD 2006-08-15 #i68520#
                SwRect aTmp( _pAnchoredObj->GetObjRectWithSpaces() );
                if ( !aTmp.IsOver( mpCurrAnchoredObj->GetObjRectWithSpaces() ) )
                    bEvade = sal_False;
                // <--
            }
        }

        if ( bEvade )
        {
            // --> OD 2004-10-06 #i26945#
            const SwFmtAnchor& rNewA = _pAnchoredObj->GetFrmFmt().GetAnchor();
            // <--
            OSL_ENSURE( FLY_AS_CHAR != rNewA.GetAnchorId(),
                    "Don't call GetTop with a FlyInCntFrm" );
            if (FLY_AT_PAGE == rNewA.GetAnchorId())
                return sal_True;  // Seitengebundenen wird immer ausgewichen.

            // Wenn absatzgebundene Flys in einem FlyCnt gefangen sind, so
            // endet deren Einflussbereich an den Grenzen des FlyCnt!
            // Wenn wir aber gerade den Text des FlyCnt formatieren, dann
            // muss er natuerlich dem absatzgebundenen Frm ausweichen!
            // pCurrFrm ist der Anker von pNew?
            // --> OD 2004-10-06 #i26945#
            const SwFrm* pTmp = _pAnchoredObj->GetAnchorFrm();
            // <--
            if( pTmp == pCurrFrm )
                return sal_True;
            if( pTmp->IsTxtFrm() && ( pTmp->IsInFly() || pTmp->IsInFtn() ) )
            {
                // --> OD 2004-10-06 #i26945#
                Point aPos = _pAnchoredObj->GetObjRect().Pos();
                // <--
                pTmp = GetVirtualUpper( pTmp, aPos );
            }
            // --> OD 2004-10-06 #i26945#
            // --> OD 2004-11-29 #115759#
            // If <pTmp> is a text frame inside a table, take the upper
            // of the anchor frame, which contains the anchor position.
            else if ( pTmp->IsTxtFrm() && pTmp->IsInTab() )
            {
                pTmp = const_cast<SwAnchoredObject*>(_pAnchoredObj)
                                ->GetAnchorFrmContainingAnchPos()->GetUpper();
            }
            // <--
            // --> OD 2004-05-13 #i28701# - consider all objects in same context,
            // if wrapping style is considered on object positioning.
            // Thus, text will wrap around negative positioned objects.
            // --> OD 2004-08-25 #i3317# - remove condition on checking,
            // if wrappings style is considered on object postioning.
            // Thus, text is wrapping around negative positioned objects.
            // --> OD 2004-10-20 #i35640# - no consideration of negative
            // positioned objects, if wrapping style isn't considered on
            // object position and former text wrapping is applied.
            // This condition is typically for documents imported from the
            // OpenOffice.org file format.
            const IDocumentSettingAccess* pIDSA = pCurrFrm->GetTxtNode()->getIDocumentSettingAccess();
            if ( (  pIDSA->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) ||
                   !pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) ) &&
                 ::FindKontext( pTmp, 0 ) == ::FindKontext( pCurrFrm, 0 ) )
            {
                return sal_True;
            }
            // <--

            const SwFrm* pHeader = 0;
            if ( pCurrFrm->GetNext() != pTmp &&
                 ( IsFrmInSameKontext( pTmp, pCurrFrm ) ||
                   // --> #i13832#, #i24135# wrap around objects in page header
                   ( !pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) &&
                     0 != ( pHeader = pTmp->FindFooterOrHeader() ) &&
                     !pHeader->IsFooterFrm() &&
                     pCurrFrm->IsInDocBody() ) ) )
                   // <--
            {
                if( pHeader || FLY_AT_FLY == rNewA.GetAnchorId() )
                    return sal_True;

                // Compare indices:
                // Den Index des anderen erhalten wir immer ueber das Ankerattr.
                sal_uLong nTmpIndex = rNewA.GetCntntAnchor()->nNode.GetIndex();
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
    return sal_False;
}
// --> OD 2006-08-15 #i68520#
struct AnchoredObjOrder
{
    sal_Bool mbR2L;
    SwRectFn mfnRect;

    AnchoredObjOrder( const sal_Bool bR2L,
                       SwRectFn fnRect )
        : mbR2L( bR2L ),
          mfnRect( fnRect )
    {}

    bool operator()( const SwAnchoredObject* pListedAnchoredObj,
                     const SwAnchoredObject* pNewAnchoredObj )
    {
        const SwRect aBoundRectOfListedObj( pListedAnchoredObj->GetObjRectWithSpaces() );
        const SwRect aBoundRectOfNewObj( pNewAnchoredObj->GetObjRectWithSpaces() );
        if ( ( mbR2L &&
               ( (aBoundRectOfListedObj.*mfnRect->fnGetRight)() ==
                 (aBoundRectOfNewObj.*mfnRect->fnGetRight)() ) ) ||
             ( !mbR2L &&
               ( (aBoundRectOfListedObj.*mfnRect->fnGetLeft)() ==
                 (aBoundRectOfNewObj.*mfnRect->fnGetLeft)() ) ) )
        {
            SwTwips nTopDiff =
                (*mfnRect->fnYDiff)( (aBoundRectOfNewObj.*mfnRect->fnGetTop)(),
                                    (aBoundRectOfListedObj.*mfnRect->fnGetTop)() );
            if ( nTopDiff == 0 &&
                 ( ( mbR2L &&
                     ( (aBoundRectOfNewObj.*mfnRect->fnGetLeft)() >
                       (aBoundRectOfListedObj.*mfnRect->fnGetLeft)() ) ) ||
                   ( !mbR2L &&
                     ( (aBoundRectOfNewObj.*mfnRect->fnGetRight)() <
                       (aBoundRectOfListedObj.*mfnRect->fnGetRight)() ) ) ) )
            {
                return true;
            }
            else if ( nTopDiff > 0 )
            {
                return true;
            }
        }
        else if ( ( mbR2L &&
                    ( (aBoundRectOfListedObj.*mfnRect->fnGetRight)() >
                      (aBoundRectOfNewObj.*mfnRect->fnGetRight)() ) ) ||
                  ( !mbR2L &&
                    ( (aBoundRectOfListedObj.*mfnRect->fnGetLeft)() <
                      (aBoundRectOfNewObj.*mfnRect->fnGetLeft)() ) ) )
        {
            return true;
        }

        return false;
    }
};

// --> OD 2006-08-15 #i68520#
SwAnchoredObjList* SwTxtFly::InitAnchoredObjList()
{
    OSL_ENSURE( pCurrFrm, "InitFlyList: No Frame, no FlyList" );
    // --> OD 2006-08-15 #i68520#
    OSL_ENSURE( !mpAnchoredObjList, "InitFlyList: FlyList already initialized" );
    // <--

    SWAP_IF_SWAPPED( pCurrFrm )

    const SwSortedObjs *pSorted = pPage->GetSortedObjs();
    const sal_uInt32 nCount = pSorted ? pSorted->Count() : 0;
    // --> #108724# Page header/footer content doesn't have to wrap around
    //              floating screen objects
    const bool bFooterHeader = 0 != pCurrFrm->FindFooterOrHeader();
    const IDocumentSettingAccess* pIDSA = pCurrFrm->GetTxtNode()->getIDocumentSettingAccess();
    // --> OD 2005-01-12 #i40155# - check, if frame is marked not to wrap
    const sal_Bool bWrapAllowed = ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) ||
                                    ( !pCurrFrm->IsInFtn() && !bFooterHeader ) ) &&
                                      !SwLayouter::FrmNotToWrap( *pCurrFrm->GetTxtNode()->getIDocumentLayoutAccess(), *pCurrFrm );
    // <--

    bOn = sal_False;

    if( nCount && bWrapAllowed )
    {
        // --> OD 2006-08-15 #i68520#
        mpAnchoredObjList = new SwAnchoredObjList();
        // <--

        // --> OD 2004-06-18 #i28701# - consider complete frame area for new
        // text wrapping
        SwRect aRect;
        if ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) )
        {
            aRect = pCurrFrm->Prt();
            aRect += pCurrFrm->Frm().Pos();
        }
        else
        {
            aRect = pCurrFrm->Frm();
        }
        // Wir machen uns etwas kleiner als wir sind,
        // damit Ein-Twip-Ueberlappungen ignoriert werden. (#49532)
        SWRECTFN( pCurrFrm )
        const long nRight = (aRect.*fnRect->fnGetRight)() - 1;
        const long nLeft = (aRect.*fnRect->fnGetLeft)() + 1;
        const sal_Bool bR2L = pCurrFrm->IsRightToLeft();

        const IDocumentDrawModelAccess* pIDDMA = pCurrFrm->GetTxtNode()->getIDocumentDrawModelAccess();

        for( sal_uInt32 i = 0; i < nCount; i++ )
        {
            // --> OD 2006-08-15 #i68520#
//            SwAnchoredObject* pAnchoredObj = (*pSorted)[ i ];
//            const SwRect aBound( pAnchoredObj->GetObjRectWithSpaces() );

//            // OD 2004-01-15 #110582# - do not consider hidden objects
//            // OD 2004-05-13 #i28701# - check, if object has to be considered
//            // for text wrap.
//            if ( !pDoc->IsVisibleLayerId( pAnchoredObj->GetDrawObj()->GetLayer() ) ||
//                 !pAnchoredObj->ConsiderForTextWrap() ||
//                 nRight < (aBound.*fnRect->fnGetLeft)() ||
//                 (*fnRect->fnYDiff)( (aRect.*fnRect->fnGetTop)(),
//                                     (aBound.*fnRect->fnGetBottom)() ) > 0 ||
//                 nLeft > (aBound.*fnRect->fnGetRight)() ||
//                 // --> OD 2004-12-17 #118809# - If requested, do not consider
//                 // objects in page header|footer for text frames not in page
//                 // header|footer. This is requested for the calculation of
//                 // the base offset for objects <SwTxtFrm::CalcBaseOfstForFly()>
//                 ( mbIgnoreObjsInHeaderFooter && !bFooterHeader &&
//                   pAnchoredObj->GetAnchorFrm()->FindFooterOrHeader() ) ||
//                 // <--
//                 // --> FME 2004-07-14 #i20505# Do not consider oversized objects
//                 (aBound.*fnRect->fnGetHeight)() >
//                 2 * (pPage->Frm().*fnRect->fnGetHeight)() )
//                 // <--
//            {
//              continue;
//            }
            SwAnchoredObject* pAnchoredObj = (*pSorted)[ i ];
            if ( !pIDDMA->IsVisibleLayerId( pAnchoredObj->GetDrawObj()->GetLayer() ) ||
                 !pAnchoredObj->ConsiderForTextWrap() ||
                 ( mbIgnoreObjsInHeaderFooter && !bFooterHeader &&
                   pAnchoredObj->GetAnchorFrm()->FindFooterOrHeader() ) )
            {
                continue;
            }

            const SwRect aBound( pAnchoredObj->GetObjRectWithSpaces() );
            if ( nRight < (aBound.*fnRect->fnGetLeft)() ||
                 (*fnRect->fnYDiff)( (aRect.*fnRect->fnGetTop)(),
                                     (aBound.*fnRect->fnGetBottom)() ) > 0 ||
                 nLeft > (aBound.*fnRect->fnGetRight)() ||
                 (aBound.*fnRect->fnGetHeight)() >
                                    2 * (pPage->Frm().*fnRect->fnGetHeight)() )
            {
                continue;
            }
            // <--

            // --> OD 2004-10-06 #i26945# - pass <pAnchoredObj> to method
            // <GetTop(..)> instead of only the <SdrObject> instance of the
            // anchored object
            if ( GetTop( pAnchoredObj, pCurrFrm->IsInFtn(), bFooterHeader ) )
            // <--
            {
                // OD 11.03.2003 #107862# - adjust insert position:
                // overlapping objects should be sorted from left to right and
                // inside left to right sorting from top to bottom.
                // If objects on the same position are found, they are sorted
                // on its width.
                // --> OD 2006-08-15 #i68520#
//                sal_uInt16 nPos = pFlyList->Count();
//                while ( nPos )
//                {
//                    SdrObject* pTmpObj = (*pFlyList)[ --nPos ];
//                    const SwRect aBoundRectOfTmpObj( GetBoundRect( pTmpObj ) );
//                    if ( ( bR2L &&
//                           ( (aBoundRectOfTmpObj.*fnRect->fnGetRight)() ==
//                             (aBound.*fnRect->fnGetRight)() ) ) ||
//                         ( !bR2L &&
//                           ( (aBoundRectOfTmpObj.*fnRect->fnGetLeft)() ==
//                             (aBound.*fnRect->fnGetLeft)() ) ) )
//                    {
//                        SwTwips nTopDiff =
//                            (*fnRect->fnYDiff)( (aBound.*fnRect->fnGetTop)(),
//                                                (aBoundRectOfTmpObj.*fnRect->fnGetTop)() );
//                        if ( nTopDiff == 0 &&
//                             ( ( bR2L &&
//                                 ( (aBound.*fnRect->fnGetLeft)() >
//                                   (aBoundRectOfTmpObj.*fnRect->fnGetLeft)() ) ) ||
//                               ( !bR2L &&
//                                 ( (aBound.*fnRect->fnGetRight)() <
//                                   (aBoundRectOfTmpObj.*fnRect->fnGetRight)() ) ) ) )
//                        {
//                            ++nPos;
//                            break;
//                        }
//                        else if ( nTopDiff > 0 )
//                        {
//                            ++nPos;
//                            break;
//                        }
//                    }
//                    else if ( ( bR2L &&
//                                ( (aBoundRectOfTmpObj.*fnRect->fnGetRight)() >
//                                  (aBound.*fnRect->fnGetRight)() ) ) ||
//                              ( !bR2L &&
//                                ( (aBoundRectOfTmpObj.*fnRect->fnGetLeft)() <
//                                  (aBound.*fnRect->fnGetLeft)() ) ) )
//                    {
//                        ++nPos;
//                        break;
//                    }
//                }
//                SdrObject* pSdrObj = pAnchoredObj->DrawObj();
//                pFlyList->C40_INSERT( SdrObject, pSdrObj, nPos );
                {
                    SwAnchoredObjList::iterator aInsPosIter =
                            std::lower_bound( mpAnchoredObjList->begin(),
                                              mpAnchoredObjList->end(),
                                              pAnchoredObj,
                                              AnchoredObjOrder( bR2L, fnRect ) );

                    mpAnchoredObjList->insert( aInsPosIter, pAnchoredObj );
                }
                // <--

                const SwFmtSurround &rFlyFmt = pAnchoredObj->GetFrmFmt().GetSurround();
                // --> OD 2006-08-15 #i68520#
                if ( rFlyFmt.IsAnchorOnly() &&
                     pAnchoredObj->GetAnchorFrm() == GetMaster() )
                // <--
                {
                    const SwFmtVertOrient &rTmpFmt =
                                    pAnchoredObj->GetFrmFmt().GetVertOrient();
                    if( text::VertOrientation::BOTTOM != rTmpFmt.GetVertOrient() )
                        nMinBottom = ( bVert && nMinBottom ) ?
                                     Min( nMinBottom, aBound.Left() ) :
                                     Max( nMinBottom, (aBound.*fnRect->fnGetBottom)() );
                }

                bOn = sal_True;
            }
        }
        if( nMinBottom )
        {
            SwTwips nMax = (pCurrFrm->GetUpper()->*fnRect->fnGetPrtBottom)();
            if( (*fnRect->fnYDiff)( nMinBottom, nMax ) > 0 )
                nMinBottom = nMax;
        }
    }
    else
    {
        // --> OD 2006-08-15 #i68520#
        mpAnchoredObjList = new SwAnchoredObjList();
        // <--
    }

    UNDO_SWAP( pCurrFrm )

    // --> OD 2006-08-15 #i68520#
    return mpAnchoredObjList;
    // <--
}
// <--

SwTwips SwTxtFly::CalcMinBottom() const
{
    SwTwips nRet = 0;
    const SwSortedObjs *pDrawObj = GetMaster()->GetDrawObjs();
    const sal_uInt32 nCount = pDrawObj ? pDrawObj->Count() : 0;
    if( nCount )
    {
        SwTwips nEndOfFrm = pCurrFrm->Frm().Bottom();
        for( sal_uInt32 i = 0; i < nCount; i++ )
        {
            SwAnchoredObject* pAnchoredObj = (*pDrawObj)[ i ];
            const SwFmtSurround &rFlyFmt = pAnchoredObj->GetFrmFmt().GetSurround();
            if( rFlyFmt.IsAnchorOnly() )
            {
                const SwFmtVertOrient &rTmpFmt =
                                    pAnchoredObj->GetFrmFmt().GetVertOrient();
                if( text::VertOrientation::BOTTOM != rTmpFmt.GetVertOrient() )
                {
                    const SwRect aBound( pAnchoredObj->GetObjRectWithSpaces() );
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
    nPntCnt( 0 ), nObjCnt( 0 )
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
    OSL_ENSURE( pTextRanger[ nPos ], "ClrObject: Allready cleared. Good Bye!" );
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
// --> OD 2006-08-15 #i68520#
const SwRect SwContourCache::CalcBoundRect( const SwAnchoredObject* pAnchoredObj,
                                            const SwRect &rLine,
                                            const SwTxtFrm* pFrm,
                                            const long nXPos,
                                            const sal_Bool bRight )
{
    SwRect aRet;
    const SwFrmFmt* pFmt = &(pAnchoredObj->GetFrmFmt());
    if( pFmt->GetSurround().IsContour() &&
        ( !pAnchoredObj->ISA(SwFlyFrm) ||
          ( static_cast<const SwFlyFrm*>(pAnchoredObj)->Lower() &&
            static_cast<const SwFlyFrm*>(pAnchoredObj)->Lower()->IsNoTxtFrm() ) ) )
    {
        aRet = pAnchoredObj->GetObjRectWithSpaces();
        if( aRet.IsOver( rLine ) )
        {
            if( !pContourCache )
                pContourCache = new SwContourCache;

            aRet = pContourCache->ContourRect(
                    pFmt, pAnchoredObj->GetDrawObj(), pFrm, rLine, nXPos, bRight );
        }
        else
            aRet.Width( 0 );
    }
    else
    {
        aRet = pAnchoredObj->GetObjRectWithSpaces();
    }

    return aRet;
}
// <--

const SwRect SwContourCache::ContourRect( const SwFmt* pFmt,
    const SdrObject* pObj, const SwTxtFrm* pFrm, const SwRect &rLine,
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
        ::basegfx::B2DPolyPolygon aPolyPolygon;
        ::basegfx::B2DPolyPolygon* pPolyPolygon = 0L;

        if ( pObj->ISA(SwVirtFlyDrawObj) )
        {
            // Vorsicht #37347: Das GetContour() fuehrt zum Laden der Grafik,
            // diese aendert dadurch ggf. ihre Groesse, ruft deshalb ein
            // ClrObject() auf.
            PolyPolygon aPoly;
            if( !((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetContour( aPoly ) )
                aPoly = PolyPolygon( ((SwVirtFlyDrawObj*)pObj)->
                                     GetFlyFrm()->Frm().SVRect() );
            aPolyPolygon.clear();
            aPolyPolygon.append(aPoly.getB2DPolyPolygon());
        }
        else
        {
            if( !pObj->ISA( E3dObject ) )
            {
                aPolyPolygon = pObj->TakeXorPoly();
            }

            ::basegfx::B2DPolyPolygon aContourPoly(pObj->TakeContour());
            pPolyPolygon = new ::basegfx::B2DPolyPolygon(aContourPoly);
        }
        const SvxLRSpaceItem &rLRSpace = pFmt->GetLRSpace();
        const SvxULSpaceItem &rULSpace = pFmt->GetULSpace();
        memmove( pTextRanger + 1, pTextRanger, nObjCnt * sizeof( TextRanger* ) );
        memmove( (SdrObject**)pSdrObj + 1, pSdrObj, nObjCnt++ * sizeof( SdrObject* ) );
        pSdrObj[ 0 ] = pObj; // Wg. #37347 darf das Object erst nach dem
                             // GetContour() eingetragen werden.
        pTextRanger[ 0 ] = new TextRanger( aPolyPolygon, pPolyPolygon, 20,
            (sal_uInt16)rLRSpace.GetLeft(), (sal_uInt16)rLRSpace.GetRight(),
            pFmt->GetSurround().IsOutside(), sal_False, pFrm->IsVertical() );
        pTextRanger[ 0 ]->SetUpper( rULSpace.GetUpper() );
        pTextRanger[ 0 ]->SetLower( rULSpace.GetLower() );

        delete pPolyPolygon;
        // UPPER_LOWER_TEST
#if OSL_DEBUG_LEVEL > 1
        const ViewShell* pTmpViewShell = pFmt->GetDoc()->GetCurrentViewShell();
        if( pTmpViewShell )
        {
            sal_Bool bT2 = pTmpViewShell->GetViewOptions()->IsTest2();
            sal_Bool bT6 = pTmpViewShell->GetViewOptions()->IsTest6();
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
    SWRECTFN( pFrm )
    long nTmpTop = (rLine.*fnRect->fnGetTop)();
    // fnGetBottom is top + height
    long nTmpBottom = (rLine.*fnRect->fnGetBottom)();

    Range aRange( Min( nTmpTop, nTmpBottom ), Max( nTmpTop, nTmpBottom ) );

    LongDqPtr pTmp = pTextRanger[ 0 ]->GetTextRanges( aRange );

    MSHORT nCount;
    if( 0 != ( nCount = pTmp->size() ) )
    {
        MSHORT nIdx = 0;
        while( nIdx < nCount && (*pTmp)[ nIdx ] < nXPos )
            ++nIdx;
        sal_Bool bOdd = nIdx % 2 ? sal_True : sal_False;
        sal_Bool bSet = sal_True;
        if( bOdd )
            --nIdx; // innerhalb eines Intervalls
        else if( ! bRight && ( nIdx >= nCount || (*pTmp)[ nIdx ] != nXPos ) )
        {
            if( nIdx )
                nIdx -= 2; // ein Intervall nach links gehen
            else
                bSet = sal_False; // vor dem erstem Intervall
        }

        if( bSet && nIdx < nCount )
        {
            (aRet.*fnRect->fnSetTopAndHeight)( (rLine.*fnRect->fnGetTop)(),
                                               (rLine.*fnRect->fnGetHeight)() );
            (aRet.*fnRect->fnSetLeft)( (*pTmp)[ nIdx ] );
            (aRet.*fnRect->fnSetRight)( (*pTmp)[ nIdx + 1 ] + 1 );
        }
    }
    return aRet;
}

/*************************************************************************
 *                      SwContourCache::ShowContour()
 * zeichnet die PolyPolygone des Caches zu Debugzwecken.
 *************************************************************************/
#if OSL_DEBUG_LEVEL > 1

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
    }
}
#endif

/*************************************************************************
 *                      SwTxtFly::ShowContour()
 * zeichnet die PolyPolygone des Caches zu Debugzwecken.
 *************************************************************************/
#if OSL_DEBUG_LEVEL > 1

void SwTxtFly::ShowContour( OutputDevice* pOut )
{
    MSHORT nFlyCount;
    if( bOn && ( 0 != ( nFlyCount = static_cast<sal_uInt16>(GetAnchoredObjList()->size() ) ) ) )
    {
        Color aRedColor( COL_LIGHTRED );
        Color aGreenColor( COL_LIGHTGREEN );
        Color aSaveColor( pOut->GetLineColor() );
        for( MSHORT j = 0; j < nFlyCount; ++j )
        {
            const SwAnchoredObject* pObj = (*mpAnchoredObjList)[ j ];
            if( !pObj->GetFrmFmt().GetSurround().IsContour() )
            {
                Rectangle aRect = pObj->GetObjRectWithSpaces().SVRect();
                pOut->DrawRect( aRect );
                continue;
            }
            pContourCache->ShowContour( pOut, pObj->GetDrawObj(), aRedColor, aGreenColor );
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
    SWAP_IF_SWAPPED( pCurrFrm )

    sal_Bool bRet = sal_False;
    // --> OD 2006-08-15 #i68520#
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    // <--
    {
        for( SwAnchoredObjList::size_type i = 0; i < nCount; ++i )
        {
            // --> OD 2006-08-15 #i68520#
            const SwAnchoredObject* pAnchoredObj = (*mpAnchoredObjList)[i];

            SwRect aRect( pAnchoredObj->GetObjRectWithSpaces() );
            // <--

            // Optimierung
            SWRECTFN( pCurrFrm )
            if( (aRect.*fnRect->fnGetLeft)() > (rRect.*fnRect->fnGetRight)() )
                break;
            // --> OD 2006-08-15 #i68520#
            if ( mpCurrAnchoredObj != pAnchoredObj && aRect.IsOver( rRect ) )
            // <--
            {
                // --> OD 2006-08-15 #i68520#
                const SwFmt* pFmt( &(pAnchoredObj->GetFrmFmt()) );
                const SwFmtSurround &rSur = pFmt->GetSurround();
                // <--
                if( bAvoid )
                {
                    // Wenn der Text drunter durchlaeuft, bleibt die
                    // Formatierung unbeeinflusst. Im LineIter::DrawText()
                    // muessen "nur" geschickt die ClippingRegions gesetzt werden ...
                    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                    if( ( SURROUND_THROUGHT == rSur.GetSurround() &&
                          ( !rSur.IsAnchorOnly() ||
                            // --> OD 2006-08-15 #i68520#
                            GetMaster() == pAnchoredObj->GetAnchorFrm() ||
                            // <--
                            ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                             (FLY_AT_CHAR != rAnchor.GetAnchorId())) ) )
                        || aRect.Top() == WEIT_WECH )
                        continue;
                }

                // --> OD 2006-01-20 #i58642#
                // Compare <GetMaster()> instead of <pCurrFrm> with the anchor
                // frame of the anchored object, because a follow frame have
                // to ignore the anchored objects of its master frame.
                // Note: Anchored objects are always registered at the master
                //       frame, exception are as-character anchored objects,
                //       but these aren't handled here.
                // --> OD 2006-08-15 #i68520#
                if ( mbIgnoreCurrentFrame &&
                     GetMaster() == pAnchoredObj->GetAnchorFrm() )
                    continue;
                // <--

                if( pRect )
                {
                    // --> OD 2006-08-15 #i68520#
                    SwRect aFly = AnchoredObjToRect( pAnchoredObj, rRect );
                    // <--
                    if( aFly.IsEmpty() || !aFly.IsOver( rRect ) )
                        continue;
                    if( !bRet || (
                        ( !pCurrFrm->IsRightToLeft() &&
                          ( (aFly.*fnRect->fnGetLeft)() <
                            (pRect->*fnRect->fnGetLeft)() ) ) ||
                        ( pCurrFrm->IsRightToLeft() &&
                          ( (aFly.*fnRect->fnGetRight)() >
                            (pRect->*fnRect->fnGetRight)() ) ) ) )
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

    UNDO_SWAP( pCurrFrm )

    return bRet;
}

/*************************************************************************
 *                      SwTxtFly::GetPos()
 *
 * liefert die Position im sorted Array zurueck
 *************************************************************************/

// --> OD 2006-08-15 #i68520#
SwAnchoredObjList::size_type SwTxtFly::GetPos( const SwAnchoredObject* pAnchoredObj ) const
{
    SwAnchoredObjList::size_type nCount = GetAnchoredObjList()->size();
    SwAnchoredObjList::size_type nRet = 0;
    while ( nRet < nCount && pAnchoredObj != (*mpAnchoredObjList)[ nRet ] )
        ++nRet;
    return nRet;
}
// <--

/*************************************************************************
 *                      SwTxtFly::CalcRightMargin()
 *
 * pObj ist das Object, der uns gerade ueberlappt.
 * pCurrFrm ist der aktuelle Textframe, der ueberlappt wird.
 * Der rechte Rand ist der rechte Rand oder
 * er wird durch das naechste Object, welches in die Zeile ragt, bestimmt.
 *************************************************************************/
// --> OD 2006-08-15 #i68520#
void SwTxtFly::CalcRightMargin( SwRect &rFly,
                                SwAnchoredObjList::size_type nFlyPos,
                                const SwRect &rLine ) const
{
    // Normalerweise ist der rechte Rand der rechte Rand der Printarea.
    OSL_ENSURE( ! pCurrFrm->IsVertical() || ! pCurrFrm->IsSwapped(),
            "SwTxtFly::CalcRightMargin with swapped frame" );
    SWRECTFN( pCurrFrm )
    // --> OD 2004-12-14 #118796# - correct determination of right of printing area
    SwTwips nRight = (pCurrFrm->*fnRect->fnGetPrtRight)();
    // <--
    SwTwips nFlyRight = (rFly.*fnRect->fnGetRight)();
    SwRect aLine( rLine );
    (aLine.*fnRect->fnSetRight)( nRight );
    (aLine.*fnRect->fnSetLeft)( (rFly.*fnRect->fnGetLeft)() );

    // Es koennte aber sein, dass in die gleiche Zeile noch ein anderes
    // Object hineinragt, welches _ueber_ uns liegt.
    // Wunder der Technik: Flys mit Durchlauf sind fuer die darunterliegenden
    // unsichtbar, das heisst, dass sie bei der Berechnung der Raender
    // anderer Flys ebenfalls nicht auffallen.
    // 3301: pNext->Frm().IsOver( rLine ) ist noetig
    // --> OD 2006-08-15 #i68520#
    SwSurround eSurroundForTextWrap;
    // <--

    sal_Bool bStop = sal_False;
    // --> OD 2006-08-15 #i68520#
    SwAnchoredObjList::size_type nPos = 0;
    // <--

    // --> OD 2006-08-15 #i68520#
    while( nPos < mpAnchoredObjList->size() && !bStop )
    // <--
    {
        if( nPos == nFlyPos )
        {
            ++nPos;
            continue;
        }
        // --> OD 2006-08-15 #i68520#
        const SwAnchoredObject* pNext = (*mpAnchoredObjList)[ nPos++ ];
        if ( pNext == mpCurrAnchoredObj )
            continue;
        eSurroundForTextWrap = _GetSurroundForTextWrap( pNext );
        if( SURROUND_THROUGHT == eSurroundForTextWrap )
            continue;
        // <--

        const SwRect aTmp( SwContourCache::CalcBoundRect
                ( pNext, aLine, pCurrFrm, nFlyRight, sal_True ) );
        SwTwips nTmpRight = (aTmp.*fnRect->fnGetRight)();

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
        const long nTmpTop = (aTmp.*fnRect->fnGetTop)();
        if( (*fnRect->fnYDiff)( nTmpTop, (aLine.*fnRect->fnGetTop)() ) > 0 )
        {
            if( (*fnRect->fnYDiff)( nNextTop, nTmpTop ) > 0 )
                SetNextTop( nTmpTop ); // Die Oberkante des "naechsten" Rahmens
        }
        else if( ! (aTmp.*fnRect->fnGetWidth)() ) // Typisch fuer Objekte mit Konturumlauf
        {   // Bei Objekten mit Konturumlauf, die vor der aktuellen Zeile beginnen
            // und hinter ihr enden, trotzdem aber nicht mit ihr ueberlappen,
            // muss die Optimierung ausgeschaltet werden, denn bereits in der
            // naechsten Zeile kann sich dies aendern.
            if( ! (aTmp.*fnRect->fnGetHeight)() ||
                (*fnRect->fnYDiff)( (aTmp.*fnRect->fnGetBottom)(),
                                    (aLine.*fnRect->fnGetTop)() ) > 0 )
                SetNextTop( 0 );
        }
        if( aTmp.IsOver( aLine ) && nTmpRight > nFlyRight )
        {
            nFlyRight = nTmpRight;
            if( SURROUND_RIGHT == eSurroundForTextWrap ||
                SURROUND_PARALLEL == eSurroundForTextWrap )
            {
                // der FlyFrm wird ueberstimmt.
                if( nRight > nFlyRight )
                    nRight = nFlyRight;
                bStop = sal_True;
            }
        }
    }
    (rFly.*fnRect->fnSetRight)( nRight );
}
// <--

/*************************************************************************
 *                      SwTxtFly::CalcLeftMargin()
 *
 * pFly ist der FlyFrm, der uns gerade ueberlappt.
 * pCurrFrm ist der aktuelle Textframe, der ueberlappt wird.
 * Der linke Rand ist der linke Rand der aktuellen PrintArea oder
 * er wird durch den vorigen FlyFrm, der in die Zeile ragt, bestimmt.
 *************************************************************************/
// --> OD 2006-08-15 #i68520#
void SwTxtFly::CalcLeftMargin( SwRect &rFly,
                               SwAnchoredObjList::size_type nFlyPos,
                               const SwRect &rLine ) const
{
    OSL_ENSURE( ! pCurrFrm->IsVertical() || ! pCurrFrm->IsSwapped(),
            "SwTxtFly::CalcLeftMargin with swapped frame" );
    SWRECTFN( pCurrFrm )
    // --> OD 2004-12-14 #118796# - correct determination of left of printing area
    SwTwips nLeft = (pCurrFrm->*fnRect->fnGetPrtLeft)();
    // <--
    const SwTwips nFlyLeft = (rFly.*fnRect->fnGetLeft)();

    if( nLeft > nFlyLeft )
        nLeft = rFly.Left();

    SwRect aLine( rLine );
    (aLine.*fnRect->fnSetLeft)( nLeft );

    // Es koennte aber sein, dass in die gleiche Zeile noch ein anderes
    // Object hineinragt, welches _ueber_ uns liegt.
    // Wunder der Technik: Flys mit Durchlauf sind fuer die darunterliegenden
    // unsichtbar, das heisst, dass sie bei der Berechnung der Raender
    // anderer Flys ebenfalls nicht auffallen.
    // 3301: pNext->Frm().IsOver( rLine ) ist noetig

    // --> OD 2006-08-15 #i68520#
    SwAnchoredObjList::size_type nMyPos = nFlyPos;
    while( ++nFlyPos < mpAnchoredObjList->size() )
    // <--
    {
        // --> OD 2006-08-15 #i68520#
        const SwAnchoredObject* pNext = (*mpAnchoredObjList)[ nFlyPos ];
        const SwRect aTmp( pNext->GetObjRectWithSpaces() );
        // <--
        if( (aTmp.*fnRect->fnGetLeft)() >= nFlyLeft )
            break;
    }

    while( nFlyPos )
    {
        if( --nFlyPos == nMyPos )
            continue;
        // --> OD 2006-08-15 #i68520#
        const SwAnchoredObject* pNext = (*mpAnchoredObjList)[ nFlyPos ];
        if( pNext == mpCurrAnchoredObj )
            continue;
        SwSurround eSurroundForTextWrap = _GetSurroundForTextWrap( pNext );
        if( SURROUND_THROUGHT == eSurroundForTextWrap )
            continue;
        // <--

        const SwRect aTmp( SwContourCache::CalcBoundRect
                ( pNext, aLine, pCurrFrm, nFlyLeft, sal_False ) );

        if( (aTmp.*fnRect->fnGetLeft)() < nFlyLeft && aTmp.IsOver( aLine ) )
        {
            // --> OD 2004-12-14 #118796# - no '+1', because <..fnGetRight>
            // returns the correct value.
            SwTwips nTmpRight = (aTmp.*fnRect->fnGetRight)();
            if ( nLeft <= nTmpRight )
                nLeft = nTmpRight;
            // <--

            break;
        }
    }
    (rFly.*fnRect->fnSetLeft)( nLeft );
}
// <--

/*************************************************************************
 *                      SwTxtFly::FlyToRect()
 *
 * IN:  dokumentglobal  (rRect)
 * OUT: dokumentglobal  (return-Wert)
 * Liefert zu einem SwFlyFrm das von ihm in Anspruch genommene Rechteck
 * unter Beruecksichtigung der eingestellten Attribute fuer den Abstand
 * zum Text zurueck.
 *************************************************************************/
// --> OD 2006-08-15 #i68520#
SwRect SwTxtFly::AnchoredObjToRect( const SwAnchoredObject* pAnchoredObj,
                            const SwRect &rLine ) const
{
    SWRECTFN( pCurrFrm )

    const long nXPos = pCurrFrm->IsRightToLeft() ?
                       rLine.Right() :
                       (rLine.*fnRect->fnGetLeft)();

    SwRect aFly = mbIgnoreContour ?
                  pAnchoredObj->GetObjRectWithSpaces() :
                  SwContourCache::CalcBoundRect( pAnchoredObj, rLine, pCurrFrm,
                                                 nXPos, ! pCurrFrm->IsRightToLeft() );

    if( !aFly.Width() )
        return aFly;

    SetNextTop( (aFly.*fnRect->fnGetBottom)() ); // Damit die Zeile ggf. bis zur Unterkante
                                 // des Rahmens waechst.
    SwAnchoredObjList::size_type nFlyPos = GetPos( pAnchoredObj );

    // Bei LEFT und RIGHT vergroessern wir das Rechteck.
    // Hier gibt es einige Probleme, wenn mehrere Frames zu sehen sind.
    // Zur Zeit wird nur der einfachste Fall angenommen:
    // LEFT bedeutet, dass der Text links vom Frame fliessen soll,
    // d.h. der Frame blaeht sich bis zum rechten Rand der Printarea
    // oder bis zum naechsten Frame auf.
    // Bei RIGHT ist es umgekehrt.
    // Ansonsten wird immer der eingestellte Abstand zwischen Text
    // und Frame aufaddiert.
    switch( _GetSurroundForTextWrap( pAnchoredObj ) )
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
        default:
            break;
    }
    return aFly;
}

// --> OD 2006-08-15 #i68520#
// new method <_GetSurroundForTextWrap(..)> replaces methods
// <CalcSmart(..)> and <GetOrder(..)>
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

SwSurround SwTxtFly::_GetSurroundForTextWrap( const SwAnchoredObject* pAnchoredObj ) const
{
    const SwFrmFmt* pFmt = &(pAnchoredObj->GetFrmFmt());
    const SwFmtSurround &rFlyFmt = pFmt->GetSurround();
    SwSurround eSurroundForTextWrap = rFlyFmt.GetSurround();

    if( rFlyFmt.IsAnchorOnly() && pAnchoredObj->GetAnchorFrm() != GetMaster() )
    {
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        if ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
            (FLY_AT_CHAR == rAnchor.GetAnchorId()))
        {
            return SURROUND_NONE;
        }
    }

    // Beim Durchlauf und Nowrap wird smart ignoriert.
    if( SURROUND_THROUGHT == eSurroundForTextWrap ||
        SURROUND_NONE == eSurroundForTextWrap )
        return eSurroundForTextWrap;

    // left is left and right is right
    if ( pCurrFrm->IsRightToLeft() )
    {
        if ( SURROUND_LEFT == eSurroundForTextWrap )
            eSurroundForTextWrap = SURROUND_RIGHT;
        else if ( SURROUND_RIGHT == eSurroundForTextWrap )
            eSurroundForTextWrap = SURROUND_LEFT;
    }

    // "idealer Seitenumlauf":
    if ( SURROUND_IDEAL == eSurroundForTextWrap )
    {
        SWRECTFN( pCurrFrm )
        const long nCurrLeft = (pCurrFrm->*fnRect->fnGetPrtLeft)();
        const long nCurrRight = (pCurrFrm->*fnRect->fnGetPrtRight)();
        const SwRect aRect( pAnchoredObj->GetObjRectWithSpaces() );
        long nFlyLeft = (aRect.*fnRect->fnGetLeft)();
        long nFlyRight = (aRect.*fnRect->fnGetRight)();

        if ( nFlyRight < nCurrLeft || nFlyLeft > nCurrRight )
            eSurroundForTextWrap = SURROUND_PARALLEL;
        else
        {
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
                eSurroundForTextWrap = nRight ? SURROUND_PARALLEL : SURROUND_LEFT;
            else
                eSurroundForTextWrap = nRight ? SURROUND_RIGHT: SURROUND_NONE;
        }
    }

    return eSurroundForTextWrap;
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

    SWAP_IF_SWAPPED( pCurrFrm )

    OSL_ENSURE( bOn, "IsAnyFrm: Why?" );

    const sal_Bool bRet = ForEach( rLine, NULL, sal_False );
    UNDO_SWAP( pCurrFrm )
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
