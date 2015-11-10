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

#include "porlay.hxx"
#include "porexp.hxx"

class SwTextFormatInfo;
class SwFieldPortion;
class SwTextCursor;
class SwLineLayout;
class SwTextPaintInfo;
class SwTextAttr;
class SfxPoolItem;
class SwFont;

// SwMultiCreator is a small structure to create a multiportion.
// It contains the kind of multiportion and a textattribute
// or a poolitem.
// The GetMultiCreator-function fills this structure and
// the Ctor of the SwMultiPortion uses it.
#define SW_MC_DOUBLE    0
#define SW_MC_RUBY      1
#define SW_MC_ROTATE    2
#define SW_MC_BIDI      3

struct SwMultiCreator
{
    const SwTextAttr* pAttr;
    const SfxPoolItem* pItem;
    sal_uInt8 nId;
    sal_uInt8 nLevel;
};

// A two-line-portion (SwMultiPortion) could have surrounding brackets,
// in this case the structure SwBracket will be used.
struct SwBracket
{
    sal_Int32 nStart;      // Start of text attribute determins the font
    sal_uInt16 nAscent;         // Ascent of the brackets
    sal_uInt16 nHeight;         // Height of them
    sal_uInt16 nPreWidth;       // Width of the opening bracket
    sal_uInt16 nPostWidth;      // Width of the closing bracket
    sal_Unicode cPre;       // Initial character, e.g. '('
    sal_Unicode cPost;      // Final character, e.g. ')'
    sal_uInt8 nPreScript;       // Script of the initial character
    sal_uInt8 nPostScript;       // Script of the final character
};

// The SwMultiPortion is line portion inside a line portion,
// it's a group of portions,
// e.g. a double line portion in a line
// or phonetics (ruby)
// or combined characters
// or a rotated portion.
class SwMultiPortion : public SwLinePortion
{
    SwLineLayout aRoot;     // One or more lines
    SwFieldPortion *pFieldRest; // Field rest from the previous line
    bool bTab1      :1;     // First line tabulator
    bool bTab2      :1;     // Second line includes tabulator
    bool bDouble    :1;     // Double line
    bool bRuby      :1;     // Phonetics
    bool bBidi      :1;
    bool bTop       :1;     // Phonetic position
    bool bFormatted :1;     // Already formatted
    bool bFollowField :1;     // Field follow inside
    bool bFlyInContent:1;     // Fly as character inside
    sal_uInt8 nDirection:2; // Direction (0/90/180/270 degrees)
protected:
    explicit SwMultiPortion(sal_Int32 nEnd)
        : pFieldRest(nullptr)
        , bTab1(false)
        , bTab2(false)
        , bDouble(false)
        , bRuby(false)
        , bBidi(false)
        , bTop(false)
        , bFormatted(false)
        , bFollowField(false)
        , bFlyInContent(false)
        , nDirection(0)
    {
        SetWhichPor(POR_MULTI);
        SetLen(nEnd);
    }
    void SetDouble() { bDouble = true; }
    void SetRuby() { bRuby = true; }
    void SetBidi() { bBidi = true; }
    void SetTop( bool bNew ) { bTop = bNew; }
    void SetTab1( bool bNew ) { bTab1 = bNew; }
    void SetTab2( bool bNew ) { bTab2 = bNew; }
    void SetDirection( sal_uInt8 nNew ) { nDirection = nNew; }
    bool GetTab1() const { return bTab1; }
    bool GetTab2() const { return bTab2; }
public:
    virtual ~SwMultiPortion();
    const SwLineLayout& GetRoot() const { return aRoot; }
    SwLineLayout& GetRoot() { return aRoot; }

    inline bool HasTabulator() const { return bTab1 || bTab2; }
    inline bool IsFormatted() const { return bFormatted; }
    inline void SetFormatted() { bFormatted = true; }
    inline bool IsFollowField() const { return bFollowField; }
    inline void SetFollowField() { bFollowField = true; }
    inline bool HasFlyInContent() const { return bFlyInContent; }
    inline void SetFlyInContent( bool bNew ) { bFlyInContent = bNew; }
    inline bool IsDouble() const { return bDouble; }
    inline bool IsRuby() const { return bRuby; }
    inline bool IsBidi() const { return bBidi; }
    inline bool OnTop() const { return bTop; }
    void ActualizeTabulator();

    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual long CalcSpacing( long nSpaceAdd, const SwTextSizeInfo &rInf ) const override;
    virtual bool ChgSpaceAdd( SwLineLayout* pCurr, long nSpaceAdd ) const;

    // Summarize the internal lines to calculate the (external) size
    void CalcSize( SwTextFormatter& rLine, SwTextFormatInfo &rInf );

    inline bool HasBrackets() const;
    inline bool HasRotation() const { return 0 != (1 & nDirection); }
    inline bool IsRevers() const { return 0 != (2 & nDirection); }
    inline sal_uInt8 GetDirection() const { return nDirection; }

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;

    OUTPUT_OPERATOR_OVERRIDE
};

class SwDoubleLinePortion : public SwMultiPortion
{
    SwBracket* pBracket;    // Surrounding brackets
    SwTwips nLineDiff;      // Difference of the width of the both lines
    sal_Int32 nBlank1;     // Number of blanks in the first line
    sal_Int32 nBlank2;     // Number of blanks in the second line
public:
    SwDoubleLinePortion( SwDoubleLinePortion& rDouble, sal_Int32 nEnd );
    SwDoubleLinePortion( const SwMultiCreator& rCreate, sal_Int32 nEnd );
    virtual ~SwDoubleLinePortion();

    inline SwBracket* GetBrackets() const { return pBracket; }
    void SetBrackets( const SwDoubleLinePortion& rDouble );
    void PaintBracket( SwTextPaintInfo& rInf, long nSpaceAdd, bool bOpen ) const;
    void FormatBrackets( SwTextFormatInfo &rInf, SwTwips& nMaxWidth );
    inline sal_uInt16 PreWidth() const { return pBracket->nPreWidth; };
    inline sal_uInt16 PostWidth() const { return pBracket->nPostWidth; }
    inline void ClearBrackets()
        { pBracket->nPreWidth = pBracket->nPostWidth=0; Width( 0 ); }
    inline sal_uInt16 BracketWidth(){ return PreWidth() + PostWidth(); }

    void CalcBlanks( SwTextFormatInfo &rInf );
    static void ResetSpaceAdd( SwLineLayout* pCurr );
    inline SwTwips GetLineDiff() const { return nLineDiff; }
    inline sal_Int32 GetSpaceCnt() const
        { return ( nLineDiff < 0 ) ? nBlank2 : nBlank1; }
    inline sal_Int32 GetSmallerSpaceCnt() const
        { return ( nLineDiff < 0 ) ? nBlank1 : nBlank2; }

    virtual long CalcSpacing( long nSpaceAdd, const SwTextSizeInfo &rInf ) const override;
    virtual bool ChgSpaceAdd( SwLineLayout* pCurr, long nSpaceAdd ) const override;
};

class SwRubyPortion : public SwMultiPortion
{
    sal_Int32 nRubyOffset;
    sal_uInt16 nAdjustment;
    void _Adjust( SwTextFormatInfo &rInf);
public:
    SwRubyPortion( const SwRubyPortion& rRuby, sal_Int32 nEnd );

    SwRubyPortion( const SwMultiCreator& rCreate, const SwFont& rFnt,
                   const IDocumentSettingAccess& rIDocumentSettingAccess,
                   sal_Int32 nEnd, sal_Int32 nOffs,
                   const bool* pForceRubyPos );

    void CalcRubyOffset();
    inline void Adjust( SwTextFormatInfo &rInf )
        { if(nAdjustment && GetRoot().GetNext()) _Adjust(rInf); }
    inline sal_uInt16 GetAdjustment() const { return nAdjustment; }
    inline sal_Int32 GetRubyOffset() const { return nRubyOffset; }
};

class SwRotatedPortion : public SwMultiPortion
{
public:
    SwRotatedPortion( sal_Int32 nEnd, sal_uInt8 nDir = 1 )
        : SwMultiPortion( nEnd ) { SetDirection( nDir ); }
    SwRotatedPortion( const SwMultiCreator& rCreate, sal_Int32 nEnd,
                      bool bRTL );
};

class SwBidiPortion : public SwMultiPortion
{
    sal_uInt8 nLevel;

public:
    SwBidiPortion( sal_Int32 nEnd, sal_uInt8 nLv );

    inline sal_uInt8 GetLevel() const { return nLevel; }
    // Get number of blanks for justified alignment
    sal_Int32 GetSpaceCnt( const SwTextSizeInfo &rInf ) const;
    // Calculates extra spacing based on number of blanks
    virtual long CalcSpacing( long nSpaceAdd, const SwTextSizeInfo &rInf ) const override;
    // Manipulate the spacing array at pCurr
    virtual bool ChgSpaceAdd( SwLineLayout* pCurr, long nSpaceAdd ) const override;
};

// For cursor travelling in multiportions

class SwTextCursorSave
{
    SwTextCursor* pTextCrsr;
    SwLineLayout* pCurr;
    sal_Int32 nStart;
    sal_uInt16 nWidth;
    sal_uInt8 nOldProp;
    bool bSpaceChg;
public:
    SwTextCursorSave( SwTextCursor* pTextCursor, SwMultiPortion* pMulti,
        SwTwips nY, sal_uInt16& nX, sal_Int32 nCurrStart, long nSpaceAdd );
    ~SwTextCursorSave();
};

inline bool SwMultiPortion::HasBrackets() const
{
    return IsDouble() && nullptr != static_cast<const SwDoubleLinePortion*>(this)->GetBrackets();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
