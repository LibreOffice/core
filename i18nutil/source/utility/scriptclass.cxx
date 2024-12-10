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

#include <config_locales.h>

#include <i18nutil/scriptclass.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unicode/uchar.h>
#include <i18nutil/unicode.hxx>
#include <o3tl/string_view.hxx>

#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;

namespace i18nutil
{
namespace
{
sal_Int16 getScriptClassByUAX24Script(sal_uInt32 currentChar)
{
    int32_t script = u_getIntPropertyValue(currentChar, UCHAR_SCRIPT);
    return unicode::getScriptClassFromUScriptCode(static_cast<UScriptCode>(script));
}

struct UBlock2Script
{
    UBlockCode from;
    UBlockCode to;
    sal_Int16 script;
};

const UBlock2Script scriptList[] = {
    { UBLOCK_NO_BLOCK, UBLOCK_NO_BLOCK, ScriptType::WEAK },
    { UBLOCK_BASIC_LATIN, UBLOCK_SPACING_MODIFIER_LETTERS, ScriptType::LATIN },
    { UBLOCK_GREEK, UBLOCK_ARMENIAN, ScriptType::LATIN },
    { UBLOCK_HEBREW, UBLOCK_MYANMAR, ScriptType::COMPLEX },
    { UBLOCK_GEORGIAN, UBLOCK_GEORGIAN, ScriptType::LATIN },
    { UBLOCK_HANGUL_JAMO, UBLOCK_HANGUL_JAMO, ScriptType::ASIAN },
    { UBLOCK_ETHIOPIC, UBLOCK_ETHIOPIC, ScriptType::COMPLEX },
    { UBLOCK_CHEROKEE, UBLOCK_RUNIC, ScriptType::LATIN },
    { UBLOCK_KHMER, UBLOCK_MONGOLIAN, ScriptType::COMPLEX },
    { UBLOCK_LATIN_EXTENDED_ADDITIONAL, UBLOCK_GREEK_EXTENDED, ScriptType::LATIN },
    { UBLOCK_NUMBER_FORMS, UBLOCK_NUMBER_FORMS, ScriptType::WEAK },
    { UBLOCK_CJK_RADICALS_SUPPLEMENT, UBLOCK_HANGUL_SYLLABLES, ScriptType::ASIAN },
    { UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS, UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS, ScriptType::ASIAN },
    { UBLOCK_ARABIC_PRESENTATION_FORMS_A, UBLOCK_ARABIC_PRESENTATION_FORMS_A, ScriptType::COMPLEX },
    { UBLOCK_CJK_COMPATIBILITY_FORMS, UBLOCK_CJK_COMPATIBILITY_FORMS, ScriptType::ASIAN },
    { UBLOCK_ARABIC_PRESENTATION_FORMS_B, UBLOCK_ARABIC_PRESENTATION_FORMS_B, ScriptType::COMPLEX },
    { UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS, UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS,
      ScriptType::ASIAN },
    { UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B, UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT,
      ScriptType::ASIAN },
    { UBLOCK_CJK_STROKES, UBLOCK_CJK_STROKES, ScriptType::ASIAN },
    { UBLOCK_LATIN_EXTENDED_C, UBLOCK_LATIN_EXTENDED_D, ScriptType::LATIN }
};

#define scriptListCount SAL_N_ELEMENTS(scriptList)

//always sets rScriptType

//returns true for characters historically explicitly assigned to
//latin/weak/asian

//returns false for characters that historically implicitly assigned to
//weak as unknown
bool getCompatibilityScriptClassByBlock(sal_uInt32 currentChar, sal_Int16& rScriptType)
{
    bool bKnown = true;
    //handle specific characters always as weak:
    //  0x01 - this breaks a word
    //  0x02 - this can be inside a word
    //  0x20 & 0xA0 - Bug 102975, declare western space and non-break space as WEAK char.
    if (0x01 == currentChar || 0x02 == currentChar || 0x20 == currentChar || 0xA0 == currentChar)
        rScriptType = ScriptType::WEAK;
    // Few Spacing Modifier Letters that can be Bopomofo tonal marks.
    else if (0x2CA == currentChar || 0x2CB == currentChar || 0x2C7 == currentChar
             || 0x2D9 == currentChar)
        rScriptType = ScriptType::WEAK;
    // tdf#52577 superscript numbers should be we weak.
    else if (0xB2 == currentChar || 0xB3 == currentChar || 0xB9 == currentChar)
        rScriptType = ScriptType::WEAK;
    // workaround for Coptic
    else if (0x2C80 <= currentChar && 0x2CE3 >= currentChar)
        rScriptType = ScriptType::LATIN;
    else
    {
        UBlockCode block = ublock_getCode(currentChar);
        size_t i = 0;
        while (i < scriptListCount)
        {
            if (block <= scriptList[i].to)
                break;
            ++i;
        }
        if (i < scriptListCount && block >= scriptList[i].from)
            rScriptType = scriptList[i].script;
        else
        {
            rScriptType = ScriptType::WEAK;
            bKnown = false;
        }
    }
    return bKnown;
}
}
}

sal_Int16 i18nutil::GetScriptClass(sal_uInt32 currentChar)
{
    sal_Int16 nRet = ScriptType::WEAK;

    if (!getCompatibilityScriptClassByBlock(currentChar, nRet))
    {
        nRet = getScriptClassByUAX24Script(currentChar);
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
