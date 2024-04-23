/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfcharsets.hxx"

#include <iterator>

#include <rtl/textenc.h>

namespace writerfilter::rtftok
{
// See RTF spec v1.9.1, page 19
RTFEncoding const aRTFEncodings[] = {
    // charset  codepage    Windows / Mac name
    { 0, 1252 }, // ANSI
    { 1, 0 }, // Default
    { 2, 42 }, // Symbol
    { 77, 10000 }, // Mac Roman
    { 78, 10001 }, // Mac Shift Jis
    { 79, 10003 }, // Mac Hangul
    { 80, 10008 }, // Mac GB2312
    { 81, 10002 }, // Mac Big5
    { 83, 10005 }, // Mac Herbrew
    { 84, 10004 }, // Mac Arabic
    { 85, 10006 }, // Mac Greek
    { 86, 10081 }, // Mac Turkish
    { 87, 10021 }, // Mac Thai
    { 88, 10029 }, // Mac East Europe
    { 89, 10007 }, // Mac Russian
    { 128, 932 }, // Shift JIS
    { 129, 949 }, // Hangul
    { 130, 1361 }, // Johab
    { 134, 936 }, // GB2312
    { 136, 950 }, // Big5
    { 161, 1253 }, // Greek
    { 162, 1254 }, // Turkish
    { 163, 1258 }, // Vietnamese
    { 177, 1255 }, // Herbrew
    { 178, 1256 }, // Arabic
    { 186, 1257 }, // Baltic
    { 204, 1251 }, // Russian
    { 222, 874 }, // Thai
    { 238, 1250 }, // Eastern European
    { 254, 437 }, // PC 437
    { 255, 850 }, // OEM
};

int const nRTFEncodings = std::size(aRTFEncodings);

RTFFontNameSuffix const aRTFFontNameSuffixes[] = {
    { "Baltic", RTL_TEXTENCODING_MS_1257 },   { "CE", RTL_TEXTENCODING_MS_1250 },
    { "Cyr", RTL_TEXTENCODING_MS_1251 },      { "Greek", RTL_TEXTENCODING_MS_1253 },
    { "Tur", RTL_TEXTENCODING_MS_1254 },      { "(Hebrew)", RTL_TEXTENCODING_MS_1255 },
    { "(Arabic)", RTL_TEXTENCODING_MS_1256 }, { "(Vietnamese)", RTL_TEXTENCODING_MS_1258 },
    { "", RTL_TEXTENCODING_DONTKNOW } // End of array
};

} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
