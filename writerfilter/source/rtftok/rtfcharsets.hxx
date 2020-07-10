/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFCHARSETS_HXX
#define INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFCHARSETS_HXX

namespace writerfilter::rtftok
{
/// RTF legacy charsets
struct RTFEncoding
{
    int charset;
    int codepage;
};
extern RTFEncoding const aRTFEncodings[];
extern int nRTFEncodings;
} // namespace writerfilter::rtftok

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFCHARSETS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
