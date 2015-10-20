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

#ifndef INCLUDED_FORMULA_GRAMMAR_HXX
#define INCLUDED_FORMULA_GRAMMAR_HXX

#include <com/sun/star/sheet/FormulaLanguage.hpp>
#include <formula/formuladllapi.h>

namespace formula
{

/** Grammars digested by ScCompiler.
 */
class FORMULA_DLLPUBLIC FormulaGrammar
{
public:
    enum AddressConvention{
        CONV_UNSPECIFIED = -1,  /* useful when we want method to chose, must be first */

        /* elements must be sequential and changes should be reflected in ScCompiler::pCharTables */
        CONV_OOO     =  0,  /* 'doc'#sheet.A1:sheet2.B2 */
        CONV_ODF,           /* ['doc'#sheet.A1:sheet2.B2] */
        CONV_XL_A1,         /* [doc]sheet:sheet2!A1:B2 */
        CONV_XL_R1C1,       /* [doc]sheet:sheet2!R1C1:R2C2 */
        CONV_XL_OOX,        /* [#]sheet:sheet2!A1:B2 */

        CONV_LOTUS_A1,      /* external? 3d? A1.B2 <placeholder/> */

        CONV_LAST,   /* for loops, must always be last */

        // not a real address convention, a special case for INDIRECT function interpretation
        // only -> try using CONV_OOO, failing that CONV_XL_A1
        CONV_A1_XL_A1
    };

    //! CONV_UNSPECIFIED is a negative value!
    static const int kConventionOffset = - CONV_UNSPECIFIED + 1;
    // Room for 32k hypothetical languages plus EXTERNAL.
    static const int kConventionShift  = 16;
    // Room for 256 reference conventions.
    static const int kEnglishBit       = (1 << (kConventionShift + 8));
    // Mask off all non-language bits.
    static const int kFlagMask         = ~((~unsigned(0)) << kConventionShift);

    /** Values encoding the formula language plus address reference convention
        plus English parsing/formatting
     */
    //! When adding new values adapt isSupported() below as well.
    enum Grammar
    {
        /// Used only in ScCompiler ctor and in some XML import API context.
        GRAM_UNSPECIFIED    = -1,
        /// ODFF with default ODF A1 bracketed references.
        GRAM_ODFF           = css::sheet::FormulaLanguage::ODFF                 |
                                ((CONV_ODF           +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// ODF 1.1 with default ODF A1 bracketed references.
        GRAM_PODF           = css::sheet::FormulaLanguage::ODF_11               |
                                ((CONV_ODF           +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// English with default A1 reference style.
        GRAM_ENGLISH        = css::sheet::FormulaLanguage::ENGLISH              |
                                ((CONV_OOO           +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// Native with default A1 reference style.
        GRAM_NATIVE         = css::sheet::FormulaLanguage::NATIVE               |
                                ((CONV_OOO           +
                                  kConventionOffset) << kConventionShift),
        /// ODFF with reference style as set in UI, may be A1 or R1C1.
        GRAM_ODFF_UI        = css::sheet::FormulaLanguage::ODFF                 |
                                ((CONV_UNSPECIFIED   +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// ODFF with A1 reference style, unbracketed.
        GRAM_ODFF_A1        = css::sheet::FormulaLanguage::ODFF                 |
                                ((CONV_OOO           +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// ODF 1.1 with reference style as set in UI, may be A1 or R1C1.
        GRAM_PODF_UI        = css::sheet::FormulaLanguage::ODF_11               |
                                ((CONV_UNSPECIFIED   +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// ODF 1.1 with A1 reference style, unbracketed.
        GRAM_PODF_A1        = css::sheet::FormulaLanguage::ODF_11               |
                                ((CONV_OOO           +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// Native with reference style as set in UI, may be A1 or R1C1.
        GRAM_NATIVE_UI      = css::sheet::FormulaLanguage::NATIVE               |
                                ((CONV_UNSPECIFIED   +
                                  kConventionOffset) << kConventionShift),
        /// Native with ODF A1 bracketed references. Not very useful but supported.
        GRAM_NATIVE_ODF     = css::sheet::FormulaLanguage::NATIVE               |
                                ((CONV_ODF           +
                                  kConventionOffset) << kConventionShift),
        /// Native with Excel A1 reference style.
        GRAM_NATIVE_XL_A1   = css::sheet::FormulaLanguage::NATIVE               |
                                ((CONV_XL_A1         +
                                  kConventionOffset) << kConventionShift),
        /// Native with Excel R1C1 reference style.
        GRAM_NATIVE_XL_R1C1 = css::sheet::FormulaLanguage::NATIVE               |
                                ((CONV_XL_R1C1       +
                                  kConventionOffset) << kConventionShift),
        /// English with Excel A1 reference style.
        GRAM_ENGLISH_XL_A1   = css::sheet::FormulaLanguage::XL_ENGLISH               |
                                ((CONV_XL_A1         +
                                  kConventionOffset) << kConventionShift)            |
                                kEnglishBit,
        /// English with Excel R1C1 reference style.
        GRAM_ENGLISH_XL_R1C1 = css::sheet::FormulaLanguage::XL_ENGLISH               |
                                ((CONV_XL_R1C1       +
                                  kConventionOffset) << kConventionShift)            |
                                kEnglishBit,
        /// English with Excel OOXML reference style.
        GRAM_ENGLISH_XL_OOX  = css::sheet::FormulaLanguage::XL_ENGLISH               |
                                ((CONV_XL_OOX        +
                                  kConventionOffset) << kConventionShift)            |
                                kEnglishBit,
        /// Excel OOXML with Excel OOXML reference style.
        GRAM_OOXML           = css::sheet::FormulaLanguage::OOXML               |
                                ((CONV_XL_OOX        +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// Central definition of the default grammar to be used.
        GRAM_DEFAULT        = GRAM_NATIVE_UI,

        /// Central definition of the default storage grammar to be used.
        GRAM_STORAGE_DEFAULT = GRAM_ODFF,

        /** OpCodeMap set by external filter and merged with reference
            convention plus English bit on top. Plain value acts as
            FormulaLanguage. */
        GRAM_EXTERNAL       = (1 << (kConventionShift - 1))
    };

    /// If English parsing/formatting is associated with a grammar.
    static inline bool isEnglish( const Grammar eGrammar )
    {
        return (eGrammar & kEnglishBit) != 0;
    }

    /** Compatibility helper for old "bCompileEnglish, bCompileXML" API calls
        to obtain the new grammar. */
    static Grammar mapAPItoGrammar( const bool bEnglish, const bool bXML );

    static bool isSupported( const Grammar eGrammar );

    static inline sal_Int32 extractFormulaLanguage( const Grammar eGrammar )
    {
        return eGrammar & kFlagMask;
    }

    static inline AddressConvention extractRefConvention( const Grammar eGrammar )
    {
        return static_cast<AddressConvention>(
                ((eGrammar & ~kEnglishBit) >> kConventionShift) -
                kConventionOffset);
    }

    static Grammar setEnglishBit( const Grammar eGrammar, const bool bEnglish );

    static Grammar mergeToGrammar( const Grammar eGrammar, const AddressConvention eConv );

    /// If grammar is of ODF 1.1
    static inline bool isPODF( const Grammar eGrammar )
    {
        return extractFormulaLanguage( eGrammar) ==
            css::sheet::FormulaLanguage::ODF_11;
    }

    /// If grammar is of ODFF
    static inline bool isODFF( const Grammar eGrammar )
    {
        return extractFormulaLanguage( eGrammar) ==
            css::sheet::FormulaLanguage::ODFF;
    }

    /// If grammar is of OOXML
    static inline bool isOOXML( const Grammar eGrammar )
    {
        return extractFormulaLanguage( eGrammar) ==
            css::sheet::FormulaLanguage::OOXML;
    }

};

} // formula


#endif // INCLUDED_FORMULA_GRAMMAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
