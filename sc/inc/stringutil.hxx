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

#pragma once

#include <rtl/ustring.hxx>
#include "scdllapi.h"
#include <i18nlangtag/lang.h>
#include "types.hxx"

class SvNumberFormatter;
enum class SvNumFormatType : sal_Int16;

/**
 * Store parameters used in the ScDocument::SetString() method.  Various
 * options for string-setting operation are specified herein.
 */
struct SAL_WARN_UNUSED SC_DLLPUBLIC ScSetStringParam
{
    /** Enum settings that take effect if mbDetectNumberFormat=false or if
        true a number was not detected.
     */
    enum TextFormatPolicy
    {
        /**
         * Set Text number format if the input string can be parsed as a number
         * or formula text.
         */
        Always,

        /**
         * Set Text number format only when the input string is considered a
         * special number but we only want to detect a simple number.
         */
        SpecialNumberOnly,

        /**
         * Keep an existing number format, do not set Text number format and do
         * not set another number format.
         */
        Keep,

        /**
         * Never set Text number format.
         */
        Never
    };

    /**
     * Stores the pointer to the number formatter instance to be used during
     * number format detection.  The caller must manage the life cycle of the
     * instance.
     */
    SvNumberFormatter* mpNumFormatter;

    /**
     * Specify which number formats are detected:
     * mbDetectNumberFormat=true && mbDetectScientificNumberFormat=true : detect all special number formats : basic decimal number, date, scientific notation, etc
     * mbDetectNumberFormat=false && mbDetectScientificNumberFormat=true : detect scientific notation and basic decimal number, but not other special number formats (date etc)
     * mbDetectNumberFormat=false && mbDetectScientificNumberFormat=false : detect only basic decimal number, but not scientific notation or other special number formats (date etc)
     * Note: mbDetectNumberFormat=true && mbDetectScientificNumberFormat=false not allowed
     */
    bool mbDetectNumberFormat;
    bool mbDetectScientificNumberFormat;

    /**
     * Determine when to set the 'Text' number format to the cell where the
     * input string is being set.
     */
    TextFormatPolicy meSetTextNumFormat;

    /**
     * When true, treat input with a leading apostrophe as an escape character
     * for all content, to treat also numeric value as a text. When
     * false, the whole string input including the leading apostrophe will be
     * entered literally as string.
     */
    bool mbHandleApostrophe;

    sc::StartListeningType meStartListening;

    /** When true and the string results in a compiled formula, check the
        formula tokens for presence of functions that could trigger access to
        external resources. This is to be set to true in import filter code,
        but not for user input.
     */
    bool mbCheckLinkFormula;

    ScSetStringParam();

    /**
     * Call this whenever you need to unconditionally set input as text, no
     * matter what the input is.
     */
    void setTextInput();

    /**
     * Call this whenever you need to maximize the chance of input being
     * detected as a numeric value (numbers, dates, times etc).
     */
    void setNumericInput();
};

struct ScInputStringType
{
    enum StringType { Unknown = 0, Text, Formula, Number };

    StringType meType;

    OUString maText;
    double mfValue;
    SvNumFormatType mnFormatType;
};

struct ScInterpreterContext;

class ScStringUtil
{
public:

    /**
     * Check if a given string is a simple decimal number (e.g. 12.345). We
     * don't do any elaborate parsing here; we only check for the simplest
     * case of decimal number format.
     *
     * Note that preceding and trailing spaces are ignored during parsing.
     *
     * @param rStr string to parse
     * @param dsep decimal separator
     * @param gsep group separator (aka thousands separator)
     * @param dsepa decimal separator alternative, usually 0
     * @param rVal value of successfully parsed number
     *
     * @return true if the string is a valid number, false otherwise.
     */
    static bool parseSimpleNumber(
        const OUString& rStr, sal_Unicode dsep, sal_Unicode gsep, sal_Unicode dsepa, double& rVal, bool bDetectScientificNumber = true);

    static bool parseSimpleNumber(
        const char* p, size_t n, char dsep, char gsep, double& rVal);

    static OUString  SC_DLLPUBLIC GetQuotedToken(const OUString &rIn, sal_Int32 nToken, const OUString& rQuotedPairs,
                                        sal_Unicode cTok,  sal_Int32& rIndex );

    static bool SC_DLLPUBLIC isMultiline( std::u16string_view rStr );

    static ScInputStringType parseInputString(
        ScInterpreterContext& rContext, const OUString& rStr, LanguageType eLang );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
