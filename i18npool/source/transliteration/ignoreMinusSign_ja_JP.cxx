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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_MinusSign_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreMinusSign_ja_JP_translator (const sal_Unicode c)
{
    switch (c) {
        case 0x2212: // MINUS SIGN
        case 0x002d: // HYPHEN-MINUS
        case 0x2010: // HYPHEN
        case 0x2011: // NON-BREAKING HYPHEN
        case 0x2012: // FIGURE DASH
        case 0x2013: // EN DASH
        case 0x2014: // EM DASH
        case 0x2015: // HORIZONTAL BAR
        case 0xff0d: // FULLWIDTH HYPHEN-MINUS
        case 0xff70: // HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK
        return 0x30fc; // KATAKANA-HIRAGANA PROLONGED SOUND MARK
    }
    return c;
}

ignoreMinusSign_ja_JP::ignoreMinusSign_ja_JP()
{
        func = ignoreMinusSign_ja_JP_translator;
        table = 0;
        map = 0;
        transliterationName = "ignoreMinusSign_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreMinusSign_ja_JP";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
