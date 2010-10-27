/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SC_SPELLPARAM_HXX
#define SC_SPELLPARAM_HXX

#include <vcl/font.hxx>

// ============================================================================

/** Specifiers for sheet conversion (functions iterating over the sheet and modifying cells). */
enum ScConversionType
{
    SC_CONVERSION_SPELLCHECK,       /// Spell checker.
    SC_CONVERSION_HANGULHANJA,      /// Hangul-Hanja converter.
    SC_CONVERSION_CHINESE_TRANSL    /// Chinese simplified/traditional converter.
};

// ---------------------------------------------------------------------------

/** Parameters for conversion. */
class ScConversionParam
{
public:
    /** Constructs an empty parameter struct with the passed conversion type. */
    explicit            ScConversionParam( ScConversionType eConvType );

    /** Constructs parameter struct for text conversion without changing the language. */
    explicit            ScConversionParam(
                            ScConversionType eConvType,
                            LanguageType eLang,
                            sal_Int32 nOptions,
                            bool bIsInteractive );

    /** Constructs parameter struct for text conversion with language change. */
    explicit            ScConversionParam(
                            ScConversionType eConvType,
                            LanguageType eSourceLang,
                            LanguageType eTargetLang,
                            const Font& rTargetFont,
                            sal_Int32 nOptions,
                            bool bIsInteractive );

    inline ScConversionType GetType() const     { return meConvType; }
    inline LanguageType GetSourceLang() const   { return meSourceLang; }
    inline LanguageType GetTargetLang() const   { return meTargetLang; }
    inline const Font*  GetTargetFont() const   { return mbUseTargetFont ? &maTargetFont : 0; }
    inline sal_Int32    GetOptions() const      { return mnOptions; }
    inline bool         IsInteractive() const   { return mbIsInteractive; }

private:
    ScConversionType    meConvType;         /// Type of the conversion.
    LanguageType        meSourceLang;       /// Source language for conversion.
    LanguageType        meTargetLang;       /// Target language for conversion.
    Font                maTargetFont;       /// Target font to be used if language has to be changed.
    sal_Int32           mnOptions;          /// Conversion options.
    bool                mbUseTargetFont;    /// True = Use maTargetFont to change font during conversion.
    bool                mbIsInteractive;    /// True = Text conversion has (specific) dialog that may be raised.
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
