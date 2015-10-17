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

#ifndef INCLUDED_UNOTOOLS_SEARCHOPT_HXX
#define INCLUDED_UNOTOOLS_SEARCHOPT_HXX

#include <unotools/unotoolsdllapi.h>

class SvtSearchOptions_Impl;

class UNOTOOLS_DLLPUBLIC SvtSearchOptions
{
    SvtSearchOptions_Impl   *pImpl;

    SvtSearchOptions( const SvtSearchOptions & ) = delete;
    SvtSearchOptions & operator = ( const SvtSearchOptions & ) = delete;

public:
    SvtSearchOptions();
    ~SvtSearchOptions();

    void Commit();

    sal_Int32   GetTransliterationFlags() const;

    // General Options

    bool    IsWholeWordsOnly() const;
    bool    IsBackwards() const;
    bool    IsUseRegularExpression() const;
    bool    IsSimilaritySearch() const;
    bool    IsUseAsianOptions() const;
    bool    IsMatchCase() const;            // also Japanese search option
    bool    IsSearchFormatted() const;
    bool    IsNotes() const;

    void    SetWholeWordsOnly( bool bVal );
    void    SetBackwards( bool bVal );
    void    SetUseRegularExpression( bool bVal );
    void    SetSearchForStyles( bool bVal );
    void    SetSimilaritySearch( bool bVal );
    void    SetUseAsianOptions( bool bVal );
    void    SetMatchCase( bool bVal );      // also Japanese search option
    void    SetSearchFormatted( bool bVal );
    void    SetNotes( bool bVal);

    // Japanese search options

    bool    IsMatchFullHalfWidthForms() const;
    bool    IsMatchHiraganaKatakana() const;
    bool    IsMatchContractions() const;
    bool    IsMatchMinusDashChoon() const;
    bool    IsMatchRepeatCharMarks() const;
    bool    IsMatchVariantFormKanji() const;
    bool    IsMatchOldKanaForms() const;
    bool    IsMatchDiziDuzu() const;
    bool    IsMatchBavaHafa() const;
    bool    IsMatchTsithichiDhizi() const;
    bool    IsMatchHyuiyuByuvyu() const;
    bool    IsMatchSesheZeje() const;
    bool    IsMatchIaiya() const;
    bool    IsMatchKiku() const;
    bool    IsIgnorePunctuation() const;
    bool    IsIgnoreWhitespace() const;
    bool    IsIgnoreProlongedSoundMark() const;
    bool    IsIgnoreMiddleDot() const;
    bool    IsIgnoreDiacritics_CTL() const;
    bool    IsIgnoreKashida_CTL() const;

    void    SetMatchFullHalfWidthForms( bool bVal );
    void    SetMatchHiraganaKatakana( bool bVal );
    void    SetMatchContractions( bool bVal );
    void    SetMatchMinusDashChoon( bool bVal );
    void    SetMatchRepeatCharMarks( bool bVal );
    void    SetMatchVariantFormKanji( bool bVal );
    void    SetMatchOldKanaForms( bool bVal );
    void    SetMatchDiziDuzu( bool bVal );
    void    SetMatchBavaHafa( bool bVal );
    void    SetMatchTsithichiDhizi( bool bVal );
    void    SetMatchHyuiyuByuvyu( bool bVal );
    void    SetMatchSesheZeje( bool bVal );
    void    SetMatchIaiya( bool bVal );
    void    SetMatchKiku( bool bVal );
    void    SetIgnorePunctuation( bool bVal );
    void    SetIgnoreWhitespace( bool bVal );
    void    SetIgnoreProlongedSoundMark( bool bVal );
    void    SetIgnoreMiddleDot( bool bVal );
    void    SetIgnoreDiacritics_CTL( bool bVal );
    void    SetIgnoreKashida_CTL( bool bVal );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
