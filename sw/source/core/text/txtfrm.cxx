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
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
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
#include <ftnidx.hxx>

TYPEINIT1( SwTxtFrm, SwCntntFrm );

// Switches width and height of the text frame
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

// Calculates the coordinates of a rectangle when switching from
// horizontal to vertical layout.
void SwTxtFrm::SwitchHorizontalToVertical( SwRect& rRect ) const
{
    // calc offset inside frame
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
            // frame is rotated
            rRect.Left( Frm().Left() + Frm().Width() - nOfstY );
    }

    rRect.Top( Frm().Top() + nOfstX );
    rRect.Width( nHeight );
    rRect.Height( nWidth );
}

// Calculates the coordinates of a point when switching from
// horizontal to vertical layout.
void SwTxtFrm::SwitchHorizontalToVertical( Point& rPoint ) const
{
    // calc offset inside frame
    const long nOfstX = rPoint.X() - Frm().Left();
    const long nOfstY = rPoint.Y() - Frm().Top();
    if ( IsVertLR() )
        rPoint.X() = Frm().Left() + nOfstY;
    else
    {
        if ( bIsSwapped )
            rPoint.X() = Frm().Left() + Frm().Height() - nOfstY;
        else
            // calc rotated coords
            rPoint.X() = Frm().Left() + Frm().Width() - nOfstY;
    }

    rPoint.Y() = Frm().Top() + nOfstX;
}

// Calculates the a limit value when switching from
// horizontal to vertical layout.
long SwTxtFrm::SwitchHorizontalToVertical( long nLimit ) const
{
    Point aTmp( 0, nLimit );
    SwitchHorizontalToVertical( aTmp );
    return aTmp.X();
}

// Calculates the coordinates of a rectangle when switching from
// vertical to horizontal layout.
void SwTxtFrm::SwitchVerticalToHorizontal( SwRect& rRect ) const
{
    long nOfstX;

    // calc offset inside frame

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

    // calc rotated coords
    rRect.Left( Frm().Left() + nOfstY );
    rRect.Top( Frm().Top() + nOfstX );
    rRect.Width( nWidth );
    rRect.Height( nHeight );
}

// Calculates the coordinates of a point when switching from
// vertical to horizontal layout.
void SwTxtFrm::SwitchVerticalToHorizontal( Point& rPoint ) const
{
    long nOfstX;

    // calc offset inside frame

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

    // calc rotated coords
    rPoint.X() = Frm().Left() + nOfstY;
    rPoint.Y() = Frm().Top() + nOfstX;
}

// Calculates the a limit value when switching from
// vertical to horizontal layout.
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
    const ComplexTextLayoutMode nNewLayoutMode = nOldLayoutMode & ~TEXT_LAYOUT_BIDI_STRONG;
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
        // set flags directly to save a ResetPreps call,
        // and thereby an unnecessary GetPara call
        // don't set bOrphan, bLocked or bWait to false!
        // bOrphan = bFlag7 = bFlag8 = false;
    }
}

SwTxtFrm::SwTxtFrm(SwTxtNode * const pNode, SwFrm* pSib )
    : SwCntntFrm( pNode, pSib )
    , nAllLines( 0 )
    , nThisLines( 0 )
    , mnFlyAnchorOfst( 0 )
    , mnFlyAnchorOfstNoWrap( 0 )
    , mnFtnLine( 0 )
    , mnHeightOfLastLine( 0 ) // OD 2004-03-17 #i11860#
    , mnAdditionalFirstLineOffset( 0 )
    , nOfst( 0 )
    , nCacheIdx( USHRT_MAX )
    , bLocked( false )
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
    , mbFollowFormatAllowed( true ) // OD 14.03.2003 #i11760#
{
    mnType = FRMC_TXT;
}

SwTxtFrm::~SwTxtFrm()
{
    // Remove associated SwParaPortion from pTxtCache
    ClearPara();

    SwCntntNode* pCNd;
    if( 0 != ( pCNd = PTR_CAST( SwCntntNode, GetRegisteredIn() )) &&
        !pCNd->GetDoc()->IsInDtor() && HasFtn() )
    {
        SwTxtNode *pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
        const SwFtnIdxs &rFtnIdxs = pCNd->GetDoc()->GetFtnIdxs();
        size_t nPos = 0;
        sal_uLong nIndex = pCNd->GetIndex();
        rFtnIdxs.SeekEntry( *pTxtNd, &nPos );
        SwTxtFtn* pTxtFtn;
        if( nPos < rFtnIdxs.size() )
        {
            while( nPos && pTxtNd == &(rFtnIdxs[ nPos ]->GetTxtNode()) )
                --nPos;
            if( nPos || pTxtNd != &(rFtnIdxs[ nPos ]->GetTxtNode()) )
                ++nPos;
        }
        while( nPos < rFtnIdxs.size() )
        {
            pTxtFtn = rFtnIdxs[ nPos ];
            if( pTxtFtn->GetTxtNode().GetIndex() > nIndex )
                break;
            pTxtFtn->DelFrms( this );
            ++nPos;
        }
    }
}

const OUString& SwTxtFrm::GetTxt() const
{
    return GetTxtNode()->GetTxt();
}

void SwTxtFrm::ResetPreps()
{
    if ( GetCacheIdx() != USHRT_MAX )
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
                                       // invalid when stack overflows (StackHack)!
    {
//        OSL_FAIL( "SwTxtFrm::IsHiddenNow: thin frame" );
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

// removes Textfrm's attachments, when it's hidden
void SwTxtFrm::HideHidden()
{
    OSL_ENSURE( !GetFollow() && IsHiddenNow(),
            "HideHidden on visible frame of hidden frame has follow" );

    const sal_Int32 nEnd = COMPLETE_STRING;
    HideFootnotes( GetOfst(), nEnd );
    // OD 2004-01-15 #110582#
    HideAndShowObjects();

    // format information is obsolete
    ClearPara();
}

void SwTxtFrm::HideFootnotes( sal_Int32 nStart, sal_Int32 nEnd )
{
    const SwpHints *pHints = GetTxtNode()->GetpSwpHints();
    if( pHints )
    {
        const size_t nSize = pHints->Count();
        SwPageFrm *pPage = 0;
        for ( size_t i = 0; i < nSize; ++i )
        {
            const SwTxtAttr *pHt = (*pHints)[i];
            if ( pHt->Which() == RES_TXTATR_FTN )
            {
                const sal_Int32 nIdx = pHt->GetStart();
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

// #120729# - hotfix
// as-character anchored graphics, which are used for a graphic bullet list.
// As long as these graphic bullet list aren't imported, do not hide a
// at-character anchored object, if
// (a) the document is an imported WW8 document -
//     checked by checking certain compatibility options -,
// (b) the paragraph is the last content in the document and
// (c) the anchor character is an as-character anchored graphic.
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
                            // set needed data structure values for object positioning
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
            // complete paragraph is hidden. Thus, hide all objects
            for ( size_t i = 0; i < GetDrawObjs()->size(); ++i )
            {
                SdrObject* pObj = (*GetDrawObjs())[i]->DrawObj();
                SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
                // #120729# - hotfix
                // under certain conditions
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
            // paragraph is visible, but can contain hidden text portion.
            // first we check if objects are allowed to be hidden:
            const SwTxtNode& rNode = *GetTxtNode();
            const SwViewShell* pVsh = getRootFrm()->GetCurrShell();
            const bool bShouldBeHidden = !pVsh || !pVsh->GetWin() ||
                                         !pVsh->GetViewOptions()->IsShowHiddenChar();

            // Thus, show all objects, which are anchored at paragraph and
            // hide/show objects, which are anchored at/as character, according
            // to the visibility of the anchor character.
            for ( size_t i = 0; i < GetDrawObjs()->size(); ++i )
            {
                SdrObject* pObj = (*GetDrawObjs())[i]->DrawObj();
                SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
                // #120729# - determine anchor type only once
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
                    // #120729# - hotfix
                    // under certain conditions
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

// Returns the first possible break point in the current line.
// This method is used in SwTxtFrm::Format() to decide whether the previous
// line has to be formatted as well.
// nFound is <= nEndLine.
sal_Int32 SwTxtFrm::FindBrk( const OUString &rTxt,
                              const sal_Int32 nStart,
                              const sal_Int32 nEnd ) const
{
    sal_Int32 nFound = nStart;
    const sal_Int32 nEndLine = std::min( nEnd, rTxt.getLength() - 1 );

    // skip all leading blanks (see bug #2235).
    while( nFound <= nEndLine && ' ' == rTxt[nFound] )
    {
         nFound++;
    }

    // A tricky situation with the TxtAttr-Dummy-character (in this case "$"):
    // "Dr.$Meyer" at the beginning of the second line. Typing a blank after that
    // doesn't result in the word moving into first line, even though that would work.
    // For this reason we don't skip the dummy char.
    while( nFound <= nEndLine && ' ' != rTxt[nFound] )
    {
        nFound++;
    }

    return nFound;
}

bool SwTxtFrm::IsIdxInside( const sal_Int32 nPos, const sal_Int32 nLen ) const
{
    if( nLen != COMPLETE_STRING && GetOfst() > nPos + nLen ) // the range preceded us
        return false;

    if( !GetFollow() )            // the range doesn't precede us,
        return true;          // nobody follows us.

    const sal_Int32 nMax = GetFollow()->GetOfst();

    // either the range overlap or our text has been deleted
    if( nMax > nPos || nMax > GetTxt().getLength() )
        return true;

    // changes made in the first line of a follow can modify the master
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
        // In nDelta the differences betwen old and new
        // linelengths are being added, that's why it's negative
        // if chars have been added and positive, if chars have
        // deleted
        pPara->GetDelta() += nD;
        bInv = true;
    }
    SwCharRange &rReformat = pPara->GetReformat();
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
    // 4291: underflow with free-flying frames
    if( aInf.GetTxtFly().IsOn() )
    {
        SwRect aTmpFrm( Frm() );
        if( nDelta < 0 )
            aTmpFrm.Height( Prt().Height() );
        else
            aTmpFrm.Height( aNewSize.Height() );
        if( aInf.GetTxtFly().Relax( aTmpFrm ) )
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
            // if there is a Follow() or if we need to break here, reformat
            Init();
        }
        else
        {
            // everything is business as usual...
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
            pFrm->ManipOfst( pFrm->GetTxtNode()->GetTxt().getLength() );
        else
            pFrm->ManipOfst( pFrm->GetOfst() + nLen );
        pFrm = pFrm->GetFollow();
    }
}

// Related: fdo#56031 filter out attribute changes that don't matter for
// humans/a11y to stop flooding the destination mortal with useless noise
static bool isA11yRelevantAttribute(sal_uInt16 nWhich)
{
    return nWhich != RES_CHRATR_RSID;
}

static bool hasA11yRelevantAttribute( const std::vector<sal_uInt16>& nWhich )
{
    for( std::vector<sal_uInt16>::const_iterator nItr = nWhich.begin();
            nItr < nWhich.end(); ++nItr )
        if ( isA11yRelevantAttribute( *nItr ) )
            return true;

    return false;
}

void SwTxtFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;

    // modifications concerning frame attributes are processed by the base class
    if( IsInRange( aFrmFmtSetRange, nWhich ) || RES_FMT_CHG == nWhich )
    {
        SwCntntFrm::Modify( pOld, pNew );
        if( nWhich == RES_FMT_CHG && getRootFrm()->GetCurrShell() )
        {
            // collection has changed
            Prepare( PREP_CLEAR );
            _InvalidatePrt();
            lcl_SetWrong( *this, 0, COMPLETE_STRING, false );
            SetDerivedR2L( false );
            CheckDirChange();
            // OD 09.12.2002 #105576# - Force complete paint due to existing
            // indents.
            SetCompletePaint();
            InvalidateLineNum();
        }
        return;
    }

    // while locked ignore all modifications
    if( IsLocked() )
        return;

    // save stack
    // warning: one has to ensure that all variables are set
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
                    // 6969: refresh NumPortions even when line is empty!
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
            nPos = ((SwUpdateAttr*)pNew)->getStart();
            nLen = ((SwUpdateAttr*)pNew)->getEnd() - nPos;
            if( IsIdxInside( nPos, nLen ) )
            {
                // Es muss in jedem Fall neu formatiert werden,
                // auch wenn der invalidierte Bereich null ist.
                // Beispiel: leere Zeile, 14Pt einstellen !
                // if( !nLen ) nLen = 1;

                // 6680: FtnNummern muessen formatiert werden.
                if( !nLen )
                    nLen = 1;

                _InvalidateRange( SwCharRange( nPos, nLen) );
                const sal_uInt16 nTmp = ((SwUpdateAttr*)pNew)->getWhichAttr();

                if( ! nTmp || RES_TXTATR_CHARFMT == nTmp || RES_TXTATR_AUTOFMT == nTmp ||
                    RES_FMT_CHG == nTmp || RES_ATTRSET_CHG == nTmp )
                {
                    lcl_SetWrong( *this, nPos, nPos + nLen, false );
                    lcl_SetScriptInval( *this, nPos );
                }
            }

            if( isA11yRelevantAttribute( ((SwUpdateAttr*)pNew)->getWhichAttr() ) &&
                    hasA11yRelevantAttribute( ((SwUpdateAttr*)pNew)->getFmtAttr() ) )
            {
                // #i104008#
                SwViewShell* pViewSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
                if ( pViewSh  )
                {
                    pViewSh->InvalidateAccessibleParaAttrs( *this );
                }
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

                // OD 09.01.2004 #i11859# - correction:
                //  (1) Also invalidate next frame on next page/column.
                //  (2) Skip empty sections and hidden paragraphs
                //  Thus, use method <InvalidateNextPrtArea()>
                InvalidateNextPrtArea();

                SetCompletePaint();
            }
            break;

        case RES_TXTATR_FIELD:
        case RES_TXTATR_ANNOTATION:
            {
                nPos = ((SwFmtFld*)pNew)->GetTxtFld()->GetStart();
                if( IsIdxInside( nPos, 1 ) )
                {
                    if( pNew == pOld )
                    {
                        // only repaint
                        // opt: invalidate window?
                        InvalidatePage();
                        SetCompletePaint();
                    }
                    else
                        _InvalidateRange( SwCharRange( nPos, 1 ) );
                }
                bSetFldsDirty = true;
                // ST2
                if ( SwSmartTagMgr::Get().IsSmartTagsEnabled() )
                    lcl_SetWrong( *this, nPos, nPos + 1, false );
            }
            break;

        case RES_TXTATR_FTN :
        {
            nPos = ((SwFmtFtn*)pNew)->GetTxtFtn()->GetStart();
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
            sal_uInt16 nCount = rNewSet.Count();

            if( SfxItemState::SET == rNewSet.GetItemState( RES_TXTATR_FTN, false, &pItem ))
            {
                nPos = ((SwFmtFtn*)pItem)->GetTxtFtn()->GetStart();
                if( IsIdxInside( nPos, 1 ) )
                    Prepare( PREP_FTN, pNew );
                nClear = 0x01;
                --nCount;
            }

            if( SfxItemState::SET == rNewSet.GetItemState( RES_TXTATR_FIELD, false, &pItem ))
            {
                nPos = ((SwFmtFld*)pItem)->GetTxtFld()->GetStart();
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
            bool bLineSpace = SfxItemState::SET == rNewSet.GetItemState(
                                            RES_PARATR_LINESPACING, false ),
                     bRegister  = SfxItemState::SET == rNewSet.GetItemState(
                                            RES_PARATR_REGISTER, false );
            if ( bLineSpace || bRegister )
            {
                Prepare( bRegister ? PREP_REGISTER : PREP_ADJUST_FRM );
                CalcLineSpace();
                InvalidateSize();
                _InvalidatePrt();

                // OD 09.01.2004 #i11859# - correction:
                //  (1) Also invalidate next frame on next page/column.
                //  (2) Skip empty sections and hidden paragraphs
                //  Thus, use method <InvalidateNextPrtArea()>
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
            if ( SfxItemState::SET == rNewSet.GetItemState( RES_PARATR_SPLIT,
                                                       false ))
            {
                if ( GetPrev() )
                    CheckKeep();
                Prepare( PREP_CLEAR );
                InvalidateSize();
                nClear |= 0x08;
                --nCount;
            }

            if( SfxItemState::SET == rNewSet.GetItemState( RES_BACKGROUND, false)
                && !IsFollow() && GetDrawObjs() )
            {
                SwSortedObjs *pObjs = GetDrawObjs();
                for ( size_t i = 0; GetDrawObjs() && i < pObjs->size(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                    if ( pAnchoredObj->ISA(SwFlyFrm) )
                    {
                        SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                        if( !pFly->IsFlyInCntFrm() )
                        {
                            const SvxBrushItem &rBack =
                                pFly->GetAttrSet()->GetBackground();
                            // OD 20.08.2002 #99657# #GetTransChg#
                            //     following condition determines, if the fly frame
                            //     "inherites" the background color of text frame.
                            //     This is the case, if fly frame background
                            //     color is "no fill"/"auto fill" and if the fly frame
                            //     has no background graphic.
                            //     Thus, check complete fly frame background
                            //     color and *not* only its transparency value
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

            if ( SfxItemState::SET ==
                 rNewSet.GetItemState( RES_TXTATR_CHARFMT, false ) )
            {
                lcl_SetWrong( *this, 0, COMPLETE_STRING, false );
                lcl_SetScriptInval( *this, 0 );
            }
            else if ( SfxItemState::SET ==
                      rNewSet.GetItemState( RES_CHRATR_LANGUAGE, false ) ||
                      SfxItemState::SET ==
                      rNewSet.GetItemState( RES_CHRATR_CJK_LANGUAGE, false ) ||
                      SfxItemState::SET ==
                      rNewSet.GetItemState( RES_CHRATR_CTL_LANGUAGE, false ) )
                lcl_SetWrong( *this, 0, COMPLETE_STRING, false );
            else if ( SfxItemState::SET ==
                      rNewSet.GetItemState( RES_CHRATR_FONT, false ) ||
                      SfxItemState::SET ==
                      rNewSet.GetItemState( RES_CHRATR_CJK_FONT, false ) ||
                      SfxItemState::SET ==
                      rNewSet.GetItemState( RES_CHRATR_CTL_FONT, false ) )
                lcl_SetScriptInval( *this, 0 );
            else if ( SfxItemState::SET ==
                      rNewSet.GetItemState( RES_FRAMEDIR, false ) )
            {
                SetDerivedR2L( false );
                CheckDirChange();
                // OD 09.12.2002 #105576# - Force complete paint due to existing
                // indents.
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
                // #i88069#
                SwViewShell* pViewSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
                if ( pViewSh  )
                {
                    pViewSh->InvalidateAccessibleParaAttrs( *this );
                }
            }
        }
        break;

        // 6870: process SwDocPosUpdate
        case RES_DOCPOS_UPDATE:
        {
            if( pOld && pNew )
            {
                const SwDocPosUpdate *pDocPos = (const SwDocPosUpdate*)pOld;
                if( pDocPos->nDocPos <= maFrm.Top() )
                {
                    const SwFmtFld *pFld = (const SwFmtFld *)pNew;
                    InvalidateRange(
                        SwCharRange( pFld->GetTxtFld()->GetStart(), 1 ) );
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
            SetDerivedR2L( false );
            CheckDirChange();
            break;
        default:
        {
            Prepare( PREP_CLEAR );
            _InvalidatePrt();
            if ( !nWhich )
            {
                // is called by e. g. HiddenPara with 0
                SwFrm *pNxt;
                if ( 0 != (pNxt = FindNext()) )
                    pNxt->InvalidatePrt();
            }
        }
    } // switch

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
                // this should be the one
                // (could only differ temporarily; is that disturbing?)
                rInfo.SetInfo( pPage, this );
                return false;
            }
            if ( pPage->GetPhyPageNum() < rInfo.GetOrigPage()->GetPhyPageNum() &&
                 (!rInfo.GetPage() || pPage->GetPhyPageNum() > rInfo.GetPage()->GetPhyPageNum()))
            {
                // this could be the one
                rInfo.SetInfo( pPage, this );
            }
        }
    }
    return true;
}

void SwTxtFrm::PrepWidows( const sal_uInt16 nNeed, bool bNotify )
{
    OSL_ENSURE(GetFollow() && nNeed, "+SwTxtFrm::Prepare: lost all friends");

    SwParaPortion *pPara = GetPara();
    if ( !pPara )
        return;
    pPara->SetPrepWidows();

    sal_uInt16 nHave = nNeed;

    // Wir geben ein paar Zeilen ab und schrumpfen im CalcPreps()
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
    // In dieser Ecke tummelten sich einige Bugs: 7513, 7606.
    // Wenn feststeht, dass Zeilen abgegeben werden koennen,
    // muss der Master darueber hinaus die Widow-Regel ueberpruefen.
    if( !nHave )
    {
        bool bSplit = true;
        if( !IsFollow() )   // only a master decides about orphans
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
            return false;
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
            --rPos; // our last character
    }
    return true;
}

void SwTxtFrm::Prepare( const PrepareHint ePrep, const void* pVoid,
                        bool bNotify )
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
                SetInvalidVert( true );  // Test
            case PREP_WIDOWS_ORPHANS:
            case PREP_WIDOWS:
            case PREP_FTN_GONE :    return;

            case PREP_POS_CHGD :
            {
                // Auch in (spaltigen) Bereichen ist ein InvalidateSize notwendig,
                // damit formatiert wird und ggf. das bUndersized gesetzt wird.
                if( IsInFly() || IsInSct() )
                {
                    SwTwips nTmpBottom = GetUpper()->Frm().Top() +
                        GetUpper()->Prt().Bottom();
                    if( nTmpBottom < Frm().Bottom() )
                        break;
                }
                // are there any free-flying frames on this page?
                SwTxtFly aTxtFly( this );
                if( aTxtFly.IsOn() )
                {
                    // does any free-flying frame overlap?
                    if ( aTxtFly.Relax() || IsUndersized() )
                        break;
                }
                if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue())
                    break;

                SwTextGridItem const*const pGrid(GetGridItem(FindPageFrm()));
                if ( pGrid && GetTxtNode()->GetSwAttrSet().GetParaGrid().GetValue() )
                    break;

                // #i28701# - consider anchored objects
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
        SetInvalidVert( true );  // Test
        OSL_ENSURE( !IsLocked(), "SwTxtFrm::Prepare: three of a perfect pair" );
        if ( bNotify )
            InvalidateSize();
        else
            _InvalidateSize();
        return;
    }

    // get object from cache while locking
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
            // MustFit is stronger than anything else
            if( pPara->IsPrepMustFit() )
                return;
            // see comment in WidowsAndOrphans::FindOrphans and CalcPreps()
            PrepWidows( *(const sal_uInt16 *)pVoid, bNotify );
            break;

        case PREP_FTN :
        {
            SwTxtFtn *pFtn = (SwTxtFtn *)pVoid;
            if( IsInFtn() )
            {
                // am I the first TxtFrm of a footnote?
                if( !GetPrev() )
                    // Wir sind also ein TxtFrm der Fussnote, die
                    // die Fussnotenzahl zur Anzeige bringen muss.
                    // Oder den ErgoSum-Text...
                    InvalidateRange( SwCharRange( 0, 1 ), 1);

                if( !GetNext() )
                {
                    // Wir sind der letzte Ftn, jetzt muessten die
                    // QuoVadis-Texte geupdated werden.
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
                // we are the TxtFrm _with_ the footnote
                const sal_Int32 nPos = pFtn->GetStart();
                InvalidateRange( SwCharRange( nPos, 1 ), 1);
            }
            break;
        }
        case PREP_BOSS_CHGD :
        {
    // Test
            {
                SetInvalidVert( false );
                bool bOld = IsVertical();
                SetInvalidVert( true );
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
            // 4739: if we have a page number field, we must invalidate those spots
            SwpHints *pHints = GetTxtNode()->GetpSwpHints();
            if( pHints )
            {
                const size_t nSize = pHints->Count();
                const sal_Int32 nEnd = GetFollow() ?
                                    GetFollow()->GetOfst() : COMPLETE_STRING;
                for ( size_t i = 0; i < nSize; ++i )
                {
                    const SwTxtAttr *pHt = (*pHints)[i];
                    const sal_Int32 nStart = pHt->GetStart();
                    if( nStart >= GetOfst() )
                    {
                        if( nStart >= nEnd )
                            break;

                // 4029: wenn wir zurueckfliessen und eine Ftn besitzen, so
                // fliesst die Ftn in jedem Fall auch mit. Damit sie nicht im
                // Weg steht, schicken wir uns ein ADJUST_FRM.
                // pVoid != 0 bedeutet MoveBwd()
                        const sal_uInt16 nWhich = pHt->Which();
                        if( RES_TXTATR_FIELD == nWhich ||
                            (HasFtn() && pVoid && RES_TXTATR_FTN == nWhich))
                        InvalidateRange( SwCharRange( nStart, 1 ), 1 );
                    }
                }
            }
            // A new boss, a new chance for growing
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
                SwTextGridItem const*const pGrid(GetGridItem(FindPageFrm()));
                if ( pGrid && GetTxtNode()->GetSwAttrSet().GetParaGrid().GetValue() )
                    InvalidatePrt();
            }

            // if we don't overlap with anybody:
            // did any free-flying frame overlapped _before_ the position change?
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
                        const size_t nCnt = GetDrawObjs()->size();
                        for ( size_t i = 0; i < nCnt; ++i )
                        {
                            SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
                            // #i28701# - consider all
                            // to-character anchored objects
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
                        // are there any free-flying frames on this page?
                        SwTxtFly aTxtFly( this );
                        if( aTxtFly.IsOn() )
                        {
                            // does any free-flying frame overlap?
                            bFormat = aTxtFly.Relax() || IsUndersized();
                        }
                    }
                }
            }

            if( bFormat )
            {
                if( !IsLocked() )
                {
                    if( pPara->GetRepaint().HasArea() )
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
                // Durch Positionsverschiebungen mit Ftns muessen die
                // Frames neu adjustiert werden.
                else if( HasFtn() )
                {
                    Prepare( PREP_ADJUST_FRM, 0, bNotify );
                    _InvalidateSize();
                }
                else
                    return;     // damit kein SetPrep() erfolgt.
            }
            break;
        }
        case PREP_REGISTER:
            if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue() )
            {
                pPara->SetPrepAdjust();
                CalcLineSpace();
                // possible that pPara was deleted above; retrieve it again
                pPara = aAccess.GetPara();
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
                // Wenn ein Follow uns ruft, weil eine Fussnote geloescht wird, muss unsere
                // letzte Zeile formatiert werden, damit ggf. die erste Zeile des Follows
                // hochrutschen kann, die extra auf die naechste Seite gerutscht war, um mit
                // der Fussnote zusammen zu sein, insbesondere bei spaltigen Bereichen.
                OSL_ENSURE( GetFollow(), "PREP_FTN_GONE darf nur vom Follow gerufen werden" );
                sal_Int32 nPos = GetFollow()->GetOfst();
                if( IsFollow() && GetOfst() == nPos )       // falls wir gar keine Textmasse besitzen,
                    FindMaster()->Prepare( PREP_FTN_GONE ); // rufen wir das Prepare unseres Masters
                if( nPos )
                    --nPos; // das Zeichen vor unserem Follow
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
            // else: continue with default case block
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
                if( pPara->GetRepaint().HasArea() )
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
            return;     // no SetPrep() happened
        }
    }
    if( pPara )
        pPara->SetPrep();
}

// Small Helper class:
// Prepares a test format.
// The frame is changed in size and position, its SwParaPortion is moved aside
// and a new one is created.
// To achieve this, run formatting with bTestFormat flag set.
// In the destructor the TxtFrm is reset to its original state.
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

bool SwTxtFrm::TestFormat( const SwFrm* pPrv, SwTwips &rMaxHeight, bool &bSplit )
{
    PROTOCOL_ENTER( this, PROT_TESTFORMAT, 0, 0 )

    if( IsLocked() && GetUpper()->Prt().Width() <= 0 )
        return false;

    SwTestFormat aSave( this, pPrv, rMaxHeight );

    return SwTxtFrm::WouldFit( rMaxHeight, bSplit, true );
}

/* SwTxtFrm::WouldFit()
 * true: wenn ich aufspalten kann.
 * Es soll und braucht nicht neu formatiert werden.
 * Wir gehen davon aus, dass bereits formatiert wurde und dass
 * die Formatierungsdaten noch aktuell sind.
 * Wir gehen davon aus, dass die Framebreiten des evtl. Masters und
 * Follows gleich sind. Deswegen wird kein FindBreak() mit FindOrphans()
 * gerufen.
 * Die benoetigte Hoehe wird von nMaxHeight abgezogen!
 */

bool SwTxtFrm::WouldFit( SwTwips &rMaxHeight, bool &bSplit, bool bTst )
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTxtFrm::WouldFit with swapped frame" );
    SWRECTFN( this );

    if( IsLocked() )
        return false;

    // it can happen that the IdleCollector removed the cached information
    if( !IsEmpty() )
        GetFormatted();

    // OD 2004-05-24 #i27801# - correction: 'short cut' for empty paragraph
    // can *not* be applied, if test format is in progress. The test format doesn't
    // adjust the frame and the printing area - see method <SwTxtFrm::_Format(..)>,
    // which is called in <SwTxtFrm::TestFormat(..)>
    if ( IsEmpty() && !bTst )
    {
        bSplit = false;
        SwTwips nHeight = bVert ? Prt().SSize().Width() : Prt().SSize().Height();
        if( rMaxHeight < nHeight )
            return false;
        else
        {
            rMaxHeight -= nHeight;
            return true;
        }
    }

    // In sehr unguenstigen Faellen kann GetPara immer noch 0 sein.
    // Dann returnen wir true, um auf der neuen Seite noch einmal
    // anformatiert zu werden.
    OSL_ENSURE( HasPara() || IsHiddenNow(), "WouldFit: GetFormatted() and then !HasPara()" );
    if( !HasPara() || ( !(Frm().*fnRect->fnGetHeight)() && IsHiddenNow() ) )
        return true;

    // Da das Orphan-Flag nur sehr fluechtig existiert, wird als zweite
    // Bedingung  ueberprueft, ob die Rahmengroesse durch CalcPreps
    // auf riesengross gesetzt wird, um ein MoveFwd zu erzwingen.
    if( IsWidow() || ( bVert ?
                       ( 0 == Frm().Left() ) :
                       ( LONG_MAX - 20000 < Frm().Bottom() ) ) )
    {
        SetWidow(false);
        if ( GetFollow() )
        {
            // Wenn wir hier durch eine Widow-Anforderung unseres Follows gelandet
            // sind, wird ueberprueft, ob es ueberhaupt einen Follow mit einer
            // echten Hoehe gibt, andernfalls (z.B. in neu angelegten SctFrms)
            // ignorieren wir das IsWidow() und pruefen doch noch, ob wir
            // genung Platz finden.
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
                    return false;
            }
            else
                return false;
        }
    }

    SWAP_IF_NOT_SWAPPED( this );

    SwTxtSizeInfo aInf( this );
    SwTxtMargin aLine( this, &aInf );

    WidowsAndOrphans aFrmBreak( this, rMaxHeight, bSplit );

    bool bRet = true;

    aLine.Bottom();
    // is breaking necessary?
    bSplit = !aFrmBreak.IsInside( aLine );
    if ( bSplit )
        bRet = !aFrmBreak.IsKeepAlways() && aFrmBreak.WouldFit( aLine, rMaxHeight, bTst );
    else
    {
        // we need the total height including the current line
        aLine.Top();
        do
        {
            rMaxHeight -= aLine.GetLineHeight();
        } while ( aLine.Next() );
    }

    UNDO_SWAP( this )

    return bRet;
}

sal_uInt16 SwTxtFrm::GetParHeight() const
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTxtFrm::GetParHeight with swapped frame" );

    if( !HasPara() )
    {   // Fuer nichtleere Absaetze ist dies ein Sonderfall, da koennen wir
        // bei UnderSized ruhig nur 1 Twip mehr anfordern.
        sal_uInt16 nRet = (sal_uInt16)Prt().SSize().Height();
        if( IsUndersized() )
        {
            if( IsEmpty() || GetTxt().isEmpty() )
                nRet = (sal_uInt16)EmptyHeight();
            else
                ++nRet;
        }
        return nRet;
    }

    // FME, OD 08.01.2004 #i11859# - refactoring and improve code
    const SwLineLayout* pLineLayout = GetPara();
    sal_uInt16 nHeight = pLineLayout ? pLineLayout->GetRealHeight() : 0;
    if( GetOfst() && !IsFollow() )  // Ist dieser Absatz gescrollt? Dann ist unsere
        nHeight *= 2;               // bisherige Hoehe mind. eine Zeilenhoehe zu gering
    // OD 2004-03-04 #115793#
    while ( pLineLayout && pLineLayout->GetNext() )
    {
        pLineLayout = pLineLayout->GetNext();
        nHeight = nHeight + pLineLayout->GetRealHeight();
    }

    return nHeight;
}

// returns this _always_ in the formated state!
SwTxtFrm* SwTxtFrm::GetFormatted( bool bForceQuickFormat )
{
    SWAP_IF_SWAPPED( this )

    // Kann gut sein, dass mir der IdleCollector mir die gecachten
    // Informationen entzogen hat. Calc() ruft unser Format.
                      // Nicht bei leeren Absaetzen!
    if( !HasPara() && !(IsValid() && IsEmpty()) )
    {
        // Calc() must be called, because frame position can be wrong
        const bool bFormat = GetValidSizeFlag();
        Calc();
        // Es kann durchaus sein, dass Calc() das Format()
        // nicht anstiess (weil wir einst vom Idle-Zerstoerer
        // aufgefordert wurden unsere Formatinformationen wegzuschmeissen).
        // 6995: Optimierung mit FormatQuick()
        if( bFormat && !FormatQuick( bForceQuickFormat ) )
            Format();
    }

    UNDO_SWAP( this )

    return this;
}

SwTwips SwTxtFrm::CalcFitToContent()
{
    // #i31490#
    // If we are currently locked, we better return with a
    // fairly reasonable value:
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

    // #i25422# objects anchored as character in RTL
    if ( IsRightToLeft() )
        Frm().Pos().X() += nOldFrmWidth - nPageWidth;

    // #i31490#
    SwTxtFrmLocker aLock( this );

    SwTxtFormatInfo aInf( this, false, true, true );
    aInf.SetIgnoreFly( true );
    SwTxtFormatter  aLine( this, &aInf );
    SwHookOut aHook( aInf );

    // #i54031# - assure mininum of MINLAY twips.
    const SwTwips nMax = std::max( (SwTwips)MINLAY,
                              aLine._CalcFitToContent() + 1 );

    Frm().Width( nOldFrmWidth );
    Prt().Width( nOldPrtWidth );

    // #i25422# objects anchored as character in RTL
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

    // reset additional first line offset
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
            // keep current paragraph portion and apply dummy paragraph portion
            SwParaPortion* pOldPara = GetPara();
            SwParaPortion *pDummy = new SwParaPortion();
            SetPara( pDummy, false );

            // lock paragraph
            SwTxtFrmLocker aLock( this );

            // simulate text formatting
            SwTxtFormatInfo aInf( this, false, true, true );
            aInf.SetIgnoreFly( true );
            SwTxtFormatter aLine( this, &aInf );
            SwHookOut aHook( aInf );
            aLine._CalcFitToContent();

            // determine additional first line offset
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

            // restore paragraph portion
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
    // #i71281#
    // invalidate printing area, if height of last line changes
    const SwTwips mnOldHeightOfLastLine( mnHeightOfLastLine );
    // determine output device
    SwViewShell* pVsh = getRootFrm()->GetCurrShell();
    OSL_ENSURE( pVsh, "<SwTxtFrm::_GetHeightOfLastLineForPropLineSpacing()> - no SwViewShell" );
    // #i78921# - make code robust, according to provided patch
    // There could be no <SwViewShell> instance in the case of loading a binary
    // StarOffice file format containing an embedded Writer document.
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
    // #i78921# - make code robust, according to provided patch
    if ( !pOut )
    {
        return;
    }

    // determine height of last line

    if ( _bUseFont || pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING ) )
    {
        // former determination of last line height for proprotional line
        // spacing - take height of font set at the paragraph
        SwFont aFont( GetAttrSet(), pIDSA );

        // we must ensure that the font is restored correctly on the OutputDevice
        // otherwise Last!=Owner could occur
        if ( pLastFont )
        {
            SwFntObj *pOldFont = pLastFont;
            pLastFont = NULL;
            aFont.SetFntChg( true );
            aFont.ChgPhysFnt( pVsh, *pOut );
            mnHeightOfLastLine = aFont.GetHeight( pVsh, *pOut );
            pLastFont->Unlock();
            pLastFont = pOldFont;
            pLastFont->SetDevFont( pVsh, *pOut );
        }
        else
        {
            vcl::Font aOldFont = pOut->GetFont();
            aFont.SetFntChg( true );
            aFont.ChgPhysFnt( pVsh, *pOut );
            mnHeightOfLastLine = aFont.GetHeight( pVsh, *pOut );
            pLastFont->Unlock();
            pLastFont = NULL;
            pOut->SetFont( aOldFont );
        }
    }
    else
    {
        // new determination of last line height - take actually height of last line
        // #i89000#
        // assure same results, if paragraph is undersized
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
                    // iteration to last line
                    pLineLayout = pLineLayout->GetNext();
                }
                if ( pLineLayout )
                {
                    SwTwips nAscent, nDescent, nDummy1, nDummy2;
                    // #i47162# - suppress consideration of
                    // fly content portions and the line portion.
                    pLineLayout->MaxAscentDescent( nAscent, nDescent,
                                                   nDummy1, nDummy2,
                                                   0, true );
                    // #i71281#
                    // Suppress wrong invalidation of printing area, if method is
                    // called recursive.
                    // Thus, member <mnHeightOfLastLine> is only set directly, if
                    // no recursive call is needed.
                    const SwTwips nNewHeightOfLastLine = nAscent + nDescent;
                    // #i47162# - if last line only contains
                    // fly content portions, <mnHeightOfLastLine> is zero.
                    // In this case determine height of last line by the font
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
    // #i71281#
    // invalidate printing area, if height of last line changes
    if ( mnHeightOfLastLine != mnOldHeightOfLastLine )
    {
        InvalidatePrt();
    }
}

// OD 07.01.2004 #i11859# - change return data type
//      add default parameter <_bNoPropLineSpacing> to control, if the
//      value of a proportional line spacing is returned or not
// OD 07.01.2004 - trying to describe purpose of method:
//      Method returns the value of the inter line spacing for a text frame.
//      Such a value exists for proportional line spacings ("1,5 Lines",
//      "Double", "Proportional" and for leading line spacing ("Leading").
//      By parameter <_bNoPropLineSpace> (default value false) it can be
//      controlled, if the value of a proportional line spacing is returned.
long SwTxtFrm::GetLineSpace( const bool _bNoPropLineSpace ) const
{
    long nRet = 0;

    const SwAttrSet* pSet = GetAttrSet();
    const SvxLineSpacingItem &rSpace = pSet->GetLineSpacing();

    switch( rSpace.GetInterLineSpaceRule() )
    {
        case SVX_INTER_LINE_SPACE_PROP:
        {
            // OD 07.01.2004 #i11859#
            if ( _bNoPropLineSpace )
            {
                break;
            }

            // OD 2004-03-17 #i11860# - use method <GetHeightOfLastLine()>
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

sal_uInt16 SwTxtFrm::FirstLineHeight() const
{
    if ( !HasPara() )
    {
        if( IsEmpty() && IsValid() )
            return IsVertical() ? (sal_uInt16)Prt().Width() : (sal_uInt16)Prt().Height();
        return USHRT_MAX;
    }
    const SwParaPortion *pPara = GetPara();
    if ( !pPara )
        return USHRT_MAX;

    return pPara->Height();
}

sal_uInt16 SwTxtFrm::GetLineCount( sal_Int32 nPos )
{
    sal_uInt16 nRet = 0;
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
    // not necessary to format here (GerFormatted etc.), because we have to come from there!
    sal_uLong nNew = 0;
    const SwLineNumberInfo &rInf = GetNode()->GetDoc()->GetLineNumberInfo();
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

            // Extend repaint to the bottom.
            if ( HasPara() )
            {
                SwRepaint& rRepaint = GetPara()->GetRepaint();
                rRepaint.Bottom( std::max( rRepaint.Bottom(),
                                       Frm().Top()+Prt().Bottom()));
            }
        }
        else // Paragraphs which are not counted should not manipulate the AllLines.
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
        const bool bRestart = GetTxtNode()->GetDoc()->GetLineNumberInfo().IsRestartEachPage();

        if ( !IsFollow() && rLineNum.GetStartValue() && rLineNum.IsCount() )
            nNewNum = rLineNum.GetStartValue() - 1;
        // If it is a follow or not has not be considered if it is a restart at each page; the
        // restart should also take affekt at follows.
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

            // #i78254# Restart line numbering at page change
            // First body content may be in table!
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

// Helper function for SwTxtFrm::CalcBasePosForFly()
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

    // Get first 'real' line and adjust position and height of line rectangle
    // OD 08.09.2003 #110978#, #108749#, #110354# - correct behaviour,
    // if no 'real' line exists (empty paragraph with and without a dummy portion)
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
    // #118809# - ignore objects in page header|footer for
    // text frames not in page header|footer
    aTxtFly.SetIgnoreObjsInHeaderFooter( true );
    SwTwips nRet1 = lcl_CalcFlyBasePos( *this, aFlyRect, aTxtFly );
    aTxtFly.SetIgnoreCurrentFrame( false );
    SwTwips nRet2 = lcl_CalcFlyBasePos( *this, aFlyRect, aTxtFly );

    // make values relative to frame start position
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
