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
#include "dpmacros.hxx"
#include "dpglobal.hxx"

/**
 * When assigning a string value, you can also assign an interned string
 * whose life-cycle is managed by the pivot cache that it belongs to.  Those
 * methods that take a string pointer assume that the string is interned.
 *
 * <p>Do make sure that an item with an interned string won't persist after
 * the pivot cache has been destroyed or reloaded.</p>
 */
class SC_DLLPUBLIC ScDPItemData
{
    friend class ScDPCache;

public:
    enum Type { GroupValue = 0, RangeStart = 1, Value = 2, String = 3, Error = 4, Empty = 5 };

    static const sal_Int32 DateFirst;
    static const sal_Int32 DateLast;

    struct GroupValueAttr
    {
        sal_Int32 mnGroupType;
        sal_Int32 mnValue;
    };

private:

    union {
        const rtl::OUString* mpString;
        GroupValueAttr maGroupValue;
        double mfValue;
    };

    sal_uInt8 meType:3;
    bool mbStringInterned:1;

    void DisposeString();

public:
    // case insensitive equality
    static sal_Int32 Compare(const ScDPItemData& rA, const ScDPItemData& rB);

    ScDPItemData();
    ScDPItemData(const ScDPItemData& r);
    ScDPItemData(const rtl::OUString& rStr);
    ScDPItemData(sal_Int32 nGroupType, sal_Int32 nValue);
    ~ScDPItemData();

    Type GetType() const;
    void SetEmpty();
    void SetString(const rtl::OUString& rS);
    void SetString(const rtl::OUString* pS);
    void SetValue(double fVal);
    void SetRangeStart(double fVal);
    void SetRangeFirst();
    void SetRangeLast();
    void SetErrorString(const rtl::OUString* pS);
    bool IsCaseInsEqual(const ScDPItemData& r) const;

    // exact equality
    bool operator==(const ScDPItemData& r) const;
    bool operator!=(const ScDPItemData& r) const;
    bool operator< (const ScDPItemData& r) const;

    ScDPItemData& operator= (const ScDPItemData& r);

    bool IsEmpty() const;
    bool IsValue() const;
    rtl::OUString GetString() const;
    double GetValue() const;
    GroupValueAttr GetGroupValue() const;
    bool HasStringData() const ;

    ScDPValue::Type GetCellType() const;

#if DEBUG_PIVOT_TABLE
    void Dump(const char* msg) const;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
