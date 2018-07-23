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

#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <vcl/graph.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <viewopt.hxx>
#include <SwPortionHandler.hxx>
#include "porlay.hxx"
#include "porfld.hxx"
#include "inftxt.hxx"
#include <blink.hxx>
#include <frmtool.hxx>
#include <viewsh.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <rootfrm.hxx>
#include <breakit.hxx>
#include "porrst.hxx"
#include "porftn.hxx"
#include <accessibilityoptions.hxx>
#include <editeng/lrspitem.hxx>
#include <unicode/ubidi.h>

using namespace ::com::sun::star;

SwLinePortion *SwFieldPortion::Compress()
{ return (GetLen() || !m_aExpand.isEmpty() || SwLinePortion::Compress()) ? this : nullptr; }

SwFieldPortion *SwFieldPortion::Clone( const OUString &rExpand ) const
{
    SwFont *pNewFnt;
    if( nullptr != ( pNewFnt = m_pFont ) )
    {
        pNewFnt = new SwFont( *m_pFont );
    }
    // #i107143#
    // pass placeholder property to created <SwFieldPortion> instance.
    SwFieldPortion* pClone = new SwFieldPortion( rExpand, pNewFnt, m_bPlaceHolder );
    pClone->SetNextOffset( m_nNextOffset );
    pClone->m_bNoLength = m_bNoLength;
    return pClone;
}

void SwFieldPortion::TakeNextOffset( const SwFieldPortion* pField )
{
    OSL_ENSURE( pField, "TakeNextOffset: Missing Source" );
    m_nNextOffset = pField->GetNextOffset();
    m_aExpand = m_aExpand.replaceAt(0, sal_Int32(m_nNextOffset), "");
    m_bFollow = true;
}

SwFieldPortion::SwFieldPortion( const OUString &rExpand, SwFont *pFont, bool bPlaceHold )
    : m_aExpand(rExpand), m_pFont(pFont), m_nNextOffset(0), m_nNextScriptChg(COMPLETE_STRING), m_nViewWidth(0)
    , m_bFollow( false ), m_bLeft( false), m_bHide( false)
    , m_bCenter (false), m_bHasFollow( false )
    , m_bAnimated( false), m_bNoPaint( false)
    , m_bReplace( false), m_bPlaceHolder( bPlaceHold )
    , m_bNoLength( false )
    , m_nAttrFieldType(0)
{
    SetWhichPor( POR_FLD );
}

SwFieldPortion::SwFieldPortion( const SwFieldPortion& rField )
    : SwExpandPortion( rField )
    , m_aExpand( rField.GetExp() )
    , m_nNextOffset( rField.GetNextOffset() )
    , m_nNextScriptChg( rField.m_nNextScriptChg )
    , m_nViewWidth( rField.m_nViewWidth )
    , m_bFollow( rField.IsFollow() )
    , m_bLeft( rField.IsLeft() )
    , m_bHide( rField.IsHide() )
    , m_bCenter( rField.IsCenter() )
    , m_bHasFollow( rField.HasFollow() )
    , m_bAnimated ( rField.m_bAnimated )
    , m_bNoPaint( rField.m_bNoPaint)
    , m_bReplace( rField.m_bReplace )
    , m_bPlaceHolder( rField.m_bPlaceHolder )
    , m_bNoLength( rField.m_bNoLength )
    , m_nAttrFieldType( rField.m_nAttrFieldType)
{
    if ( rField.HasFont() )
        m_pFont = new SwFont( *rField.GetFont() );
    else
        m_pFont = nullptr;

    SetWhichPor( POR_FLD );
}

SwFieldPortion::~SwFieldPortion()
{
    delete m_pFont;
    if( pBlink )
        pBlink->Delete( this );
}

sal_uInt16 SwFieldPortion::GetViewWidth( const SwTextSizeInfo &rInf ) const
{
    // even though this is const, nViewWidth should be computed at the very end:
    SwFieldPortion* pThis = const_cast<SwFieldPortion*>(this);
    if( !Width() && rInf.OnWin() && !rInf.GetOpt().IsPagePreview() &&
            !rInf.GetOpt().IsReadonly() && SwViewOption::IsFieldShadings() )
    {
        if( !m_nViewWidth )
            pThis->m_nViewWidth = rInf.GetTextSize(OUString(' ')).Width();
    }
    else
        pThis->m_nViewWidth = 0;
    return m_nViewWidth;
}

/**
 * Never just use SetLen(0)
 */
class SwFieldSlot
{
    std::shared_ptr<vcl::TextLayoutCache> m_pOldCachedVclData;
    const OUString *pOldText;
    OUString aText;
    TextFrameIndex nIdx;
    TextFrameIndex nLen;
    SwTextFormatInfo *pInf;
    bool bOn;
public:
    SwFieldSlot( const SwTextFormatInfo* pNew, const SwFieldPortion *pPor );
    ~SwFieldSlot();
};

SwFieldSlot::SwFieldSlot( const SwTextFormatInfo* pNew, const SwFieldPortion *pPor )
    : pOldText(nullptr)
    , nIdx(0)
    , nLen(0)
    , pInf(nullptr)
{
    bOn = pPor->GetExpText( *pNew, aText );

    // The text will be replaced ...
    if( bOn )
    {
        pInf = const_cast<SwTextFormatInfo*>(pNew);
        nIdx = pInf->GetIdx();
        nLen = pInf->GetLen();
        pOldText = &(pInf->GetText());
        m_pOldCachedVclData = pInf->GetCachedVclData();
        pInf->SetLen(TextFrameIndex(aText.getLength()));
        pInf->SetCachedVclData(nullptr);
        if( pPor->IsFollow() )
        {
            pInf->SetFakeLineStart( nIdx > pInf->GetLineStart() );
            pInf->SetIdx(TextFrameIndex(0));
        }
        else
        {
            aText = (*pOldText).replaceAt(sal_Int32(nIdx), 1, aText);
        }
        pInf->SetText( aText );
    }
}

SwFieldSlot::~SwFieldSlot()
{
    if( bOn )
    {
        pInf->SetCachedVclData(m_pOldCachedVclData);
        pInf->SetText( *pOldText );
        pInf->SetIdx( nIdx );
        pInf->SetLen( nLen );
        pInf->SetFakeLineStart( false );
    }
}

void SwFieldPortion::CheckScript( const SwTextSizeInfo &rInf )
{
    OUString aText;
    if (!GetExpText(rInf, aText) || aText.isEmpty())
        return;

    SwFontScript nActual = m_pFont ? m_pFont->GetActual() : rInf.GetFont()->GetActual();
    sal_uInt16 nScript = g_pBreakIt->GetBreakIter()->getScriptType( aText, 0 );
    sal_Int32 nChg = 0;
    if( i18n::ScriptType::WEAK == nScript )
    {
        nChg = g_pBreakIt->GetBreakIter()->endOfScript(aText,0,nScript);
        if (nChg < aText.getLength() && nChg >= 0)
            nScript = g_pBreakIt->GetBreakIter()->getScriptType( aText, nChg );
    }

    // nNextScriptChg will be evaluated during SwFieldPortion::Format()

    if (nChg < aText.getLength() && nChg >= 0)
        m_nNextScriptChg = TextFrameIndex(
                g_pBreakIt->GetBreakIter()->endOfScript(aText, nChg, nScript));
    else
        m_nNextScriptChg = TextFrameIndex(aText.getLength());

    SwFontScript nTmp;
    switch ( nScript ) {
        case i18n::ScriptType::LATIN : nTmp = SwFontScript::Latin; break;
        case i18n::ScriptType::ASIAN : nTmp = SwFontScript::CJK; break;
        case i18n::ScriptType::COMPLEX : nTmp = SwFontScript::CTL; break;
        default: nTmp = nActual;
    }

    // #i16354# Change script type for RTL text to CTL.
    const SwScriptInfo& rSI = rInf.GetParaPortion()->GetScriptInfo();
    // #i98418#
    const sal_uInt8 nFieldDir = (IsNumberPortion() || IsFootnoteNumPortion())
        ? rSI.GetDefaultDir()
        : rSI.DirType(IsFollow() ? rInf.GetIdx() - TextFrameIndex(1) : rInf.GetIdx());

    {
        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized( aText.getLength(), 0, &nError );
        ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(aText.getStr()), aText.getLength(), nFieldDir, nullptr, &nError );
        int32_t nEnd;
        UBiDiLevel nCurrDir;
        ubidi_getLogicalRun( pBidi, 0, &nEnd, &nCurrDir );
        ubidi_close( pBidi );
        const TextFrameIndex nNextDirChg(nEnd);
        m_nNextScriptChg = std::min( m_nNextScriptChg, nNextDirChg );

        // #i89825# change the script type also to CTL
        // if there is no strong LTR char in the LTR run (numbers)
        if (nCurrDir != UBIDI_RTL &&
            (UBIDI_LTR != nFieldDir || i18n::ScriptType::COMPLEX == nScript))
        {
            nCurrDir = UBIDI_RTL;
            for( sal_Int32 nCharIdx = 0; nCharIdx < nEnd; ++nCharIdx )
            {
                UCharDirection nCharDir = u_charDirection ( aText[ nCharIdx ]);
                if ( nCharDir == U_LEFT_TO_RIGHT ||
                     nCharDir == U_LEFT_TO_RIGHT_EMBEDDING ||
                     nCharDir == U_LEFT_TO_RIGHT_OVERRIDE )
                {
                    nCurrDir = UBIDI_LTR;
                    break;
                }
            }
        }

        if (nCurrDir == UBIDI_RTL)
        {
            nTmp = SwFontScript::CTL;
            // If we decided that this range was RTL after all and the
            // previous range was complex but clipped to the start of this
            // range, then extend it to be complex over the additional RTL range
            if (nScript == i18n::ScriptType::COMPLEX)
                m_nNextScriptChg = nNextDirChg;
        }
    }

    // #i98418#
    // keep determined script type for footnote portions as preferred script type.
    // For footnote portions a font can not be created directly - see footnote
    // portion format method.
    if ( IsFootnotePortion() )
    {
        static_cast<SwFootnotePortion*>(this)->SetPreferredScriptType( nTmp );
    }
    else if ( nTmp != nActual )
    {
        if( !m_pFont )
            m_pFont = new SwFont( *rInf.GetFont() );
        m_pFont->SetActual( nTmp );
    }

}

bool SwFieldPortion::Format( SwTextFormatInfo &rInf )
{
    // Scope wegen aDiffText::DTOR!
    TextFrameIndex nRest;
    bool bFull = false;
    bool bEOL = false;
    TextFrameIndex const nTextRest = TextFrameIndex(rInf.GetText().getLength()) - rInf.GetIdx();
    {
        SwFieldSlot aDiffText( &rInf, this );
        SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
        aLayoutModeModifier.SetAuto();

        // Field portion has to be split in several parts if
        // 1. There are script/direction changes inside the field
        // 2. There are portion breaks (tab, break) inside the field:
        const TextFrameIndex nOldFullLen = rInf.GetLen();
        TextFrameIndex nFullLen = rInf.ScanPortionEnd(rInf.GetIdx(), rInf.GetIdx() + nOldFullLen) - rInf.GetIdx();
        if ( m_nNextScriptChg < nFullLen )
        {
            nFullLen = m_nNextScriptChg;
            rInf.SetHookChar( 0 );
        }
        rInf.SetLen( nFullLen );

        if (TextFrameIndex(COMPLETE_STRING) != rInf.GetUnderScorePos() &&
             rInf.GetUnderScorePos() > rInf.GetIdx() )
             rInf.SetUnderScorePos( rInf.GetIdx() );

        if( m_pFont )
            m_pFont->GoMagic( rInf.GetVsh(), m_pFont->GetActual() );

        SwFontSave aSave( rInf, m_pFont );

        // Length must be 0: the length is set for bFull after format
        // and passed along in nRest. Or else the old length would be
        // retained and be used for nRest!
        SetLen(TextFrameIndex(0));
        TextFrameIndex const nFollow(IsFollow() ? 0 : 1);

        // As odd is may seem: the query for GetLen() must return false due
        // to the ExpandPortions _after_ aDiffText (see SoftHyphs), caused
        // by SetFull.
        if( !nFullLen )
        {
            // Don't Init(), as we need height and ascent
            Width(0);
            bFull = rInf.Width() <= rInf.GetPos().X();
        }
        else
        {
            TextFrameIndex const nOldLineStart = rInf.GetLineStart();
            if( IsFollow() )
                rInf.SetLineStart(TextFrameIndex(0));
            rInf.SetNotEOL( nFullLen == nOldFullLen && nTextRest > nFollow );

            // the height depending on the fields font is set,
            // this is required for SwTextGuess::Guess
            Height( rInf.GetTextHeight() + rInf.GetFont()->GetTopBorderSpace() +
                    rInf.GetFont()->GetBottomBorderSpace() );
            // If a kerning portion is inserted after our field portion,
            // the ascent and height must be known
            SetAscent( rInf.GetAscent() + rInf.GetFont()->GetTopBorderSpace() );
            bFull = SwTextPortion::Format( rInf );
            rInf.SetNotEOL( false );
            rInf.SetLineStart( nOldLineStart );
        }
        TextFrameIndex const nTmpLen = GetLen();
        bEOL = !nTmpLen && nFollow && bFull;
        nRest = nOldFullLen - nTmpLen;

        // The char is held in the first position
        // Unconditionally after format!
        SetLen( (m_bNoLength) ? TextFrameIndex(0) : nFollow );

        if( nRest )
        {
            // aExpand has not yet been shortened; the new Ofst is a
            // result of nRest
            TextFrameIndex nNextOfst = TextFrameIndex(m_aExpand.getLength()) - nRest;

            if ( IsQuoVadisPortion() )
                nNextOfst = nNextOfst + TextFrameIndex(static_cast<SwQuoVadisPortion*>(this)->GetContText().getLength());

            OUString aNew( m_aExpand.copy(sal_Int32(nNextOfst)) );
            m_aExpand = m_aExpand.copy(0, sal_Int32(nNextOfst));

            // These characters should not be contained in the follow
            // field portion. They are handled via the HookChar mechanism.
            const sal_Unicode nNew = !aNew.isEmpty() ? aNew[0] : 0;
            switch (nNew)
            {
                case CH_BREAK  : bFull = true;
                    SAL_FALLTHROUGH;
                case ' ' :
                case CH_TAB    :
                case CHAR_HARDHYPHEN:               // non-breaking hyphen
                case CHAR_SOFTHYPHEN:
                case CHAR_HARDBLANK:
                case CHAR_ZWSP :
                case CHAR_ZWNBSP :
                case CH_TXTATR_BREAKWORD:
                case CH_TXTATR_INWORD:
                {
                    aNew = aNew.copy( 1 );
                    ++nNextOfst;
                    break;
                }
                default: ;
            }

            // Even if there is no more text left for a follow field,
            // we have to build a follow field portion (without font),
            // otherwise the HookChar mechanism would not work.
            SwFieldPortion *pField = Clone( aNew );
            if( !aNew.isEmpty() && !pField->GetFont() )
            {
                SwFont *pNewFnt = new SwFont( *rInf.GetFont() );
                pField->SetFont( pNewFnt );
            }
            pField->SetFollow( true );
            SetHasFollow( true );

            // For a newly created field, nNextOffset contains the Offset
            // of its start of the original string
            // If a FollowField is created when formatting, this FollowField's
            // Offset is being held in nNextOffset
            m_nNextOffset = m_nNextOffset + nNextOfst;
            pField->SetNextOffset( m_nNextOffset );
            rInf.SetRest( pField );
        }
    }

    if( bEOL && rInf.GetLast() && !rInf.GetUnderflow() )
        rInf.GetLast()->FormatEOL( rInf );
    return bFull;
}

void SwFieldPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    SwFontSave aSave( rInf, m_pFont );

    OSL_ENSURE(GetLen() <= TextFrameIndex(1), "SwFieldPortion::Paint: rest-portion pollution?");
    if( Width() && ( !m_bPlaceHolder || rInf.GetOpt().IsShowPlaceHolderFields() ) )
    {
        // A very liberal use of the background
        rInf.DrawViewOpt( *this, POR_FLD );
        SwExpandPortion::Paint( rInf );
    }
}

bool SwFieldPortion::GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const
{
    rText = m_aExpand;
    if( rText.isEmpty() && rInf.OnWin() &&
        !rInf.GetOpt().IsPagePreview() && !rInf.GetOpt().IsReadonly() &&
            SwViewOption::IsFieldShadings() &&
            !HasFollow() )
        rText = " ";
    return true;
}

void SwFieldPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    sal_Int32 nH = 0;
    sal_Int32 nW = 0;
    if (m_pFont)
    {
        nH = m_pFont->GetSize(m_pFont->GetActual()).Height();
        nW = m_pFont->GetSize(m_pFont->GetActual()).Width();
    }
    rPH.Special( GetLen(), m_aExpand, GetWhichPor(), nH, nW, m_pFont );
    if( GetWhichPor() == POR_FLD )
    {
        rPH.SetAttrFieldType(m_nAttrFieldType);
    }
}

SwPosSize SwFieldPortion::GetTextSize( const SwTextSizeInfo &rInf ) const
{
    SwFontSave aSave( rInf, m_pFont );
    SwPosSize aSize( SwExpandPortion::GetTextSize( rInf ) );
    return aSize;
}

SwFieldPortion *SwHiddenPortion::Clone(const OUString &rExpand ) const
{
    SwFont *pNewFnt;
    if( nullptr != ( pNewFnt = m_pFont ) )
        pNewFnt = new SwFont( *m_pFont );
    return new SwHiddenPortion( rExpand, pNewFnt );
}

void SwHiddenPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( Width() )
    {
        SwFontSave aSave( rInf, m_pFont );
        rInf.DrawViewOpt( *this, POR_HIDDEN );
        SwExpandPortion::Paint( rInf );
    }
}

bool SwHiddenPortion::GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const
{
    // Do not query for IsHidden()!
    return SwFieldPortion::GetExpText( rInf, rText );
}

SwNumberPortion::SwNumberPortion( const OUString &rExpand,
                                  SwFont *pFont,
                                  const bool bLft,
                                  const bool bCntr,
                                  const sal_uInt16 nMinDst,
                                  const bool bLabelAlignmentPosAndSpaceModeActive )
        : SwFieldPortion( rExpand, pFont ),
          nFixWidth(0),
          nMinDist( nMinDst ),
          mbLabelAlignmentPosAndSpaceModeActive( bLabelAlignmentPosAndSpaceModeActive )
{
    SetWhichPor( POR_NUMBER );
    SetLeft( bLft );
    SetHide( false );
    SetCenter( bCntr );
}

TextFrameIndex SwNumberPortion::GetCursorOfst(const sal_uInt16) const
{
    return TextFrameIndex(0);
}

SwFieldPortion *SwNumberPortion::Clone( const OUString &rExpand ) const
{
    SwFont *pNewFnt;
    if( nullptr != ( pNewFnt = m_pFont ) )
        pNewFnt = new SwFont( *m_pFont );

    return new SwNumberPortion( rExpand, pNewFnt, IsLeft(), IsCenter(),
                                nMinDist, mbLabelAlignmentPosAndSpaceModeActive );
}

/**
 * We can create multiple NumFields
 * Tricky, if one enters enough previous-text in the dialog box
 * to cause the line to overflow
 * We need to keep the Fly's evasion tactics in mind
 */
bool SwNumberPortion::Format( SwTextFormatInfo &rInf )
{
    SetHide( false );
    const bool bFull = SwFieldPortion::Format( rInf );
    SetLen(TextFrameIndex(0));
    // a numbering portion can be contained in a rotated portion!!!
    nFixWidth = rInf.IsMulti() ? Height() : Width();
    rInf.SetNumDone( !rInf.GetRest() );
    if( rInf.IsNumDone() )
    {
//        SetAscent( rInf.GetAscent() );
        OSL_ENSURE( Height() && nAscent, "NumberPortions without Height | Ascent" );

        long nDiff( 0 );

        if ( !mbLabelAlignmentPosAndSpaceModeActive )
        {
            if (!rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING) &&
                 // #i32902#
                 !IsFootnoteNumPortion() )
            {
                nDiff = rInf.Left()
                    + rInf.GetTextFrame()->GetTextNodeForParaProps()->
                    GetSwAttrSet().GetLRSpace().GetTextFirstLineOfst()
                    - rInf.First()
                    + rInf.ForcedLeftMargin();
            }
            else
            {
                nDiff = rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
            }
        }
        // The text part of the numbering should always at least
        // start at the left margin
        if( nDiff < 0 )
            nDiff = 0;
        else if ( nDiff > rInf.X() )
            nDiff -= rInf.X();
        else
            nDiff = 0;

        if( nDiff < nFixWidth + nMinDist )
            nDiff = nFixWidth + nMinDist;

        // Numbering evades the Fly, no nDiff in the second round
        // Tricky special case: FlyFrame is in an Area we're just about to
        // acquire
        // The NumberPortion is marked as hidden
        const bool bFly = rInf.GetFly() ||
            ( rInf.GetLast() && rInf.GetLast()->IsFlyPortion() );
        if( nDiff > rInf.Width() )
        {
            nDiff = rInf.Width();
            if ( bFly )
                SetHide( true );
        }

        // A numbering portion can be inside a SwRotatedPortion. Then the
        // Height has to be changed
        if ( rInf.IsMulti() )
        {
            if ( Height() < nDiff )
                Height( sal_uInt16( nDiff ) );
        }
        else if( Width() < nDiff )
            Width( sal_uInt16(nDiff) );
    }
    return bFull;
}


/**
 * A FormatEOL indicates that the subsequent text did not fit onto
 * the line anymore. In order for the Numbering to follow through,
 * we hide this NumberPortion
 */
void SwNumberPortion::FormatEOL( SwTextFormatInfo& )
{

    // This caused trouble with flys anchored as characters.
    // If one of these is numbered but does not fit to the line,
    // it calls this function, causing a loop because both the number
    // portion and the fly portion go to the next line
//    SetHide( true );
}


/**
 * A hidden NumberPortion is not displayed, unless there are TextPortions in
 * this line or there's just one line at all
 */
void SwNumberPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if ( IsHide() && rInf.GetParaPortion() && rInf.GetParaPortion()->GetNext() )
    {
        SwLinePortion *pTmp = GetPortion();
        while ( pTmp && !pTmp->InTextGrp() )
            pTmp = pTmp->GetPortion();
        if ( !pTmp )
            return;
    }

    // calculate the width of the number portion, including follows
    const sal_uInt16 nOldWidth = Width();
    sal_uInt16 nSumWidth = 0;
    sal_uInt16 nOffset = 0;

    const SwLinePortion* pTmp = this;
    while ( pTmp && pTmp->InNumberGrp() )
    {
        nSumWidth = nSumWidth + pTmp->Width();
        if ( static_cast<const SwNumberPortion*>(pTmp)->HasFollow() )
            pTmp = pTmp->GetPortion();
        else
        {
            nOffset = pTmp->Width() - static_cast<const SwNumberPortion*>(pTmp)->nFixWidth;
            break;
        }
    }

    // The master portion takes care for painting the background of the
    // follow field portions
    if ( ! IsFollow() )
    {
        SwNumberPortion *pThis = const_cast<SwNumberPortion*>(this);
        pThis->Width( nSumWidth );
        rInf.DrawViewOpt( *this, POR_NUMBER );
        pThis->Width( nOldWidth );
    }

    if( !m_aExpand.isEmpty() )
    {
        const SwFont *pTmpFnt = rInf.GetFont();
        bool bPaintSpace = ( LINESTYLE_NONE != pTmpFnt->GetUnderline() ||
                                 LINESTYLE_NONE != pTmpFnt->GetOverline()  ||
                                 STRIKEOUT_NONE != pTmpFnt->GetStrikeout() ) &&
                                 !pTmpFnt->IsWordLineMode();
        if( bPaintSpace && m_pFont )
            bPaintSpace = ( LINESTYLE_NONE != m_pFont->GetUnderline() ||
                            LINESTYLE_NONE != m_pFont->GetOverline()  ||
                            STRIKEOUT_NONE != m_pFont->GetStrikeout() ) &&
                            !m_pFont->IsWordLineMode();

        SwFontSave aSave( rInf, m_pFont );

        if( nFixWidth == Width() && ! HasFollow() )
            SwExpandPortion::Paint( rInf );
        else
        {
            // logical const: reset width
            SwNumberPortion *pThis = const_cast<SwNumberPortion*>(this);
            bPaintSpace = bPaintSpace && nFixWidth < nOldWidth;
            sal_uInt16 nSpaceOffs = nFixWidth;
            pThis->Width( nFixWidth );

            if( ( IsLeft() && ! rInf.GetTextFrame()->IsRightToLeft() ) ||
                ( ! IsLeft() && ! IsCenter() && rInf.GetTextFrame()->IsRightToLeft() ) )
                SwExpandPortion::Paint( rInf );
            else
            {
                SwTextPaintInfo aInf( rInf );
                if( nOffset < nMinDist )
                    nOffset = 0;
                else
                {
                    if( IsCenter() )
                    {
                        /* #110778# a / 2 * 2 == a is not a tautology */
                        sal_uInt16 nTmpOffset = nOffset;
                        nOffset /= 2;
                        if( nOffset < nMinDist )
                            nOffset = nTmpOffset - nMinDist;
                    }
                    else
                        nOffset = nOffset - nMinDist;
                }
                aInf.X( aInf.X() + nOffset );
                SwExpandPortion::Paint( aInf );
                if( bPaintSpace )
                    nSpaceOffs = nSpaceOffs + nOffset;
            }
            if( bPaintSpace && nOldWidth > nSpaceOffs )
            {
                SwTextPaintInfo aInf( rInf );
                aInf.X( aInf.X() + nSpaceOffs );

                // #i53199# Adjust position of underline:
                if ( rInf.GetUnderFnt() )
                {
                    const Point aNewPos( aInf.GetPos().X(), rInf.GetUnderFnt()->GetPos().Y() );
                    rInf.GetUnderFnt()->SetPos( aNewPos );
                }

                pThis->Width( nOldWidth - nSpaceOffs + 12 );
                {
                    SwTextSlot aDiffText( &aInf, this, true, false, "  " );
                    aInf.DrawText( *this, aInf.GetLen(), true );
                }
            }
            pThis->Width( nOldWidth );
        }
    }
}

SwBulletPortion::SwBulletPortion( const sal_Unicode cBullet,
                                  const OUString& rBulletFollowedBy,
                                  SwFont *pFont,
                                  const bool bLft,
                                  const bool bCntr,
                                  const sal_uInt16 nMinDst,
                                  const bool bLabelAlignmentPosAndSpaceModeActive )
    : SwNumberPortion( OUStringLiteral1(cBullet) + rBulletFollowedBy,
                       pFont, bLft, bCntr, nMinDst,
                       bLabelAlignmentPosAndSpaceModeActive )
{
    SetWhichPor( POR_BULLET );
}

#define GRFNUM_SECURE 10

SwGrfNumPortion::SwGrfNumPortion(
        const OUString& rGraphicFollowedBy,
        const SvxBrushItem* pGrfBrush,
        const SwFormatVertOrient* pGrfOrient, const Size& rGrfSize,
        const bool bLft, const bool bCntr, const sal_uInt16 nMinDst,
        const bool bLabelAlignmentPosAndSpaceModeActive ) :
    SwNumberPortion( rGraphicFollowedBy, nullptr, bLft, bCntr, nMinDst,
                     bLabelAlignmentPosAndSpaceModeActive ),
    pBrush( new SvxBrushItem(RES_BACKGROUND) ), nId( 0 )
{
    SetWhichPor( POR_GRFNUM );
    SetAnimated( false );
    m_bReplace = false;
    if( pGrfBrush )
    {
        *pBrush = *pGrfBrush;
        const Graphic* pGraph = pGrfBrush->GetGraphic();
        if( pGraph )
            SetAnimated( pGraph->IsAnimated() );
        else
            m_bReplace = true;
    }
    if( pGrfOrient )
    {
        nYPos = pGrfOrient->GetPos();
        eOrient = pGrfOrient->GetVertOrient();
    }
    else
    {
        nYPos = 0;
        eOrient = text::VertOrientation::TOP;
    }
    Width( static_cast<sal_uInt16>(rGrfSize.Width() + 2 * GRFNUM_SECURE) );
    nFixWidth = Width();
    nGrfHeight = rGrfSize.Height() + 2 * GRFNUM_SECURE;
    Height( sal_uInt16(nGrfHeight) );
    m_bNoPaint = false;
}

SwGrfNumPortion::~SwGrfNumPortion()
{
    if ( IsAnimated() )
    {
        Graphic* pGraph = const_cast<Graphic*>(pBrush->GetGraphic());
        if (pGraph)
            pGraph->StopAnimation( nullptr, nId );
    }
    delete pBrush;
}

void SwGrfNumPortion::StopAnimation( OutputDevice* pOut )
{
    if ( IsAnimated() )
    {
        Graphic* pGraph = const_cast<Graphic*>(pBrush->GetGraphic());
        if (pGraph)
            pGraph->StopAnimation( pOut, nId );
    }
}

bool SwGrfNumPortion::Format( SwTextFormatInfo &rInf )
{
    SetHide( false );
//    Width( nFixWidth );
    sal_uInt16 nFollowedByWidth( 0 );
    if ( mbLabelAlignmentPosAndSpaceModeActive )
    {
        SwFieldPortion::Format( rInf );
        nFollowedByWidth = Width();
        SetLen(TextFrameIndex(0));
    }
    Width( nFixWidth + nFollowedByWidth );
    const bool bFull = rInf.Width() < rInf.X() + Width();
    const bool bFly = rInf.GetFly() ||
        ( rInf.GetLast() && rInf.GetLast()->IsFlyPortion() );
    SetAscent( GetRelPos() > 0 ? GetRelPos() : 0 );
    if( GetAscent() > Height() )
        Height( GetAscent() );

    if( bFull )
    {
        Width( rInf.Width() - static_cast<sal_uInt16>(rInf.X()) );
        if( bFly )
        {
            SetLen(TextFrameIndex(0));
            m_bNoPaint = true;
            rInf.SetNumDone( false );
            return true;
        }
    }
    rInf.SetNumDone( true );
//    long nDiff = rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
    long nDiff = mbLabelAlignmentPosAndSpaceModeActive
                 ? 0
                 : rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
    // The TextPortion should at least always start on the
    // left margin
    if( nDiff < 0 )
        nDiff = 0;
    else if ( nDiff > rInf.X() )
        nDiff -= rInf.X();
    if( nDiff < nFixWidth + nMinDist )
        nDiff = nFixWidth + nMinDist;

    // Numbering evades Fly, no nDiff in the second round
    // Tricky special case: FlyFrame is in the Area we were just
    // about to get a hold of.
    // The NumberPortion is marked as hidden
    if( nDiff > rInf.Width() )
    {
        nDiff = rInf.Width();
        if( bFly )
            SetHide( true );
    }

    if( Width() < nDiff )
        Width( sal_uInt16(nDiff) );
    return bFull;
}


/**
 * A hidden NumberPortion is not displayed, unless there are TextPortions in
 * this line or there's only one line at all
 */
void SwGrfNumPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( m_bNoPaint )
        return;
    if ( IsHide() && rInf.GetParaPortion() && rInf.GetParaPortion()->GetNext() )
    {
        SwLinePortion *pTmp = GetPortion();
        while ( pTmp && !pTmp->InTextGrp() )
            pTmp = pTmp->GetPortion();
        if ( !pTmp )
            return;
    }
    Point aPos( rInf.X() + GRFNUM_SECURE, rInf.Y() - GetRelPos() + GRFNUM_SECURE );
    long nTmpWidth = std::max( long(0), static_cast<long>(nFixWidth - 2 * GRFNUM_SECURE) );
    Size aSize( nTmpWidth, GetGrfHeight() - 2 * GRFNUM_SECURE );

    const bool bTmpLeft = mbLabelAlignmentPosAndSpaceModeActive ||
                              ( IsLeft() && ! rInf.GetTextFrame()->IsRightToLeft() ) ||
                              ( ! IsLeft() && ! IsCenter() && rInf.GetTextFrame()->IsRightToLeft() );

    if( nFixWidth < Width() && !bTmpLeft )
    {
        sal_uInt16 nOffset = Width() - nFixWidth;
        if( nOffset < nMinDist )
            nOffset = 0;
        else
        {
            if( IsCenter() )
            {
                nOffset /= 2;
                if( nOffset < nMinDist )
                    nOffset = Width() - nFixWidth - nMinDist;
            }
            else
                nOffset = nOffset - nMinDist;
        }
        aPos.AdjustX(nOffset );
    }

    if( m_bReplace )
    {
        const long nTmpH = GetPortion() ? GetPortion()->GetAscent() : 120;
        aSize = Size( nTmpH, nTmpH );
        aPos.setY( rInf.Y() - nTmpH );
    }
    SwRect aTmp( aPos, aSize );

    bool bDraw = true;

    if ( IsAnimated() )
    {
        bDraw = !rInf.GetOpt().IsGraphic();
        if( !nId )
        {
            SetId( sal_IntPtr( rInf.GetTextFrame() ) );
            rInf.GetTextFrame()->SetAnimation();
        }
        if( aTmp.IsOver( rInf.GetPaintRect() ) && !bDraw )
        {
            rInf.NoteAnimation();
            const SwViewShell* pViewShell = rInf.GetVsh();

            // virtual device, not pdf export
            if( OUTDEV_VIRDEV == rInf.GetOut()->GetOutDevType() &&
                pViewShell && pViewShell->GetWin()  )
            {
                Graphic* pGraph = const_cast<Graphic*>(pBrush->GetGraphic());
                if (pGraph)
                    pGraph->StopAnimation(nullptr,nId);
                rInf.GetTextFrame()->getRootFrame()->GetCurrShell()->InvalidateWindows( aTmp );
            }

            else if ( pViewShell &&
                     !pViewShell->GetAccessibilityOptions()->IsStopAnimatedGraphics() &&
                     !pViewShell->IsPreview() &&
                      // #i9684# Stop animation during printing/pdf export.
                      pViewShell->GetWin() )
            {
                Graphic* pGraph = const_cast<Graphic*>(pBrush->GetGraphic());
                if (pGraph)
                {
                    pGraph->StartAnimation(
                        const_cast<OutputDevice*>(rInf.GetOut()), aPos, aSize, nId );
                }
            }

            // pdf export, printing, preview, stop animations...
            else
                bDraw = true;
        }
        if( bDraw )
        {

            Graphic* pGraph = const_cast<Graphic*>(pBrush->GetGraphic());
            if (pGraph)
                pGraph->StopAnimation( nullptr, nId );
        }
    }

    SwRect aRepaint( rInf.GetPaintRect() );
    const SwTextFrame& rFrame = *rInf.GetTextFrame();
    if( rFrame.IsVertical() )
    {
        rFrame.SwitchHorizontalToVertical( aTmp );
        rFrame.SwitchHorizontalToVertical( aRepaint );
    }

    if( rFrame.IsRightToLeft() )
    {
        rFrame.SwitchLTRtoRTL( aTmp );
        rFrame.SwitchLTRtoRTL( aRepaint );
    }

    if( bDraw && aTmp.HasArea() )
    {
        DrawGraphic( pBrush, const_cast<OutputDevice*>(rInf.GetOut()),
            aTmp, aRepaint, m_bReplace ? GRFNUM_REPLACE : GRFNUM_YES );
    }
}

void SwGrfNumPortion::SetBase( long nLnAscent, long nLnDescent,
                               long nFlyAsc, long nFlyDesc )
{
    if ( GetOrient() != text::VertOrientation::NONE )
    {
        SetRelPos( 0 );
        if ( GetOrient() == text::VertOrientation::CENTER )
            SetRelPos( GetGrfHeight() / 2 );
        else if ( GetOrient() == text::VertOrientation::TOP )
            SetRelPos( GetGrfHeight() - GRFNUM_SECURE );
        else if ( GetOrient() == text::VertOrientation::BOTTOM )
            ;
        else if ( GetOrient() == text::VertOrientation::CHAR_CENTER )
            SetRelPos( ( GetGrfHeight() + nLnAscent - nLnDescent ) / 2 );
        else if ( GetOrient() == text::VertOrientation::CHAR_TOP )
            SetRelPos( nLnAscent );
        else if ( GetOrient() == text::VertOrientation::CHAR_BOTTOM )
            SetRelPos( GetGrfHeight() - nLnDescent );
        else
        {
            if( GetGrfHeight() >= nFlyAsc + nFlyDesc )
            {
                // If I'm as large as the line, I do not need to adjust
                // at the line; I'll leave the max. ascent unchanged
                SetRelPos( nFlyAsc );
            }
            else if ( GetOrient() == text::VertOrientation::LINE_CENTER )
                SetRelPos( ( GetGrfHeight() + nFlyAsc - nFlyDesc ) / 2 );
            else if ( GetOrient() == text::VertOrientation::LINE_TOP )
                SetRelPos( nFlyAsc );
            else if ( GetOrient() == text::VertOrientation::LINE_BOTTOM )
                SetRelPos( GetGrfHeight() - nFlyDesc );
        }
    }
}

void SwTextFrame::StopAnimation( OutputDevice* pOut )
{
    OSL_ENSURE( HasAnimation(), "SwTextFrame::StopAnimation: Which Animation?" );
    if( HasPara() )
    {
        SwLineLayout *pLine = GetPara();
        while( pLine )
        {
            SwLinePortion *pPor = pLine->GetPortion();
            while( pPor )
            {
                if( pPor->IsGrfNumPortion() )
                    static_cast<SwGrfNumPortion*>(pPor)->StopAnimation( pOut );
                // The NumberPortion is always at the first char,
                // which means we can cancel as soon as we've reached a portion
                // with a length > 0
                pPor = pPor->GetLen() ? nullptr : pPor->GetPortion();
            }
            pLine = pLine->GetLen() ? nullptr : pLine->GetNext();
        }
    }
}

/**
 * Initializes the script array and clears the width array
 */
SwCombinedPortion::SwCombinedPortion( const OUString &rText )
    : SwFieldPortion( rText )
    , nUpPos(0)
    , nLowPos(0)
    , nProportion(55)
{
    SetLen(TextFrameIndex(1));
    SetWhichPor( POR_COMBINED );
    if( m_aExpand.getLength() > 6 )
        m_aExpand = m_aExpand.copy( 0, 6 );

    // Initialization of the scripttype array,
    // the arrays of width and position are filled by the format function
    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

    SwFontScript nScr = SW_SCRIPTS;
    for( sal_Int32 i = 0; i < rText.getLength(); ++i )
    {
        switch ( g_pBreakIt->GetBreakIter()->getScriptType( rText, i ) ) {
            case i18n::ScriptType::LATIN : nScr = SwFontScript::Latin; break;
            case i18n::ScriptType::ASIAN : nScr = SwFontScript::CJK; break;
            case i18n::ScriptType::COMPLEX : nScr = SwFontScript::CTL; break;
        }
        aScrType[i] = nScr;
    }

    memset( &aWidth, 0, sizeof(aWidth) );
}

void SwCombinedPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    OSL_ENSURE(GetLen() <= TextFrameIndex(1), "SwFieldPortion::Paint: rest-portion pollution?");
    if( !Width() )
        return;

    rInf.DrawBackBrush( *this );
    rInf.DrawViewOpt( *this, POR_FLD );

    // do we have to repaint a post it portion?
    if( rInf.OnWin() && pPortion && !pPortion->Width() )
        pPortion->PrePaint( rInf, this );

    const sal_Int32 nCount = m_aExpand.getLength();
    if( !nCount )
        return;
    OSL_ENSURE( nCount < 7, "Too much combined characters" );

    // the first character of the second row
    const sal_Int32 nTop = ( nCount + 1 ) / 2;

    SwFont aTmpFont( *rInf.GetFont() );
    aTmpFont.SetProportion( nProportion );  // a smaller font
    SwFontSave aFontSave( rInf, &aTmpFont );

    Point aOldPos = rInf.GetPos();
    Point aOutPos( aOldPos.X(), aOldPos.Y() - nUpPos );// Y of the first row
    for( sal_Int32 i = 0 ; i < nCount; ++i )
    {
        if( i == nTop ) // change the row
            aOutPos.setY( aOldPos.Y() + nLowPos );    // Y of the second row
        aOutPos.setX( aOldPos.X() + aPos[i] );        // X position
        const SwFontScript nAct = aScrType[i];        // script type
        aTmpFont.SetActual( nAct );

        // if there're more than 4 characters to display, we choose fonts
        // with 2/3 of the original font width.
        if( aWidth[ nAct ] )
        {
            Size aTmpSz = aTmpFont.GetSize( nAct );
            if( aTmpSz.Width() != aWidth[ nAct ] )
            {
                aTmpSz.setWidth( aWidth[ nAct ] );
                aTmpFont.SetSize( aTmpSz, nAct );
            }
        }
        const_cast<SwTextPaintInfo&>(rInf).SetPos( aOutPos );
        rInf.DrawText(m_aExpand, *this, TextFrameIndex(i), TextFrameIndex(1));
    }
    // rInf is const, so we have to take back our manipulations
    const_cast<SwTextPaintInfo&>(rInf).SetPos( aOldPos );

}

bool SwCombinedPortion::Format( SwTextFormatInfo &rInf )
{
    const sal_Int32 nCount = m_aExpand.getLength();
    if( !nCount )
    {
        Width( 0 );
        return false;
    }

    OSL_ENSURE( nCount < 7, "Too much combined characters" );

    // If there are leading "weak"-scripttyped characters in this portion,
    // they get the actual scripttype.
    for( sal_Int32 i = 0; i < nCount && SW_SCRIPTS == aScrType[i]; ++i )
        aScrType[i] = rInf.GetFont()->GetActual();
    if( nCount > 4 )
    {
        // more than four? Ok, then we need the 2/3 font width
        for( sal_Int32 i = 0; i < m_aExpand.getLength(); ++i )
        {
            OSL_ENSURE( aScrType[i] < SW_SCRIPTS, "Combined: Script fault" );
            if( !aWidth[ aScrType[i] ] )
            {
                rInf.GetOut()->SetFont( rInf.GetFont()->GetFnt( aScrType[i] ) );
                aWidth[ aScrType[i] ] =
                        static_cast<sal_uInt16>(2 * rInf.GetOut()->GetFontMetric().GetFontSize().Width() / 3);
            }
        }
    }

    const sal_Int32 nTop = ( nCount + 1 ) / 2; // the first character of the second line
    SwViewShell *pSh = rInf.GetTextFrame()->getRootFrame()->GetCurrShell();
    SwFont aTmpFont( *rInf.GetFont() );
    SwFontSave aFontSave( rInf, &aTmpFont );
    nProportion = 55;
    // In nMainAscent/Descent we store the ascent and descent
    // of the original surrounding font
    sal_uInt16 nMaxDescent, nMaxAscent, nMaxWidth;
    sal_uInt16 nMainDescent = rInf.GetFont()->GetHeight( pSh, *rInf.GetOut() );
    const sal_uInt16 nMainAscent = rInf.GetFont()->GetAscent( pSh, *rInf.GetOut() );
    nMainDescent = nMainDescent - nMainAscent;
    // we start with a 50% font, but if we notice that the combined portion
    // becomes bigger than the surrounding font, we check 45% and maybe 40%.
    do
    {
        nProportion -= 5;
        aTmpFont.SetProportion( nProportion );
        memset( &aPos, 0, sizeof(aPos) );
        nMaxDescent = 0;
        nMaxAscent = 0;
        nMaxWidth = 0;
        nUpPos = nLowPos = 0;

        // Now we get the width of all characters.
        // The ascent and the width of the first line are stored in the
        // ascent member of the portion, the descent in nLowPos.
        // The ascent, descent and width of the second line are stored in the
        // local nMaxAscent, nMaxDescent and nMaxWidth variables.
        for( sal_Int32 i = 0; i < nCount; ++i )
        {
            SwFontScript nScrp = aScrType[i];
            aTmpFont.SetActual( nScrp );
            if( aWidth[ nScrp ] )
            {
                Size aFontSize( aTmpFont.GetSize( nScrp ) );
                aFontSize.setWidth( aWidth[ nScrp ] );
                aTmpFont.SetSize( aFontSize, nScrp );
            }

            SwDrawTextInfo aDrawInf(pSh, *rInf.GetOut(), m_aExpand, i, 1);
            Size aSize = aTmpFont.GetTextSize_( aDrawInf );
            const sal_uInt16 nAsc = aTmpFont.GetAscent( pSh, *rInf.GetOut() );
            aPos[ i ] = static_cast<sal_uInt16>(aSize.Width());
            if( i == nTop ) // enter the second line
            {
                nLowPos = nMaxDescent;
                Height( nMaxDescent + nMaxAscent );
                Width( nMaxWidth );
                SetAscent( nMaxAscent );
                nMaxAscent = 0;
                nMaxDescent = 0;
                nMaxWidth = 0;
            }
            nMaxWidth = nMaxWidth + aPos[ i ];
            if( nAsc > nMaxAscent )
                nMaxAscent = nAsc;
            if( aSize.Height() - nAsc > nMaxDescent )
                nMaxDescent = static_cast<sal_uInt16>(aSize.Height() - nAsc);
        }
        // for one or two characters we double the width of the portion
        if( nCount < 3 )
        {
            nMaxWidth *= 2;
            Width( 2*Width() );
            if( nCount < 2 )
            {
                Height( nMaxAscent + nMaxDescent );
                nLowPos = nMaxDescent;
            }
        }
        Height( Height() + nMaxDescent + nMaxAscent );
        nUpPos = nMaxAscent;
        SetAscent( Height() - nMaxDescent - nLowPos );
    } while( nProportion > 40 && ( GetAscent() > nMainAscent ||
                                    Height() - GetAscent() > nMainDescent ) );
    // if the combined portion is smaller than the surrounding text,
    // the portion grows. This looks better, if there's a character background.
    if( GetAscent() < nMainAscent )
    {
        Height( Height() + nMainAscent - GetAscent() );
        SetAscent( nMainAscent );
    }
    if( Height() < nMainAscent + nMainDescent )
        Height( nMainAscent + nMainDescent );

    // We calculate the x positions of the characters in both lines..
    sal_uInt16 nTopDiff = 0;
    sal_uInt16 nBotDiff = 0;
    if( nMaxWidth > Width() )
    {
        nTopDiff = ( nMaxWidth - Width() ) / 2;
        Width( nMaxWidth );
    }
    else
        nBotDiff = ( Width() - nMaxWidth ) / 2;
    switch( nTop)
    {
        case 3: aPos[1] = aPos[0] + nTopDiff;
            SAL_FALLTHROUGH;
        case 2: aPos[nTop-1] = Width() - aPos[nTop-1];
    }
    aPos[0] = 0;
    switch( nCount )
    {
        case 5: aPos[4] = aPos[3] + nBotDiff;
            SAL_FALLTHROUGH;
        case 3: aPos[nTop] = nBotDiff;          break;
        case 6: aPos[4] = aPos[3] + nBotDiff;
            SAL_FALLTHROUGH;
        case 4: aPos[nTop] = 0;
            SAL_FALLTHROUGH;
        case 2: aPos[nCount-1] = Width() - aPos[nCount-1];
    }

    // Does the combined portion fit the line?
    const bool bFull = rInf.Width() < rInf.X() + Width();
    if( bFull )
    {
        if( rInf.GetLineStart() == rInf.GetIdx() && (!rInf.GetLast()->InFieldGrp()
            || !static_cast<SwFieldPortion*>(rInf.GetLast())->IsFollow() ) )
            Width( static_cast<sal_uInt16>( rInf.Width() - rInf.X() ) );
        else
        {
            Truncate();
            Width( 0 );
            SetLen(TextFrameIndex(0));
            if( rInf.GetLast() )
                rInf.GetLast()->FormatEOL( rInf );
        }
    }
    return bFull;
}

sal_uInt16 SwCombinedPortion::GetViewWidth( const SwTextSizeInfo &rInf ) const
{
    if( !GetLen() ) // for the dummy part at the end of the line, where
        return 0;   // the combined portion doesn't fit.
    return SwFieldPortion::GetViewWidth( rInf );
}

SwFieldPortion *SwFieldFormDropDownPortion::Clone(const OUString &rExpand) const
{
    return new SwFieldFormDropDownPortion(rExpand);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
