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
#ifndef INCLUDED_I18NUTIL_SEARCHOPT_HXX
#define INCLUDED_I18NUTIL_SEARCHOPT_HXX

#include <sal/types.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchOptions2.hpp>
#include <i18nutil/transliteration.hxx>
#include <utility>

namespace i18nutil
{

inline constexpr css::util::SearchAlgorithms downgradeSearchAlgorithms2(sal_Int16 searchAlgorithms2)
{
    switch (searchAlgorithms2)
    {
        case css::util::SearchAlgorithms2::ABSOLUTE:
            return css::util::SearchAlgorithms_ABSOLUTE;
        case css::util::SearchAlgorithms2::REGEXP:
            return css::util::SearchAlgorithms_REGEXP;
        case css::util::SearchAlgorithms2::APPROXIMATE:
            return css::util::SearchAlgorithms_APPROXIMATE;
        default: // default what?
        case css::util::SearchAlgorithms2::WILDCARD: // something valid
            return css::util::SearchAlgorithms_ABSOLUTE;
    }
}

inline constexpr sal_Int16 upgradeSearchAlgorithms(css::util::SearchAlgorithms searchAlgorithms)
{
    switch (searchAlgorithms)
    {
        default: // default what?
        case css::util::SearchAlgorithms_ABSOLUTE:
            return css::util::SearchAlgorithms2::ABSOLUTE;
        case css::util::SearchAlgorithms_REGEXP:
            return css::util::SearchAlgorithms2::REGEXP;
        case css::util::SearchAlgorithms_APPROXIMATE:
            return css::util::SearchAlgorithms2::APPROXIMATE;
    }
}

/**
 * This is a wrapper around com::sun::star::util::SearchOptions and SearchOptions2,
 * but using the more type-safe TransliterationFlags enum, and without obsolete
 * algorithmType, which is superseded by AlgorithmType2.
 */
struct SAL_WARN_UNUSED SearchOptions2 {
    sal_Int32 searchFlag;
    OUString searchString;
    OUString replaceString;
    css::lang::Locale Locale;
    sal_Int32 changedChars;
    sal_Int32 deletedChars;
    sal_Int32 insertedChars;
    TransliterationFlags transliterateFlags;

    sal_Int16 AlgorithmType2;
    sal_Int32 WildcardEscapeCharacter;

    SearchOptions2& operator=(css::util::SearchOptions2 const & other)
    {
        searchFlag = other.searchFlag;
        searchString = other.searchString;
        replaceString = other.replaceString;
        Locale = other.Locale;
        changedChars = other.changedChars;
        deletedChars = other.deletedChars;
        insertedChars = other.insertedChars;
        transliterateFlags = static_cast<TransliterationFlags>(other.transliterateFlags);
        AlgorithmType2 = other.AlgorithmType2;
        WildcardEscapeCharacter = other.WildcardEscapeCharacter;
        return *this;
    }

    css::util::SearchOptions2 toUnoSearchOptions2() const
    {
        return css::util::SearchOptions2(downgradeSearchAlgorithms2(AlgorithmType2), searchFlag,
                       searchString, replaceString,
                       Locale,
                       changedChars, deletedChars, insertedChars,
                       static_cast<sal_Int32>(transliterateFlags),
                       AlgorithmType2, WildcardEscapeCharacter);
    }

    SearchOptions2()
        : searchFlag(0)
        , changedChars(0)
        , deletedChars(0)
        , insertedChars(0)
        , transliterateFlags(TransliterationFlags::NONE)
        , AlgorithmType2(0)
        , WildcardEscapeCharacter(0)
    {}

    SearchOptions2(const sal_Int32 searchFlag_,
                   OUString searchString_, OUString replaceString_,
                   css::lang::Locale Locale_,
                   const sal_Int32 changedChars_, const sal_Int32 deletedChars_, const sal_Int32 insertedChars_,
                   const TransliterationFlags& transliterateFlags_,
                   const sal_Int16 AlgorithmType2_, const sal_Int32 WildcardEscapeCharacter_)
        : searchFlag(searchFlag_)
        , searchString(std::move(searchString_))
        , replaceString(std::move(replaceString_))
        , Locale(std::move(Locale_))
        , changedChars(changedChars_)
        , deletedChars(deletedChars_)
        , insertedChars(insertedChars_)
        , transliterateFlags(transliterateFlags_)
        , AlgorithmType2(AlgorithmType2_)
        , WildcardEscapeCharacter(WildcardEscapeCharacter_)
    {}
};

}; // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
