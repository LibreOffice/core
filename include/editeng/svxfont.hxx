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
#ifndef INCLUDED_EDITENG_SVXFONT_HXX
#define INCLUDED_EDITENG_SVXFONT_HXX

#include <editeng/svxenum.hxx>
#include <tools/long.hxx>
#include <vcl/font.hxx>
#include <editeng/editengdllapi.h>
#include <tools/poly.hxx>

// Percentage of height of lower case small capital letters compared to upper case letters
// See i#1526# for full explanation
#define SMALL_CAPS_PERCENTAGE 80

class SvxDoCapitals;
class OutputDevice;
class Printer;
class Point;
namespace tools { class Rectangle; }
class Size;

class EDITENG_DLLPUBLIC SvxFont : public vcl::Font
{
    SvxCaseMap   eCaseMap;      // Text Markup
    short nEsc;                 // Degree of Superscript/Subscript
    sal_uInt8  nPropr;          // Degree of reduction of the font height
    short nKern;                // Kerning in Pt

public:
    SvxFont();
    SvxFont( const vcl::Font &rFont );
    SvxFont( const SvxFont &rFont );

    // Methods for Superscript/Subscript
    short GetEscapement() const { return nEsc; }
    void SetEscapement( const short nNewEsc ) { nEsc = nNewEsc; }
    // set specific values instead of automatic, and ensure valid value. Depends on nPropr being set already.
    void SetNonAutoEscapement(short nNewEsc, const OutputDevice* pOutDev = nullptr);

    sal_uInt8 GetPropr() const { return nPropr; }
    void SetPropr( const sal_uInt8 nNewPropr ) { nPropr = nNewPropr; }
    void SetProprRel( const sal_uInt8 nNewPropr )
        { SetPropr( static_cast<sal_uInt8>( static_cast<tools::Long>(nNewPropr) * static_cast<tools::Long>(nPropr) / 100 ) ); }

    // Kerning
    short GetFixKerning() const { return nKern; }
    void  SetFixKerning( const short nNewKern ) { nKern = nNewKern; }

    SvxCaseMap GetCaseMap() const { return eCaseMap; }
    void    SetCaseMap( const SvxCaseMap eNew ) { eCaseMap = eNew; }

    // Is-Methods:
    bool IsCaseMap() const { return SvxCaseMap::NotMapped != eCaseMap; }
    bool IsCapital() const { return SvxCaseMap::SmallCaps == eCaseMap; }
    bool IsKern() const { return 0 != nKern; }
    bool IsEsc() const { return 0 != nEsc; }

    // Consider Upper case, Lower case letters etc.
    OUString CalcCaseMap(const OUString &rTxt) const;

    // Handle upper case letters
    void DoOnCapitals(SvxDoCapitals &rDo) const;

    void SetPhysFont( OutputDevice *pOut ) const;
    vcl::Font ChgPhysFont( OutputDevice *pOut ) const;

    Size GetCapitalSize( const OutputDevice *pOut, const OUString &rTxt,
                          const sal_Int32 nIdx, const sal_Int32 nLen) const;
    void DrawCapital( OutputDevice *pOut, const Point &rPos, const OUString &rTxt,
                      const sal_Int32 nIdx, const sal_Int32 nLen ) const;

    Size GetPhysTxtSize( const OutputDevice *pOut, const OUString &rTxt,
                         const sal_Int32 nIdx, const sal_Int32 nLen ) const;

    Size GetPhysTxtSize( const OutputDevice *pOut );

    Size GetTextSize( const OutputDevice *pOut, const OUString &rTxt,
                      const sal_Int32 nIdx = 0, const sal_Int32 nLen = SAL_MAX_INT32 ) const;

    void QuickDrawText( OutputDevice *pOut, const Point &rPos, const OUString &rTxt,
                        const sal_Int32 nIdx = 0, const sal_Int32 nLen = SAL_MAX_INT32, const tools::Long* pDXArray = nullptr ) const;

    Size QuickGetTextSize( const OutputDevice *pOut, const OUString &rTxt,
                         const sal_Int32 nIdx, const sal_Int32 nLen, tools::Long* pDXArray = nullptr ) const;

    void DrawPrev( OutputDevice* pOut, Printer* pPrinter,
                   const Point &rPos, const OUString &rTxt,
                   const sal_Int32 nIdx = 0, const sal_Int32 nLen = SAL_MAX_INT32 ) const;

    static tools::Polygon DrawArrow( OutputDevice &rOut, const tools::Rectangle& rRect,
                                    const Size& rSize, const Color& rCol, bool bLeftOrTop,
                                    bool bVertical );

    SvxFont&    operator=( const SvxFont& rFont );
    SvxFont&    operator=( const Font& rFont );
};

#endif // INCLUDED_EDITENG_SVXFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
