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
    SwLineLayout *pNext;                // The next Line
    std::vector<long>* pLLSpaceAdd;     // Used for justified alignment
    std::deque<sal_uInt16>* pKanaComp;  // Used for Kana compression
    sal_uInt16 nRealHeight;                 // The height resulting from line spacing and register
    bool bFormatAdj : 1;
    bool bDummy     : 1;
    bool bFntChg    : 1;
    bool bEndHyph   : 1;
    bool bMidHyph   : 1;
    bool bTab       : 1;
    bool bFly       : 1;
    bool bRest      : 1;
    bool bBlinking  : 1;
    bool bClipping  : 1; // Clipping needed for exact line height
    bool bContent   : 1; // Text for line numbering
    bool bRedline   : 1; // The Redlining
    bool bForcedLeftMargin : 1; // Left adjustment moved by the Fly
    bool bHanging : 1; // Contains a hanging portion in the margin
    bool bUnderscore : 1;

    SwTwips _GetHangingMargin() const;

public:
    // From SwLinePortion
    virtual SwLinePortion *Insert( SwLinePortion *pPortion ) override;
    virtual SwLinePortion *Append( SwLinePortion *pPortion ) override;
    inline SwLinePortion *GetFirstPortion() const;

    // Flags
    inline void ResetFlags();
    inline void SetFormatAdj( const bool bNew ) { bFormatAdj = bNew; }
    inline bool IsFormatAdj() const { return bFormatAdj; }
    inline void SetEndHyph( const bool bNew ) { bEndHyph = bNew; }
    inline bool IsEndHyph() const { return bEndHyph; }
    inline void SetMidHyph( const bool bNew ) { bMidHyph = bNew; }
    inline bool IsMidHyph() const { return bMidHyph; }
    inline void SetFly( const bool bNew ) { bFly = bNew; }
    inline bool IsFly() const { return bFly; }
    inline void SetRest( const bool bNew ) { bRest = bNew; }
    inline bool IsRest() const { return bRest; }
    inline void SetBlinking( const bool bNew = true ) { bBlinking = bNew; }
    inline bool IsBlinking() const { return bBlinking; }
    inline void SetContent( const bool bNew = true ) { bContent = bNew; }
    inline bool HasContent() const { return bContent; }
    inline void SetRedline( const bool bNew = true ) { bRedline = bNew; }
    inline bool HasRedline() const { return bRedline; }
    inline void SetForcedLeftMargin( const bool bNew = true ) { bForcedLeftMargin = bNew; }
    inline bool HasForcedLeftMargin() const { return bForcedLeftMargin; }
    inline void SetHanging( const bool bNew = true ) { bHanging = bNew; }
    inline void SetUnderscore( const bool bNew = true ) { bUnderscore = bNew; }
    inline bool HasUnderscore() const { return bUnderscore; }

    // Respecting empty dummy lines
    inline void SetDummy( const bool bNew ) { bDummy = bNew; }
    inline bool IsDummy() const { return bDummy; }

    inline void SetClipping( const bool bNew ) { bClipping = bNew; }
    inline bool IsClipping() const { return bClipping; }

    inline SwLineLayout();
    virtual ~SwLineLayout();

    inline SwLineLayout *GetNext() { return pNext; }
    inline const SwLineLayout *GetNext() const { return pNext; }
    inline void SetNext( SwLineLayout *pNew ) { pNext = pNew; }

    void Init( SwLinePortion *pNextPortion = nullptr);

    // Collects the data for the line
    void CalcLine( SwTextFormatter &rLine, SwTextFormatInfo &rInf );

    inline void SetRealHeight( sal_uInt16 nNew ) { nRealHeight = nNew; }
    inline sal_uInt16 GetRealHeight() const { return nRealHeight; }

    // Creates the glue chain for short lines
    SwMarginPortion *CalcLeftMargin();

    inline SwTwips GetHangingMargin() const
        { return _GetHangingMargin(); }

    // For special treatment for empty lines
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    // Stuff for justified alignment
    inline bool IsSpaceAdd() { return pLLSpaceAdd != nullptr; }
    void InitSpaceAdd();     // Creates pLLSpaceAdd if necessary
    void CreateSpaceAdd( const long nInit = 0 );
    inline void FinishSpaceAdd() { delete pLLSpaceAdd; pLLSpaceAdd = nullptr; }
    inline sal_uInt16 GetLLSpaceAddCount() const { return sal::static_int_cast< sal_uInt16 >(pLLSpaceAdd->size()); }
    inline void SetLLSpaceAdd( long nNew, sal_uInt16 nIdx )
    {
        if ( nIdx == GetLLSpaceAddCount() )
            pLLSpaceAdd->push_back( nNew );
        else
            (*pLLSpaceAdd)[ nIdx ] = nNew;
    }
    inline long GetLLSpaceAdd( sal_uInt16 nIdx ) { return (*pLLSpaceAdd)[ nIdx ]; }
    inline void RemoveFirstLLSpaceAdd() { pLLSpaceAdd->erase( pLLSpaceAdd->begin() ); }
    inline std::vector<long>* GetpLLSpaceAdd() const { return pLLSpaceAdd; }

    // Stuff for Kana compression
    inline void SetKanaComp( std::deque<sal_uInt16>* pNew ){ pKanaComp = pNew; }
    inline void FinishKanaComp() { delete pKanaComp; pKanaComp = nullptr; }
    inline std::deque<sal_uInt16>* GetpKanaComp() const { return pKanaComp; }
    inline std::deque<sal_uInt16>& GetKanaComp() { return *pKanaComp; }

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
    SwRepaint aRepaint;
    // Area that needs reformatting
    SwCharRange aReformat;
    SwScriptInfo aScriptInfo;
    // Fraction aZoom;
    long nDelta;

    // If a SwTextFrm is locked, no changes occur to the formatting data (under
    // pLine) (compare with Orphans)
    bool bFlys          : 1; // Overlapping Flys?
    bool bPrep          : 1; // PREP_*
    bool bPrepWidows    : 1; // PREP_WIDOWS
    bool bPrepAdjust    : 1; // PREP_ADJUST_FRM
    bool bPrepMustFit   : 1; // PREP_MUST_FIT
    bool bFollowField   : 1; // We have a bit of field left for the Follow

    bool bFixLineHeight : 1; // Fixed line height
    bool bFootnoteNum    : 1; // contains a footnotenumberportion
    bool bMargin    : 1; // contains a hanging punctuation in the margin

    bool bFlag00    : 1;
    bool bFlag11    : 1;
    bool bFlag12    : 1;
    bool bFlag13    : 1;
    bool bFlag14    : 1;
    bool bFlag15    : 1;
    bool bFlag16    : 1;

public:
    SwParaPortion();
    virtual ~SwParaPortion();

    // Resets all formatting information (except for bFlys)
    inline void FormatReset();

    // Resets the Flags
    inline void ResetPreps();

    // Get/Set methods
    inline SwRepaint& GetRepaint() { return aRepaint; }
    inline const SwRepaint& GetRepaint() const { return aRepaint; }
    inline SwCharRange& GetReformat() { return aReformat; }
    inline const SwCharRange& GetReformat() const { return aReformat; }
    inline long& GetDelta() { return nDelta; }
    inline const long& GetDelta() const { return nDelta; }
    inline SwScriptInfo& GetScriptInfo() { return aScriptInfo; }
    inline const SwScriptInfo& GetScriptInfo() const { return aScriptInfo; }

    // For SwTextFrm::Format: returns the paragraph's current length
    sal_Int32 GetParLen() const;

    // For Prepare()
    bool UpdateQuoVadis( const OUString &rQuo );

    // Flags
    inline void SetFly( const bool bNew = true ) { bFlys = bNew; }
    inline bool HasFly() const { return bFlys; }

    // Preps
    inline void SetPrep( const bool bNew = true ) { bPrep = bNew; }
    inline bool IsPrep() const { return bPrep; }
    inline void SetPrepWidows( const bool bNew = true ) { bPrepWidows = bNew; }
    inline bool IsPrepWidows() const { return bPrepWidows; }
    inline void SetPrepMustFit( const bool bNew = true ) { bPrepMustFit = bNew; }
    inline bool IsPrepMustFit() const { return bPrepMustFit; }
    inline void SetPrepAdjust( const bool bNew = true ) { bPrepAdjust = bNew; }
    inline bool IsPrepAdjust() const { return bPrepAdjust; }
    inline void SetFollowField( const bool bNew = true ) { bFollowField = bNew; }
    inline bool IsFollowField() const { return bFollowField; }
    inline void SetFixLineHeight( const bool bNew = true ) { bFixLineHeight = bNew; }
    inline bool IsFixLineHeight() const { return bFixLineHeight; }

    inline void SetFootnoteNum( const bool bNew = true ) { bFootnoteNum = bNew; }
    inline bool IsFootnoteNum() const { return bFootnoteNum; }
    inline void SetMargin( const bool bNew = true ) { bMargin = bNew; }
    inline bool IsMargin() const { return bMargin; }

    // Set nErgo in the QuoVadisPortion
    void SetErgoSumNum( const OUString &rErgo );

    const SwDropPortion *FindDropPortion() const;

    OUTPUT_OPERATOR_OVERRIDE
    DECL_FIXEDMEMPOOL_NEWDEL(SwParaPortion)
};

inline void SwLineLayout::ResetFlags()
{
    bFormatAdj = bDummy = bFntChg = bTab = bEndHyph = bMidHyph = bFly
    = bRest = bBlinking = bClipping = bContent = bRedline
    = bForcedLeftMargin = bHanging = false;
}

inline SwLineLayout::SwLineLayout()
    : pNext( nullptr ), pLLSpaceAdd( nullptr ), pKanaComp( nullptr ), nRealHeight( 0 ),
      bUnderscore( false )
{
    ResetFlags();
    SetWhichPor( POR_LAY );
}

inline void SwParaPortion::ResetPreps()
{
    bPrep = bPrepWidows = bPrepAdjust = bPrepMustFit = false;
}

inline void SwParaPortion::FormatReset()
{
    nDelta = 0;
    aReformat = SwCharRange(0, COMPLETE_STRING);
    // bFlys needs to be retained in SwTextFrm::_Format() so that empty
    // paragraphs that needed to avoid Frames with no flow, reformat
    // when the Frame disappears from the Area
    // bFlys = false;
    ResetPreps();
    bFollowField = bFixLineHeight = bMargin = false;
}

inline SwLinePortion *SwLineLayout::GetFirstPortion() const
{
    const SwLinePortion *pRet = pPortion ? pPortion : this;
    return const_cast<SwLinePortion*>(pRet);
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
