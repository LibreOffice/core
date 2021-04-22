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

bool ScTypedStrData::LessCaseSensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return left.meStrType < right.meStrType;

    if (left.meStrType == Value)
        return left.mfValue < right.mfValue;

    if (left.mbIsDate != right.mbIsDate)
        return left.mbIsDate < right.mbIsDate;

    return ScGlobal::GetCaseCollator()->compareString(
        left.maStrValue, right.maStrValue) < 0;
}

bool ScTypedStrData::LessCaseInsensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return left.meStrType < right.meStrType;

    if (left.meStrType == Value)
        return left.mfValue < right.mfValue;

    if (left.mbIsDate != right.mbIsDate)
        return left.mbIsDate < right.mbIsDate;

    return ScGlobal::GetCollator()->compareString(
        left.maStrValue, right.maStrValue) < 0;
}

bool ScTypedStrData::EqualCaseSensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return false;

    if (left.meStrType == Value && left.mfValue != right.mfValue &&
        !left.mbIsFormatted)
        return false;

    if (left.mbIsDate != right.mbIsDate )
        return false;

    if (ScGlobal::GetCaseCollator()->compareString(
        left.maStrValue, right.maStrValue) == 0)
    {
        // hack: it's possible, because we only compare values of the same filter range
        const_cast<bool&>(left.mbIsDuplicated) = true;
        return true;
    }
    else
        return false;
}

bool ScTypedStrData::EqualCaseInsensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return false;

    if (left.meStrType == Value && left.mfValue != right.mfValue &&
        !left.mbIsFormatted)
        return false;

    if (left.mbIsDate != right.mbIsDate )
        return false;

    if (ScGlobal::GetCollator()->compareString(
        left.maStrValue, right.maStrValue) == 0)
    {
        // hack: it's possible, because we only compare values of the same filter range
        const_cast<bool&>(left.mbIsDuplicated) = true;
        return true;
    }
    else
        return false;
}

bool ScTypedStrData::operator< (const ScTypedStrData& r) const
{
    // Case insensitive comparison by default.
    return LessCaseInsensitive()(*this, r);
}

ScTypedStrData::ScTypedStrData(
    const OUString& rStr, double nVal, StringType nType, bool bDate, bool bFormatted, bool bDuplicated ) :
    maStrValue(rStr),
    mfValue(nVal),
    meStrType(nType),
    mbIsDate( bDate ),
    mbIsFormatted( bFormatted ),
    mbIsDuplicated( bDuplicated ) {}

FindTypedStrData::FindTypedStrData(const ScTypedStrData& rVal, bool bCaseSens) :
    maVal(rVal), mbCaseSens(bCaseSens) {}

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
