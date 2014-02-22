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
#include <hints.hxx>
#include <svl/ctloptions.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/sfxuno.hxx>
#include <editeng/langitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/pgrditem.hxx>
#include <swmodule.hxx>
#include <SwSmartTagMgr.hxx>
#include <doc.hxx>
#include "rootfrm.hxx"
#include <pagefrm.hxx>
#include <viewsh.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <txtatr.hxx>
#include <paratr.hxx>
#include <viewopt.hxx>
#include <dflyobj.hxx>
#include <flyfrm.hxx>
#include <tabfrm.hxx>
#include <frmtool.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <dbg_lay.hxx>
#include <fmtfld.hxx>
#include <fmtftn.hxx>
#include <txtfld.hxx>
#include <txtftn.hxx>
#include <charatr.hxx>
#include <ftninfo.hxx>
#include <fmtline.hxx>
#include <txtfrm.hxx>
#include <sectfrm.hxx>
#include <itrform2.hxx>
#include <widorp.hxx>
#include <txtcache.hxx>
#include <fntcache.hxx>
#include <SwGrammarMarkUp.hxx>
#include <lineinfo.hxx>
#include <SwPortionHandler.hxx>
#include <dcontact.hxx>
#include <sortedobjs.hxx>
#include <txtflcnt.hxx>
#include <fmtflcnt.hxx>
#include <fmtcntnt.hxx>
#include <numrule.hxx>
#include <swtable.hxx>
#include <fldupde.hxx>
#include <IGrammarContact.hxx>
#include <switerator.hxx>


TYPEINIT1( SwTxtFrm, SwCntntFrm );


void SwTxtFrm::SwapWidthAndHeight()
{
    if ( ! bIsSwapped )
    {
        const long nPrtOfstX = Prt().Pos().X();
        Prt().Pos().X() = Prt().Pos().Y();
        
        if( IsVertLR() )
            Prt().Pos().Y() = nPrtOfstX;
        else
            Prt().Pos().Y() = Frm().Width() - ( nPrtOfstX + Prt().Width() );

    }
    else
    {
        const long nPrtOfstY = Prt().Pos().Y();
        Prt().Pos().Y() = Prt().Pos().X();
        
        if( IsVertLR() )
            Prt().Pos().X() = nPrtOfstY;
        else
            Prt().Pos().X() = Frm().Height() - ( nPrtOfstY + Prt().Height() );
    }

    const long nFrmWidth = Frm().Width();
    Frm().Width( Frm().Height() );
    Frm().Height( nFrmWidth );
    const long nPrtWidth = Prt().Width();
    Prt().Width( Prt().Height() );
    Prt().Height( nPrtWidth );

    bIsSwapped = ! bIsSwapped;
}



void SwTxtFrm::SwitchHorizontalToVertical( SwRect& rRect ) const
{
    
    
    long nOfstX, nOfstY;
    if ( IsVertLR() )
    {
        nOfstX = rRect.Left() - Frm().Left();
        nOfstY = rRect.Top() - Frm().Top();
    }
    else
    {
        nOfstX = rRect.Left() - Frm().Left();
        nOfstY = rRect.Top() + rRect.Height() - Frm().Top();
    }

    const long nWidth = rRect.Width();
    const long nHeight = rRect.Height();

    
    if ( IsVertLR() )
        rRect.Left(Frm().Left() + nOfstY);
    else
    {
        if ( bIsSwapped )
            rRect.Left( Frm().Left() + Frm().Height() - nOfstY );
        else
            
            rRect.Left( Frm().Left() + Frm().Width() - nOfstY );
    }

    rRect.Top( Frm().Top() + nOfstX );
    rRect.Width( nHeight );
    rRect.Height( nWidth );
}



void SwTxtFrm::SwitchHorizontalToVertical( Point& rPoint ) const
{
    
    const long nOfstX = rPoint.X() - Frm().Left();
    const long nOfstY = rPoint.Y() - Frm().Top();
    
    if ( IsVertLR() )
        rPoint.X() = Frm().Left() + nOfstY;
    else
    {
        if ( bIsSwapped )
            rPoint.X() = Frm().Left() + Frm().Height() - nOfstY;
        else
            
            rPoint.X() = Frm().Left() + Frm().Width() - nOfstY;
    }

    rPoint.Y() = Frm().Top() + nOfstX;
}



long SwTxtFrm::SwitchHorizontalToVertical( long nLimit ) const
{
    Point aTmp( 0, nLimit );
    SwitchHorizontalToVertical( aTmp );
    return aTmp.X();
}



void SwTxtFrm::SwitchVerticalToHorizontal( SwRect& rRect ) const
{
    long nOfstX;

    

    
    if ( IsVertLR() )
        nOfstX = rRect.Left() - Frm().Left();
    else
    {
        if ( bIsSwapped )
            nOfstX = Frm().Left() + Frm().Height() - ( rRect.Left() + rRect.Width() );
        else
            nOfstX = Frm().Left() + Frm().Width() - ( rRect.Left() + rRect.Width() );
    }

    const long nOfstY = rRect.Top() - Frm().Top();
    const long nWidth = rRect.Height();
    const long nHeight = rRect.Width();

    
    rRect.Left( Frm().Left() + nOfstY );
    rRect.Top( Frm().Top() + nOfstX );
    rRect.Width( nWidth );
    rRect.Height( nHeight );
}



void SwTxtFrm::SwitchVerticalToHorizontal( Point& rPoint ) const
{
    long nOfstX;

    

    
    if ( IsVertLR() )
        nOfstX = rPoint.X() - Frm().Left();
    else
    {
        if ( bIsSwapped )
            nOfstX = Frm().Left() + Frm().Height() - rPoint.X();
        else
            nOfstX = Frm().Left() + Frm().Width() - rPoint.X();
    }

    const long nOfstY = rPoint.Y() - Frm().Top();

    
    rPoint.X() = Frm().Left() + nOfstY;
    rPoint.Y() = Frm().Top() + nOfstX;
}



long SwTxtFrm::SwitchVerticalToHorizontal( long nLimit ) const
{
    Point aTmp( nLimit, 0 );
    SwitchVerticalToHorizontal( aTmp );
    return aTmp.Y();
}

SwFrmSwapper::SwFrmSwapper( const SwTxtFrm* pTxtFrm, bool bSwapIfNotSwapped )
    : pFrm( pTxtFrm ), bUndo( false )
{
    if ( pFrm->IsVertical() &&
        ( (   bSwapIfNotSwapped && ! pFrm->IsSwapped() ) ||
          ( ! bSwapIfNotSwapped && pFrm->IsSwapped() ) ) )
    {
        bUndo = true;
        ((SwTxtFrm*)pFrm)->SwapWidthAndHeight();
    }
}

SwFrmSwapper::~SwFrmSwapper()
{
    if ( bUndo )
        ((SwTxtFrm*)pFrm)->SwapWidthAndHeight();
}

void SwTxtFrm::SwitchLTRtoRTL( SwRect& rRect ) const
{
    SWAP_IF_NOT_SWAPPED( this )

    long nWidth = rRect.Width();
    rRect.Left( 2 * ( Frm().Left() + Prt().Left() ) +
                Prt().Width() - rRect.Right() - 1 );

    rRect.Width( nWidth );

    UNDO_SWAP( this )
}

void SwTxtFrm::SwitchLTRtoRTL( Point& rPoint ) const
{
    SWAP_IF_NOT_SWAPPED( this )

    rPoint.X() = 2 * ( Frm().Left() + Prt().Left() ) + Prt().Width() - rPoint.X() - 1;

    UNDO_SWAP( this )
}

SwLayoutModeModifier::SwLayoutModeModifier( const OutputDevice& rOutp ) :
        rOut( rOutp ), nOldLayoutMode( rOutp.GetLayoutMode() )
{
}

SwLayoutModeModifier::~SwLayoutModeModifier()
{
    ((OutputDevice&)rOut).SetLayoutMode( nOldLayoutMode );
}

void SwLayoutModeModifier::Modify( bool bChgToRTL )
{
    ((OutputDevice&)rOut).SetLayoutMode( bChgToRTL ?
                                         TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_BIDI_RTL :
                                         TEXT_LAYOUT_BIDI_STRONG );
}

void SwLayoutModeModifier::SetAuto()
{
    const sal_uLong nNewLayoutMode = nOldLayoutMode & ~TEXT_LAYOUT_BIDI_STRONG;
    ((OutputDevice&)rOut).SetLayoutMode( nNewLayoutMode );
}

SwDigitModeModifier::SwDigitModeModifier( const OutputDevice& rOutp, LanguageType eCurLang ) :
        rOut( rOutp ), nOldLanguageType( rOutp.GetDigitLanguage() )
{
    LanguageType eLang = eCurLang;
    const SvtCTLOptions::TextNumerals nTextNumerals = SW_MOD()->GetCTLOptions().GetCTLTextNumerals();

    if ( SvtCTLOptions::NUMERALS_HINDI == nTextNumerals )
        eLang = LANGUAGE_ARABIC_SAUDI_ARABIA;
    else if ( SvtCTLOptions::NUMERALS_ARABIC == nTextNumerals )
        eLang = LANGUAGE_ENGLISH;
    else if ( SvtCTLOptions::NUMERALS_SYSTEM == nTextNumerals )
        eLang = ::GetAppLanguage();

    ((OutputDevice&)rOut).SetDigitLanguage( eLang );
}

SwDigitModeModifier::~SwDigitModeModifier()
{
    ((OutputDevice&)rOut).SetDigitLanguage( nOldLanguageType );
}

void SwTxtFrm::Init()
{
    OSL_ENSURE( !IsLocked(), "+SwTxtFrm::Init: this is locked." );
    if( !IsLocked() )
    {
        ClearPara();
        ResetBlinkPor();
        
        
        
        
    }
}

SwTxtFrm::SwTxtFrm(SwTxtNode * const pNode, SwFrm* pSib )
    : SwCntntFrm( pNode, pSib )
    , nAllLines( 0 )
    , nThisLines( 0 )
    , mnFlyAnchorOfst( 0 )
    , mnFlyAnchorOfstNoWrap( 0 )
    , mnFtnLine( 0 )
    , mnHeightOfLastLine( 0 ) 
    , mnAdditionalFirstLineOffset( 0 )
    , nOfst( 0 )
    , nCacheIdx( MSHRT_MAX )
    , bLocked( false )
    , bFormatted( false )
    , bWidow( false )
    , bJustWidow( false )
    , bEmpty( false )
    , bInFtnConnect( false )
    , bFtn( false )
    , bRepaint( false )
    , bBlinkPor( false )
    , bFieldFollow( false )
    , bHasAnimation( false )
    , bIsSwapped( false )
    , mbFollowFormatAllowed( true ) 
{
    mnType = FRMC_TXT;
}

SwTxtFrm::~SwTxtFrm()
{
    
    ClearPara();
}

const OUString& SwTxtFrm::GetTxt() const
{
    return GetTxtNode()->GetTxt();
}

void SwTxtFrm::ResetPreps()
{
    if ( GetCacheIdx() != MSHRT_MAX )
    {
        SwParaPortion *pPara;
        if( 0 != (pPara = GetPara()) )
            pPara->ResetPreps();
    }
}

bool SwTxtFrm::IsHiddenNow() const
{
    SwFrmSwapper aSwapper( this, true );

    if( !Frm().Width() && IsValid() && GetUpper()->IsValid() )
                                       
    {

        return true;
    }

    const bool bHiddenCharsHidePara = GetTxtNode()->HasHiddenCharAttribute( true );
    const bool bHiddenParaField = GetTxtNode()->HasHiddenParaField();
    const SwViewShell* pVsh = getRootFrm()->GetCurrShell();

    if ( pVsh && ( bHiddenCharsHidePara || bHiddenParaField ) )
    {
        if (
             ( bHiddenParaField &&
               ( !pVsh->GetViewOptions()->IsShowHiddenPara() &&
                 !pVsh->GetViewOptions()->IsFldName() ) ) ||
             ( bHiddenCharsHidePara &&
               !pVsh->GetViewOptions()->IsShowHiddenChar() ) )
        {
            return true;
        }
    }

    return false;
}


void SwTxtFrm::HideHidden()
{
    OSL_ENSURE( !GetFollow() && IsHiddenNow(),
            "HideHidden on visible frame of hidden frame has follow" );

    const sal_Int32 nEnd = COMPLETE_STRING;
    HideFootnotes( GetOfst(), nEnd );
    
    HideAndShowObjects();

    
    ClearPara();
}

void SwTxtFrm::HideFootnotes( sal_Int32 nStart, sal_Int32 nEnd )
{
    const SwpHints *pHints = GetTxtNode()->GetpSwpHints();
    if( pHints )
    {
        const sal_uInt16 nSize = pHints->Count();
        SwPageFrm *pPage = 0;
        for ( sal_uInt16 i = 0; i < nSize; ++i )
        {
            const SwTxtAttr *pHt = (*pHints)[i];
            if ( pHt->Which() == RES_TXTATR_FTN )
            {
                const sal_Int32 nIdx = *pHt->GetStart();
                if ( nEnd < nIdx )
                    break;
                if( nStart <= nIdx )
                {
                    if( !pPage )
                        pPage = FindPageFrm();
                    pPage->RemoveFtn( this, (SwTxtFtn*)pHt );
                }
            }
        }
    }
}









bool sw_HideObj( const SwTxtFrm& _rFrm,
                  const RndStdIds _eAnchorType,
                  const sal_Int32 _nObjAnchorPos,
                  SwAnchoredObject* _pAnchoredObj )
{
    bool bRet( true );

    if (_eAnchorType == FLY_AT_CHAR)
    {
        const IDocumentSettingAccess* pIDSA = _rFrm.GetTxtNode()->getIDocumentSettingAccess();
        if ( !pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) &&
             !pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING) &&
             !pIDSA->get(IDocumentSettingAccess::USE_FORMER_OBJECT_POS) &&
              pIDSA->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) &&
             _rFrm.IsInDocBody() && !_rFrm.FindNextCnt() )
        {
            const sal_Unicode cAnchorChar =
                        _rFrm.GetTxtNode()->GetTxt()[_nObjAnchorPos];
            if ( cAnchorChar == CH_TXTATR_BREAKWORD )
            {
                const SwTxtAttr* const pHint(
                    _rFrm.GetTxtNode()->GetTxtAttrForCharAt(_nObjAnchorPos,
                        RES_TXTATR_FLYCNT) );
                if ( pHint )
                {
                    const SwFrmFmt* pFrmFmt =
                        static_cast<const SwTxtFlyCnt*>(pHint)->GetFlyCnt().GetFrmFmt();
                    if ( pFrmFmt->Which() == RES_FLYFRMFMT )
                    {
                        SwNodeIndex nCntntIndex = *(pFrmFmt->GetCntnt().GetCntntIdx());
                        ++nCntntIndex;
                        if ( nCntntIndex.GetNode().IsNoTxtNode() )
                        {
                            bRet = false;
                            
                            SWRECTFN( (&_rFrm) );
                            SwRect aLastCharRect( _rFrm.Frm() );
                            (aLastCharRect.*fnRect->fnSetWidth)( 1 );
                            _pAnchoredObj->maLastCharRect = aLastCharRect;
                            _pAnchoredObj->mnLastTopOfLine = (aLastCharRect.*fnRect->fnGetTop)();
                        }
                    }
                }
            }
        }
    }

    return bRet;
}

/** method to hide/show objects

    OD 2004-01-15 #110582#
    method hides respectively shows objects, which are anchored at paragraph,
    at/as a character of the paragraph, corresponding to the paragraph and
    paragraph portion visibility.

    - is called from HideHidden() - should hide objects in hidden paragraphs and
    - from _Format() - should hide/show objects in partly visible paragraphs
*/
void SwTxtFrm::HideAndShowObjects()
{
    if ( GetDrawObjs() )
    {
        if ( IsHiddenNow() )
        {
            
            for ( sal_uInt32 i = 0; i < GetDrawObjs()->Count(); ++i )
            {
                SdrObject* pObj = (*GetDrawObjs())[i]->DrawObj();
                SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
                
                
                const RndStdIds eAnchorType( pContact->GetAnchorId() );
                const sal_Int32 nObjAnchorPos = pContact->GetCntntAnchorIndex().GetIndex();
                if ((eAnchorType != FLY_AT_CHAR) ||
                    sw_HideObj( *this, eAnchorType, nObjAnchorPos,
                                 (*GetDrawObjs())[i] ))
                {
                    pContact->MoveObjToInvisibleLayer( pObj );
                }
            }
        }
        else
        {
            
            
            const SwTxtNode& rNode = *GetTxtNode();
            const SwViewShell* pVsh = getRootFrm()->GetCurrShell();
            const bool bShouldBeHidden = !pVsh || !pVsh->GetWin() ||
                                         !pVsh->GetViewOptions()->IsShowHiddenChar();

            
            
            
            for ( sal_uInt32 i = 0; i < GetDrawObjs()->Count(); ++i )
            {
                SdrObject* pObj = (*GetDrawObjs())[i]->DrawObj();
                SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
                
                const RndStdIds eAnchorType( pContact->GetAnchorId() );

                if (eAnchorType == FLY_AT_PARA)
                {
                    pContact->MoveObjToVisibleLayer( pObj );
                }
                else if ((eAnchorType == FLY_AT_CHAR) ||
                         (eAnchorType == FLY_AS_CHAR))
                {
                    sal_Int32 nHiddenStart;
                    sal_Int32 nHiddenEnd;
                    const sal_Int32 nObjAnchorPos = pContact->GetCntntAnchorIndex().GetIndex();
                    SwScriptInfo::GetBoundsOfHiddenRange( rNode, nObjAnchorPos, nHiddenStart, nHiddenEnd, 0 );
                    
                    
                    if ( nHiddenStart != COMPLETE_STRING && bShouldBeHidden &&
                         sw_HideObj( *this, eAnchorType, nObjAnchorPos, (*GetDrawObjs())[i] ) )
                        pContact->MoveObjToInvisibleLayer( pObj );
                    else
                        pContact->MoveObjToVisibleLayer( pObj );
                }
                else
                {
                    OSL_FAIL( "<SwTxtFrm::HideAndShowObjects()> - object not anchored at/inside paragraph!?" );
                }
            }
        }
    }

    if (IsFollow())
    {
        SwTxtFrm *pMaster = FindMaster();
        OSL_ENSURE(pMaster, "SwTxtFrm without master");
        if (pMaster)
            pMaster->HideAndShowObjects();
    }
}

/*************************************************************************
 *                      SwTxtFrm::FindBrk()
 *
 * Returns the first possible break point in the current line.
 * This method is used in SwTxtFrm::Format() to decide whether the previous
 * line has to be formatted as well.
 * nFound is <= nEndLine.
 *************************************************************************/

sal_Int32 SwTxtFrm::FindBrk( const OUString &rTxt,
                              const sal_Int32 nStart,
                              const sal_Int32 nEnd ) const
{
    sal_Int32 nFound = nStart;
    const sal_Int32 nEndLine = std::min( nEnd, rTxt.getLength() - 1 );

    
    while( nFound <= nEndLine && ' ' == rTxt[nFound] )
    {
         nFound++;
    }

    
    
    
    
    while( nFound <= nEndLine && ' ' != rTxt[nFound] )
    {
        nFound++;
    }

    return nFound;
}

bool SwTxtFrm::IsIdxInside( const sal_Int32 nPos, const sal_Int32 nLen ) const
{
    if( nLen != COMPLETE_STRING && GetOfst() > nPos + nLen ) 
        return false;

    if( !GetFollow() )            
        return true;          

    const sal_Int32 nMax = GetFollow()->GetOfst();

    
    if( nMax > nPos || nMax > GetTxt().getLength() )
        return true;

    
    const SwParaPortion* pPara = GetFollow()->GetPara();
    return pPara && ( nPos <= nMax + pPara->GetLen() );
}

inline void SwTxtFrm::InvalidateRange(const SwCharRange &aRange, const long nD)
{
    if ( IsIdxInside( aRange.Start(), aRange.Len() ) )
        _InvalidateRange( aRange, nD );
}

void SwTxtFrm::_InvalidateRange( const SwCharRange &aRange, const long nD)
{
    if ( !HasPara() )
    {   InvalidateSize();
        return;
    }

    SetWidow( false );
    SwParaPortion *pPara = GetPara();

    bool bInv = false;
    if( 0 != nD )
    {
        
        
        
        
        *(pPara->GetDelta()) += nD;
        bInv = true;
    }
    SwCharRange &rReformat = *(pPara->GetReformat());
    if(aRange != rReformat) {
        if( COMPLETE_STRING == rReformat.Len() )
            rReformat = aRange;
        else
            rReformat += aRange;
        bInv = true;
    }
    if(bInv)
    {
        InvalidateSize();
    }
}

void SwTxtFrm::CalcLineSpace()
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTxtFrm::CalcLineSpace with swapped frame!" );

    if( IsLocked() || !HasPara() )
        return;

    SwParaPortion *pPara;
    if( GetDrawObjs() ||
        GetTxtNode()->GetSwAttrSet().GetLRSpace().IsAutoFirst() ||
        ( pPara = GetPara() )->IsFixLineHeight() )
    {
        Init();
        return;
    }

    Size aNewSize( Prt().SSize() );

    SwTxtFormatInfo aInf( this );
    SwTxtFormatter aLine( this, &aInf );
    if( aLine.GetDropLines() )
    {
        Init();
        return;
    }

    aLine.Top();
    aLine.RecalcRealHeight();

    aNewSize.Height() = (aLine.Y() - Frm().Top()) + aLine.GetLineHeight();

    SwTwips nDelta = aNewSize.Height() - Prt().Height();
    
    if( aInf.GetTxtFly()->IsOn() )
    {
        SwRect aTmpFrm( Frm() );
        if( nDelta < 0 )
            aTmpFrm.Height( Prt().Height() );
        else
            aTmpFrm.Height( aNewSize.Height() );
        if( aInf.GetTxtFly()->Relax( aTmpFrm ) )
        {
            Init();
            return;
        }
    }

    if( nDelta )
    {
        SwTxtFrmBreak aBreak( this );
        if( GetFollow() || aBreak.IsBreakNow( aLine ) )
        {
            
            Init();
        }
        else
        {
            
            pPara->SetPrepAdjust();
            pPara->SetPrep();
        }
    }
}

static void lcl_SetWrong( SwTxtFrm& rFrm, sal_Int32 nPos, sal_Int32 nCnt, bool bMove )
{
    if ( !rFrm.IsFollow() )
    {
        SwTxtNode* pTxtNode = rFrm.GetTxtNode();
        IGrammarContact* pGrammarContact = getGrammarContact( *pTxtNode );
        SwGrammarMarkUp* pWrongGrammar = pGrammarContact ?
            pGrammarContact->getGrammarCheck( *pTxtNode, false ) :
            pTxtNode->GetGrammarCheck();
        bool bGrammarProxy = pWrongGrammar != pTxtNode->GetGrammarCheck();
        if( bMove )
        {
            if( pTxtNode->GetWrong() )
                pTxtNode->GetWrong()->Move( nPos, nCnt );
            if( pWrongGrammar )
                pWrongGrammar->MoveGrammar( nPos, nCnt );
            if( bGrammarProxy && pTxtNode->GetGrammarCheck() )
                pTxtNode->GetGrammarCheck()->MoveGrammar( nPos, nCnt );
            if( pTxtNode->GetSmartTags() )
                pTxtNode->GetSmartTags()->Move( nPos, nCnt );
        }
        else
        {
            if( pTxtNode->GetWrong() )
                pTxtNode->GetWrong()->Invalidate( nPos, nCnt );
            if( pWrongGrammar )
                pWrongGrammar->Invalidate( nPos, nCnt );
            if( pTxtNode->GetSmartTags() )
                pTxtNode->GetSmartTags()->Invalidate( nPos, nCnt );
        }
        const sal_Int32 nEnd = nPos + (nCnt > 0 ? nCnt : 1 );
        if ( !pTxtNode->GetWrong() && !pTxtNode->IsWrongDirty() )
        {
            pTxtNode->SetWrong( new SwWrongList( WRONGLIST_SPELL ) );
            pTxtNode->GetWrong()->SetInvalid( nPos, nEnd );
        }
        if ( !pTxtNode->GetSmartTags() && !pTxtNode->IsSmartTagDirty() )
        {
            
            pTxtNode->SetSmartTags( new SwWrongList( WRONGLIST_SMARTTAG ) );
            pTxtNode->GetSmartTags()->SetInvalid( nPos, nEnd );
        }
        pTxtNode->SetWrongDirty( true );
        pTxtNode->SetGrammarCheckDirty( true );
        pTxtNode->SetWordCountDirty( true );
        pTxtNode->SetAutoCompleteWordDirty( true );
        
        pTxtNode->SetSmartTagDirty( true );
    }

    SwRootFrm *pRootFrm = rFrm.getRootFrm();
    if (pRootFrm)
    {
        pRootFrm->SetNeedGrammarCheck( true );
    }

    SwPageFrm *pPage = rFrm.FindPageFrm();
    if( pPage )
    {
        pPage->InvalidateSpelling();
        pPage->InvalidateAutoCompleteWords();
        pPage->InvalidateWordCount();
        pPage->InvalidateSmartTags();
    }
}

static void lcl_SetScriptInval( SwTxtFrm& rFrm, sal_Int32 nPos )
{
    if( rFrm.GetPara() )
        rFrm.GetPara()->GetScriptInfo().SetInvalidityA( nPos );
}

static void lcl_ModifyOfst( SwTxtFrm* pFrm, sal_Int32 nPos, sal_Int32 nLen )
{
    while( pFrm && pFrm->GetOfst() <= nPos )
        pFrm = pFrm->GetFollow();
    while( pFrm )
    {
        if (nLen == COMPLETE_STRING)
            pFrm->ManipOfst( COMPLETE_STRING );
        else
            pFrm->ManipOfst( pFrm->GetOfst() + nLen );
        pFrm = pFrm->GetFollow();
    }
}



static bool isA11yRelevantAttribute(MSHORT nWhich)
{
    return nWhich != RES_CHRATR_RSID;
}

void SwTxtFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    const MSHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;

    
    if( IsInRange( aFrmFmtSetRange, nWhich ) || RES_FMT_CHG == nWhich )
    {
        SwCntntFrm::Modify( pOld, pNew );
        if( nWhich == RES_FMT_CHG && getRootFrm()->GetCurrShell() )
        {
            
            Prepare( PREP_CLEAR );
            _InvalidatePrt();
            lcl_SetWrong( *this, 0, COMPLETE_STRING, false );
            SetDerivedR2L( sal_False );
            CheckDirChange();
            
            
            SetCompletePaint();
            InvalidateLineNum();
        }
        return;
    }

    
    if( IsLocked() )
        return;

    
    
    sal_Int32 nPos;
    sal_Int32 nLen;
    bool bSetFldsDirty = false;
    bool bRecalcFtnFlag = false;

    switch( nWhich )
    {
        case RES_LINENUMBER:
        {
            InvalidateLineNum();
        }
        break;
        case RES_INS_TXT:
        {
            nPos = ((SwInsTxt*)pNew)->nPos;
            nLen = ((SwInsTxt*)pNew)->nLen;
            if( IsIdxInside( nPos, nLen ) )
            {
                if( !nLen )
                {
                    
                    if( nPos )
                        InvalidateSize();
                    else
                        Prepare( PREP_CLEAR );
                }
                else
                    _InvalidateRange( SwCharRange( nPos, nLen ), nLen );
            }
            lcl_SetWrong( *this, nPos, nLen, true );
            lcl_SetScriptInval( *this, nPos );
            bSetFldsDirty = true;
            if( HasFollow() )
                lcl_ModifyOfst( this, nPos, nLen );
        }
        break;
        case RES_DEL_CHR:
        {
            nPos = ((SwDelChr*)pNew)->nPos;
            InvalidateRange( SwCharRange( nPos, 1 ), -1 );
            lcl_SetWrong( *this, nPos, -1, true );
            lcl_SetScriptInval( *this, nPos );
            bSetFldsDirty = bRecalcFtnFlag = true;
            if( HasFollow() )
                lcl_ModifyOfst( this, nPos, COMPLETE_STRING );
        }
        break;
        case RES_DEL_TXT:
        {
            nPos = ((SwDelTxt*)pNew)->nStart;
            nLen = ((SwDelTxt*)pNew)->nLen;
            const sal_Int32 m = -nLen;
            if( IsIdxInside( nPos, nLen ) )
            {
                if( !nLen )
                    InvalidateSize();
                else
                    InvalidateRange( SwCharRange( nPos, 1 ), m );
            }
            lcl_SetWrong( *this, nPos, m, true );
            lcl_SetScriptInval( *this, nPos );
            bSetFldsDirty = bRecalcFtnFlag = true;
            if( HasFollow() )
                lcl_ModifyOfst( this, nPos, nLen );
        }
        break;
        case RES_UPDATE_ATTR:
        {
            nPos = ((SwUpdateAttr*)pNew)->nStart;
            nLen = ((SwUpdateAttr*)pNew)->nEnd - nPos;
            if( IsIdxInside( nPos, nLen ) )
            {
                
                
                
                

                
                if( !nLen )
                    nLen = 1;

                _InvalidateRange( SwCharRange( nPos, nLen) );
                MSHORT nTmp = ((SwUpdateAttr*)pNew)->nWhichAttr;

                if( ! nTmp || RES_TXTATR_CHARFMT == nTmp || RES_TXTATR_AUTOFMT == nTmp ||
                    RES_FMT_CHG == nTmp || RES_ATTRSET_CHG == nTmp )
                {
                    lcl_SetWrong( *this, nPos, nPos + nLen, false );
                    lcl_SetScriptInval( *this, nPos );
                }
            }

            
            SwViewShell* pViewSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
            if ( pViewSh  )
            {
                pViewSh->InvalidateAccessibleParaAttrs( *this );
            }
        }
        break;
        case RES_OBJECTDYING:
        break;

        case RES_PARATR_LINESPACING:
            {
                CalcLineSpace();
                InvalidateSize();
                _InvalidatePrt();
                if( IsInSct() && !GetPrev() )
                {
                    SwSectionFrm *pSect = FindSctFrm();
                    if( pSect->ContainsAny() == this )
                        pSect->InvalidatePrt();
                }

                
                
                
                
                InvalidateNextPrtArea();

                SetCompletePaint();
            }
            break;

        case RES_TXTATR_FIELD:
        case RES_TXTATR_ANNOTATION:
            {
                nPos = *((SwFmtFld*)pNew)->GetTxtFld()->GetStart();
                if( IsIdxInside( nPos, 1 ) )
                {
                    if( pNew == pOld )
                    {
                        
                        
                        InvalidatePage();
                        SetCompletePaint();
                    }
                    else
                        _InvalidateRange( SwCharRange( nPos, 1 ) );
                }
                bSetFldsDirty = true;
                
                if ( SwSmartTagMgr::Get().IsSmartTagsEnabled() )
                    lcl_SetWrong( *this, nPos, nPos + 1, false );
            }
            break;

        case RES_TXTATR_FTN :
        {
            nPos = *((SwFmtFtn*)pNew)->GetTxtFtn()->GetStart();
            if( IsInFtn() || IsIdxInside( nPos, 1 ) )
                Prepare( PREP_FTN, ((SwFmtFtn*)pNew)->GetTxtFtn() );
            break;
        }

        case RES_ATTRSET_CHG:
        {
            InvalidateLineNum();

            SwAttrSet& rNewSet = *((SwAttrSetChg*)pNew)->GetChgSet();
            const SfxPoolItem* pItem = 0;
            int nClear = 0;
            MSHORT nCount = rNewSet.Count();

            if( SFX_ITEM_SET == rNewSet.GetItemState( RES_TXTATR_FTN, false, &pItem ))
            {
                nPos = *((SwFmtFtn*)pItem)->GetTxtFtn()->GetStart();
                if( IsIdxInside( nPos, 1 ) )
                    Prepare( PREP_FTN, pNew );
                nClear = 0x01;
                --nCount;
            }

            if( SFX_ITEM_SET == rNewSet.GetItemState( RES_TXTATR_FIELD, false, &pItem ))
            {
                nPos = *((SwFmtFld*)pItem)->GetTxtFld()->GetStart();
                if( IsIdxInside( nPos, 1 ) )
                {
                    const SfxPoolItem& rOldItem =
                        ((SwAttrSetChg*)pOld)->GetChgSet()->Get( RES_TXTATR_FIELD );
                    if( pItem == &rOldItem )
                    {
                        InvalidatePage();
                        SetCompletePaint();
                    }
                    else
                        _InvalidateRange( SwCharRange( nPos, 1 ) );
                }
                nClear |= 0x02;
                --nCount;
            }
            bool bLineSpace = SFX_ITEM_SET == rNewSet.GetItemState(
                                            RES_PARATR_LINESPACING, false ),
                     bRegister  = SFX_ITEM_SET == rNewSet.GetItemState(
                                            RES_PARATR_REGISTER, false );
            if ( bLineSpace || bRegister )
            {
                Prepare( bRegister ? PREP_REGISTER : PREP_ADJUST_FRM );
                CalcLineSpace();
                InvalidateSize();
                _InvalidatePrt();

                
                
                
                
                InvalidateNextPrtArea();

                SetCompletePaint();
                nClear |= 0x04;
                if ( bLineSpace )
                {
                    --nCount;
                    if( IsInSct() && !GetPrev() )
                    {
                        SwSectionFrm *pSect = FindSctFrm();
                        if( pSect->ContainsAny() == this )
                            pSect->InvalidatePrt();
                    }
                }
                if ( bRegister )
                    --nCount;
            }
            if ( SFX_ITEM_SET == rNewSet.GetItemState( RES_PARATR_SPLIT,
                                                       false ))
            {
                if ( GetPrev() )
                    CheckKeep();
                Prepare( PREP_CLEAR );
                InvalidateSize();
                nClear |= 0x08;
                --nCount;
            }

            if( SFX_ITEM_SET == rNewSet.GetItemState( RES_BACKGROUND, false)
                && !IsFollow() && GetDrawObjs() )
            {
                SwSortedObjs *pObjs = GetDrawObjs();
                for ( int i = 0; GetDrawObjs() && i < int(pObjs->Count()); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pObjs)[MSHORT(i)];
                    if ( pAnchoredObj->ISA(SwFlyFrm) )
                    {
                        SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                        if( !pFly->IsFlyInCntFrm() )
                        {
                            const SvxBrushItem &rBack =
                                pFly->GetAttrSet()->GetBackground();
                            
                            
                            
                            
                            
                            
                            
                            
                            if ( (rBack.GetColor() == COL_TRANSPARENT)  &&
                                rBack.GetGraphicPos() == GPOS_NONE )
                            {
                                pFly->SetCompletePaint();
                                pFly->InvalidatePage();
                            }
                        }
                    }
                }
            }

            if ( SFX_ITEM_SET ==
                 rNewSet.GetItemState( RES_TXTATR_CHARFMT, false ) )
            {
                lcl_SetWrong( *this, 0, COMPLETE_STRING, false );
                lcl_SetScriptInval( *this, 0 );
            }
            else if ( SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_LANGUAGE, false ) ||
                      SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_CJK_LANGUAGE, false ) ||
                      SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_CTL_LANGUAGE, false ) )
                lcl_SetWrong( *this, 0, COMPLETE_STRING, false );
            else if ( SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_FONT, false ) ||
                      SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_CJK_FONT, false ) ||
                      SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_CTL_FONT, false ) )
                lcl_SetScriptInval( *this, 0 );
            else if ( SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_FRAMEDIR, false ) )
            {
                SetDerivedR2L( sal_False );
                CheckDirChange();
                
                
                SetCompletePaint();
            }


            if( nCount )
            {
                if( getRootFrm()->GetCurrShell() )
                {
                    Prepare( PREP_CLEAR );
                    _InvalidatePrt();
                }

                if( nClear )
                {
                    SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
                    SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );

                    if( 0x01 & nClear )
                    {
                        aOldSet.ClearItem( RES_TXTATR_FTN );
                        aNewSet.ClearItem( RES_TXTATR_FTN );
                    }
                    if( 0x02 & nClear )
                    {
                        aOldSet.ClearItem( RES_TXTATR_FIELD );
                        aNewSet.ClearItem( RES_TXTATR_FIELD );
                    }
                    if ( 0x04 & nClear )
                    {
                        if ( bLineSpace )
                        {
                            aOldSet.ClearItem( RES_PARATR_LINESPACING );
                            aNewSet.ClearItem( RES_PARATR_LINESPACING );
                        }
                        if ( bRegister )
                        {
                            aOldSet.ClearItem( RES_PARATR_REGISTER );
                            aNewSet.ClearItem( RES_PARATR_REGISTER );
                        }
                    }
                    if ( 0x08 & nClear )
                    {
                        aOldSet.ClearItem( RES_PARATR_SPLIT );
                        aNewSet.ClearItem( RES_PARATR_SPLIT );
                    }
                    SwCntntFrm::Modify( &aOldSet, &aNewSet );
                }
                else
                    SwCntntFrm::Modify( pOld, pNew );
            }

            if (isA11yRelevantAttribute(nWhich))
            {
                
                SwViewShell* pViewSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
                if ( pViewSh  )
                {
                    pViewSh->InvalidateAccessibleParaAttrs( *this );
                }
            }
        }
        break;

        
        case RES_DOCPOS_UPDATE:
        {
            if( pOld && pNew )
            {
                const SwDocPosUpdate *pDocPos = (const SwDocPosUpdate*)pOld;
                if( pDocPos->nDocPos <= maFrm.Top() )
                {
                    const SwFmtFld *pFld = (const SwFmtFld *)pNew;
                    InvalidateRange(
                        SwCharRange( *pFld->GetTxtFld()->GetStart(), 1 ) );
                }
            }
            break;
        }
        case RES_PARATR_SPLIT:
            if ( GetPrev() )
                CheckKeep();
            Prepare( PREP_CLEAR );
            bSetFldsDirty = true;
            break;
        case RES_FRAMEDIR :
            SetDerivedR2L( sal_False );
            CheckDirChange();
            break;
        default:
        {
            Prepare( PREP_CLEAR );
            _InvalidatePrt();
            if ( !nWhich )
            {
                
                SwFrm *pNxt;
                if ( 0 != (pNxt = FindNext()) )
                    pNxt->InvalidatePrt();
            }
        }
    } 

    if( bSetFldsDirty )
        GetNode()->getIDocumentFieldsAccess()->SetFieldsDirty( true, GetNode(), 1 );

    if ( bRecalcFtnFlag )
        CalcFtnFlag();
}

bool SwTxtFrm::GetInfo( SfxPoolItem &rHnt ) const
{
    if ( RES_VIRTPAGENUM_INFO == rHnt.Which() && IsInDocBody() && ! IsFollow() )
    {
        SwVirtPageNumInfo &rInfo = (SwVirtPageNumInfo&)rHnt;
        const SwPageFrm *pPage = FindPageFrm();
        if ( pPage )
        {
            if ( pPage == rInfo.GetOrigPage() && !GetPrev() )
            {
                
                
                rInfo.SetInfo( pPage, this );
                return false;
            }
            if ( pPage->GetPhyPageNum() < rInfo.GetOrigPage()->GetPhyPageNum() &&
                 (!rInfo.GetPage() || pPage->GetPhyPageNum() > rInfo.GetPage()->GetPhyPageNum()))
            {
                
                rInfo.SetInfo( pPage, this );
            }
        }
    }
    return true;
}

void SwTxtFrm::PrepWidows( const MSHORT nNeed, bool bNotify )
{
    OSL_ENSURE(GetFollow() && nNeed, "+SwTxtFrm::Prepare: lost all friends");

    SwParaPortion *pPara = GetPara();
    if ( !pPara )
        return;
    pPara->SetPrepWidows();

    MSHORT nHave = nNeed;

    
    SWAP_IF_NOT_SWAPPED( this )

    SwTxtSizeInfo aInf( this );
    SwTxtMargin aLine( this, &aInf );
    aLine.Bottom();
    sal_Int32 nTmpLen = aLine.GetCurr()->GetLen();
    while( nHave && aLine.PrevLine() )
    {
        if( nTmpLen )
            --nHave;
        nTmpLen = aLine.GetCurr()->GetLen();
    }
    
    
    
    if( !nHave )
    {
        bool bSplit = true;
        if( !IsFollow() )   
        {
            const WidowsAndOrphans aWidOrp( this );
            bSplit = ( aLine.GetLineNr() >= aWidOrp.GetOrphansLines() &&
                       aLine.GetLineNr() >= aLine.GetDropLines() );
        }

        if( bSplit )
        {
            GetFollow()->SetOfst( aLine.GetEnd() );
            aLine.TruncLines( true );
            if( pPara->IsFollowField() )
                GetFollow()->SetFieldFollow( true );
        }
    }
    if ( bNotify )
    {
        _InvalidateSize();
        InvalidatePage();
    }

    UNDO_SWAP( this )
}

static bool lcl_ErgoVadis( SwTxtFrm* pFrm, sal_Int32 &rPos, const PrepareHint ePrep )
{
    const SwFtnInfo &rFtnInfo = pFrm->GetNode()->GetDoc()->GetFtnInfo();
    if( ePrep == PREP_ERGOSUM )
    {
        if( rFtnInfo.aErgoSum.isEmpty() )
            return false;;
        rPos = pFrm->GetOfst();
    }
    else
    {
        if( rFtnInfo.aQuoVadis.isEmpty() )
            return false;
        if( pFrm->HasFollow() )
            rPos = pFrm->GetFollow()->GetOfst();
        else
            rPos = pFrm->GetTxt().getLength();
        if( rPos )
            --rPos; 
    }
    return true;
}

void SwTxtFrm::Prepare( const PrepareHint ePrep, const void* pVoid,
                        sal_Bool bNotify )
{
    SwFrmSwapper aSwapper( this, false );

#if OSL_DEBUG_LEVEL > 1
    const SwTwips nDbgY = Frm().Top();
    (void)nDbgY;
#endif

    if ( IsEmpty() )
    {
        switch ( ePrep )
        {
            case PREP_BOSS_CHGD:
                SetInvalidVert( sal_True );  
            case PREP_WIDOWS_ORPHANS:
            case PREP_WIDOWS:
            case PREP_FTN_GONE :    return;

            case PREP_POS_CHGD :
            {
                
                
                if( IsInFly() || IsInSct() )
                {
                    SwTwips nTmpBottom = GetUpper()->Frm().Top() +
                        GetUpper()->Prt().Bottom();
                    if( nTmpBottom < Frm().Bottom() )
                        break;
                }
                
                SwTxtFly aTxtFly( this );
                if( aTxtFly.IsOn() )
                {
                    
                    if ( aTxtFly.Relax() || IsUndersized() )
                        break;
                }
                if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue())
                    break;

                GETGRID( FindPageFrm() )
                if ( pGrid && GetTxtNode()->GetSwAttrSet().GetParaGrid().GetValue() )
                    break;

                
                if ( GetDrawObjs() )
                    break;

                return;
            }
            default:
                break;
        }
    }

    if( !HasPara() && PREP_MUST_FIT != ePrep )
    {
        SetInvalidVert( sal_True );  
        OSL_ENSURE( !IsLocked(), "SwTxtFrm::Prepare: three of a perfect pair" );
        if ( bNotify )
            InvalidateSize();
        else
            _InvalidateSize();
        return;
    }

    
    SwTxtLineAccess aAccess( this );
    SwParaPortion *pPara = aAccess.GetPara();

    switch( ePrep )
    {
        case PREP_MOVEFTN :     Frm().Height(0);
                                Prt().Height(0);
                                _InvalidatePrt();
                                _InvalidateSize();
            /* no break here */
        case PREP_ADJUST_FRM :  pPara->SetPrepAdjust();
                                if( IsFtnNumFrm() != pPara->IsFtnNum() ||
                                    IsUndersized() )
                                {
                                    InvalidateRange( SwCharRange( 0, 1 ), 1);
                                    if( GetOfst() && !IsFollow() )
                                        _SetOfst( 0 );
                                }
                                break;
        case PREP_MUST_FIT :        pPara->SetPrepMustFit();
            /* no break here */
        case PREP_WIDOWS_ORPHANS :  pPara->SetPrepAdjust();
                                    break;

        case PREP_WIDOWS :
            
            if( pPara->IsPrepMustFit() )
                return;
            
            PrepWidows( *(const MSHORT *)pVoid, bNotify );
            break;

        case PREP_FTN :
        {
            SwTxtFtn *pFtn = (SwTxtFtn *)pVoid;
            if( IsInFtn() )
            {
                
                if( !GetPrev() )
                    
                    
                    
                    InvalidateRange( SwCharRange( 0, 1 ), 1);

                if( !GetNext() )
                {
                    
                    
                    const SwFtnInfo &rFtnInfo = GetNode()->GetDoc()->GetFtnInfo();
                    if( !pPara->UpdateQuoVadis( rFtnInfo.aQuoVadis ) )
                    {
                        sal_Int32 nPos = pPara->GetParLen();
                        if( nPos )
                            --nPos;
                        InvalidateRange( SwCharRange( nPos, 1 ), 1);
                    }
                }
            }
            else
            {
                
                const sal_Int32 nPos = *pFtn->GetStart();
                InvalidateRange( SwCharRange( nPos, 1 ), 1);
            }
            break;
        }
        case PREP_BOSS_CHGD :
        {
    
            {
                SetInvalidVert( sal_False );
                bool bOld = IsVertical();
                SetInvalidVert( sal_True );
                if( bOld != IsVertical() )
                    InvalidateRange( SwCharRange( GetOfst(), COMPLETE_STRING ) );
            }

            if( HasFollow() )
            {
                sal_Int32 nNxtOfst = GetFollow()->GetOfst();
                if( nNxtOfst )
                    --nNxtOfst;
                InvalidateRange( SwCharRange( nNxtOfst, 1 ), 1);
            }
            if( IsInFtn() )
            {
                sal_Int32 nPos;
                if( lcl_ErgoVadis( this, nPos, PREP_QUOVADIS ) )
                    InvalidateRange( SwCharRange( nPos, 1 ), 0 );
                if( lcl_ErgoVadis( this, nPos, PREP_ERGOSUM ) )
                    InvalidateRange( SwCharRange( nPos, 1 ), 0 );
            }
            
            SwpHints *pHints = GetTxtNode()->GetpSwpHints();
            if( pHints )
            {
                const sal_uInt16 nSize = pHints->Count();
                const sal_Int32 nEnd = GetFollow() ?
                                    GetFollow()->GetOfst() : COMPLETE_STRING;
                for ( sal_uInt16 i = 0; i < nSize; ++i )
                {
                    const SwTxtAttr *pHt = (*pHints)[i];
                    const sal_Int32 nStart = *pHt->GetStart();
                    if( nStart >= GetOfst() )
                    {
                        if( nStart >= nEnd )
                            i = nSize;          
                        else
                        {
                
                
                
                
                            const MSHORT nWhich = pHt->Which();
                            if( RES_TXTATR_FIELD == nWhich ||
                                (HasFtn() && pVoid && RES_TXTATR_FTN == nWhich))
                            InvalidateRange( SwCharRange( nStart, 1 ), 1 );
                        }
                    }
                }
            }
            
            if( IsUndersized() )
            {
                _InvalidateSize();
                InvalidateRange( SwCharRange( GetOfst(), 1 ), 1);
            }
            break;
        }

        case PREP_POS_CHGD :
        {
            if ( GetValidPrtAreaFlag() )
            {
                GETGRID( FindPageFrm() )
                if ( pGrid && GetTxtNode()->GetSwAttrSet().GetParaGrid().GetValue() )
                    InvalidatePrt();
            }

            
            
            bool bFormat = pPara->HasFly();
            if( !bFormat )
            {
                if( IsInFly() )
                {
                    SwTwips nTmpBottom = GetUpper()->Frm().Top() +
                        GetUpper()->Prt().Bottom();
                    if( nTmpBottom < Frm().Bottom() )
                        bFormat = true;
                }
                if( !bFormat )
                {
                    if ( GetDrawObjs() )
                    {
                        const sal_uInt32 nCnt = GetDrawObjs()->Count();
                        for ( MSHORT i = 0; i < nCnt; ++i )
                        {
                            SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
                            
                            
                            if ( pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId()
                                    == FLY_AT_CHAR )
                            {
                                bFormat = true;
                                break;
                            }
                        }
                    }
                    if( !bFormat )
                    {
                        
                        SwTxtFly aTxtFly( this );
                        if( aTxtFly.IsOn() )
                        {
                            
                            bFormat = aTxtFly.Relax() || IsUndersized();
                        }
                    }
                }
            }

            if( bFormat )
            {
                if( !IsLocked() )
                {
                    if( pPara->GetRepaint()->HasArea() )
                        SetCompletePaint();
                    Init();
                    pPara = 0;
                    _InvalidateSize();
                }
            }
            else
            {
                if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue() )
                    Prepare( PREP_REGISTER, 0, bNotify );
                
                
                else if( HasFtn() )
                {
                    Prepare( PREP_ADJUST_FRM, 0, bNotify );
                    _InvalidateSize();
                }
                else
                    return;     
            }
            break;
        }
        case PREP_REGISTER:
            if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue() )
            {
                pPara->SetPrepAdjust();
                CalcLineSpace();
                InvalidateSize();
                _InvalidatePrt();
                SwFrm* pNxt;
                if ( 0 != ( pNxt = GetIndNext() ) )
                {
                    pNxt->_InvalidatePrt();
                    if ( pNxt->IsLayoutFrm() )
                        pNxt->InvalidatePage();
                }
                SetCompletePaint();
            }
            break;
        case PREP_FTN_GONE :
            {
                
                
                
                
                OSL_ENSURE( GetFollow(), "PREP_FTN_GONE darf nur vom Follow gerufen werden" );
                sal_Int32 nPos = GetFollow()->GetOfst();
                if( IsFollow() && GetOfst() == nPos )       
                    FindMaster()->Prepare( PREP_FTN_GONE ); 
                if( nPos )
                    --nPos; 
                InvalidateRange( SwCharRange( nPos, 1 ), 0 );
                return;
            }
        case PREP_ERGOSUM:
        case PREP_QUOVADIS:
            {
                sal_Int32 nPos;
                if( lcl_ErgoVadis( this, nPos, ePrep ) )
                    InvalidateRange( SwCharRange( nPos, 1 ), 0 );
            }
            break;
        case PREP_FLY_ATTR_CHG:
        {
            if( pVoid )
            {
                sal_Int32 nWhere = CalcFlyPos( (SwFrmFmt*)pVoid );
                OSL_ENSURE( COMPLETE_STRING != nWhere, "Prepare: Why me?" );
                InvalidateRange( SwCharRange( nWhere, 1 ) );
                return;
            }
            
        }
        case PREP_CLEAR:
        default:
        {
            if( IsLocked() )
            {
                if( PREP_FLY_ARRIVE == ePrep || PREP_FLY_LEAVE == ePrep )
                {
                    sal_Int32 nLen = ( GetFollow() ? GetFollow()->GetOfst() :
                                      COMPLETE_STRING ) - GetOfst();
                    InvalidateRange( SwCharRange( GetOfst(), nLen ), 0 );
                }
            }
            else
            {
                if( pPara->GetRepaint()->HasArea() )
                    SetCompletePaint();
                Init();
                pPara = 0;
                if( GetOfst() && !IsFollow() )
                    _SetOfst( 0 );
                if ( bNotify )
                    InvalidateSize();
                else
                    _InvalidateSize();
            }
            return;     
        }
    }
    if( pPara )
        pPara->SetPrep();
}

/* --------------------------------------------------
 * Small Helper class:
 * Prepares a test format.
 * The frame is changed in size and position, its SwParaPortion is moved aside
 * and a new one is created.
 * To achieve this, run formatting with bTestFormat flag set.
 * In the destructor the TxtFrm is reset to its original state.
 * --------------------------------------------------*/

class SwTestFormat
{
    SwTxtFrm *pFrm;
    SwParaPortion *pOldPara;
    SwRect aOldFrm, aOldPrt;
public:
    SwTestFormat( SwTxtFrm* pTxtFrm, const SwFrm* pPrv, SwTwips nMaxHeight );
    ~SwTestFormat();
};

SwTestFormat::SwTestFormat( SwTxtFrm* pTxtFrm, const SwFrm* pPre, SwTwips nMaxHeight )
    : pFrm( pTxtFrm )
{
    aOldFrm = pFrm->Frm();
    aOldPrt = pFrm->Prt();

    SWRECTFN( pFrm )
    SwTwips nLower = (pFrm->*fnRect->fnGetBottomMargin)();

    pFrm->Frm() = pFrm->GetUpper()->Prt();
    pFrm->Frm() += pFrm->GetUpper()->Frm().Pos();

    (pFrm->Frm().*fnRect->fnSetHeight)( nMaxHeight );
    if( pFrm->GetPrev() )
        (pFrm->Frm().*fnRect->fnSetPosY)(
                (pFrm->GetPrev()->Frm().*fnRect->fnGetBottom)() -
                ( bVert ? nMaxHeight + 1 : 0 ) );

    SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
    const SwBorderAttrs &rAttrs = *aAccess.Get();
    (pFrm->Prt().*fnRect->fnSetPosX)( rAttrs.CalcLeft( pFrm ) );

    if( pPre )
    {
        SwTwips nUpper = pFrm->CalcUpperSpace( &rAttrs, pPre );
        (pFrm->Prt().*fnRect->fnSetPosY)( nUpper );
    }
    (pFrm->Prt().*fnRect->fnSetHeight)(
        std::max( 0L , (pFrm->Frm().*fnRect->fnGetHeight)() -
                  (pFrm->Prt().*fnRect->fnGetTop)() - nLower ) );
    (pFrm->Prt().*fnRect->fnSetWidth)(
        (pFrm->Frm().*fnRect->fnGetWidth)() -
        ( rAttrs.CalcLeft( pFrm ) + rAttrs.CalcRight( pFrm ) ) );
    pOldPara = pFrm->HasPara() ? pFrm->GetPara() : NULL;
    pFrm->SetPara( new SwParaPortion(), false );

    OSL_ENSURE( ! pFrm->IsSwapped(), "A frame is swapped before _Format" );

    if ( pFrm->IsVertical() )
        pFrm->SwapWidthAndHeight();

    SwTxtFormatInfo aInf( pFrm, false, true, true );
    SwTxtFormatter  aLine( pFrm, &aInf );

    pFrm->_Format( aLine, aInf );

    if ( pFrm->IsVertical() )
        pFrm->SwapWidthAndHeight();

    OSL_ENSURE( ! pFrm->IsSwapped(), "A frame is swapped after _Format" );
}

SwTestFormat::~SwTestFormat()
{
    pFrm->Frm() = aOldFrm;
    pFrm->Prt() = aOldPrt;
    pFrm->SetPara( pOldPara );
}

sal_Bool SwTxtFrm::TestFormat( const SwFrm* pPrv, SwTwips &rMaxHeight, sal_Bool &bSplit )
{
    PROTOCOL_ENTER( this, PROT_TESTFORMAT, 0, 0 )

    if( IsLocked() && GetUpper()->Prt().Width() <= 0 )
        return sal_False;

    SwTestFormat aSave( this, pPrv, rMaxHeight );

    return SwTxtFrm::WouldFit( rMaxHeight, bSplit, sal_True );
}

/* SwTxtFrm::WouldFit()
 * sal_True: wenn ich aufspalten kann.
 * Es soll und braucht nicht neu formatiert werden.
 * Wir gehen davon aus, dass bereits formatiert wurde und dass
 * die Formatierungsdaten noch aktuell sind.
 * Wir gehen davon aus, dass die Framebreiten des evtl. Masters und
 * Follows gleich sind. Deswegen wird kein FindBreak() mit FindOrphans()
 * gerufen.
 * Die benoetigte Hoehe wird von nMaxHeight abgezogen!
 */

sal_Bool SwTxtFrm::WouldFit( SwTwips &rMaxHeight, sal_Bool &bSplit, sal_Bool bTst )
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTxtFrm::WouldFit with swapped frame" );
    SWRECTFN( this );

    if( IsLocked() )
        return sal_False;

    
    if( !IsEmpty() )
        GetFormatted();

    
    
    
    
    if ( IsEmpty() && !bTst )
    {
        bSplit = sal_False;
        SwTwips nHeight = bVert ? Prt().SSize().Width() : Prt().SSize().Height();
        if( rMaxHeight < nHeight )
            return sal_False;
        else
        {
            rMaxHeight -= nHeight;
            return sal_True;
        }
    }

    
    
    
    OSL_ENSURE( HasPara() || IsHiddenNow(), "WouldFit: GetFormatted() and then !HasPara()" );
    if( !HasPara() || ( !(Frm().*fnRect->fnGetHeight)() && IsHiddenNow() ) )
        return sal_True;

    
    
    
    if( IsWidow() || ( bVert ?
                       ( 0 == Frm().Left() ) :
                       ( LONG_MAX - 20000 < Frm().Bottom() ) ) )
    {
        SetWidow(false);
        if ( GetFollow() )
        {
            
            
            
            
            
            if( ( ( ! bVert && LONG_MAX - 20000 >= Frm().Bottom() ) ||
                  (   bVert && 0 < Frm().Left() ) ) &&
                  ( GetFollow()->IsVertical() ?
                    !GetFollow()->Frm().Width() :
                    !GetFollow()->Frm().Height() ) )
            {
                SwTxtFrm* pFoll = GetFollow()->GetFollow();
                while( pFoll &&
                        ( pFoll->IsVertical() ?
                         !pFoll->Frm().Width() :
                         !pFoll->Frm().Height() ) )
                    pFoll = pFoll->GetFollow();
                if( pFoll )
                    return sal_False;
            }
            else
                return sal_False;
        }
    }

    SWAP_IF_NOT_SWAPPED( this );

    SwTxtSizeInfo aInf( this );
    SwTxtMargin aLine( this, &aInf );

    WidowsAndOrphans aFrmBreak( this, rMaxHeight, bSplit );

    sal_Bool bRet = sal_True;

    aLine.Bottom();
    
    bSplit = !aFrmBreak.IsInside( aLine );
    if ( bSplit )
        bRet = !aFrmBreak.IsKeepAlways() && aFrmBreak.WouldFit( aLine, rMaxHeight, bTst );
    else
    {
        
        aLine.Top();
        do
        {
            rMaxHeight -= aLine.GetLineHeight();
        } while ( aLine.Next() );
    }

    UNDO_SWAP( this )

    return bRet;
}

KSHORT SwTxtFrm::GetParHeight() const
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTxtFrm::GetParHeight with swapped frame" );

    if( !HasPara() )
    {   
        
        KSHORT nRet = (KSHORT)Prt().SSize().Height();
        if( IsUndersized() )
        {
            if( IsEmpty() || GetTxt().isEmpty() )
                nRet = (KSHORT)EmptyHeight();
            else
                ++nRet;
        }
        return nRet;
    }

    
    const SwLineLayout* pLineLayout = GetPara();
    KSHORT nHeight = pLineLayout->GetRealHeight();
    if( GetOfst() && !IsFollow() )  
        nHeight *= 2;               
    
    while ( pLineLayout && pLineLayout->GetNext() )
    {
        pLineLayout = pLineLayout->GetNext();
        nHeight = nHeight + pLineLayout->GetRealHeight();
    }

    return nHeight;
}


SwTxtFrm* SwTxtFrm::GetFormatted( bool bForceQuickFormat )
{
    SWAP_IF_SWAPPED( this )

    
    
                      
    if( !HasPara() && !(IsValid() && IsEmpty()) )
    {
        
        const sal_Bool bFormat = GetValidSizeFlag();
        Calc();
        
        
        
        
        if( bFormat && !FormatQuick( bForceQuickFormat ) )
            Format();
    }

    UNDO_SWAP( this )

    return this;
}

SwTwips SwTxtFrm::CalcFitToContent()
{
    
    
    
    if ( IsLocked() )
        return Prt().Width();

    SwParaPortion* pOldPara = GetPara();
    SwParaPortion *pDummy = new SwParaPortion();
    SetPara( pDummy, false );
    const SwPageFrm* pPage = FindPageFrm();

    const Point   aOldFrmPos   = Frm().Pos();
    const SwTwips nOldFrmWidth = Frm().Width();
    const SwTwips nOldPrtWidth = Prt().Width();
    const SwTwips nPageWidth = GetUpper()->IsVertical() ?
                               pPage->Prt().Height() :
                               pPage->Prt().Width();

    Frm().Width( nPageWidth );
    Prt().Width( nPageWidth );

    
    if ( IsRightToLeft() )
        Frm().Pos().X() += nOldFrmWidth - nPageWidth;

    
    SwTxtFrmLocker aLock( this );

    SwTxtFormatInfo aInf( this, false, true, true );
    aInf.SetIgnoreFly( true );
    SwTxtFormatter  aLine( this, &aInf );
    SwHookOut aHook( aInf );

    
    const SwTwips nMax = std::max( (SwTwips)MINLAY,
                              aLine._CalcFitToContent() + 1 );

    Frm().Width( nOldFrmWidth );
    Prt().Width( nOldPrtWidth );

    
    if ( IsRightToLeft() )
        Frm().Pos() = aOldFrmPos;


    SetPara( pOldPara );

    return nMax;
}

/** simulate format for a list item paragraph, whose list level attributes
    are in LABEL_ALIGNMENT mode, in order to determine additional first
    line offset for the real text formatting due to the value of label
    adjustment attribute of the list level.
*/
void SwTxtFrm::CalcAdditionalFirstLineOffset()
{
    if ( IsLocked() )
        return;

    
    mnAdditionalFirstLineOffset = 0;

    const SwTxtNode* pTxtNode( GetTxtNode() );
    if ( pTxtNode && pTxtNode->IsNumbered() && pTxtNode->IsCountedInList() &&
         pTxtNode->GetNumRule() )
    {
        int nListLevel = pTxtNode->GetActualListLevel();

        if (nListLevel < 0)
            nListLevel = 0;

        if (nListLevel >= MAXLEVEL)
            nListLevel = MAXLEVEL - 1;

        const SwNumFmt& rNumFmt =
                pTxtNode->GetNumRule()->Get( static_cast<sal_uInt16>(nListLevel) );
        if ( rNumFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            
            SwParaPortion* pOldPara = GetPara();
            SwParaPortion *pDummy = new SwParaPortion();
            SetPara( pDummy, false );

            
            SwTxtFrmLocker aLock( this );

            
            SwTxtFormatInfo aInf( this, false, true, true );
            aInf.SetIgnoreFly( true );
            SwTxtFormatter aLine( this, &aInf );
            SwHookOut aHook( aInf );
            aLine._CalcFitToContent();

            
            const SwLinePortion* pFirstPortion = aLine.GetCurr()->GetFirstPortion();
            if ( pFirstPortion->InNumberGrp() && !pFirstPortion->IsFtnNumPortion() )
            {
                SwTwips nNumberPortionWidth( pFirstPortion->Width() );

                const SwLinePortion* pPortion = pFirstPortion->GetPortion();
                while ( pPortion &&
                        pPortion->InNumberGrp() && !pPortion->IsFtnNumPortion())
                {
                    nNumberPortionWidth += pPortion->Width();
                    pPortion = pPortion->GetPortion();
                }

                if ( ( IsRightToLeft() &&
                       rNumFmt.GetNumAdjust() == SVX_ADJUST_LEFT ) ||
                     ( !IsRightToLeft() &&
                       rNumFmt.GetNumAdjust() == SVX_ADJUST_RIGHT ) )
                {
                    mnAdditionalFirstLineOffset = -nNumberPortionWidth;
                }
                else if ( rNumFmt.GetNumAdjust() == SVX_ADJUST_CENTER )
                {
                    mnAdditionalFirstLineOffset = -(nNumberPortionWidth/2);
                }
            }

            
            SetPara( pOldPara );
        }
    }
}

/** determine height of last line for the calculation of the proportional line
    spacing

    OD 08.01.2004 #i11859#
    OD 2004-03-17 #i11860# - method <GetHeightOfLastLineForPropLineSpacing()>
    replace by method <_CalcHeightOfLastLine()>. Height of last line will be
    stored in new member <mnHeightOfLastLine> and can be accessed via method
    <GetHeightOfLastLine()>
    OD 2005-05-20 #i47162# - introduce new optional parameter <_bUseFont>
    in order to force the usage of the former algorithm to determine the
    height of the last line, which uses the font.
*/
void SwTxtFrm::_CalcHeightOfLastLine( const bool _bUseFont )
{
    
    
    const SwTwips mnOldHeightOfLastLine( mnHeightOfLastLine );
    
    SwViewShell* pVsh = getRootFrm()->GetCurrShell();
    OSL_ENSURE( pVsh, "<SwTxtFrm::_GetHeightOfLastLineForPropLineSpacing()> - no SwViewShell" );
    
    
    
    if ( !pVsh )
    {
        return;
    }
    OutputDevice* pOut = pVsh->GetOut();
    const IDocumentSettingAccess* pIDSA = GetTxtNode()->getIDocumentSettingAccess();
    if ( !pVsh->GetViewOptions()->getBrowseMode() ||
          pVsh->GetViewOptions()->IsPrtFormat() )
    {
        pOut = GetTxtNode()->getIDocumentDeviceAccess()->getReferenceDevice( true );
    }
    OSL_ENSURE( pOut, "<SwTxtFrm::_GetHeightOfLastLineForPropLineSpacing()> - no OutputDevice" );
    
    if ( !pOut )
    {
        return;
    }

    

    if ( _bUseFont || pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING ) )
    {
        
        
        SwFont aFont( GetAttrSet(), pIDSA );

        
        
        if ( pLastFont )
        {
            SwFntObj *pOldFont = pLastFont;
            pLastFont = NULL;
            aFont.SetFntChg( sal_True );
            aFont.ChgPhysFnt( pVsh, *pOut );
            mnHeightOfLastLine = aFont.GetHeight( pVsh, *pOut );
            pLastFont->Unlock();
            pLastFont = pOldFont;
            pLastFont->SetDevFont( pVsh, *pOut );
        }
        else
        {
            Font aOldFont = pOut->GetFont();
            aFont.SetFntChg( sal_True );
            aFont.ChgPhysFnt( pVsh, *pOut );
            mnHeightOfLastLine = aFont.GetHeight( pVsh, *pOut );
            pLastFont->Unlock();
            pLastFont = NULL;
            pOut->SetFont( aOldFont );
        }
    }
    else
    {
        
        
        
        if ( IsUndersized() )
        {
            mnHeightOfLastLine = 0;
        }
        else
        {
            bool bCalcHeightOfLastLine = true;
            if ( ( !HasPara() && IsEmpty( ) ) || GetTxt().isEmpty() )
            {
                mnHeightOfLastLine = EmptyHeight();
                bCalcHeightOfLastLine = false;
            }

            if ( bCalcHeightOfLastLine )
            {
                OSL_ENSURE( HasPara(),
                        "<SwTxtFrm::_CalcHeightOfLastLine()> - missing paragraph portions." );
                const SwLineLayout* pLineLayout = GetPara();
                while ( pLineLayout && pLineLayout->GetNext() )
                {
                    
                    pLineLayout = pLineLayout->GetNext();
                }
                if ( pLineLayout )
                {
                    SwTwips nAscent, nDescent, nDummy1, nDummy2;
                    
                    
                    pLineLayout->MaxAscentDescent( nAscent, nDescent,
                                                   nDummy1, nDummy2,
                                                   0, true );
                    
                    
                    
                    
                    
                    const SwTwips nNewHeightOfLastLine = nAscent + nDescent;
                    
                    
                    
                    if ( nNewHeightOfLastLine == 0 )
                    {
                        _CalcHeightOfLastLine( true );
                    }
                    else
                    {
                        mnHeightOfLastLine = nNewHeightOfLastLine;
                    }
                }
            }
        }
    }
    
    
    if ( mnHeightOfLastLine != mnOldHeightOfLastLine )
    {
        InvalidatePrt();
    }
}










long SwTxtFrm::GetLineSpace( const bool _bNoPropLineSpace ) const
{
    long nRet = 0;

    const SwAttrSet* pSet = GetAttrSet();
    const SvxLineSpacingItem &rSpace = pSet->GetLineSpacing();

    switch( rSpace.GetInterLineSpaceRule() )
    {
        case SVX_INTER_LINE_SPACE_PROP:
        {
            
            if ( _bNoPropLineSpace )
            {
                break;
            }

            
            nRet = GetHeightOfLastLine();

            long nTmp = nRet;
            nTmp *= rSpace.GetPropLineSpace();
            nTmp /= 100;
            nTmp -= nRet;
            if ( nTmp > 0 )
                nRet = nTmp;
            else
                nRet = 0;
        }
            break;
        case SVX_INTER_LINE_SPACE_FIX:
        {
            if ( rSpace.GetInterLineSpace() > 0 )
                nRet = rSpace.GetInterLineSpace();
        }
            break;
        default:
            break;
    }
    return nRet;
}

KSHORT SwTxtFrm::FirstLineHeight() const
{
    if ( !HasPara() )
    {
        if( IsEmpty() && IsValid() )
            return IsVertical() ? (KSHORT)Prt().Width() : (KSHORT)Prt().Height();
        return KSHRT_MAX;
    }
    const SwParaPortion *pPara = GetPara();
    if ( !pPara )
        return KSHRT_MAX;

    return pPara->Height();
}

MSHORT SwTxtFrm::GetLineCount( sal_Int32 nPos )
{
    MSHORT nRet = 0;
    SwTxtFrm *pFrm = this;
    do
    {
        pFrm->GetFormatted();
        if( !pFrm->HasPara() )
            break;
        SwTxtSizeInfo aInf( pFrm );
        SwTxtMargin aLine( pFrm, &aInf );
        if( COMPLETE_STRING == nPos )
            aLine.Bottom();
        else
            aLine.CharToLine( nPos );
        nRet = nRet + aLine.GetLineNr();
        pFrm = pFrm->GetFollow();
    } while ( pFrm && pFrm->GetOfst() <= nPos );
    return nRet;
}

void SwTxtFrm::ChgThisLines()
{
    
    sal_uLong nNew = 0;
    const SwLineNumberInfo &rInf = GetNode()->getIDocumentLineNumberAccess()->GetLineNumberInfo();
    if ( !GetTxt().isEmpty() && HasPara() )
    {
        SwTxtSizeInfo aInf( this );
        SwTxtMargin aLine( this, &aInf );
        if ( rInf.IsCountBlankLines() )
        {
            aLine.Bottom();
            nNew = (sal_uLong)aLine.GetLineNr();
        }
        else
        {
            do
            {
                if( aLine.GetCurr()->HasCntnt() )
                    ++nNew;
            } while ( aLine.NextLine() );
        }
    }
    else if ( rInf.IsCountBlankLines() )
        nNew = 1;

    if ( nNew != nThisLines )
    {
        if ( !IsInTab() && GetAttrSet()->GetLineNumber().IsCount() )
        {
            nAllLines -= nThisLines;
            nThisLines = nNew;
            nAllLines  += nThisLines;
            SwFrm *pNxt = GetNextCntntFrm();
            while( pNxt && pNxt->IsInTab() )
            {
                if( 0 != (pNxt = pNxt->FindTabFrm()) )
                    pNxt = pNxt->FindNextCnt();
            }
            if( pNxt )
                pNxt->InvalidateLineNum();

            
            if ( HasPara() )
            {
                SwRepaint *pRepaint = GetPara()->GetRepaint();
                pRepaint->Bottom( std::max( pRepaint->Bottom(),
                                       Frm().Top()+Prt().Bottom()));
            }
        }
        else 
            nThisLines = nNew;
    }
}


void SwTxtFrm::RecalcAllLines()
{
    ValidateLineNum();

    const SwAttrSet *pAttrSet = GetAttrSet();

    if ( !IsInTab() )
    {
        const sal_uLong nOld = GetAllLines();
        const SwFmtLineNumber &rLineNum = pAttrSet->GetLineNumber();
        sal_uLong nNewNum;
        const bool bRestart = GetTxtNode()->getIDocumentLineNumberAccess()->GetLineNumberInfo().IsRestartEachPage();

        if ( !IsFollow() && rLineNum.GetStartValue() && rLineNum.IsCount() )
            nNewNum = rLineNum.GetStartValue() - 1;
        
        
        else if ( bRestart && FindPageFrm()->FindFirstBodyCntnt() == this )
        {
            nNewNum = 0;
        }
        else
        {
            SwCntntFrm *pPrv = GetPrevCntntFrm();
            while ( pPrv &&
                    (pPrv->IsInTab() || pPrv->IsInDocBody() != IsInDocBody()) )
                pPrv = pPrv->GetPrevCntntFrm();

            
            
            if ( bRestart && pPrv && pPrv->FindPageFrm() != FindPageFrm() )
                pPrv = 0;

            nNewNum = pPrv ? ((SwTxtFrm*)pPrv)->GetAllLines() : 0;
        }
        if ( rLineNum.IsCount() )
            nNewNum += GetThisLines();

        if ( nOld != nNewNum )
        {
            nAllLines = nNewNum;
            SwCntntFrm *pNxt = GetNextCntntFrm();
            while ( pNxt &&
                    (pNxt->IsInTab() || pNxt->IsInDocBody() != IsInDocBody()) )
                pNxt = pNxt->GetNextCntntFrm();
            if ( pNxt )
            {
                if ( pNxt->GetUpper() != GetUpper() )
                    pNxt->InvalidateLineNum();
                else
                    pNxt->_InvalidateLineNum();
            }
        }
    }
}

void SwTxtFrm::VisitPortions( SwPortionHandler& rPH ) const
{
    const SwParaPortion* pPara = GetPara();

    if( pPara )
    {
        if ( IsFollow() )
            rPH.Skip( GetOfst() );

        const SwLineLayout* pLine = pPara;
        while ( pLine )
        {
            const SwLinePortion* pPor = pLine->GetFirstPortion();
            while ( pPor )
            {
                pPor->HandlePortion( rPH );
                pPor = pPor->GetPortion();
            }

            rPH.LineBreak(pLine->Width());
            pLine = pLine->GetNext();
        }
    }

    rPH.Finish();
}

const SwScriptInfo* SwTxtFrm::GetScriptInfo() const
{
    const SwParaPortion* pPara = GetPara();
    return pPara ? &pPara->GetScriptInfo() : 0;
}


static SwTwips lcl_CalcFlyBasePos( const SwTxtFrm& rFrm, SwRect aFlyRect,
                            SwTxtFly& rTxtFly )
{
    SWRECTFN( (&rFrm) )
    SwTwips nRet = rFrm.IsRightToLeft() ?
                   (rFrm.Frm().*fnRect->fnGetRight)() :
                   (rFrm.Frm().*fnRect->fnGetLeft)();

    do
    {
        SwRect aRect = rTxtFly.GetFrm( aFlyRect );
        if ( 0 != (aRect.*fnRect->fnGetWidth)() )
        {
            if ( rFrm.IsRightToLeft() )
            {
                if ( (aRect.*fnRect->fnGetRight)() -
                     (aFlyRect.*fnRect->fnGetRight)() >= 0 )
                {
                    (aFlyRect.*fnRect->fnSetRight)(
                        (aRect.*fnRect->fnGetLeft)() );
                    nRet = (aRect.*fnRect->fnGetLeft)();
                }
                else
                    break;
            }
            else
            {
                if ( (aFlyRect.*fnRect->fnGetLeft)() -
                     (aRect.*fnRect->fnGetLeft)() >= 0 )
                {
                    (aFlyRect.*fnRect->fnSetLeft)(
                        (aRect.*fnRect->fnGetRight)() + 1 );
                    nRet = (aRect.*fnRect->fnGetRight)();
                }
                else
                    break;
            }
        }
        else
            break;
    }
    while ( (aFlyRect.*fnRect->fnGetWidth)() > 0 );

    return nRet;
}

void SwTxtFrm::CalcBaseOfstForFly()
{
    OSL_ENSURE( !IsVertical() || !IsSwapped(),
            "SwTxtFrm::CalcBasePosForFly with swapped frame!" );

    const SwNode* pNode = GetTxtNode();
    if ( !pNode->getIDocumentSettingAccess()->get(IDocumentSettingAccess::ADD_FLY_OFFSETS) )
        return;

    SWRECTFN( this )

    SwRect aFlyRect( Frm().Pos() + Prt().Pos(), Prt().SSize() );

    
    
    
    {
        SwTwips nTop = (aFlyRect.*fnRect->fnGetTop)();
        const SwLineLayout* pLay = GetPara();
        SwTwips nLineHeight = 200;
        while( pLay && pLay->IsDummy() && pLay->GetNext() )
        {
            nTop += pLay->Height();
            pLay = pLay->GetNext();
        }
        if ( pLay )
        {
            nLineHeight = pLay->Height();
        }
        (aFlyRect.*fnRect->fnSetTopAndHeight)( nTop, nLineHeight );
    }

    SwTxtFly aTxtFly( this );
    aTxtFly.SetIgnoreCurrentFrame( true );
    aTxtFly.SetIgnoreContour( true );
    
    
    aTxtFly.SetIgnoreObjsInHeaderFooter( true );
    SwTwips nRet1 = lcl_CalcFlyBasePos( *this, aFlyRect, aTxtFly );
    aTxtFly.SetIgnoreCurrentFrame( false );
    SwTwips nRet2 = lcl_CalcFlyBasePos( *this, aFlyRect, aTxtFly );

    
    SwTwips nLeft = IsRightToLeft() ?
                    (Frm().*fnRect->fnGetRight)() :
                    (Frm().*fnRect->fnGetLeft)();

    mnFlyAnchorOfst = nRet1 - nLeft;
    mnFlyAnchorOfstNoWrap = nRet2 - nLeft;
}

/* repaint all text frames of the given text node */
void SwTxtFrm::repaintTextFrames( const SwTxtNode& rNode )
{
    SwIterator<SwTxtFrm,SwTxtNode> aIter( rNode );
    for( const SwTxtFrm *pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
    {
        SwRect aRec( pFrm->PaintArea() );
        const SwRootFrm *pRootFrm = pFrm->getRootFrm();
        SwViewShell *pCurShell = pRootFrm ? pRootFrm->GetCurrShell() : NULL;
        if( pCurShell )
            pCurShell->InvalidateWindows( aRec );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
