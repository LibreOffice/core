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
#pragma once

#include <swtypes.hxx>
#include "itratr.hxx"
#include "inftxt.hxx"

struct SwPosition;
struct SwCursorMoveState;
class SwMarginPortion;
class SwFlyPortion;

class SwTextIter : public SwAttrIter
{
protected:
    SwLineInfo m_aLineInf;
    SwTextFrame  *m_pFrame;
    SwTextInfo *m_pInf;
    SwLineLayout *m_pCurr;
    SwLineLayout *m_pPrev;
    SwTwips m_nFrameStart;
    SwTwips m_nY;
    SwTwips m_nRegStart;          // The register's start position (Y)
    TextFrameIndex m_nStart;          // Start in the text string, end = pCurr->GetLen()
    sal_uInt16 m_nRegDiff;            // Register's line distance
    sal_Int32 m_nLineNr;             // Line number
    bool m_bPrev          : 1;
    bool m_bRegisterOn    : 1;    // Keep in register
    bool m_bOneBlock      : 1;    // Justified text: Dispose single words
    bool m_bLastBlock     : 1;    // Justified text: Also the last line
    bool m_bLastCenter    : 1;    // Justified text: Center last line

    SwLineLayout *GetPrev_();

    // Reset in the first line
    void Init();
    void CtorInitTextIter( SwTextFrame *pFrame, SwTextInfo *pInf );
    explicit SwTextIter(SwTextNode const * pTextNode)
        : SwAttrIter(pTextNode)
        , m_pFrame(nullptr)
        , m_pInf(nullptr)
        , m_pCurr(nullptr)
        , m_pPrev(nullptr)
        , m_nFrameStart(0)
        , m_nY(0)
        , m_nRegStart(0)
        , m_nStart(0)
        , m_nRegDiff(0)
        , m_nLineNr(0)
        , m_bPrev(false)
        , m_bRegisterOn(false)
        , m_bOneBlock(false)
        , m_bLastBlock(false)
        , m_bLastCenter(false)
    {
    }
public:
    SwTextIter(SwTextFrame *pTextFrame, SwTextInfo *pTextInf)
        : SwAttrIter(pTextFrame->GetTextNodeFirst())
        , m_bOneBlock(false)
        , m_bLastBlock(false)
        , m_bLastCenter(false)
    {
        CtorInitTextIter(pTextFrame, pTextInf);
    }
    const SwLineLayout *GetCurr() const { return m_pCurr; } // NEVER 0!
    const SwLineLayout *GetNext() const { return m_pCurr->GetNext(); }
           const SwLineLayout *GetPrev();
    TextFrameIndex GetLength() const { return m_pCurr->GetLen(); }
    sal_Int32 GetLineNr() const { return m_nLineNr; }
    TextFrameIndex GetStart() const { return m_nStart; }
    TextFrameIndex GetEnd() const { return GetStart() + GetLength(); }
    SwTwips Y() const { return m_nY; }

    SwTwips RegStart() const { return m_nRegStart; }
    sal_uInt16 RegDiff() const { return m_nRegDiff; }
    bool IsRegisterOn() const { return m_bRegisterOn; }

    SwTextInfo &GetInfo() { return *m_pInf; }
    const SwTextInfo &GetInfo() const { return *m_pInf; }

    void Top() { Init(); }
    void Bottom();
    const SwLineLayout *Next();
    const SwLineLayout *Prev();

    // Skips the FlyFrames dummy line
    const SwLineLayout *NextLine();
    const SwLineLayout *PrevLine();
    const SwLineLayout *GetNextLine() const;
    const SwLineLayout *GetPrevLine();

    void CharToLine(TextFrameIndex);
    void TwipsToLine(const SwTwips);

    // Truncates all after pCurr
    void TruncLines( bool bNoteFollow = false );

    SwTwips GetLineHeight() const { return m_pCurr->GetRealHeight(); }
    void CalcAscentAndHeight( SwTwips &rAscent, SwTwips &rHeight ) const;

    // Lots of trouble for querying pCurr == pPara
    bool IsFirstTextLine() const
    { return m_nStart == GetInfo().GetTextStart() &&
        !( m_pCurr->IsDummy() && GetNextLine() ); }

    // Replacement for the old IsFirstLine()
    bool IsParaLine() const
        { return m_pCurr == m_pInf->GetParaPortion(); }

    const SwLineInfo &GetLineInfo() const { return m_aLineInf; }
    SwTwips GetFirstPos() const { return m_nFrameStart; }
    inline bool SeekAndChg( SwTextSizeInfo &rInf );
    inline bool SeekAndChgBefore( SwTextSizeInfo &rInf );
    inline bool SeekStartAndChg( SwTextSizeInfo &rInf, const bool bPara=false );

    SwTextFrame *GetTextFrame() { return m_pFrame; }
    const SwTextFrame *GetTextFrame() const { return m_pFrame; }

    // Counts consecutive hyphens in order to be within the boundary given by MaxHyphens
    void CntHyphens( sal_uInt8 &nEndCnt, sal_uInt8 &nMidCnt) const;
};

class SwTextMargin : public SwTextIter
{
private:
          SwTwips mnLeft;
          SwTwips mnRight;
          SwTwips mnFirst;
          SwTwips mnDropLeft;
          SwTwips mnDropHeight;
          SwTwips mnDropDescent;
          sal_uInt16  mnDropLines;
          SvxAdjust  mnAdjust;
          // #i91133#
          SwTwips mnTabLeft;

protected:
    // For FormatQuoVadis
    void Right( const SwTwips nNew ) { mnRight = nNew; }

    SW_DLLPUBLIC void CtorInitTextMargin( SwTextFrame *pFrame, SwTextSizeInfo *pInf );
    explicit SwTextMargin(SwTextNode const * pTextNode)
        : SwTextIter(pTextNode)
        , mnLeft(0)
        , mnRight(0)
        , mnFirst(0)
        , mnDropLeft(0)
        , mnDropHeight(0)
        , mnDropDescent(0)
        , mnDropLines(0)
        , mnAdjust(SvxAdjust::Left)
        , mnTabLeft(0)
    {
    }
public:
    SwTextMargin(SwTextFrame *pTextFrame, SwTextSizeInfo *pTextSizeInf)
        : SwTextIter(pTextFrame->GetTextNodeFirst())
    {
        CtorInitTextMargin( pTextFrame, pTextSizeInf );
    }
    inline SwTwips GetLeftMargin() const;
    inline SwTwips Left() const;
    SwTwips Right() const { return mnRight; }
    SwTwips FirstLeft() const { return mnFirst; }
    SwTwips CurrWidth() const { return m_pCurr->PrtWidth(); }
           SwTwips GetLineStart() const;
    SwTwips GetLineEnd() const { return GetLineStart() + CurrWidth(); }
    Point GetTopLeft() const { return Point( GetLineStart(), Y() ); }
    bool IsOneBlock() const { return m_bOneBlock; }
    bool IsLastBlock() const { return m_bLastBlock; }
    bool IsLastCenter() const { return m_bLastCenter; }
    SvxAdjust GetAdjust() const { return mnAdjust; }
    SwTwips GetLineWidth() const { return Right() - GetLeftMargin() + 1; }
    SwTwips GetLeftMin() const { return std::min(mnFirst, mnLeft); }
    bool HasNegFirst() const { return mnFirst < mnLeft; }

    // #i91133#
    SwTwips GetTabLeft() const
    {
        return mnTabLeft;
    }
    // DropCaps
    sal_uInt16 GetDropLines() const { return mnDropLines; }
    void SetDropLines( const sal_uInt16 nNew ) { mnDropLines = nNew; }
    SwTwips GetDropLeft() const { return mnDropLeft; }
    SwTwips GetDropHeight() const { return mnDropHeight; }
    void SetDropHeight(const SwTwips nNew) { mnDropHeight = nNew; }
    SwTwips GetDropDescent() const { return mnDropDescent; }
    void SetDropDescent(const SwTwips nNew) { mnDropDescent = nNew; }
    void DropInit();

    // Returns the TextPos for start and end of the current line without whitespace
    // Implemented in frminf.cxx
    TextFrameIndex GetTextStart() const;
    TextFrameIndex GetTextEnd() const;

    SwTextSizeInfo &GetInfo()
        { return static_cast<SwTextSizeInfo&>(SwTextIter::GetInfo()); }
    const SwTextSizeInfo &GetInfo() const
        { return static_cast<const SwTextSizeInfo&>(SwTextIter::GetInfo()); }

};

class SwTextAdjuster : public SwTextMargin
{
    // Adjusts the portion, if we have adjustment and FlyFrames
    void CalcFlyAdjust( SwLineLayout *pCurr );

    // Calls SplitGlues and CalcBlockAdjust
    void FormatBlock( );

    // Creates the glue chain for short lines
    SwMarginPortion* CalcRightMargin( SwLineLayout *pCurr, SwTwips nReal = 0 );

    // Calculate the adjustment (FlyPortions)
    SwFlyPortion *CalcFlyPortion( const tools::Long nRealWidth,
                                  const SwRect &rCurrRect );

protected:
    explicit SwTextAdjuster(SwTextNode const * pTextNode) : SwTextMargin(pTextNode) { }
    // Creates the Glues for adjusted paragraphs
    void CalcNewBlock( SwLineLayout *pCurr, const SwLinePortion *pStopAt,
        SwTwips nReal = 0, bool bSkipKashida = false );
    SwTwips CalcKanaAdj( SwLineLayout *pCurr );

public:
    // Is overloaded by SwTextFormatter due to UpdatePos
    void CalcAdjLine( SwLineLayout *pCurr );

    // For adjusting afterwards
    void GetAdjusted() const
    {
        if( m_pCurr->IsFormatAdj() )
            const_cast<SwTextAdjuster*>(this)->CalcAdjLine( m_pCurr );
    }

    // Special treatment for DropCaps
    void CalcDropAdjust();
    void CalcDropRepaint();
};

class SwTextCursor : public SwTextAdjuster
{
    // A small helper-class to save SwTextCursor member, manipulate them
    // and to restore them
    friend class SwTextCursorSave;

    // Ambiguities
    static bool s_bRightMargin;
    void GetCharRect_(SwRect *, TextFrameIndex, SwCursorMoveState *);
protected:
    void CtorInitTextCursor( SwTextFrame *pFrame, SwTextSizeInfo *pInf );
    explicit SwTextCursor(SwTextNode const * pTextNode) : SwTextAdjuster(pTextNode) { }
    void AddExtraBlankWidth();
public:
    SwTextCursor( SwTextFrame *pTextFrame, SwTextSizeInfo *pTextSizeInf )
        : SwTextAdjuster(pTextFrame->GetTextNodeFirst())
    {
        CtorInitTextCursor(pTextFrame, pTextSizeInf);
    }
    void GetCharRect(SwRect *, TextFrameIndex, SwCursorMoveState* = nullptr,
        const tools::Long nMax = 0 );
    void GetEndCharRect(SwRect *, TextFrameIndex, SwCursorMoveState* = nullptr,
        const tools::Long nMax = 0 );
    TextFrameIndex GetModelPositionForViewPoint( SwPosition *pPos, const Point &rPoint,
                bool bChgNode, SwCursorMoveState* = nullptr ) const;
    // Respects ambiguities: For the implementation see below
    const SwLineLayout *CharCursorToLine(TextFrameIndex const nPos);

    // calculates baseline for portion rPor
    // bAutoToCentered indicates, if AUTOMATIC mode means CENTERED or BASELINE
    SwTwips AdjustBaseLine( const SwLineLayout& rLine, const SwLinePortion* pPor,
                           SwTwips nPorHeight = 0, SwTwips nAscent = 0,
                           const bool bAutoToCentered = false ) const;

    static void SetRightMargin( const bool bNew ){ s_bRightMargin = bNew; }
    static bool IsRightMargin() { return s_bRightMargin; }
};

// Change current output device to printer, this has to be done before
// formatting.
class SwHookOut
{
    SwTextSizeInfo* pInf;
    VclPtr<OutputDevice> pOut;
    bool bOnWin;
public:
    explicit SwHookOut( SwTextSizeInfo& rInfo );
    ~SwHookOut();
};

inline bool SwTextIter::SeekAndChg( SwTextSizeInfo &rInf )
{
    return SeekAndChgAttrIter( rInf.GetIdx(), rInf.GetOut() );
}

inline bool SwTextIter::SeekAndChgBefore( SwTextSizeInfo &rInf )
{
    if ( rInf.GetIdx() )
        return SeekAndChgAttrIter(rInf.GetIdx() - TextFrameIndex(1), rInf.GetOut());
    else
        return SeekAndChgAttrIter( rInf.GetIdx(), rInf.GetOut() );
}

inline bool SwTextIter::SeekStartAndChg( SwTextSizeInfo &rInf, const bool bPara )
{
    return SeekStartAndChgAttrIter( rInf.GetOut(), bPara );
}

inline SwTwips SwTextMargin::GetLeftMargin() const
{
    return IsFirstTextLine() ? mnFirst : Left();
}

inline SwTwips SwTextMargin::Left() const
{
    return (mnDropLines >= m_nLineNr && 1 != m_nLineNr) ? mnFirst + mnDropLeft : mnLeft;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
