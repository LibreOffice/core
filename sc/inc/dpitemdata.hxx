/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "scdllapi.h"

#include <sal/types.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include "calcmacros.hxx"
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
        rtl_uString* mpString;
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
    ScDPItemData(const OUString& rStr);
    ScDPItemData(sal_Int32 nGroupType, sal_Int32 nValue);
    ~ScDPItemData();

    Type GetType() const { return static_cast<Type>(meType); }
    void SetEmpty();
    void SetString(const OUString& rS);
    void SetStringInterned( rtl_uString* pS );
    void SetValue(double fVal);
    void SetRangeStart(double fVal);
    void SetRangeFirst();
    void SetRangeLast();
    void SetErrorStringInterned( rtl_uString* pS );
    bool IsCaseInsEqual(const ScDPItemData& r) const;

    // exact equality
    bool operator==(const ScDPItemData& r) const;
    bool operator< (const ScDPItemData& r) const;

    ScDPItemData& operator= (const ScDPItemData& r);

    bool IsEmpty() const;
    bool IsValue() const;
    OUString GetString() const;
    double GetValue() const;
    GroupValueAttr GetGroupValue() const;
    bool HasStringData() const ;

    ScDPValue::Type GetCellType() const;

#if DEBUG_PIVOT_TABLE
    void Dump(const char* msg) const;
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
