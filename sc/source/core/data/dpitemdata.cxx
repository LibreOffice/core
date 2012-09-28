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

#include "dpitemdata.hxx"

#include "document.hxx"
#include "dpobject.hxx"
#include "cell.hxx"
#include "globstr.hrc"
#include "dptabdat.hxx"
#include "rtl/math.hxx"

const sal_Int32 ScDPItemData::DateFirst = -1;
const sal_Int32 ScDPItemData::DateLast  = 10000;

sal_Int32 ScDPItemData::Compare(const ScDPItemData& rA, const ScDPItemData& rB)
{
    if (rA.meType != rB.meType)
    {
        // group value, value and string in this order.
        return rA.meType < rB.meType ? -1 : 1;
    }

    switch (rA.meType)
    {
        case GroupValue:
        {
            if (rA.maGroupValue.mnGroupType == rB.maGroupValue.mnGroupType)
            {
                if (rA.maGroupValue.mnValue == rB.maGroupValue.mnValue)
                    return 0;

                return rA.maGroupValue.mnValue < rB.maGroupValue.mnValue ? -1 : 1;
            }

            return rA.maGroupValue.mnGroupType < rB.maGroupValue.mnGroupType ? -1 : 1;
        }
        case Value:
        case RangeStart:
        {
            if (rA.mfValue == rB.mfValue)
                return 0;

            return rA.mfValue < rB.mfValue ? -1 : 1;
        }
        case String:
        case Error:
            if (rA.mpString == rB.mpString)
                // strings may be interned.
                return 0;

            return ScGlobal::GetCollator()->compareString(rA.GetString(), rB.GetString());
        default:
            ;
    }
    return 0;
}

ScDPItemData::ScDPItemData() :
    mfValue(0.0), meType(Empty), mbStringInterned(false) {}

ScDPItemData::ScDPItemData(const ScDPItemData& r) :
    meType(r.meType), mbStringInterned(r.mbStringInterned)
{
    switch (r.meType)
    {
        case String:
        case Error:
            mpString = mbStringInterned ? r.mpString :  new rtl::OUString(*r.mpString);
        break;
        case Value:
        case RangeStart:
            mfValue = r.mfValue;
        break;
        case GroupValue:
            maGroupValue.mnGroupType = r.maGroupValue.mnGroupType;
            maGroupValue.mnValue = r.maGroupValue.mnValue;
        break;
        case Empty:
        default:
            mfValue = 0.0;
    }
}

void ScDPItemData::DisposeString()
{
    if (!mbStringInterned)
    {
        if (meType == String || meType == Error)
            delete mpString;
    }

    mbStringInterned = false;
}

ScDPItemData::ScDPItemData(const rtl::OUString& rStr) :
    mpString(new rtl::OUString(rStr)), meType(String), mbStringInterned(false) {}

ScDPItemData::ScDPItemData(sal_Int32 nGroupType, sal_Int32 nValue) :
    meType(GroupValue), mbStringInterned(false)
{
    maGroupValue.mnGroupType = nGroupType;
    maGroupValue.mnValue = nValue;
}

ScDPItemData::~ScDPItemData()
{
    DisposeString();
}

ScDPItemData::Type ScDPItemData::GetType() const
{
    return static_cast<Type>(meType);
}

void ScDPItemData::SetEmpty()
{
    DisposeString();
    meType = Empty;
}

void ScDPItemData::SetString(const rtl::OUString& rS)
{
    DisposeString();
    mpString = new rtl::OUString(rS);
    meType = String;
}

void ScDPItemData::SetString(const rtl::OUString* pS)
{
    DisposeString();
    mpString = pS;
    meType = String;
    mbStringInterned = true;
}

void ScDPItemData::SetValue(double fVal)
{
    DisposeString();
    mfValue = fVal;
    meType = Value;
}

void ScDPItemData::SetRangeStart(double fVal)
{
    DisposeString();
    mfValue = fVal;
    meType = RangeStart;
}

void ScDPItemData::SetRangeFirst()
{
    DisposeString();
    rtl::math::setInf(&mfValue, true);
    meType = RangeStart;
}

void ScDPItemData::SetRangeLast()
{
    DisposeString();
    rtl::math::setInf(&mfValue, false);
    meType = RangeStart;
}

void ScDPItemData::SetErrorString(const rtl::OUString* pS)
{
    SetString(pS);
    meType = Error;
}

bool ScDPItemData::IsCaseInsEqual(const ScDPItemData& r) const
{
    if (meType != r.meType)
        return false;

    switch (meType)
    {
        case Value:
        case RangeStart:
            return rtl::math::approxEqual(mfValue, r.mfValue);
        case GroupValue:
            return maGroupValue.mnGroupType == r.maGroupValue.mnGroupType &&
                maGroupValue.mnValue == r.maGroupValue.mnValue;
        default:
            ;
    }

    if (mpString == r.mpString)
        // Fast equality check for interned strings.
        return true;

    return ScGlobal::GetpTransliteration()->isEqual(GetString(), r.GetString());
}

bool ScDPItemData::operator== (const ScDPItemData& r) const
{
    if (meType != r.meType)
        return false;

    switch (meType)
    {
        case Value:
        case RangeStart:
            return rtl::math::approxEqual(mfValue, r.mfValue);
        case GroupValue:
            return maGroupValue.mnGroupType == r.maGroupValue.mnGroupType &&
                maGroupValue.mnValue == r.maGroupValue.mnValue;
        default:
            ;
    }

    // need exact equality until we have a safe case insensitive string hash
    return GetString() == r.GetString();
}

bool ScDPItemData::operator!= (const ScDPItemData& r) const
{
    return !operator== (r);
}

bool ScDPItemData::operator< (const ScDPItemData& r) const
{
    return Compare(*this, r) == -1;
}

ScDPItemData& ScDPItemData::operator= (const ScDPItemData& r)
{
    DisposeString();
    meType = r.meType;
    mbStringInterned = false;
    switch (r.meType)
    {
        case String:
        case Error:
            mpString = r.mbStringInterned ? r.mpString : new rtl::OUString(*r.mpString);
            mbStringInterned = r.mbStringInterned;
        break;
        case Value:
        case RangeStart:
            mfValue = r.mfValue;
        break;
        case GroupValue:
            maGroupValue.mnGroupType = r.maGroupValue.mnGroupType;
            maGroupValue.mnValue = r.maGroupValue.mnValue;
        break;
        case Empty:
        default:
            mfValue = 0.0;
    }
    return *this;
}

sal_uInt8 ScDPItemData::GetCellType() const
{
    switch (meType)
    {
        case Error:
            return SC_VALTYPE_ERROR;
        case Empty:
            return SC_VALTYPE_EMPTY;
        case Value:
            return SC_VALTYPE_VALUE;
        default:
            ;
    }

    return SC_VALTYPE_STRING;
}

#if DEBUG_PIVOT_TABLE
void ScDPItemData::Dump(const char* msg) const
{
    printf("--- (%s)\n", msg);
    switch (meType)
    {
        case Empty:
            printf("empty\n");
        break;
        case Error:
            printf("error: %s\n",
                   rtl::OUStringToOString(*mpString, RTL_TEXTENCODING_UTF8).getStr());
        break;
        case GroupValue:
            printf("group value: group type = %d  value = %d\n",
                   maGroupValue.mnGroupType, maGroupValue.mnValue);
        break;
        case String:
            printf("string: %s\n",
                   rtl::OUStringToOString(*mpString, RTL_TEXTENCODING_UTF8).getStr());
        break;
        case Value:
            printf("value: %g\n", mfValue);
        break;
        case RangeStart:
            printf("range start: %g\n", mfValue);
        break;
        default:
            printf("unknown type\n");
    }
    printf("---\n");
}
#endif

bool ScDPItemData::IsEmpty() const
{
    return meType == Empty;
}

bool ScDPItemData::IsValue() const
{
    return meType == Value;
}

rtl::OUString ScDPItemData::GetString() const
{
    switch (meType)
    {
        case String:
        case Error:
            return *mpString;
        case Value:
            return rtl::OUString::valueOf(mfValue);
        case GroupValue:
        case RangeStart:
            return rtl::OUString::createFromAscii("fail");
        case Empty:
        default:
            ;
    }

    return rtl::OUString();
}

double ScDPItemData::GetValue() const
{
    if (meType == Value || meType == RangeStart)
        return mfValue;

    return 0.0;
}

ScDPItemData::GroupValueAttr ScDPItemData::GetGroupValue() const
{
    if (meType == GroupValue)
        return maGroupValue;

    GroupValueAttr aGV;
    aGV.mnGroupType = -1;
    aGV.mnValue = -1;
    return aGV;
}

bool ScDPItemData::HasStringData() const
{
    return meType == String || meType == Error;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
