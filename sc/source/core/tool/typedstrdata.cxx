/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
