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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DRAWFONT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DRAWFONT_HXX

#include <osl/diagnose.h>
#include <vcl/vclptr.hxx>
#include <vcl/outdev.hxx>
#include <swtypes.hxx>
#include "TextFrameIndex.hxx"
#include <swdllapi.h>
#include "swporlayoutcontext.hxx"

class SwTextFrame;
class SwViewShell;
class SwScriptInfo;
namespace sw { class WrongListIterator; }
class SwFont;
namespace vcl {
    class Font;
    namespace text {
        class TextLayoutCache;
    }
    typedef OutputDevice RenderContext;
}
class SwUnderlineFont;

// encapsulates information for drawing text
class SW_DLLPUBLIC SwDrawTextInfo
{
    const SwTextFrame* m_pFrame = nullptr;
    VclPtr<OutputDevice> m_pOut;
    SwViewShell const* m_pSh;
    const SwScriptInfo* m_pScriptInfo;
    Point m_aPos;
    vcl::text::TextLayoutCache const* m_pCachedVclData;
    OUString m_aText;
    sw::WrongListIterator* m_pWrong = nullptr;
    sw::WrongListIterator* m_pGrammarCheck = nullptr;
    sw::WrongListIterator* m_pSmartTags = nullptr;
    Size m_aSize;
    SwFont* m_pFnt = nullptr;
    SwUnderlineFont* m_pUnderFnt = nullptr;
    TextFrameIndex* m_pHyphPos = nullptr;
    tools::Long m_nKanaDiff = 0;
    TextFrameIndex m_nIdx;
    TextFrameIndex m_nLen;
    TextFrameIndex m_nMeasureLen = TextFrameIndex{ COMPLETE_STRING };
    std::optional<SwLinePortionLayoutContext> m_nLayoutContext;
    /// this is not a string index
    sal_Int32 m_nOfst = 0;
    sal_uInt16 m_nWidth;
    sal_uInt16 m_nAscent = 0;
    sal_uInt16 m_nCompress = 0;
    tools::Long m_nCharacterSpacing = 0;
    tools::Long m_nSpace = 0;
    tools::Long m_nKern = 0;
    TextFrameIndex m_nNumberOfBlanks = TextFrameIndex{ 0 };
    sal_uInt8 m_nCursorBidiLevel = 0;
    bool m_bBullet : 1;
    bool m_bUpper : 1 = false; // for small caps: upper case flag
    bool m_bDrawSpace : 1 = false; // for small caps: underline/ line through
    bool m_bGreyWave : 1 = false; // grey wave line for extended text input
    // For underlining we need to know, if a section is right in front of a
    // whole block or a fix margin section.
    bool m_bSpaceStop : 1 = false;
    bool m_bSnapToGrid : 1 = false; // Does paragraph snap to grid?
    // Paint text as if text has LTR direction, used for line numbering
    bool m_bIgnoreFrameRTL : 1 = false;
    // GetModelPositionForViewPoint should not return the next position if screen position is
    // inside second half of bound rect, used for Accessibility
    bool m_bPosMatchesBounds : 1 = false;

#ifdef DBG_UTIL
    // These flags should control that the appropriate Set-function has been
    // called before calling the Get-function of a member
    bool m_bPos : 1 = false;
    bool m_bWrong : 1 = false;
    bool m_bGrammarCheck : 1 = false;
    bool m_bSize : 1 = false;
    bool m_bFnt : 1 = false;
    bool m_bHyph : 1 = false;
    bool m_bKana : 1 = false;
    bool m_bOfst : 1 = false;
    bool m_bAscent : 1 = false;
    bool m_bCharacterSpacing : 1 = false;
    bool m_bSpace : 1 = false;
    bool m_bNumberOfBlanks : 1 = false;
    bool m_bUppr : 1 = false;
    bool m_bDrawSp : 1 = false;
#endif

public:

    /// constructor for simple strings
    SwDrawTextInfo(SwViewShell const* pSh, OutputDevice& rOut, const OUString& rText,
                   sal_Int32 const nIdx, sal_Int32 const nLen, sal_uInt16 nWidth = 0,
                   bool bBullet = false)
        : SwDrawTextInfo(pSh, rOut, nullptr, rText, TextFrameIndex(nIdx), TextFrameIndex(nLen),
                         /*layout context*/ std::nullopt, nWidth, bBullet)
    {}
    /// constructor for text frame contents
    SwDrawTextInfo(SwViewShell const* pSh, OutputDevice& rOut, const SwScriptInfo* pSI,
                   const OUString& rText, TextFrameIndex const nIdx, TextFrameIndex const nLen,
                   std::optional<SwLinePortionLayoutContext> nLayoutContext, sal_uInt16 nWidth = 0,
                   bool bBullet = false,
                   vcl::text::TextLayoutCache const* const pCachedVclData = nullptr)
        : m_pOut(&rOut)
        , m_pSh(pSh)
        , m_pScriptInfo(pSI)
        , m_pCachedVclData(pCachedVclData)
        , m_aText(rText)
        , m_nIdx(nIdx)
        , m_nLen(nLen)
        , m_nLayoutContext(nLayoutContext)
        , m_nWidth(nWidth)
        , m_bBullet(bBullet)
    {
    }

    const SwTextFrame* GetFrame() const
    {
        return m_pFrame;
    }

    void SetFrame( const SwTextFrame* pNewFrame )
    {
        m_pFrame = pNewFrame;
    }

    SwViewShell const *GetShell() const
    {
        return m_pSh;
    }

    vcl::RenderContext& GetOut() const
    {
        return *m_pOut;
    }

    vcl::RenderContext *GetpOut() const
    {
        return m_pOut;
    }

    const SwScriptInfo* GetScriptInfo() const
    {
        return m_pScriptInfo;
    }

    const Point &GetPos() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bPos, "DrawTextInfo: Undefined Position" );
#endif
        return m_aPos;
    }

    TextFrameIndex *GetHyphPos() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bHyph, "DrawTextInfo: Undefined Hyph Position" );
#endif
        return m_pHyphPos;
    }

    vcl::text::TextLayoutCache const* GetVclCache() const
    {
        return m_pCachedVclData;
    }

    const OUString &GetText() const
    {
        return m_aText;
    }

    sw::WrongListIterator* GetWrong() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bWrong, "DrawTextInfo: Undefined WrongList" );
#endif
        return m_pWrong;
    }

    sw::WrongListIterator* GetGrammarCheck() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bGrammarCheck, "DrawTextInfo: Undefined GrammarCheck List" );
#endif
        return m_pGrammarCheck;
    }

    sw::WrongListIterator* GetSmartTags() const
    {
        return m_pSmartTags;
    }

    const Size &GetSize() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bSize, "DrawTextInfo: Undefined Size" );
#endif
        return m_aSize;
    }

    SwFont* GetFont() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bFnt, "DrawTextInfo: Undefined Font" );
#endif
        return m_pFnt;
    }

    SwUnderlineFont* GetUnderFnt() const
    {
        return m_pUnderFnt;
    }

    TextFrameIndex GetIdx() const
    {
        return m_nIdx;
    }

    TextFrameIndex GetLen() const
    {
        return m_nLen;
    }

    TextFrameIndex GetMeasureLen() const
    {
        return m_nMeasureLen;
    }

    std::optional<SwLinePortionLayoutContext> GetLayoutContext() const { return m_nLayoutContext; }

    sal_Int32 GetOffset() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bOfst, "DrawTextInfo: Undefined Offset" );
#endif
        return m_nOfst;
    }

    TextFrameIndex GetEnd() const
    {
        return m_nIdx + m_nLen;
    }

    tools::Long GetKanaDiff() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bKana, "DrawTextInfo: Undefined kana difference" );
#endif
        return m_nKanaDiff;
    }

    sal_uInt16 GetWidth() const
    {
        return m_nWidth;
    }

    sal_uInt16 GetAscent() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bAscent, "DrawTextInfo: Undefined Ascent" );
#endif
        return m_nAscent;
    }

    sal_uInt16 GetKanaComp() const
    {
        return m_nCompress;
    }

    tools::Long GetCharacterSpacing() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bCharacterSpacing, "DrawTextInfo: Undefined CharacterSpacing" );
#endif
        return m_nCharacterSpacing;
    }

    tools::Long GetKern() const
    {
        return m_nKern;
    }

    tools::Long GetSpace() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bSpace, "DrawTextInfo: Undefined Spacing" );
#endif
        return m_nSpace;
    }

    TextFrameIndex GetNumberOfBlanks() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bNumberOfBlanks, "DrawTextInfo::Undefined NumberOfBlanks" );
#endif
        return m_nNumberOfBlanks;
    }

    sal_uInt8 GetCursorBidiLevel() const
    {
        return m_nCursorBidiLevel;
    }

    bool GetBullet() const
    {
        return m_bBullet;
    }

    bool GetUpper() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bUppr, "DrawTextInfo: Undefined Upperflag" );
#endif
        return m_bUpper;
    }

    bool GetDrawSpace() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bDrawSp, "DrawTextInfo: Undefined DrawSpaceflag" );
#endif
        return m_bDrawSpace;
    }

    bool GetGreyWave() const
    {
        return m_bGreyWave;
    }

    bool IsSpaceStop() const
    {
        return m_bSpaceStop;
    }

    bool SnapToGrid() const
    {
        return m_bSnapToGrid;
    }

    bool IsIgnoreFrameRTL() const
    {
        return m_bIgnoreFrameRTL;
    }

    bool IsPosMatchesBounds() const
    {
        return m_bPosMatchesBounds;
    }

    void SetOut( OutputDevice &rNew )
    {
        m_pOut = &rNew;
    }

    void SetPos( const Point &rNew )
    {
        m_aPos = rNew;
#ifdef DBG_UTIL
        m_bPos = true;
#endif
    }

    void SetHyphPos(TextFrameIndex *const pNew)
    {
        m_pHyphPos = pNew;
#ifdef DBG_UTIL
        m_bHyph = true;
#endif
    }

    void SetText( const OUString &rNew )
    {
        assert( (m_nLen == TextFrameIndex(COMPLETE_STRING)) ? (m_nIdx.get() < rNew.getLength()) : (m_nIdx + m_nLen).get() <= rNew.getLength() );
        m_aText = rNew;
        m_pCachedVclData = nullptr; // would any case benefit from save/restore?
    }

    // These methods are here so we can set all the related fields together to preserve the invariants that we assert
    void SetTextIdxLen( const OUString &rNewStr, TextFrameIndex const nNewIdx, TextFrameIndex const nNewLen )
    {
        assert( (nNewLen == TextFrameIndex(COMPLETE_STRING)) ? (nNewIdx.get() < rNewStr.getLength()) : (nNewIdx + nNewLen).get() <= rNewStr.getLength() );
        m_aText = rNewStr;
        m_nIdx = nNewIdx;
        m_nLen = nNewLen;
        m_pCachedVclData = nullptr; // would any case benefit from save/restore?
    }

    // These methods are here so we can set all the related fields together to preserve the invariants that we assert
    void SetIdxLen( TextFrameIndex const nNewIdx, TextFrameIndex const nNewLen )
    {
        assert( (nNewLen == TextFrameIndex(COMPLETE_STRING)) ? (nNewIdx.get() < m_aText.getLength()) : (nNewIdx + nNewLen).get() <= m_aText.getLength() );
        m_nIdx = nNewIdx;
        m_nLen = nNewLen;
    }

    void SetLayoutContext(std::optional<SwLinePortionLayoutContext> nNew)
    {
        m_nLayoutContext = nNew;
    }

    void SetWrong(sw::WrongListIterator *const pNew)
    {
        m_pWrong = pNew;
#ifdef DBG_UTIL
        m_bWrong = true;
#endif
    }

    void SetGrammarCheck(sw::WrongListIterator *const pNew)
    {
        m_pGrammarCheck = pNew;
#ifdef DBG_UTIL
        m_bGrammarCheck = true;
#endif
    }

    void SetSmartTags(sw::WrongListIterator *const pNew)
    {
        m_pSmartTags = pNew;
    }

    void SetSize( const Size &rNew )
    {
        m_aSize = rNew;
#ifdef DBG_UTIL
        m_bSize = true;
#endif
    }

    void SetFont( SwFont* pNew )
    {
        m_pFnt = pNew;
#ifdef DBG_UTIL
        m_bFnt = true;
#endif
    }

    void SetIdx(TextFrameIndex const nNew)
    {
        assert( (m_nLen == TextFrameIndex(COMPLETE_STRING)) ? (nNew.get() < m_aText.getLength()) : (nNew + m_nLen).get() <= m_aText.getLength() );
        m_nIdx = nNew;
    }

    void SetLen(TextFrameIndex const nNew)
    {
        m_nLen = nNew;
    }

    void SetMeasureLen(TextFrameIndex const nNew)
    {
        assert( nNew == TextFrameIndex(COMPLETE_STRING) || nNew <= m_nLen );
        m_nMeasureLen = nNew;
    }

    void SetOffset( sal_Int32 nNew )
    {
        m_nOfst = nNew;
#ifdef DBG_UTIL
        m_bOfst = true;
#endif
    }

    void SetKanaDiff( tools::Long nNew )
    {
        m_nKanaDiff = nNew;
#ifdef DBG_UTIL
        m_bKana = true;
#endif
    }

    void SetWidth( sal_uInt16 nNew )
    {
        m_nWidth = nNew;
    }

    void SetAscent( sal_uInt16 nNew )
    {
        m_nAscent = nNew;
#ifdef DBG_UTIL
        m_bAscent = true;
#endif
    }

    void SetKern( tools::Long nNew )
    {
        m_nKern = nNew;
    }

    void SetSpace( tools::Long nNew )
    {
        if( nNew < 0 )
        {
            m_nCharacterSpacing = -nNew;
            m_nSpace = 0;
        }
        else
        {
            // negative space (shrinking) stored over LONG_MAX/2
            if ( nNew < LONG_MAX/2 )
                m_nSpace = nNew;
            else
                m_nSpace = LONG_MAX/2 - nNew;
            m_nCharacterSpacing = 0;
        }
#ifdef DBG_UTIL
        m_bSpace = true;
        m_bCharacterSpacing = true;
#endif
    }

    void SetNumberOfBlanks( TextFrameIndex const nNew )
    {
#ifdef DBG_UTIL
        m_bNumberOfBlanks = true;
#endif
        m_nNumberOfBlanks = nNew;
    }

    void SetCursorBidiLevel( sal_uInt8 nNew )
    {
        m_nCursorBidiLevel = nNew;
    }

    void SetKanaComp( short nNew )
    {
        m_nCompress = nNew;
    }

    void SetBullet( bool bNew )
    {
        m_bBullet = bNew;
    }

    void SetUnderFnt( SwUnderlineFont* pULFnt )
    {
        m_pUnderFnt = pULFnt;
    }

    void SetUpper( bool bNew )
    {
        m_bUpper = bNew;
#ifdef DBG_UTIL
        m_bUppr = true;
#endif
    }

    void SetDrawSpace( bool bNew )
    {
        m_bDrawSpace = bNew;
#ifdef DBG_UTIL
        m_bDrawSp = true;
#endif
    }

    void SetGreyWave( bool bNew )
    {
        m_bGreyWave = bNew;
    }

    void SetSpaceStop( bool bNew )
    {
        m_bSpaceStop = bNew;
    }

    void SetSnapToGrid( bool bNew )
    {
        m_bSnapToGrid = bNew;
    }

    void SetIgnoreFrameRTL( bool bNew )
    {
        m_bIgnoreFrameRTL = bNew;
    }

    void SetPosMatchesBounds( bool bNew )
    {
        m_bPosMatchesBounds = bNew;
    }

    void Shift( Degree10 nDir );

    // sets a new color at the output device if necessary if a font is passed
    // as argument, the change if made to the font otherwise the font at the
    // output device is changed returns if the font has been changed
    bool ApplyAutoColor( vcl::Font* pFnt = nullptr );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
