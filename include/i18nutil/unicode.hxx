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
#include <unicode/uscript.h>
#include <i18nutil/i18nutildllapi.h>

class LanguageTag;

typedef struct _ScriptTypeList {
    sal_Int16 from;
    sal_Int16 to;
    sal_Int16 value;
} ScriptTypeList;

class I18NUTIL_DLLPUBLIC unicode
{
public:

    static sal_Int16 SAL_CALL getUnicodeType( const sal_Unicode ch );
    static sal_Int16 SAL_CALL getUnicodeScriptType( const sal_Unicode ch, const ScriptTypeList *typeList = NULL, sal_Int16 unknownType = 0 );
    static sal_Unicode SAL_CALL getUnicodeScriptStart(css::i18n::UnicodeScript type);
    static sal_Unicode SAL_CALL getUnicodeScriptEnd(css::i18n::UnicodeScript type);
    static sal_uInt8 SAL_CALL getUnicodeDirection( const sal_Unicode ch );
    static bool SAL_CALL isControl( const sal_Unicode ch);
    static bool SAL_CALL isAlpha( const sal_Unicode ch);
    static bool SAL_CALL isSpace( const sal_Unicode ch);
    static bool SAL_CALL isWhiteSpace( const sal_Unicode ch);

    //Map an ISO 15924 script code to Latin/Asian/Complex/Weak
    static sal_Int16 SAL_CALL getScriptClassFromUScriptCode(UScriptCode eScript);

    //Return a language that can be written in a given ISO 15924 script code
    static OString SAL_CALL getExemplarLanguageForUScriptCode(UScriptCode eScript);

    //Format a number as a percentage according to the rules of the given
    //language, e.g. 100 -> "100%" for en-US vs "100 %" for de-DE
    static OUString SAL_CALL formatPercent(double dNumber,
        const LanguageTag &rLangTag);
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
    OUStringBuffer maOutput;
    OUStringBuffer maUtf16;
    OUStringBuffer maCombining;
    bool mbAllowMoreChars = true;
    bool mbRequiresU = false;
    bool mbIsHexString = false;

public:
    ToggleUnicodeCodepoint();

    /**
    Build an input string of valid UTF16 units to toggle.
        -do not call the other functions until the input process is complete
        -build string from Right to Left.  (Start from the character to the left of the cursor: move left.)
    */
    bool AllowMoreInput(sal_Unicode uChar);

    /**
    Validates (and potentially modifies) the input string.
        -all non-input functions must use this function to first to validate the input string
        -additional input may be prevented after this function is called
    */
    OUString StringToReplace();
    OUString ReplacementString();

    /**
    While sInput.getLength() returns the number of utf16 units to delete,
        this function returns the number of "characters" to delete - potentially a smaller number
    */
    sal_uInt32 CharsToDelete();
};

#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
