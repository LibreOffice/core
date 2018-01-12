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
#ifndef INCLUDED_SVX_FLAGSDEF_HXX
#define INCLUDED_SVX_FLAGSDEF_HXX

#include <o3tl/typed_flags_set.hxx>
#include <o3tl/enumarray.hxx>

// Border-Modes for SvxBorderTabPage
enum class SwBorderModes
{
    NONE     = 0x00,
    PARA     = 0x01,
    TABLE    = 0x02,
    FRAME    = 0x04,
};

namespace o3tl
{
    template<> struct typed_flags<SwBorderModes> : is_typed_flags<SwBorderModes, 0x07> {};
}

// flags for SvxBackgroundTabPage
enum class SvxBackgroundTabFlags
{
    NONE                 = 0x00,
    SHOW_SELECTOR        = 0x01,
    SHOW_TBLCTL          = 0x08,
    SHOW_HIGHLIGHTING    = 0x10,
    SHOW_CHAR_BKGCOLOR   = 0x20,
};
namespace o3tl
{
    template<> struct typed_flags<SvxBackgroundTabFlags> : is_typed_flags<SvxBackgroundTabFlags, 0x39> {};
}

// flags for SvxBorderTabPage
#define SVX_HIDESHADOWCTL   0x01

#define DISABLE_CASEMAP             (sal_uInt16(0x0001))
#define DISABLE_WORDLINE            (sal_uInt16(0x0002))
#define DISABLE_BLINK               (sal_uInt16(0x0004))
#define DISABLE_UNDERLINE_COLOR     (sal_uInt16(0x0008))

#define DISABLE_LANGUAGE            (sal_uInt16(0x0010))
#define DISABLE_HIDE_LANGUAGE       (sal_uInt16(0x0020))

// flags for SvxCharBasePage's child class
#define SVX_PREVIEW_CHARACTER   0x01
// flags for SvxCharNamePage
#define SVX_RELATIVE_MODE       0x02
// flags for SvxCharEffectsPage
#define SVX_ENABLE_FLASH    0x04


// Default values for Number Format Category List and Preview
enum class SvxNumValCategory
{
    Standard = 0,
    Percent,
    Currency,
    Date,
    Time,
    Scientific,
    Fraction,
    Boolean,
    NoValue,
    LAST = NoValue
};
const o3tl::enumarray<SvxNumValCategory, double> fSvxNumValConst = {
    -1234.56789012345678,   // SvxNumValCategory::Standard
    -0.1295,                // SvxNumValCategory::Percent
    -1234.0,                // SvxNumValCategory::Currency
    36525.5678935185,       // SvxNumValCategory::Date
    36525.5678935185,       // SvxNumValCategory::Time
    12345.67889,            // SvxNumValCategory::Scientific
    123.456,                // SvxNumValCategory::Fraction
    1.0,                    // SvxNumValCategory::Boolean
    0.0                     // SvxNumValCategory::NoValue
};

enum SvxModeType
{
    SVX_PAGE_MODE_STANDARD,
    SVX_PAGE_MODE_CENTER,
    SVX_PAGE_MODE_PRESENTATION
};

// 1/2 cm in TWIPS
// Is also used for minimum size of LayFrms of any kind
#define MM50   283  //from original svx/inc/paragrph.hxx

enum class TabulatorDisableFlags {
    TypeLeft      = 0x0001,
    TypeRight     = 0x0002,
    TypeCenter    = 0x0004,
    TypeDecimal   = 0x0008,
    TypeMask      = 0x000F,

    FillNone      = 0x0010,
    FillPoint     = 0x0020,
    FillDashLine  = 0x0040,
    FillSolidLine = 0x0080,
    FillSpecial   = 0x0100,
    FillMask      = 0x01F0
};
namespace o3tl {
    template<> struct typed_flags<TabulatorDisableFlags> : is_typed_flags<TabulatorDisableFlags, 0x1ff> {};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
