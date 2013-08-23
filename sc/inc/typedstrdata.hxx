/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_TYPEDSTRDATA_HXX__
#define __SC_TYPEDSTRDATA_HXX__

#include "rtl/ustring.hxx"
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

    bool IsStrData() const;
    bool IsDate() const;
    SC_DLLPUBLIC const OUString& GetString() const;
    StringType GetStringType() const;
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

    bool operator== (const ScTypedStrData& r) const;
    bool operator< (const ScTypedStrData& r) const;

private:
    OUString maStrValue;
    double mfValue;
    StringType meStrType;
    bool   mbIsDate;
};

class FindTypedStrData : std::unary_function<ScTypedStrData, bool>
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
