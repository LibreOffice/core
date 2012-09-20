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

#include "hintids.hxx"

#include "layfrm.hxx"
#include "ftnboss.hxx"
#include "ndtxt.hxx"
#include "paratr.hxx"
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/spltitem.hxx>
#include <frmatr.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <rowfrm.hxx>

#include "widorp.hxx"
#include "txtfrm.hxx"
#include "itrtxt.hxx"
#include "sectfrm.hxx"  //SwSectionFrm
#include "ftnfrm.hxx"

#undef WIDOWTWIPS


/*************************************************************************
 *                  inline IsNastyFollow()
 *************************************************************************/
// A Follow on the same page as its master ist nasty.
inline sal_Bool IsNastyFollow( const SwTxtFrm *pFrm )
{
    OSL_ENSURE( !pFrm->IsFollow() || !pFrm->GetPrev() ||
            ((const SwTxtFrm*)pFrm->GetPrev())->GetFollow() == pFrm,
            "IsNastyFollow: Was ist denn hier los?" );
    return  pFrm->IsFollow() && pFrm->GetPrev();
}

/*************************************************************************
 *                  SwTxtFrmBreak::SwTxtFrmBreak()
 *************************************************************************/

SwTxtFrmBreak::SwTxtFrmBreak( SwTxtFrm *pNewFrm, const SwTwips nRst )
    : nRstHeight(nRst), pFrm(pNewFrm)
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
 * In contrast to the first implementation the Widows are not calculated
 * in advance but detected when formating the splitted Follow.
 * In Master the Widows-calculation is dropped completely
 * (nWidows is manipulated). If the Follow detects that the
 * Widows rule applies it sends a Prepare to its ancestor.
 * A special problem is when the Widow rule applies but in Master
 * there are some lines available.
 *
 */

/*************************************************************************
 *                  SwTxtFrmBreak::IsInside()
 *************************************************************************/

/* BP(22.07.92): Calculation of Widows and Orphans.
 * The method returns sal_True if one of the rules matches.
 *
 * One difficulty with Widows and different formats between
 * Master- and Follow-Frame:
 * Example: If the first column is 3cm and the second is 4cm and
 * Widows is set to 3. The decision if the Widows rule matches can not
 * be done until the Follow is formated. Unfortunately this is crucial
 * sto decide if the whole paragraph goes to the next page or not.
 */

sal_Bool SwTxtFrmBreak::IsInside( SwTxtMargin &rLine ) const
{
    sal_Bool bFit = sal_False;

    SWAP_IF_SWAPPED( pFrm )
    SWRECTFN( pFrm )
    // nOrigin is an absolut value, rLine referes to the swapped situation.

    SwTwips nTmpY;
    if ( pFrm->IsVertical() )
        nTmpY = pFrm->SwitchHorizontalToVertical( rLine.Y() + rLine.GetLineHeight() );
    else
        nTmpY = rLine.Y() + rLine.GetLineHeight();

    SwTwips nLineHeight = (*fnRect->fnYDiff)( nTmpY , nOrigin );

    // 7455 und 6114: Calculate extra space for bottom border.
    nLineHeight += (pFrm->*fnRect->fnGetBottomMargin)();

    if( nRstHeight )
        bFit = nRstHeight >= nLineHeight;
    else
    {
        // The Frm has a height to fit on the page.
        SwTwips nHeight =
            (*fnRect->fnYDiff)( (pFrm->GetUpper()->*fnRect->fnGetPrtBottom)(), nOrigin );
        // If everything is inside the existing frame the result is sal_True;
        bFit = nHeight >= nLineHeight;

        // --> OD #i103292#
        if ( !bFit )
        {
            if ( rLine.GetNext() &&
                 pFrm->IsInTab() && !pFrm->GetFollow() && !pFrm->GetIndNext() )
            {
                // add additional space taken as lower space as last content in a table
                // for all text lines except the last one.
                nHeight += pFrm->CalcAddLowerSpaceAsLastInTableCell();
                bFit = nHeight >= nLineHeight;
            }
        }
        // <--
        if( !bFit )
        {
            // The LineHeight exceeds the current Frm height.
            // Call a test Grow to detect if the Frame could
            // grow the requested area.
            nHeight += pFrm->GrowTst( LONG_MAX );

            // The Grow() returnes the height, that the Upper of the TxtFrm
            // would grow the TxtFrm.
            // The TxtFrm itself can grow as much as it want.
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

    // bKeep is stronger than IsBreakNow()
    // Is enough space ?
    if( bKeep || IsInside( rLine ) )
        bBreak = sal_False;
    else
    {
        /* This class assumes that the SwTxtMargin is processed from Top to
         * Bottom. Because of performance reasons we stop splitting in the
         * following cases:
         * If only one line does not fit.
         * Special case: with DummyPortions there is LineNr == 1, though we
         * want to split.
         */
        // 6010: include DropLines

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

// OD 2004-02-27 #106629# - no longer inline
void SwTxtFrmBreak::SetRstHeight( const SwTxtMargin &rLine )
{
    // OD, FME 2004-02-27 #106629# - consider bottom margin
    SWRECTFN( pFrm )

    nRstHeight = (pFrm->*fnRect->fnGetBottomMargin)();

    if ( bVert )
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    {
           if ( pFrm->IsVertLR() )
              nRstHeight = (*fnRect->fnYDiff)( pFrm->SwitchHorizontalToVertical( rLine.Y() ) , nOrigin );
           else
               nRstHeight += nOrigin - pFrm->SwitchHorizontalToVertical( rLine.Y() );
    }
    else
        nRstHeight += rLine.Y() - nOrigin;
}

/*************************************************************************
 *                  WidowsAndOrphans::WidowsAndOrphans()
 *************************************************************************/

WidowsAndOrphans::WidowsAndOrphans( SwTxtFrm *pNewFrm, const SwTwips nRst,
    sal_Bool bChkKeep   )
    : SwTxtFrmBreak( pNewFrm, nRst ), nWidLines( 0 ), nOrphLines( 0 )
{
    SWAP_IF_SWAPPED( pFrm )

    if( bKeep )
    {
        // 5652: If pararagraph should not be splited but is larger than
        // the page, then bKeep is overruled.
        if( bChkKeep && !pFrm->GetPrev() && !pFrm->IsInFtn() &&
            pFrm->IsMoveable() &&
            ( !pFrm->IsInSct() || pFrm->FindSctFrm()->MoveAllowed(pFrm) ) )
            bKeep = sal_False;
        // Even if Keep is set, the Orphans has to be regarded
        // e.g. if there are chained frames where a Follow in the last frame
        // receives a Keep, because it is not (forward) movable -
        // nevertheless the paragraph can request lines from the Master
        // because of the Orphan rule.
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

    if ( bKeep || nWidLines || nOrphLines )
    {
        bool bResetFlags = false;

        if ( pFrm->IsInTab() )
        {
            // For compatibility reasons, we disable Keep/Widows/Orphans
            // inside splittable row frames:
            if ( pFrm->GetNextCellLeaf( MAKEPAGE_NONE ) || pFrm->IsInFollowFlowRow() )
            {
                const SwFrm* pTmpFrm = pFrm->GetUpper();
                while ( !pTmpFrm->IsRowFrm() )
                    pTmpFrm = pTmpFrm->GetUpper();
                if ( static_cast<const SwRowFrm*>(pTmpFrm)->IsRowSplitAllowed() )
                    bResetFlags = true;
            }
        }

        if( pFrm->IsInFtn() && !pFrm->GetIndPrev() )
        {
            // Inside of footnotes there are good reasons to turn off the Keep attribute
            // as well as Widows/Orphans.
            SwFtnFrm *pFtn = pFrm->FindFtnFrm();
            sal_Bool bFt = !pFtn->GetAttr()->GetFtn().IsEndNote();
            if( !pFtn->GetPrev() &&
                pFtn->FindFtnBossFrm( bFt ) != pFtn->GetRef()->FindFtnBossFrm( bFt )
                && ( !pFrm->IsInSct() || pFrm->FindSctFrm()->MoveAllowed(pFrm) ) )
            {
                bResetFlags = true;
            }
        }

        if ( bResetFlags )
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

/* The Find*-Methodes do not only search, but adjust the SwTxtMargin to the
 * line where the paragraph should have a break and truncate the paragraph there.
 * FindBreak()
 */

sal_Bool WidowsAndOrphans::FindBreak( SwTxtFrm *pFrame, SwTxtMargin &rLine,
    sal_Bool bHasToFit )
{
    // OD 2004-02-25 #i16128# - Why member <pFrm> _*and*_ parameter <pFrame>??
    // Thus, assertion on situation, that these are different to figure out why.
    OSL_ENSURE( pFrm == pFrame, "<WidowsAndOrphans::FindBreak> - pFrm != pFrame" );

    SWAP_IF_SWAPPED( pFrm )

    sal_Bool bRet = sal_True;
    MSHORT nOldOrphans = nOrphLines;
    if( bHasToFit )
        nOrphLines = 0;
    rLine.Bottom();
    // OD 2004-02-25 #i16128# - method renamed
    if( !IsBreakNowWidAndOrp( rLine ) )
        bRet = sal_False;
    if( !FindWidows( pFrame, rLine ) )
    {
        sal_Bool bBack = sal_False;
        // OD 2004-02-25 #i16128# - method renamed
        while( IsBreakNowWidAndOrp( rLine ) )
        {
            if( rLine.PrevLine() )
                bBack = sal_True;
            else
                break;
        }
        // Normaly Orphans are not taken into account for HasToFit.
        // But if Dummy-Lines are concerned and they violate the orphans rules
        // we make an exception: We leave behind one Dummyline and take
        // the whole text to the next page/column.
        if( rLine.GetLineNr() <= nOldOrphans &&
            rLine.GetInfo().GetParaPortion()->IsDummy() &&
            ( ( bHasToFit && bRet ) || IsBreakNow( rLine ) ) )
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

/*  FindWidows positiones the SwTxtMargin of the Master to the line where to
 *  break by examining and formatting the Follow.
 *  Returns sal_True if the Widows-rule matches, that means that the
 *  paragraph should be not splitted (keep) !
 */

sal_Bool WidowsAndOrphans::FindWidows( SwTxtFrm *pFrame, SwTxtMargin &rLine )
{
    OSL_ENSURE( ! pFrame->IsVertical() || ! pFrame->IsSwapped(),
            "WidowsAndOrphans::FindWidows with swapped frame" );

    if( !nWidLines || !pFrame->IsFollow() )
        return sal_False;

    rLine.Bottom();

    // Wir koennen noch was abzwacken
    SwTxtFrm *pMaster = pFrame->FindMaster();
    OSL_ENSURE(pMaster, "+WidowsAndOrphans::FindWidows: Widows in a master?");
    if( !pMaster )
        return sal_False;

    // 5156: If the first line of the Follow does not fit, the master
    // probably is full of Dummies. In this case a PREP_WIDOWS would be fatal.
    if( pMaster->GetOfst() == pFrame->GetOfst() )
        return sal_False;

    // Remaining height of the master
    SWRECTFN( pFrame )

    const SwTwips nDocPrtTop = (pFrame->*fnRect->fnGetPrtTop)();
    SwTwips nOldHeight;
    SwTwips nTmpY = rLine.Y() + rLine.GetLineHeight();

    if ( bVert )
    {
        nTmpY = pFrame->SwitchHorizontalToVertical( nTmpY );
        nOldHeight = -(pFrame->Prt().*fnRect->fnGetHeight)();
    }
    else
        nOldHeight = (pFrame->Prt().*fnRect->fnGetHeight)();

    const SwTwips nChg = (*fnRect->fnYDiff)( nTmpY, nDocPrtTop + nOldHeight );

    // below the Widows-treshold...
    if( rLine.GetLineNr() >= nWidLines )
    {
        // 8575: Follow to Master I
        // If the Follow *grows*, there is the chance for the Master to
        // receive lines, that it was forced to hand over to the Follow lately:
        // Prepare(Need); This query below of nChg!
        // (0W, 2O, 2M, 2F) + 1F = 3M, 2F
        if( rLine.GetLineNr() > nWidLines && pFrame->IsJustWidow() )
        {
            // Wenn der Master gelockt ist, so hat er vermutlich gerade erst
            // eine Zeile an uns abgegeben, diese geben nicht zurueck, nur
            // weil bei uns daraus mehrere geworden sind (z.B. durch Rahmen).
            if( !pMaster->IsLocked() && pMaster->GetUpper() )
            {
                const SwTwips nTmpRstHeight = (pMaster->Frm().*fnRect->fnBottomDist)
                            ( (pMaster->GetUpper()->*fnRect->fnGetPrtBottom)() );
                if ( nTmpRstHeight >=
                     SwTwips(rLine.GetInfo().GetParaPortion()->Height() ) )
                {
                    pMaster->Prepare( PREP_ADJUST_FRM );
                    pMaster->_InvalidateSize();
                    pMaster->InvalidatePage();
                }
            }

            pFrame->SetJustWidow( sal_False );
        }
        return sal_False;
    }

    // 8575: Follow to Master II
    // If the Follow *shrinks*, maybe the Master can absorb the whole Orphan.
    // (0W, 2O, 2M, 1F) - 1F = 3M, 0F     -> PREP_ADJUST_FRM
    // (0W, 2O, 3M, 2F) - 1F = 2M, 2F     -> PREP_WIDOWS

    if( 0 > nChg && !pMaster->IsLocked() && pMaster->GetUpper() )
    {
        SwTwips nTmpRstHeight = (pMaster->Frm().*fnRect->fnBottomDist)
                             ( (pMaster->GetUpper()->*fnRect->fnGetPrtBottom)() );
        if( nTmpRstHeight >= SwTwips(rLine.GetInfo().GetParaPortion()->Height() ) )
        {
            pMaster->Prepare( PREP_ADJUST_FRM );
            pMaster->_InvalidateSize();
            pMaster->InvalidatePage();
            pFrame->SetJustWidow( sal_False );
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
    // #i91421#
    if ( !pMaster->GetIndPrev() )
    {
        sal_uLong nLines = pMaster->GetThisLines();
        if(nLines == 0 && pMaster->HasPara())
        {
            const SwParaPortion *pMasterPara = pMaster->GetPara();
            if(pMasterPara && pMasterPara->GetNext())
                nLines = 2;
        }
        if( nLines <= nNeed )
            return sal_False;
    }

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

    // We expect that rLine is set to the last line
    OSL_ENSURE( !rLine.GetNext(), "WouldFit: aLine::Bottom missed!" );
    MSHORT nLineCnt = rLine.GetLineNr();

    // First satisfy the Orphans-rule and the wish for initials ...
    const MSHORT nMinLines = Max( GetOrphansLines(), rLine.GetDropLines() );
    if ( nLineCnt < nMinLines )
        return sal_False;

    rLine.Top();
    SwTwips nLineSum = rLine.GetLineHeight();

    while( nMinLines > rLine.GetLineNr() )
    {
        if( !rLine.NextLine() )
            return sal_False;
        nLineSum += rLine.GetLineHeight();
    }

    // We do not fit
    if( !IsInside( rLine ) )
        return sal_False;

    // Check the Widows-rule
    if( !nWidLines && !pFrm->IsFollow() )
    {
        // I.A. We only have to check for Widows if we are a Follow.
        // On WouldFit the rule has to be checked for the Master too,
        // because we are just in the middle of calculating the break.
        // In Ctor of WidowsAndOrphans the nWidLines are only calced for
        // Follows from the AttrSet - so we catch up now:
        const SwAttrSet& rSet = pFrm->GetTxtNode()->GetSwAttrSet();
        nWidLines = rSet.GetWidows().GetValue();
    }

    // Remain after Orphans/Initials enough lines for Widows?
    // #111937#: If we are currently doing a test formatting, we may not
    // consider the widows rule for two reasons:
    // 1. The columns may have different widths.
    //    Widow lines would have wrong width.
    // 2. Test formatting is only done up to the given space.
    //    we do not have any lines for widows at all.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
