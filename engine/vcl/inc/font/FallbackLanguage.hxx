/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/dllapi.h>

namespace vcl::font
{
// true for a code point whose script is not written in the language of
// rLangTag, so the language says nothing about which font to pick for it
VCL_DLLPUBLIC bool isImpossibleCodePointForLang(const LanguageTag& rLangTag, sal_uInt32 nChar);

// true for an emoji code point
VCL_DLLPUBLIC bool isEmoji(sal_uInt32 nChar);

// a BCP 47 tag of a language typically written with the script of the code
// point, qualified enough for a font lookup by language
VCL_DLLPUBLIC OUString getExemplarLangTagForCodePoint(sal_uInt32 nChar);

// the language to look a fallback font up by for the missing chars: the
// language of rLangTag, unless one of the chars is never written in it, then
// a language guessed from that char
VCL_DLLPUBLIC LanguageTag getLanguageForMissingChars(const LanguageTag& rLangTag,
                                                     const OUString& rMissingChars);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
