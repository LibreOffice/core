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
                    StringType eType = Standard );

    ScTypedStrData( const ScTypedStrData& rCpy );

    bool IsStrData() const;
    SC_DLLPUBLIC const OUString& GetString() const;
    StringType GetStringType() const;

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
