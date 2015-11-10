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
#ifndef INCLUDED_SC_SOURCE_UI_INC_SPELLPARAM_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SPELLPARAM_HXX

#include <vcl/font.hxx>

/** Specifiers for sheet conversion (functions iterating over the sheet and modifying cells). */
enum ScConversionType
{
    SC_CONVERSION_SPELLCHECK,       /// Spell checker.
    SC_CONVERSION_HANGULHANJA,      /// Hangul-Hanja converter.
    SC_CONVERSION_CHINESE_TRANSL    /// Chinese simplified/traditional converter.
};

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
                            const vcl::Font& rTargetFont,
                            sal_Int32 nOptions,
                            bool bIsInteractive );

    inline ScConversionType GetType() const     { return meConvType; }
    inline LanguageType GetSourceLang() const   { return meSourceLang; }
    inline LanguageType GetTargetLang() const   { return meTargetLang; }
    inline const vcl::Font*  GetTargetFont() const   { return mbUseTargetFont ? &maTargetFont : nullptr; }
    inline sal_Int32    GetOptions() const      { return mnOptions; }
    inline bool         IsInteractive() const   { return mbIsInteractive; }

private:
    ScConversionType    meConvType;         /// Type of the conversion.
    LanguageType        meSourceLang;       /// Source language for conversion.
    LanguageType        meTargetLang;       /// Target language for conversion.
    vcl::Font          maTargetFont;       /// Target font to be used if language has to be changed.
    sal_Int32           mnOptions;          /// Conversion options.
    bool                mbUseTargetFont;    /// True = Use maTargetFont to change font during conversion.
    bool                mbIsInteractive;    /// True = Text conversion has (specific) dialog that may be raised.
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
