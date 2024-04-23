/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace writerfilter::rtftok
{
/// RTF legacy charsets
struct RTFEncoding
{
    int charset;
    int codepage;
};
extern RTFEncoding const aRTFEncodings[];
extern int const nRTFEncodings;

/// Font name can contain special suffixes used
/// to determine encoding for given font table entry
/// For example "Arial CE" is "Arial" with CP1250 encoding
/// List of these suffixes is not official and detected in a empirical
/// way thus may be inexact and incomplete.
struct RTFFontNameSuffix
{
    const char* suffix;
    int codepage;
};
extern RTFFontNameSuffix const aRTFFontNameSuffixes[];

} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
