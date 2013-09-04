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
#ifndef _PORLAY_HXX
#define _PORLAY_HXX

#include <tools/string.hxx>
#include <tools/fract.hxx>
#include <scriptinfo.hxx>

#include "swrect.hxx"   // SwRepaint
#include "portxt.hxx"
#include "swfont.hxx"

#include <vector>
#include <deque>

class SwMarginPortion;
class SwDropPortion;
class SvStream;
class SwTxtFormatter;

/*************************************************************************
 * class SwCharRange
 *************************************************************************/

class SwCharRange
{
    xub_StrLen nStart, nLen;
public:
    inline SwCharRange( const xub_StrLen nInitStart = 0,
        const xub_StrLen nInitLen = 0): nStart( nInitStart ), nLen(nInitLen) {}
    inline xub_StrLen &Start() { return nStart; }
    inline const xub_StrLen &Start() const { return nStart; }
    inline void LeftMove( xub_StrLen nNew )
            { if ( nNew < nStart ) { nLen += nStart-nNew; nStart = nNew; } }
    inline xub_StrLen End() const
                { return nStart + nLen; }
    inline xub_StrLen &Len() { return nLen; }
    inline const xub_StrLen &Len() const { return nLen; }
    inline bool operator<(const SwCharRange &rRange) const
                { return nStart < rRange.nStart; }
    inline bool operator>(const SwCharRange &rRange) const
                { return nStart + nLen > rRange.nStart + rRange.nLen; }
    inline bool operator!=(const SwCharRange &rRange) const
                { return *this < rRange || *this > rRange; }
    SwCharRange &operator+=(const SwCharRange &rRange);
};

/*************************************************************************
 * class SwRepaint
 *************************************************************************/

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


/// Collection of SwLinePortion instances, representing one line of text.
class SwLineLayout : public SwTxtPortion
{
private:
    SwLineLayout *pNext;                // The next Line
    std::vector<long>* pLLSpaceAdd;     // Used for justified alignment.
    std::deque<sal_uInt16>* pKanaComp;  // Used for Kana compression.
    KSHORT nRealHeight;                 // The height resulting from line spacing and register
    sal_Bool bFormatAdj : 1;
    sal_Bool bDummy     : 1;
    sal_Bool bFntChg    : 1;
    sal_Bool bEndHyph   : 1;
    sal_Bool bMidHyph   : 1;
    sal_Bool bTab       : 1;
    sal_Bool bFly       : 1;
    sal_Bool bRest      : 1;
    sal_Bool bBlinking  : 1;
    sal_Bool bClipping  : 1; // Clipping needed for exact line height
    sal_Bool bContent   : 1; // Text for line numbering
    sal_Bool bRedline   : 1; // The Redlining
    sal_Bool bForcedLeftMargin : 1; // Left adjustment moved by the Fly
    sal_Bool bHanging : 1; // Contains a hanging portion in the margin
    sal_Bool bUnderscore : 1;

    SwTwips _GetHangingMargin() const;

public:
    // From SwLinePortion
    virtual SwLinePortion *Insert( SwLinePortion *pPortion );
    virtual SwLinePortion *Append( SwLinePortion *pPortion );
    inline SwLinePortion *GetFirstPortion() const;

    // Flags
    inline void ResetFlags();
    inline void SetFormatAdj( const sal_Bool bNew ) { bFormatAdj = bNew; }
    inline sal_Bool IsFormatAdj() const { return bFormatAdj; }
    inline void SetFntChg( const sal_Bool bNew ) { bFntChg = bNew; }
    inline sal_Bool IsFntChg() const { return bFntChg; }
    inline void SetEndHyph( const sal_Bool bNew ) { bEndHyph = bNew; }
    inline sal_Bool IsEndHyph() const { return bEndHyph; }
    inline void SetMidHyph( const sal_Bool bNew ) { bMidHyph = bNew; }
    inline sal_Bool IsMidHyph() const { return bMidHyph; }
    inline void SetTab( const sal_Bool bNew ) { bTab = bNew; }
    inline sal_Bool IsTab() const { return bTab; }
    inline void SetFly( const sal_Bool bNew ) { bFly = bNew; }
    inline sal_Bool IsFly() const { return bFly; }
    inline void SetRest( const sal_Bool bNew ) { bRest = bNew; }
    inline sal_Bool IsRest() const { return bRest; }
    inline void SetBlinking( const sal_Bool bNew = sal_True ) { bBlinking = bNew; }
    inline sal_Bool IsBlinking() const { return bBlinking; }
    inline void SetCntnt( const sal_Bool bNew = sal_True ) { bContent = bNew; }
    inline sal_Bool HasCntnt() const { return bContent; }
    inline void SetRedline( const sal_Bool bNew = sal_True ) { bRedline = bNew; }
    inline sal_Bool HasRedline() const { return bRedline; }
    inline void SetForcedLeftMargin( const sal_Bool bNew = sal_True ) { bForcedLeftMargin = bNew; }
    inline sal_Bool HasForcedLeftMargin() const { return bForcedLeftMargin; }
    inline void SetHanging( const sal_Bool bNew = sal_True ) { bHanging = bNew; }
    inline sal_Bool IsHanging() const { return bHanging; }
    inline void SetUnderscore( const sal_Bool bNew = sal_True ) { bUnderscore = bNew; }
    inline sal_Bool HasUnderscore() const { return bUnderscore; }

    // Respecting empty dummy lines
    inline void SetDummy( const sal_Bool bNew ) { bDummy = bNew; }
    inline sal_Bool IsDummy() const { return bDummy; }

    inline void SetClipping( const sal_Bool bNew ) { bClipping = bNew; }
    inline sal_Bool IsClipping() const { return bClipping; }

    inline SwLineLayout();
    virtual ~SwLineLayout();

    inline SwLineLayout *GetNext() { return pNext; }
    inline const SwLineLayout *GetNext() const { return pNext; }
    inline void SetNext( SwLineLayout *pNew ) { pNext = pNew; }

    void Init( SwLinePortion *pNextPortion = NULL);

    // Collects the data for the line
    void CalcLine( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf );

    inline void SetRealHeight( KSHORT nNew ) { nRealHeight = nNew; }
    inline KSHORT GetRealHeight() const { return nRealHeight; }

    // Creates the glue chain for short lines
    SwMarginPortion *CalcLeftMargin();

    inline SwTwips GetHangingMargin() const
        { return _GetHangingMargin(); }

    // For special treatment for empty lines
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    //
    // STUFF FOR JUSTIFIED ALIGNMENT
    //
    inline bool IsSpaceAdd() { return pLLSpaceAdd != NULL; }
    void InitSpaceAdd();     // Creates pLLSpaceAdd if necessary
    void CreateSpaceAdd( const long nInit = 0 );
    inline void FinishSpaceAdd() { delete pLLSpaceAdd; pLLSpaceAdd = NULL; }
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

    //
    // STUFF FOR KANA COMPRESSION
    //
    inline void SetKanaComp( std::deque<sal_uInt16>* pNew ){ pKanaComp = pNew; }
    inline void FinishKanaComp() { delete pKanaComp; pKanaComp = NULL; }
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

        @author OD
    */
    void MaxAscentDescent( SwTwips& _orAscent,
                           SwTwips& _orDescent,
                           SwTwips& _orObjAscent,
                           SwTwips& _orObjDescent,
                           const SwLinePortion* _pDontConsiderPortion = NULL,
                           const bool _bNoFlyCntPorAndLinePor = false ) const;

    OUTPUT_OPERATOR
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

    // If a SwTxtFrm is locked, no changes occur to the formatting data (under
    // pLine) (compare with Orphans)
    sal_Bool bFlys          : 1; // Overlapping Flys?
    sal_Bool bPrep          : 1; // PREP_*
    sal_Bool bPrepWidows    : 1; // PREP_WIDOWS
    sal_Bool bPrepAdjust    : 1; // PREP_ADJUST_FRM
    sal_Bool bPrepMustFit   : 1; // PREP_MUST_FIT
    sal_Bool bFollowField   : 1; // We have a bit of field left for the Follow

    sal_Bool bFixLineHeight : 1; // Fixed line height
    sal_Bool bFtnNum    : 1; // contains a footnotenumberportion
    sal_Bool bMargin    : 1; // contains a hanging punctuation in the margin

    sal_Bool bFlag00    : 1;
    sal_Bool bFlag11    : 1;
    sal_Bool bFlag12    : 1;
    sal_Bool bFlag13    : 1;
    sal_Bool bFlag14    : 1;
    sal_Bool bFlag15    : 1;
    sal_Bool bFlag16    : 1;

public:
    SwParaPortion();
    virtual ~SwParaPortion();

    // Resets all formatting information (except for bFlys)
    inline void FormatReset();

    // Resets the Flags
    inline void ResetPreps();

    // Get/Set methods
    inline SwRepaint *GetRepaint() { return &aRepaint; }
    inline const SwRepaint *GetRepaint() const { return &aRepaint; }
    inline SwCharRange *GetReformat() { return &aReformat; }
    inline const SwCharRange *GetReformat() const { return &aReformat; }
    inline long *GetDelta() { return &nDelta; }
    inline const long *GetDelta() const { return &nDelta; }
    inline SwScriptInfo& GetScriptInfo() { return aScriptInfo; }
    inline const SwScriptInfo& GetScriptInfo() const { return aScriptInfo; }

    // For SwTxtFrm::Format: returns the paragraph's current length
    xub_StrLen GetParLen() const;

    // For Prepare()
    sal_Bool UpdateQuoVadis( const OUString &rQuo );

    // Flags
    inline void SetFly( const sal_Bool bNew = sal_True ) { bFlys = bNew; }
    inline sal_Bool HasFly() const { return bFlys; }

    // Preps
    inline void SetPrep( const sal_Bool bNew = sal_True ) { bPrep = bNew; }
    inline sal_Bool IsPrep() const { return bPrep; }
    inline void SetPrepWidows( const sal_Bool bNew = sal_True ) { bPrepWidows = bNew; }
    inline sal_Bool IsPrepWidows() const { return bPrepWidows; }
    inline void SetPrepMustFit( const sal_Bool bNew = sal_True ) { bPrepMustFit = bNew; }
    inline sal_Bool IsPrepMustFit() const { return bPrepMustFit; }
    inline void SetPrepAdjust( const sal_Bool bNew = sal_True ) { bPrepAdjust = bNew; }
    inline sal_Bool IsPrepAdjust() const { return bPrepAdjust; }
    inline void SetFollowField( const sal_Bool bNew = sal_True ) { bFollowField = bNew; }
    inline sal_Bool IsFollowField() const { return bFollowField; }
    inline void SetFixLineHeight( const sal_Bool bNew = sal_True ) { bFixLineHeight = bNew; }
    inline sal_Bool  IsFixLineHeight() const { return bFixLineHeight; }

    inline void SetFtnNum( const sal_Bool bNew = sal_True ) { bFtnNum = bNew; }
    inline sal_Bool  IsFtnNum() const { return bFtnNum; }
    inline void SetMargin( const sal_Bool bNew = sal_True ) { bMargin = bNew; }
    inline sal_Bool  IsMargin() const { return bMargin; }
    inline void SetFlag00( const sal_Bool bNew = sal_True ) { bFlag00 = bNew; }
    inline sal_Bool  IsFlag00() const { return bFlag00; }
    inline void SetFlag11( const sal_Bool bNew = sal_True ) { bFlag11 = bNew; }
    inline sal_Bool  IsFlag11() const { return bFlag11; }
    inline void SetFlag12( const sal_Bool bNew = sal_True ) { bFlag12 = bNew; }
    inline sal_Bool  IsFlag12() const { return bFlag12; }
    inline void SetFlag13( const sal_Bool bNew = sal_True ) { bFlag13 = bNew; }
    inline sal_Bool  IsFlag13() const { return bFlag13; }
    inline void SetFlag14( const sal_Bool bNew = sal_True ) { bFlag14 = bNew; }
    inline sal_Bool  IsFlag14() const { return bFlag14; }
    inline void SetFlag15( const sal_Bool bNew = sal_True ) { bFlag15 = bNew; }
    inline sal_Bool  IsFlag15() const { return bFlag15; }
    inline void SetFlag16( const sal_Bool bNew = sal_True ) { bFlag16 = bNew; }
    inline sal_Bool  IsFlag16() const { return bFlag16; }

    // Read/Write methods for the SWG filter
    SvStream &ReadSwg ( SvStream& rStream ); //$ istream
    SvStream &WriteSwg( SvStream& rStream ); //$ ostream

    // Set nErgo in the QuoVadisPortion
    void SetErgoSumNum( const OUString &rErgo );

    const SwDropPortion *FindDropPortion() const;

#ifdef DBG_UTIL
    void dumpAsXml( xmlTextWriter* writer, SwTxtFrm* pTxtFrm );
#endif

    OUTPUT_OPERATOR
    DECL_FIXEDMEMPOOL_NEWDEL(SwParaPortion)
};

/*************************************************************************
 * Inline implementations
 *************************************************************************/

inline void SwLineLayout::ResetFlags()
{
    bFormatAdj = bDummy = bFntChg = bTab = bEndHyph = bMidHyph = bFly
    = bRest = bBlinking = bClipping = bContent = bRedline
    = bForcedLeftMargin = bHanging = sal_False;
}

inline SwLineLayout::SwLineLayout()
    : pNext( 0 ), pLLSpaceAdd( 0 ), pKanaComp( 0 ), nRealHeight( 0 ),
      bUnderscore( sal_False )
{
    ResetFlags();
    SetWhichPor( POR_LAY );
}

inline void SwParaPortion::ResetPreps()
{
    bPrep = bPrepWidows = bPrepAdjust = bPrepMustFit = sal_False;
}

inline void SwParaPortion::FormatReset()
{
    nDelta = 0;
    aReformat = SwCharRange( 0, STRING_LEN );
    // bFlys needs to be retained in SwTxtFrm::_Format() so that empty
    // paragraphs that needed to avoid Frames with no flow, reformat
    // when the Frame disappears from the Area
    // bFlys = sal_False;
    ResetPreps();
    bFollowField = bFixLineHeight = bMargin = sal_False;
}

inline SwLinePortion *SwLineLayout::GetFirstPortion() const
{ return( pPortion ? pPortion : (SwLinePortion*)this ); }

CLASSIO( SwLineLayout )
CLASSIO( SwParaPortion )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
