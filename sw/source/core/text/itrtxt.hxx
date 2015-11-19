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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_ITRTXT_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_ITRTXT_HXX
#include "swtypes.hxx"
#include "itratr.hxx"
#include "inftxt.hxx"

class SwTextFrame;
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
    sal_Int32 m_nStart;          // Start in the text string, end = pCurr->GetLen()
    sal_uInt16 m_nRegDiff;            // Register's line distance
    sal_uInt16 m_nLineNr;             // Line number
    bool m_bPrev          : 1;
    bool m_bRegisterOn    : 1;    // Keep in register
    bool m_bOneBlock      : 1;    // Justified text: Dispose single words
    bool m_bLastBlock     : 1;    // Justified text: Also the last line
    bool m_bLastCenter    : 1;    // Justified text: Center last line

    SwLineLayout *_GetPrev();

    // Reset in the first line
    void Init();
    void CtorInitTextIter( SwTextFrame *pFrame, SwTextInfo *pInf );
    explicit SwTextIter(SwTextNode* pTextNode)
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
        : SwAttrIter(pTextFrame->GetTextNode())
        , m_bOneBlock(false)
        , m_bLastBlock(false)
        , m_bLastCenter(false)
    {
        CtorInitTextIter(pTextFrame, pTextInf);
    }
    inline const SwLineLayout *GetCurr() const { return m_pCurr; } // NEVER 0!
    inline const SwLineLayout *GetNext() const { return m_pCurr->GetNext(); }
           const SwLineLayout *GetPrev();
    inline sal_Int32 GetLength() const { return m_pCurr->GetLen(); }
    inline sal_uInt16 GetLineNr() const { return m_nLineNr; }
    inline sal_Int32 GetStart() const { return m_nStart; }
    inline sal_Int32 GetEnd() const { return GetStart() + GetLength(); }
    inline SwTwips Y() const { return m_nY; }

    inline SwTwips RegStart() const { return m_nRegStart; }
    inline sal_uInt16 RegDiff() const { return m_nRegDiff; }
    inline bool IsRegisterOn() const { return m_bRegisterOn; }

    inline SwTextInfo &GetInfo() { return *m_pInf; }
    inline const SwTextInfo &GetInfo() const { return *m_pInf; }

    inline void Top() { Init(); }
    void Bottom();
    const SwLineLayout *Next();
    const SwLineLayout *Prev();

    // Skips the FlyFrames dummy line
    const SwLineLayout *NextLine();
    const SwLineLayout *PrevLine();
    const SwLineLayout *GetNextLine() const;
    const SwLineLayout *GetPrevLine();

    void CharToLine( const sal_Int32 );
    const SwLineLayout *TwipsToLine(const SwTwips);

    // Truncates all after pCurr
    void TruncLines( bool bNoteFollow = false );

    inline sal_uInt16 GetLineHeight() const { return m_pCurr->GetRealHeight(); }
    void CalcAscentAndHeight( sal_uInt16 &rAscent, sal_uInt16 &rHeight ) const;

    // Lots of trouble for querying pCurr == pPara
    inline bool IsFirstTextLine() const
    { return m_nStart == GetInfo().GetTextStart() &&
        !( m_pCurr->IsDummy() && GetNextLine() ); }

    // Replacement for the old IsFirstLine()
    inline bool IsParaLine() const
        { return m_pCurr == m_pInf->GetParaPortion(); }

    const SwLineInfo &GetLineInfo() const { return m_aLineInf; }
    inline SwTwips GetFirstPos() const { return m_nFrameStart; }
    inline bool SeekAndChg( SwTextSizeInfo &rInf );
    inline bool SeekAndChgBefore( SwTextSizeInfo &rInf );
    inline bool SeekStartAndChg( SwTextSizeInfo &rInf, const bool bPara=false );

    inline SwTextFrame *GetTextFrame() { return m_pFrame; }
    inline const SwTextFrame *GetTextFrame() const { return m_pFrame; }

    // Counts consecutive hyphens in order to be within the boundary given by MaxHyphens
    void CntHyphens( sal_uInt8 &nEndCnt, sal_uInt8 &nMidCnt) const;
};

class SwTextMargin : public SwTextIter
{
private:
          SwTwips nLeft;
          SwTwips nRight;
          SwTwips nFirst;
          sal_uInt16  nDropLeft;
          sal_uInt16  nDropHeight;
          sal_uInt16  nDropDescent;
          sal_uInt16  nDropLines;
          sal_uInt16  nAdjust;
          // #i91133#
          SwTwips mnTabLeft;

protected:
    // For FormatQuoVadis
    inline void Right( const SwTwips nNew ) { nRight = nNew; }

    void CtorInitTextMargin( SwTextFrame *pFrame, SwTextSizeInfo *pInf );
    explicit SwTextMargin(SwTextNode* pTextNode)
        : SwTextIter(pTextNode)
        , nLeft(0)
        , nRight(0)
        , nFirst(0)
        , nDropLeft(0)
        , nDropHeight(0)
        , nDropDescent(0)
        , nDropLines(0)
        , nAdjust(0)
        , mnTabLeft(0)
    {
    }
public:
    SwTextMargin(SwTextFrame *pTextFrame, SwTextSizeInfo *pTextSizeInf)
        : SwTextIter(pTextFrame->GetTextNode())
    {
        CtorInitTextMargin( pTextFrame, pTextSizeInf );
    }
    inline SwTwips GetLeftMargin() const;
    inline SwTwips Left() const;
    inline SwTwips Right() const { return nRight; }
    inline SwTwips FirstLeft() const { return nFirst; }
    inline SwTwips CurrWidth() const { return m_pCurr->PrtWidth(); }
           SwTwips GetLineStart() const;
    inline SwTwips GetLineEnd() const { return GetLineStart() + CurrWidth(); }
    inline Point GetTopLeft() const { return Point( GetLineStart(), Y() ); }
    inline bool IsOneBlock() const { return m_bOneBlock; }
    inline bool IsLastBlock() const { return m_bLastBlock; }
    inline bool IsLastCenter() const { return m_bLastCenter; }
    inline sal_uInt16 GetAdjust() const { return nAdjust; }
    inline sal_uInt16 GetLineWidth() const
           { return sal_uInt16( Right() - GetLeftMargin() + 1 ); }
    inline SwTwips GetLeftMin() const { return nFirst < nLeft ? nFirst : nLeft; }
    inline bool HasNegFirst() const { return nFirst < nLeft; }

    // #i91133#
    inline SwTwips GetTabLeft() const
    {
        return mnTabLeft;
    }
    // DropCaps
    inline sal_uInt16 GetDropLines() const { return nDropLines; }
    inline void SetDropLines( const sal_uInt16 nNew ) { nDropLines = nNew; }
    inline sal_uInt16 GetDropLeft() const { return nDropLeft; }
    inline sal_uInt16 GetDropHeight() const { return nDropHeight; }
    inline void SetDropHeight( const sal_uInt16 nNew ) { nDropHeight = nNew; }
    inline sal_uInt16 GetDropDescent() const { return nDropDescent; }
    inline void SetDropDescent( const sal_uInt16 nNew ) { nDropDescent = nNew; }
    void DropInit();

    // Returns the TextPos for start and end of the current line without whitespace
    // Implemented in frminf.cxx
    sal_Int32 GetTextStart() const;
    sal_Int32 GetTextEnd() const;

    inline SwTextSizeInfo &GetInfo()
        { return static_cast<SwTextSizeInfo&>(SwTextIter::GetInfo()); }
    inline const SwTextSizeInfo &GetInfo() const
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
    SwFlyPortion *CalcFlyPortion( const long nRealWidth,
                                  const SwRect &rCurrRect );

protected:
    explicit SwTextAdjuster(SwTextNode* pTextNode) : SwTextMargin(pTextNode) { }
    // Creates the Glues for adjusted paragraphs
    void CalcNewBlock( SwLineLayout *pCurr, const SwLinePortion *pStopAt,
        SwTwips nReal = 0, bool bSkipKashida = false );
    SwTwips CalcKanaAdj( SwLineLayout *pCurr );
public:
    inline SwTextAdjuster( SwTextFrame *pTextFrame, SwTextSizeInfo *pTextSizeInf ) : SwTextMargin(pTextFrame!=nullptr?pTextFrame->GetTextNode():nullptr)
           { CtorInitTextMargin( pTextFrame, pTextSizeInf ); }

    // Is overloaded by SwTextFormatter due to UpdatePos
    void CalcAdjLine( SwLineLayout *pCurr );

    // For adjusting afterwards
    inline void GetAdjusted() const
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
    static bool bRightMargin;
    void _GetCharRect(SwRect *, const sal_Int32, SwCursorMoveState* );
protected:
    void CtorInitTextCursor( SwTextFrame *pFrame, SwTextSizeInfo *pInf );
    explicit SwTextCursor(SwTextNode* pTextNode) : SwTextAdjuster(pTextNode) { }
public:
    SwTextCursor( SwTextFrame *pTextFrame, SwTextSizeInfo *pTextSizeInf )
        : SwTextAdjuster(pTextFrame->GetTextNode())
    {
        CtorInitTextCursor(pTextFrame, pTextSizeInf);
    }
    bool GetCharRect(SwRect *, const sal_Int32, SwCursorMoveState* = nullptr,
        const long nMax = 0 );
    bool GetEndCharRect(SwRect *, const sal_Int32, SwCursorMoveState* = nullptr,
        const long nMax = 0 );
    sal_Int32 GetCursorOfst( SwPosition *pPos, const Point &rPoint,
                bool bChgNode, SwCursorMoveState* = nullptr ) const;
    // Respects ambiguities: For the implementation see below
    const SwLineLayout *CharCursorToLine( const sal_Int32 nPos );

    // calculates baseline for portion rPor
    // bAutoToCentered indicates, if AUTOMATIC mode means CENTERED or BASELINE
    sal_uInt16 AdjustBaseLine( const SwLineLayout& rLine, const SwLinePortion* pPor,
                           sal_uInt16 nPorHeight = 0, sal_uInt16 nAscent = 0,
                           const bool bAutoToCentered = false ) const;

    static inline void SetRightMargin( const bool bNew ){ bRightMargin = bNew; }
    static inline bool IsRightMargin() { return bRightMargin; }
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
        return SeekAndChgAttrIter( rInf.GetIdx()-1, rInf.GetOut() );
    else
        return SeekAndChgAttrIter( rInf.GetIdx(), rInf.GetOut() );
}

inline bool SwTextIter::SeekStartAndChg( SwTextSizeInfo &rInf, const bool bPara )
{
    return SeekStartAndChgAttrIter( rInf.GetOut(), bPara );
}

inline SwTwips SwTextMargin::GetLeftMargin() const
{
    return IsFirstTextLine() ? nFirst : Left();
}

inline SwTwips SwTextMargin::Left() const
{
    return (nDropLines >= m_nLineNr && 1 != m_nLineNr) ? nFirst + nDropLeft : nLeft;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
