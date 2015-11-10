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

class SwTextFrm;
struct SwPosition;
struct SwCrsrMoveState;
class SwMarginPortion;
class SwFlyPortion;

class SwTextIter : public SwAttrIter
{
protected:
    SwLineInfo aLineInf;
    SwTextFrm  *pFrm;
    SwTextInfo *pInf;
    SwLineLayout *pCurr;
    SwLineLayout *pPrev;
    SwTwips nFrameStart;
    SwTwips nY;
    SwTwips nRegStart;          // The register's start position (Y)
    sal_Int32 nStart;          // Start in the text string, end = pCurr->GetLen()
    sal_uInt16 nRegDiff;            // Register's line distance
    sal_uInt16 nLineNr;             // Line number
    bool bPrev          : 1;
    bool bRegisterOn    : 1;    // Keep in register
    bool bOneBlock      : 1;    // Justified text: Dispose single words
    bool bLastBlock     : 1;    // Justified text: Also the last line
    bool bLastCenter    : 1;    // Justified text: Center last line

    SwLineLayout *_GetPrev();

    // Reset in the first line
    void Init();
    void CtorInitTextIter( SwTextFrm *pFrm, SwTextInfo *pInf );
    explicit SwTextIter(SwTextNode* pTextNode)
        : SwAttrIter(pTextNode)
        , pFrm(nullptr)
        , pInf(nullptr)
        , pCurr(nullptr)
        , pPrev(nullptr)
        , nFrameStart(0)
        , nY(0)
        , nRegStart(0)
        , nStart(0)
        , nRegDiff(0)
        , nLineNr(0)
        , bPrev(false)
        , bRegisterOn(false)
        , bOneBlock(false)
        , bLastBlock(false)
        , bLastCenter(false)
    {
    }
public:
    SwTextIter(SwTextFrm *pTextFrm, SwTextInfo *pTextInf)
        : SwAttrIter(pTextFrm->GetTextNode())
        , bOneBlock(false)
        , bLastBlock(false)
        , bLastCenter(false)
    {
        CtorInitTextIter(pTextFrm, pTextInf);
    }
    inline const SwLineLayout *GetCurr() const { return pCurr; } // NEVER 0!
    inline const SwLineLayout *GetNext() const { return pCurr->GetNext(); }
           const SwLineLayout *GetPrev();
    inline sal_Int32 GetLength() const { return pCurr->GetLen(); }
    inline sal_uInt16 GetLineNr() const { return nLineNr; }
    inline sal_Int32 GetStart() const { return nStart; }
    inline sal_Int32 GetEnd() const { return GetStart() + GetLength(); }
    inline SwTwips Y() const { return nY; }

    inline SwTwips RegStart() const { return nRegStart; }
    inline sal_uInt16 RegDiff() const { return nRegDiff; }
    inline bool IsRegisterOn() const { return bRegisterOn; }

    inline SwTextInfo &GetInfo() { return *pInf; }
    inline const SwTextInfo &GetInfo() const { return *pInf; }

    inline void Top() { Init(); }
    void Bottom();
    const SwLineLayout *Next();
    const SwLineLayout *Prev();

    // Skips the FlyFrms dummy line
    const SwLineLayout *NextLine();
    const SwLineLayout *PrevLine();
    const SwLineLayout *GetNextLine() const;
    const SwLineLayout *GetPrevLine();

    void CharToLine( const sal_Int32 );
    const SwLineLayout *TwipsToLine(const SwTwips);

    // Truncates all after pCurr
    void TruncLines( bool bNoteFollow = false );

    inline sal_uInt16 GetLineHeight() const { return pCurr->GetRealHeight(); }
    void CalcAscentAndHeight( sal_uInt16 &rAscent, sal_uInt16 &rHeight ) const;

    // Lots of trouble for querying pCurr == pPara
    inline bool IsFirstTextLine() const
    { return nStart == GetInfo().GetTextStart() &&
        !( pCurr->IsDummy() && GetNextLine() ); }

    // Replacement for the old IsFirstLine()
    inline bool IsParaLine() const
        { return pCurr == pInf->GetParaPortion(); }

    const SwLineInfo &GetLineInfo() const { return aLineInf; }
    inline SwTwips GetFirstPos() const { return nFrameStart; }
    inline bool SeekAndChg( SwTextSizeInfo &rInf );
    inline bool SeekAndChgBefore( SwTextSizeInfo &rInf );
    inline bool SeekStartAndChg( SwTextSizeInfo &rInf, const bool bPara=false );

    inline SwTextFrm *GetTextFrm() { return pFrm; }
    inline const SwTextFrm *GetTextFrm() const { return pFrm; }

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

    void CtorInitTextMargin( SwTextFrm *pFrm, SwTextSizeInfo *pInf );
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
    SwTextMargin(SwTextFrm *pTextFrm, SwTextSizeInfo *pTextSizeInf)
        : SwTextIter(pTextFrm->GetTextNode())
    {
        CtorInitTextMargin( pTextFrm, pTextSizeInf );
    }
    inline SwTwips GetLeftMargin() const;
    inline SwTwips Left() const;
    inline SwTwips Right() const { return nRight; }
    inline SwTwips FirstLeft() const { return nFirst; }
    inline SwTwips CurrWidth() const { return pCurr->PrtWidth(); }
           SwTwips GetLineStart() const;
    inline SwTwips GetLineEnd() const { return GetLineStart() + CurrWidth(); }
    inline Point GetTopLeft() const { return Point( GetLineStart(), Y() ); }
    inline bool IsOneBlock() const { return bOneBlock; }
    inline bool IsLastBlock() const { return bLastBlock; }
    inline bool IsLastCenter() const { return bLastCenter; }
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
    // Adjusts the portion, if we have adjustment and FlyFrms
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
    inline SwTextAdjuster( SwTextFrm *pTextFrm, SwTextSizeInfo *pTextSizeInf ) : SwTextMargin(pTextFrm!=nullptr?pTextFrm->GetTextNode():nullptr)
           { CtorInitTextMargin( pTextFrm, pTextSizeInf ); }

    // Is overloaded by SwTextFormatter due to UpdatePos
    void CalcAdjLine( SwLineLayout *pCurr );

    // For adjusting afterwards
    inline void GetAdjusted() const
    {
        if( pCurr->IsFormatAdj() )
            const_cast<SwTextAdjuster*>(this)->CalcAdjLine( pCurr );
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
    void _GetCharRect(SwRect *, const sal_Int32, SwCrsrMoveState* );
protected:
    void CtorInitTextCursor( SwTextFrm *pFrm, SwTextSizeInfo *pInf );
    explicit SwTextCursor(SwTextNode* pTextNode) : SwTextAdjuster(pTextNode) { }
public:
    SwTextCursor( SwTextFrm *pTextFrm, SwTextSizeInfo *pTextSizeInf )
        : SwTextAdjuster(pTextFrm->GetTextNode())
    {
        CtorInitTextCursor(pTextFrm, pTextSizeInf);
    }
    bool GetCharRect(SwRect *, const sal_Int32, SwCrsrMoveState* = nullptr,
        const long nMax = 0 );
    bool GetEndCharRect(SwRect *, const sal_Int32, SwCrsrMoveState* = nullptr,
        const long nMax = 0 );
    sal_Int32 GetCrsrOfst( SwPosition *pPos, const Point &rPoint,
                bool bChgNode, SwCrsrMoveState* = nullptr ) const;
    // Respects ambiguities: For the implementation see below
    const SwLineLayout *CharCrsrToLine( const sal_Int32 nPos );

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
    return (nDropLines >= nLineNr && 1 != nLineNr) ? nFirst + nDropLeft : nLeft;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
