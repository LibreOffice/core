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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DRAWFONT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DRAWFONT_HXX

#include <tools/solar.h>
#include <osl/diagnose.h>
#include <vcl/vclptr.hxx>
#include <vcl/outdev.hxx>

class SwTextFrm;
class SwViewShell;
class SwScriptInfo;
class Point;
class SwWrongList;
class Size;
class SwFont;
namespace vcl {
    class Font;
    class TextLayoutCache;
    typedef OutputDevice RenderContext;
}
class SwUnderlineFont;

// encapsulates information for drawing text
class SwDrawTextInfo
{
    const SwTextFrm* pFrm;
    VclPtr<OutputDevice> pOut;
    SwViewShell const * pSh;
    const SwScriptInfo* pScriptInfo;
    Point m_aPos;
    vcl::TextLayoutCache const* m_pCachedVclData;
    OUString m_aText;
    const SwWrongList* pWrong;
    const SwWrongList* pGrammarCheck;
    const SwWrongList* pSmartTags;
    Size m_aSize;
    SwFont *pFnt;
    SwUnderlineFont* pUnderFnt;
    sal_Int32* pHyphPos;
    long nLeft;
    long nRight;
    long nKanaDiff;
    sal_Int32 nIdx;
    sal_Int32 nLen;
    sal_Int32 nOfst;
    sal_uInt16 nWidth;
    sal_uInt16 nAscent;
    sal_uInt16 nCompress;
    long nSperren;
    long nSpace;
    long nKern;
    sal_Int32 nNumberOfBlanks;
    sal_uInt8 nCursorBidiLevel;
    bool bBullet : 1;
    bool bUpper : 1;        // for small caps: upper case flag
    bool bDrawSpace : 1;    // for small caps: underline/ line through
    bool bGreyWave  : 1;    // grey wave line for extended text input
    // For underlining we need to know, if a section is right in front of a
    // whole block or a fix margin section.
    bool bSpaceStop : 1;
    bool bSnapToGrid : 1;   // Does paragraph snap to grid?
    // Paint text as if text has LTR direction, used for line numbering
    bool bIgnoreFrmRTL : 1;
    // GetCrsrOfst should not return the next position if screen position is
    // inside second half of bound rect, used for Accessibility
    bool bPosMatchesBounds :1;

    SwDrawTextInfo();          // prohibited
public:

#ifdef DBG_UTIL
    // These flags should control that the appropriate Set-function has been
    // called before calling the Get-function of a member
    bool m_bPos   : 1;
    bool m_bWrong : 1;
    bool m_bGrammarCheck : 1;
    bool m_bSize  : 1;
    bool m_bFnt   : 1;
    bool m_bHyph  : 1;
    bool m_bLeft  : 1;
    bool m_bRight : 1;
    bool m_bKana  : 1;
    bool m_bOfst  : 1;
    bool m_bAscent: 1;
    bool m_bSperr : 1;
    bool m_bSpace : 1;
    bool m_bNumberOfBlanks : 1;
    bool m_bUppr  : 1;
    bool m_bDrawSp: 1;
#endif

    SwDrawTextInfo( SwViewShell const *pS, OutputDevice &rO, const SwScriptInfo* pSI,
                    const OUString &rSt, sal_Int32 nI, sal_Int32 nL,
                    sal_uInt16 nW = 0, bool bB = false,
                    vcl::TextLayoutCache const*const pCachedVclData = nullptr)
        : m_pCachedVclData(pCachedVclData)
    {
        pFrm = NULL;
        pSh = pS;
        pOut = &rO;
        pScriptInfo = pSI;
        m_aText = rSt;
        nIdx = nI;
        nLen = nL;
        nKern = 0;
        nCompress = 0;
        nWidth = nW;
        nNumberOfBlanks = 0;
        nCursorBidiLevel = 0;
        bBullet = bB;
        pUnderFnt = 0;
        bGreyWave = false;
        bSpaceStop = false;
        bSnapToGrid = false;
        bIgnoreFrmRTL = false;
        bPosMatchesBounds = false;

        // These values are initialized but have to be set explicitly via their
        // Set-function before they may be accessed by their Get-function:
        pWrong = 0;
        pGrammarCheck = 0;
        pSmartTags = 0;
        pFnt = 0;
        pHyphPos = 0;
        nLeft = 0;
        nRight = 0;
        nKanaDiff = 0;
        nOfst = 0;
        nAscent = 0;
        nSperren = 0;
        nSpace = 0;
        bUpper = false;
        bDrawSpace = false;

#ifdef DBG_UTIL
        // these flags control whether the matching member variables have been
        // set by using the Set-function before they may be accessed by their
        // Get-function:
        m_bPos = m_bWrong = m_bGrammarCheck = m_bSize = m_bFnt = m_bAscent =
        m_bSpace = m_bNumberOfBlanks = m_bUppr =
        m_bDrawSp = m_bLeft = m_bRight = m_bKana = m_bOfst = m_bHyph =
        m_bSperr = false;
#endif
    }

    const SwTextFrm* GetFrm() const
    {
        return pFrm;
    }

    void SetFrm( const SwTextFrm* pNewFrm )
    {
        pFrm = pNewFrm;
    }

    SwViewShell const *GetShell() const
    {
        return pSh;
    }

    vcl::RenderContext& GetOut() const
    {
        return *pOut;
    }

    vcl::RenderContext *GetpOut() const
    {
        return pOut;
    }

    const SwScriptInfo* GetScriptInfo() const
    {
        return pScriptInfo;
    }

    const Point &GetPos() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bPos, "DrawTextInfo: Undefined Position" );
#endif
        return m_aPos;
    }

    sal_Int32 *GetHyphPos() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bHyph, "DrawTextInfo: Undefined Hyph Position" );
#endif
        return pHyphPos;
    }

    vcl::TextLayoutCache const* GetVclCache() const
    {
        return m_pCachedVclData;
    }

    const OUString &GetText() const
    {
        return m_aText;
    }

    const SwWrongList* GetWrong() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bWrong, "DrawTextInfo: Undefined WrongList" );
#endif
        return pWrong;
    }

    const SwWrongList* GetGrammarCheck() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bGrammarCheck, "DrawTextInfo: Undefined GrammarCheck List" );
#endif
        return pGrammarCheck;
    }

    const SwWrongList* GetSmartTags() const
    {
        return pSmartTags;
    }

    const Size &GetSize() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bSize, "DrawTextInfo: Undefined Size" );
#endif
        return m_aSize;
    }

    SwFont* GetFont() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bFnt, "DrawTextInfo: Undefined Font" );
#endif
        return pFnt;
    }

    SwUnderlineFont* GetUnderFnt() const
    {
        return pUnderFnt;
    }

    sal_Int32 GetIdx() const
    {
        return nIdx;
    }

    sal_Int32 GetLen() const
    {
        return nLen;
    }

    sal_Int32 GetOfst() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bOfst, "DrawTextInfo: Undefined Offset" );
#endif
        return nOfst;
    }

    sal_Int32 GetEnd() const
    {
        return nIdx + nLen;
    }

    long GetLeft() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bLeft, "DrawTextInfo: Undefined left range" );
#endif
        return nLeft;
    }

    long GetRight() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bRight, "DrawTextInfo: Undefined right range" );
#endif
        return nRight;
    }

    long GetKanaDiff() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bKana, "DrawTextInfo: Undefined kana difference" );
#endif
        return nKanaDiff;
    }

    sal_uInt16 GetWidth() const
    {
        return nWidth;
    }

    sal_uInt16 GetAscent() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bAscent, "DrawTextInfo: Undefined Ascent" );
#endif
        return nAscent;
    }

    sal_uInt16 GetKanaComp() const
    {
        return nCompress;
    }

    long GetSperren() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bSperr, "DrawTextInfo: Undefined >Sperren<" );
#endif
        return nSperren;
    }

    long GetKern() const
    {
        return nKern;
    }

    long GetSpace() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bSpace, "DrawTextInfo: Undefined Spacing" );
#endif
        return nSpace;
    }

    sal_Int32 GetNumberOfBlanks() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bNumberOfBlanks, "DrawTextInfo::Undefined NumberOfBlanks" );
#endif
        return nNumberOfBlanks;
    }

    sal_uInt8 GetCursorBidiLevel() const
    {
        return nCursorBidiLevel;
    }

    bool GetBullet() const
    {
        return bBullet;
    }

    bool GetUpper() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bUppr, "DrawTextInfo: Undefined Upperflag" );
#endif
        return bUpper;
    }

    bool GetDrawSpace() const
    {
#ifdef DBG_UTIL
        OSL_ENSURE( m_bDrawSp, "DrawTextInfo: Undefined DrawSpaceflag" );
#endif
        return bDrawSpace;
    }

    bool GetGreyWave() const
    {
        return bGreyWave;
    }

    bool IsSpaceStop() const
    {
        return bSpaceStop;
    }

    bool SnapToGrid() const
    {
        return bSnapToGrid;
    }

    bool IsIgnoreFrmRTL() const
    {
        return bIgnoreFrmRTL;
    }

    bool IsPosMatchesBounds() const
    {
        return bPosMatchesBounds;
    }

    void SetOut( OutputDevice &rNew )
    {
        pOut = &rNew;
    }

    void SetPos( const Point &rNew )
    {
        m_aPos = rNew;
#ifdef DBG_UTIL
        m_bPos = true;
#endif
    }

    void SetHyphPos( sal_Int32 *pNew )
    {
        pHyphPos = pNew;
#ifdef DBG_UTIL
        m_bHyph = true;
#endif
    }

    void SetText( const OUString &rNew )
    {
        m_aText = rNew;
        m_pCachedVclData = nullptr; // would any case benefit from save/restore?
    }

    void SetWrong( const SwWrongList* pNew )
    {
        pWrong = pNew;
#ifdef DBG_UTIL
        m_bWrong = true;
#endif
    }

    void SetGrammarCheck( const SwWrongList* pNew )
    {
        pGrammarCheck = pNew;
#ifdef DBG_UTIL
        m_bGrammarCheck = true;
#endif
    }

    void SetSmartTags( const SwWrongList* pNew )
    {
        pSmartTags = pNew;
    }

    void SetSize( const Size &rNew )
    {
        m_aSize = rNew;
#ifdef DBG_UTIL
        m_bSize = true;
#endif
    }

    void SetFont( SwFont* pNew )
    {
        pFnt = pNew;
#ifdef DBG_UTIL
        m_bFnt = true;
#endif
    }

    void SetIdx( sal_Int32 nNew )
    {
        nIdx = nNew;
    }

    void SetLen( sal_Int32 nNew )
    {
        nLen = nNew;
    }

    void SetOfst( sal_Int32 nNew )
    {
        nOfst = nNew;
#ifdef DBG_UTIL
        m_bOfst = true;
#endif
    }

    void SetLeft( long nNew )
    {
        nLeft = nNew;
#ifdef DBG_UTIL
        m_bLeft = true;
#endif
    }

    void SetRight( long nNew )
    {
        nRight = nNew;
#ifdef DBG_UTIL
        m_bRight = true;
#endif
    }

    void SetKanaDiff( long nNew )
    {
        nKanaDiff = nNew;
#ifdef DBG_UTIL
        m_bKana = true;
#endif
    }

    void SetWidth( sal_uInt16 nNew )
    {
        nWidth = nNew;
    }

    void SetAscent( sal_uInt16 nNew )
    {
        nAscent = nNew;
#ifdef DBG_UTIL
        m_bAscent = true;
#endif
    }

    void SetKern( long nNew )
    {
        nKern = nNew;
    }

    void SetSpace( long nNew )
    {
        if( nNew < 0 )
        {
            nSperren = -nNew;
            nSpace = 0;
        }
        else
        {
            nSpace = nNew;
            nSperren = 0;
        }
#ifdef DBG_UTIL
        m_bSpace = true;
        m_bSperr = true;
#endif
    }

    void SetNumberOfBlanks( sal_Int32 nNew )
    {
#ifdef DBG_UTIL
        m_bNumberOfBlanks = true;
#endif
        nNumberOfBlanks = nNew;
    }

    void SetCursorBidiLevel( sal_uInt8 nNew )
    {
        nCursorBidiLevel = nNew;
    }

    void SetKanaComp( short nNew )
    {
        nCompress = nNew;
    }

    void SetBullet( bool bNew )
    {
        bBullet = bNew;
    }

    void SetUnderFnt( SwUnderlineFont* pULFnt )
    {
        pUnderFnt = pULFnt;
    }

    void SetUpper( bool bNew )
    {
        bUpper = bNew;
#ifdef DBG_UTIL
        m_bUppr = true;
#endif
    }

    void SetDrawSpace( bool bNew )
    {
        bDrawSpace = bNew;
#ifdef DBG_UTIL
        m_bDrawSp = true;
#endif
    }

    void SetGreyWave( bool bNew )
    {
        bGreyWave = bNew;
    }

    void SetSpaceStop( bool bNew )
    {
        bSpaceStop = bNew;
    }

    void SetSnapToGrid( bool bNew )
    {
        bSnapToGrid = bNew;
    }

    void SetIgnoreFrmRTL( bool bNew )
    {
        bIgnoreFrmRTL = bNew;
    }

    void SetPosMatchesBounds( bool bNew )
    {
        bPosMatchesBounds = bNew;
    }

    void Shift( sal_uInt16 nDir );

    // sets a new color at the output device if necessary if a font is passed
    // as argument, the change if made to the font otherwise the font at the
    // output device is changed returns if the font has been changed
    bool ApplyAutoColor( vcl::Font* pFnt = 0 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
