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

#include "swrect.hxx"
#include "portxt.hxx"
#include "swfont.hxx"

#include <vector>
#include <deque>

class SwMarginPortion;
class SwDropPortion;
class SvStream;
class SwTextFormatter;

class SwCharRange
{
    sal_Int32 nStart, nLen;
public:
    inline SwCharRange( const sal_Int32 nInitStart = 0,
        const sal_Int32 nInitLen = 0): nStart( nInitStart ), nLen(nInitLen) {}
    inline sal_Int32 &Start() { return nStart; }
    inline const sal_Int32 &Start() const { return nStart; }
    inline void LeftMove( sal_Int32 nNew )
            { if ( nNew < nStart ) { nLen += nStart-nNew; nStart = nNew; } }
    inline sal_Int32 &Len() { return nLen; }
    inline const sal_Int32 &Len() const { return nLen; }
    inline bool operator<(const SwCharRange &rRange) const
                { return nStart < rRange.nStart; }
    inline bool operator>(const SwCharRange &rRange) const
                { return nStart + nLen > rRange.nStart + rRange.nLen; }
    inline bool operator!=(const SwCharRange &rRange) const
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
    SwRepaint( const SwRepaint& rRep ) : SwRect( rRep ), nOfst( rRep.nOfst ),
        nRightOfst( rRep.nRightOfst ) {}

    SwTwips GetOfst() const { return nOfst; }
    void   SetOfst( const SwTwips nNew ) { nOfst = nNew; }
    SwTwips GetRightOfst() const { return nRightOfst; }
    void   SetRightOfst( const SwTwips nNew ) { nRightOfst = nNew; }
};

/// Collection of SwLinePortion instances, representing one line of text
class SwLineLayout : public SwTextPortion
{
private:
    SwLineLayout *m_pNext;                // The next Line
    std::vector<long>* m_pLLSpaceAdd;     // Used for justified alignment
    std::deque<sal_uInt16>* m_pKanaComp;  // Used for Kana compression
    sal_uInt16 m_nRealHeight;                 // The height resulting from line spacing and register
    bool m_bFormatAdj : 1;
    bool m_bDummy     : 1;
    bool m_bFntChg    : 1;
    bool m_bEndHyph   : 1;
    bool m_bMidHyph   : 1;
    bool m_bTab       : 1;
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

public:
    // From SwLinePortion
    virtual SwLinePortion *Insert( SwLinePortion *pPortion ) override;
    virtual SwLinePortion *Append( SwLinePortion *pPortion ) override;
    inline SwLinePortion *GetFirstPortion() const;

    // Flags
    inline void ResetFlags();
    inline void SetFormatAdj( const bool bNew ) { m_bFormatAdj = bNew; }
    inline bool IsFormatAdj() const { return m_bFormatAdj; }
    inline void SetEndHyph( const bool bNew ) { m_bEndHyph = bNew; }
    inline bool IsEndHyph() const { return m_bEndHyph; }
    inline void SetMidHyph( const bool bNew ) { m_bMidHyph = bNew; }
    inline bool IsMidHyph() const { return m_bMidHyph; }
    inline void SetFly( const bool bNew ) { m_bFly = bNew; }
    inline bool IsFly() const { return m_bFly; }
    inline void SetRest( const bool bNew ) { m_bRest = bNew; }
    inline bool IsRest() const { return m_bRest; }
    inline void SetBlinking( const bool bNew = true ) { m_bBlinking = bNew; }
    inline bool IsBlinking() const { return m_bBlinking; }
    inline void SetContent( const bool bNew = true ) { m_bContent = bNew; }
    inline bool HasContent() const { return m_bContent; }
    inline void SetRedline( const bool bNew = true ) { m_bRedline = bNew; }
    inline bool HasRedline() const { return m_bRedline; }
    inline void SetForcedLeftMargin() { m_bForcedLeftMargin = true; }
    inline bool HasForcedLeftMargin() const { return m_bForcedLeftMargin; }
    inline void SetHanging( const bool bNew = true ) { m_bHanging = bNew; }
    inline bool IsHanging() const { return m_bHanging; }
    inline void SetUnderscore( const bool bNew = true ) { m_bUnderscore = bNew; }
    inline bool HasUnderscore() const { return m_bUnderscore; }

    // Respecting empty dummy lines
    inline void SetDummy( const bool bNew ) { m_bDummy = bNew; }
    inline bool IsDummy() const { return m_bDummy; }

    inline void SetClipping( const bool bNew ) { m_bClipping = bNew; }
    inline bool IsClipping() const { return m_bClipping; }

    inline SwLineLayout();
    virtual ~SwLineLayout();

    inline SwLineLayout *GetNext() { return m_pNext; }
    inline const SwLineLayout *GetNext() const { return m_pNext; }
    inline void SetNext( SwLineLayout *pNew ) { m_pNext = pNew; }

    void Init( SwLinePortion *pNextPortion = nullptr);

    // Collects the data for the line
    void CalcLine( SwTextFormatter &rLine, SwTextFormatInfo &rInf );

    inline void SetRealHeight( sal_uInt16 nNew ) { m_nRealHeight = nNew; }
    inline sal_uInt16 GetRealHeight() const { return m_nRealHeight; }

    // Creates the glue chain for short lines
    SwMarginPortion *CalcLeftMargin();

    inline SwTwips GetHangingMargin() const
        { return GetHangingMargin_(); }

    // For special treatment for empty lines
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    // Stuff for justified alignment
    inline bool IsSpaceAdd() { return m_pLLSpaceAdd != nullptr; }
    void InitSpaceAdd();     // Creates pLLSpaceAdd if necessary
    void CreateSpaceAdd( const long nInit = 0 );
    inline void FinishSpaceAdd() { delete m_pLLSpaceAdd; m_pLLSpaceAdd = nullptr; }
    inline sal_uInt16 GetLLSpaceAddCount() const { return sal::static_int_cast< sal_uInt16 >(m_pLLSpaceAdd->size()); }
    inline void SetLLSpaceAdd( long nNew, sal_uInt16 nIdx )
    {
        if ( nIdx == GetLLSpaceAddCount() )
            m_pLLSpaceAdd->push_back( nNew );
        else
            (*m_pLLSpaceAdd)[ nIdx ] = nNew;
    }
    inline long GetLLSpaceAdd( sal_uInt16 nIdx ) { return (*m_pLLSpaceAdd)[ nIdx ]; }
    inline void RemoveFirstLLSpaceAdd() { m_pLLSpaceAdd->erase( m_pLLSpaceAdd->begin() ); }
    inline std::vector<long>* GetpLLSpaceAdd() const { return m_pLLSpaceAdd; }

    // Stuff for Kana compression
    inline void SetKanaComp( std::deque<sal_uInt16>* pNew ){ m_pKanaComp = pNew; }
    inline void FinishKanaComp() { delete m_pKanaComp; m_pKanaComp = nullptr; }
    inline std::deque<sal_uInt16>* GetpKanaComp() const { return m_pKanaComp; }
    inline std::deque<sal_uInt16>& GetKanaComp() { return *m_pKanaComp; }

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

    OUTPUT_OPERATOR_OVERRIDE
    DECL_FIXEDMEMPOOL_NEWDEL(SwLineLayout)
};

/// Collection of SwLineLayout instances, represents the paragraph text in Writer layout.
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
    virtual ~SwParaPortion();

    // Resets all formatting information (except for bFlys)
    inline void FormatReset();

    // Resets the Flags
    inline void ResetPreps();

    // Get/Set methods
    inline SwRepaint& GetRepaint() { return m_aRepaint; }
    inline const SwRepaint& GetRepaint() const { return m_aRepaint; }
    inline SwCharRange& GetReformat() { return m_aReformat; }
    inline const SwCharRange& GetReformat() const { return m_aReformat; }
    inline long& GetDelta() { return m_nDelta; }
    inline const long& GetDelta() const { return m_nDelta; }
    inline SwScriptInfo& GetScriptInfo() { return m_aScriptInfo; }
    inline const SwScriptInfo& GetScriptInfo() const { return m_aScriptInfo; }

    // For SwTextFrame::Format: returns the paragraph's current length
    sal_Int32 GetParLen() const;

    // For Prepare()
    bool UpdateQuoVadis( const OUString &rQuo );

    // Flags
    inline void SetFly() { m_bFlys = true; }
    inline bool HasFly() const { return m_bFlys; }

    // Preps
    inline void SetPrep() { m_bPrep = true; }
    inline bool IsPrep() const { return m_bPrep; }
    inline void SetPrepWidows() { m_bPrepWidows = true; }
    inline bool IsPrepWidows() const { return m_bPrepWidows; }
    inline void SetPrepMustFit( const bool bNew = true ) { m_bPrepMustFit = bNew; }
    inline bool IsPrepMustFit() const { return m_bPrepMustFit; }
    inline void SetPrepAdjust() { m_bPrepAdjust = true; }
    inline bool IsPrepAdjust() const { return m_bPrepAdjust; }
    inline void SetFollowField( const bool bNew = true ) { m_bFollowField = bNew; }
    inline bool IsFollowField() const { return m_bFollowField; }
    inline void SetFixLineHeight() { m_bFixLineHeight = true; }
    inline bool IsFixLineHeight() const { return m_bFixLineHeight; }

    inline void SetFootnoteNum( const bool bNew = true ) { m_bFootnoteNum = bNew; }
    inline bool IsFootnoteNum() const { return m_bFootnoteNum; }
    inline void SetMargin( const bool bNew = true ) { m_bMargin = bNew; }
    inline bool IsMargin() const { return m_bMargin; }

    // Set nErgo in the QuoVadisPortion
    void SetErgoSumNum( const OUString &rErgo );

    const SwDropPortion *FindDropPortion() const;

    OUTPUT_OPERATOR_OVERRIDE
    DECL_FIXEDMEMPOOL_NEWDEL(SwParaPortion)
};

inline void SwLineLayout::ResetFlags()
{
    m_bFormatAdj = m_bDummy = m_bFntChg = m_bTab = m_bEndHyph = m_bMidHyph = m_bFly
    = m_bRest = m_bBlinking = m_bClipping = m_bContent = m_bRedline
    = m_bForcedLeftMargin = m_bHanging = false;
}

inline SwLineLayout::SwLineLayout()
    : m_pNext( nullptr ), m_pLLSpaceAdd( nullptr ), m_pKanaComp( nullptr ), m_nRealHeight( 0 ),
      m_bUnderscore( false )
{
    ResetFlags();
    SetWhichPor( POR_LAY );
}

inline void SwParaPortion::ResetPreps()
{
    m_bPrep = m_bPrepWidows = m_bPrepAdjust = m_bPrepMustFit = false;
}

inline void SwParaPortion::FormatReset()
{
    m_nDelta = 0;
    m_aReformat = SwCharRange(0, COMPLETE_STRING);
    // bFlys needs to be retained in SwTextFrame::Format_() so that empty
    // paragraphs that needed to avoid Frames with no flow, reformat
    // when the Frame disappears from the Area
    // bFlys = false;
    ResetPreps();
    m_bFollowField = m_bFixLineHeight = m_bMargin = false;
}

inline SwLinePortion *SwLineLayout::GetFirstPortion() const
{
    const SwLinePortion *pRet = pPortion ? pPortion : this;
    return const_cast<SwLinePortion*>(pRet);
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
