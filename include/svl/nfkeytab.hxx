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

#ifndef INCLUDED_SVL_NFKEYTAB_HXX
#define INCLUDED_SVL_NFKEYTAB_HXX

#include <vector>
#include <rtl/ustring.hxx>

//! For ImpSvNumberformatScan: first the short symbols, then the long symbols!
//! e.g. first YY then YYYY
//! The internal order is essential for the format code string scanner.
//
// This table is externally only to be used with method
// OUString SvNumberformat::GetMappedFormatstring( const NfKeywordTable&, const LocaleDataWrapper& );
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
    NF_KEY_MMMMM,       // month narrow name, first letter
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
    NF_KEY_NNN,         // day of week long without separator, as of version 6, 10.10.97
    NF_KEY_NNNN,        // day of week long with separator
    NF_KEY_AAA,         // abbreviated day name from Japanese Xcl, same as DDD or NN English
    NF_KEY_AAAA,        // full day name from Japanese Xcl, same as DDDD or NNN English
    NF_KEY_EC,          // E non-gregorian calendar year without preceding 0
    NF_KEY_EEC,         // EE non-gregorian calendar year with preceding 0 (two digit)
    NF_KEY_G,           // abbreviated era name, latin characters M T S or H for Gengou calendar
    NF_KEY_GG,          // abbreviated era name
    NF_KEY_GGG,         // full era name
    NF_KEY_R,           // acts as EE (Xcl) => GR==GEE, GGR==GGEE, GGGR==GGGEE
    NF_KEY_RR,          // acts as GGGEE (Xcl)
    NF_KEY_WW,          // week of year, as of version 8, 19.06.98
    NF_KEY_THAI_T,      // Thai T modifier, speciality of Thai Excel, only used with Thai locale and converted to [NatNum1]
    NF_KEY_CCC,         // currency bank symbol (old version)
    NF_KEY_GENERAL,     // General / Standard
    NF_KEY_LASTKEYWORD = NF_KEY_GENERAL,

    // Reserved words translated and color names follow:
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

    NF_KEYWORD_ENTRIES_COUNT
};

class NfKeywordTable final
{
    typedef ::std::vector<OUString> Keywords_t;
    Keywords_t m_keywords;

public:
    NfKeywordTable() : m_keywords(NF_KEYWORD_ENTRIES_COUNT) {};
    NfKeywordTable( const std::initializer_list<OUString> & l ) : m_keywords(l)
    {
        assert(m_keywords.size() == NF_KEYWORD_ENTRIES_COUNT);
    }

    OUString & operator[] (Keywords_t::size_type n) { return m_keywords[n]; }
    const OUString & operator[] (Keywords_t::size_type n) const { return m_keywords[n]; }

    Keywords_t::size_type size() const { return m_keywords.size(); }

    NfKeywordTable& operator=( const NfKeywordTable& r )
    {
        m_keywords = r.m_keywords;
        return *this;
    }
};

#endif // INCLUDED_SVL_NFKEYTAB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
