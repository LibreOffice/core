/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: grammar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:17:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_GRAMMAR_HXX
#define SC_GRAMMAR_HXX

#ifndef _COM_SUN_STAR_SHEET_FORMULALANGUAGE_HPP_
#include "com/sun/star/sheet/FormulaLanguage.hpp"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

/** Grammars digested by ScCompiler.
 */
class ScGrammar
{
public:

    //! ScAddress::CONV_UNSPECIFIED is a negative value!
    static const int kConventionOffset = - ScAddress::CONV_UNSPECIFIED + 1;
    // Room for 32k hypothetical languages plus EXTERNAL.
    static const int kConventionShift  = 16;
    // Room for 256 reference conventions.
    static const int kEnglishBit       = (1 << (kConventionShift + 8));
    // Mask off all non-language bits.
    static const int kFlagMask         = ~((~int(0)) << kConventionShift);

    /** Values encoding the formula language plus address reference convention
        plus English parsing/formatting
     */
    //! When adding new values adapt isSupported() below as well.
    enum Grammar
    {
        /// Used only in ScCompiler ctor and in some XML import API context.
        GRAM_UNSPECIFIED    = -1,
        /// ODFF with default ODF A1 bracketed references.
        GRAM_ODFF           = ::com::sun::star::sheet::FormulaLanguage::ODFF    |
                                ((ScAddress::CONV_ODF           +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// ODF 1.1 with default ODF A1 bracketed references.
        GRAM_PODF           = ::com::sun::star::sheet::FormulaLanguage::ODF_11  |
                                ((ScAddress::CONV_ODF           +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// English with default A1 reference style.
        GRAM_ENGLISH        = ::com::sun::star::sheet::FormulaLanguage::ENGLISH |
                                ((ScAddress::CONV_OOO           +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// Native with default A1 reference style.
        GRAM_NATIVE         = ::com::sun::star::sheet::FormulaLanguage::NATIVE  |
                                ((ScAddress::CONV_OOO           +
                                  kConventionOffset) << kConventionShift),
        /// ODFF with reference style as set in UI, may be A1 or R1C1.
        GRAM_ODFF_UI        = ::com::sun::star::sheet::FormulaLanguage::ODFF    |
                                ((ScAddress::CONV_UNSPECIFIED   +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// ODFF with A1 reference style, unbracketed.
        GRAM_ODFF_A1        = ::com::sun::star::sheet::FormulaLanguage::ODFF    |
                                ((ScAddress::CONV_OOO           +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// ODF 1.1 with reference style as set in UI, may be A1 or R1C1.
        GRAM_PODF_UI        = ::com::sun::star::sheet::FormulaLanguage::ODF_11  |
                                ((ScAddress::CONV_UNSPECIFIED   +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// ODF 1.1 with A1 reference style, unbracketed.
        GRAM_PODF_A1        = ::com::sun::star::sheet::FormulaLanguage::ODF_11  |
                                ((ScAddress::CONV_OOO           +
                                  kConventionOffset) << kConventionShift)       |
                                kEnglishBit,
        /// Native with reference style as set in UI, may be A1 or R1C1.
        GRAM_NATIVE_UI      = ::com::sun::star::sheet::FormulaLanguage::NATIVE  |
                                ((ScAddress::CONV_UNSPECIFIED   +
                                  kConventionOffset) << kConventionShift),
        /// Native with ODF A1 bracketed references. Not very useful but supported.
        GRAM_NATIVE_ODF     = ::com::sun::star::sheet::FormulaLanguage::NATIVE  |
                                ((ScAddress::CONV_ODF           +
                                  kConventionOffset) << kConventionShift),
        /// Native with Excel A1 reference style.
        GRAM_NATIVE_XL_A1   = ::com::sun::star::sheet::FormulaLanguage::NATIVE  |
                                ((ScAddress::CONV_XL_A1         +
                                  kConventionOffset) << kConventionShift),
        /// Native with Excel R1C1 reference style.
        GRAM_NATIVE_XL_R1C1 = ::com::sun::star::sheet::FormulaLanguage::NATIVE  |
                                ((ScAddress::CONV_XL_R1C1       +
                                  kConventionOffset) << kConventionShift),
        /// Central definition of the default grammar to be used.
        GRAM_DEFAULT        = GRAM_NATIVE_UI,

        /** Central definition of the default storage grammar to be used.
            If GRAM_PODF switch this to GRAM_ODFF when we're ready. */
        GRAM_STORAGE_DEFAULT = GRAM_PODF,

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
    static Grammar mapAPItoGrammar( const bool bEnglish, const bool bXML )
    {
        Grammar eGrammar;
        if (bEnglish && bXML)
            eGrammar = GRAM_PODF;
        else if (bEnglish && !bXML)
            eGrammar = GRAM_PODF_A1;
        else if (!bEnglish && bXML)
            eGrammar = GRAM_NATIVE_ODF;
        else // (!bEnglish && !bXML)
            eGrammar = GRAM_NATIVE;
        return eGrammar;
    }

    static bool isSupported( const Grammar eGrammar )
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
                return true;
            default:
                return extractFormulaLanguage( eGrammar) == GRAM_EXTERNAL;
        }
    }

    static inline sal_Int32 extractFormulaLanguage( const Grammar eGrammar )
    {
        return eGrammar & kFlagMask;
    }

    static inline ScAddress::Convention extractRefConvention( const Grammar eGrammar )
    {
        return static_cast<ScAddress::Convention>(
                ((eGrammar & ~kEnglishBit) >> kConventionShift) -
                kConventionOffset);
    }

    static inline Grammar setEnglishBit( const Grammar eGrammar, const bool bEnglish )
    {
        if (bEnglish)
            return static_cast<Grammar>( eGrammar | kEnglishBit);
        else
            return static_cast<Grammar>( eGrammar & ~kEnglishBit);
    }

    static inline Grammar mergeToGrammar( const Grammar eGrammar, const ScAddress::Convention eConv )
    {
        bool bEnglish = isEnglish( eGrammar);
        Grammar eGram = static_cast<Grammar>(
                extractFormulaLanguage( eGrammar) |
                ((eConv + kConventionOffset) << kConventionShift));
        eGram = setEnglishBit( eGram, bEnglish);
        DBG_ASSERT( isSupported( eGram), "ScCompilerGrammarMap::mergeToGrammar: unsupported grammar");
        return eGram;
    }

    /// If grammar is of ODF 1.1
    static inline bool isPODF( const Grammar eGrammar )
    {
        return extractFormulaLanguage( eGrammar) ==
            ::com::sun::star::sheet::FormulaLanguage::ODF_11;
    }

    /// If grammar is of ODFF
    static inline bool isODFF( const Grammar eGrammar )
    {
        return extractFormulaLanguage( eGrammar) ==
            ::com::sun::star::sheet::FormulaLanguage::ODFF;
    }

};

#endif // SC_GRAMMAR_HXX
