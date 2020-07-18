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

#ifndef INCLUDED_STARMATH_SOURCE_MATHTYPE2_HXX
#define INCLUDED_STARMATH_SOURCE_MATHTYPE2_HXX

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <set>
#include <vector>
#include <types.hxx>
#include <token.hxx>
#include <stdio.h>
#include <iostream>

// Note: wikipedia has the unicode offitial document as attachment
// Mathml codes: https://elsenaju.eu/mathml/MathML-Examples.htm
// Currency symbols: https://en.wikipedia.org/wiki/Currency_symbol
// Alphanumeric symbols: https://en.wikipedia.org/wiki/Mathematical_Alphanumeric_Symbols
// Greek: https://en.wikipedia.org/wiki/Greek_alphabet#Greek_in_Unicode
// Coptic: https://en.wikipedia.org/wiki/Coptic_alphabet
// Hebrew: https://en.wikipedia.org/wiki/Unicode_and_HTML_for_the_Hebrew_alphabet
// Look for unicode: https://unicode-search.net
// Metric prefix: https://en.wikipedia.org/wiki/Metric_prefix
// Mathematical symbols: https://en.wikipedia.org/wiki/Mathematical_operators_and_symbols_in_Unicode
// Matrmatical symbols: http://xahlee.info/comp/unicode_math_operators.html
// \mathit \mathcal \mathsf \mathbb \boldsymbol \mathfrak \mathrm
// TODO add latex commands
// TODO find missing mathml charnames
// TODO add serif and sans support
// TODO add remaining currency symbols
// TODO complete unicode sections ( cirilic, arabic, ghotic, ... )
// TODO japanese numerals: https://en.wikipedia.org/wiki/Japanese_numerals
// TODO korean numerals: https://en.wikipedia.org/wiki/Korean_numerals
// TODO Attic numerals: https://en.wikipedia.org/wiki/Attic_numerals
// TODO Greek numbers: https://en.wikipedia.org/wiki/Numerals_in_Unicode
// TODO Numerals: https://en.wikipedia.org/wiki/Numerals_in_Unicode
// TODO Greek numerals: https://wiki.kidzsearch.com/wiki/Greek_numerals
// WARN undefined behaviour with NULL.
// In order to use it finish latex implementations.
// And then uncomment it's arrays.
// ¡¡¡ Chars must be listed only once !!!

namespace MathType2Namespace {

    struct MathType2 {
        sal_Unicode32 nSymbol;   // Unicode value
        SmTokenType   eType;     // Token type
        TG            nGroup;    // Token group
        const char*   pCommand;  // Starmath command
        const char*   pName;     // Starmath name
        const char*   pMlName;   // Mathml name
        const char*   plCommand; // Latex command

        inline MathType2( sal_Unicode32 cchar, SmTokenType type, TG group, const char* cmd,
                          const char* name, const char* mlname, const char* lcommand )
                        : nSymbol(cchar)
                        , eType(type)
                        , nGroup(group)
                        , pCommand(cmd)
                        , pName(name)
                        , pMlName(mlname)
                        , plCommand(lcommand)
                        { };

    };
    typedef struct MathType2 MathType2;

    /**
     * Gets mathtype2 ready for work.
     */
    extern "C" void /*__attribute__((constructor))*/ initMathtype2();

    /**
     * Clears mathtype2 stuff.
     */
    extern "C" void __attribute__((destructor)) endMathtype2();

    /**
     * Identify a char command.
     * @param cChar
     * @return command
     */
    OUString identifyCharCommand(sal_Unicode32 cChar);

    /**
     * Identify a char name.
     * @param cChar
     * @return mathmlcode
     */
    OUString identifyCharName(sal_Unicode32 cChar);

    /**
     * Identify a char mathml code.
     * @param cChar
     * @return mathmlcode
     */
    OUString identifyCharMlname(sal_Unicode32 cChar);

    /**
     * Identify a char from a command.
     * @param cmd
     * @return char
     */
    sal_Unicode32 identifyCharFromCommand(const OUString& cmd);

    /**
     * Identify a char from a charname.
     * @param charname
     * @return char
     */
    sal_Unicode32 identifyCharFromCharName(const OUString& charname);

    /**
     * Identify a char from a mathml charcode.
     * @param MlName
     * @return char
     */
    sal_Unicode32 identifyCharFromMathml(const OUString& MlName);

    /**
     * Identify a char from a char.
     * @param cmd
     * @return char data
     */
    const MathType2* identifyAndDataCharFromChar(sal_Unicode32 cChar);

    /**
     * Identify a char from a command.
     * @param cmd
     * @return char data
     */
    const MathType2* identifyAndDataCharFromCommand(const OUString& cmd);

    /**
     * Identify a char from a name.
     * @param name
     * @return char data
     */
    const MathType2* identifyAndDataCharFromName(const OUString& name);

    /**
     * Identify a char from a mathml code.
     * @param name
     * @return char data
     */
    const MathType2* identifyAndDataCharFromMathml(const OUString& name);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
