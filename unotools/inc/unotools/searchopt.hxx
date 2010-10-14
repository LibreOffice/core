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

    INT32   GetTransliterationFlags() const;

    //
    // General Options
    //

    BOOL    IsWholeWordsOnly() const;
    BOOL    IsBackwards() const;
    BOOL    IsUseRegularExpression() const;
    BOOL    IsSearchForStyles() const;
    BOOL    IsSimilaritySearch() const;
    BOOL    IsUseAsianOptions() const;
    BOOL    IsMatchCase() const;            // also Japanese search option
    BOOL    IsNotes() const;

    void    SetWholeWordsOnly( BOOL bVal );
    void    SetBackwards( BOOL bVal );
    void    SetUseRegularExpression( BOOL bVal );
    void    SetSearchForStyles( BOOL bVal );
    void    SetSimilaritySearch( BOOL bVal );
    void    SetUseAsianOptions( BOOL bVal );
    void    SetMatchCase( BOOL bVal );      // also Japanese search option
    void    SetNotes( BOOL bVal);

    //
    // Japanese search options
    //

    BOOL    IsMatchFullHalfWidthForms() const;
    BOOL    IsMatchHiraganaKatakana() const;
    BOOL    IsMatchContractions() const;
    BOOL    IsMatchMinusDashChoon() const;
    BOOL    IsMatchRepeatCharMarks() const;
    BOOL    IsMatchVariantFormKanji() const;
    BOOL    IsMatchOldKanaForms() const;
    BOOL    IsMatchDiziDuzu() const;
    BOOL    IsMatchBavaHafa() const;
    BOOL    IsMatchTsithichiDhizi() const;
    BOOL    IsMatchHyuiyuByuvyu() const;
    BOOL    IsMatchSesheZeje() const;
    BOOL    IsMatchIaiya() const;
    BOOL    IsMatchKiku() const;
    BOOL    IsIgnorePunctuation() const;
    BOOL    IsIgnoreWhitespace() const;
    BOOL    IsIgnoreProlongedSoundMark() const;
    BOOL    IsIgnoreMiddleDot() const;

    void    SetMatchFullHalfWidthForms( BOOL bVal );
    void    SetMatchHiraganaKatakana( BOOL bVal );
    void    SetMatchContractions( BOOL bVal );
    void    SetMatchMinusDashChoon( BOOL bVal );
    void    SetMatchRepeatCharMarks( BOOL bVal );
    void    SetMatchVariantFormKanji( BOOL bVal );
    void    SetMatchOldKanaForms( BOOL bVal );
    void    SetMatchDiziDuzu( BOOL bVal );
    void    SetMatchBavaHafa( BOOL bVal );
    void    SetMatchTsithichiDhizi( BOOL bVal );
    void    SetMatchHyuiyuByuvyu( BOOL bVal );
    void    SetMatchSesheZeje( BOOL bVal );
    void    SetMatchIaiya( BOOL bVal );
    void    SetMatchKiku( BOOL bVal );
    void    SetIgnorePunctuation( BOOL bVal );
    void    SetIgnoreWhitespace( BOOL bVal );
    void    SetIgnoreProlongedSoundMark( BOOL bVal );
    void    SetIgnoreMiddleDot( BOOL bVal );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
