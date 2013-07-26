/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "typedstrdata.hxx"
#include "global.hxx"

#include "unotools/collatorwrapper.hxx"

bool ScTypedStrData::LessCaseSensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return left.meStrType < right.meStrType;

    if (left.meStrType == Value)
        return left.mfValue < right.mfValue;

    return ScGlobal::GetCaseCollator()->compareString(
        left.maStrValue, right.maStrValue) < 0;
}

bool ScTypedStrData::LessCaseInsensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return left.meStrType < right.meStrType;

    if (left.meStrType == Value)
        return left.mfValue < right.mfValue;

    return ScGlobal::GetCollator()->compareString(
        left.maStrValue, right.maStrValue) < 0;
}

bool ScTypedStrData::EqualCaseSensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return false;

    if (left.meStrType == Value && left.mfValue != right.mfValue)
        return false;

    return ScGlobal::GetCaseCollator()->compareString(
        left.maStrValue, right.maStrValue) == 0;
}

bool ScTypedStrData::EqualCaseInsensitive::operator() (const ScTypedStrData& left, const ScTypedStrData& right) const
{
    if (left.meStrType != right.meStrType)
        return false;

    if (left.meStrType == Value && left.mfValue != right.mfValue)
        return false;

    return ScGlobal::GetCollator()->compareString(
        left.maStrValue, right.maStrValue) == 0;
}

bool ScTypedStrData::operator== (const ScTypedStrData& r) const
{
    // Case insensitive comparison by default.
    EqualCaseInsensitive aHdl;
    return aHdl(*this, r);
}

bool ScTypedStrData::operator< (const ScTypedStrData& r) const
{
    // Case insensitive comparison by default.
    LessCaseInsensitive aHdl;
    return aHdl(*this, r);
}

ScTypedStrData::ScTypedStrData(
    const OUString& rStr, double nVal, StringType nType ) :
    maStrValue(rStr),
    mfValue(nVal),
    meStrType(nType) {}

ScTypedStrData::ScTypedStrData( const ScTypedStrData& rCpy ) :
    maStrValue(rCpy.maStrValue),
    mfValue(rCpy.mfValue),
    meStrType(rCpy.meStrType) {}

bool ScTypedStrData::IsStrData() const
{
    return meStrType != Value;
}

const OUString& ScTypedStrData::GetString() const
{
    return maStrValue;
}

ScTypedStrData::StringType ScTypedStrData::GetStringType() const
{
    return meStrType;
}

FindTypedStrData::FindTypedStrData(const ScTypedStrData& rVal, bool bCaseSens) :
    maVal(rVal), mbCaseSens(bCaseSens) {}

bool FindTypedStrData::operator() (const ScTypedStrData& r) const
{
    if (mbCaseSens)
    {
        ScTypedStrData::EqualCaseSensitive aHdl;
        return aHdl(maVal, r);
    }
    else
    {
        ScTypedStrData::EqualCaseInsensitive aHdl;
        return aHdl(maVal, r);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
