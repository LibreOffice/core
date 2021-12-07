/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>

#include <set>

class ScTypedStrData
{
public:
    enum StringType {
        Value,
        MRU,
        Standard,
        Name,
        DbName,
        Header
    };

    ScTypedStrData( const OUString& rStr, double fVal = 0.0, double fRVal = 0.0, StringType eType = Standard,
                    bool bDate = false );

    bool IsDate() const { return mbIsDate;}
    const OUString& GetString() const { return maStrValue;}
    StringType GetStringType() const { return meStrType;}
    double GetValue() const { return mfValue; }
    double GetRoundedValue() const { return mfRoundedValue; }

    struct LessCaseSensitive
    {
        bool operator() (const ScTypedStrData& left, const ScTypedStrData& right) const;
    };

    struct LessCaseInsensitive
    {
        bool operator() (const ScTypedStrData& left, const ScTypedStrData& right) const;
    };

    struct EqualCaseSensitive
    {
        bool operator() (const ScTypedStrData& left, const ScTypedStrData& right) const;
    };

    struct EqualCaseInsensitive
    {
        bool operator() (const ScTypedStrData& left, const ScTypedStrData& right) const;
    };

    bool operator< (const ScTypedStrData& r) const;

private:
    OUString maStrValue;
    double mfValue;
    double mfRoundedValue; // rounded value by format code
    StringType meStrType;
    bool   mbIsDate;
};

class FindTypedStrData
{
    ScTypedStrData maVal;
    bool mbCaseSens;
public:
    FindTypedStrData(const ScTypedStrData& rVal, bool bCaseSens);
    bool operator() (const ScTypedStrData& r) const;
};

typedef std::set<ScTypedStrData, ScTypedStrData::LessCaseSensitive> ScTypedCaseStrSet;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
