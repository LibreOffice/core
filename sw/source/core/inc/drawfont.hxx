/*************************************************************************
 *
 *  $RCSfile: drawfont.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-13 09:36:59 $
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


#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#include "errhdl.hxx"
#ifndef _FRACT_HXX
#include <tools/fract.hxx>
#endif

class OutputDevice;
class Point;
class SwWrongList;
class Size;
class SwFont;
class ViewShell;

class SwDrawTextInfo
{
    OutputDevice* pOut;
    ViewShell* pSh;
    const Point* pPos;
    const XubString* pText;
    const SwWrongList* pWrong;
    const Size* pSize;
    SwFont *pFnt;
    Fraction aZoom;
    long nLeft;
    long nRight;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    USHORT nWidth;
    USHORT nAscent;
    short nSperren;
    short nKern;
    short nSpace;
    BOOL bBullet : 1;
    BOOL bSpecialUnderline : 1;
    BOOL bUpper : 1;        // Fuer Kapitaelchen: Grossbuchstaben-Flag
    BOOL bDrawSpace : 1;    // Fuer Kapitaelchen: Unter/Durchstreichung
    BOOL bGreyWave  : 1;    // Graue Wellenlinie beim extended TextInput
    BOOL bDarkBack : 1; // Dark background sets automatic font color to white
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
#endif
    SwDrawTextInfo( ViewShell *pS, OutputDevice &rO,
                    const XubString &rSt, xub_StrLen nI, xub_StrLen nL,
                    USHORT nW, BOOL bB )
    {   pSh = pS; pOut = &rO; pText = &rSt; nIdx = nI; nLen = nL; nWidth = nW;
        bBullet = bB; bSpecialUnderline = bGreyWave = bDarkBack = FALSE;
#ifndef PRODUCT
        bOut = bText = bIdx = bLen = bWidth = bBull = bSpec = bGreyWv = TRUE;
        bPos = bWrong = bSize = bFnt = bAscent = bKern = bSpace = bUppr =
            bDrawSp = bLeft = bRight = FALSE;
#endif
    }
    ViewShell *GetShell() const { return pSh; }
    OutputDevice& GetOut() const {
        ASSERT( bOut, "DrawTextInfo: Undefined Outputdevice" );
        return *pOut;
    }
    OutputDevice *GetpOut() const {
        ASSERT( bOut, "DrawTextInfo: Undefined Outputdevice" );
        return pOut;
    }
    const Point &GetPos() const {
        ASSERT( bPos, "DrawTextInfo: Undefined Position" );
        return *pPos;
    }

    const Fraction &GetZoom() const { return aZoom; }
    Fraction &GetZoom() { return aZoom; }

    const XubString &GetText() const {
        ASSERT( bText, "DrawTextInfo: Undefined String" );
        return *pText;
    }
    const SwWrongList* GetWrong() const {
        ASSERT( bWrong, "DrawTextInfo: Undefined WrongList" );
        return pWrong;
    }
    const Size &GetSize() const {
        ASSERT( bSize, "DrawTextInfo: Undefined Size" );
        return *pSize;
    }
    SwFont* GetFont() const {
        ASSERT( bFnt, "DrawTextInfo: Undefined Font" );
        return pFnt;
    }
    xub_StrLen GetIdx() const {
        ASSERT( bIdx, "DrawTextInfo: Undefined Index" );
        return nIdx;
    }
    xub_StrLen GetLen() const {
        ASSERT( bLen, "DrawTextInfo: Undefined Length" );
        return nLen;
    }
    xub_StrLen GetEnd() const {
        ASSERT( bIdx, "DrawTextInfo: Undefined Index" );
        ASSERT( bLen, "DrawTextInfo: Undefined Length" );
        return nIdx + nLen;
    }
    long GetLeft() const {
        ASSERT( bLeft, "DrawTextInfo: Undefined left range" );
        return nLeft;
    }
    long GetRight() const {
        ASSERT( bRight, "DrawTextInfo: Undefined right range" );
        return nRight;
    }
    USHORT GetWidth() const {
        ASSERT( bWidth, "DrawTextInfo: Undefined Width" );
        return nWidth;
    }
    USHORT GetAscent() const {
        ASSERT( bAscent, "DrawTextInfo: Undefined Ascent" );
        return nAscent;
    }
    short GetSperren() const {
        ASSERT( bSperr, "DrawTextInfo: Undefined >Sperren<" );
        return nSperren;
    }
    short GetKern() const {
        ASSERT( bKern, "DrawTextInfo: Undefined Kerning" );
        return nKern;
    }
    short GetSpace() const {
        ASSERT( bSpace, "DrawTextInfo: Undefined Spacing" );
        return nSpace;
    }
    BOOL GetBullet() const {
        ASSERT( bBull, "DrawTextInfo: Undefined Bulletflag" );
        return bBullet;
    }
    BOOL GetSpecialUnderline() const {
        ASSERT( bSpec, "DrawTextInfo: Undefined Underlineflag" );
        return bSpecialUnderline;
    }
    BOOL GetUpper() const {
        ASSERT( bUppr, "DrawTextInfo: Undefined Upperflag" );
        return bUpper;
    }
    BOOL GetDrawSpace() const {
        ASSERT( bDrawSp, "DrawTextInfo: Undefined DrawSpaceflag" );
        return bDrawSpace;
    }
    BOOL GetGreyWave() const {
        ASSERT( bGreyWv, "DrawTextInfo: Undefined GreyWave" );
        return bGreyWave;
    }
    BOOL GetDarkBack() const {
        return bDarkBack;
    }

    void SetOut( OutputDevice &rNew ){ pOut = &rNew;
#ifndef PRODUCT
        bOut = TRUE;
#endif
    }
    void SetPos( const Point &rNew ){ pPos = &rNew;
#ifndef PRODUCT
        bPos = TRUE;
#endif
    }
    void SetText( const XubString &rNew ){ pText = &rNew;
#ifndef PRODUCT
        bText = TRUE;
#endif
    }
    void SetWrong( const SwWrongList* pNew ){ pWrong = pNew;
#ifndef PRODUCT
        bWrong = TRUE;
#endif
    }
    void SetSize( const Size &rNew ){ pSize = &rNew;
#ifndef PRODUCT
        bSize = TRUE;
#endif
    }
    void SetFont( SwFont* pNew ){ pFnt = pNew;
#ifndef PRODUCT
        bFnt = TRUE;
#endif
    }
    void SetIdx( xub_StrLen nNew ){ nIdx = nNew;
#ifndef PRODUCT
        bIdx = TRUE;
#endif
    }
    void SetLen( xub_StrLen nNew ){ nLen = nNew;
#ifndef PRODUCT
        bLen = TRUE;
#endif
    }
    void SetLeft( long nNew ){ nLeft = nNew;
#ifndef PRODUCT
        bLeft = TRUE;
#endif
    }
    void SetRight( long nNew ){ nRight = nNew;
#ifndef PRODUCT
        bRight = TRUE;
#endif
    }
    void SetWidth( USHORT nNew ){ nWidth = nNew;
#ifndef PRODUCT
        bWidth = TRUE;
#endif
    }
    void SetAscent( USHORT nNew ){ nAscent = nNew;
#ifndef PRODUCT
        bAscent = TRUE;
#endif
    }
    void SetKern( short nNew ){ nKern = nNew;
#ifndef PRODUCT
        bKern = TRUE;
#endif
    }
    void SetSperren( short nNew ){ nSperren = nNew;
#ifndef PRODUCT
        bSperr = TRUE;
#endif
    }
    void SetSpace( short nNew ){
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
    void SetBullet( BOOL bNew ){ bBullet = bNew;
#ifndef PRODUCT
        bBull = TRUE;
#endif
    }
    void SetSpecialUnderline( BOOL bNew ){ bSpecialUnderline = bNew;
#ifndef PRODUCT
        bSpec = TRUE;
#endif
    }
    void SetUpper( BOOL bNew ){ bUpper = bNew;
#ifndef PRODUCT
        bUppr = TRUE;
#endif
    }
    void SetDrawSpace( BOOL bNew ){ bDrawSpace = bNew;
#ifndef PRODUCT
        bDrawSp = TRUE;
#endif
    }
    void SetGreyWave( BOOL bNew ){ bGreyWave = bNew;
#ifndef PRODUCT
        bGreyWv = TRUE;
#endif
    }
    void SetDarkBack( BOOL bNew ){ bDarkBack = bNew; }
    void ShiftX()
    {
        ASSERT( bPos, "DrawTextInfo: Undefined Position" );
        ASSERT( bSize, "DrawTextInfo: Undefined Width" );
        ((Point*)pPos)->X() += GetSize().Width();
    }
};


#endif

