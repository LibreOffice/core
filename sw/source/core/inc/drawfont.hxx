/*************************************************************************
 *
 *  $RCSfile: drawfont.hxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-30 10:18:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _DRAWFONT_HXX
#define _DRAWFONT_HXX

class SwTxtFrm;
class OutputDevice;
class ViewShell;
class SwScriptInfo;
class Point;
class SwWrongList;
class Size;
class SwFont;
class Font;
class SwTxtNode;
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
    const Size* pSize;
    SwFont *pFnt;
    SwUnderlineFont* pUnderFnt;
    xub_StrLen* pHyphPos;
//   Fraction aZoom;
    long nLeft;
    long nRight;
    long nKanaDiff;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    xub_StrLen nOfst;
    USHORT nWidth;
    USHORT nAscent;
    USHORT nCompress;
    short nSperren;
    short nKern;
    short nSpace;
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
                                // have of bound rect, used for Accessibility

    SwDrawTextInfo();       // nicht zulaessig
public:
#ifndef PRODUCT
    BOOL bOut   : 1;    // In der Non-Product wird der Zugriff auf die Member
    BOOL bPos   : 1;    // durch diese Flags ueberwacht.
    BOOL bText  : 1;    // Wird ein Member gesetzt, so wird das entsprechende
    BOOL bWrong : 1;    // Flag gesetzt.
    BOOL bSize  : 1;    // Wird ein Member ausgelesen, so wird ASSERTet, dass
    BOOL bFnt   : 1;    // dieser zuvor gesetzt worden ist.
    BOOL bIdx   : 1;
    BOOL bLen   : 1;
    BOOL bWidth : 1;
    BOOL bAscent: 1;
    BOOL bSperr : 1;
    BOOL bKern  : 1;
    BOOL bSpace : 1;
    BOOL bBull  : 1;
    BOOL bSpec  : 1;
    BOOL bUppr  : 1;
    BOOL bDrawSp: 1;
    BOOL bGreyWv: 1;
    BOOL bLeft  : 1;
    BOOL bRight : 1;
    BOOL bKana  : 1;
    BOOL bOfst  : 1;
    BOOL bHyph  : 1;
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
        nCursorBidiLevel = 0;
        bBullet = bB;
        pUnderFnt = 0;
        bGreyWave = FALSE;
        bSpaceStop = FALSE;
        bSnapToGrid = FALSE;
        bIgnoreFrmRTL = FALSE;
        bPosMatchesBounds = FALSE;

#ifndef PRODUCT
        bOut = bText = bIdx = bLen = bWidth = bKern = bBull = bSpec =
        bGreyWv = TRUE;

        bPos = bWrong = bSize = bFnt = bAscent = bSpace = bUppr =
        bDrawSp = bLeft = bRight = bKana = bOfst = bHyph = FALSE;
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
        ASSERT( bOut, "DrawTextInfo: Undefined Outputdevice" );
        return *pOut;
    }

    OutputDevice *GetpOut() const
    {
        ASSERT( bOut, "DrawTextInfo: Undefined Outputdevice" );
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
        ASSERT( bText, "DrawTextInfo: Undefined String" );
        return *pText;
    }

    const SwWrongList* GetWrong() const
    {
        ASSERT( bWrong, "DrawTextInfo: Undefined WrongList" );
        return pWrong;
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
        ASSERT( bSpec, "DrawTextInfo: Undefined Underlinefont" );
        return pUnderFnt;
    }

    xub_StrLen GetIdx() const
    {
        ASSERT( bIdx, "DrawTextInfo: Undefined Index" );
        return nIdx;
    }

    xub_StrLen GetLen() const
    {
        ASSERT( bLen, "DrawTextInfo: Undefined Length" );
        return nLen;
    }

    xub_StrLen GetOfst() const
    {
        ASSERT( bOfst, "DrawTextInfo: Undefined Offset" );
        return nOfst;
    }

    xub_StrLen GetEnd() const
    {
        ASSERT( bIdx, "DrawTextInfo: Undefined Index" );
        ASSERT( bLen, "DrawTextInfo: Undefined Length" );
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
        ASSERT( bWidth, "DrawTextInfo: Undefined Width" );
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

    short GetSperren() const
    {
        ASSERT( bSperr, "DrawTextInfo: Undefined >Sperren<" );
        return nSperren;
    }

    short GetKern() const
    {
        ASSERT( bKern, "DrawTextInfo: Undefined Kerning" );
        return nKern;
    }

    short GetSpace() const
    {
        ASSERT( bSpace, "DrawTextInfo: Undefined Spacing" );
        return nSpace;
    }

    BYTE GetCursorBidiLevel() const
    {
        return nCursorBidiLevel;
    }

    BOOL GetBullet() const
    {
        ASSERT( bBull, "DrawTextInfo: Undefined Bulletflag" );
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
        ASSERT( bGreyWv, "DrawTextInfo: Undefined GreyWave" );
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
#ifndef PRODUCT
        bOut = TRUE;
#endif
    }

    void SetPos( const Point &rNew )
    {
        pPos = &rNew;
#ifndef PRODUCT
        bPos = TRUE;
#endif
    }

    void SetHyphPos( xub_StrLen *pNew )
    {
        pHyphPos = pNew;
#ifndef PRODUCT
        bHyph = TRUE;
#endif
    }

    void SetText( const XubString &rNew )
    {
        pText = &rNew;
#ifndef PRODUCT
        bText = TRUE;
#endif
    }

    void SetWrong( const SwWrongList* pNew )
    {
        pWrong = pNew;
#ifndef PRODUCT
        bWrong = TRUE;
#endif
    }

    void SetSize( const Size &rNew )
    {
        pSize = &rNew;
#ifndef PRODUCT
        bSize = TRUE;
#endif
    }

    void SetFont( SwFont* pNew )
    {
        pFnt = pNew;
#ifndef PRODUCT
        bFnt = TRUE;
#endif
    }

    void SetIdx( xub_StrLen nNew )
    {
        nIdx = nNew;
#ifndef PRODUCT
        bIdx = TRUE;
#endif
    }

    void SetLen( xub_StrLen nNew )
    {
        nLen = nNew;
#ifndef PRODUCT
        bLen = TRUE;
#endif
    }

    void SetOfst( xub_StrLen nNew )
    {
        nOfst = nNew;
#ifndef PRODUCT
        bOfst = TRUE;
#endif
    }

    void SetLeft( long nNew )
    {
        nLeft = nNew;
#ifndef PRODUCT
        bLeft = TRUE;
#endif
    }

    void SetRight( long nNew )
    {
        nRight = nNew;
#ifndef PRODUCT
        bRight = TRUE;
#endif
    }

    void SetKanaDiff( long nNew )
    {
        nKanaDiff = nNew;
#ifndef PRODUCT
        bKana = TRUE;
#endif
    }

    void SetWidth( USHORT nNew )
    {
        nWidth = nNew;
#ifndef PRODUCT
        bWidth = TRUE;
#endif
    }

    void SetAscent( USHORT nNew )
    {
        nAscent = nNew;
#ifndef PRODUCT
        bAscent = TRUE;
#endif
    }

    void SetKern( short nNew )
    {
        nKern = nNew;
#ifndef PRODUCT
        bKern = TRUE;
#endif
    }

    void SetSperren( short nNew )
    {
        nSperren = nNew;
#ifndef PRODUCT
        bSperr = TRUE;
#endif
    }

    void SetSpace( short nNew )
    {
        if( nNew < 0 )
        {
            SetSperren( -nNew );
            nSpace = 0;
        }
        else
        {
            nSpace = nNew;
            SetSperren( 0 );
        }
#ifndef PRODUCT
        bSpace = TRUE;
#endif
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
#ifndef PRODUCT
        bBull = TRUE;
#endif
    }

    void SetUnderFnt( SwUnderlineFont* pFnt )
    {
        pUnderFnt = pFnt;
#ifndef PRODUCT
        bSpec = TRUE;
#endif
    }

    void SetUpper( BOOL bNew )
    {
        bUpper = bNew;
#ifndef PRODUCT
        bUppr = TRUE;
#endif
    }

    void SetDrawSpace( BOOL bNew )
    {
        bDrawSpace = bNew;
#ifndef PRODUCT
        bDrawSp = TRUE;
#endif
    }

    void SetGreyWave( BOOL bNew )
    {
        bGreyWave = bNew;
#ifndef PRODUCT
        bGreyWv = TRUE;
#endif
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
