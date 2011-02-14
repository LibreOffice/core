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
    sal_Bool    IsSearchForStyles() const;
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
};


#endif

