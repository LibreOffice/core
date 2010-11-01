/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _DRAWFONT_HXX
#define _DRAWFONT_HXX

#include <tools/solar.h>
#include <tools/string.hxx>
#include <errhdl.hxx>

class SwTxtFrm;
class OutputDevice;
class ViewShell;
class SwScriptInfo;
class Point;
class SwWrongList;
class Size;
class SwFont;
class Font;
class SwUnderlineFont;

/*************************************************************************
 *                      class SwDrawTextInfo
 *
 * encapsulates information for drawing text
 *************************************************************************/

class SwDrawTextInfo
{
    const SwTxtFrm* pFrm;
    OutputDevice* pOut;
    ViewShell* pSh;
    const SwScriptInfo* pScriptInfo;
    const Point* pPos;
    const XubString* pText;
    const SwWrongList* pWrong;
    const SwWrongList* pGrammarCheck;
    const SwWrongList* pSmartTags; // SMARTTAGS
    const Size* pSize;
    SwFont *pFnt;
    SwUnderlineFont* pUnderFnt;
    xub_StrLen* pHyphPos;
    long nLeft;
    long nRight;
    long nKanaDiff;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    xub_StrLen nOfst;
    USHORT nWidth;
    USHORT nAscent;
    USHORT nCompress;
    long nSperren;
    long nSpace;
    long nKern;
    xub_StrLen nNumberOfBlanks;
    BYTE nCursorBidiLevel;
    BOOL bBullet : 1;
    BOOL bUpper : 1;        // Fuer Kapitaelchen: Grossbuchstaben-Flag
    BOOL bDrawSpace : 1;    // Fuer Kapitaelchen: Unter/Durchstreichung
    BOOL bGreyWave  : 1;    // Graue Wellenlinie beim extended TextInput
    BOOL bSpaceStop : 1;    // For underlining we need to know, if a portion
                            // is right in front of a hole portion or a
                            // fix margin portion.
    BOOL bSnapToGrid : 1;   // Does paragraph snap to grid?
    BOOL bIgnoreFrmRTL : 1; // Paint text as if text has LTR direction, used for
                            // line numbering
    BOOL bPosMatchesBounds :1;  // GetCrsrOfst should not return the next
                                // position if screen position is inside second
                                // half of bound rect, used for Accessibility

    SwDrawTextInfo();       // nicht zulaessig
public:

#ifdef DBG_UTIL
    BOOL bPos   : 1;            // These flags should control, that the appropriate
    BOOL bWrong : 1;            // Set-function has been called before calling
    BOOL bGrammarCheck : 1;     //  the Get-function of a member
    BOOL bSize  : 1;
    BOOL bFnt   : 1;
    BOOL bHyph  : 1;
    BOOL bLeft  : 1;
    BOOL bRight : 1;
    BOOL bKana  : 1;
    BOOL bOfst  : 1;
    BOOL bAscent: 1;
    BOOL bSperr : 1;
    BOOL bSpace : 1;
    BOOL bNumberOfBlanks : 1;
    BOOL bUppr  : 1;
    BOOL bDrawSp: 1;
#endif

    SwDrawTextInfo( ViewShell *pS, OutputDevice &rO, const SwScriptInfo* pSI,
                    const XubString &rSt, xub_StrLen nI, xub_StrLen nL,
                    USHORT nW = 0, BOOL bB = FALSE )
    {
        pFrm = NULL;
        pSh = pS;
        pOut = &rO;
        pScriptInfo = pSI;
        pText = &rSt;
        nIdx = nI;
        nLen = nL;
        nKern = 0;
        nCompress = 0;
        nWidth = nW;
        nNumberOfBlanks = 0;
        nCursorBidiLevel = 0;
        bBullet = bB;
        pUnderFnt = 0;
        bGreyWave = FALSE;
        bSpaceStop = FALSE;
        bSnapToGrid = FALSE;
        bIgnoreFrmRTL = FALSE;
        bPosMatchesBounds = FALSE;

        // These values are initialized but, they have to be
        // set explicitly via their Set-function before they may
        // be accessed by their Get-function:
        pPos = 0;
        pWrong = 0;
        pGrammarCheck = 0;
        pSmartTags = 0;
        pSize = 0;
        pFnt = 0;
        pHyphPos = 0;
        nLeft = 0;
        nRight = 0;
        nKanaDiff = 0;
        nOfst = 0;
        nAscent = 0;
        nSperren = 0;
        nSpace = 0;
        bUpper = FALSE;
        bDrawSpace = FALSE;

#ifdef DBG_UTIL
        // these flags control, whether the matching member variables have
        // been set by using the Set-function before they may be accessed
        // by their Get-function:
        bPos = bWrong = bGrammarCheck = bSize = bFnt = bAscent = bSpace = bNumberOfBlanks = bUppr =
        bDrawSp = bLeft = bRight = bKana = bOfst = bHyph = bSperr = FALSE;
#endif
    }

    const SwTxtFrm* GetFrm() const
    {
        return pFrm;
    }

    void SetFrm( const SwTxtFrm* pNewFrm )
    {
        pFrm = pNewFrm;
    }

    ViewShell *GetShell() const
    {
        return pSh;
    }

    OutputDevice& GetOut() const
    {
        return *pOut;
    }

    OutputDevice *GetpOut() const
    {
        return pOut;
    }

    const SwScriptInfo* GetScriptInfo() const
    {
        return pScriptInfo;
    }

    const Point &GetPos() const
    {
        ASSERT( bPos, "DrawTextInfo: Undefined Position" );
        return *pPos;
    }

    xub_StrLen *GetHyphPos() const
    {
        ASSERT( bHyph, "DrawTextInfo: Undefined Hyph Position" );
        return pHyphPos;
    }

    const XubString &GetText() const
    {
        return *pText;
    }

    const SwWrongList* GetWrong() const
    {
        ASSERT( bWrong, "DrawTextInfo: Undefined WrongList" );
        return pWrong;
    }

    const SwWrongList* GetGrammarCheck() const
    {
        ASSERT( bGrammarCheck, "DrawTextInfo: Undefined GrammarCheck List" );
        return pGrammarCheck;
    }

    const SwWrongList* GetSmartTags() const
    {
        return pSmartTags;
    }

    const Size &GetSize() const
    {
        ASSERT( bSize, "DrawTextInfo: Undefined Size" );
        return *pSize;
    }

    SwFont* GetFont() const
    {
        ASSERT( bFnt, "DrawTextInfo: Undefined Font" );
        return pFnt;
    }

    SwUnderlineFont* GetUnderFnt() const
    {
        return pUnderFnt;
    }

    xub_StrLen GetIdx() const
    {
        return nIdx;
    }

    xub_StrLen GetLen() const
    {
        return nLen;
    }

    xub_StrLen GetOfst() const
    {
        ASSERT( bOfst, "DrawTextInfo: Undefined Offset" );
        return nOfst;
    }

    xub_StrLen GetEnd() const
    {
        return nIdx + nLen;
    }

    long GetLeft() const
    {
        ASSERT( bLeft, "DrawTextInfo: Undefined left range" );
        return nLeft;
    }

    long GetRight() const
    {
        ASSERT( bRight, "DrawTextInfo: Undefined right range" );
        return nRight;
    }

    long GetKanaDiff() const
    {
        ASSERT( bKana, "DrawTextInfo: Undefined kana difference" );
        return nKanaDiff;
    }

    USHORT GetWidth() const
    {
        return nWidth;
    }

    USHORT GetAscent() const
    {
        ASSERT( bAscent, "DrawTextInfo: Undefined Ascent" );
        return nAscent;
    }

    USHORT GetKanaComp() const
    {
        return nCompress;
    }

    long GetSperren() const
    {
        ASSERT( bSperr, "DrawTextInfo: Undefined >Sperren<" );
        return nSperren;
    }

    long GetKern() const
    {
        return nKern;
    }

    long GetSpace() const
    {
        ASSERT( bSpace, "DrawTextInfo: Undefined Spacing" );
        return nSpace;
    }

    xub_StrLen GetNumberOfBlanks() const
    {
        ASSERT( bNumberOfBlanks, "DrawTextInfo::Undefined NumberOfBlanks" );
        return nNumberOfBlanks;
    }

    BYTE GetCursorBidiLevel() const
    {
        return nCursorBidiLevel;
    }

    BOOL GetBullet() const
    {
        return bBullet;
    }

    BOOL GetUpper() const
    {
        ASSERT( bUppr, "DrawTextInfo: Undefined Upperflag" );
        return bUpper;
    }

    BOOL GetDrawSpace() const
    {
        ASSERT( bDrawSp, "DrawTextInfo: Undefined DrawSpaceflag" );
        return bDrawSpace;
    }

    BOOL GetGreyWave() const
    {
        return bGreyWave;
    }

    BOOL IsSpaceStop() const
    {
        return bSpaceStop;
    }

    BOOL SnapToGrid() const
    {
        return bSnapToGrid;
    }

    BOOL IsIgnoreFrmRTL() const
    {
        return bIgnoreFrmRTL;
    }

    BOOL IsPosMatchesBounds() const
    {
        return bPosMatchesBounds;
    }

    void SetOut( OutputDevice &rNew )
    {
        pOut = &rNew;
    }

    void SetPos( const Point &rNew )
    {
        pPos = &rNew;
#ifdef DBG_UTIL
        bPos = TRUE;
#endif
    }

    void SetHyphPos( xub_StrLen *pNew )
    {
        pHyphPos = pNew;
#ifdef DBG_UTIL
        bHyph = TRUE;
#endif
    }

    void SetText( const XubString &rNew )
    {
        pText = &rNew;
    }

    void SetWrong( const SwWrongList* pNew )
    {
        pWrong = pNew;
#ifdef DBG_UTIL
        bWrong = TRUE;
#endif
    }

    void SetGrammarCheck( const SwWrongList* pNew )
    {
        pGrammarCheck = pNew;
#ifdef DBG_UTIL
        bGrammarCheck = TRUE;
#endif
    }

    void SetSmartTags( const SwWrongList* pNew )
    {
        pSmartTags = pNew;
    }

    void SetSize( const Size &rNew )
    {
        pSize = &rNew;
#ifdef DBG_UTIL
        bSize = TRUE;
#endif
    }

    void SetFont( SwFont* pNew )
    {
        pFnt = pNew;
#ifdef DBG_UTIL
        bFnt = TRUE;
#endif
    }

    void SetIdx( xub_StrLen nNew )
    {
        nIdx = nNew;
    }

    void SetLen( xub_StrLen nNew )
    {
        nLen = nNew;
    }

    void SetOfst( xub_StrLen nNew )
    {
        nOfst = nNew;
#ifdef DBG_UTIL
        bOfst = TRUE;
#endif
    }

    void SetLeft( long nNew )
    {
        nLeft = nNew;
#ifdef DBG_UTIL
        bLeft = TRUE;
#endif
    }

    void SetRight( long nNew )
    {
        nRight = nNew;
#ifdef DBG_UTIL
        bRight = TRUE;
#endif
    }

    void SetKanaDiff( long nNew )
    {
        nKanaDiff = nNew;
#ifdef DBG_UTIL
        bKana = TRUE;
#endif
    }

    void SetWidth( USHORT nNew )
    {
        nWidth = nNew;
    }

    void SetAscent( USHORT nNew )
    {
        nAscent = nNew;
#ifdef DBG_UTIL
        bAscent = TRUE;
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
        bSpace = TRUE;
        bSperr = TRUE;
#endif
    }

    void SetNumberOfBlanks( xub_StrLen nNew )
    {
#ifdef DBG_UTIL
        bNumberOfBlanks = TRUE;
#endif
        nNumberOfBlanks = nNew;
    }

    void SetCursorBidiLevel( BYTE nNew )
    {
        nCursorBidiLevel = nNew;
    }

    void SetKanaComp( short nNew )
    {
        nCompress = nNew;
    }

    void SetBullet( BOOL bNew )
    {
        bBullet = bNew;
    }

    void SetUnderFnt( SwUnderlineFont* pULFnt )
    {
        pUnderFnt = pULFnt;
    }

    void SetUpper( BOOL bNew )
    {
        bUpper = bNew;
#ifdef DBG_UTIL
        bUppr = TRUE;
#endif
    }

    void SetDrawSpace( BOOL bNew )
    {
        bDrawSpace = bNew;
#ifdef DBG_UTIL
        bDrawSp = TRUE;
#endif
    }

    void SetGreyWave( BOOL bNew )
    {
        bGreyWave = bNew;
    }

    void SetSpaceStop( BOOL bNew )
    {
        bSpaceStop = bNew;
    }

    void SetSnapToGrid( BOOL bNew )
    {
        bSnapToGrid = bNew;
    }

    void SetIgnoreFrmRTL( BOOL bNew )
    {
        bIgnoreFrmRTL = bNew;
    }

    void SetPosMatchesBounds( BOOL bNew )
    {
        bPosMatchesBounds = bNew;
    }

    void Shift( USHORT nDir );

    // sets a new color at the output device if necessary
    // if a font is passed as argument, the change if made to the font
    // otherwise the font at the output device is changed
    // returns if the font has been changed
    sal_Bool ApplyAutoColor( Font* pFnt = 0 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
