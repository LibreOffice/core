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
#if 1

#include "porlay.hxx"
#include "porexp.hxx"

class SwTxtFormatInfo;
class SwFldPortion;
class SwTxtCursor;
class SwLineLayout;
class SwTxtPaintInfo;
class SwTxtAttr;
class SfxPoolItem;
class SwFont;

/*--------------------------------------------------
 * SwMultiCreator is a small structur to create a multiportion.
 * It contains the kind of multiportion and a textattribute
 * or a poolitem.
 * The GetMultiCreator-function fills this structur and
 * the Ctor of the SwMultiPortion uses it.
 * --------------------------------------------------*/

#define SW_MC_DOUBLE    0
#define SW_MC_RUBY      1
#define SW_MC_ROTATE    2
#define SW_MC_BIDI      3

struct SwMultiCreator
{
    const SwTxtAttr* pAttr;
    const SfxPoolItem* pItem;
    sal_uInt8 nId;
    sal_uInt8 nLevel;
};

/*--------------------------------------------------
 * A two-line-portion (SwMultiPortion) could have surrounding brackets,
 * in this case the structur SwBracket will be used.
 * --------------------------------------------------*/

struct SwBracket
{
    xub_StrLen nStart;      // Start of text attribute determins the font
    KSHORT nAscent;         // Ascent of the brackets
    KSHORT nHeight;         // Height of them
    KSHORT nPreWidth;       // Width of the opening bracket
    KSHORT nPostWidth;      // Width of the closing bracket
    sal_Unicode cPre;       // Initial character, e.g. '('
    sal_Unicode cPost;      // Final character, e.g. ')'
    sal_uInt8 nPreScript;       // Script of the initial character
    sal_uInt8 nPostScript;       // Script of the final character
};

/*--------------------------------------------------
 * The SwMultiPortion is line portion inside a line portion,
 * it's a group of portions,
 * e.g. a double line portion in a line
 * or phonetics (ruby)
 * or combined characters
 * or a rotated portion.
 * --------------------------------------------------*/

class SwMultiPortion : public SwLinePortion
{
    SwLineLayout aRoot;     // One or more lines
    SwFldPortion *pFldRest; // Field rest from the previous line
    sal_Bool bTab1      :1; // First line tabulator
    sal_Bool bTab2      :1; // Second line includes tabulator
    sal_Bool bDouble    :1; // Double line
    bool bRuby      :1; // Phonetics
    sal_Bool bBidi      :1;
    sal_Bool bTop       :1; // Phonetic position
    sal_Bool bFormatted :1; // Already formatted
    sal_Bool bFollowFld :1; // Field follow inside
    sal_uInt8 nDirection:2; // Direction (0/90/180/270 degrees)
    sal_Bool bFlyInCntnt:1; // Fly as character inside
protected:
    SwMultiPortion( xub_StrLen nEnd ) : pFldRest( 0 ), bTab1( sal_False ),
        bTab2( sal_False ), bDouble( sal_False ), bRuby( false ),
        bBidi( sal_False ), bFormatted( sal_False ), bFollowFld( sal_False ),
        nDirection( 0 ), bFlyInCntnt( sal_False )
        { SetWhichPor( POR_MULTI ); SetLen( nEnd ); }
    inline void SetDouble() { bDouble = sal_True; }
    inline void SetRuby() { bRuby = true; }
    inline void SetBidi() { bBidi = sal_True; }
    inline void SetTop( sal_Bool bNew ) { bTop = bNew; }
    inline void SetTab1( sal_Bool bNew ) { bTab1 = bNew; }
    inline void SetTab2( sal_Bool bNew ) { bTab2 = bNew; }
    inline void SetDirection( sal_uInt8 nNew ) { nDirection = nNew; }
    inline sal_Bool GetTab1() const { return bTab1; }
    inline sal_Bool GetTab2() const { return bTab2; }
public:
    ~SwMultiPortion();
    const SwLineLayout& GetRoot() const { return aRoot; }
    SwLineLayout& GetRoot() { return aRoot; }
    SwFldPortion* GetFldRest() { return pFldRest; }
    void SetFldRest( SwFldPortion* pNew ) { pFldRest = pNew; }

    inline sal_Bool HasTabulator() const { return bTab1 || bTab2; }
    inline sal_Bool IsFormatted() const { return bFormatted; }
    inline void SetFormatted() { bFormatted = sal_True; }
    inline sal_Bool IsFollowFld() const { return bFollowFld; }
    inline void SetFollowFld() { bFollowFld = sal_True; }
    inline sal_Bool HasFlyInCntnt() const { return bFlyInCntnt; }
    inline void SetFlyInCntnt( sal_Bool bNew ) { bFlyInCntnt = bNew; }
    inline sal_Bool IsDouble() const { return bDouble; }
    inline bool IsRuby() const { return bRuby; }
    inline sal_Bool IsBidi() const { return bBidi; }
    inline sal_Bool OnTop() const { return bTop; }
    void ActualizeTabulator();

    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual long CalcSpacing( long nSpaceAdd, const SwTxtSizeInfo &rInf ) const;
    virtual sal_Bool ChgSpaceAdd( SwLineLayout* pCurr, long nSpaceAdd ) const;

    // Summarize the internal lines to calculate the (external) size
    void CalcSize( SwTxtFormatter& rLine, SwTxtFormatInfo &rInf );

    inline bool HasBrackets() const;
    inline bool HasRotation() const { return 0 != (1 & nDirection); }
    inline bool IsRevers() const { return 0 != (2 & nDirection); }
    inline sal_uInt8 GetDirection() const { return nDirection; }
    inline sal_uInt16 GetFontRotation() const
        { return ( HasRotation() ? ( IsRevers() ? 2700 : 900 ) : 0 ); }

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

class SwDoubleLinePortion : public SwMultiPortion
{
    SwBracket* pBracket;    // Surrounding brackets
    SwTwips nLineDiff;      // Difference of the width of the both lines
    xub_StrLen nBlank1;     // Number of blanks in the first line
    xub_StrLen nBlank2;     // Number of blanks in the second line
public:
    SwDoubleLinePortion( SwDoubleLinePortion& rDouble, xub_StrLen nEnd );
    SwDoubleLinePortion( const SwMultiCreator& rCreate, xub_StrLen nEnd );
    ~SwDoubleLinePortion();

    inline SwBracket* GetBrackets() const { return pBracket; }
    void SetBrackets( const SwDoubleLinePortion& rDouble );
    void PaintBracket( SwTxtPaintInfo& rInf, long nSpaceAdd, sal_Bool bOpen ) const;
    void FormatBrackets( SwTxtFormatInfo &rInf, SwTwips& nMaxWidth );
    inline KSHORT PreWidth() const { return pBracket->nPreWidth; };
    inline KSHORT PostWidth() const { return pBracket->nPostWidth; }
    inline void ClearBrackets()
        { pBracket->nPreWidth = pBracket->nPostWidth=0; Width( 0 ); }
    inline KSHORT BracketWidth(){ return PreWidth() + PostWidth(); }

    void CalcBlanks( SwTxtFormatInfo &rInf );
    static void ResetSpaceAdd( SwLineLayout* pCurr );
    inline SwTwips GetLineDiff() const { return nLineDiff; }
    inline xub_StrLen GetSpaceCnt() const
        { return ( nLineDiff < 0 ) ? nBlank2 : nBlank1; }
    inline xub_StrLen GetSmallerSpaceCnt() const
        { return ( nLineDiff < 0 ) ? nBlank1 : nBlank2; }
    inline xub_StrLen GetBlank1() const { return nBlank1; }
    inline xub_StrLen GetBlank2() const { return nBlank2; }

    virtual long CalcSpacing( long nSpaceAdd, const SwTxtSizeInfo &rInf ) const;
    virtual sal_Bool ChgSpaceAdd( SwLineLayout* pCurr, long nSpaceAdd ) const;
};

class SwRubyPortion : public SwMultiPortion
{
    xub_StrLen nRubyOffset;
    sal_uInt16 nAdjustment;
    void _Adjust( SwTxtFormatInfo &rInf);
public:
    SwRubyPortion( const SwRubyPortion& rRuby, xub_StrLen nEnd );

    SwRubyPortion( const SwMultiCreator& rCreate, const SwFont& rFnt,
                   const IDocumentSettingAccess& rIDocumentSettingAccess,
                   xub_StrLen nEnd, xub_StrLen nOffs,
                   const sal_Bool* pForceRubyPos );

    void CalcRubyOffset();
    inline void Adjust( SwTxtFormatInfo &rInf )
        { if(nAdjustment && GetRoot().GetNext()) _Adjust(rInf); }
    inline sal_uInt16 GetAdjustment() const { return nAdjustment; }
    inline xub_StrLen GetRubyOffset() const { return nRubyOffset; }
};

class SwRotatedPortion : public SwMultiPortion
{
public:
    SwRotatedPortion( xub_StrLen nEnd, sal_uInt8 nDir = 1 )
        : SwMultiPortion( nEnd ) { SetDirection( nDir ); }
    SwRotatedPortion( const SwMultiCreator& rCreate, xub_StrLen nEnd,
                      sal_Bool bRTL );
};

class SwBidiPortion : public SwMultiPortion
{
    sal_uInt8 nLevel;

public:
    SwBidiPortion( xub_StrLen nEnd, sal_uInt8 nLv );

    inline sal_uInt8 GetLevel() const { return nLevel; }
    // Get number of blanks for justified alignment
    xub_StrLen GetSpaceCnt( const SwTxtSizeInfo &rInf ) const;
    // Calculates extra spacing based on number of blanks
    virtual long CalcSpacing( long nSpaceAdd, const SwTxtSizeInfo &rInf ) const;
    // Manipulate the spacing array at pCurr
    virtual sal_Bool ChgSpaceAdd( SwLineLayout* pCurr, long nSpaceAdd ) const;
};

// For cursor travelling in multiportions

class SwTxtCursorSave
{
    SwTxtCursor* pTxtCrsr;
    SwLineLayout* pCurr;
    SwTwips nWidth;
    xub_StrLen nStart;
    sal_uInt8 nOldProp;
    sal_Bool bSpaceChg;
public:
    SwTxtCursorSave( SwTxtCursor* pTxtCursor, SwMultiPortion* pMulti,
        SwTwips nY, sal_uInt16& nX, xub_StrLen nCurrStart, long nSpaceAdd );
    ~SwTxtCursorSave();
};

/*************************************************************************
 *                  inline - Implementations
 *************************************************************************/

inline bool SwMultiPortion::HasBrackets() const
{
    return IsDouble() ? 0 != ((SwDoubleLinePortion*)this)->GetBrackets() : false;
}

CLASSIO( SwMultiPortion )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
