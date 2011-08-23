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

#ifndef INCLUDED_SVTOOLS_NFKEYTAB_HXX
#define INCLUDED_SVTOOLS_NFKEYTAB_HXX

#include <tools/string.hxx>

namespace binfilter
{

//! For ImpSvNumberformatScan: first the short symbols, then the long symbols!
//! e.g. first TT then TTTT
//! The internal order is essentially for the format code string scanner!
//! New keywords MUST NOT be inserted, only the NF_KEY_UNUSEDn may be used,
//! NF_KEY_LASTKEYWORD must be adjusted accordingly. Otherwise old versions
//! will fail upon reading these entries. Nevertheless, old versions are NOT
//! able to display those new keywords => blank display.
//
// Note: 2005-09-02: the above applies to the binary file format.
//
// ER 15.12.99: This table is externally only to be used with method
// String SvNumberformat::GetMappedFormatstring( const NfKeywordTable&, const LocaleDataWrapper& );
// and method
// void SvNumberFormatter::FillKeywordTable( NfKeywordTable&, LanguageType );
enum NfKeywordIndex
{
    NF_KEY_NONE = 0,
    NF_KEY_E,           // exponential symbol
    NF_KEY_AMPM,        // AM/PM
    NF_KEY_AP,          // a/p
    NF_KEY_MI,          // minute       (!)
    NF_KEY_MMI,         // minute 02    (!)
    NF_KEY_M,           // month        (!)
    NF_KEY_MM,          // month 02     (!)
    NF_KEY_MMM,         // month short name
    NF_KEY_MMMM,        // month long name
    NF_KEY_H,           // hour
    NF_KEY_HH,          // hour 02
    NF_KEY_S,           // second
    NF_KEY_SS,          // second 02
    NF_KEY_Q,           // quarter
    NF_KEY_QQ,          // quarter 02
    NF_KEY_D,           // day of month
    NF_KEY_DD,          // day of month 02
    NF_KEY_DDD,         // day of week short
    NF_KEY_DDDD,        // day of week long
    NF_KEY_YY,          // year two digits
    NF_KEY_YYYY,        // year four digits
    NF_KEY_NN,          // day of week short
    NF_KEY_NNNN,        // day of week long with separator
    NF_KEY_CCC,         // currency bank symbol (old version)
    NF_KEY_GENERAL,     // General / Standard
    NF_KEY_LASTOLDKEYWORD = NF_KEY_GENERAL,
    NF_KEY_NNN,         // day of week long without separator, as of version 6, 10.10.97
    NF_KEY_WW,          // week of year, as of version 8, 19.06.98
    NF_KEY_MMMMM,       // first letter of month name
    NF_KEY_LASTKEYWORD = NF_KEY_MMMMM,
    NF_KEY_UNUSED4,
    NF_KEY_QUARTER,     // was quarter word, not used anymore from SRC631 on (26.04.01)
    NF_KEY_TRUE,        // boolean true
    NF_KEY_FALSE,       // boolean false
    NF_KEY_BOOLEAN,     // boolean
    NF_KEY_COLOR,       // color
    NF_KEY_FIRSTCOLOR,
    NF_KEY_BLACK = NF_KEY_FIRSTCOLOR,   // you do know colors, don't you?
    NF_KEY_BLUE,
    NF_KEY_GREEN,
    NF_KEY_CYAN,
    NF_KEY_RED,
    NF_KEY_MAGENTA,
    NF_KEY_BROWN,
    NF_KEY_GREY,
    NF_KEY_YELLOW,
    NF_KEY_WHITE,
    NF_KEY_LASTCOLOR = NF_KEY_WHITE,
    NF_KEY_LASTKEYWORD_SO5 = NF_KEY_LASTCOLOR,
    //! Keys from here on can't be saved in SO5 file format and MUST be
    //! converted to string which means losing any information.
    NF_KEY_AAA,         // abbreviated day name from Japanese Xcl, same as DDD or NN English
    NF_KEY_AAAA,        // full day name from Japanese Xcl, same as DDDD or NNN English
    NF_KEY_EC,          // E non-gregorian calendar year without preceding 0
    NF_KEY_EEC,         // EE non-gregorian calendar year with preceding 0 (two digit)
    NF_KEY_G,           // abbreviated era name, latin characters M T S or H for Gengou calendar
    NF_KEY_GG,          // abbreviated era name
    NF_KEY_GGG,         // full era name
    NF_KEY_R,           // acts as EE (Xcl) => GR==GEE, GGR==GGEE, GGGR==GGGEE
    NF_KEY_RR,          // acts as GGGEE (Xcl)
    NF_KEY_THAI_T,      // Thai T modifier, speciality of Thai Excel, only used with Thai locale and converted to [NatNum1]
    NF_KEYWORD_ENTRIES_COUNT
};

typedef String NfKeywordTable [NF_KEYWORD_ENTRIES_COUNT];

}

#endif // INCLUDED_SVTOOLS_NFKEYTAB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
