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
#include <utility>

#include <comphelper/string.hxx>
#include <vcl/graph.hxx>
#include <editeng/brushitem.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/pdfwriter.hxx>
#include <viewopt.hxx>
#include <SwPortionHandler.hxx>
#include "porlay.hxx"
#include "porfld.hxx"
#include "inftxt.hxx"
#include <fmtornt.hxx>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <viewsh.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <rootfrm.hxx>
#include <breakit.hxx>
#include "porftn.hxx"
#include <accessibilityoptions.hxx>
#include <editeng/lrspitem.hxx>
#include <unicode/ubidi.h>
#include <bookmark.hxx>
#include <docufld.hxx>

using namespace ::com::sun::star;

SwLinePortion *SwFieldPortion::Compress()
{ return (GetLen() || !m_aExpand.isEmpty() || SwLinePortion::Compress()) ? this : nullptr; }

SwFieldPortion *SwFieldPortion::Clone( const OUString &rExpand ) const
{
    std::unique_ptr<SwFont> pNewFnt;
    if( m_pFont )
    {
        pNewFnt.reset(new SwFont( *m_pFont ));
    }
    // #i107143#
    // pass placeholder property to created <SwFieldPortion> instance.
    SwFieldPortion* pClone = new SwFieldPortion(rExpand, std::move(pNewFnt));
    pClone->SetNextOffset( m_nNextOffset );
    pClone->m_bNoLength = m_bNoLength;
    return pClone;
}

void SwFieldPortion::TakeNextOffset( const SwFieldPortion* pField )
{
    assert(pField && "TakeNextOffset: Missing Source");
    m_nNextOffset = pField->GetNextOffset();
    m_aExpand = m_aExpand.replaceAt(0, sal_Int32(m_nNextOffset), u"");
    m_bFollow = true;
}

SwFieldPortion::SwFieldPortion(OUString aExpand, std::unique_ptr<SwFont> pFont, TextFrameIndex const nFieldLen)
    : m_aExpand(std::move(aExpand)), m_pFont(std::move(pFont)), m_nNextOffset(0)
    , m_nNextScriptChg(COMPLETE_STRING), m_nFieldLen(nFieldLen), m_nViewWidth(0)
    , m_bFollow( false ), m_bLeft( false), m_bHide( false)
    , m_bCenter (false), m_bHasFollow( false )
    , m_bAnimated( false), m_bNoPaint( false)
    , m_bReplace(false)
    , m_bNoLength( false )
{
    SetWhichPor( PortionType::Field );
}

SwFieldPortion::SwFieldPortion( const SwFieldPortion& rField )
    : SwExpandPortion( rField )
    , m_aExpand( rField.GetExp() )
    , m_nNextOffset( rField.GetNextOffset() )
    , m_nNextScriptChg( rField.m_nNextScriptChg )
    , m_nFieldLen(rField.m_nFieldLen)
    , m_nViewWidth( rField.m_nViewWidth )
    , m_bFollow( rField.IsFollow() )
    , m_bLeft( rField.IsLeft() )
    , m_bHide( rField.IsHide() )
    , m_bCenter( rField.IsCenter() )
    , m_bHasFollow( rField.HasFollow() )
    , m_bAnimated ( rField.m_bAnimated )
    , m_bNoPaint( rField.m_bNoPaint)
    , m_bReplace( rField.m_bReplace )
    , m_bNoLength( rField.m_bNoLength )
{
    if ( rField.HasFont() )
        m_pFont.reset( new SwFont( *rField.GetFont() ) );

    SetWhichPor( PortionType::Field );
}

SwFieldPortion::~SwFieldPortion()
{
    m_pFont.reset();
}

sal_uInt16 SwFieldPortion::GetViewWidth( const SwTextSizeInfo &rInf ) const
{
    // even though this is const, nViewWidth should be computed at the very end:
    SwFieldPortion* pThis = const_cast<SwFieldPortion*>(this);
    if( !Width() && rInf.OnWin() && !rInf.GetOpt().IsPagePreview() &&
            !rInf.GetOpt().IsReadonly() && rInf.GetOpt().IsFieldShadings() )
    {
        if( !m_nViewWidth )
            pThis->m_nViewWidth = rInf.GetTextSize(OUString(' ')).Width();
    }
    else
        pThis->m_nViewWidth = 0;
    return m_nViewWidth;
}

namespace {

/**
 * Never just use SetLen(0)
 */
class SwFieldSlot
{
    std::shared_ptr<const vcl::text::TextLayoutCache> m_pOldCachedVclData;
    const OUString *pOldText;
    OUString aText;
    TextFrameIndex nIdx;
    TextFrameIndex nLen;
    sal_Unicode nOrigHookChar;
    SwTextFormatInfo *pInf;
    bool bOn;
public:
    SwFieldSlot( const SwTextFormatInfo* pNew, const SwFieldPortion *pPor );
    ~SwFieldSlot();
};

}

SwFieldSlot::SwFieldSlot( const SwTextFormatInfo* pNew, const SwFieldPortion *pPor )
    : pOldText(nullptr)
    , nIdx(0)
    , nLen(0)
    , nOrigHookChar(0)
    , pInf(nullptr)
{
    bOn = pPor->GetExpText( *pNew, aText );

    // The text will be replaced ...
    if( !bOn )
        return;

    pInf = const_cast<SwTextFormatInfo*>(pNew);
    nIdx = pInf->GetIdx();
    nLen = pInf->GetLen();
    pOldText = &(pInf->GetText());
    nOrigHookChar = pInf->GetHookChar();
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
        TextFrameIndex nEnd(pOldText->getLength());
        if (nIdx < nEnd)
        {
            sal_Int32 const nFieldLen(pPor->GetFieldLen());
            aText = (*pOldText).replaceAt(sal_Int32(nIdx), nFieldLen, aText);
        }
        else if (nIdx == nEnd)
            aText = *pOldText + aText;
        else
            SAL_WARN("sw.core", "SwFieldSlot bad SwFieldPortion index.");
    }
    pInf->SetText( aText );
}

SwFieldSlot::~SwFieldSlot()
{
    if( bOn )
    {
        pInf->SetCachedVclData(m_pOldCachedVclData);
        pInf->SetText( *pOldText );
        // ofz#64109 at last for ruby-text when we restore the original text to
        // continue laying out the 'body' text of the ruby, then a tab or other
        // 'hook char' in the text drawn above it shouldn't affect the 'body'
        // While there are other cases, such as tdf#148360, where the tab in an
        // inline expanded field, that should affect the body.
        if (pInf->IsRuby())
            pInf->SetHookChar(nOrigHookChar);
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
        : rSI.DirType(IsFollow() ? rInf.GetIdx() - m_nFieldLen : rInf.GetIdx());

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
            m_pFont.reset( new SwFont( *rInf.GetFont() ) );
        m_pFont->SetActual( nTmp );
    }

}

bool SwFieldPortion::Format( SwTextFormatInfo &rInf )
{
    // Scope wegen aDiffText::DTOR!
    bool bFull = false;
    bool bEOL = false;
    TextFrameIndex const nTextRest = TextFrameIndex(rInf.GetText().getLength()) - rInf.GetIdx();
    {
        TextFrameIndex nRest;
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
            m_pFont->AllocFontCacheId( rInf.GetVsh(), m_pFont->GetActual() );

        SwFontSave aSave( rInf, m_pFont.get() );

        // Length must be 0: the length is set for bFull after format
        // and passed along in nRest. Or else the old length would be
        // retained and be used for nRest!
        SetLen(TextFrameIndex(0));
        TextFrameIndex const nFollow(IsFollow() ? TextFrameIndex(0) : m_nFieldLen);

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
        SetLen( m_bNoLength ? TextFrameIndex(0) : nFollow );

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
            auto IsHook = [](const sal_Unicode cNew, bool const isSpace = false) -> bool
            {
                switch (cNew)
                {
                    case ' ': // tdf#159101 this one is not in ScanPortionEnd
                              // but is required for justified text
                        return isSpace;
                    case CH_BREAK:
                    case CH_TAB:
                    case CHAR_HARDHYPHEN: // non-breaking hyphen
                    case CHAR_SOFTHYPHEN:
                    case CHAR_HARDBLANK:
                    case CHAR_ZWSP:
                    case CHAR_WJ:
                    case CH_TXTATR_BREAKWORD:
                    case CH_TXTATR_INWORD:
                    {
                        return true;
                    }
                    default:
                        return false;
                }
            };
            if (IsHook(nNew, true))
            {
                if (nNew == CH_BREAK)
                {
                    bFull = true;
                }
                aNew = aNew.copy(1);
                ++nNextOfst;
            }

            // Even if there is no more text left for a follow field,
            // we have to build a follow field portion (without font),
            // otherwise the HookChar mechanism would not work.
            SwFieldPortion *pField = Clone( aNew );
            if( !aNew.isEmpty() && !pField->GetFont() )
            {
                pField->SetFont( std::make_unique<SwFont>( *rInf.GetFont() ) );
            }
            if (IsFollow() || Compress())
            {   // empty this will be deleted in SwLineLayout::CalcLine()
                // anyway so make sure pField doesn't have a stale flag
                pField->SetFollow( true );
            }
            if (pField->Compress() && !std::all_of(std::u16string_view(aNew).begin(),
                        std::u16string_view(aNew).end(), IsHook))
            {   // empty pField will be deleted in SwLineLayout::CalcLine()
                // anyway so make sure this one doesn't have a stale flag
                SetHasFollow( true );
            }

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
    SwFontSave aSave( rInf, m_pFont.get() );

//    OSL_ENSURE(GetLen() <= TextFrameIndex(1), "SwFieldPortion::Paint: rest-portion pollution?");
    if (Width() && !m_bContentControl)
    {
        // A very liberal use of the background
        rInf.DrawViewOpt( *this, PortionType::Field );
        SwExpandPortion::Paint( rInf );
    }
}

bool SwFieldPortion::GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const
{
    rText = m_aExpand;
    if( rText.isEmpty() && rInf.OnWin() &&
        !rInf.GetOpt().IsPagePreview() && !rInf.GetOpt().IsReadonly() &&
            rInf.GetOpt().IsFieldShadings() &&
            !HasFollow() )
        rText = " ";
    return true;
}

void SwFieldPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Special( GetLen(), m_aExpand, GetWhichPor() );
}

void SwFieldPortion::dumpAsXml(xmlTextWriterPtr pWriter, const OUString& rText,
                               TextFrameIndex& nOffset) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFieldPortion"));
    dumpAsXmlAttributes(pWriter, rText, nOffset);
    nOffset += GetLen();

    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("expand"), BAD_CAST(m_aExpand.toUtf8().getStr()));

    if (m_pFont)
    {
        m_pFont->dumpAsXml(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

SwPosSize SwFieldPortion::GetTextSize( const SwTextSizeInfo &rInf ) const
{
    SwFontSave aSave( rInf, m_pFont.get() );
    SwPosSize aSize( SwExpandPortion::GetTextSize( rInf ) );
    return aSize;
}

SwFieldPortion *SwHiddenPortion::Clone(const OUString &rExpand ) const
{
    std::unique_ptr<SwFont> pNewFnt;
    if( m_pFont )
        pNewFnt.reset(new SwFont( *m_pFont ));
    return new SwHiddenPortion( rExpand, std::move(pNewFnt) );
}

void SwHiddenPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( Width() )
    {
        SwFontSave aSave( rInf, m_pFont.get() );
        rInf.DrawViewOpt( *this, PortionType::Hidden );
        SwExpandPortion::Paint( rInf );
    }
}

bool SwHiddenPortion::GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const
{
    // Do not query for IsHidden()!
    return SwFieldPortion::GetExpText( rInf, rText );
}

SwNumberPortion::SwNumberPortion( const OUString &rExpand,
                                  std::unique_ptr<SwFont> pFont,
                                  const bool bLft,
                                  const bool bCntr,
                                  const sal_uInt16 nMinDst,
                                  const bool bLabelAlignmentPosAndSpaceModeActive )
    : SwFieldPortion(rExpand, std::move(pFont), TextFrameIndex(0))
    , m_nFixWidth(0)
    , m_nMinDist(nMinDst)
    , mbLabelAlignmentPosAndSpaceModeActive(bLabelAlignmentPosAndSpaceModeActive)
{
    SetWhichPor( PortionType::Number );
    SetLeft( bLft );
    SetHide( false );
    SetCenter( bCntr );
}

TextFrameIndex SwNumberPortion::GetModelPositionForViewPoint(const sal_uInt16) const
{
    return TextFrameIndex(0);
}

SwFieldPortion *SwNumberPortion::Clone( const OUString &rExpand ) const
{
    std::unique_ptr<SwFont> pNewFnt;
    if( m_pFont )
        pNewFnt.reset(new SwFont( *m_pFont ));

    return new SwNumberPortion( rExpand, std::move(pNewFnt), IsLeft(), IsCenter(),
                                m_nMinDist, mbLabelAlignmentPosAndSpaceModeActive );
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
    m_nFixWidth = rInf.IsMulti() ? Height() : Width();
    rInf.SetNumDone( !rInf.GetRest() );
    if( rInf.IsNumDone() )
    {
//        SetAscent( rInf.GetAscent() );
        OSL_ENSURE( Height() && mnAscent, "NumberPortions without Height | Ascent" );

        tools::Long nDiff( 0 );

        if ( !mbLabelAlignmentPosAndSpaceModeActive )
        {
            if ((!rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING) &&
                 // #i32902#
                 !IsFootnoteNumPortion()) ||
                 // tdf#159382
                (IsFootnoteNumPortion() &&
                 rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::NO_GAP_AFTER_NOTE_NUMBER)))
            {
                nDiff = rInf.Left()
                    + rInf.GetTextFrame()->GetTextNodeForParaProps()->
                        GetSwAttrSet().GetFirstLineIndent().GetTextFirstLineOffset()
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

        if( nDiff < m_nFixWidth + m_nMinDist )
            nDiff = m_nFixWidth + m_nMinDist;

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
                Height( nDiff );
        }
        else if( Width() < nDiff )
            Width( nDiff );
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
        SwLinePortion *pTmp = GetNextPortion();
        while ( pTmp && !pTmp->InTextGrp() )
            pTmp = pTmp->GetNextPortion();
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
            pTmp = pTmp->GetNextPortion();
        else
        {
            nOffset = pTmp->Width() - static_cast<const SwNumberPortion*>(pTmp)->m_nFixWidth;
            break;
        }
    }

    // The master portion takes care for painting the background of the
    // follow field portions
    if ( ! IsFollow() )
    {
        SwNumberPortion *pThis = const_cast<SwNumberPortion*>(this);
        pThis->Width( nSumWidth );
        rInf.DrawViewOpt( *this, PortionType::Number );
        pThis->Width( nOldWidth );
    }

    if( m_aExpand.isEmpty() )
        return;

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

    SwFontSave aSave( rInf, m_pFont.get() );

    if( m_nFixWidth == Width() && ! HasFollow() )
        SwExpandPortion::Paint( rInf );
    else
    {
        // logical const: reset width
        SwNumberPortion *pThis = const_cast<SwNumberPortion*>(this);
        bPaintSpace = bPaintSpace && m_nFixWidth < nOldWidth;
        sal_uInt16 nSpaceOffs = m_nFixWidth;
        pThis->Width( m_nFixWidth );

        if( ( IsLeft() && ! rInf.GetTextFrame()->IsRightToLeft() ) ||
            ( ! IsLeft() && ! IsCenter() && rInf.GetTextFrame()->IsRightToLeft() ) )
            SwExpandPortion::Paint( rInf );
        else
        {
            SwTextPaintInfo aInf( rInf );
            if( nOffset < m_nMinDist )
                nOffset = 0;
            else
            {
                if( IsCenter() )
                {
                    /* #110778# a / 2 * 2 == a is not a tautology */
                    sal_uInt16 nTmpOffset = nOffset;
                    nOffset /= 2;
                    if( nOffset < m_nMinDist )
                        nOffset = nTmpOffset - m_nMinDist;
                }
                else
                    nOffset = nOffset - m_nMinDist;
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

SwBulletPortion::SwBulletPortion( const sal_UCS4 cBullet,
                                  std::u16string_view rBulletFollowedBy,
                                  std::unique_ptr<SwFont> pFont,
                                  const bool bLft,
                                  const bool bCntr,
                                  const sal_uInt16 nMinDst,
                                  const bool bLabelAlignmentPosAndSpaceModeActive )
    : SwNumberPortion( OUString(&cBullet, 1) + rBulletFollowedBy,
                       std::move(pFont), bLft, bCntr, nMinDst,
                       bLabelAlignmentPosAndSpaceModeActive )
{
    SetWhichPor( PortionType::Bullet );
}

#define GRFNUM_SECURE 10

SwGrfNumPortion::SwGrfNumPortion(
        const OUString& rGraphicFollowedBy,
        const SvxBrushItem* pGrfBrush, OUString const & referer,
        const SwFormatVertOrient* pGrfOrient, const Size& rGrfSize,
        const bool bLft, const bool bCntr, const sal_uInt16 nMinDst,
        const bool bLabelAlignmentPosAndSpaceModeActive ) :
    SwNumberPortion( rGraphicFollowedBy, nullptr, bLft, bCntr, nMinDst,
                     bLabelAlignmentPosAndSpaceModeActive ),
    m_pBrush( new SvxBrushItem(RES_BACKGROUND) ), m_nId( 0 )
{
    SetWhichPor( PortionType::GrfNum );
    SetAnimated( false );
    m_bReplace = false;
    if( pGrfBrush )
    {
        m_pBrush.reset(pGrfBrush->Clone());
        const Graphic* pGraph = pGrfBrush->GetGraphic(referer);
        if( pGraph )
            SetAnimated( pGraph->IsAnimated() );
        else
            m_bReplace = true;
    }
    if( pGrfOrient )
    {
        m_nYPos = pGrfOrient->GetPos();
        m_eOrient = pGrfOrient->GetVertOrient();
    }
    else
    {
        m_nYPos = 0;
        m_eOrient = text::VertOrientation::TOP;
    }
    Width( rGrfSize.Width() + 2 * GRFNUM_SECURE );
    m_nFixWidth = Width();
    m_nGrfHeight = rGrfSize.Height() + 2 * GRFNUM_SECURE;
    Height( sal_uInt16(m_nGrfHeight) );
    m_bNoPaint = false;
}

SwGrfNumPortion::~SwGrfNumPortion()
{
    if ( IsAnimated() )
    {
        Graphic* pGraph = const_cast<Graphic*>(m_pBrush->GetGraphic());
        if (pGraph)
            pGraph->StopAnimation( nullptr, m_nId );
    }
    m_pBrush.reset();
}

void SwGrfNumPortion::StopAnimation( const OutputDevice* pOut )
{
    if ( IsAnimated() )
    {
        Graphic* pGraph = const_cast<Graphic*>(m_pBrush->GetGraphic());
        if (pGraph)
            pGraph->StopAnimation( pOut, m_nId );
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
    Width( m_nFixWidth + nFollowedByWidth );
    const bool bFull = rInf.Width() < rInf.X() + Width();
    const bool bFly = rInf.GetFly() ||
        ( rInf.GetLast() && rInf.GetLast()->IsFlyPortion() );
    SetAscent( GetRelPos() > 0 ? GetRelPos() : 0 );
    if( GetAscent() > Height() )
        Height( GetAscent() );

    if( bFull )
    {
        Width( rInf.Width() - rInf.X() );
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
    tools::Long nDiff = mbLabelAlignmentPosAndSpaceModeActive
                 ? 0
                 : rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
    // The TextPortion should at least always start on the
    // left margin
    if( nDiff < 0 )
        nDiff = 0;
    else if ( nDiff > rInf.X() )
        nDiff -= rInf.X();
    if( nDiff < m_nFixWidth + m_nMinDist )
        nDiff = m_nFixWidth + m_nMinDist;

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
        Width( nDiff );
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
        SwLinePortion *pTmp = GetNextPortion();
        while ( pTmp && !pTmp->InTextGrp() )
            pTmp = pTmp->GetNextPortion();
        if ( !pTmp )
            return;
    }
    Point aPos( rInf.X() + GRFNUM_SECURE, rInf.Y() - GetRelPos() + GRFNUM_SECURE );
    tools::Long nTmpWidth = std::max( tools::Long(0), static_cast<tools::Long>(m_nFixWidth - 2 * GRFNUM_SECURE) );
    Size aSize( nTmpWidth, GetGrfHeight() - 2 * GRFNUM_SECURE );

    const bool bTmpLeft = mbLabelAlignmentPosAndSpaceModeActive ||
                              ( IsLeft() && ! rInf.GetTextFrame()->IsRightToLeft() ) ||
                              ( ! IsLeft() && ! IsCenter() && rInf.GetTextFrame()->IsRightToLeft() );

    if( m_nFixWidth < Width() && !bTmpLeft )
    {
        sal_uInt16 nOffset = Width() - m_nFixWidth;
        if( nOffset < m_nMinDist )
            nOffset = 0;
        else
        {
            if( IsCenter() )
            {
                nOffset /= 2;
                if( nOffset < m_nMinDist )
                    nOffset = Width() - m_nFixWidth - m_nMinDist;
            }
            else
                nOffset = nOffset - m_nMinDist;
        }
        aPos.AdjustX(nOffset );
    }

    if( m_bReplace )
    {
        const tools::Long nTmpH = GetNextPortion() ? GetNextPortion()->GetAscent() : 120;
        aSize = Size( nTmpH, nTmpH );
        aPos.setY( rInf.Y() - nTmpH );
    }
    SwRect aTmp( aPos, aSize );

    bool bDraw = true;

    if ( IsAnimated() )
    {
        bDraw = !rInf.GetOpt().IsGraphic();
        if( !m_nId )
        {
            SetId( reinterpret_cast<sal_IntPtr>( rInf.GetTextFrame() ) );
            rInf.GetTextFrame()->SetAnimation();
        }
        if( aTmp.Overlaps( rInf.GetPaintRect() ) && !bDraw )
        {
            rInf.NoteAnimation();
            const SwViewShell* pViewShell = rInf.GetVsh();

            // virtual device, not pdf export
            if( OUTDEV_VIRDEV == rInf.GetOut()->GetOutDevType() &&
                pViewShell && pViewShell->GetWin()  )
            {
                Graphic* pGraph = const_cast<Graphic*>(m_pBrush->GetGraphic());
                if (pGraph)
                    pGraph->StopAnimation(nullptr,m_nId);
                rInf.GetTextFrame()->getRootFrame()->GetCurrShell()->InvalidateWindows( aTmp );
            }

            else if ( pViewShell &&
                     !pViewShell->GetAccessibilityOptions()->IsStopAnimatedGraphics() &&
                     !pViewShell->IsPreview() &&
                      // #i9684# Stop animation during printing/pdf export.
                      pViewShell->GetWin() )
            {
                Graphic* pGraph = const_cast<Graphic*>(m_pBrush->GetGraphic());
                if (pGraph)
                {
                    const OutputDevice* pOut = rInf.GetOut();
                    assert(pOut);
                    pGraph->StartAnimation(
                        *const_cast<OutputDevice*>(pOut), aPos, aSize, m_nId);
                }
            }

            // pdf export, printing, preview, stop animations...
            else
                bDraw = true;
        }
        if( bDraw )
        {

            Graphic* pGraph = const_cast<Graphic*>(m_pBrush->GetGraphic());
            if (pGraph)
                pGraph->StopAnimation( nullptr, m_nId );
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
        const OutputDevice* pOut = rInf.GetOut();
        assert(pOut);
        DrawGraphic( m_pBrush.get(), *const_cast<OutputDevice*>(pOut),
            aTmp, aRepaint, m_bReplace ? GRFNUM_REPLACE : GRFNUM_YES );
    }
}

void SwGrfNumPortion::SetBase( tools::Long nLnAscent, tools::Long nLnDescent,
                               tools::Long nFlyAsc, tools::Long nFlyDesc )
{
    if ( GetOrient() == text::VertOrientation::NONE )
        return;

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

void SwTextFrame::StopAnimation( const OutputDevice* pOut )
{
    OSL_ENSURE( HasAnimation(), "SwTextFrame::StopAnimation: Which Animation?" );
    if( !HasPara() )
        return;

    SwLineLayout *pLine = GetPara();
    while( pLine )
    {
        SwLinePortion *pPor = pLine->GetNextPortion();
        while( pPor )
        {
            if( pPor->IsGrfNumPortion() )
                static_cast<SwGrfNumPortion*>(pPor)->StopAnimation( pOut );
            // The NumberPortion is always at the first char,
            // which means we can cancel as soon as we've reached a portion
            // with a length > 0
            pPor = pPor->GetLen() ? nullptr : pPor->GetNextPortion();
        }
        pLine = pLine->GetLen() ? nullptr : pLine->GetNext();
    }
}

/**
 * Initializes the script array and clears the width array
 */
SwCombinedPortion::SwCombinedPortion( const OUString &rText )
    : SwFieldPortion( rText )
    , m_aWidth{ static_cast<sal_uInt16>(0),
                static_cast<sal_uInt16>(0),
                static_cast<sal_uInt16>(0) }
    , m_nUpPos(0)
    , m_nLowPos(0)
    , m_nProportion(55)
{
    SetLen(TextFrameIndex(1));
    SetWhichPor( PortionType::Combined );
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
        m_aScrType[i] = nScr;
    }
}

void SwCombinedPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    OSL_ENSURE(GetLen() <= TextFrameIndex(1), "SwFieldPortion::Paint: rest-portion pollution?");
    if( !Width() )
        return;

    rInf.DrawBackBrush( *this );
    rInf.DrawViewOpt( *this, PortionType::Field );

    // do we have to repaint a post it portion?
    if( rInf.OnWin() && mpNextPortion && !mpNextPortion->Width() )
        mpNextPortion->PrePaint( rInf, this );

    const sal_Int32 nCount = m_aExpand.getLength();
    if( !nCount )
        return;
    OSL_ENSURE( nCount < 7, "Too much combined characters" );

    // the first character of the second row
    const sal_Int32 nTop = ( nCount + 1 ) / 2;

    SwFont aTmpFont( *rInf.GetFont() );
    aTmpFont.SetProportion( m_nProportion );  // a smaller font
    SwFontSave aFontSave( rInf, &aTmpFont );

    Point aOldPos = rInf.GetPos();
    Point aOutPos( aOldPos.X(), aOldPos.Y() - m_nUpPos );// Y of the first row
    for( sal_Int32 i = 0 ; i < nCount; ++i )
    {
        if( i == nTop ) // change the row
            aOutPos.setY( aOldPos.Y() + m_nLowPos );    // Y of the second row
        aOutPos.setX( aOldPos.X() + m_aPos[i] );        // X position
        const SwFontScript nAct = m_aScrType[i];        // script type
        aTmpFont.SetActual( nAct );

        // if there're more than 4 characters to display, we choose fonts
        // with 2/3 of the original font width.
        if( m_aWidth[ nAct ] )
        {
            Size aTmpSz = aTmpFont.GetSize( nAct );
            if( aTmpSz.Width() != m_aWidth[ nAct ] )
            {
                aTmpSz.setWidth( m_aWidth[ nAct ] );
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
    for( sal_Int32 i = 0; i < nCount && SW_SCRIPTS == m_aScrType[i]; ++i )
        m_aScrType[i] = rInf.GetFont()->GetActual();
    if( nCount > 4 )
    {
        // more than four? Ok, then we need the 2/3 font width
        for( sal_Int32 i = 0; i < m_aExpand.getLength(); ++i )
        {
            OSL_ENSURE( m_aScrType[i] < SW_SCRIPTS, "Combined: Script fault" );
            if( !m_aWidth[ m_aScrType[i] ] )
            {
                rInf.GetOut()->SetFont( rInf.GetFont()->GetFnt( m_aScrType[i] ) );
                m_aWidth[ m_aScrType[i] ] =
                        o3tl::narrowing<sal_uInt16>(2 * rInf.GetOut()->GetFontMetric().GetFontSize().Width() / 3);
            }
        }
    }

    const sal_Int32 nTop = ( nCount + 1 ) / 2; // the first character of the second line
    SwViewShell *pSh = rInf.GetTextFrame()->getRootFrame()->GetCurrShell();
    SwFont aTmpFont( *rInf.GetFont() );
    SwFontSave aFontSave( rInf, &aTmpFont );
    m_nProportion = 55;
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
        m_nProportion -= 5;
        aTmpFont.SetProportion( m_nProportion );
        memset( &m_aPos, 0, sizeof(m_aPos) );
        nMaxDescent = 0;
        nMaxAscent = 0;
        nMaxWidth = 0;
        m_nUpPos = m_nLowPos = 0;

        // Now we get the width of all characters.
        // The ascent and the width of the first line are stored in the
        // ascent member of the portion, the descent in nLowPos.
        // The ascent, descent and width of the second line are stored in the
        // local nMaxAscent, nMaxDescent and nMaxWidth variables.
        for( sal_Int32 i = 0; i < nCount; ++i )
        {
            SwFontScript nScrp = m_aScrType[i];
            aTmpFont.SetActual( nScrp );
            if( m_aWidth[ nScrp ] )
            {
                Size aFontSize( aTmpFont.GetSize( nScrp ) );
                aFontSize.setWidth( m_aWidth[ nScrp ] );
                aTmpFont.SetSize( aFontSize, nScrp );
            }

            SwDrawTextInfo aDrawInf(pSh, *rInf.GetOut(), m_aExpand, i, 1);
            Size aSize = aTmpFont.GetTextSize_( aDrawInf );
            const sal_uInt16 nAsc = aTmpFont.GetAscent( pSh, *rInf.GetOut() );
            m_aPos[ i ] = o3tl::narrowing<sal_uInt16>(aSize.Width());
            if( i == nTop ) // enter the second line
            {
                m_nLowPos = nMaxDescent;
                Height( nMaxDescent + nMaxAscent );
                Width( nMaxWidth );
                SetAscent( nMaxAscent );
                nMaxAscent = 0;
                nMaxDescent = 0;
                nMaxWidth = 0;
            }
            nMaxWidth = nMaxWidth + m_aPos[ i ];
            if( nAsc > nMaxAscent )
                nMaxAscent = nAsc;
            if( aSize.Height() - nAsc > nMaxDescent )
                nMaxDescent = aSize.Height() - nAsc;
        }
        // for one or two characters we double the width of the portion
        if( nCount < 3 )
        {
            nMaxWidth *= 2;
            Width( 2*Width() );
            if( nCount < 2 )
            {
                Height( nMaxAscent + nMaxDescent );
                m_nLowPos = nMaxDescent;
            }
        }
        Height( Height() + nMaxDescent + nMaxAscent );
        m_nUpPos = nMaxAscent;
        SetAscent( Height() - nMaxDescent - m_nLowPos );
    } while( m_nProportion > 40 && ( GetAscent() > nMainAscent ||
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

    // We calculate the x positions of the characters in both lines...
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
        case 3: m_aPos[1] = m_aPos[0] + nTopDiff;
            [[fallthrough]];
        case 2: m_aPos[nTop-1] = Width() - m_aPos[nTop-1];
    }
    m_aPos[0] = 0;
    switch( nCount )
    {
        case 5: m_aPos[4] = m_aPos[3] + nBotDiff;
            [[fallthrough]];
        case 3: m_aPos[nTop] = nBotDiff;          break;
        case 6: m_aPos[4] = m_aPos[3] + nBotDiff;
            [[fallthrough]];
        case 4: m_aPos[nTop] = 0;
            [[fallthrough]];
        case 2: m_aPos[nCount-1] = Width() - m_aPos[nCount-1];
    }

    // Does the combined portion fit the line?
    const bool bFull = rInf.Width() < rInf.X() + Width();
    if( bFull )
    {
        if( rInf.GetLineStart() == rInf.GetIdx() && (!rInf.GetLast()->InFieldGrp()
            || !static_cast<SwFieldPortion*>(rInf.GetLast())->IsFollow() ) )
            Width( rInf.Width() - rInf.X() );
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
    return new SwFieldFormDropDownPortion(m_pFieldMark, rExpand);
}

void SwFieldFormDropDownPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    SwFieldPortion::Paint( rInf );

    ::sw::mark::DropDownFieldmark* pDropDownField = dynamic_cast< ::sw::mark::DropDownFieldmark* >(m_pFieldMark);
    if(pDropDownField)
    {
        SwRect aPaintArea;
        rInf.CalcRect( *this, &aPaintArea );
        pDropDownField->SetPortionPaintArea(aPaintArea);
    }
}

SwFieldPortion *SwFieldFormDatePortion::Clone(const OUString &/*rExpand*/) const
{
    return new SwFieldFormDatePortion(m_pFieldMark, m_bStart);
}

void SwFieldFormDatePortion::Paint( const SwTextPaintInfo &rInf ) const
{
    SwFieldPortion::Paint( rInf );

    ::sw::mark::DateFieldmark* pDateField = dynamic_cast< ::sw::mark::DateFieldmark* >(m_pFieldMark);
    if(pDateField)
    {
        SwRect aPaintArea;
        rInf.CalcRect( *this, &aPaintArea );
        if(m_bStart)
            pDateField->SetPortionPaintAreaStart(aPaintArea);
        else
            pDateField->SetPortionPaintAreaEnd(aPaintArea);
    }
}

SwFieldPortion* SwJumpFieldPortion::Clone(const OUString& rExpand) const
{
    auto pRet = new SwJumpFieldPortion(*this);
    pRet->m_aExpand = rExpand;
    return pRet;
}

bool SwJumpFieldPortion::DescribePDFControl(const SwTextPaintInfo& rInf) const
{
    auto pPDFExtOutDevData
        = dynamic_cast<vcl::PDFExtOutDevData*>(rInf.GetOut()->GetExtOutDevData());
    if (!pPDFExtOutDevData)
        return false;

    if (!pPDFExtOutDevData->GetIsExportFormFields())
        return false;

    if (m_nFormat != SwJumpEditFormat::JE_FMT_TEXT)
        return false;

    vcl::PDFWriter::EditWidget aDescriptor;

    aDescriptor.Border = true;
    aDescriptor.BorderColor = COL_BLACK;

    SwRect aLocation;
    rInf.CalcRect(*this, &aLocation);
    aDescriptor.Location = aLocation.SVRect();

    // Map the text of the field to the descriptor's text.
    static sal_Unicode constexpr aForbidden[] = { CH_TXTATR_BREAKWORD, 0 };
    aDescriptor.Text = comphelper::string::removeAny(GetExp(), aForbidden);

    // Description for accessibility purposes.
    if (!m_sHelp.isEmpty())
        aDescriptor.Description = m_sHelp;

    pPDFExtOutDevData->WrapBeginStructureElement(vcl::PDFWriter::Form);
    pPDFExtOutDevData->CreateControl(aDescriptor);
    pPDFExtOutDevData->EndStructureElement();

    return true;
}

void SwJumpFieldPortion::Paint(const SwTextPaintInfo& rInf) const
{
    if (Width() && DescribePDFControl(rInf))
        return;

    if (rInf.GetOpt().IsShowPlaceHolderFields())
        SwFieldPortion::Paint(rInf);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
