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

#ifndef _SVT_SEARCHOPT_HXX_
#define _SVT_SEARCHOPT_HXX_

#include "unotools/unotoolsdllapi.h"
#include "tools/solar.h"

class SvtSearchOptions_Impl;

class UNOTOOLS_DLLPUBLIC SvtSearchOptions
{
    SvtSearchOptions_Impl   *pImpl;

    // disallow copy-constructor and assignment-operator for now
    SvtSearchOptions( const SvtSearchOptions & );
    SvtSearchOptions & operator = ( const SvtSearchOptions & );

protected:

public:
    SvtSearchOptions();
    ~SvtSearchOptions();

    sal_Int32   GetTransliterationFlags() const;

    //
    // General Options
    //

    sal_Bool    IsWholeWordsOnly() const;
    sal_Bool    IsBackwards() const;
    sal_Bool    IsUseRegularExpression() const;
    sal_Bool    IsSimilaritySearch() const;
    sal_Bool    IsUseAsianOptions() const;
    sal_Bool    IsMatchCase() const;            // also Japanese search option
    sal_Bool    IsNotes() const;

    void    SetWholeWordsOnly( sal_Bool bVal );
    void    SetBackwards( sal_Bool bVal );
    void    SetUseRegularExpression( sal_Bool bVal );
    void    SetSearchForStyles( sal_Bool bVal );
    void    SetSimilaritySearch( sal_Bool bVal );
    void    SetUseAsianOptions( sal_Bool bVal );
    void    SetMatchCase( sal_Bool bVal );      // also Japanese search option
    void    SetNotes( sal_Bool bVal);

    //
    // Japanese search options
    //

    sal_Bool    IsMatchFullHalfWidthForms() const;
    sal_Bool    IsMatchHiraganaKatakana() const;
    sal_Bool    IsMatchContractions() const;
    sal_Bool    IsMatchMinusDashChoon() const;
    sal_Bool    IsMatchRepeatCharMarks() const;
    sal_Bool    IsMatchVariantFormKanji() const;
    sal_Bool    IsMatchOldKanaForms() const;
    sal_Bool    IsMatchDiziDuzu() const;
    sal_Bool    IsMatchBavaHafa() const;
    sal_Bool    IsMatchTsithichiDhizi() const;
    sal_Bool    IsMatchHyuiyuByuvyu() const;
    sal_Bool    IsMatchSesheZeje() const;
    sal_Bool    IsMatchIaiya() const;
    sal_Bool    IsMatchKiku() const;
    sal_Bool    IsIgnorePunctuation() const;
    sal_Bool    IsIgnoreWhitespace() const;
    sal_Bool    IsIgnoreProlongedSoundMark() const;
    sal_Bool    IsIgnoreMiddleDot() const;
    sal_Bool    IsIgnoreDiacritics_CTL() const;

    void    SetMatchFullHalfWidthForms( sal_Bool bVal );
    void    SetMatchHiraganaKatakana( sal_Bool bVal );
    void    SetMatchContractions( sal_Bool bVal );
    void    SetMatchMinusDashChoon( sal_Bool bVal );
    void    SetMatchRepeatCharMarks( sal_Bool bVal );
    void    SetMatchVariantFormKanji( sal_Bool bVal );
    void    SetMatchOldKanaForms( sal_Bool bVal );
    void    SetMatchDiziDuzu( sal_Bool bVal );
    void    SetMatchBavaHafa( sal_Bool bVal );
    void    SetMatchTsithichiDhizi( sal_Bool bVal );
    void    SetMatchHyuiyuByuvyu( sal_Bool bVal );
    void    SetMatchSesheZeje( sal_Bool bVal );
    void    SetMatchIaiya( sal_Bool bVal );
    void    SetMatchKiku( sal_Bool bVal );
    void    SetIgnorePunctuation( sal_Bool bVal );
    void    SetIgnoreWhitespace( sal_Bool bVal );
    void    SetIgnoreProlongedSoundMark( sal_Bool bVal );
    void    SetIgnoreMiddleDot( sal_Bool bVal );
    void    SetIgnoreDiacritics_CTL( sal_Bool bVal );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
