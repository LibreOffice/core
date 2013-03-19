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

#ifndef SC_STRINGUTIL_HXX
#define SC_STRINGUTIL_HXX

#include "rtl/ustring.hxx"
#include "tools/string.hxx"
#include "scdllapi.h"

class SvNumberFormatter;

/**
 * Store parameters used in the ScDocument::SetString() method.  Various
 * options for string-setting operation are specified herein.
 */
struct SC_DLLPUBLIC ScSetStringParam
{
    enum TextFormatPolicy
    {
        /**
         * Set Text number format no matter what the input string is.
         */
        Always,

        /**
         * Set Text number format only when the input string is considered a
         * special number but we only want to detect a simple number.
         */
        SpecialNumberOnly,

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
     * When true, we try to detect special number format (dates etc) from the
     * input string, when false, we only try to detect a basic decimal number
     * format.
     */
    bool mbDetectNumberFormat;

    /**
     * Determine when to set the 'Text' number format to the cell where the
     * input string is being set.
     */
    TextFormatPolicy meSetTextNumFormat;

    /**
     * When true, treat input with a leading apostrophe / single quote special
     * in that it escapes numeric or date/time input such that it is not
     * interpreted and the input string is taken instead. This can be used
     * during text file import so the leading apostrophe is not lost if it
     * precedes a numeric value.
     */
    bool mbHandleApostrophe;

    ScSetStringParam();
};

// ============================================================================

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
     * @param rVal value of successfully parsed number
     *
     * @return true if the string is a valid number, false otherwise.
     */
    static bool parseSimpleNumber(
        const ::rtl::OUString& rStr, sal_Unicode dsep, sal_Unicode gsep, double& rVal);

    static sal_Int32 SC_DLLPUBLIC GetQuotedTokenCount(const OUString &rIn, const OUString& rQuotedPairs, sal_Unicode cTok = ';' );
    static OUString  SC_DLLPUBLIC GetQuotedToken(const OUString &rIn, sal_Int32 nToken, const OUString& rQuotedPairs,
                                        sal_Unicode cTok,  sal_Int32& rIndex );

    static bool SC_DLLPUBLIC isMultiline( const OUString& rStr );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
