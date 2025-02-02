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
#ifndef INCLUDED_I18NUTIL_UNICODE_HXX
#define INCLUDED_I18NUTIL_UNICODE_HXX

#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <sal/types.h>
#include <rtl/ustrbuf.hxx>
#include <unicode/uchar.h>
#include <unicode/uscript.h>
#include <i18nutil/i18nutildllapi.h>

class LanguageTag;

struct ScriptTypeList
{
    css::i18n::UnicodeScript from;
    css::i18n::UnicodeScript to;
    sal_Int16 value;
};

class I18NUTIL_DLLPUBLIC unicode
{
public:
    static sal_Int16 getUnicodeType(const sal_uInt32 ch);
    static sal_Int16 getUnicodeScriptType(const sal_Unicode ch, const ScriptTypeList* typeList,
                                          sal_Int16 unknownType = 0);
    static sal_Unicode getUnicodeScriptStart(css::i18n::UnicodeScript type);
    static sal_Unicode getUnicodeScriptEnd(css::i18n::UnicodeScript type);
    static sal_uInt8 getUnicodeDirection(const sal_Unicode ch);
    static sal_uInt32 GetMirroredChar(sal_uInt32);
    static bool isControl(const sal_uInt32 ch);
    static bool isAlpha(const sal_uInt32 ch);
    static bool isSpace(const sal_uInt32 ch);
    static bool isWhiteSpace(const sal_uInt32 ch);

    /** Check for Unicode variation sequence selectors

        @param nCode  A Unicode code point.

        @return  True if code is a Unicode variation sequence selector.
     */
    static bool isVariationSelector(sal_uInt32 nCode)
    {
        return u_getIntPropertyValue(nCode, UCHAR_VARIATION_SELECTOR) != 0;
    }

    //Map an ISO 15924 script code to Latin/Asian/Complex/Weak
    static sal_Int16 getScriptClassFromUScriptCode(UScriptCode eScript);

    //Return a language that can be written in a given ISO 15924 script code
    static OString getExemplarLanguageForUScriptCode(UScriptCode eScript);

    //Format a number as a percentage according to the rules of the given
    //language, e.g. 100 -> "100%" for en-US vs "100 %" for de-DE
    static OUString formatPercent(double dNumber, const LanguageTag& rLangTag);

    /** Map a LanguageTag's language ISO 639 code or script ISO 15924 code or
        language-script or locale to Latin/Asian/Complex/Weak. If more than one
        script is used with a language(-country) tag then the first (default)
        script is mapped for that language.

        @return a css::i18n::ScriptType value.
     */
    static sal_Int16 getScriptClassFromLanguageTag(const LanguageTag& rLanguageTag);
};

/*
    Toggle between a character and its Unicode Notation.
        -implements the concept found in Microsoft Word's Alt-X
        -accepts sequences of up to 8 hex characters and converts into the corresponding Unicode Character
            -example:  0000A78c   or   2bc
        -accepts sequences of up to 256 characters in Unicode notation
            -example:  U+00000065u+0331u+308
        -handles complex characters (with combining elements) and the all of the Unicode planes.
*/
class I18NUTIL_DLLPUBLIC ToggleUnicodeCodepoint
{
private:
    OUStringBuffer maInput;
    OUStringBuffer maUtf16;
    OUStringBuffer maCombining;
    bool mbAllowMoreChars = true;
    bool mbRequiresU = false;
    bool mbIsHexString = false;

public:
    /**
    Build an input string of valid UTF16/UCS4 units to toggle.
        -do not call the other functions until the input process is complete
        -build string from Right to Left.  (Start from the character to the left of the cursor: move left.)
        - accepted input:
          - a sequence of 2 to 8 hex characters not preceded by U+, to convert to Unicode;
          - a sequence of up to 256 concatenated U+ notation - like u+xxxxU+yyyy, where xxxx and
            yyyy are sequences of 2 to 8 hexadecimal digits - to convert it all to Unicode;
          - a single (maybe combined) "symbol" - i.e., one or several codepoints that constitute
            one glyph - to convert from Unicode to U+ notation.
    */
    bool AllowMoreInput(sal_uInt32 uChar);

    /**
    Validates (and potentially modifies) the input string.
        -all non-input functions must use this function to first to validate the input string
        -additional input may be prevented after this function is called
    */
    OUString StringToReplace();
    OUString ReplacementString();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
