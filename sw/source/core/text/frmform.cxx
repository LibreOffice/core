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
#include <editeng/keepitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <pagefrm.hxx>      // ChangeFtnRef
#include <ndtxt.hxx>        // MakeFrm()
#include <dcontact.hxx>     // SwDrawContact
#include <dflyobj.hxx>      // SwVirtFlyDrawObj
#include <flyfrm.hxx>
#include <ftnfrm.hxx>       // SwFtnFrm
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <paratr.hxx>
#include <viewopt.hxx>      // SwViewOptions
#include <viewsh.hxx>       // ViewShell
#include <frmatr.hxx>
#include <pam.hxx>
#include <flyfrms.hxx>
#include <fmtanchr.hxx>
#include <itrform2.hxx>     // SwTxtFormatter
#include <widorp.hxx>       // Widows and Orphans
#include <txtcache.hxx>
#include <porrst.hxx>       // SwEmptyPortion
#include <blink.hxx>        // pBlink
#include <porfld.hxx>       // SwFldPortion
#include <sectfrm.hxx>      // SwSectionFrm
#include <pormulti.hxx>     // SwMultiPortion

#include <rootfrm.hxx>
#include <frmfmt.hxx>       // SwFrmFmt
// OD 2004-05-24 #i28701#
#include <sortedobjs.hxx>

// Tolerance in formatting and text output
#define SLOPPY_TWIPS    5

class FormatLevel
{
    static MSHORT nLevel;
public:
    inline FormatLevel()  { ++nLevel; }
    inline ~FormatLevel() { --nLevel; }
    inline MSHORT GetLevel() const { return nLevel; }
    static bool LastLevel() { return 10 < nLevel; }
};
MSHORT FormatLevel::nLevel = 0;

void ValidateTxt( SwFrm *pFrm )     // Friend of frame
{
    if ( ( ! pFrm->IsVertical() &&
             pFrm->Frm().Width() == pFrm->GetUpper()->Prt().Width() ) ||
         (   pFrm->IsVertical() &&
             pFrm->Frm().Height() == pFrm->GetUpper()->Prt().Height() ) )
        pFrm->mbValidSize = sal_True;
}

void SwTxtFrm::ValidateFrm()
{
    // Validate surroundings to avoid oscillation
    SWAP_IF_SWAPPED( this )

    if ( !IsInFly() && !IsInTab() )
    {   // Only validate 'this' when inside a fly, the rest should actually only be
        // needed for footnotes, which do not exist in flys.
        SwSectionFrm* pSct = FindSctFrm();
        if( pSct )
        {
            if( !pSct->IsColLocked() )
                pSct->ColLock();
            else
                pSct = NULL;
        }

        SwFrm *pUp = GetUpper();
        pUp->Calc();
        if( pSct )
            pSct->ColUnlock();
    }
    ValidateTxt( this );

    // We at least have to save the MustFit flag!
    OSL_ENSURE( HasPara(), "ResetPreps(), missing ParaPortion." );
    SwParaPortion *pPara = GetPara();
    const sal_Bool bMustFit = pPara->IsPrepMustFit();
    ResetPreps();
    pPara->SetPrepMustFit( bMustFit );

    UNDO_SWAP( this )
}

/*************************************************************************
 * ValidateBodyFrm()
 * After a RemoveFtn the BodyFrm and all Frms contained within it, need to be
 * recalculated, so that the DeadLine is right.
 * First we search outwards, on the way back we calculate everything.
*************************************************************************/

void _ValidateBodyFrm( SwFrm *pFrm )
{
    if( pFrm && !pFrm->IsCellFrm() )
    {
        if( !pFrm->IsBodyFrm() && pFrm->GetUpper() )
            _ValidateBodyFrm( pFrm->GetUpper() );
        if( !pFrm->IsSctFrm() )
            pFrm->Calc();
        else
        {
            sal_Bool bOld = ((SwSectionFrm*)pFrm)->IsCntntLocked();
            ((SwSectionFrm*)pFrm)->SetCntntLock( sal_True );
            pFrm->Calc();
            if( !bOld )
                ((SwSectionFrm*)pFrm)->SetCntntLock( sal_False );
        }
    }
}

void SwTxtFrm::ValidateBodyFrm()
{
    SWAP_IF_SWAPPED( this )

     // See comment in ValidateFrm()
    if ( !IsInFly() && !IsInTab() &&
         !( IsInSct() && FindSctFrm()->Lower()->IsColumnFrm() ) )
        _ValidateBodyFrm( GetUpper() );

    UNDO_SWAP( this )
}

sal_Bool SwTxtFrm::_GetDropRect( SwRect &rRect ) const
{
    SWAP_IF_NOT_SWAPPED( this )

    OSL_ENSURE( HasPara(), "SwTxtFrm::_GetDropRect: try again next year." );
    SwTxtSizeInfo aInf( (SwTxtFrm*)this );
    SwTxtMargin aLine( (SwTxtFrm*)this, &aInf );
    if( aLine.GetDropLines() )
    {
        rRect.Top( aLine.Y() );
        rRect.Left( aLine.GetLineStart() );
        rRect.Height( aLine.GetDropHeight() );
        rRect.Width( aLine.GetDropLeft() );

        if ( IsRightToLeft() )
            SwitchLTRtoRTL( rRect );

        if ( IsVertical() )
            SwitchHorizontalToVertical( rRect );
        UNDO_SWAP( this )
        return sal_True;
    }

    UNDO_SWAP( this )

    return sal_False;
}

const SwBodyFrm *SwTxtFrm::FindBodyFrm() const
{
    if ( IsInDocBody() )
    {
        const SwFrm *pFrm = GetUpper();
        while( pFrm && !pFrm->IsBodyFrm() )
            pFrm = pFrm->GetUpper();
        return (const SwBodyFrm*)pFrm;
    }
    return 0;
}

sal_Bool SwTxtFrm::CalcFollow( const xub_StrLen nTxtOfst )
{
    SWAP_IF_SWAPPED( this )

    OSL_ENSURE( HasFollow(), "CalcFollow: missing Follow." );

    SwTxtFrm* pMyFollow = GetFollow();

    SwParaPortion *pPara = GetPara();
    sal_Bool bFollowFld = pPara ? pPara->IsFollowField() : sal_False;

    if( !pMyFollow->GetOfst() || pMyFollow->GetOfst() != nTxtOfst ||
        bFollowFld || pMyFollow->IsFieldFollow() ||
        ( pMyFollow->IsVertical() && !pMyFollow->Prt().Width() ) ||
        ( ! pMyFollow->IsVertical() && !pMyFollow->Prt().Height() ) )
    {
#if OSL_DEBUG_LEVEL > 0
        const SwFrm *pOldUp = GetUpper();
#endif

        SWRECTFN ( this )
        SwTwips nOldBottom = (GetUpper()->Frm().*fnRect->fnGetBottom)();
        SwTwips nMyPos = (Frm().*fnRect->fnGetTop)();

        const SwPageFrm *pPage = 0;
        sal_Bool  bOldInvaCntnt = sal_True;
        if ( !IsInFly() && GetNext() )
        {
            pPage = FindPageFrm();
            // Minimize = that is set back if needed - for invalidation see below
            bOldInvaCntnt  = pPage->IsInvalidCntnt();
        }

        pMyFollow->_SetOfst( nTxtOfst );
        pMyFollow->SetFieldFollow( bFollowFld );
        if( HasFtn() || pMyFollow->HasFtn() )
        {
            ValidateFrm();
            ValidateBodyFrm();
            if( pPara )
            {
                *(pPara->GetReformat()) = SwCharRange();
                *(pPara->GetDelta()) = 0;
            }
        }

        // The footnote area must not get larger
        SwSaveFtnHeight aSave( FindFtnBossFrm( sal_True ), LONG_MAX );

        pMyFollow->CalcFtnFlag();
        if ( !pMyFollow->GetNext() && !pMyFollow->HasFtn() )
            nOldBottom = bVert ? 0 : LONG_MAX;

        while( true )
        {
            if( !FormatLevel::LastLevel() )
            {
                // If the follow is contained within a column section or column
                // frame, we need to calculate that first. This is because the
                // FormatWidthCols() does not work if it is called from MakeAll
                // of the _locked_ follow.
                SwSectionFrm* pSct = pMyFollow->FindSctFrm();
                if( pSct && !pSct->IsAnLower( this ) )
                {
                    if( pSct->GetFollow() )
                        pSct->SimpleFormat();
                    else if( ( pSct->IsVertical() && !pSct->Frm().Width() ) ||
                             ( ! pSct->IsVertical() && !pSct->Frm().Height() ) )
                        break;
                }
                // OD 14.03.2003 #i11760# - intrinsic format of follow is controlled.
                if ( FollowFormatAllowed() )
                {
                    // OD 14.03.2003 #i11760# - no nested format of follows, if
                    // text frame is contained in a column frame.
                    // Thus, forbid intrinsic format of follow.
                    {
                        bool bIsFollowInColumn = false;
                        SwFrm* pFollowUpper = pMyFollow->GetUpper();
                        while ( pFollowUpper )
                        {
                            if ( pFollowUpper->IsColumnFrm() )
                            {
                                bIsFollowInColumn = true;
                                break;
                            }
                            if ( pFollowUpper->IsPageFrm() ||
                                 pFollowUpper->IsFlyFrm() )
                            {
                                break;
                            }
                            pFollowUpper = pFollowUpper->GetUpper();
                        }
                        if ( bIsFollowInColumn )
                        {
                            pMyFollow->ForbidFollowFormat();
                        }
                    }

                    pMyFollow->Calc();
                    // The Follow can tell from its Frm().Height() that something went wrong
                    OSL_ENSURE( !pMyFollow->GetPrev(), "SwTxtFrm::CalcFollow: cheesy follow" );
                    if( pMyFollow->GetPrev() )
                    {
                        pMyFollow->Prepare( PREP_CLEAR );
                        pMyFollow->Calc();
                        OSL_ENSURE( !pMyFollow->GetPrev(), "SwTxtFrm::CalcFollow: very cheesy follow" );
                    }

                    // OD 14.03.2003 #i11760# - reset control flag for follow format.
                    pMyFollow->AllowFollowFormat();
                }

                // Make sure that the Follow gets painted
                pMyFollow->SetCompletePaint();
            }

            pPara = GetPara();
            // As long as the Follow is requested due to orphan lines, it is passed these
            // and is reformatted if possible
            if( pPara && pPara->IsPrepWidows() )
                CalcPreps();
            else
                break;
        }

        if( HasFtn() || pMyFollow->HasFtn() )
        {
            ValidateBodyFrm();
            ValidateFrm();
            if( pPara )
            {
                *(pPara->GetReformat()) = SwCharRange();
                *(pPara->GetDelta()) = 0;
            }
        }

        if ( pPage )
        {
            if ( !bOldInvaCntnt )
                pPage->ValidateCntnt();
        }

#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( pOldUp == GetUpper(), "SwTxtFrm::CalcFollow: heavy follow" );
#endif

        const long nRemaining =
                 - (GetUpper()->Frm().*fnRect->fnBottomDist)( nOldBottom );
        if (  nRemaining > 0 && !GetUpper()->IsSctFrm() &&
              nRemaining != ( bVert ?
                              nMyPos - Frm().Right() :
                              Frm().Top() - nMyPos ) )
        {
            UNDO_SWAP( this )
            return sal_True;
        }
    }

    UNDO_SWAP( this )

    return sal_False;
}

void SwTxtFrm::AdjustFrm( const SwTwips nChgHght, sal_Bool bHasToFit )
{
    if( IsUndersized() )
    {
        if( GetOfst() && !IsFollow() ) // A scrolled paragraph (undersized)
            return;
        SetUndersized( nChgHght == 0 || bHasToFit );
    }

    // AdjustFrm is called with a swapped frame during
    // formatting but the frame is not swapped during FormatEmpty
    SWAP_IF_SWAPPED( this )
    SWRECTFN ( this )

    // The Frame's size variable is incremented by Grow or decremented by Shrink.
    // If the size cannot change, nothing should happen!
    if( nChgHght >= 0)
    {
        SwTwips nChgHeight = nChgHght;
        if( nChgHght && !bHasToFit )
        {
            if( IsInFtn() && !IsInSct() )
            {
                SwTwips nReal = Grow( nChgHght, sal_True );
                if( nReal < nChgHght )
                {
                    SwTwips nBot = (*fnRect->fnYInc)( (Frm().*fnRect->fnGetBottom)(),
                                                      nChgHght - nReal );
                    SwFrm* pCont = FindFtnFrm()->GetUpper();

                    if( (pCont->Frm().*fnRect->fnBottomDist)( nBot ) > 0 )
                    {
                        (Frm().*fnRect->fnAddBottom)( nChgHght );
                        if( bVert )
                            Prt().SSize().Width() += nChgHght;
                        else
                            Prt().SSize().Height() += nChgHght;
                        UNDO_SWAP( this )
                        return;
                    }
                }
            }

            Grow( nChgHght );

            if ( IsInFly() )
            {
                // If one of the Upper is a Fly, it's very likely that this fly changes its
                // position by the Grow. Therefore, my position has to be corrected also or
                // the check further down is not meaningful.
                // The predecessors need to be calculated, so that the position can be
                // calculated correctly.
                if ( GetPrev() )
                {
                    SwFrm *pPre = GetUpper()->Lower();
                    do
                    {   pPre->Calc();
                        pPre = pPre->GetNext();
                    } while ( pPre && pPre != this );
                }
                const Point aOldPos( Frm().Pos() );
                MakePos();
                if ( aOldPos != Frm().Pos() )
                {
                    // OD 2004-07-01 #i28701# - use new method <SwFrm::InvalidateObjs(..)>
                    // No format is performed for the floating screen objects.
                    InvalidateObjs( true );
                }
            }
            nChgHeight = 0;
        }
        // A Grow() is always accepted by the Layout, even if the
        // FixSize of the surrounding layout frame should not allow it.
        // We text for this case and correct the values.
        // The Frm must NOT be shrinked further than its size permits
        // even in the case of an emergency.
        SwTwips nRstHeight;
        if ( IsVertical() )
        {
            OSL_ENSURE( ! IsSwapped(),"Swapped frame while calculating nRstHeight" );

            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
            if ( IsVertLR() )
                    nRstHeight = GetUpper()->Frm().Left()
                               + GetUpper()->Prt().Left()
                               + GetUpper()->Prt().Width()
                               - Frm().Left();
            else
                nRstHeight = Frm().Left() + Frm().Width() -
                            ( GetUpper()->Frm().Left() + GetUpper()->Prt().Left() );
         }
        else
            nRstHeight = GetUpper()->Frm().Top()
                       + GetUpper()->Prt().Top()
                       + GetUpper()->Prt().Height()
                       - Frm().Top();

        // We can get a bit of space in table cells, because there could be some
        // left through a vertical alignment to the top.
        // #115759# - assure, that first lower in upper
        // is the current one or is valid.
        if ( IsInTab() &&
             ( GetUpper()->Lower() == this ||
               GetUpper()->Lower()->IsValid() ) )
        {
            long nAdd = (*fnRect->fnYDiff)( (GetUpper()->Lower()->Frm().*fnRect->fnGetTop)(),
                                            (GetUpper()->*fnRect->fnGetPrtTop)() );
            OSL_ENSURE( nAdd >= 0, "Ey" );
            nRstHeight += nAdd;
        }

/* ------------------------------------
 * nRstHeight < 0 means that the TxtFrm is located completely outside of its Upper.
 * This can happen, if it's located within a FlyAtCntFrm, which changed sides by a
 * Grow(). In such a case, it's wrong to execute the following Grow().
 * In the case of a bug, we end up with an infinite loop.
 * -----------------------------------*/
        SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
        SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();

        if( nRstHeight < nFrmHeight )
        {
            // It can be that I have the right size, but the Upper is too small and can get me some room
            if( ( nRstHeight >= 0 || ( IsInFtn() && IsInSct() ) ) && !bHasToFit )
                nRstHeight += GetUpper()->Grow( nFrmHeight - nRstHeight );
            // In column sections we do not want to get too big or else more areas are created by
            // GetNextSctLeaf. Instead, we shrink and remember bUndersized, so that FormatWidthCols
            // can calculate the right column size.
            if ( nRstHeight < nFrmHeight )
            {
                if( bHasToFit || !IsMoveable() ||
                    ( IsInSct() && !FindSctFrm()->MoveAllowed(this) ) )
                {
                    SetUndersized( sal_True );
                    Shrink( std::min( ( nFrmHeight - nRstHeight), nPrtHeight ) );
                }
                else
                    SetUndersized( sal_False );
            }
        }
        else if( nChgHeight )
        {
            if( nRstHeight - nFrmHeight < nChgHeight )
                nChgHeight = nRstHeight - nFrmHeight;
            if( nChgHeight )
                Grow( nChgHeight );
        }
    }
    else
        Shrink( -nChgHght );

    UNDO_SWAP( this )
}

/*************************************************************************
 * SwTxtFrm::AdjustFollow()
 * AdjustFollow expects the following situation:
 * The SwTxtIter points to the lower end of the Master, the Offset is set in
 * the Follow.
 * nOffset holds the Offset in the text string, from which the Master closes
 * and the Follow starts.
 * If it's 0, the FollowFrame is deleted.
 *************************************************************************/

void SwTxtFrm::_AdjustFollow( SwTxtFormatter &rLine,
                             const xub_StrLen nOffset, const xub_StrLen nEnd,
                             const sal_uInt8 nMode )
{
    SwFrmSwapper aSwapper( this, sal_False );

    // We got the rest of the text mass: Delete all Follows
    // DummyPortions() are a special case.
    // Special cases are controlled by parameter <nMode>.
    if( HasFollow() && !(nMode & 1) && nOffset == nEnd )
    {
        while( GetFollow() )
        {
            if( ((SwTxtFrm*)GetFollow())->IsLocked() )
            {
                OSL_FAIL( "+SwTxtFrm::JoinFrm: Follow is locked." );
                return;
            }
            JoinFrm();
        }

        return;
    }

    // Dancing on the volcano: We'll just format the last line quickly
    // for the QuoVadis stuff.
    // The Offset can move of course:
    const xub_StrLen nNewOfst = ( IsInFtn() && ( !GetIndNext() || HasFollow() ) ) ?
                            rLine.FormatQuoVadis(nOffset) : nOffset;

    if( !(nMode & 1) )
    {
        // We steal text mass from our Follows
        // It can happen that we have to join some of them
        while( GetFollow() && GetFollow()->GetFollow() &&
               nNewOfst >= GetFollow()->GetFollow()->GetOfst() )
        {
            JoinFrm();
        }
    }

    // The Offset moved
    if( GetFollow() )
    {
#if OSL_DEBUG_LEVEL > 1
        static bool bTest = false;
        if( !bTest || ( nMode & 1 ) )
#endif
        if ( nMode )
            GetFollow()->ManipOfst( 0 );

        if ( CalcFollow( nNewOfst ) )   // CalcFollow only at the end, we do a SetOfst there
            rLine.SetOnceMore( sal_True );
    }
}

SwCntntFrm *SwTxtFrm::JoinFrm()
{
    OSL_ENSURE( GetFollow(), "+SwTxtFrm::JoinFrm: no follow" );
    SwTxtFrm  *pFoll = GetFollow();

    SwTxtFrm *pNxt = pFoll->GetFollow();

    // All footnotes of the to-be-destroyed Follow are relocated to us
    xub_StrLen nStart = pFoll->GetOfst();
    if ( pFoll->HasFtn() )
    {
        const SwpHints *pHints = pFoll->GetTxtNode()->GetpSwpHints();
        if( pHints )
        {
            SwFtnBossFrm *pFtnBoss = 0;
            SwFtnBossFrm *pEndBoss = 0;
            for ( sal_uInt16 i = 0; i < pHints->Count(); ++i )
            {
                const SwTxtAttr *pHt = (*pHints)[i];
                if( RES_TXTATR_FTN==pHt->Which() && *pHt->GetStart()>=nStart )
                {
                    if( pHt->GetFtn().IsEndNote() )
                    {
                        if( !pEndBoss )
                            pEndBoss = pFoll->FindFtnBossFrm();
                        pEndBoss->ChangeFtnRef( pFoll, (SwTxtFtn*)pHt, this );
                    }
                    else
                    {
                        if( !pFtnBoss )
                            pFtnBoss = pFoll->FindFtnBossFrm( sal_True );
                        pFtnBoss->ChangeFtnRef( pFoll, (SwTxtFtn*)pHt, this );
                    }
                    SetFtn( sal_True );
                }
            }
        }
    }

#ifdef DBG_UTIL
    else if ( pFoll->GetValidPrtAreaFlag() ||
              pFoll->GetValidSizeFlag() )
    {
        pFoll->CalcFtnFlag();
        OSL_ENSURE( !pFoll->HasFtn(), "Missing FtnFlag." );
    }
#endif

    pFoll->MoveFlyInCnt( this, nStart, STRING_LEN );
    pFoll->SetFtn( sal_False );
    // #i27138#
    // notify accessibility paragraphs objects about changed CONTENT_FLOWS_FROM/_TO relation.
    // Relation CONTENT_FLOWS_FROM for current next paragraph will change
    // and relation CONTENT_FLOWS_TO for current previous paragraph, which
    // is <this>, will change.
    {
        ViewShell* pViewShell( pFoll->getRootFrm()->GetCurrShell() );
        if ( pViewShell && pViewShell->GetLayout() &&
             pViewShell->GetLayout()->IsAnyShellAccessible() )
        {
            pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pFoll->FindNextCnt( true )),
                            this );
        }
    }
    pFoll->Cut();
    SetFollow(pNxt);
    delete pFoll;
    return pNxt;
}

SwCntntFrm *SwTxtFrm::SplitFrm( const xub_StrLen nTxtPos )
{
    SWAP_IF_SWAPPED( this )

    // The Paste sends a Modify() to me
    // I lock myself, so that my data does not disappear
    SwTxtFrmLocker aLock( this );
    SwTxtFrm *pNew = (SwTxtFrm *)(GetTxtNode()->MakeFrm( this ));

    pNew->SetFollow( GetFollow() );
    SetFollow( pNew );

    pNew->Paste( GetUpper(), GetNext() );
    // #i27138#
    // notify accessibility paragraphs objects about changed CONTENT_FLOWS_FROM/_TO relation.
    // Relation CONTENT_FLOWS_FROM for current next paragraph will change
    // and relation CONTENT_FLOWS_TO for current previous paragraph, which
    // is <this>, will change.
    {
        ViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
        if ( pViewShell && pViewShell->GetLayout() &&
             pViewShell->GetLayout()->IsAnyShellAccessible() )
        {
            pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pNew->FindNextCnt( true )),
                            this );
        }
    }

    // If footnotes end up in pNew bz our actions, we need
    // to re-register them
    if ( HasFtn() )
    {
        const SwpHints *pHints = GetTxtNode()->GetpSwpHints();
        if( pHints )
        {
            SwFtnBossFrm *pFtnBoss = 0;
            SwFtnBossFrm *pEndBoss = 0;
            for ( sal_uInt16 i = 0; i < pHints->Count(); ++i )
            {
                const SwTxtAttr *pHt = (*pHints)[i];
                if( RES_TXTATR_FTN==pHt->Which() && *pHt->GetStart()>=nTxtPos )
                {
                    if( pHt->GetFtn().IsEndNote() )
                    {
                        if( !pEndBoss )
                            pEndBoss = FindFtnBossFrm();
                        pEndBoss->ChangeFtnRef( this, (SwTxtFtn*)pHt, pNew );
                    }
                    else
                    {
                        if( !pFtnBoss )
                            pFtnBoss = FindFtnBossFrm( sal_True );
                        pFtnBoss->ChangeFtnRef( this, (SwTxtFtn*)pHt, pNew );
                    }
                    pNew->SetFtn( sal_True );
                }
            }
        }
    }

#ifdef DBG_UTIL
    else
    {
        CalcFtnFlag( nTxtPos-1 );
        OSL_ENSURE( !HasFtn(), "Missing FtnFlag." );
    }
#endif

    MoveFlyInCnt( pNew, nTxtPos, STRING_LEN );

    // No SetOfst or CalcFollow, because an AdjustFollow follows immediately anyways

    pNew->ManipOfst( nTxtPos );

    UNDO_SWAP( this )
    return pNew;
}

void SwTxtFrm::_SetOfst( const xub_StrLen nNewOfst )
{
    // We do not need to invalidate out Follow.
    // We are a Follow, get formatted right away and call
    // SetOfst() from there
    nOfst = nNewOfst;
    SwParaPortion *pPara = GetPara();
    if( pPara )
    {
        SwCharRange &rReformat = *(pPara->GetReformat());
        rReformat.Start() = 0;
        rReformat.Len() = GetTxt().getLength();
        *(pPara->GetDelta()) = rReformat.Len();
    }
    InvalidateSize();
}

sal_Bool SwTxtFrm::CalcPreps()
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(), "SwTxtFrm::CalcPreps with swapped frame" );
    SWRECTFN( this );

    SwParaPortion *pPara = GetPara();
    if ( !pPara )
        return sal_False;
    sal_Bool bPrep = pPara->IsPrep();
    sal_Bool bPrepWidows = pPara->IsPrepWidows();
    sal_Bool bPrepAdjust = pPara->IsPrepAdjust();
    sal_Bool bPrepMustFit = pPara->IsPrepMustFit();
    ResetPreps();

    sal_Bool bRet = sal_False;
    if( bPrep && !pPara->GetReformat()->Len() )
    {
        // PREP_WIDOWS means that the orphans rule got activated in the Follow.
        // In unfortunate cases we could also have a PrepAdjust!
        if( bPrepWidows )
        {
            if( !GetFollow() )
            {
                OSL_ENSURE( GetFollow(), "+SwTxtFrm::CalcPreps: no credits" );
                return sal_False;
            }

            // We need to prepare for two cases:
            // We were able to hand over a few lines to the Follow
            // -> we need to shrink
            // or we need to go on the next page
            // -> we let our Frame become too big

            SwTwips nChgHeight = GetParHeight();
            if( nChgHeight >= (Prt().*fnRect->fnGetHeight)() )
            {
                if( bPrepMustFit )
                {
                    GetFollow()->SetJustWidow( sal_True );
                    GetFollow()->Prepare( PREP_CLEAR );
                }
                else if ( bVert )
                {
                    Frm().Width( Frm().Width() + Frm().Left() );
                    Prt().Width( Prt().Width() + Frm().Left() );
                    Frm().Left( 0 );
                    SetWidow( sal_True );
                }
                else
                {
                    SwTwips nTmp  = LONG_MAX - (Frm().Top()+10000);
                    SwTwips nDiff = nTmp - Frm().Height();
                    Frm().Height( nTmp );
                    Prt().Height( Prt().Height() + nDiff );
                    SetWidow( sal_True );
                }
            }
            else
            {
                OSL_ENSURE( nChgHeight < (Prt().*fnRect->fnGetHeight)(),
                        "+SwTxtFrm::CalcPrep: wanna shrink" );

                nChgHeight = (Prt().*fnRect->fnGetHeight)() - nChgHeight;

                GetFollow()->SetJustWidow( sal_True );
                GetFollow()->Prepare( PREP_CLEAR );
                Shrink( nChgHeight );
                SwRect &rRepaint = *(pPara->GetRepaint());

                if ( bVert )
                {
                    SwRect aRepaint( Frm().Pos() + Prt().Pos(), Prt().SSize() );
                    SwitchVerticalToHorizontal( aRepaint );
                    rRepaint.Chg( aRepaint.Pos(), aRepaint.SSize() );
                }
                else
                    rRepaint.Chg( Frm().Pos() + Prt().Pos(), Prt().SSize() );

                if( 0 >= rRepaint.Width() )
                    rRepaint.Width(1);
            }
            bRet = sal_True;
        }

        else if ( bPrepAdjust )
        {
            if ( HasFtn() )
            {
                if( !CalcPrepFtnAdjust() )
                {
                    if( bPrepMustFit )
                    {
                        SwTxtLineAccess aAccess( this );
                        aAccess.GetPara()->SetPrepMustFit( sal_True );
                    }
                    return sal_False;
                }
            }

            SWAP_IF_NOT_SWAPPED( this )

            SwTxtFormatInfo aInf( this );
            SwTxtFormatter aLine( this, &aInf );

            WidowsAndOrphans aFrmBreak( this );
            // Whatever the attributes say: we split the paragraph in
            // MustFit in any case
            if( bPrepMustFit )
            {
                aFrmBreak.SetKeep( sal_False );
                aFrmBreak.ClrOrphLines();
            }
            // Before calling FormatAdjust, we need to make sure
            // that the lines protruding at the bottom get indeed
            // truncated
            sal_Bool bBreak = aFrmBreak.IsBreakNowWidAndOrp( aLine );
            bRet = sal_True;
            while( !bBreak && aLine.Next() )
            {
                bBreak = aFrmBreak.IsBreakNowWidAndOrp( aLine );
            }
            if( bBreak )
            {
                // We run into troubles: when TruncLines get called, the
                // conditions in IsInside change immediately such that
                // IsBreakNow can return different results.
                // For this reason, we make it clear to rFrmBreak, that the
                // end is reached at the location of rLine.
                // Let's see if it works ...
                aLine.TruncLines();
                aFrmBreak.SetRstHeight( aLine );
                FormatAdjust( aLine, aFrmBreak, aInf.GetTxt().getLength(), aInf.IsStop() );
            }
            else
            {
                if( !GetFollow() )
                {
                    FormatAdjust( aLine, aFrmBreak,
                                  aInf.GetTxt().getLength(), aInf.IsStop() );
                }
                else if ( !aFrmBreak.IsKeepAlways() )
                {
                    // We delete a line before the Master, because the Follow
                    // could hand over a line
                    const SwCharRange aFollowRg( GetFollow()->GetOfst(), 1 );
                    *(pPara->GetReformat()) += aFollowRg;
                    // We should continue!
                    bRet = sal_False;
                }
            }

            UNDO_SWAP( this )
            // A final check, if FormatAdjust() didn't help we need to
            // truncate
            if( bPrepMustFit )
            {
                const SwTwips nMust = (GetUpper()->*fnRect->fnGetPrtBottom)();
                const SwTwips nIs   = (Frm().*fnRect->fnGetBottom)();

                if( bVert && nIs < nMust )
                {
                    Shrink( nMust - nIs );
                    if( Prt().Width() < 0 )
                        Prt().Width( 0 );
                    SetUndersized( sal_True );
                }
                else if ( ! bVert && nIs > nMust )
                {
                    Shrink( nIs - nMust );
                    if( Prt().Height() < 0 )
                        Prt().Height( 0 );
                    SetUndersized( sal_True );
                }
            }
        }
    }
    pPara->SetPrepMustFit( bPrepMustFit );
    return bRet;
}

/*************************************************************************
 * SwTxtFrm::FormatAdjust()
 * We rewire the footnotes and the character bound objects
 *************************************************************************/

#define CHG_OFFSET( pFrm, nNew )\
    {\
        if( pFrm->GetOfst() < nNew )\
            pFrm->MoveFlyInCnt( this, 0, nNew );\
        else if( pFrm->GetOfst() > nNew )\
            MoveFlyInCnt( pFrm, nNew, STRING_LEN );\
    }

void SwTxtFrm::FormatAdjust( SwTxtFormatter &rLine,
                             WidowsAndOrphans &rFrmBreak,
                             const xub_StrLen nStrLen,
                             const sal_Bool bDummy )
{
    SWAP_IF_NOT_SWAPPED( this )

    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();

    xub_StrLen nEnd = rLine.GetStart();

    sal_Bool bHasToFit = pPara->IsPrepMustFit();

    // The StopFlag is set by footnotes which want to go onto the next page
    // Call base class method <SwTxtFrmBreak::IsBreakNow(..)>
    // instead of method <WidowsAndOrphans::IsBreakNow(..)> to get a break,
    // even if due to widow rule no enough lines exists.
    sal_uInt8 nNew = ( !GetFollow() &&
                       nEnd < nStrLen &&
                       ( rLine.IsStop() ||
                         ( bHasToFit
                           ? ( rLine.GetLineNr() > 1 &&
                               !rFrmBreak.IsInside( rLine ) )
                           : rFrmBreak.IsBreakNow( rLine ) ) ) )
                     ? 1 : 0;
    // --> OD #i84870#
    // no split of text frame, which only contains a as-character anchored object
    const bool bOnlyContainsAsCharAnchoredObj =
            !IsFollow() && nStrLen == 1 &&
            GetDrawObjs() && GetDrawObjs()->Count() == 1 &&
            (*GetDrawObjs())[0]->GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AS_CHAR;
    if ( nNew && bOnlyContainsAsCharAnchoredObj )
    {
        nNew = 0;
    }
    // <--
    if ( nNew )
    {
        SplitFrm( nEnd );
    }

    const SwFrm *pBodyFrm = (const SwFrm*)(FindBodyFrm());

    const long nBodyHeight = pBodyFrm ? ( IsVertical() ?
                                          pBodyFrm->Frm().Width() :
                                          pBodyFrm->Frm().Height() ) : 0;

    // If the current values have been calculated, show that they
    // are valid now
    *(pPara->GetReformat()) = SwCharRange();
    bool bDelta = *pPara->GetDelta() != 0;
    *(pPara->GetDelta()) = 0;

    if( rLine.IsStop() )
    {
        rLine.TruncLines( true );
        nNew = 1;
    }

    // FindBreak truncates the last line
    if( !rFrmBreak.FindBreak( this, rLine, bHasToFit ) )
    {
        // If we're done formatting, we set nEnd to the end.
        // AdjustFollow might execute JoinFrm() because of this.
        // Else, nEnd is the end of the last line in the Master.
        xub_StrLen nOld = nEnd;
        nEnd = rLine.GetEnd();
        if( GetFollow() )
        {
            if( nNew && nOld < nEnd )
                RemoveFtn( nOld, nEnd - nOld );
            CHG_OFFSET( GetFollow(), nEnd )
            if( !bDelta )
                GetFollow()->ManipOfst( nEnd );
        }
    }
    else
    {   // If we pass over lines, we must not call Join in Follows, instead we even
        // need to create a Follow.
        // We also need to do this if the whole mass of text remains in the Master,
        // because a hard line break could necessitate another line (without text mass)!
        nEnd = rLine.GetEnd();
        if( GetFollow() )
        {
            // Another case for not joining the follow:
            // Text frame has no content, but a numbering. Then, do *not* join.
            // Example of this case: When an empty, but numbered paragraph
            // at the end of page is completely displaced by a fly frame.
            // Thus, the text frame introduced a follow by a
            // <SwTxtFrm::SplitFrm(..)> - see below. The follow then shows
            // the numbering and must stay.
            if ( GetFollow()->GetOfst() != nEnd ||
                 GetFollow()->IsFieldFollow() ||
                 ( nStrLen == 0 && GetTxtNode()->GetNumRule() ) )
            {
                nNew |= 3;
            }
            CHG_OFFSET( GetFollow(), nEnd )
            GetFollow()->ManipOfst( nEnd );
        }
        else
        {
            // Only split frame, if the frame contains
            // content or contains no content, but has a numbering.
            // OD #i84870# - no split, if text frame only contains one
            // as-character anchored object.
            if ( !bOnlyContainsAsCharAnchoredObj &&
                 ( nStrLen > 0 ||
                   ( nStrLen == 0 && GetTxtNode()->GetNumRule() ) )
               )
            {
                SplitFrm( nEnd );
                nNew |= 3;
            }
        }
        // If the remaining height changed e.g by RemoveFtn() we need to
        // fill up in order to avoid oscillation.
        if( bDummy && pBodyFrm &&
           nBodyHeight < ( IsVertical() ?
                           pBodyFrm->Frm().Width() :
                           pBodyFrm->Frm().Height() ) )
            rLine.MakeDummyLine();
    }

    // In AdjustFrm() we set ourselves via Grow/Shrink
    // In AdjustFollow() we set our FollowFrame

    const SwTwips nDocPrtTop = Frm().Top() + Prt().Top();
    const SwTwips nOldHeight = Prt().SSize().Height();
    SwTwips nChg = rLine.CalcBottomLine() - nDocPrtTop - nOldHeight;
    // --> OD #i84870# - no shrink of text frame, if it only contains one
    // as-character anchored object.
    if ( nChg < 0 &&
         bOnlyContainsAsCharAnchoredObj )
    {
        nChg = 0;
    }
    // <--

    // Vertical Formatting:
    // The (rotated) repaint rectangle's x coordinate referes to the frame.
    // If the frame grows (or shirks) the repaint rectangle cannot simply
    // be rotated back after formatting, because we use the upper left point
    // of the frame for rotation. This point changes when growing/shrinking.

    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    if ( IsVertical() && !IsVertLR() && nChg )
    {
        SwRect &rRepaint = *(pPara->GetRepaint());
        rRepaint.Left( rRepaint.Left() - nChg );
        rRepaint.Width( rRepaint.Width() - nChg );
    }

    AdjustFrm( nChg, bHasToFit );

    if( HasFollow() || IsInFtn() )
        _AdjustFollow( rLine, nEnd, nStrLen, nNew );

    pPara->SetPrepMustFit( sal_False );

    UNDO_SWAP( this )
}

/*************************************************************************
 * SwTxtFrm::FormatLine()
 * bPrev is set whether Reformat.Start() was called because of Prev().
 * Else, wo don't know whether we can limit the repaint or not.
*************************************************************************/

sal_Bool SwTxtFrm::FormatLine( SwTxtFormatter &rLine, const sal_Bool bPrev )
{
    OSL_ENSURE( ! IsVertical() || IsSwapped(),
            "SwTxtFrm::FormatLine( rLine, bPrev) with unswapped frame" );
    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
    const SwLineLayout *pOldCur = rLine.GetCurr();
    const xub_StrLen nOldLen    = pOldCur->GetLen();
    const KSHORT nOldAscent = pOldCur->GetAscent();
    const KSHORT nOldHeight = pOldCur->Height();
    const SwTwips nOldWidth = pOldCur->Width() + pOldCur->GetHangingMargin();
    const sal_Bool bOldHyph = pOldCur->IsEndHyph();
    SwTwips nOldTop = 0;
    SwTwips nOldBottom = 0;
    if( rLine.GetCurr()->IsClipping() )
        rLine.CalcUnclipped( nOldTop, nOldBottom );

    const xub_StrLen nNewStart = rLine.FormatLine( rLine.GetStart() );

    OSL_ENSURE( Frm().Pos().Y() + Prt().Pos().Y() == rLine.GetFirstPos(),
            "SwTxtFrm::FormatLine: frame leaves orbit." );
    OSL_ENSURE( rLine.GetCurr()->Height(),
            "SwTxtFrm::FormatLine: line height is zero" );

    // The current line break object
    const SwLineLayout *pNew = rLine.GetCurr();

    bool bUnChg = nOldLen == pNew->GetLen() &&
                  bOldHyph == pNew->IsEndHyph();
    if ( bUnChg && !bPrev )
    {
        const long nWidthDiff = nOldWidth > pNew->Width()
                                ? nOldWidth - pNew->Width()
                                : pNew->Width() - nOldWidth;

        // we only declare a line as unchanged, if its main values have not
        // changed and it is not the last line (!paragraph end symbol!)
        bUnChg = nOldHeight == pNew->Height() &&
                 nOldAscent == pNew->GetAscent() &&
                 nWidthDiff <= SLOPPY_TWIPS &&
                 pOldCur->GetNext();
    }

    // Calculate rRepaint
    const SwTwips nBottom = rLine.Y() + rLine.GetLineHeight();
    SwRepaint &rRepaint = *(pPara->GetRepaint());
    if( bUnChg && rRepaint.Top() == rLine.Y()
               && (bPrev || nNewStart <= pPara->GetReformat()->Start())
               && (nNewStart < GetTxtNode()->GetTxt().getLength()))
    {
        rRepaint.Top( nBottom );
        rRepaint.Height( 0 );
    }
    else
    {
        if( nOldTop )
        {
            if( nOldTop < rRepaint.Top() )
                rRepaint.Top( nOldTop );
            if( !rLine.IsUnclipped() || nOldBottom > rRepaint.Bottom() )
            {
                rRepaint.Bottom( nOldBottom - 1 );
                rLine.SetUnclipped( sal_True );
            }
        }
        if( rLine.GetCurr()->IsClipping() && rLine.IsFlyInCntBase() )
        {
            SwTwips nTmpTop, nTmpBottom;
            rLine.CalcUnclipped( nTmpTop, nTmpBottom );
            if( nTmpTop < rRepaint.Top() )
                rRepaint.Top( nTmpTop );
            if( !rLine.IsUnclipped() || nTmpBottom > rRepaint.Bottom() )
            {
                rRepaint.Bottom( nTmpBottom - 1 );
                rLine.SetUnclipped( sal_True );
            }
        }
        else
        {
            if( !rLine.IsUnclipped() || nBottom > rRepaint.Bottom() )
            {
                rRepaint.Bottom( nBottom - 1 );
                rLine.SetUnclipped( sal_False );
            }
        }
        SwTwips nRght = std::max( nOldWidth, pNew->Width() +
                             pNew->GetHangingMargin() );
        ViewShell *pSh = getRootFrm()->GetCurrShell();
        const SwViewOption *pOpt = pSh ? pSh->GetViewOptions() : 0;
        if( pOpt && (pOpt->IsParagraph() || pOpt->IsLineBreak()) )
            nRght += ( std::max( nOldAscent, pNew->GetAscent() ) );
        else
            nRght += ( std::max( nOldAscent, pNew->GetAscent() ) / 4);
        nRght += rLine.GetLeftMargin();
        if( rRepaint.GetOfst() || rRepaint.GetRightOfst() < nRght )
            rRepaint.SetRightOfst( nRght );

        // Finally we enlarge the repaint rectangle if we found an underscore
        // within our line. 40 Twips should be enough
        const sal_Bool bHasUnderscore =
                ( rLine.GetInfo().GetUnderScorePos() < nNewStart );
        if ( bHasUnderscore || rLine.GetCurr()->HasUnderscore() )
            rRepaint.Bottom( rRepaint.Bottom() + 40 );

        ((SwLineLayout*)rLine.GetCurr())->SetUnderscore( bHasUnderscore );
    }
    if( !bUnChg )
        rLine.SetChanges();

    // Calculating the good ol' nDelta
    *(pPara->GetDelta()) -= long(pNew->GetLen()) - long(nOldLen);

    // Stop!
    if( rLine.IsStop() )
        return sal_False;

    // Absolutely another line
    if( rLine.IsNewLine() )
        return sal_True;

    // Until the String's end?
    if (nNewStart >= GetTxtNode()->GetTxt().getLength())
        return sal_False;

    if( rLine.GetInfo().IsShift() )
        return sal_True;

    // Reached the Reformat's end?
    const xub_StrLen nEnd = pPara->GetReformat()->Start() +
                        pPara->GetReformat()->Len();

    if( nNewStart <= nEnd )
        return sal_True;

    return 0 != *(pPara->GetDelta());
}

void SwTxtFrm::_Format( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf,
                        const sal_Bool bAdjust )
{
    OSL_ENSURE( ! IsVertical() || IsSwapped(),"SwTxtFrm::_Format with unswapped frame" );

    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
    rLine.SetUnclipped( sal_False );

    // That was too complicated for the C30: aString( GetTxt() );
    const OUString &rString = GetTxtNode()->GetTxt();
    const xub_StrLen nStrLen = rString.getLength();

    SwCharRange &rReformat = *(pPara->GetReformat());
    SwRepaint   &rRepaint = *(pPara->GetRepaint());
    SwRepaint *pFreeze = NULL;

    // Due to performance reasons we set rReformat to STRING_LEN in Init()
    // In this case we adjust rReformat
    if( rReformat.Len() > nStrLen )
        rReformat.Len() = nStrLen;

    // Optimized:
    xub_StrLen nEnd = rReformat.Start() + rReformat.Len();
    if( nEnd > nStrLen )
    {
        rReformat.Len() = nStrLen - rReformat.Start();
        nEnd = nStrLen;
    }

    SwTwips nOldBottom;
    if( GetOfst() && !IsFollow() )
    {
        rLine.Bottom();
        nOldBottom = rLine.Y();
        rLine.Top();
    }
    else
        nOldBottom = 0;
    rLine.CharToLine( rReformat.Start() );

    // Words can be swapped-out when inserting a space into the
    // line that comes before the edited one. That's why we also
    // need to format that.
    // Optimization: If rReformat starts after the first word of the line
    // this line cannot possibly influence the previous one.
    // Unfortunately it can: Text size changes + FlyFrames.
    // The backlash can affect multiple lines (Frame!)!

    // #i46560#
    // FME: Yes, consider this case: (word) has to go to the next line
    // because) is a forbidden character at the beginning of a line although
    // (word would still fit on the previous line. Adding text right in front
    // of) would not trigger a reformatting of the previous line. Adding 1
    // to the result of FindBrk() does not solve the problem in all cases,
    // nevertheless it should be sufficient.
    bool bPrev = rLine.GetPrev() &&
                     ( FindBrk( rString, rLine.GetStart(), rReformat.Start() + 1 )
                       // #i46560#
                       + 1
                       >= rReformat.Start() ||
                       rLine.GetCurr()->IsRest() );
    if( bPrev )
    {
        while( rLine.Prev() )
            if( rLine.GetCurr()->GetLen() && !rLine.GetCurr()->IsRest() )
            {
                if( !rLine.GetStart() )
                    rLine.Top(); // So that NumDone doesn't get confused
                break;
            }
        xub_StrLen nNew = rLine.GetStart() + rLine.GetLength();
        if( nNew )
        {
            --nNew;
            if( CH_BREAK == rString[nNew] )
            {
                ++nNew;
                rLine.Next();
                bPrev = false;
            }
        }
        rReformat.Len()  += rReformat.Start() - nNew;
        rReformat.Start() = nNew;
    }

    rRepaint.SetOfst( 0 );
    rRepaint.SetRightOfst( 0 );
    rRepaint.Chg( Frm().Pos() + Prt().Pos(), Prt().SSize() );
    if( pPara->IsMargin() )
        rRepaint.Width( rRepaint.Width() + pPara->GetHangingMargin() );
    rRepaint.Top( rLine.Y() );
    if( 0 >= rRepaint.Width() )
        rRepaint.Width(1);
    WidowsAndOrphans aFrmBreak( this, rInf.IsTest() ? 1 : 0 );

    // rLine is now set to the first line which needs formatting.
    // The bFirst flag makes sure that Next() is not called.
    // The whole thing looks weird, but we need to make sure that
    // rLine stops at the last non-fitting line when calling IsBreakNow.
    bool bFirst  = true;
    sal_Bool bFormat = sal_True;

    // The CharToLine() can also get us into the danger zone.
    // In that case we need to walk back until rLine is set
    // to the non-fitting line. Or else the mass of text is lost,
    // because the Ofst was set wrongly in the Follow.

    sal_Bool bBreak = ( !pPara->IsPrepMustFit() || rLine.GetLineNr() > 1 )
                    && aFrmBreak.IsBreakNowWidAndOrp( rLine );
    if( bBreak )
    {
        bool bPrevDone = 0 != rLine.Prev();
        while( bPrevDone && aFrmBreak.IsBreakNowWidAndOrp(rLine) )
            bPrevDone = 0 != rLine.Prev();
        if( bPrevDone )
        {
            aFrmBreak.SetKeep( sal_False );
            rLine.Next();
        }
        rLine.TruncLines();

        // Play it safe
        bBreak = aFrmBreak.IsBreakNowWidAndOrp(rLine) &&
                  ( !pPara->IsPrepMustFit() || rLine.GetLineNr() > 1 );
    }

 /* Meaning if the following flags are set:

    Watch(End/Mid)Hyph: we need to format if we have a break at
    the line end/Fly, as long as MaxHyph is reached

    Jump(End/Mid)Flag: the next line which has no break (line end/Fly),
    needs to be formatted, because we could wrap now. This might have been
    forbidden earlier by MaxHyph

    Watch(End/Mid)Hyph: if the last formatted line got a cutoff point, but
    didn't have one before

    Jump(End/Mid)Hyph: if a cutoff point disappears
 */
    sal_Bool bJumpEndHyph  = sal_False,
         bWatchEndHyph = sal_False,
         bJumpMidHyph  = sal_False,
         bWatchMidHyph = sal_False;

    const SwAttrSet& rAttrSet = GetTxtNode()->GetSwAttrSet();
    bool bMaxHyph = ( 0 !=
        ( rInf.MaxHyph() = rAttrSet.GetHyphenZone().GetMaxHyphens() ) );
    if ( bMaxHyph )
        rLine.InitCntHyph();

    if( IsFollow() && IsFieldFollow() && rLine.GetStart() == GetOfst() )
    {
        SwTxtFrm *pMaster = FindMaster();
        OSL_ENSURE( pMaster, "SwTxtFrm::Format: homeless follow" );
        const SwLineLayout* pLine=NULL;
        if (pMaster)
        {
            if( !pMaster->HasPara() )
                pMaster->GetFormatted();
            SwTxtSizeInfo aInf( pMaster );
            SwTxtIter aMasterLine( pMaster, &aInf );
            aMasterLine.Bottom();
            pLine = aMasterLine.GetCurr();
        }
        SwLinePortion* pRest = pLine ?
            rLine.MakeRestPortion(pLine, GetOfst()) : NULL;
        if( pRest )
            rInf.SetRest( pRest );
        else
            SetFieldFollow( sal_False );
    }

    /* Ad cancel criterion:
     * In order to recognize, whether a line does not fit onto the page
     * anymore, we need to format it. This overflow is removed again in
     * e.g. AdjustFollow.
     * Another complication: if we are the Master, we need to traverse
     * the lines, because it could happen that one line can overflow
     * from the Follow to the Master.
     */
    do
    {
        if( bFirst )
            bFirst = false;
        else
        {
            if ( bMaxHyph )
            {
                if ( rLine.GetCurr()->IsEndHyph() )
                    rLine.CntEndHyph()++;
                else
                    rLine.CntEndHyph() = 0;
                if ( rLine.GetCurr()->IsMidHyph() )
                    rLine.CntMidHyph()++;
                else
                    rLine.CntMidHyph() = 0;
            }
            if( !rLine.Next() )
            {
                if( !bFormat )
                {
                    SwLinePortion* pRest =
                        rLine.MakeRestPortion( rLine.GetCurr(), rLine.GetEnd() );
                    if( pRest )
                        rInf.SetRest( pRest );
                }
                rLine.Insert( new SwLineLayout() );
                rLine.Next();
                bFormat = sal_True;
            }
        }
        if ( !bFormat && bMaxHyph &&
              (bWatchEndHyph || bJumpEndHyph || bWatchMidHyph || bJumpMidHyph) )
        {
            if ( rLine.GetCurr()->IsEndHyph() )
            {
                if ( bWatchEndHyph )
                    bFormat = ( rLine.CntEndHyph() == rInf.MaxHyph() );
            }
            else
            {
                bFormat = bJumpEndHyph;
                bWatchEndHyph = sal_False;
                bJumpEndHyph = sal_False;
            }
            if ( rLine.GetCurr()->IsMidHyph() )
            {
                if ( bWatchMidHyph && !bFormat )
                    bFormat = ( rLine.CntEndHyph() == rInf.MaxHyph() );
            }
            else
            {
                bFormat = bFormat || bJumpMidHyph;
                bWatchMidHyph = sal_False;
                bJumpMidHyph = sal_False;
            }
        }
        if( bFormat )
        {
            sal_Bool bOldEndHyph = rLine.GetCurr()->IsEndHyph();
            sal_Bool bOldMidHyph = rLine.GetCurr()->IsMidHyph();
            bFormat = FormatLine( rLine, bPrev );
            // There can only be one bPrev ... (???)
            bPrev = sal_False;
            if ( bMaxHyph )
            {
                if ( rLine.GetCurr()->IsEndHyph() != bOldEndHyph )
                {
                    bWatchEndHyph = !bOldEndHyph;
                    bJumpEndHyph = bOldEndHyph;
                }
                if ( rLine.GetCurr()->IsMidHyph() != bOldMidHyph )
                {
                    bWatchMidHyph = !bOldMidHyph;
                    bJumpMidHyph = bOldMidHyph;
                }
            }
        }

        if( !rInf.IsNewLine() )
        {
            if( !bFormat )
                 bFormat = 0 != rInf.GetRest();
            if( rInf.IsStop() || rInf.GetIdx() >= nStrLen )
                break;
            if( !bFormat && ( !bMaxHyph || ( !bWatchEndHyph &&
                    !bJumpEndHyph && !bWatchMidHyph && !bJumpMidHyph ) ) )
            {
                if( GetFollow() )
                {
                    while( rLine.Next() )
                        ; //Nothing
                    pFreeze = new SwRepaint( rRepaint ); // to minimize painting
                }
                else
                    break;
            }
        }
        bBreak = aFrmBreak.IsBreakNowWidAndOrp(rLine);
    }while( !bBreak );

    if( pFreeze )
    {
        rRepaint = *pFreeze;
        delete pFreeze;
    }

    if( !rLine.IsStop() )
    {
        // If we're finished formatting the text and we still
        // have other line objects left, these are superfluous
        // now because the text has gotten shorter.
        if( rLine.GetStart() + rLine.GetLength() >= nStrLen &&
            rLine.GetCurr()->GetNext() )
        {
            rLine.TruncLines();
            rLine.SetTruncLines( sal_True );
        }
    }

    if( !rInf.IsTest() )
    {
        // FormatAdjust does not pay off at OnceMore
        if( bAdjust || !rLine.GetDropFmt() || !rLine.CalcOnceMore() )
        {
            FormatAdjust( rLine, aFrmBreak, nStrLen, rInf.IsStop() );
        }
        if( rRepaint.HasArea() )
            SetRepaint();
        rLine.SetTruncLines( sal_False );
        if( nOldBottom ) // We check whether paragraphs that need scrolling can
                         // be shrunk, so that they don't need scrolling anymore
        {
            rLine.Bottom();
            SwTwips nNewBottom = rLine.Y();
            if( nNewBottom < nOldBottom )
                _SetOfst( 0 );
        }
    }
}

void SwTxtFrm::FormatOnceMore( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf )
{
    OSL_ENSURE( ! IsVertical() || IsSwapped(),
            "A frame is not swapped in SwTxtFrm::FormatOnceMore" );

    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
    if( !pPara )
        return;

    // If necessary the pPara
    KSHORT nOld  = ((const SwTxtMargin&)rLine).GetDropHeight();
    sal_Bool bShrink = sal_False,
         bGrow   = sal_False,
         bGoOn   = rLine.IsOnceMore();
    sal_uInt8 nGo    = 0;
    while( bGoOn )
    {
        ++nGo;
        rInf.Init();
        rLine.Top();
        if( !rLine.GetDropFmt() )
            rLine.SetOnceMore( sal_False );
        SwCharRange aRange( 0, rInf.GetTxt().getLength() );
        *(pPara->GetReformat()) = aRange;
        _Format( rLine, rInf );

        bGoOn = rLine.IsOnceMore();
        if( bGoOn )
        {
            const KSHORT nNew = ((const SwTxtMargin&)rLine).GetDropHeight();
            if( nOld == nNew )
                bGoOn = sal_False;
            else
            {
                if( nOld > nNew )
                    bShrink = sal_True;
                else
                    bGrow = sal_True;

                if( bShrink == bGrow || 5 < nGo )
                    bGoOn = sal_False;

                nOld = nNew;
            }

            // If something went wrong, we need to reformat again
            if( !bGoOn )
            {
                rInf.CtorInitTxtFormatInfo( this );
                rLine.CtorInitTxtFormatter( this, &rInf );
                rLine.SetDropLines( 1 );
                rLine.CalcDropHeight( 1 );
                SwCharRange aTmpRange( 0, rInf.GetTxt().getLength() );
                *(pPara->GetReformat()) = aTmpRange;
                _Format( rLine, rInf, sal_True );
                // We paint everything ...
                SetCompletePaint();
            }
        }
    }
}

void SwTxtFrm::_Format( SwParaPortion *pPara )
{
    const bool bIsEmpty = GetTxt().isEmpty();

    if ( bIsEmpty )
    {
        // Empty lines do not get tortured for very long:
        // pPara is cleared, which is the same as:
        // *pPara = SwParaPortion;
        sal_Bool bMustFit = pPara->IsPrepMustFit();
        pPara->Truncate();
        pPara->FormatReset();
        if( pBlink && pPara->IsBlinking() )
            pBlink->Delete( pPara );

        // delete pSpaceAdd und pKanaComp
        pPara->FinishSpaceAdd();
        pPara->FinishKanaComp();
        pPara->ResetFlags();
        pPara->SetPrepMustFit( bMustFit );
    }

    OSL_ENSURE( ! IsSwapped(), "A frame is swapped before _Format" );

    if ( IsVertical() )
        SwapWidthAndHeight();

    SwTxtFormatInfo aInf( this );
    SwTxtFormatter  aLine( this, &aInf );

    HideAndShowObjects();

    _Format( aLine, aInf );

    if( aLine.IsOnceMore() )
        FormatOnceMore( aLine, aInf );

    if ( IsVertical() )
        SwapWidthAndHeight();

    OSL_ENSURE( ! IsSwapped(), "A frame is swapped after _Format" );

    if( 1 < aLine.GetDropLines() )
    {
        if( SVX_ADJUST_LEFT != aLine.GetAdjust() &&
            SVX_ADJUST_BLOCK != aLine.GetAdjust() )
        {
            aLine.CalcDropAdjust();
            aLine.SetPaintDrop( sal_True );
        }

        if( aLine.IsPaintDrop() )
        {
            aLine.CalcDropRepaint();
            aLine.SetPaintDrop( sal_False );
        }
    }
}

/*************************************************************************
 * SwTxtFrm::Format()
 * We calculate the text frame's size and send a notification.
 * Shrink() or Grow() to adjust the frame's size to the changed required space.
 *************************************************************************/

void SwTxtFrm::Format( const SwBorderAttrs * )
{
    SWRECTFN( this )

    CalcAdditionalFirstLineOffset();

    // The range autopilot or the BASIC interface pass us TxtFrms with
    // a width <= 0 from time to time
    if( (Prt().*fnRect->fnGetWidth)() <= 0 )
    {
        // If MustFit is set, we shrink to the Upper's bottom edge if needed.
        // Else we just take a standard size of 12 Pt. (240 twip).
        SwTxtLineAccess aAccess( this );
        long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
        if( aAccess.GetPara()->IsPrepMustFit() )
        {
            const SwTwips nLimit = (GetUpper()->*fnRect->fnGetPrtBottom)();
            const SwTwips nDiff = - (Frm().*fnRect->fnBottomDist)( nLimit );
            if( nDiff > 0 )
                Shrink( nDiff );
        }
        else if( 240 < nFrmHeight )
            Shrink( nFrmHeight - 240 );
        else if( 240 > nFrmHeight )
            Grow( 240 - nFrmHeight );
        nFrmHeight = (Frm().*fnRect->fnGetHeight)();

        long nTop = (this->*fnRect->fnGetTopMargin)();
        if( nTop > nFrmHeight )
            (this->*fnRect->fnSetYMargins)( nFrmHeight, 0 );
        else if( (Prt().*fnRect->fnGetHeight)() < 0 )
            (Prt().*fnRect->fnSetHeight)( 0 );
        return;
    }

    const xub_StrLen nStrLen = GetTxtNode()->GetTxt().getLength();
    if ( nStrLen || !FormatEmpty() )
    {

        SetEmpty( sal_False );
        // In order to not get confused by nested Formats
        FormatLevel aLevel;
        if( 12 == aLevel.GetLevel() )
            return;

        // We could be possibly not allowed to alter the format information
        if( IsLocked() )
            return;

        // Attention: Format() could be triggered by GetFormatted()
        if( IsHiddenNow() )
        {
            long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
            if( nPrtHeight )
            {
                HideHidden();
                Shrink( nPrtHeight );
            }
            else
            {
                // Assure that objects anchored
                // at paragraph resp. at/as character inside paragraph
                // are hidden.
                HideAndShowObjects();
            }
            ChgThisLines();
            return;
        }

        // We do not want to be interrupted during formatting
        SwTxtFrmLocker aLock(this);
        SwTxtLineAccess aAccess( this );
        const bool bNew = !aAccess.SwTxtLineAccess::IsAvailable();
        const bool bSetOfst =
            (GetOfst() && GetOfst() > GetTxtNode()->GetTxt().getLength());

        if( CalcPreps() )
            ; // nothing
        // We return if already formatted, but if the TxtFrm was just created
        // and does not have any format information
        else if( !bNew && !aAccess.GetPara()->GetReformat()->Len() )
        {
            if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue() )
            {
                aAccess.GetPara()->SetPrepAdjust( sal_True );
                aAccess.GetPara()->SetPrep( sal_True );
                CalcPreps();
            }
            SetWidow( sal_False );
        }
        else if( bSetOfst && IsFollow() )
        {
            SwTxtFrm *pMaster = FindMaster();
            OSL_ENSURE( pMaster, "SwTxtFrm::Format: homeless follow" );
            if( pMaster )
                pMaster->Prepare( PREP_FOLLOW_FOLLOWS );
            SwTwips nMaxY = (GetUpper()->*fnRect->fnGetPrtBottom)();
            if( (Frm().*fnRect->fnOverStep)( nMaxY  ) )
                (this->*fnRect->fnSetLimit)( nMaxY );
            else if( (Frm().*fnRect->fnBottomDist)( nMaxY  ) < 0 )
                (Frm().*fnRect->fnAddBottom)( -(Frm().*fnRect->fnGetHeight)() );
        }
        else
        {
            // bSetOfst here means that we have the "red arrow situation"
            if ( bSetOfst )
                _SetOfst( 0 );

            const sal_Bool bOrphan = IsWidow();
            const SwFtnBossFrm* pFtnBoss = HasFtn() ? FindFtnBossFrm() : 0;
            SwTwips nFtnHeight = 0;
            if( pFtnBoss )
            {
                const SwFtnContFrm* pCont = pFtnBoss->FindFtnCont();
                nFtnHeight = pCont ? (pCont->Frm().*fnRect->fnGetHeight)() : 0;
            }
            do
            {
                _Format( aAccess.GetPara() );
                if( pFtnBoss && nFtnHeight )
                {
                    const SwFtnContFrm* pCont = pFtnBoss->FindFtnCont();
                    SwTwips nNewHeight = pCont ? (pCont->Frm().*fnRect->fnGetHeight)() : 0;
                    // If we lost some footnotes, we may have more space
                    // for our main text, so we have to format again ...
                    if( nNewHeight < nFtnHeight )
                        nFtnHeight = nNewHeight;
                    else
                        break;
                }
                else
                    break;
            } while ( pFtnBoss );
            if( bOrphan )
            {
                ValidateFrm();
                SetWidow( sal_False );
            }
        }
        if( IsEmptyMaster() )
        {
            SwFrm* pPre = GetPrev();
            if( pPre &&
                // #i10826# It's the first, it cannot keep!
                pPre->GetIndPrev() &&
                pPre->GetAttrSet()->GetKeep().GetValue() )
            {
                pPre->InvalidatePos();
            }
        }
    }

    ChgThisLines();

    // the PrepMustFit should not survive a Format operation
    SwParaPortion *pPara = GetPara();
    if ( pPara )
           pPara->SetPrepMustFit( sal_False );

    CalcBaseOfstForFly();
    _CalcHeightOfLastLine(); // #i11860#
}

/*************************************************************************
 * SwTxtFrm::FormatQuick()
 *
 * bForceQuickFormat is set if GetFormatted() has been called during the
 * painting process. Actually I cannot imagine a situation which requires
 * a full formatting of the paragraph during painting, on the other hand
 * a full formatting can cause the invalidation of other layout frames,
 * e.g., if there are footnotes in this paragraph, and invalid layout
 * frames will not calculated during the painting. So I actually want to
 * avoid a formatting during painting, but since I'm a coward, I'll only
 * force the quick formatting in the situation of issue i29062.
 *************************************************************************/

sal_Bool SwTxtFrm::FormatQuick( bool bForceQuickFormat )
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTxtFrm::FormatQuick with swapped frame" );

#if OSL_DEBUG_LEVEL > 1
    const OUString aXXX = GetTxtNode()->GetTxt();
    const SwTwips nDbgY = Frm().Top();
    (void)nDbgY;
    // nStopAt allows CV to alter it
    static MSHORT nStopAt = 0;
    if( nStopAt == GetFrmId() )
    {
        int i = GetFrmId();
        (void)i;
    }
#endif

    if( IsEmpty() && FormatEmpty() )
        return sal_True;

    // We're very picky:
    if( HasPara() || IsWidow() || IsLocked()
        || !GetValidSizeFlag() ||
        ( ( IsVertical() ? Prt().Width() : Prt().Height() ) && IsHiddenNow() ) )
        return sal_False;

    SwTxtLineAccess aAccess( this );
    SwParaPortion *pPara = aAccess.GetPara();
    if( !pPara )
        return sal_False;

    SwFrmSwapper aSwapper( this, sal_True );

    SwTxtFrmLocker aLock(this);
    SwTxtFormatInfo aInf( this, sal_False, sal_True );
    if( 0 != aInf.MaxHyph() )   // Respect MaxHyphen!
        return sal_False;

    SwTxtFormatter  aLine( this, &aInf );

    // DropCaps are too complicated ...
    if( aLine.GetDropFmt() )
        return sal_False;

    xub_StrLen nStart = GetOfst();
    const xub_StrLen nEnd = GetFollow()
                      ? GetFollow()->GetOfst() : aInf.GetTxt().getLength();
    do
    {
        nStart = aLine.FormatLine( nStart );
        if( aInf.IsNewLine() || (!aInf.IsStop() && nStart < nEnd) )
            aLine.Insert( new SwLineLayout() );
    } while( aLine.Next() );

    // Last exit: the heights need to match
    Point aTopLeft( Frm().Pos() );
    aTopLeft += Prt().Pos();
    const SwTwips nNewHeight = aLine.Y() + aLine.GetLineHeight();
    const SwTwips nOldHeight = aTopLeft.Y() + Prt().Height();

    if( !bForceQuickFormat && nNewHeight != nOldHeight && !IsUndersized() )
    {
        // Attention: This situation can occur due to FormatLevel==12. Don't panic!
        const xub_StrLen nStrt = GetOfst();
        _InvalidateRange( SwCharRange( nStrt, nEnd - nStrt) );
        return sal_False;
    }

    if (m_pFollow && nStart != (static_cast<SwTxtFrm*>(m_pFollow))->GetOfst())
        return sal_False; // can be caused by e.g. Orphans

    // We made it!

    // Set repaint
    pPara->GetRepaint()->Pos( aTopLeft );
    pPara->GetRepaint()->SSize( Prt().SSize() );

    // Delete reformat
    *(pPara->GetReformat()) = SwCharRange();
    *(pPara->GetDelta()) = 0;

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
