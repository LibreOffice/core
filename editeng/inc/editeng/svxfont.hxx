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
#ifndef _SVX_SVXFONT_HXX
#define _SVX_SVXFONT_HXX

#include <limits.h>     // USHRT_MAX
#include <editeng/svxenum.hxx>
#include <i18npool/lang.h>
#include <vcl/font.hxx>
#include "editeng/editengdllapi.h"

class SvxDoCapitals;
class OutputDevice;
class Printer;

class EDITENG_DLLPUBLIC SvxFont : public Font
{
    LanguageType eLang;         // Language
    SvxCaseMap   eCaseMap;      // Textauszeichnung
    short nEsc;                 // Grad der Hoch-/Tiefstellung
    sal_uInt8  nPropr;               // Grad der Verkleinerung der Fonthoehe
    short nKern;                // Kerning in Pt

public:
    SvxFont();
    SvxFont( const Font &rFont );
    SvxFont( const SvxFont &rFont );

    // Methoden fuer die Hoch-/Tiefstellung
    inline short GetEscapement() const { return nEsc; }
    inline void SetEscapement( const short nNewEsc ) { nEsc = nNewEsc; }

    inline sal_uInt8 GetPropr() const { return nPropr; }
    inline void SetPropr( const sal_uInt8 nNewPropr ) { nPropr = nNewPropr; }
    inline void SetProprRel( const sal_uInt8 nNewPropr )
        { SetPropr( (sal_uInt8)( (long)nNewPropr * (long)nPropr / 100L ) ); }

    // Kerning
    inline short GetFixKerning() const { return nKern; }
    inline void  SetFixKerning( const short nNewKern ) { nKern = nNewKern; }

    inline SvxCaseMap GetCaseMap() const { return eCaseMap; }
    inline void    SetCaseMap( const SvxCaseMap eNew ) { eCaseMap = eNew; }

    inline LanguageType GetLanguage() const { return eLang; }
    inline void SetLanguage( const LanguageType eNewLan )
        { eLang = eNewLan;  Font::SetLanguage(eNewLan); }

    // Is-Methoden:
    inline sal_Bool IsCaseMap() const { return SVX_CASEMAP_NOT_MAPPED != eCaseMap; }
    inline sal_Bool IsCapital() const { return SVX_CASEMAP_KAPITAELCHEN == eCaseMap; }
    inline sal_Bool IsKern() const { return 0 != nKern; }
    inline sal_Bool IsEsc() const { return 0 != nEsc; }

    // Versalien, Gemeine etc. beruecksichtigen
    String CalcCaseMap( const String &rTxt ) const;

// Der folgende Bereich wird nicht von jedem benoetigt, er kann deshalb
// ausgeklammert werden.
#ifndef REDUCEDSVXFONT
    // Kapitaelchenbearbeitung
    void DoOnCapitals( SvxDoCapitals &rDo,
                       const sal_uInt16 nPartLen = USHRT_MAX ) const;

    void SetPhysFont( OutputDevice *pOut ) const;
    Font ChgPhysFont( OutputDevice *pOut ) const;

    Size GetCapitalSize( const OutputDevice *pOut, const String &rTxt,
                          const sal_uInt16 nIdx, const sal_uInt16 nLen) const;
    void DrawCapital( OutputDevice *pOut, const Point &rPos, const String &rTxt,
                      const sal_uInt16 nIdx, const sal_uInt16 nLen ) const;

    Size GetPhysTxtSize( const OutputDevice *pOut, const String &rTxt,
                         const sal_uInt16 nIdx, const sal_uInt16 nLen ) const;

    Size GetPhysTxtSize( const OutputDevice *pOut, const String &rTxt );

    Size GetTxtSize( const OutputDevice *pOut, const String &rTxt,
                      const sal_uInt16 nIdx = 0, const sal_uInt16 nLen = STRING_LEN );

    void DrawText( OutputDevice *pOut, const Point &rPos, const String &rTxt,
               const sal_uInt16 nIdx = 0, const sal_uInt16 nLen = STRING_LEN ) const;

    void QuickDrawText( OutputDevice *pOut, const Point &rPos, const String &rTxt,
               const sal_uInt16 nIdx = 0, const sal_uInt16 nLen = STRING_LEN, const sal_Int32* pDXArray = NULL ) const;

    Size QuickGetTextSize( const OutputDevice *pOut, const String &rTxt,
                         const sal_uInt16 nIdx, const sal_uInt16 nLen, sal_Int32* pDXArray = NULL ) const;

    void DrawPrev( OutputDevice* pOut, Printer* pPrinter,
                   const Point &rPos, const String &rTxt,
                   const sal_uInt16 nIdx = 0, const sal_uInt16 nLen = STRING_LEN ) const;

#endif // !REDUCEDSVXFONT
    static void DrawArrow( OutputDevice &rOut, const Rectangle& rRect,
        const Size& rSize, const Color& rCol, sal_Bool bLeft );
    SvxFont&    operator=( const SvxFont& rFont );
    SvxFont&    operator=( const Font& rFont );
};

#endif // #ifndef   _SVX_SVXFONT_HXX

