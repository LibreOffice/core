/*************************************************************************
 *
 *  $RCSfile: txtfly.cxx,v $
 *
 *  $Revision: 1.47 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:15:54 $
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
#ifndef _PORMULTI_HXX
#include <pormulti.hxx>     // SwMultiPortion
#endif

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
// --> OD 2004-06-16 #i28701#
#ifndef _SVX_LSPCITEM_HXX
#include <svx/lspcitem.hxx>
#endif
// <--
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

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> // SwPageDesc
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
#endif

// #102344#
#ifndef _SVDOEDGE_HXX
#include <svx/svdoedge.hxx>
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
#include "fmtcnct.hxx"  // SwFmtChain
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

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

// OD 03.07.2003 #108784# - change return type from <pointer> to <reference>
const SwFrm& lcl_TheAnchor( const SdrObject* pObj )
{
    // OD 2004-03-29 #i26791#
    SwContact* pContact = static_cast<SwContact*>(GetUserCall( pObj ));
    const SwFrm* pRet = pContact->GetAnchoredObj( pObj )->GetAnchorFrm();
    ASSERT( pRet, "<lcl_TheAnchor(..)> - no anchor frame found!" );

    return *pRet;
}

void SwTxtFormatter::CalcUnclipped( SwTwips& rTop, SwTwips& rBottom )
{
    ASSERT( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::CalcUnclipped with unswapped frame" )

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

void SwTxtFormatter::UpdatePos( SwLineLayout *pCurr, Point aStart,
    xub_StrLen nStartIdx, sal_Bool bAllWays ) const
{
    ASSERT( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::UpdatePos with unswapped frame" )

    if( GetInfo().IsTest() )
        return;
    SwLinePortion *pFirst = pCurr->GetFirstPortion();
    SwLinePortion *pPos = pFirst;
    SwTxtPaintInfo aTmpInf( GetInfo() );
    aTmpInf.SetSpaceAdd( pCurr->GetpSpaceAdd() );
    aTmpInf.ResetSpaceIdx();
    aTmpInf.SetKanaComp( pCurr->GetpKanaComp() );
    aTmpInf.ResetKanaIdx();

    // Die Groesse des Frames
    aTmpInf.SetIdx( nStartIdx );
    aTmpInf.SetPos( aStart );

    long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
    // OD 08.01.2004 #i11859# - use new method <SwLineLayout::MaxAscentDescent(..)>
    //lcl_MaxAscDescent( pPos, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );
    pCurr->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );

    KSHORT nTmpHeight = pCurr->GetRealHeight();
    KSHORT nAscent = pCurr->GetAscent() + nTmpHeight - pCurr->Height();
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
            pCurr->MaxAscentDescent( nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, pPos );

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
            ASSERT( !GetMulti(), "Too much multi" );
            ((SwTxtFormatter*)this)->pMulti = (SwMultiPortion*)pPos;
            SwLineLayout *pLay = &GetMulti()->GetRoot();
            Point aSt( aTmpInf.X(), aStart.Y() );

            if ( GetMulti()->HasBrackets() )
            {
                ASSERT( GetMulti()->IsDouble(), "Brackets only for doubles");
                aSt.X() += ((SwDoubleLinePortion*)GetMulti())->PreWidth();
            }
            else if( GetMulti()->HasRotation() )
            {
                aSt.Y() += pCurr->GetAscent() - GetMulti()->GetAscent();
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
                nStIdx += pLay->GetLen();
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
    ASSERT( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "SwTxtFormatter::AlignFlyInCntBase with unswapped frame" )

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
    ASSERT( rInf.GetTxtFly()->IsOn(), "SwTxtFormatter::ChkFlyUnderflow: why?" );
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

    register const SwLinePortion *pLast = rInf.GetLast();

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
            rInf.ForcedLeftMargin( (USHORT)aInter.Width() );
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
            SWRECTFN( pFrm )
            long nNextTop = pTxtFly->GetNextTop();
            if ( bVert )
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

            const USHORT nGridWidth = pGrid->GetBaseHeight();

            SwTwips nStartX = GetLeftMargin();
            if ( bVert )
            {
                Point aPoint( nStartX, 0 );
                pFrm->SwitchHorizontalToVertical( aPoint );
                nStartX = aPoint.Y();
            }

            const SwTwips nOfst = nStartX - nGridOrigin;
            const SwTwips nTmpWidth = rInf.Width() + nOfst;

            const ULONG i = nTmpWidth / nGridWidth + 1;

            const long nNewWidth = ( i - 1 ) * nGridWidth - nOfst;
            if ( nNewWidth > 0 )
                rInf.Width( (USHORT)nNewWidth );
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
    KSHORT nAscent;
    if ( IsQuick() || !pFly || !pFly->GetValidPosFlag() ||
        ( GetInfo().GetTxtFrm()->IsVertical() ?
          ( ! pFly->GetRefPoint().X() ||
            ( nAscent = Abs( int( pFly->GetRelPos().X() ) ) ) ) :
          ( ! pFly->GetRefPoint().Y() ||
            ( nAscent = Abs( int( pFly->GetRelPos().Y() ) ) ) ) ) )
        nAscent = rInf.GetLast()->GetAscent();
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
    pCurrFly = rTxtFly.pCurrFly;
    pCurrFrm = rTxtFly.pCurrFrm;
    pMaster = rTxtFly.pMaster;
    if( rTxtFly.pFlyList )
    {
        pFlyList = new SwFlyList( (BYTE)rTxtFly.pFlyList->Count(), 10 );
        pFlyList->Insert( rTxtFly.pFlyList, 0 );
    }
    else
        pFlyList = NULL;

    bOn = rTxtFly.bOn;
    bLeftSide = rTxtFly.bLeftSide;
    bTopRule = rTxtFly.bTopRule;
}

void SwTxtFly::CtorInit( const SwTxtFrm *pFrm )
{
    mbIgnoreCurrentFrame = sal_False;
    mbIgnoreContour = sal_False;
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

    ASSERT( bOn, "IsAnyFrm: Why?" );
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
    ASSERT ( bOn, "SwTxtFly::IsAnyObj: Who's knocking?" );

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

            if( pCurrFly != pObj->GetDrawObj() && aBound.IsOver( aRect ) )
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
            if( pTmp->ISA(SwVirtFlyDrawObj) && pCurrFly != pTmp )
            {
                SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pTmp)->GetFlyFrm();
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
                          GetMaster() == &lcl_TheAnchor( pTmp ) ||
                          ( FLY_AT_CNTNT != rAnchor.GetAnchorId() &&
                              FLY_AUTO_CNTNT != rAnchor.GetAnchorId()
                          )
                        ) &&
                        pTmp->GetLayer() != nHellId &&
                        nCurrOrd < pTmp->GetOrdNum()
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
    ASSERT( !bTopRule, "DrawFlyRect: Wrong TopRule" );
    MSHORT nCount;
    if( bOn && ( 0 != ( nCount = GetFlyList()->Count() ) ) )
    {
        MSHORT nHellId = pPage->GetShell()->GetDoc()->GetHellId();
        for( MSHORT i = 0; i < nCount; ++i )
        {
            const SdrObject *pTmp = (*pFlyList)[ i ];
            if( pCurrFly != pTmp && pTmp->ISA(SwVirtFlyDrawObj) )
            {
                const SwFrmFmt *pFmt =
                    ((SwContact*)GetUserCall(pTmp))->GetFmt();
                const SwFmtSurround &rSur = pFmt->GetSurround();

                // OD 24.01.2003 #106593# - correct clipping of fly frame area.
                // Consider that fly frame background/shadow can be transparent
                // and <SwAlignRect(..)> fly frame area
                const SwFlyFrm *pFly = static_cast<const SwVirtFlyDrawObj*>(pTmp)->GetFlyFrm();
                bool bClipFlyArea =
                        ( (SURROUND_THROUGHT == rSur.GetSurround()) ?
                          (pTmp->GetLayer() != nHellId) : !rSur.IsContour() ) &&
                        !pFly->IsBackgroundTransparent() &&
                        !pFly->IsShadowTransparent();
                if ( bClipFlyArea )
                {
                    SwRect aFly( pTmp->GetCurrentBoundRect() );
                    // OD 24.01.2003 #106593#
                    ::SwAlignRect( aFly, pPage->GetShell() );
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

sal_Bool SwTxtFly::GetTop( const SdrObject *pNew, const sal_Bool bInFtn,
                           const sal_Bool bInFooterOrHeader )
{
    // pCurrFly is set, if pCurrFrm is inside a fly frame
    if( pNew != pCurrFly )
    {
        // #102344# Ignore connectors which have one or more connections
        if(pNew && pNew->ISA(SdrEdgeObj))
        {
            if(((SdrEdgeObj*)pNew)->GetConnectedNode(TRUE)
                || ((SdrEdgeObj*)pNew)->GetConnectedNode(FALSE))
            {
                return sal_False;
            }
        }

        if( ( bInFtn || bInFooterOrHeader ) && bTopRule )
        {
            SwFrmFmt *pFmt = ((SwContact*)GetUserCall(pNew))->GetFmt();
            const SwFmtAnchor& rNewA = pFmt->GetAnchor();

            if ( FLY_PAGE == rNewA.GetAnchorId() )
            {
                if ( bInFtn )
                    return sal_False;

                if ( bInFooterOrHeader )
                {
                    SwFmtVertOrient aVert( pFmt->GetVertOrient() );
                    BOOL bVertPrt = aVert.GetRelationOrient() == PRTAREA ||
                            aVert.GetRelationOrient() == REL_PG_PRTAREA;
                    if( bVertPrt )
                        return sal_False;
                }
            }
        }

        // bEvade: consider pNew, if we are not inside a fly
        //         consider pNew, if pNew is lower of pCurrFly
        sal_Bool bEvade = !pCurrFly
                       || Is_Lower_Of(((SwVirtFlyDrawObj*)pCurrFly)->GetFlyFrm(), pNew);

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
                const SwFmtChain &rChain = ((SwContact*)GetUserCall(pCurrFly))->GetFmt()->GetChain();
                if ( !rChain.GetPrev() && !rChain.GetNext() )
                {
                    const SwFmtAnchor& rNewA =
                        ((SwContact*)GetUserCall(pNew))->GetFmt()->GetAnchor();
                    const SwFmtAnchor& rCurrA =
                        ((SwContact*)GetUserCall(pCurrFly))->GetFmt()->GetAnchor();

                    // If pCurrFly is anchored as character, its content
                    // does not wrap around pNew
                    if( FLY_IN_CNTNT == rCurrA.GetAnchorId() )
                        return sal_False;

                    // If pNew is anchored to page and pCurrFly is not anchored
                    // to page, the content of pCurrFly does not wrap around pNew
                    // If both pNew and pCurrFly are anchored to page, we can do
                    // some more checks
                    if( FLY_PAGE == rNewA.GetAnchorId() )
                    {
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
                    else if( bInFooterOrHeader )
                        return sal_False;  // In header or footer no wrapping
                                           // if both bounded at paragraph
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
                if( !aTmp.IsOver( pCurrFly->GetCurrentBoundRect() ) )
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
            const SwFrm* pTmp = &lcl_TheAnchor( pNew );
            if( pTmp == pCurrFrm )
                return sal_True;
            if( pTmp->IsTxtFrm() && ( pTmp->IsInFly() || pTmp->IsInFtn() ) )
            {
                Point aPos;
                if( pNew->ISA(SwVirtFlyDrawObj) )
                    aPos = ( (SwVirtFlyDrawObj*)pNew )->GetFlyFrm()->Frm().Pos();
                else
                    aPos = pNew->GetCurrentBoundRect().TopLeft();
                pTmp = GetVirtualUpper( pTmp, aPos );
            }
            // OD 2004-05-13 #i28701# - consider all objects in same context,
            // if wrapping style is considered on object positioning.
            // Thus, text will wrap around negative positioned objects.
            if ( pCurrFrm->GetTxtNode()->GetDoc()->ConsiderWrapOnObjPos() &&
                 ::FindKontext( pTmp, 0 ) == ::FindKontext( pCurrFrm, 0 ) )
            {
                return sal_True;
            }

            const SwFrm* pHeader = 0;
            if ( pCurrFrm->GetNext() != pTmp &&
                 ( IsFrmInSameKontext( pTmp, pCurrFrm ) ||
                   // --> #i13832#, #i24135# wrap around objects in page header
                   ( !pCurrFrm->GetTxtNode()->GetDoc()->IsFormerTextWrapping() &&
                     0 != ( pHeader = pTmp->FindFooterOrHeader() ) &&
                     !pHeader->IsFooterFrm() &&
                     pCurrFrm->IsInDocBody() ) ) )
                   // <--
            {
                if( pHeader || FLY_AT_FLY == rNewA.GetAnchorId() )
                    return sal_True;

                // Compare indices:
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

    SWAP_IF_SWAPPED( pCurrFrm )

    const SwSortedObjs *pSorted = pPage->GetSortedObjs();
    const sal_uInt32 nCount = pSorted ? pSorted->Count() : 0;
    // --> #108724# Page header/footer content doesn't have to wrap around
    //              floating screen objects
    const bool bFooterHeader = 0 != pCurrFrm->FindFooterOrHeader();
    const SwDoc* pDoc = pCurrFrm->GetTxtNode()->GetDoc();
    const sal_Bool bWrapAllowed = pDoc->IsFormerTextWrapping() ||
                                ( !pCurrFrm->IsInFtn() && !bFooterHeader );
    // <--

    bOn = sal_False;

    if( nCount && bWrapAllowed )
    {
        pFlyList = new SwFlyList( 10, 10 );

        // --> OD 2004-06-18 #i28701# - consider complete frame area for new
        // text wrapping
        SwRect aRect;
        if ( pDoc->IsFormerTextWrapping() )
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

        for( sal_uInt32 i = 0; i < nCount; i++ )
        {
            SwAnchoredObject* pAnchoredObj = (*pSorted)[ i ];
            const SwRect aBound( pAnchoredObj->GetObjRectWithSpaces() );

            // OD 2004-01-15 #110582# - do not consider hidden objects
            // OD 2004-05-13 #i28701# - check, if object has to be considered
            // for text wrap.
            if ( !pDoc->IsVisibleLayerId( pAnchoredObj->GetDrawObj()->GetLayer() ) ||
                 !pAnchoredObj->ConsiderForTextWrap() ||
                 nRight < (aBound.*fnRect->fnGetLeft)() ||
                 (*fnRect->fnYDiff)( (aRect.*fnRect->fnGetTop)(),
                                     (aBound.*fnRect->fnGetBottom)() ) > 0 ||
                 nLeft > (aBound.*fnRect->fnGetRight)() ||
                 // --> FME 2004-07-14 #i20505# Do not consider oversized objects
                 (aBound.*fnRect->fnGetHeight)() >
                 2 * (pPage->Frm().*fnRect->fnGetHeight)() )
                 // <--
            {
                continue;
            }

            if( GetTop( pAnchoredObj->GetDrawObj(), pCurrFrm->IsInFtn(), bFooterHeader ) )
            {
                // OD 11.03.2003 #107862# - adjust insert position:
                // overlapping objects should be sorted from left to right and
                // inside left to right sorting from top to bottom.
                // If objects on the same position are found, they are sorted
                // on its width.
                sal_uInt16 nPos = pFlyList->Count();
                while ( nPos )
                {
                    SdrObject* pTmpObj = (*pFlyList)[ --nPos ];
                    const SwRect aBoundRectOfTmpObj( GetBoundRect( pTmpObj ) );
                    if ( ( bR2L &&
                           ( (aBoundRectOfTmpObj.*fnRect->fnGetRight)() ==
                             (aBound.*fnRect->fnGetRight)() ) ) ||
                         ( !bR2L &&
                           ( (aBoundRectOfTmpObj.*fnRect->fnGetLeft)() ==
                             (aBound.*fnRect->fnGetLeft)() ) ) )
                    {
                        SwTwips nTopDiff =
                            (*fnRect->fnYDiff)( (aBound.*fnRect->fnGetTop)(),
                                                (aBoundRectOfTmpObj.*fnRect->fnGetTop)() );
                        if ( nTopDiff == 0 &&
                             ( ( bR2L &&
                                 ( (aBound.*fnRect->fnGetLeft)() >
                                   (aBoundRectOfTmpObj.*fnRect->fnGetLeft)() ) ) ||
                               ( !bR2L &&
                                 ( (aBound.*fnRect->fnGetRight)() <
                                   (aBoundRectOfTmpObj.*fnRect->fnGetRight)() ) ) ) )
                        {
                            ++nPos;
                            break;
                        }
                        else if ( nTopDiff > 0 )
                        {
                            ++nPos;
                            break;
                        }
                    }
                    else if ( ( bR2L &&
                                ( (aBoundRectOfTmpObj.*fnRect->fnGetRight)() >
                                  (aBound.*fnRect->fnGetRight)() ) ) ||
                              ( !bR2L &&
                                ( (aBoundRectOfTmpObj.*fnRect->fnGetLeft)() <
                                  (aBound.*fnRect->fnGetLeft)() ) ) )
                    {
                        ++nPos;
                        break;
                    }
                }
                SdrObject* pSdrObj = pAnchoredObj->DrawObj();
                pFlyList->C40_INSERT( SdrObject, pSdrObj, nPos );

                const SwFmtSurround &rFlyFmt = pAnchoredObj->GetFrmFmt().GetSurround();
                if( rFlyFmt.IsAnchorOnly() && &lcl_TheAnchor( pSdrObj ) == GetMaster() )
                {
                    const SwFmtVertOrient &rTmpFmt =
                                    pAnchoredObj->GetFrmFmt().GetVertOrient();
                    if( VERT_BOTTOM != rTmpFmt.GetVertOrient() )
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
        pFlyList = new SwFlyList( 0, 10 );

    UNDO_SWAP( pCurrFrm )

    return pFlyList;
}

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
                if( VERT_BOTTOM != rTmpFmt.GetVertOrient() )
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
        const SwRect &rLine, const SwTxtFrm* pFrm, const long nXPos,
        const sal_Bool bRight )
{
    SWRECTFN( pFrm )

    SwRect aRet;
    const SwFmt *pFmt =
        ((SwContact*)GetUserCall(pObj))->GetFmt();
    if( pFmt->GetSurround().IsContour() &&
        ( !pObj->ISA(SwVirtFlyDrawObj) ||
          ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Lower() &&
          ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Lower()->IsNoTxtFrm() ) )
    {
        aRet = GetBoundRect( pObj );
        if( aRet.IsOver( rLine ) )
        {
            if( !pContourCache )
                pContourCache = new SwContourCache;

            aRet = pContourCache->ContourRect(
                    pFmt, pObj, pFrm, rLine, nXPos, bRight );
        }
        else
            aRet.Width( 0 );
    }
    else
    {
        aRet = GetBoundRect( pObj );
    }

    return aRet;
}

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
        XPolyPolygon aXPoly;
        XPolyPolygon *pXPoly = NULL;
        if ( pObj->ISA(SwVirtFlyDrawObj) )
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
            (USHORT)rLRSpace.GetLeft(), (USHORT)rLRSpace.GetRight(),
            pFmt->GetSurround().IsOutside(), sal_False, pFrm->IsVertical() );
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
    SWRECTFN( pFrm )
    long nTmpTop = (rLine.*fnRect->fnGetTop)();
    // fnGetBottom is top + height
    long nTmpBottom = (rLine.*fnRect->fnGetBottom)();

    Range aRange( Min( nTmpTop, nTmpBottom ), Max( nTmpTop, nTmpBottom ) );

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
#if OSL_DEBUG_LEVEL > 1
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
                Rectangle aRect = pObj->GetCurrentBoundRect();
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
    SWAP_IF_SWAPPED( pCurrFrm )

    sal_Bool bRet = sal_False;
    MSHORT nCount;
    if( bOn && ( 0 != ( nCount = GetFlyList()->Count() ) ) )
    {
        for( MSHORT i = 0; i < nCount; ++i )
        {
            const SdrObject *pObj = (*pFlyList)[ i ];

            SwRect aRect( GetBoundRect( pObj ) );

            // Optimierung
            SWRECTFN( pCurrFrm )
            if( (aRect.*fnRect->fnGetLeft)() > (rRect.*fnRect->fnGetRight)() )
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
                            GetMaster() == &lcl_TheAnchor( pObj ) ||
                            ( FLY_AT_CNTNT != rAnchor.GetAnchorId() &&
                              FLY_AUTO_CNTNT != rAnchor.GetAnchorId() ) ) )
                        || aRect.Top() == WEIT_WECH )
                        continue;
                }

                if ( mbIgnoreCurrentFrame && pCurrFrm == &lcl_TheAnchor( pObj ) )
                    continue;

                if( pRect )
                {
                    SwRect aFly = FlyToRect( pObj, rRect );
                    if( aFly.IsEmpty() || !aFly.IsOver( rRect ) )
                        continue;
                    if( !bRet ||
                        ( !pCurrFrm->IsRightToLeft() &&
                          ( (aFly.*fnRect->fnGetLeft)() <
                            (pRect->*fnRect->fnGetLeft)() ) ||
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
    ASSERT( ! pCurrFrm->IsVertical() || ! pCurrFrm->IsSwapped(),
            "SwTxtFly::CalcRightMargin with swapped frame" )
    SWRECTFN( pCurrFrm )
    SwTwips nRight = (pCurrFrm->Frm().*fnRect->fnGetLeft)() +
                     (pCurrFrm->Prt().*fnRect->fnGetRight)() + 1;
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
    (rFly.*fnRect->fnSetRight)( nRight );
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
    ASSERT( ! pCurrFrm->IsVertical() || ! pCurrFrm->IsSwapped(),
            "SwTxtFly::CalcLeftMargin with swapped frame" )
    SWRECTFN( pCurrFrm )
    SwTwips nLeft = (pCurrFrm->Frm().*fnRect->fnGetLeft)() +
                    (pCurrFrm->Prt().*fnRect->fnGetLeft)();
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

    MSHORT nMyPos = nFlyPos;
    while( ++nFlyPos < pFlyList->Count() )
    {
        const SdrObject *pNext = (*pFlyList)[ nFlyPos ];
        const SwRect aTmp( GetBoundRect( pNext ) );
        if( (aTmp.*fnRect->fnGetLeft)() >= nFlyLeft )
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
                ( pNext, aLine, pCurrFrm, nFlyLeft, sal_False ) );

        if( (aTmp.*fnRect->fnGetLeft)() < nFlyLeft && aTmp.IsOver( aLine ) )
        {
            SwTwips nTmpRight = (aTmp.*fnRect->fnGetRight)();
            if( nLeft <= nTmpRight )
                nLeft = nTmpRight + 1;

            break;
        }
    }
    (rFly.*fnRect->fnSetLeft)( nLeft );
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
    SWRECTFN( pCurrFrm )

    const long nXPos = pCurrFrm->IsRightToLeft() ?
                       rLine.Right() :
                       (rLine.*fnRect->fnGetLeft)();

    SwRect aFly = mbIgnoreContour ?
                  GetBoundRect( pObj ) :
                  SwContourCache::CalcBoundRect( pObj, rLine, pCurrFrm,
                                                 nXPos, ! pCurrFrm->IsRightToLeft() );

    if( !aFly.Width() )
        return aFly;

    SetNextTop( (aFly.*fnRect->fnGetBottom)() ); // Damit die Zeile ggf. bis zur Unterkante
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

    SWRECTFN( pCurrFrm )
    const long nCurrLeft = (pCurrFrm->*fnRect->fnGetPrtLeft)();
    const long nCurrRight = (pCurrFrm->*fnRect->fnGetPrtRight)();
    const SwRect aRect( GetBoundRect( pObj ) );
    long nFlyLeft = (aRect.*fnRect->fnGetLeft)();
    long nFlyRight = (aRect.*fnRect->fnGetRight)();

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

    if( rFlyFmt.IsAnchorOnly() && &lcl_TheAnchor( pObj ) != GetMaster() )
    {
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        if( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
            FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
            return SURROUND_NONE;
    }

    // Beim Durchlauf und Nowrap wird smart ignoriert.
    if( SURROUND_THROUGHT == eOrder || SURROUND_NONE == eOrder )
        return eOrder;

    // left is left and right is right
    if ( pCurrFrm->IsRightToLeft() )
    {
        if ( SURROUND_LEFT == eOrder )
            eOrder = SURROUND_RIGHT;
        else if ( SURROUND_RIGHT == eOrder )
            eOrder = SURROUND_LEFT;
    }

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

    SWAP_IF_SWAPPED( pCurrFrm )

    ASSERT( bOn, "IsAnyFrm: Why?" );

    const sal_Bool bRet = ForEach( rLine, NULL, sal_False );
    UNDO_SWAP( pCurrFrm )
    return bRet;
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
    const SwSortedObjs *pSorted = pPage->GetSortedObjs();
    if( pSorted )
    {
        for ( MSHORT i = 0; i < pSorted->Count(); ++i )
        {
            const SdrObject* pObj = (*pSorted)[i]->GetDrawObj();
            if( this == &lcl_TheAnchor( pObj ) )
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


