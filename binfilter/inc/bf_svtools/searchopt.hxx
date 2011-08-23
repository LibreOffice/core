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

#include "tools/solar.h"

namespace binfilter
{

class SvtSearchOptions_Impl;

class  SvtSearchOptions
{
    SvtSearchOptions_Impl	*pImpl;

    // disallow copy-constructor and assignment-operator for now
    SvtSearchOptions( const SvtSearchOptions & );
    SvtSearchOptions & operator = ( const SvtSearchOptions & );

protected:

public:
    SvtSearchOptions();
    ~SvtSearchOptions();

    //
    // General Options
    //

    BOOL	IsWholeWordsOnly() const;
    BOOL	IsBackwards() const;
    BOOL	IsUseRegularExpression() const;
    BOOL	IsSimilaritySearch() const;
    BOOL	IsUseAsianOptions() const;
    BOOL	IsMatchCase() const;			// also Japanese search option

    //
    // Japanese search options
    //

    BOOL	IsMatchFullHalfWidthForms() const;
    BOOL	IsMatchHiraganaKatakana() const;
    BOOL	IsMatchContractions() const;
    BOOL	IsMatchMinusDashChoon() const;
    BOOL	IsMatchRepeatCharMarks() const;
    BOOL	IsMatchVariantFormKanji() const;
    BOOL	IsMatchOldKanaForms() const;
    BOOL	IsMatchDiziDuzu() const;
    BOOL	IsMatchBavaHafa() const;
    BOOL	IsMatchTsithichiDhizi() const;
    BOOL	IsMatchHyuiyuByuvyu() const;
    BOOL	IsMatchSesheZeje() const;
    BOOL	IsMatchIaiya() const;
    BOOL	IsMatchKiku() const;
    BOOL	IsIgnorePunctuation() const;
    BOOL	IsIgnoreWhitespace() const;
    BOOL	IsIgnoreProlongedSoundMark() const;
    BOOL	IsIgnoreMiddleDot() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
