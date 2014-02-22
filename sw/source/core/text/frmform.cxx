/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <hintids.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <pagefrm.hxx>
#include <ndtxt.hxx>
#include <dcontact.hxx>
#include <dflyobj.hxx>
#include <flyfrm.hxx>
#include <ftnfrm.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <paratr.hxx>
#include <viewopt.hxx>
#include <viewsh.hxx>
#include <frmatr.hxx>
#include <pam.hxx>
#include <flyfrms.hxx>
#include <fmtanchr.hxx>
#include <itrform2.hxx>
#include <widorp.hxx>
#include <txtcache.hxx>
#include <porrst.hxx>
#include <blink.hxx>
#include <porfld.hxx>
#include <sectfrm.hxx>
#include <pormulti.hxx>

#include <rootfrm.hxx>
#include <frmfmt.hxx>

#include <sortedobjs.hxx>
#include <portab.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>


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

void ValidateTxt( SwFrm *pFrm )     
{
    if ( ( ! pFrm->IsVertical() &&
             pFrm->Frm().Width() == pFrm->GetUpper()->Prt().Width() ) ||
         (   pFrm->IsVertical() &&
             pFrm->Frm().Height() == pFrm->GetUpper()->Prt().Height() ) )
        pFrm->mbValidSize = sal_True;
}

void SwTxtFrm::ValidateFrm()
{
    
    SWAP_IF_SWAPPED( this )

    if ( !IsInFly() && !IsInTab() )
    {   
        
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

    
    OSL_ENSURE( HasPara(), "ResetPreps(), missing ParaPortion." );
    SwParaPortion *pPara = GetPara();
    const bool bMustFit = pPara->IsPrepMustFit();
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
            ((SwSectionFrm*)pFrm)->SetCntntLock( true );
            pFrm->Calc();
            if( !bOld )
                ((SwSectionFrm*)pFrm)->SetCntntLock( false );
        }
    }
}

void SwTxtFrm::ValidateBodyFrm()
{
    SWAP_IF_SWAPPED( this )

     
    if ( !IsInFly() && !IsInTab() &&
         !( IsInSct() && FindSctFrm()->Lower()->IsColumnFrm() ) )
        _ValidateBodyFrm( GetUpper() );

    UNDO_SWAP( this )
}

bool SwTxtFrm::_GetDropRect( SwRect &rRect ) const
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
        return true;
    }

    UNDO_SWAP( this )

    return false;
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

bool SwTxtFrm::CalcFollow( const sal_Int32 nTxtOfst )
{
    SWAP_IF_SWAPPED( this )

    OSL_ENSURE( HasFollow(), "CalcFollow: missing Follow." );

    SwTxtFrm* pMyFollow = GetFollow();

    SwParaPortion *pPara = GetPara();
    const bool bFollowFld = pPara && pPara->IsFollowField();

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
        bool bOldInvaCntnt = true;
        if ( !IsInFly() && GetNext() )
        {
            pPage = FindPageFrm();
            
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

        
        SwSaveFtnHeight aSave( FindFtnBossFrm( true ), LONG_MAX );

        pMyFollow->CalcFtnFlag();
        if ( !pMyFollow->GetNext() && !pMyFollow->HasFtn() )
            nOldBottom = bVert ? 0 : LONG_MAX;

        while( true )
        {
            if( !FormatLevel::LastLevel() )
            {
                
                
                
                
                SwSectionFrm* pSct = pMyFollow->FindSctFrm();
                if( pSct && !pSct->IsAnLower( this ) )
                {
                    if( pSct->GetFollow() )
                        pSct->SimpleFormat();
                    else if( ( pSct->IsVertical() && !pSct->Frm().Width() ) ||
                             ( ! pSct->IsVertical() && !pSct->Frm().Height() ) )
                        break;
                }
                
                if ( FollowFormatAllowed() )
                {
                    
                    
                    
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
                    
                    OSL_ENSURE( !pMyFollow->GetPrev(), "SwTxtFrm::CalcFollow: cheesy follow" );
                    if( pMyFollow->GetPrev() )
                    {
                        pMyFollow->Prepare( PREP_CLEAR );
                        pMyFollow->Calc();
                        OSL_ENSURE( !pMyFollow->GetPrev(), "SwTxtFrm::CalcFollow: very cheesy follow" );
                    }

                    
                    pMyFollow->AllowFollowFormat();
                }

                
                pMyFollow->SetCompletePaint();
            }

            pPara = GetPara();
            
            
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
            return true;
        }
    }

    UNDO_SWAP( this )

    return false;
}

void SwTxtFrm::AdjustFrm( const SwTwips nChgHght, bool bHasToFit )
{
    if( IsUndersized() )
    {
        if( GetOfst() && !IsFollow() ) 
            return;
        SetUndersized( nChgHght == 0 || bHasToFit );
    }

    
    
    SWAP_IF_SWAPPED( this )
    SWRECTFN ( this )

    
    
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
                    
                    
                    InvalidateObjs( true );
                }
            }
            nChgHeight = 0;
        }
        
        
        
        
        
        SwTwips nRstHeight;
        if ( IsVertical() )
        {
            OSL_ENSURE( ! IsSwapped(),"Swapped frame while calculating nRstHeight" );

            
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
            
            if( ( nRstHeight >= 0 || ( IsInFtn() && IsInSct() ) ) && !bHasToFit )
                nRstHeight += GetUpper()->Grow( nFrmHeight - nRstHeight );
            
            
            
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

com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop > SwTxtFrm::GetTabStopInfo( SwTwips CurrentPos )
{
    com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop > tabs(1);
    ::com::sun::star::style::TabStop ts;

    SwTxtFormatInfo     aInf( this );
    SwTxtFormatter      aLine( this, &aInf );
    SwTxtCursor         TxtCursor( this, &aInf );
    const Point aCharPos( TxtCursor.GetTopLeft() );


    SwTwips nRight = aLine.Right();
    CurrentPos -= aCharPos.X();

    
    const SvxTabStop *pTS = aLine.GetLineInfo().GetTabStop( CurrentPos, nRight );

    if( !pTS )
    {
        return com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop >();
    }

    
    ts.Position = pTS->GetTabPos();
    ts.DecimalChar = pTS->GetDecimal();
    ts.FillChar = pTS->GetFill();
    switch( pTS->GetAdjustment() )
    {
    case SVX_TAB_ADJUST_LEFT   : ts.Alignment = ::com::sun::star::style::TabAlign_LEFT; break;
    case SVX_TAB_ADJUST_CENTER : ts.Alignment = ::com::sun::star::style::TabAlign_CENTER; break;
    case SVX_TAB_ADJUST_RIGHT  : ts.Alignment = ::com::sun::star::style::TabAlign_RIGHT; break;
    case SVX_TAB_ADJUST_DECIMAL: ts.Alignment = ::com::sun::star::style::TabAlign_DECIMAL; break;
    case SVX_TAB_ADJUST_DEFAULT: ts.Alignment = ::com::sun::star::style::TabAlign_DEFAULT; break;
    default: break; 
    }

    tabs[0] = ts;
    return tabs;
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
                             const sal_Int32 nOffset, const sal_Int32 nEnd,
                             const sal_uInt8 nMode )
{
    SwFrmSwapper aSwapper( this, false );

    
    
    
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

    
    
    
    const sal_Int32 nNewOfst = ( IsInFtn() && ( !GetIndNext() || HasFollow() ) ) ?
                            rLine.FormatQuoVadis(nOffset) : nOffset;

    if( !(nMode & 1) )
    {
        
        
        while( GetFollow() && GetFollow()->GetFollow() &&
               nNewOfst >= GetFollow()->GetFollow()->GetOfst() )
        {
            JoinFrm();
        }
    }

    
    if( GetFollow() )
    {
#if OSL_DEBUG_LEVEL > 1
        static bool bTest = false;
        if( !bTest || ( nMode & 1 ) )
#endif
        if ( nMode )
            GetFollow()->ManipOfst( 0 );

        if ( CalcFollow( nNewOfst ) )   
            rLine.SetOnceMore( true );
    }
}

SwCntntFrm *SwTxtFrm::JoinFrm()
{
    OSL_ENSURE( GetFollow(), "+SwTxtFrm::JoinFrm: no follow" );
    SwTxtFrm  *pFoll = GetFollow();

    SwTxtFrm *pNxt = pFoll->GetFollow();

    
    sal_Int32 nStart = pFoll->GetOfst();
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
                    SetFtn( true );
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

    pFoll->MoveFlyInCnt( this, nStart, COMPLETE_STRING );
    pFoll->SetFtn( false );
    
    
    
    
    
    {
        SwViewShell* pViewShell( pFoll->getRootFrm()->GetCurrShell() );
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

SwCntntFrm *SwTxtFrm::SplitFrm( const sal_Int32 nTxtPos )
{
    SWAP_IF_SWAPPED( this )

    
    
    SwTxtFrmLocker aLock( this );
    SwTxtFrm *pNew = (SwTxtFrm *)(GetTxtNode()->MakeFrm( this ));

    pNew->SetFollow( GetFollow() );
    SetFollow( pNew );

    pNew->Paste( GetUpper(), GetNext() );
    
    
    
    
    
    {
        SwViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
        if ( pViewShell && pViewShell->GetLayout() &&
             pViewShell->GetLayout()->IsAnyShellAccessible() )
        {
            pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pNew->FindNextCnt( true )),
                            this );
        }
    }

    
    
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
                    pNew->SetFtn( true );
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

    MoveFlyInCnt( pNew, nTxtPos, COMPLETE_STRING );

    

    pNew->ManipOfst( nTxtPos );

    UNDO_SWAP( this )
    return pNew;
}

void SwTxtFrm::_SetOfst( const sal_Int32 nNewOfst )
{
    
    
    
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

bool SwTxtFrm::CalcPreps()
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(), "SwTxtFrm::CalcPreps with swapped frame" );
    SWRECTFN( this );

    SwParaPortion *pPara = GetPara();
    if ( !pPara )
        return false;
    const bool bPrep = pPara->IsPrep();
    const bool bPrepWidows = pPara->IsPrepWidows();
    const bool bPrepAdjust = pPara->IsPrepAdjust();
    const bool bPrepMustFit = pPara->IsPrepMustFit();
    ResetPreps();

    bool bRet = false;
    if( bPrep && !pPara->GetReformat()->Len() )
    {
        
        
        if( bPrepWidows )
        {
            if( !GetFollow() )
            {
                OSL_ENSURE( GetFollow(), "+SwTxtFrm::CalcPreps: no credits" );
                return false;
            }

            
            
            
            
            

            SwTwips nChgHeight = GetParHeight();
            if( nChgHeight >= (Prt().*fnRect->fnGetHeight)() )
            {
                if( bPrepMustFit )
                {
                    GetFollow()->SetJustWidow( true );
                    GetFollow()->Prepare( PREP_CLEAR );
                }
                else if ( bVert )
                {
                    Frm().Width( Frm().Width() + Frm().Left() );
                    Prt().Width( Prt().Width() + Frm().Left() );
                    Frm().Left( 0 );
                    SetWidow( true );
                }
                else
                {
                    SwTwips nTmp  = LONG_MAX - (Frm().Top()+10000);
                    SwTwips nDiff = nTmp - Frm().Height();
                    Frm().Height( nTmp );
                    Prt().Height( Prt().Height() + nDiff );
                    SetWidow( true );
                }
            }
            else
            {
                OSL_ENSURE( nChgHeight < (Prt().*fnRect->fnGetHeight)(),
                        "+SwTxtFrm::CalcPrep: wanna shrink" );

                nChgHeight = (Prt().*fnRect->fnGetHeight)() - nChgHeight;

                GetFollow()->SetJustWidow( true );
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
            bRet = true;
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
                        aAccess.GetPara()->SetPrepMustFit();
                    }
                    return false;
                }
            }

            SWAP_IF_NOT_SWAPPED( this )

            SwTxtFormatInfo aInf( this );
            SwTxtFormatter aLine( this, &aInf );

            WidowsAndOrphans aFrmBreak( this );
            
            
            if( bPrepMustFit )
            {
                aFrmBreak.SetKeep( false );
                aFrmBreak.ClrOrphLines();
            }
            
            
            
            bool bBreak = aFrmBreak.IsBreakNowWidAndOrp( aLine );
            bRet = true;
            while( !bBreak && aLine.Next() )
            {
                bBreak = aFrmBreak.IsBreakNowWidAndOrp( aLine );
            }
            if( bBreak )
            {
                
                
                
                
                
                
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
                    
                    
                    const SwCharRange aFollowRg( GetFollow()->GetOfst(), 1 );
                    *(pPara->GetReformat()) += aFollowRg;
                    
                    bRet = false;
                }
            }

            UNDO_SWAP( this )
            
            
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
            MoveFlyInCnt( pFrm, nNew, COMPLETE_STRING );\
    }

void SwTxtFrm::FormatAdjust( SwTxtFormatter &rLine,
                             WidowsAndOrphans &rFrmBreak,
                             const sal_Int32 nStrLen,
                             const bool bDummy )
{
    SWAP_IF_NOT_SWAPPED( this )

    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();

    sal_Int32 nEnd = rLine.GetStart();

    const bool bHasToFit = pPara->IsPrepMustFit();

    
    
    
    
    sal_uInt8 nNew = ( !GetFollow() &&
                       nEnd < nStrLen &&
                       ( rLine.IsStop() ||
                         ( bHasToFit
                           ? ( rLine.GetLineNr() > 1 &&
                               !rFrmBreak.IsInside( rLine ) )
                           : rFrmBreak.IsBreakNow( rLine ) ) ) )
                     ? 1 : 0;
    
    
    const bool bOnlyContainsAsCharAnchoredObj =
            !IsFollow() && nStrLen == 1 &&
            GetDrawObjs() && GetDrawObjs()->Count() == 1 &&
            (*GetDrawObjs())[0]->GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AS_CHAR;
    if ( nNew && bOnlyContainsAsCharAnchoredObj )
    {
        nNew = 0;
    }
    
    if ( nNew )
    {
        SplitFrm( nEnd );
    }

    const SwFrm *pBodyFrm = (const SwFrm*)(FindBodyFrm());

    const long nBodyHeight = pBodyFrm ? ( IsVertical() ?
                                          pBodyFrm->Frm().Width() :
                                          pBodyFrm->Frm().Height() ) : 0;

    
    
    *(pPara->GetReformat()) = SwCharRange();
    bool bDelta = *pPara->GetDelta() != 0;
    *(pPara->GetDelta()) = 0;

    if( rLine.IsStop() )
    {
        rLine.TruncLines( true );
        nNew = 1;
    }

    
    if( !rFrmBreak.FindBreak( this, rLine, bHasToFit ) )
    {
        
        
        
        sal_Int32 nOld = nEnd;
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
    {   
        
        
        
        nEnd = rLine.GetEnd();
        if( GetFollow() )
        {
            
            
            
            
            
            
            
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
            
            
            
            
            if ( !bOnlyContainsAsCharAnchoredObj &&
                 ( nStrLen > 0 ||
                   ( nStrLen == 0 && GetTxtNode()->GetNumRule() ) )
               )
            {
                SplitFrm( nEnd );
                nNew |= 3;
            }
        }
        
        
        if( bDummy && pBodyFrm &&
           nBodyHeight < ( IsVertical() ?
                           pBodyFrm->Frm().Width() :
                           pBodyFrm->Frm().Height() ) )
            rLine.MakeDummyLine();
    }

    
    

    const SwTwips nDocPrtTop = Frm().Top() + Prt().Top();
    const SwTwips nOldHeight = Prt().SSize().Height();
    SwTwips nChg = rLine.CalcBottomLine() - nDocPrtTop - nOldHeight;
    
    
    if ( nChg < 0 &&
         bOnlyContainsAsCharAnchoredObj )
    {
        nChg = 0;
    }
    

    
    
    
    
    

    
    if ( IsVertical() && !IsVertLR() && nChg )
    {
        SwRect &rRepaint = *(pPara->GetRepaint());
        rRepaint.Left( rRepaint.Left() - nChg );
        rRepaint.Width( rRepaint.Width() - nChg );
    }

    AdjustFrm( nChg, bHasToFit );

    if( HasFollow() || IsInFtn() )
        _AdjustFollow( rLine, nEnd, nStrLen, nNew );

    pPara->SetPrepMustFit( false );

    UNDO_SWAP( this )
}

/*************************************************************************
 * SwTxtFrm::FormatLine()
 * bPrev is set whether Reformat.Start() was called because of Prev().
 * Else, wo don't know whether we can limit the repaint or not.
*************************************************************************/

bool SwTxtFrm::FormatLine( SwTxtFormatter &rLine, const bool bPrev )
{
    OSL_ENSURE( ! IsVertical() || IsSwapped(),
            "SwTxtFrm::FormatLine( rLine, bPrev) with unswapped frame" );
    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
    const SwLineLayout *pOldCur = rLine.GetCurr();
    const sal_Int32 nOldLen    = pOldCur->GetLen();
    const KSHORT nOldAscent = pOldCur->GetAscent();
    const KSHORT nOldHeight = pOldCur->Height();
    const SwTwips nOldWidth = pOldCur->Width() + pOldCur->GetHangingMargin();
    const bool bOldHyph = pOldCur->IsEndHyph();
    SwTwips nOldTop = 0;
    SwTwips nOldBottom = 0;
    if( rLine.GetCurr()->IsClipping() )
        rLine.CalcUnclipped( nOldTop, nOldBottom );

    const sal_Int32 nNewStart = rLine.FormatLine( rLine.GetStart() );

    OSL_ENSURE( Frm().Pos().Y() + Prt().Pos().Y() == rLine.GetFirstPos(),
            "SwTxtFrm::FormatLine: frame leaves orbit." );
    OSL_ENSURE( rLine.GetCurr()->Height(),
            "SwTxtFrm::FormatLine: line height is zero" );

    
    const SwLineLayout *pNew = rLine.GetCurr();

    bool bUnChg = nOldLen == pNew->GetLen() &&
                  bOldHyph == pNew->IsEndHyph();
    if ( bUnChg && !bPrev )
    {
        const long nWidthDiff = nOldWidth > pNew->Width()
                                ? nOldWidth - pNew->Width()
                                : pNew->Width() - nOldWidth;

        
        
        bUnChg = nOldHeight == pNew->Height() &&
                 nOldAscent == pNew->GetAscent() &&
                 nWidthDiff <= SLOPPY_TWIPS &&
                 pOldCur->GetNext();
    }

    
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
                rLine.SetUnclipped( true );
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
                rLine.SetUnclipped( true );
            }
        }
        else
        {
            if( !rLine.IsUnclipped() || nBottom > rRepaint.Bottom() )
            {
                rRepaint.Bottom( nBottom - 1 );
                rLine.SetUnclipped( false );
            }
        }
        SwTwips nRght = std::max( nOldWidth, pNew->Width() +
                             pNew->GetHangingMargin() );
        SwViewShell *pSh = getRootFrm()->GetCurrShell();
        const SwViewOption *pOpt = pSh ? pSh->GetViewOptions() : 0;
        if( pOpt && (pOpt->IsParagraph() || pOpt->IsLineBreak()) )
            nRght += ( std::max( nOldAscent, pNew->GetAscent() ) );
        else
            nRght += ( std::max( nOldAscent, pNew->GetAscent() ) / 4);
        nRght += rLine.GetLeftMargin();
        if( rRepaint.GetOfst() || rRepaint.GetRightOfst() < nRght )
            rRepaint.SetRightOfst( nRght );

        
        
        const bool bHasUnderscore =
                ( rLine.GetInfo().GetUnderScorePos() < nNewStart );
        if ( bHasUnderscore || rLine.GetCurr()->HasUnderscore() )
            rRepaint.Bottom( rRepaint.Bottom() + 40 );

        ((SwLineLayout*)rLine.GetCurr())->SetUnderscore( bHasUnderscore );
    }
    if( !bUnChg )
        rLine.SetChanges();

    
    *(pPara->GetDelta()) -= long(pNew->GetLen()) - long(nOldLen);

    
    if( rLine.IsStop() )
        return false;

    
    if( rLine.IsNewLine() )
        return true;

    
    if (nNewStart >= GetTxtNode()->GetTxt().getLength())
        return false;

    if( rLine.GetInfo().IsShift() )
        return true;

    
    const sal_Int32 nEnd = pPara->GetReformat()->Start() +
                        pPara->GetReformat()->Len();

    if( nNewStart <= nEnd )
        return true;

    return 0 != *(pPara->GetDelta());
}

void SwTxtFrm::_Format( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf,
                        const bool bAdjust )
{
    OSL_ENSURE( ! IsVertical() || IsSwapped(),"SwTxtFrm::_Format with unswapped frame" );

    SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
    rLine.SetUnclipped( false );

    
    const OUString &rString = GetTxtNode()->GetTxt();
    const sal_Int32 nStrLen = rString.getLength();

    SwCharRange &rReformat = *(pPara->GetReformat());
    SwRepaint   &rRepaint = *(pPara->GetRepaint());
    SwRepaint *pFreeze = NULL;

    
    
    if( rReformat.Len() > nStrLen )
        rReformat.Len() = nStrLen;

    
    sal_Int32 nEnd = rReformat.Start() + rReformat.Len();
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

    
    
    
    
    
    
    

    
    
    
    
    
    
    
    bool bPrev = rLine.GetPrev() &&
                     ( FindBrk( rString, rLine.GetStart(), rReformat.Start() + 1 )
                       
                       + 1
                       >= rReformat.Start() ||
                       rLine.GetCurr()->IsRest() );
    if( bPrev )
    {
        while( rLine.Prev() )
            if( rLine.GetCurr()->GetLen() && !rLine.GetCurr()->IsRest() )
            {
                if( !rLine.GetStart() )
                    rLine.Top(); 
                break;
            }
        sal_Int32 nNew = rLine.GetStart() + rLine.GetLength();
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

    
    
    
    
    bool bFirst  = true;
    bool bFormat = true;

    
    
    
    

    bool bBreak = ( !pPara->IsPrepMustFit() || rLine.GetLineNr() > 1 )
                    && aFrmBreak.IsBreakNowWidAndOrp( rLine );
    if( bBreak )
    {
        bool bPrevDone = 0 != rLine.Prev();
        while( bPrevDone && aFrmBreak.IsBreakNowWidAndOrp(rLine) )
            bPrevDone = 0 != rLine.Prev();
        if( bPrevDone )
        {
            aFrmBreak.SetKeep( false );
            rLine.Next();
        }
        rLine.TruncLines();

        
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
    bool bJumpEndHyph  = false;
    bool bWatchEndHyph = false;
    bool bJumpMidHyph  = false;
    bool bWatchMidHyph = false;

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
            SetFieldFollow( false );
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
                bFormat = true;
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
                bWatchEndHyph = false;
                bJumpEndHyph = false;
            }
            if ( rLine.GetCurr()->IsMidHyph() )
            {
                if ( bWatchMidHyph && !bFormat )
                    bFormat = ( rLine.CntEndHyph() == rInf.MaxHyph() );
            }
            else
            {
                bFormat |= bJumpMidHyph;
                bWatchMidHyph = false;
                bJumpMidHyph = false;
            }
        }
        if( bFormat )
        {
            const bool bOldEndHyph = rLine.GetCurr()->IsEndHyph();
            const bool bOldMidHyph = rLine.GetCurr()->IsMidHyph();
            bFormat = FormatLine( rLine, bPrev );
            
            bPrev = false;
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
                        ; 
                    pFreeze = new SwRepaint( rRepaint ); 
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
        
        
        
        if( rLine.GetStart() + rLine.GetLength() >= nStrLen &&
            rLine.GetCurr()->GetNext() )
        {
            rLine.TruncLines();
            rLine.SetTruncLines( true );
        }
    }

    if( !rInf.IsTest() )
    {
        
        if( bAdjust || !rLine.GetDropFmt() || !rLine.CalcOnceMore() )
        {
            FormatAdjust( rLine, aFrmBreak, nStrLen, rInf.IsStop() );
        }
        if( rRepaint.HasArea() )
            SetRepaint();
        rLine.SetTruncLines( false );
        if( nOldBottom ) 
                         
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
            rLine.SetOnceMore( false );
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

            
            if( !bGoOn )
            {
                rInf.CtorInitTxtFormatInfo( this );
                rLine.CtorInitTxtFormatter( this, &rInf );
                rLine.SetDropLines( 1 );
                rLine.CalcDropHeight( 1 );
                SwCharRange aTmpRange( 0, rInf.GetTxt().getLength() );
                *(pPara->GetReformat()) = aTmpRange;
                _Format( rLine, rInf, true );
                
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
        
        
        
        const bool bMustFit = pPara->IsPrepMustFit();
        pPara->Truncate();
        pPara->FormatReset();
        if( pBlink && pPara->IsBlinking() )
            pBlink->Delete( pPara );

        
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
            aLine.SetPaintDrop( true );
        }

        if( aLine.IsPaintDrop() )
        {
            aLine.CalcDropRepaint();
            aLine.SetPaintDrop( false );
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

    
    
    if( (Prt().*fnRect->fnGetWidth)() <= 0 )
    {
        
        
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

    const sal_Int32 nStrLen = GetTxtNode()->GetTxt().getLength();
    if ( nStrLen || !FormatEmpty() )
    {

        SetEmpty( false );
        
        FormatLevel aLevel;
        if( 12 == aLevel.GetLevel() )
            return;

        
        if( IsLocked() )
            return;

        
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
                
                
                
                HideAndShowObjects();
            }
            ChgThisLines();
            return;
        }

        
        SwTxtFrmLocker aLock(this);
        SwTxtLineAccess aAccess( this );
        const bool bNew = !aAccess.SwTxtLineAccess::IsAvailable();
        const bool bSetOfst =
            (GetOfst() && GetOfst() > GetTxtNode()->GetTxt().getLength());

        if( CalcPreps() )
            ; 
        
        
        else if( !bNew && !aAccess.GetPara()->GetReformat()->Len() )
        {
            if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue() )
            {
                aAccess.GetPara()->SetPrepAdjust();
                aAccess.GetPara()->SetPrep();
                CalcPreps();
            }
            SetWidow( false );
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
            
            if ( bSetOfst )
                _SetOfst( 0 );

            const bool bOrphan = IsWidow();
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
                SetWidow( false );
            }
        }
        if( IsEmptyMaster() )
        {
            SwFrm* pPre = GetPrev();
            if( pPre &&
                
                pPre->GetIndPrev() &&
                pPre->GetAttrSet()->GetKeep().GetValue() )
            {
                pPre->InvalidatePos();
            }
        }
    }

    ChgThisLines();

    
    SwParaPortion *pPara = GetPara();
    if ( pPara )
           pPara->SetPrepMustFit( false );

    CalcBaseOfstForFly();
    _CalcHeightOfLastLine(); 
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

bool SwTxtFrm::FormatQuick( bool bForceQuickFormat )
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTxtFrm::FormatQuick with swapped frame" );

#if OSL_DEBUG_LEVEL > 1
    const OUString aXXX = GetTxtNode()->GetTxt();
    const SwTwips nDbgY = Frm().Top();
    (void)nDbgY;
    
    static MSHORT nStopAt = 0;
    if( nStopAt == GetFrmId() )
    {
        int i = GetFrmId();
        (void)i;
    }
#endif

    if( IsEmpty() && FormatEmpty() )
        return true;

    
    if( HasPara() || IsWidow() || IsLocked()
        || !GetValidSizeFlag() ||
        ( ( IsVertical() ? Prt().Width() : Prt().Height() ) && IsHiddenNow() ) )
        return false;

    SwTxtLineAccess aAccess( this );
    SwParaPortion *pPara = aAccess.GetPara();
    if( !pPara )
        return false;

    SwFrmSwapper aSwapper( this, true );

    SwTxtFrmLocker aLock(this);
    SwTxtFormatInfo aInf( this, false, true );
    if( 0 != aInf.MaxHyph() )   
        return false;

    SwTxtFormatter  aLine( this, &aInf );

    
    if( aLine.GetDropFmt() )
        return false;

    sal_Int32 nStart = GetOfst();
    const sal_Int32 nEnd = GetFollow()
                      ? GetFollow()->GetOfst() : aInf.GetTxt().getLength();
    do
    {
        nStart = aLine.FormatLine( nStart );
        if( aInf.IsNewLine() || (!aInf.IsStop() && nStart < nEnd) )
            aLine.Insert( new SwLineLayout() );
    } while( aLine.Next() );

    
    Point aTopLeft( Frm().Pos() );
    aTopLeft += Prt().Pos();
    const SwTwips nNewHeight = aLine.Y() + aLine.GetLineHeight();
    const SwTwips nOldHeight = aTopLeft.Y() + Prt().Height();

    if( !bForceQuickFormat && nNewHeight != nOldHeight && !IsUndersized() )
    {
        
        const sal_Int32 nStrt = GetOfst();
        _InvalidateRange( SwCharRange( nStrt, nEnd - nStrt) );
        return false;
    }

    if (m_pFollow && nStart != (static_cast<SwTxtFrm*>(m_pFollow))->GetOfst())
        return false; 

    

    
    pPara->GetRepaint()->Pos( aTopLeft );
    pPara->GetRepaint()->SSize( Prt().SSize() );

    
    *(pPara->GetReformat()) = SwCharRange();
    *(pPara->GetDelta()) = 0;

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
