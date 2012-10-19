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

#include <hintids.hxx>
#include <hints.hxx>
#include <svl/ctloptions.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/sfxuno.hxx>
#include <editeng/langitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/pgrditem.hxx>
#include <swmodule.hxx>
#include <SwSmartTagMgr.hxx>
#include <doc.hxx>      // GetDoc()
#include "rootfrm.hxx"
#include <pagefrm.hxx>  // InvalidateSpelling
#include <rootfrm.hxx>
#include <viewsh.hxx>   // ViewShell
#include <pam.hxx>      // SwPosition
#include <ndtxt.hxx>        // SwTxtNode
#include <txtatr.hxx>
#include <paratr.hxx>
#include <viewopt.hxx>
#include <dflyobj.hxx>
#include <flyfrm.hxx>
#include <tabfrm.hxx>
#include <frmtool.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>
#include <dbg_lay.hxx>
#include <fmtfld.hxx>
#include <fmtftn.hxx>
#include <txtfld.hxx>
#include <txtftn.hxx>
#include <charatr.hxx>
#include <ftninfo.hxx>
#include <fmtline.hxx>
#include <txtfrm.hxx>       // SwTxtFrm
#include <sectfrm.hxx>      // SwSectFrm
#include <itrform2.hxx>       // Iteratoren
#include <widorp.hxx>       // SwFrmBreak
#include <txtcache.hxx>
#include <fntcache.hxx>     // GetLineSpace benutzt pLastFont
#include <SwGrammarMarkUp.hxx>
#include <lineinfo.hxx>
#include <SwPortionHandler.hxx>
#include <dcontact.hxx>
#include <sortedobjs.hxx>
#include <txtflcnt.hxx>     // SwTxtFlyCnt
#include <fmtflcnt.hxx>     // SwFmtFlyCnt
#include <fmtcntnt.hxx>     // SwFmtCntnt
#include <numrule.hxx>
#include <swtable.hxx>
#include <fldupde.hxx>
#include <IGrammarContact.hxx>
#include <switerator.hxx>


TYPEINIT1( SwTxtFrm, SwCntntFrm );

// Switches width and height of the text frame
void SwTxtFrm::SwapWidthAndHeight()
{
    if ( ! bIsSwapped )
    {
        const long nPrtOfstX = Prt().Pos().X();
        Prt().Pos().X() = Prt().Pos().Y();
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        if( IsVertLR() )
            Prt().Pos().Y() = nPrtOfstX;
        else
            Prt().Pos().Y() = Frm().Width() - ( nPrtOfstX + Prt().Width() );

    }
    else
    {
        const long nPrtOfstY = Prt().Pos().Y();
        Prt().Pos().Y() = Prt().Pos().X();
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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

    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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

    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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

    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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

SwFrmSwapper::SwFrmSwapper( const SwTxtFrm* pTxtFrm, sal_Bool bSwapIfNotSwapped )
    : pFrm( pTxtFrm ), bUndo( sal_False )
{
    if ( pFrm->IsVertical() &&
        ( (   bSwapIfNotSwapped && ! pFrm->IsSwapped() ) ||
          ( ! bSwapIfNotSwapped && pFrm->IsSwapped() ) ) )
    {
        bUndo = sal_True;
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

void SwLayoutModeModifier::Modify( sal_Bool bChgToRTL )
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
        eLang = (LanguageType)::GetAppLanguage();

    ((OutputDevice&)rOut).SetDigitLanguage( eLang );
}

SwDigitModeModifier::~SwDigitModeModifier()
{
    ((OutputDevice&)rOut).SetDigitLanguage( nOldLanguageType );
}

/*************************************************************************
 *                      SwTxtFrm::Init()
 *************************************************************************/

void SwTxtFrm::Init()
{
    OSL_ENSURE( !IsLocked(), "+SwTxtFrm::Init: this ist locked." );
    if( !IsLocked() )
    {
        ClearPara();
        ResetBlinkPor();
        //Die Flags direkt setzen um ResetPreps und damit ein unnuetzes GetPara
        //einzusparen.
        // Nicht bOrphan, bLocked oder bWait auf sal_False setzen !
        // bOrphan = bFlag7 = bFlag8 = sal_False;
    }
}

/*************************************************************************
 *                      SwTxtFrm::SwTxtFrm()
 *************************************************************************/
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
    , mbFollowFormatAllowed( true ) // OD 14.03.2003 #i11760#
{
    nType = FRMC_TXT;
}

/*************************************************************************
 *                      SwTxtFrm::~SwTxtFrm()
 *************************************************************************/
SwTxtFrm::~SwTxtFrm()
{
    // Remove associated SwParaPortion from pTxtCache
    ClearPara();
}

const XubString& SwTxtFrm::GetTxt() const
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

/*************************************************************************
 *                        SwTxtFrm::IsHiddenNow()
 *************************************************************************/
sal_Bool SwTxtFrm::IsHiddenNow() const
{
    SwFrmSwapper aSwapper( this, sal_True );

    if( !Frm().Width() && IsValid() && GetUpper()->IsValid() )
                                       //bei Stackueberlauf (StackHack) invalid!
    {
//        OSL_FAIL( "SwTxtFrm::IsHiddenNow: thin frame" );
        return sal_True;
    }

    const bool bHiddenCharsHidePara = GetTxtNode()->HasHiddenCharAttribute( true );
    const bool bHiddenParaField = GetTxtNode()->HasHiddenParaField();
    const ViewShell* pVsh = getRootFrm()->GetCurrShell();

    if ( pVsh && ( bHiddenCharsHidePara || bHiddenParaField ) )
    {
        if (
             ( bHiddenParaField &&
               ( !pVsh->GetViewOptions()->IsShowHiddenPara() &&
                 !pVsh->GetViewOptions()->IsFldName() ) ) ||
             ( bHiddenCharsHidePara &&
               !pVsh->GetViewOptions()->IsShowHiddenChar() ) )
        {
            return sal_True;
        }
    }

    return sal_False;
}


/*************************************************************************
 *                        SwTxtFrm::HideHidden()
 *************************************************************************/
// Entfernt die Anhaengsel des Textfrms wenn dieser hidden ist

void SwTxtFrm::HideHidden()
{
    OSL_ENSURE( !GetFollow() && IsHiddenNow(),
            "HideHidden on visible frame of hidden frame has follow" );

    const xub_StrLen nEnd = STRING_LEN;
    HideFootnotes( GetOfst(), nEnd );
    // OD 2004-01-15 #110582#
    HideAndShowObjects();

    //Die Formatinfos sind jetzt obsolete
    ClearPara();
}

/*************************************************************************
 *                        SwTxtFrm::HideFootnotes()
 *************************************************************************/
void SwTxtFrm::HideFootnotes( xub_StrLen nStart, xub_StrLen nEnd )
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
                const xub_StrLen nIdx = *pHt->GetStart();
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
                  const xub_StrLen _nObjAnchorPos,
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
                        _rFrm.GetTxtNode()->GetTxt().GetChar( _nObjAnchorPos );
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
/*************************************************************************
 *                        SwTxtFrm::HideAndShowObjects()
 *************************************************************************/
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
            for ( sal_uInt32 i = 0; i < GetDrawObjs()->Count(); ++i )
            {
                SdrObject* pObj = (*GetDrawObjs())[i]->DrawObj();
                SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
                // #120729# - hotfix
                // under certain conditions
                const RndStdIds eAnchorType( pContact->GetAnchorId() );
                const xub_StrLen nObjAnchorPos = pContact->GetCntntAnchorIndex().GetIndex();
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
            const ViewShell* pVsh = getRootFrm()->GetCurrShell();
            const bool bShouldBeHidden = !pVsh || !pVsh->GetWin() ||
                                         !pVsh->GetViewOptions()->IsShowHiddenChar();

            // Thus, show all objects, which are anchored at paragraph and
            // hide/show objects, which are anchored at/as character, according
            // to the visibility of the anchor character.
            for ( sal_uInt32 i = 0; i < GetDrawObjs()->Count(); ++i )
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
                    xub_StrLen nHiddenStart;
                    xub_StrLen nHiddenEnd;
                    xub_StrLen nObjAnchorPos = pContact->GetCntntAnchorIndex().GetIndex();
                    SwScriptInfo::GetBoundsOfHiddenRange( rNode, nObjAnchorPos, nHiddenStart, nHiddenEnd, 0 );
                    // #120729# - hotfix
                    // under certain conditions
                    if ( nHiddenStart != STRING_LEN && bShouldBeHidden &&
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
 * Liefert die erste Trennmoeglichkeit in der aktuellen Zeile zurueck.
 * Die Methode wird in SwTxtFrm::Format() benutzt, um festzustellen, ob
 * die Vorgaengerzeile mitformatiert werden muss.
 * nFound ist <= nEndLine.
 *************************************************************************/

xub_StrLen SwTxtFrm::FindBrk( const XubString &rTxt,
                              const xub_StrLen nStart,
                              const xub_StrLen nEnd ) const
{
    // #i104291# - applying patch to avoid overflow.
    unsigned long nFound = nStart;
    const xub_StrLen nEndLine = Min( nEnd, rTxt.Len() );

    // Wir ueberlesen erst alle Blanks am Anfang der Zeile (vgl. Bug 2235).
    while( nFound <= nEndLine &&
           ' ' == rTxt.GetChar( static_cast<xub_StrLen>(nFound) ) )
    {
         nFound++;
    }

    // Eine knifflige Sache mit den TxtAttr-Dummy-Zeichen (hier "$"):
    // "Dr.$Meyer" am Anfang der zweiten Zeile. Dahinter ein Blank eingegeben
    // und das Wort rutscht nicht in die erste Zeile, obwohl es ginge.
    // Aus diesem Grund nehmen wir das Dummy-Zeichen noch mit.
    while( nFound <= nEndLine &&
           ' ' != rTxt.GetChar( static_cast<xub_StrLen>(nFound) ) )
    {
        nFound++;
    }

    return nFound <= STRING_LEN
           ? static_cast<xub_StrLen>(nFound)
           : STRING_LEN;
}

/*************************************************************************
 *                      SwTxtFrm::IsIdxInside()
 *************************************************************************/

sal_Bool SwTxtFrm::IsIdxInside( const xub_StrLen nPos, const xub_StrLen nLen ) const
{
    if( GetOfst() > nPos + nLen ) // d.h., der Bereich liegt komplett vor uns.
        return sal_False;

    if( !GetFollow() )         // der Bereich liegt nicht komplett vor uns,
        return sal_True;           // nach uns kommt niemand mehr.

    const xub_StrLen nMax = GetFollow()->GetOfst();

    // der Bereich liegt nicht komplett hinter uns bzw.
    // unser Text ist geloescht worden.
    if( nMax > nPos || nMax > GetTxt().Len() )
        return sal_True;

    // changes made in the first line of a follow can modify the master
    const SwParaPortion* pPara = GetFollow()->GetPara();
    return pPara && ( nPos <= nMax + pPara->GetLen() );
}

/*************************************************************************
 *                      SwTxtFrm::InvalidateRange()
 *************************************************************************/
inline void SwTxtFrm::InvalidateRange(const SwCharRange &aRange, const long nD)
{
    if ( IsIdxInside( aRange.Start(), aRange.Len() ) )
        _InvalidateRange( aRange, nD );
}

/*************************************************************************
 *                      SwTxtFrm::_InvalidateRange()
 *************************************************************************/

void SwTxtFrm::_InvalidateRange( const SwCharRange &aRange, const long nD)
{
    if ( !HasPara() )
    {   InvalidateSize();
        return;
    }

    SetWidow( sal_False );
    SwParaPortion *pPara = GetPara();

    sal_Bool bInv = sal_False;
    if( 0 != nD )
    {
        //Auf nDelta werden die Differenzen zwischen alter und
        //neuer Zeilenlaenge aufaddiert, deshalb ist es negativ,
        //wenn Zeichen eingefuegt wurden, positiv, wenn Zeichen
        //geloescht wurden.
        *(pPara->GetDelta()) += nD;
        bInv = sal_True;
    }
    SwCharRange &rReformat = *(pPara->GetReformat());
    if(aRange != rReformat) {
        if( STRING_LEN == rReformat.Len() )
            rReformat = aRange;
        else
            rReformat += aRange;
        bInv = sal_True;
    }
    if(bInv)
    {
        InvalidateSize();
    }
}

/*************************************************************************
 *                      SwTxtFrm::CalcLineSpace()
 *************************************************************************/

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
    // 4291: Unterlauf bei Flys
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
            // Wenn es einen Follow() gibt, oder wenn wir an dieser
            // Stelle aufbrechen muessen, so wird neu formatiert.
            Init();
        }
        else
        {
            // Alles nimmt seinen gewohnten Gang ...
            pPara->SetPrepAdjust();
            pPara->SetPrep();
        }
    }
}

//
// SET_WRONG( nPos, nCnt, bMove )
//
#define SET_WRONG( nPos, nCnt, bMove ) \
{ \
    lcl_SetWrong( *this, nPos, nCnt, bMove ); \
}

static void lcl_SetWrong( SwTxtFrm& rFrm, xub_StrLen nPos, long nCnt, bool bMove )
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
            xub_StrLen nLen = (xub_StrLen)nCnt;
            if( pTxtNode->GetWrong() )
                pTxtNode->GetWrong()->Invalidate( nPos, nLen );
            if( pWrongGrammar )
                pWrongGrammar->Invalidate( nPos, nLen );
            if( pTxtNode->GetSmartTags() )
                pTxtNode->GetSmartTags()->Invalidate( nPos, nLen );
        }
        if ( !pTxtNode->GetWrong() && !pTxtNode->IsWrongDirty() )
        {
            pTxtNode->SetWrong( new SwWrongList( WRONGLIST_SPELL ) );
            pTxtNode->GetWrong()->SetInvalid( nPos, nPos + (sal_uInt16)( nCnt > 0 ? nCnt : 1 ) );
        }
        if ( !pTxtNode->GetSmartTags() && !pTxtNode->IsSmartTagDirty() )
        {
            // SMARTTAGS
            pTxtNode->SetSmartTags( new SwWrongList( WRONGLIST_SMARTTAG ) );
            pTxtNode->GetSmartTags()->SetInvalid( nPos, nPos + (sal_uInt16)( nCnt > 0 ? nCnt : 1 ) );
        }
        pTxtNode->SetWrongDirty( true );
        pTxtNode->SetGrammarCheckDirty( true );
        pTxtNode->SetWordCountDirty( true );
        pTxtNode->SetAutoCompleteWordDirty( true );
        // SMARTTAGS
        pTxtNode->SetSmartTagDirty( true );
    }

    SwRootFrm *pRootFrm = rFrm.getRootFrm();
    if (pRootFrm)
    {
        pRootFrm->SetNeedGrammarCheck( sal_True );
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

//
// SET_SCRIPT_INVAL( nPos )
//

#define SET_SCRIPT_INVAL( nPos )\
    lcl_SetScriptInval( *this, nPos );

static void lcl_SetScriptInval( SwTxtFrm& rFrm, xub_StrLen nPos )
{
    if( rFrm.GetPara() )
        rFrm.GetPara()->GetScriptInfo().SetInvalidity( nPos );
}

static void lcl_ModifyOfst( SwTxtFrm* pFrm, xub_StrLen nPos, xub_StrLen nLen )
{
    while( pFrm && pFrm->GetOfst() <= nPos )
        pFrm = pFrm->GetFollow();
    while( pFrm )
    {
        pFrm->ManipOfst( pFrm->GetOfst() + nLen );
        pFrm = pFrm->GetFollow();
    }
}

/*************************************************************************
 *                      SwTxtFrm::Modify()
 *************************************************************************/

void SwTxtFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    const MSHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;

    //Wuensche die FrmAttribute betreffen werden von der Basisklasse
    //verarbeitet.
    if( IsInRange( aFrmFmtSetRange, nWhich ) || RES_FMT_CHG == nWhich )
    {
        SwCntntFrm::Modify( pOld, pNew );
        if( nWhich == RES_FMT_CHG && getRootFrm()->GetCurrShell() )
        {
            // Collection hat sich geaendert
            Prepare( PREP_CLEAR );
            _InvalidatePrt();
            SET_WRONG( 0, STRING_LEN, false );
            SetDerivedR2L( sal_False );
            CheckDirChange();
            // OD 09.12.2002 #105576# - Force complete paint due to existing
            // indents.
            SetCompletePaint();
            InvalidateLineNum();
        }
        return;
    }

    // Im gelockten Zustand werden keine Bestellungen angenommen.
    if( IsLocked() )
        return;

    // Dies spart Stack, man muss nur aufpassen,
    // dass sie Variablen gesetzt werden.
    xub_StrLen nPos, nLen;
    sal_Bool bSetFldsDirty = sal_False;
    sal_Bool bRecalcFtnFlag = sal_False;

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
                    // 6969: Aktualisierung der NumPortions auch bei leeren Zeilen!
                    if( nPos )
                        InvalidateSize();
                    else
                        Prepare( PREP_CLEAR );
                }
                else
                    _InvalidateRange( SwCharRange( nPos, nLen ), nLen );
            }
            SET_WRONG( nPos, nLen, true )
            SET_SCRIPT_INVAL( nPos )
            bSetFldsDirty = sal_True;
            if( HasFollow() )
                lcl_ModifyOfst( this, nPos, nLen );
        }
        break;
        case RES_DEL_CHR:
        {
            nPos = ((SwDelChr*)pNew)->nPos;
            InvalidateRange( SwCharRange( nPos, 1 ), -1 );
            SET_WRONG( nPos, -1, true )
            SET_SCRIPT_INVAL( nPos )
            bSetFldsDirty = bRecalcFtnFlag = sal_True;
            if( HasFollow() )
                lcl_ModifyOfst( this, nPos, STRING_LEN );
        }
        break;
        case RES_DEL_TXT:
        {
            nPos = ((SwDelTxt*)pNew)->nStart;
            nLen = ((SwDelTxt*)pNew)->nLen;
            long m = nLen;
            m *= -1;
            if( IsIdxInside( nPos, nLen ) )
            {
                if( !nLen )
                    InvalidateSize();
                else
                    InvalidateRange( SwCharRange( nPos, 1 ), m );
            }
            SET_WRONG( nPos, m, true )
            SET_SCRIPT_INVAL( nPos )
            bSetFldsDirty = bRecalcFtnFlag = sal_True;
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
                // Es muss in jedem Fall neu formatiert werden,
                // auch wenn der invalidierte Bereich null ist.
                // Beispiel: leere Zeile, 14Pt einstellen !
                // if( !nLen ) nLen = 1;

                // 6680: FtnNummern muessen formatiert werden.
                if( !nLen )
                    nLen = 1;

                _InvalidateRange( SwCharRange( nPos, nLen) );
                MSHORT nTmp = ((SwUpdateAttr*)pNew)->nWhichAttr;

                if( ! nTmp || RES_TXTATR_CHARFMT == nTmp || RES_TXTATR_AUTOFMT == nTmp ||
                    RES_FMT_CHG == nTmp || RES_ATTRSET_CHG == nTmp )
                {
                    SET_WRONG( nPos, nPos + nLen, false )
                    SET_SCRIPT_INVAL( nPos )
                }
            }

            // #i104008#
            ViewShell* pViewSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
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

                // OD 09.01.2004 #i11859# - correction:
                //  (1) Also invalidate next frame on next page/column.
                //  (2) Skip empty sections and hidden paragraphs
                //  Thus, use method <InvalidateNextPrtArea()>
                InvalidateNextPrtArea();

                SetCompletePaint();
            }
            break;
        case RES_TXTATR_FIELD:
        {
            nPos = *((SwFmtFld*)pNew)->GetTxtFld()->GetStart();
            if( IsIdxInside( nPos, 1 ) )
            {
                if( pNew == pOld )
                {
                    // Nur repainten
                    // opt: invalidate aufs Window ?
                    InvalidatePage();
                    SetCompletePaint();
                }
                else
                    _InvalidateRange( SwCharRange( nPos, 1 ) );
            }
            bSetFldsDirty = sal_True;
            // ST2
            if ( SwSmartTagMgr::Get().IsSmartTagsEnabled() )
                SET_WRONG( nPos, nPos + 1, false )
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
            const SfxPoolItem* pItem;
            int nClear = 0;
            MSHORT nCount = rNewSet.Count();

            if( SFX_ITEM_SET == rNewSet.GetItemState( RES_TXTATR_FTN,
                sal_False, &pItem ))
            {
                nPos = *((SwFmtFtn*)pItem)->GetTxtFtn()->GetStart();
                if( IsIdxInside( nPos, 1 ) )
                    Prepare( PREP_FTN, pNew );
                nClear = 0x01;
                --nCount;
            }

            if( SFX_ITEM_SET == rNewSet.GetItemState( RES_TXTATR_FIELD,
                sal_False, &pItem ))
            {
                nPos = *((SwFmtFld*)pItem)->GetTxtFld()->GetStart();
                if( IsIdxInside( nPos, 1 ) )
                {
                    const SfxPoolItem& rOldItem = ((SwAttrSetChg*)pOld)->
                                        GetChgSet()->Get( RES_TXTATR_FIELD );
                    if( pItem == &rOldItem )
                    {
                        // Nur repainten
                        // opt: invalidate aufs Window ?
                        InvalidatePage();
                        SetCompletePaint();
                    }
                    else
                        _InvalidateRange( SwCharRange( nPos, 1 ) );
                }
                nClear |= 0x02;
                --nCount;
            }
            sal_Bool bLineSpace = SFX_ITEM_SET == rNewSet.GetItemState(
                                            RES_PARATR_LINESPACING, sal_False ),
                     bRegister  = SFX_ITEM_SET == rNewSet.GetItemState(
                                            RES_PARATR_REGISTER, sal_False );
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
            if ( SFX_ITEM_SET == rNewSet.GetItemState( RES_PARATR_SPLIT,
                                                       sal_False ))
            {
                if ( GetPrev() )
                    CheckKeep();
                Prepare( PREP_CLEAR );
                InvalidateSize();
                nClear |= 0x08;
                --nCount;
            }

            if( SFX_ITEM_SET == rNewSet.GetItemState( RES_BACKGROUND, sal_False)
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

            if ( SFX_ITEM_SET ==
                 rNewSet.GetItemState( RES_TXTATR_CHARFMT, sal_False ) )
            {
                SET_WRONG( 0, STRING_LEN, false )
                SET_SCRIPT_INVAL( 0 )
            }
            else if ( SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_LANGUAGE, sal_False ) ||
                      SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_CJK_LANGUAGE, sal_False ) ||
                      SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_CTL_LANGUAGE, sal_False ) )
                SET_WRONG( 0, STRING_LEN, false )
            else if ( SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_FONT, sal_False ) ||
                      SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_CJK_FONT, sal_False ) ||
                      SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_CHRATR_CTL_FONT, sal_False ) )
                SET_SCRIPT_INVAL( 0 )
            else if ( SFX_ITEM_SET ==
                      rNewSet.GetItemState( RES_FRAMEDIR, sal_False ) )
            {
                SetDerivedR2L( sal_False );
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

            // #i88069#
            ViewShell* pViewSh = getRootFrm() ? getRootFrm()->GetCurrShell() : 0;
            if ( pViewSh  )
            {
                pViewSh->InvalidateAccessibleParaAttrs( *this );
            }
        }
        break;

        // 6870: SwDocPosUpdate auswerten.
        case RES_DOCPOS_UPDATE:
        {
            if( pOld && pNew )
            {
                const SwDocPosUpdate *pDocPos = (const SwDocPosUpdate*)pOld;
                if( pDocPos->nDocPos <= aFrm.Top() )
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
            bSetFldsDirty = sal_True;
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
                //Wird z.B. bei HiddenPara mit 0 gerufen.
                SwFrm *pNxt;
                if ( 0 != (pNxt = FindNext()) )
                    pNxt->InvalidatePrt();
            }
        }
    } // switch

    if( bSetFldsDirty )
        GetNode()->getIDocumentFieldsAccess()->SetFieldsDirty( sal_True, GetNode(), 1 );

    if ( bRecalcFtnFlag )
        CalcFtnFlag();
}

sal_Bool SwTxtFrm::GetInfo( SfxPoolItem &rHnt ) const
{
    if ( RES_VIRTPAGENUM_INFO == rHnt.Which() && IsInDocBody() && ! IsFollow() )
    {
        SwVirtPageNumInfo &rInfo = (SwVirtPageNumInfo&)rHnt;
        const SwPageFrm *pPage = FindPageFrm();
        if ( pPage )
        {
            if ( pPage == rInfo.GetOrigPage() && !GetPrev() )
            {
                //Das sollte er sein (kann allenfalls temporaer anders sein,
                //                    sollte uns das beunruhigen?)
                rInfo.SetInfo( pPage, this );
                return sal_False;
            }
            if ( pPage->GetPhyPageNum() < rInfo.GetOrigPage()->GetPhyPageNum() &&
                 (!rInfo.GetPage() || pPage->GetPhyPageNum() > rInfo.GetPage()->GetPhyPageNum()))
            {
                //Das koennte er sein.
                rInfo.SetInfo( pPage, this );
            }
        }
    }
    return sal_True;
}

/*************************************************************************
 *                      SwTxtFrm::PrepWidows()
 *************************************************************************/

void SwTxtFrm::PrepWidows( const MSHORT nNeed, sal_Bool bNotify )
{
    OSL_ENSURE(GetFollow() && nNeed, "+SwTxtFrm::Prepare: lost all friends");

    SwParaPortion *pPara = GetPara();
    if ( !pPara )
        return;
    pPara->SetPrepWidows( sal_True );

    MSHORT nHave = nNeed;

    // Wir geben ein paar Zeilen ab und schrumpfen im CalcPreps()
    SWAP_IF_NOT_SWAPPED( this )

    SwTxtSizeInfo aInf( this );
    SwTxtMargin aLine( this, &aInf );
    aLine.Bottom();
    xub_StrLen nTmpLen = aLine.GetCurr()->GetLen();
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
        sal_Bool bSplit;
        if( !IsFollow() )   //Nur ein Master entscheidet ueber Orphans
        {
            const WidowsAndOrphans aWidOrp( this );
            bSplit = ( aLine.GetLineNr() >= aWidOrp.GetOrphansLines() &&
                       aLine.GetLineNr() >= aLine.GetDropLines() );
        }
        else
            bSplit = sal_True;

        if( bSplit )
        {
            GetFollow()->SetOfst( aLine.GetEnd() );
            aLine.TruncLines( sal_True );
            if( pPara->IsFollowField() )
                GetFollow()->SetFieldFollow( sal_True );
        }
    }
    if ( bNotify )
    {
        _InvalidateSize();
        InvalidatePage();
    }

    UNDO_SWAP( this )
}

/*************************************************************************
 *                      SwTxtFrm::Prepare
 *************************************************************************/

static sal_Bool lcl_ErgoVadis( SwTxtFrm* pFrm, xub_StrLen &rPos, const PrepareHint ePrep )
{
    const SwFtnInfo &rFtnInfo = pFrm->GetNode()->GetDoc()->GetFtnInfo();
    if( ePrep == PREP_ERGOSUM )
    {
        if( !rFtnInfo.aErgoSum.Len() )
            return sal_False;;
        rPos = pFrm->GetOfst();
    }
    else
    {
        if( !rFtnInfo.aQuoVadis.Len() )
            return sal_False;
        if( pFrm->HasFollow() )
            rPos = pFrm->GetFollow()->GetOfst();
        else
            rPos = pFrm->GetTxt().Len();
        if( rPos )
            --rPos; // unser letztes Zeichen
    }
    return sal_True;
}

void SwTxtFrm::Prepare( const PrepareHint ePrep, const void* pVoid,
                        sal_Bool bNotify )
{
    SwFrmSwapper aSwapper( this, sal_False );

#if OSL_DEBUG_LEVEL > 1
    const SwTwips nDbgY = Frm().Top();
    (void)nDbgY;
#endif

    if ( IsEmpty() )
    {
        switch ( ePrep )
        {
            case PREP_BOSS_CHGD:
                SetInvalidVert( sal_True );  // Test
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
                // Gibt es ueberhaupt Flys auf der Seite ?
                SwTxtFly aTxtFly( this );
                if( aTxtFly.IsOn() )
                {
                    // Ueberlappt irgendein Fly ?
                    aTxtFly.Relax();
                    if ( aTxtFly.IsOn() || IsUndersized() )
                        break;
                }
                if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue())
                    break;

                GETGRID( FindPageFrm() )
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
        SetInvalidVert( sal_True );  // Test
        OSL_ENSURE( !IsLocked(), "SwTxtFrm::Prepare: three of a perfect pair" );
        if ( bNotify )
            InvalidateSize();
        else
            _InvalidateSize();
        return;
    }

    //Objekt mit Locking aus dem Cache holen.
    SwTxtLineAccess aAccess( this );
    SwParaPortion *pPara = aAccess.GetPara();

    switch( ePrep )
    {
        case PREP_MOVEFTN :     Frm().Height(0);
                                Prt().Height(0);
                                _InvalidatePrt();
                                _InvalidateSize();
                                // KEIN break
        case PREP_ADJUST_FRM :  pPara->SetPrepAdjust( sal_True );
                                if( IsFtnNumFrm() != pPara->IsFtnNum() ||
                                    IsUndersized() )
                                {
                                    InvalidateRange( SwCharRange( 0, 1 ), 1);
                                    if( GetOfst() && !IsFollow() )
                                        _SetOfst( 0 );
                                }
                                break;
        case PREP_MUST_FIT :        pPara->SetPrepMustFit( sal_True );
            /* no break here */
        case PREP_WIDOWS_ORPHANS :  pPara->SetPrepAdjust( sal_True );
                                    break;

        case PREP_WIDOWS :
            // MustFit ist staerker als alles anderes
            if( pPara->IsPrepMustFit() )
                return;
            // Siehe Kommentar in WidowsAndOrphans::FindOrphans und CalcPreps()
            PrepWidows( *(const MSHORT *)pVoid, bNotify );
            break;

        case PREP_FTN :
        {
            SwTxtFtn *pFtn = (SwTxtFtn *)pVoid;
            if( IsInFtn() )
            {
                // Bin ich der erste TxtFrm einer Fussnote ?
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
                        xub_StrLen nPos = pPara->GetParLen();
                        if( nPos )
                            --nPos;
                        InvalidateRange( SwCharRange( nPos, 1 ), 1);
                    }
                }
            }
            else
            {
                // Wir sind also der TxtFrm _mit_ der Fussnote
                const xub_StrLen nPos = *pFtn->GetStart();
                InvalidateRange( SwCharRange( nPos, 1 ), 1);
            }
            break;
        }
        case PREP_BOSS_CHGD :
        {
    // Test
            {
                SetInvalidVert( sal_False );
                sal_Bool bOld = IsVertical();
                SetInvalidVert( sal_True );
                if( bOld != IsVertical() )
                    InvalidateRange( SwCharRange( GetOfst(), STRING_LEN ) );
            }

            if( HasFollow() )
            {
                xub_StrLen nNxtOfst = GetFollow()->GetOfst();
                if( nNxtOfst )
                    --nNxtOfst;
                InvalidateRange( SwCharRange( nNxtOfst, 1 ), 1);
            }
            if( IsInFtn() )
            {
                xub_StrLen nPos;
                if( lcl_ErgoVadis( this, nPos, PREP_QUOVADIS ) )
                    InvalidateRange( SwCharRange( nPos, 1 ), 0 );
                if( lcl_ErgoVadis( this, nPos, PREP_ERGOSUM ) )
                    InvalidateRange( SwCharRange( nPos, 1 ), 0 );
            }
            // 4739: Wenn wir ein Seitennummernfeld besitzen, muessen wir
            // die Stellen invalidieren.
            SwpHints *pHints = GetTxtNode()->GetpSwpHints();
            if( pHints )
            {
                const sal_uInt16 nSize = pHints->Count();
                const xub_StrLen nEnd = GetFollow() ?
                                    GetFollow()->GetOfst() : STRING_LEN;
                for ( sal_uInt16 i = 0; i < nSize; ++i )
                {
                    const SwTxtAttr *pHt = (*pHints)[i];
                    const xub_StrLen nStart = *pHt->GetStart();
                    if( nStart >= GetOfst() )
                    {
                        if( nStart >= nEnd )
                            i = nSize;          // fuehrt das Ende herbei
                        else
                        {
                // 4029: wenn wir zurueckfliessen und eine Ftn besitzen, so
                // fliesst die Ftn in jedem Fall auch mit. Damit sie nicht im
                // Weg steht, schicken wir uns ein ADJUST_FRM.
                // pVoid != 0 bedeutet MoveBwd()
                            const MSHORT nWhich = pHt->Which();
                            if( RES_TXTATR_FIELD == nWhich ||
                                (HasFtn() && pVoid && RES_TXTATR_FTN == nWhich))
                            InvalidateRange( SwCharRange( nStart, 1 ), 1 );
                        }
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
                GETGRID( FindPageFrm() )
                if ( pGrid && GetTxtNode()->GetSwAttrSet().GetParaGrid().GetValue() )
                    InvalidatePrt();
            }

            // Falls wir mit niemandem ueberlappen:
            // Ueberlappte irgendein Fly _vor_ der Positionsaenderung ?
            sal_Bool bFormat = pPara->HasFly();
            if( !bFormat )
            {
                if( IsInFly() )
                {
                    SwTwips nTmpBottom = GetUpper()->Frm().Top() +
                        GetUpper()->Prt().Bottom();
                    if( nTmpBottom < Frm().Bottom() )
                        bFormat = sal_True;
                }
                if( !bFormat )
                {
                    if ( GetDrawObjs() )
                    {
                        const sal_uInt32 nCnt = GetDrawObjs()->Count();
                        for ( MSHORT i = 0; i < nCnt; ++i )
                        {
                            SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[i];
                            // #i28701# - consider all
                            // to-character anchored objects
                            if ( pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId()
                                    == FLY_AT_CHAR )
                            {
                                bFormat = sal_True;
                                break;
                            }
                        }
                    }
                    if( !bFormat )
                    {
                        // Gibt es ueberhaupt Flys auf der Seite ?
                        SwTxtFly aTxtFly( this );
                        if( aTxtFly.IsOn() )
                        {
                            // Ueberlappt irgendein Fly ?
                            aTxtFly.Relax();
                            bFormat = aTxtFly.IsOn() || IsUndersized();
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
                pPara->SetPrepAdjust( sal_True );
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
                // Wenn ein Follow uns ruft, weil eine Fussnote geloescht wird, muss unsere
                // letzte Zeile formatiert werden, damit ggf. die erste Zeile des Follows
                // hochrutschen kann, die extra auf die naechste Seite gerutscht war, um mit
                // der Fussnote zusammen zu sein, insbesondere bei spaltigen Bereichen.
                OSL_ENSURE( GetFollow(), "PREP_FTN_GONE darf nur vom Follow gerufen werden" );
                xub_StrLen nPos = GetFollow()->GetOfst();
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
                xub_StrLen nPos;
                if( lcl_ErgoVadis( this, nPos, ePrep ) )
                    InvalidateRange( SwCharRange( nPos, 1 ), 0 );
            }
            break;
        case PREP_FLY_ATTR_CHG:
        {
            if( pVoid )
            {
                xub_StrLen nWhere = CalcFlyPos( (SwFrmFmt*)pVoid );
                OSL_ENSURE( STRING_LEN != nWhere, "Prepare: Why me?" );
                InvalidateRange( SwCharRange( nWhere, 1 ) );
                return;
            }
            // else ... Laufe in den Default-Switch
        }
        case PREP_CLEAR:
        default:
        {
            if( IsLocked() )
            {
                if( PREP_FLY_ARRIVE == ePrep || PREP_FLY_LEAVE == ePrep )
                {
                    xub_StrLen nLen = ( GetFollow() ? GetFollow()->GetOfst() :
                                      STRING_LEN ) - GetOfst();
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
            return;     // damit kein SetPrep() erfolgt.
        }
    }
    if( pPara )
        pPara->SetPrep( sal_True );
}

/* --------------------------------------------------
 * Kleine Hilfsklasse mit folgender Funktion:
 * Sie soll eine Probeformatierung vorbereiten.
 * Der Frame wird in Groesse und Position angepasst, sein SwParaPortion zur Seite
 * gestellt und eine neue erzeugt, dazu wird formatiert mit gesetztem bTestFormat.
 * Im Dtor wird der TxtFrm wieder in seinen alten Zustand zurueckversetzt.
 *
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
        Max( 0L , (pFrm->Frm().*fnRect->fnGetHeight)() -
                  (pFrm->Prt().*fnRect->fnGetTop)() - nLower ) );
    (pFrm->Prt().*fnRect->fnSetWidth)(
        (pFrm->Frm().*fnRect->fnGetWidth)() -
        ( rAttrs.CalcLeft( pFrm ) + rAttrs.CalcRight( pFrm ) ) );
    pOldPara = pFrm->HasPara() ? pFrm->GetPara() : NULL;
    pFrm->SetPara( new SwParaPortion(), sal_False );

    OSL_ENSURE( ! pFrm->IsSwapped(), "A frame is swapped before _Format" );

    if ( pFrm->IsVertical() )
        pFrm->SwapWidthAndHeight();

    SwTxtFormatInfo aInf( pFrm, sal_False, sal_True, sal_True );
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


/*************************************************************************
 *                      SwTxtFrm::WouldFit()
 *************************************************************************/

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

    //Kann gut sein, dass mir der IdleCollector mir die gecachten
    //Informationen entzogen hat.
    if( !IsEmpty() )
        GetFormatted();

    // OD 2004-05-24 #i27801# - correction: 'short cut' for empty paragraph
    // can *not* be applied, if test format is in progress. The test format doesn't
    // adjust the frame and the printing area - see method <SwTxtFrm::_Format(..)>,
    // which is called in <SwTxtFrm::TestFormat(..)>
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

    // In sehr unguenstigen Faellen kann GetPara immer noch 0 sein.
    // Dann returnen wir sal_True, um auf der neuen Seite noch einmal
    // anformatiert zu werden.
    OSL_ENSURE( HasPara() || IsHiddenNow(), "WouldFit: GetFormatted() and then !HasPara()" );
    if( !HasPara() || ( !(Frm().*fnRect->fnGetHeight)() && IsHiddenNow() ) )
        return sal_True;

    // Da das Orphan-Flag nur sehr fluechtig existiert, wird als zweite
    // Bedingung  ueberprueft, ob die Rahmengroesse durch CalcPreps
    // auf riesengross gesetzt wird, um ein MoveFwd zu erzwingen.
    if( IsWidow() || ( bVert ?
                       ( 0 == Frm().Left() ) :
                       ( LONG_MAX - 20000 < Frm().Bottom() ) ) )
    {
        SetWidow(sal_False);
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
    // Ist Aufspalten ueberhaupt notwendig?
    if ( 0 != ( bSplit = !aFrmBreak.IsInside( aLine ) ) )
        bRet = !aFrmBreak.IsKeepAlways() && aFrmBreak.WouldFit( aLine, rMaxHeight, bTst );
    else
    {
        //Wir brauchen die Gesamthoehe inklusive der aktuellen Zeile
        aLine.Top();
        do
        {
            rMaxHeight -= aLine.GetLineHeight();
        } while ( aLine.Next() );
    }

    UNDO_SWAP( this )

    return bRet;
}


/*************************************************************************
 *                      SwTxtFrm::GetParHeight()
 *************************************************************************/

KSHORT SwTxtFrm::GetParHeight() const
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTxtFrm::GetParHeight with swapped frame" );

    if( !HasPara() )
    {   // Fuer nichtleere Absaetze ist dies ein Sonderfall, da koennen wir
        // bei UnderSized ruhig nur 1 Twip mehr anfordern.
        KSHORT nRet = (KSHORT)Prt().SSize().Height();
        if( IsUndersized() )
        {
            if( IsEmpty() || GetTxt().Len() == 0 )
                nRet = (KSHORT)EmptyHeight();
            else
                ++nRet;
        }
        return nRet;
    }

    // FME, OD 08.01.2004 #i11859# - refactoring and improve code
    const SwLineLayout* pLineLayout = GetPara();
    KSHORT nHeight = pLineLayout->GetRealHeight();
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


/*************************************************************************
 *                      SwTxtFrm::GetFormatted()
 *************************************************************************/

// returnt this _immer_ im formatierten Zustand!
SwTxtFrm* SwTxtFrm::GetFormatted( bool bForceQuickFormat )
{
    SWAP_IF_SWAPPED( this )

    //Kann gut sein, dass mir der IdleCollector mir die gecachten
    //Informationen entzogen hat. Calc() ruft unser Format.
                      //Nicht bei leeren Absaetzen!
    if( !HasPara() && !(IsValid() && IsEmpty()) )
    {
        // Calc() muss gerufen werden, weil unsere Frameposition
        // nicht stimmen muss.
        const sal_Bool bFormat = GetValidSizeFlag();
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

/*************************************************************************
 *                      SwTxtFrm::CalcFitToContent()
 *************************************************************************/

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

    SwTxtFormatInfo aInf( this, sal_False, sal_True, sal_True );
    aInf.SetIgnoreFly( sal_True );
    SwTxtFormatter  aLine( this, &aInf );
    SwHookOut aHook( aInf );

    // #i54031# - assure mininum of MINLAY twips.
    const SwTwips nMax = Max( (SwTwips)MINLAY,
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
        const SwNumFmt& rNumFmt =
                pTxtNode->GetNumRule()->Get( static_cast<sal_uInt16>(pTxtNode->GetActualListLevel()) );
        if ( rNumFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            // keep current paragraph portion and apply dummy paragraph portion
            SwParaPortion* pOldPara = GetPara();
            SwParaPortion *pDummy = new SwParaPortion();
            SetPara( pDummy, false );

            // lock paragraph
            SwTxtFrmLocker aLock( this );

            // simulate text formatting
            SwTxtFormatInfo aInf( this, sal_False, sal_True, sal_True );
            aInf.SetIgnoreFly( sal_True );
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
    ViewShell* pVsh = getRootFrm()->GetCurrShell();
    OSL_ENSURE( pVsh, "<SwTxtFrm::_GetHeightOfLastLineForPropLineSpacing()> - no ViewShell" );
    // #i78921# - make code robust, according to provided patch
    // There could be no <ViewShell> instance in the case of loading a binary
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

        // Wir muessen dafuer sorgen, dass am OutputDevice der Font
        // korrekt restauriert wird, sonst droht ein Last!=Owner.
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
            if ( ( !HasPara() && IsEmpty( ) ) || GetTxt().Len( ) == 0 )
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

/*************************************************************************
 *                      SwTxtFrm::GetLineSpace()
 *************************************************************************/
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

/*************************************************************************
 *                      SwTxtFrm::FirstLineHeight()
 *************************************************************************/

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

MSHORT SwTxtFrm::GetLineCount( xub_StrLen nPos )
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
        if( STRING_LEN == nPos )
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
    //not necassary to format here (GerFormatted etc.), because we have to come from there!

    sal_uLong nNew = 0;
    const SwLineNumberInfo &rInf = GetNode()->getIDocumentLineNumberAccess()->GetLineNumberInfo();
    if ( GetTxt().Len() && HasPara() )
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

            //Extend repaint to the bottom.
            if ( HasPara() )
            {
                SwRepaint *pRepaint = GetPara()->GetRepaint();
                pRepaint->Bottom( Max( pRepaint->Bottom(),
                                       Frm().Top()+Prt().Bottom()));
            }
        }
        else //Paragraphs which are not counted should not manipulate the AllLines.
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
        //If it is a follow or not has not be considered if it is a restart at each page; the
        //restart should also take affekt at follows.
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

            rPH.LineBreak();
            pLine = pLine->GetNext();
        }
    }

    rPH.Finish();
}


/*************************************************************************
 *                      SwTxtFrm::GetScriptInfo()
 *************************************************************************/

const SwScriptInfo* SwTxtFrm::GetScriptInfo() const
{
    const SwParaPortion* pPara = GetPara();
    return pPara ? &pPara->GetScriptInfo() : 0;
}

/*************************************************************************
 *                      lcl_CalcFlyBasePos()
 * Helper function for SwTxtFrm::CalcBasePosForFly()
 *************************************************************************/

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

/*************************************************************************
 *                      SwTxtFrm::CalcBasePosForFly()
 *************************************************************************/

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
    aTxtFly.SetIgnoreCurrentFrame( sal_True );
    aTxtFly.SetIgnoreContour( sal_True );
    // #118809# - ignore objects in page header|footer for
    // text frames not in page header|footer
    aTxtFly.SetIgnoreObjsInHeaderFooter( sal_True );
    SwTwips nRet1 = lcl_CalcFlyBasePos( *this, aFlyRect, aTxtFly );
    aTxtFly.SetIgnoreCurrentFrame( sal_False );
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
        ViewShell *pCurShell = pRootFrm ? pRootFrm->GetCurrShell() : NULL;
        if( pCurShell )
            pCurShell->InvalidateWindows( aRec );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
