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
#ifndef FORMAT_HXX
#define FORMAT_HXX


#include <svl/smplhint.hxx>
#include <svl/brdcst.hxx>
#include "utility.hxx"
#include <types.hxx>


#define SM_FMT_VERSION_51   ((sal_uInt8) 0x01)
#define SM_FMT_VERSION_NOW  SM_FMT_VERSION_51

#define FNTNAME_TIMES   "Times New Roman"
#define FNTNAME_HELV    "Helvetica"
#define FNTNAME_COUR    "Courier"
#define FNTNAME_MATH    FONTNAME_MATH


// symbolic names used as array indices
#define SIZ_BEGIN       0
#define SIZ_TEXT        0
#define SIZ_INDEX       1
#define SIZ_FUNCTION    2
#define SIZ_OPERATOR    3
#define SIZ_LIMITS      4
#define SIZ_END         4

// symbolic names used as array indices
#define FNT_BEGIN       0
#define FNT_VARIABLE    0
#define FNT_FUNCTION    1
#define FNT_NUMBER      2
#define FNT_TEXT        3
#define FNT_SERIF       4
#define FNT_SANS        5
#define FNT_FIXED       6
#define FNT_MATH        7
#define FNT_END         7

// symbolic names used as array indices
#define DIS_BEGIN                0
#define DIS_HORIZONTAL           0
#define DIS_VERTICAL             1
#define DIS_ROOT                 2
#define DIS_SUPERSCRIPT          3
#define DIS_SUBSCRIPT            4
#define DIS_NUMERATOR            5
#define DIS_DENOMINATOR          6
#define DIS_FRACTION             7
#define DIS_STROKEWIDTH          8
#define DIS_UPPERLIMIT           9
#define DIS_LOWERLIMIT          10
#define DIS_BRACKETSIZE         11
#define DIS_BRACKETSPACE        12
#define DIS_MATRIXROW           13
#define DIS_MATRIXCOL           14
#define DIS_ORNAMENTSIZE        15
#define DIS_ORNAMENTSPACE       16
#define DIS_OPERATORSIZE        17
#define DIS_OPERATORSPACE       18
#define DIS_LEFTSPACE           19
#define DIS_RIGHTSPACE          20
#define DIS_TOPSPACE            21
#define DIS_BOTTOMSPACE         22
#define DIS_NORMALBRACKETSIZE   23
#define DIS_END                 23


// to be broadcastet on format changes:
#define HINT_FORMATCHANGED  10003

enum SmHorAlign { AlignLeft, AlignCenter, AlignRight };

OUString GetDefaultFontName( LanguageType nLang, sal_uInt16 nIdent );

class SmFormat : public SfxBroadcaster
{
    SmFace      vFont[FNT_END + 1];
    bool        bDefaultFont[FNT_END + 1];
    Size        aBaseSize;
    long        nVersion;
    sal_uInt16      vSize[SIZ_END + 1];
    sal_uInt16      vDist[DIS_END + 1];
    SmHorAlign  eHorAlign;
    sal_Int16       nGreekCharStyle;
    bool        bIsTextmode,
                bScaleNormalBrackets;

public:
    SmFormat();
    SmFormat(const SmFormat &rFormat) : SfxBroadcaster() { *this = rFormat; }

    const Size &    GetBaseSize() const             { return aBaseSize; }
    void            SetBaseSize(const Size &rSize)  { aBaseSize = rSize; }

    const SmFace &  GetFont(sal_uInt16 nIdent) const { return vFont[nIdent]; }
    void            SetFont(sal_uInt16 nIdent, const SmFace &rFont, bool bDefault = false);
    void            SetFontSize(sal_uInt16 nIdent, const Size &rSize)   { vFont[nIdent].SetSize( rSize ); }

    void            SetDefaultFont(sal_uInt16 nIdent, bool bVal)    { bDefaultFont[nIdent] = bVal; }
    bool            IsDefaultFont(sal_uInt16 nIdent) const   { return bDefaultFont[nIdent]; }

    sal_uInt16      GetRelSize(sal_uInt16 nIdent) const         { return vSize[nIdent]; }
    void            SetRelSize(sal_uInt16 nIdent, sal_uInt16 nVal)  { vSize[nIdent] = nVal;}

    sal_uInt16      GetDistance(sal_uInt16 nIdent) const            { return vDist[nIdent]; }
    void            SetDistance(sal_uInt16 nIdent, sal_uInt16 nVal) { vDist[nIdent] = nVal; }

    SmHorAlign      GetHorAlign() const             { return eHorAlign; }
    void            SetHorAlign(SmHorAlign eAlign)  { eHorAlign = eAlign; }

    bool            IsTextmode() const     { return bIsTextmode; }
    void            SetTextmode(bool bVal) { bIsTextmode = bVal; }

    sal_Int16       GetGreekCharStyle() const     { return nGreekCharStyle; }
    void            SetGreekCharStyle(sal_Int16 nVal) { nGreekCharStyle = nVal; }

    bool            IsScaleNormalBrackets() const     { return bScaleNormalBrackets; }
    void            SetScaleNormalBrackets(bool bVal) { bScaleNormalBrackets = bVal; }

    long            GetVersion() const { return nVersion; }

    //! at time (5.1) use only the lower byte!!!
    void            SetVersion(long nVer) { nVersion = nVer; }

    SmFormat &      operator = (const SmFormat &rFormat);

    bool            operator == (const SmFormat &rFormat) const;
    inline bool     operator != (const SmFormat &rFormat) const;

    void RequestApplyChanges() const
    {
        ((SmFormat *) this)->Broadcast(SfxSimpleHint(HINT_FORMATCHANGED));
    }

};

inline bool    SmFormat::operator != (const SmFormat &rFormat) const
{
    return !(*this == rFormat);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
