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

#ifndef __SC_DPITEMDATA_HXX__
#define __SC_DPITEMDATA_HXX__

#include "scdllapi.h"
#include "address.hxx"

#include "sal/types.h"
#include "tools/solar.h"
#include "rtl/ustring.hxx"

#include <vector>

#include <boost/unordered_map.hpp>

#define DEBUG_DP_ITEM_DATA 1

class ScDocument;

class SC_DLLPUBLIC ScDPItemData
{
    friend class ScDPCache;

public:
    enum Type { String, Value, Error, Empty, GroupValue };

    struct GroupValueAttr
    {
        sal_Int32 mnGroupType;
        sal_Int32 mnValue;
    };

private:

    union {
        rtl::OUString* mpString;
        GroupValueAttr maGroupValue;
        double mfValue;
    };

    Type meType;

public:
    // case insensitive equality
    static sal_Int32 Compare(const ScDPItemData& rA, const ScDPItemData& rB);

    ScDPItemData();
    ScDPItemData(const ScDPItemData& r);
    ScDPItemData(const rtl::OUString& rStr);
    ScDPItemData(double fVal);
    ScDPItemData(sal_Int32 nGroupType, sal_Int32 nValue);
    ~ScDPItemData();

    Type GetType() const;
    void SetString(const rtl::OUString& rS);
    void SetValue(double fVal);
    void SetGroupValue(sal_Int32 nGroupType, sal_Int32 nValue);
    void SetErrorString(const rtl::OUString& rS);
    bool IsCaseInsEqual(const ScDPItemData& r) const;

    size_t Hash() const;

    // exact equality
    bool operator==(const ScDPItemData& r) const;

    ScDPItemData& operator= (const ScDPItemData& r);

    bool IsEmpty() const;
    bool IsValue() const;
    rtl::OUString GetString() const;
    double GetValue() const;
    GroupValueAttr GetGroupValue() const;
    bool HasStringData() const ;

    sal_uInt8 GetCellType() const;

#if DEBUG_DP_ITEM_DATA
    void Dump(const char* msg) const;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
