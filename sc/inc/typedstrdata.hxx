/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_TYPEDSTRDATA_HXX
#define INCLUDED_SC_INC_TYPEDSTRDATA_HXX

#include <rtl/ustring.hxx>
#include "scdllapi.h"

#include <set>

class ScTypedStrData
{
public:
    enum StringType {
        Value    = 0,
        Standard = 1,
        Name     = 2,
        DbName   = 3,
        Header   = 4
    };

    ScTypedStrData( const OUString& rStr, double nVal = 0.0,
                    StringType eType = Standard, bool bDate = false );

    ScTypedStrData( const ScTypedStrData& rCpy );

    bool IsDate() const { return mbIsDate;}
    const OUString& GetString() const { return maStrValue;}
    StringType GetStringType() const { return meStrType;}
    double GetValue() const { return mfValue; }

    struct LessCaseSensitive : std::binary_function<ScTypedStrData, ScTypedStrData, bool>
    {
        bool operator() (const ScTypedStrData& left, const ScTypedStrData& right) const;
    };

    struct LessCaseInsensitive : std::binary_function<ScTypedStrData, ScTypedStrData, bool>
    {
        bool operator() (const ScTypedStrData& left, const ScTypedStrData& right) const;
    };

    struct EqualCaseSensitive : std::binary_function<ScTypedStrData, ScTypedStrData, bool>
    {
        bool operator() (const ScTypedStrData& left, const ScTypedStrData& right) const;
    };

    struct EqualCaseInsensitive : std::binary_function<ScTypedStrData, ScTypedStrData, bool>
    {
        bool operator() (const ScTypedStrData& left, const ScTypedStrData& right) const;
    };

    bool operator< (const ScTypedStrData& r) const;

private:
    OUString maStrValue;
    double mfValue;
    StringType meStrType;
    bool   mbIsDate;
};

class FindTypedStrData : public std::unary_function<ScTypedStrData, bool>
{
    ScTypedStrData maVal;
    bool mbCaseSens;
public:
    FindTypedStrData(const ScTypedStrData& rVal, bool bCaseSens);
    bool operator() (const ScTypedStrData& r) const;
};

typedef std::set<ScTypedStrData, ScTypedStrData::LessCaseSensitive> ScTypedCaseStrSet;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
