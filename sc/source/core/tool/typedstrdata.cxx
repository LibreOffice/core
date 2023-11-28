/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <typedstrdata.hxx>
#include <global.hxx>

#include <unotools/collatorwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <utility>

bool ScTypedStrData::LessHiddenRows::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    return left.mbIsHiddenByFilter < right.mbIsHiddenByFilter;
}

bool ScTypedStrData::LessCaseSensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return left.meStrType < right.meStrType;

    if (left.meStrType == Value)
    {
        if (left.mfValue == right.mfValue)
            return left.mbIsHiddenByFilter < right.mbIsHiddenByFilter;
        return left.mfValue < right.mfValue;
    }

    if (left.mbIsDate != right.mbIsDate)
        return left.mbIsDate < right.mbIsDate;

    sal_Int32 nEqual
        = ScGlobal::GetCaseTransliteration().compareString(left.maStrValue, right.maStrValue);

    if (!nEqual)
        return left.mbIsHiddenByFilter < right.mbIsHiddenByFilter;

    return nEqual < 0;
}

bool ScTypedStrData::LessSortCaseSensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return left.meStrType < right.meStrType;

    if (left.meStrType == Value)
    {
        if (left.mfValue == right.mfValue)
            return left.mbIsHiddenByFilter < right.mbIsHiddenByFilter;
        return left.mfValue < right.mfValue;
    }

    if (left.mbIsDate != right.mbIsDate)
        return left.mbIsDate < right.mbIsDate;

    sal_Int32 nEqual = ScGlobal::GetCaseCollator().compareString(left.maStrValue, right.maStrValue);

    if (!nEqual)
        return left.mbIsHiddenByFilter < right.mbIsHiddenByFilter;

    return nEqual < 0;
}

bool ScTypedStrData::LessCaseInsensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return left.meStrType < right.meStrType;

    if (left.meStrType == Value)
    {
        if (left.mfValue == right.mfValue)
            return left.mbIsHiddenByFilter < right.mbIsHiddenByFilter;
        return left.mfValue < right.mfValue;
    }

    if (left.mbIsDate != right.mbIsDate)
        return left.mbIsDate < right.mbIsDate;

    sal_Int32 nEqual
        = ScGlobal::GetTransliteration().compareString(left.maStrValue, right.maStrValue);

    if (!nEqual)
        return left.mbIsHiddenByFilter < right.mbIsHiddenByFilter;

    return nEqual < 0;
}

bool ScTypedStrData::LessSortCaseInsensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return left.meStrType < right.meStrType;

    if (left.meStrType == Value)
    {
        if (left.mfValue == right.mfValue)
            return left.mbIsHiddenByFilter < right.mbIsHiddenByFilter;
        return left.mfValue < right.mfValue;
    }

    if (left.mbIsDate != right.mbIsDate)
        return left.mbIsDate < right.mbIsDate;

    sal_Int32 nEqual = ScGlobal::GetCaseCollator().compareString(left.maStrValue, right.maStrValue);

    if (!nEqual)
        return left.mbIsHiddenByFilter < right.mbIsHiddenByFilter;

    return nEqual < 0;
}

bool ScTypedStrData::EqualCaseSensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return false;

    if (left.meStrType == Value && left.mfRoundedValue != right.mfRoundedValue)
        return false;

    if (left.mbIsDate != right.mbIsDate )
        return false;

    return ScGlobal::GetCaseTransliteration().isEqual(left.maStrValue, right.maStrValue);
}

bool ScTypedStrData::EqualCaseInsensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return false;

    if (left.meStrType == Value && left.mfRoundedValue != right.mfRoundedValue)
        return false;

    if (left.mbIsDate != right.mbIsDate )
        return false;

    return ScGlobal::GetTransliteration().isEqual(left.maStrValue, right.maStrValue);
}

bool ScTypedStrData::operator< (const ScTypedStrData& r) const
{
    // Case insensitive comparison by default.
    return LessCaseInsensitive()(*this, r);
}

ScTypedStrData::ScTypedStrData(
    const OUString& rStr, double fVal, double fRVal, StringType nType, bool bDate, bool bIsHiddenByFilter ) :
    maStrValue(rStr),
    mfValue(fVal),
    mfRoundedValue(fRVal),
    meStrType(nType),
    mbIsDate( bDate ),
    mbIsHiddenByFilter(bIsHiddenByFilter) {}

FindTypedStrData::FindTypedStrData(ScTypedStrData aVal, bool bCaseSens) :
    maVal(std::move(aVal)), mbCaseSens(bCaseSens) {}

bool FindTypedStrData::operator() (const ScTypedStrData& r) const
{
    if (mbCaseSens)
    {
        return ScTypedStrData::EqualCaseSensitive()(maVal, r);
    }
    else
    {
        return ScTypedStrData::EqualCaseInsensitive()(maVal, r);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
