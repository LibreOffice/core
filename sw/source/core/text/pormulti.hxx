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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORMULTI_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORMULTI_HXX

#include <memory>
#include "porlay.hxx"
#include <com/sun/star/text/RubyAdjust.hpp>

class IDocumentSettingAccess;
class SwTextFormatInfo;
class SwTextCursor;
class SwTextPaintInfo;
class SwTextAttr;
class SfxPoolItem;
class SwFont;

// SwMultiCreator is a small structure to create a multiportion.
// It contains the kind of multiportion and a textattribute
// or a poolitem.
// The GetMultiCreator-function fills this structure and
// the Ctor of the SwMultiPortion uses it.
enum class SwMultiCreatorId
{
    Double, Ruby, Rotate, Bidi
};

enum class RubyPosition : sal_uInt16
{
    ABOVE = 0,
    BELOW = 1,
    RIGHT = 2
};

struct SwMultiCreator
{
    TextFrameIndex nStartOfAttr;
    const SwTextAttr* pAttr;
    const SfxPoolItem* pItem;
    SwMultiCreatorId nId;
    sal_uInt8 nLevel;
};

// A two-line-portion (SwMultiPortion) could have surrounding brackets,
// in this case the structure SwBracket will be used.
struct SwBracket
{
    TextFrameIndex nStart;      // Start of text attribute determines the font
    sal_uInt16 nAscent;         // Ascent of the brackets
    sal_uInt16 nHeight;         // Height of them
    sal_uInt16 nPreWidth;       // Width of the opening bracket
    sal_uInt16 nPostWidth;      // Width of the closing bracket
    sal_Unicode cPre;           // Initial character, e.g. '('
    sal_Unicode cPost;          // Final character, e.g. ')'
    SwFontScript nPreScript;    // Script of the initial character
    SwFontScript nPostScript;   // Script of the final character
};

// The SwMultiPortion is line portion inside a line portion,
// it's a group of portions,
// e.g. a double line portion in a line
// or phonetics (ruby)
// or combined characters
// or a rotated portion.
class SwMultiPortion : public SwLinePortion
{
    SwLineLayout m_aRoot;     // One or more lines
    bool m_bTab1      :1;     // First line tabulator
    bool m_bTab2      :1;     // Second line includes tabulator
    bool m_bDouble    :1;     // Double line
    bool m_bRuby      :1;     // Phonetics
    bool m_bBidi      :1;
    bool m_bFormatted :1;     // Already formatted
    bool m_bFollowField :1;     // Field follow inside
    bool m_bFlyInContent:1;     // Fly as character inside
    RubyPosition m_eRubyPosition;     // Phonetic position
    sal_uInt8 m_nDirection:2; // Direction (0/90/180/270 degrees)
protected:
    explicit SwMultiPortion(TextFrameIndex const nEnd)
        : m_bTab1(false)
        , m_bTab2(false)
        , m_bDouble(false)
        , m_bRuby(false)
        , m_bBidi(false)
        , m_bFormatted(false)
        , m_bFollowField(false)
        , m_bFlyInContent(false)
        , m_eRubyPosition( RubyPosition::ABOVE )
        , m_nDirection(0)
    {
        SetWhichPor(PortionType::Multi);
        SetLen(nEnd);
    }
    void SetDouble() { m_bDouble = true; }
    void SetRuby() { m_bRuby = true; }
    void SetBidi() { m_bBidi = true; }
    void SetRubyPosition( RubyPosition eNew ) { m_eRubyPosition = eNew; }
    void SetTab1( bool bNew ) { m_bTab1 = bNew; }
    void SetTab2( bool bNew ) { m_bTab2 = bNew; }
    void SetDirection( sal_uInt8 nNew ) { m_nDirection = nNew; }
    bool GetTab1() const { return m_bTab1; }
    bool GetTab2() const { return m_bTab2; }
public:
    virtual ~SwMultiPortion() override;
    const SwLineLayout& GetRoot() const { return m_aRoot; }
    SwLineLayout& GetRoot() { return m_aRoot; }

    bool HasTabulator() const { return m_bTab1 || m_bTab2; }
    bool IsFormatted() const { return m_bFormatted; }
    void SetFormatted() { m_bFormatted = true; }
    bool IsFollowField() const { return m_bFollowField; }
    void SetFollowField() { m_bFollowField = true; }
    bool HasFlyInContent() const { return m_bFlyInContent; }
    void SetFlyInContent( bool bNew ) { m_bFlyInContent = bNew; }
    bool IsDouble() const { return m_bDouble; }
    bool IsRuby() const { return m_bRuby; }
    bool IsBidi() const { return m_bBidi; }
    bool OnTop() const { return m_eRubyPosition == RubyPosition::ABOVE; }
    bool OnRight() const { return m_eRubyPosition == RubyPosition::RIGHT; }
    RubyPosition GetRubyPosition() const { return m_eRubyPosition; }
    void ActualizeTabulator();

    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual tools::Long CalcSpacing( tools::Long nSpaceAdd, const SwTextSizeInfo &rInf ) const override;
    virtual bool ChgSpaceAdd( SwLineLayout* pCurr, tools::Long nSpaceAdd ) const;

    // Summarize the internal lines to calculate the (external) size
    void CalcSize( SwTextFormatter& rLine, SwTextFormatInfo &rInf );

    inline bool HasBrackets() const;
    bool HasRotation() const { return 0 != (1 & m_nDirection); }
    bool IsRevers() const { return 0 != (2 & m_nDirection); }
    sal_uInt8 GetDirection() const { return m_nDirection; }

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

class SwDoubleLinePortion : public SwMultiPortion
{
    std::unique_ptr<SwBracket> m_pBracket;    // Surrounding brackets
    SwTwips m_nLineDiff;      // Difference of the width of the both lines
    TextFrameIndex m_nBlank1; ///< Number of blanks in the first line
    TextFrameIndex m_nBlank2; ///< Number of blanks in the second line
public:
    SwDoubleLinePortion(SwDoubleLinePortion& rDouble, TextFrameIndex nEnd);
    SwDoubleLinePortion(const SwMultiCreator& rCreate, TextFrameIndex nEnd);
    virtual ~SwDoubleLinePortion() override;

    SwBracket* GetBrackets() const { return m_pBracket.get(); }
    void SetBrackets( const SwDoubleLinePortion& rDouble );
    void PaintBracket( SwTextPaintInfo& rInf, tools::Long nSpaceAdd, bool bOpen ) const;
    void FormatBrackets( SwTextFormatInfo &rInf, SwTwips& nMaxWidth );
    sal_uInt16 PreWidth() const { return m_pBracket->nPreWidth; };
    sal_uInt16 PostWidth() const { return m_pBracket->nPostWidth; }
    void ClearBrackets()
        { m_pBracket->nPreWidth = m_pBracket->nPostWidth=0; Width( 0 ); }
    sal_uInt16 BracketWidth(){ return PreWidth() + PostWidth(); }

    void CalcBlanks( SwTextFormatInfo &rInf );
    static void ResetSpaceAdd( SwLineLayout* pCurr );
    SwTwips GetLineDiff() const { return m_nLineDiff; }
    TextFrameIndex GetSpaceCnt() const
        { return ( m_nLineDiff < 0 ) ? m_nBlank2 : m_nBlank1; }
    TextFrameIndex GetSmallerSpaceCnt() const
        { return ( m_nLineDiff < 0 ) ? m_nBlank1 : m_nBlank2; }

    virtual tools::Long CalcSpacing( tools::Long nSpaceAdd, const SwTextSizeInfo &rInf ) const override;
    virtual bool ChgSpaceAdd( SwLineLayout* pCurr, tools::Long nSpaceAdd ) const override;
};

class SwRubyPortion : public SwMultiPortion
{
    TextFrameIndex m_nRubyOffset;
    css::text::RubyAdjust m_nAdjustment;
    void Adjust_( SwTextFormatInfo &rInf);
public:
    SwRubyPortion(const SwRubyPortion& rRuby, TextFrameIndex nEnd);

    SwRubyPortion( const SwMultiCreator& rCreate, const SwFont& rFnt,
                   const IDocumentSettingAccess& rIDocumentSettingAccess,
                   TextFrameIndex nEnd, TextFrameIndex nOffs,
                   const SwTextSizeInfo &rInf );

    void CalcRubyOffset();
    void Adjust( SwTextFormatInfo &rInf )
        { if(m_nAdjustment != css::text::RubyAdjust_LEFT && GetRoot().GetNext()) Adjust_(rInf); }
    css::text::RubyAdjust GetAdjustment() const { return m_nAdjustment; }
    TextFrameIndex GetRubyOffset() const { return m_nRubyOffset; }
};

class SwRotatedPortion : public SwMultiPortion
{
public:
    SwRotatedPortion(TextFrameIndex const nEnd, sal_uInt8 nDir)
        : SwMultiPortion( nEnd ) { SetDirection( nDir ); }
    SwRotatedPortion( const SwMultiCreator& rCreate, TextFrameIndex nEnd,
                      bool bRTL );
};

class SwBidiPortion : public SwMultiPortion
{
    sal_uInt8 m_nLevel;

public:
    SwBidiPortion(TextFrameIndex nEnd, sal_uInt8 nLv);

    sal_uInt8 GetLevel() const { return m_nLevel; }
    // Get number of blanks for justified alignment
    TextFrameIndex GetSpaceCnt(const SwTextSizeInfo &rInf) const;
    // Calculates extra spacing based on number of blanks
    virtual tools::Long CalcSpacing( tools::Long nSpaceAdd, const SwTextSizeInfo &rInf ) const override;
    // Manipulate the spacing array at pCurr
    virtual bool ChgSpaceAdd( SwLineLayout* pCurr, tools::Long nSpaceAdd ) const override;
};

// For cursor travelling in multiportions

class SwTextCursorSave
{
    SwTextCursor* pTextCursor;
    SwLineLayout* pCurr;
    TextFrameIndex nStart;
    sal_uInt16 nWidth;
    sal_uInt8 nOldProp;
    bool bSpaceChg;
public:
    SwTextCursorSave( SwTextCursor* pTextCursor, SwMultiPortion* pMulti,
        SwTwips nY, sal_uInt16& nX, TextFrameIndex nCurrStart, tools::Long nSpaceAdd);
    ~SwTextCursorSave();
};

inline bool SwMultiPortion::HasBrackets() const
{
    return IsDouble() && nullptr != static_cast<const SwDoubleLinePortion*>(this)->GetBrackets();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
