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

#include <formula/grammar.hxx>
#include <tools/debug.hxx>
#include <cassert>

namespace formula {

FormulaGrammar::Grammar FormulaGrammar::mapAPItoGrammar( const bool bEnglish, const bool bXML )
{
    Grammar eGrammar;
    if (bEnglish && bXML)
        eGrammar = GRAM_PODF;
    else if (bEnglish && !bXML)
        eGrammar = GRAM_API;
    else if (!bEnglish && bXML)
        eGrammar = GRAM_NATIVE_ODF;
    else // (!bEnglish && !bXML)
        eGrammar = GRAM_NATIVE;
    return eGrammar;
}

bool FormulaGrammar::isSupported( const Grammar eGrammar )
{
    switch (eGrammar)
    {
        case GRAM_ODFF           :
        case GRAM_PODF           :
        case GRAM_ENGLISH        :
        case GRAM_NATIVE         :
        case GRAM_ODFF_UI        :
        case GRAM_ODFF_A1        :
        case GRAM_PODF_UI        :
        case GRAM_PODF_A1        :
        case GRAM_NATIVE_UI      :
        case GRAM_NATIVE_ODF     :
        case GRAM_NATIVE_XL_A1   :
        case GRAM_NATIVE_XL_R1C1 :
        case GRAM_ENGLISH_XL_A1  :
        case GRAM_ENGLISH_XL_R1C1:
        case GRAM_ENGLISH_XL_OOX :
        case GRAM_OOXML          :
        case GRAM_CHART_OOXML    :
        case GRAM_API            :
            return true;
        default:
            return extractFormulaLanguage( eGrammar) == GRAM_EXTERNAL;
    }
}

FormulaGrammar::Grammar FormulaGrammar::setEnglishBit( const Grammar eGrammar, const bool bEnglish )
{
    if (bEnglish)
        return static_cast<Grammar>( eGrammar | kEnglishBit);
    else
        return static_cast<Grammar>( eGrammar & ~kEnglishBit);
}

FormulaGrammar::Grammar FormulaGrammar::mergeToGrammar( const Grammar eGrammar, const AddressConvention eConv )
{
    bool bEnglish = isEnglish( eGrammar);
    Grammar eGram = static_cast<Grammar>(
            extractFormulaLanguage( eGrammar) |
            ((eConv + kConventionOffset) << kConventionShift));
    eGram = setEnglishBit( eGram, bEnglish);
    assert( isSupported( eGram));
    return eGram;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
