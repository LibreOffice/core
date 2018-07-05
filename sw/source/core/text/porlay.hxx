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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORLAY_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORLAY_HXX

#include <tools/fract.hxx>
#include <scriptinfo.hxx>

#include <swrect.hxx>
#include "portxt.hxx"
#include <swfont.hxx>

#include <vector>
#include <deque>

class SwMarginPortion;
class SwDropPortion;
class SvStream;
class SwTextFormatter;

class SwCharRange
{
private:
    TextFrameIndex nStart;
    TextFrameIndex nLen;

public:
    SwCharRange(TextFrameIndex const nInitStart = TextFrameIndex(0),
                TextFrameIndex const nInitLen = TextFrameIndex(0))
        : nStart( nInitStart ), nLen(nInitLen) {}
    TextFrameIndex      & Start()       { return nStart; }
    TextFrameIndex const& Start() const { return nStart; }
    void LeftMove(TextFrameIndex const nNew)
            { if ( nNew < nStart ) { nLen += nStart-nNew; nStart = nNew; } }
    TextFrameIndex      & Len()       { return nLen; }
    TextFrameIndex const& Len() const { return nLen; }
    bool operator<(const SwCharRange &rRange) const
                { return nStart < rRange.nStart; }
    bool operator>(const SwCharRange &rRange) const
                { return nStart + nLen > rRange.nStart + rRange.nLen; }
    bool operator!=(const SwCharRange &rRange) const
                { return *this < rRange || *this > rRange; }
    SwCharRange &operator+=(const SwCharRange &rRange);
};

// SwRepaint is a document-global SwRect
// nOfst states from where in the first line should be painted
// nRightOfst gives the right margin
class SwRepaint : public SwRect
{
    SwTwips nOfst;
    SwTwips nRightOfst;
public:
    SwRepaint() : SwRect(), nOfst( 0 ), nRightOfst( 0 ) {}

    SwTwips GetOfst() const { return nOfst; }
    void   SetOfst( const SwTwips nNew ) { nOfst = nNew; }
    SwTwips GetRightOfst() const { return nRightOfst; }
    void   SetRightOfst( const SwTwips nNew ) { nRightOfst = nNew; }
};

/// Collection of SwLinePortion instances, representing one line of text.
/// Typically owned by an SwParaPortion.
class SwLineLayout : public SwTextPortion
{
private:
    SwLineLayout *m_pNext;                // The next Line
    std::unique_ptr<std::vector<long>> m_pLLSpaceAdd;     // Used for justified alignment
    std::unique_ptr<std::deque<sal_uInt16>> m_pKanaComp;  // Used for Kana compression
    sal_uInt16 m_nRealHeight;             // The height resulting from line spacing and register
    bool m_bFormatAdj : 1;
    bool m_bDummy     : 1;
    bool m_bEndHyph   : 1;
    bool m_bMidHyph   : 1;
    bool m_bFly       : 1;
    bool m_bRest      : 1;
    bool m_bBlinking  : 1;
    bool m_bClipping  : 1; // Clipping needed for exact line height
    bool m_bContent   : 1; // Text for line numbering
    bool m_bRedline   : 1; // The Redlining
    bool m_bForcedLeftMargin : 1; // Left adjustment moved by the Fly
    bool m_bHanging : 1; // Contains a hanging portion in the margin
    bool m_bUnderscore : 1;

    SwTwips GetHangingMargin_() const;

    void DeleteNext();
public:
    // From SwLinePortion
    virtual SwLinePortion *Insert( SwLinePortion *pPortion ) override;
    virtual SwLinePortion *Append( SwLinePortion *pPortion ) override;
    SwLinePortion *GetFirstPortion() const;

    // Flags
    void ResetFlags();
    void SetFormatAdj( const bool bNew ) { m_bFormatAdj = bNew; }
    bool IsFormatAdj() const { return m_bFormatAdj; }
    void SetEndHyph( const bool bNew ) { m_bEndHyph = bNew; }
    bool IsEndHyph() const { return m_bEndHyph; }
    void SetMidHyph( const bool bNew ) { m_bMidHyph = bNew; }
    bool IsMidHyph() const { return m_bMidHyph; }
    void SetFly( const bool bNew ) { m_bFly = bNew; }
    bool IsFly() const { return m_bFly; }
    void SetRest( const bool bNew ) { m_bRest = bNew; }
    bool IsRest() const { return m_bRest; }
    void SetBlinking( const bool bNew ) { m_bBlinking = bNew; }
    bool IsBlinking() const { return m_bBlinking; }
    void SetContent( const bool bNew ) { m_bContent = bNew; }
    bool HasContent() const { return m_bContent; }
    void SetRedline( const bool bNew ) { m_bRedline = bNew; }
    bool HasRedline() const { return m_bRedline; }
    void SetForcedLeftMargin() { m_bForcedLeftMargin = true; }
    bool HasForcedLeftMargin() const { return m_bForcedLeftMargin; }
    void SetHanging( const bool bNew ) { m_bHanging = bNew; }
    bool IsHanging() const { return m_bHanging; }
    void SetUnderscore( const bool bNew ) { m_bUnderscore = bNew; }
    bool HasUnderscore() const { return m_bUnderscore; }

    // Respecting empty dummy lines
    void SetDummy( const bool bNew ) { m_bDummy = bNew; }
    bool IsDummy() const { return m_bDummy; }

    void SetClipping( const bool bNew ) { m_bClipping = bNew; }
    bool IsClipping() const { return m_bClipping; }

    SwLineLayout();
    virtual ~SwLineLayout() override;

    SwLineLayout *GetNext() { return m_pNext; }
    const SwLineLayout *GetNext() const { return m_pNext; }
    void SetNext( SwLineLayout *pNew ) { m_pNext = pNew; }

    void Init( SwLinePortion *pNextPortion = nullptr);

    // Collects the data for the line
    void CalcLine( SwTextFormatter &rLine, SwTextFormatInfo &rInf );

    void SetRealHeight( sal_uInt16 nNew ) { m_nRealHeight = nNew; }
    sal_uInt16 GetRealHeight() const { return m_nRealHeight; }

    // Creates the glue chain for short lines
    SwMarginPortion *CalcLeftMargin();

    SwTwips GetHangingMargin() const
        { return GetHangingMargin_(); }

    // For special treatment for empty lines
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    // Stuff for justified alignment
    bool IsSpaceAdd() { return m_pLLSpaceAdd != nullptr; }
    void InitSpaceAdd();     // Creates pLLSpaceAdd if necessary
    void CreateSpaceAdd( const long nInit = 0 );
    void FinishSpaceAdd() { m_pLLSpaceAdd.reset(); }
    sal_uInt16 GetLLSpaceAddCount() const { return sal::static_int_cast< sal_uInt16 >(m_pLLSpaceAdd->size()); }
    void SetLLSpaceAdd( long nNew, sal_uInt16 nIdx )
    {
        if ( nIdx == GetLLSpaceAddCount() )
            m_pLLSpaceAdd->push_back( nNew );
        else
            (*m_pLLSpaceAdd)[ nIdx ] = nNew;
    }
    long GetLLSpaceAdd( sal_uInt16 nIdx ) { return (*m_pLLSpaceAdd)[ nIdx ]; }
    void RemoveFirstLLSpaceAdd() { m_pLLSpaceAdd->erase( m_pLLSpaceAdd->begin() ); }
    std::vector<long>* GetpLLSpaceAdd() const { return m_pLLSpaceAdd.get(); }

    // Stuff for Kana compression
    void SetKanaComp( std::unique_ptr<std::deque<sal_uInt16>> pNew ){ m_pKanaComp = std::move(pNew); }
    void FinishKanaComp() { m_pKanaComp.reset(); }
    std::deque<sal_uInt16>* GetpKanaComp() const { return m_pKanaComp.get(); }
    std::deque<sal_uInt16>& GetKanaComp() { return *m_pKanaComp; }

    /** determine ascent and descent for positioning of as-character anchored
        object

        OD 07.01.2004 #i11859# - previously local method <lcl_MaxAscDescent>
        Method calculates maximum ascents and descents of the line layout.
        One value considering as-character anchored objects, one without these
        objects.
        Portions for other anchored objects aren't considered.
        OD 2005-05-20 #i47162# - add optional parameter <_bNoFlyCntPorAndLinePor>
        to control, if the fly content portions and line portion are considered.

        @param _orAscent
        output parameter - maximum ascent without as-character anchored objects

        @param _orDescent
        output parameter - maximum descent without as-character anchored objects

        @param _orObjAscent
        output parameter - maximum ascent with as-character anchored objects

        @param _orObjDescent
        output parameter - maximum descent with as-character anchored objects

        @param _pDontConsiderPortion
        input parameter - portion, which isn't considered for calculating
        <_orObjAscent> and <_orObjDescent>, if it isn't a portion for a
        as-character anchored object or it isn't as high as the line.

        @param _bNoFlyCntPorAndLinePor
        optional input parameter - boolean, indicating that fly content portions
        and the line portion are considered or not.
    */
    void MaxAscentDescent( SwTwips& _orAscent,
                           SwTwips& _orDescent,
                           SwTwips& _orObjAscent,
                           SwTwips& _orObjDescent,
                           const SwLinePortion* _pDontConsiderPortion = nullptr,
                           const bool _bNoFlyCntPorAndLinePor = false ) const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwLineLayout)
};

/// Collection of SwLineLayout instances, represents the paragraph text in Writer layout.
/// Typically owned by an SwTextFrame.
class SwParaPortion : public SwLineLayout
{
    // Area that needs repainting
    SwRepaint m_aRepaint;
    // Area that needs reformatting
    SwCharRange m_aReformat;
    SwScriptInfo m_aScriptInfo;

    // Fraction aZoom;
    long m_nDelta;

    // If a SwTextFrame is locked, no changes occur to the formatting data (under
    // pLine) (compare with Orphans)
    bool m_bFlys          : 1; // Overlapping Flys?
    bool m_bPrep          : 1; // PREP_*
    bool m_bPrepWidows    : 1; // PREP_WIDOWS
    bool m_bPrepAdjust    : 1; // PREP_ADJUST_FRM
    bool m_bPrepMustFit   : 1; // PREP_MUST_FIT
    bool m_bFollowField   : 1; // We have a bit of field left for the Follow

    bool m_bFixLineHeight : 1; // Fixed line height
    bool m_bFootnoteNum   : 1; // contains a footnotenumberportion
    bool m_bMargin        : 1; // contains a hanging punctuation in the margin

public:
    SwParaPortion();
    virtual ~SwParaPortion() override;

    // Resets all formatting information (except for bFlys)
    inline void FormatReset();

    // Resets the Flags
    inline void ResetPreps();

    // Get/Set methods
    SwRepaint& GetRepaint() { return m_aRepaint; }
    const SwRepaint& GetRepaint() const { return m_aRepaint; }
    SwCharRange& GetReformat() { return m_aReformat; }
    const SwCharRange& GetReformat() const { return m_aReformat; }
    long& GetDelta() { return m_nDelta; }
    const long& GetDelta() const { return m_nDelta; }
    SwScriptInfo& GetScriptInfo() { return m_aScriptInfo; }
    const SwScriptInfo& GetScriptInfo() const { return m_aScriptInfo; }

    // For SwTextFrame::Format: returns the paragraph's current length
    TextFrameIndex GetParLen() const;

    // For Prepare()
    bool UpdateQuoVadis( const OUString &rQuo );

    // Flags
    void SetFly() { m_bFlys = true; }
    bool HasFly() const { return m_bFlys; }

    // Preps
    void SetPrep() { m_bPrep = true; }
    bool IsPrep() const { return m_bPrep; }
    void SetPrepWidows() { m_bPrepWidows = true; }
    bool IsPrepWidows() const { return m_bPrepWidows; }
    void SetPrepMustFit( const bool bNew ) { m_bPrepMustFit = bNew; }
    bool IsPrepMustFit() const { return m_bPrepMustFit; }
    void SetPrepAdjust() { m_bPrepAdjust = true; }
    bool IsPrepAdjust() const { return m_bPrepAdjust; }
    void SetFollowField( const bool bNew ) { m_bFollowField = bNew; }
    bool IsFollowField() const { return m_bFollowField; }
    void SetFixLineHeight() { m_bFixLineHeight = true; }
    bool IsFixLineHeight() const { return m_bFixLineHeight; }

    void SetFootnoteNum( const bool bNew ) { m_bFootnoteNum = bNew; }
    bool IsFootnoteNum() const { return m_bFootnoteNum; }
    void SetMargin( const bool bNew = true ) { m_bMargin = bNew; }
    bool IsMargin() const { return m_bMargin; }

    // Set nErgo in the QuoVadisPortion
    void SetErgoSumNum( const OUString &rErgo );

    const SwDropPortion *FindDropPortion() const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwParaPortion)
};

inline void SwParaPortion::ResetPreps()
{
    m_bPrep = m_bPrepWidows = m_bPrepAdjust = m_bPrepMustFit = false;
}

inline void SwParaPortion::FormatReset()
{
    m_nDelta = 0;
    m_aReformat = SwCharRange(TextFrameIndex(0), TextFrameIndex(COMPLETE_STRING));
    // bFlys needs to be retained in SwTextFrame::Format_() so that empty
    // paragraphs that needed to avoid Frames with no flow, reformat
    // when the Frame disappears from the Area
    // bFlys = false;
    ResetPreps();
    m_bFollowField = m_bFixLineHeight = m_bMargin = false;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
