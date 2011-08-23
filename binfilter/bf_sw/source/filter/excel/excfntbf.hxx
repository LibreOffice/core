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
#ifndef _EXCFNTBF_HXX
#define _EXCFNTBF_HXX

#include <tools/solar.h>
class String; 
namespace binfilter {

class SvxColorItem;
class SvxFontItem;
class SvxFontHeightItem;


//------------------------------------------------------------------------

enum ExcScript {
    EXCSCR_None = 0x00,
    EXCSCR_Super = 0x01,
    EXCSCR_Sub = 0x02 };

enum ExcUnderline {
    EXCUNDER_None = 0x00,
    EXCUNDER_Single = 0x01,
    EXCUNDER_Double = 0x02,
    EXCUNDER_SingleAccount = 0x21,
    EXCUNDER_DoubleAccount = 0x22,
    EXCUNDER_Attr = 0xFF };

struct ExcFont{
    SvxFontItem				*pFont;
    SvxFontHeightItem		*pHeight;
    UINT16					nColor;
    UINT16					nWeight;
    ExcScript				eScript;
    ExcUnderline			eUnderline;
    INT16					bItalic : 1;
    INT16					bStrikeout : 1;
    INT16					bOutline : 1;
    INT16					bShadow : 1;
    };


//------------------------------------------------------------------------
class ColorBuffer
    {
    private:
        SvxColorItem		**pArray;
        SvxColorItem		*pDefault;
        UINT16				nMax;
        UINT16				nCount;
        BOOL				bAuto;
        // ----------------------------------------------------------
    public:
                            ColorBuffer();
                            ~ColorBuffer();

#ifdef USED
        void				Reset( void );
#endif
        BOOL				NewColor( UINT16 nR, UINT16 nG, UINT16 nB );
        const SvxColorItem	*GetColor( UINT16 nIndex );

        BOOL				Auto( void ) const { return bAuto; }
    };

//------------------------------------------------------------------------
class FontBuffer
    {
    private:
        ExcFont				**ppFonts;	// Array mit Fontbeschreibungen
        UINT16				nMax;		// Groesse des Arrays
        UINT16				nCount;		// akt. Speichermarke im Array
        ExcFont				aDefaultFont;
    public:
                            FontBuffer( UINT16 nNewMax = 128 );
                            ~FontBuffer();

        void 				NewFont( UINT16 nHeight, BYTE nAttr0,
                                UINT16 nIndexCol, const String &rName );

        void				NewFont( UINT16 nHeight, BYTE nAttr0,
                                BYTE nUnderline, UINT16 nIndexCol,
                                UINT16 nBoldness, BYTE nFamily, BYTE nCharset,
                                const String &rName );

        const ExcFont		&GetFont( UINT16 nIndex );

#ifdef USED
        void				Reset( void );
#endif
    };




} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
