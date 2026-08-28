/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#ifndef INCLUDED_SVL_ASIANCFG_HXX
#define INCLUDED_SVL_ASIANCFG_HXX

#include <sal/config.h>

#include <memory>
#include <cpo/uno/Sequence.hxx>
#include <svl/svldllapi.h>

namespace com::sun::star::lang {
    struct Locale;
}

/// These constants define character compression in Asian text.
/// Must match the values in com::sun::star::text::CharacterCompressionType.
/// For bonus points, also appears to be directly stored in the ww8 file format.
enum class CharCompressType {
    NONE,               /// No Compression
    PunctuationOnly,        /// Only punctuation is compressed
    PunctuationAndKana, /// Punctuation and Japanese Kana are compressed.
    Invalid = 0xff      /// only used in SC
};

class SVL_DLLPUBLIC SvxAsianConfig {
public:
    SvxAsianConfig();
    SvxAsianConfig(const SvxAsianConfig&) = delete;
    SvxAsianConfig& operator=( const SvxAsianConfig& ) = delete;

    static bool IsKerningWesternTextOnly();

    static CharCompressType GetCharDistanceCompression();

    static cpo::uno::Sequence< css::lang::Locale > GetStartEndCharLocales();

    static bool GetStartEndChars(
        css::lang::Locale const & locale, OUString & startChars,
        OUString & endChars);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
