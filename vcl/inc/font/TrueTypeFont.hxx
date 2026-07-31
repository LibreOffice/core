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

#include <config_options.h>
#include <rtl/ustring.hxx>
#include <tools/fontenum.hxx>
#include <vcl/dllapi.h>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>

#include <hb-ot.h>

#include <vector>
#include "PhysicalFontFace.hxx"
#include <fontsubset.hxx>

namespace vcl
{
/*
  Some table OS/2 consts
  quick history:
  OpenType has been created from TrueType
  - original TrueType had an OS/2 table with a length of 68 bytes
  (cf https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6OS2.html)
  - There have been 6 versions (from version 0 to 5)
  (cf https://docs.microsoft.com/en-us/typography/opentype/otspec140/os2ver0)

  For the record:
  // From Initial TrueType version
  TYPE       NAME                       FROM BYTE
  uint16     version                    0
  int16      xAvgCharWidth              2
  uint16     usWeightClass              4
  uint16     usWidthClass               6
  uint16     fsType                     8
  int16      ySubscriptXSize           10
  int16      ySubscriptYSize           12
  int16      ySubscriptXOffset         14
  int16      ySubscriptYOffset         16
  int16      ySuperscriptXSize         18
  int16      ySuperscriptYSize         20
  int16      ySuperscriptXOffset       22
  int16      ySuperscriptYOffset       24
  int16      yStrikeoutSize            26
  int16      yStrikeoutPosition        28
  int16      sFamilyClass              30
  uint8      panose[10]                32
  uint32     ulUnicodeRange1           42
  uint32     ulUnicodeRange2           46
  uint32     ulUnicodeRange3           50
  uint32     ulUnicodeRange4           54
  Tag        achVendID                 58
  uint16     fsSelection               62
  uint16     usFirstCharIndex          64
  uint16     usLastCharIndex           66

  // From Version 0 of OpenType
  int16      sTypoAscender             68
  int16      sTypoDescender            70
  int16      sTypoLineGap              72
  uint16     usWinAscent               74
  uint16     usWinDescent              76

  => length for OpenType version 0 = 78 bytes

  // From Version 1 of OpenType
  uint32     ulCodePageRange1          78
  uint32     ulCodePageRange2          82

  => length for OpenType version 1 = 86 bytes

  // From Version 2 of OpenType
  // (idem for Versions 3 and 4)
  int16      sxHeight                  86
  int16      sCapHeight                88
  uint16     usDefaultChar             90
  uint16     usBreakChar               92
  uint16     usMaxContext              94

  => length for OpenType version 2, 3 and 4 = 96 bytes

  // From Version 5 of OpenType
  uint16     usLowerOpticalPointSize   96
  uint16     usUpperOpticalPointSize   98
  END                                 100

  => length for OS/2 table version 5 = 100 bytes

*/
constexpr int OS2_panose_offset = 32;

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) TrueTypeFont
{
    hb_face_t* m_pFace = nullptr;
    mutable hb_font_t* m_pFont = nullptr;

    hb_font_t* getFont() const;
    OUString getName(hb_ot_name_id_t nNameID,
                     const LanguageTag& rLang = LanguageTag(LANGUAGE_DONTKNOW)) const;

public:
    TrueTypeFont(const void* pBuffer, sal_uInt32 nLen, sal_uInt32 facenum);
    ~TrueTypeFont();

    bool isValid() const { return m_pFace != hb_face_get_empty(); }

    OUString getFamilyName() const;
    OUString getTypographicFamilyName() const;
    OUString getSubfamilyName() const;
    sal_uInt32 countNonEmptyGlyphs() const;
    FontWeight getFontWeight() const;
    FontWidth getFontWidth() const;
    FontItalic getFontItalic() const;
    FontPitch getFontPitch() const;
    sal_uInt32 getTypeFlags() const;
};

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
