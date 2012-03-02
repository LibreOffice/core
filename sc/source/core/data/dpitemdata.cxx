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

sal_Int32 ScDPItemData::Compare(const ScDPItemData& rA, const ScDPItemData& rB)
{
    if ( rA.IsValue() )
    {
        if ( rB.IsValue() )
            return rA.mfValue < rB.mfValue ? -1 : 1;
        else
            return -1;           // values first
    }
    else if ( rB.IsValue() )
        return 1;                // values first
    else
        return ScGlobal::GetCollator()->compareString(rA.GetString(), rB.GetString());
}

ScDPItemData::ScDPItemData() :
    mfValue(0.0), meType(Empty) {}

ScDPItemData::ScDPItemData(const rtl::OUString& rStr) :
    mpString(new rtl::OUString(rStr)), meType(String) {}

ScDPItemData::ScDPItemData(double fVal) :
    mfValue(fVal), meType(Value) {}

ScDPItemData::ScDPItemData(sal_Int32 nGroupType, sal_Int32 nValue) :
    meType(GroupValue)
{
    maGroupValue.mnGroupType = nGroupType;
    maGroupValue.mnValue = nValue;
}

ScDPItemData::~ScDPItemData()
{
    if (meType == String)
        delete mpString;
}

void ScDPItemData::SetString(const rtl::OUString& rS)
{
    if (meType == String)
        delete mpString;
    mpString = new rtl::OUString(rS);
    meType = String;
}

void ScDPItemData::SetValue(double fVal)
{
    if (meType == String)
        delete mpString;
    mfValue = fVal;
    meType = Value;
}

void ScDPItemData::SetGroupValue(sal_Int32 nGroupType, sal_Int32 nValue)
{
    if (meType == String)
        delete mpString;

    maGroupValue.mnGroupType = nGroupType;
    maGroupValue.mnValue = nValue;
    meType = GroupValue;
}

void ScDPItemData::SetErrorString(const rtl::OUString& rS)
{
    SetString(rS);
}

bool ScDPItemData::IsCaseInsEqual(const ScDPItemData& r) const
{
    if (meType != r.meType)
        return false;

    if (meType == Value)
        return rtl::math::approxEqual(mfValue, r.mfValue);

    return ScGlobal::GetpTransliteration()->isEqual(GetString(), r.GetString());
}

size_t ScDPItemData::Hash() const
{
    if (meType == Value)
        return static_cast<size_t>(rtl::math::approxFloor(mfValue));

    // If we do unicode safe case insensitive hash we can drop
    // ScDPItemData::operator== and use ::IsCasInsEqual
    rtl::OUString aStr = GetString();
    return rtl_ustr_hashCode_WithLength(aStr.getStr(), aStr.getLength());
}

bool ScDPItemData::operator== (const ScDPItemData& r) const
{
    if (meType != r.meType)
        return false;

    if (meType == Value)
        return (r.meType == Value) ? rtl::math::approxEqual(mfValue, r.mfValue) : false;

    if (meType == GroupValue)
        return maGroupValue.mnGroupType == r.maGroupValue.mnGroupType &&
            maGroupValue.mnValue == r.maGroupValue.mnValue;

    // need exact equality until we have a safe case insensitive string hash
    return GetString() == r.GetString();
}

sal_uInt8 ScDPItemData::GetCellType() const
{
    if (meType == Error)
        return SC_VALTYPE_ERROR;
    else if (meType == Empty)
        return SC_VALTYPE_EMPTY;
    else if ( IsValue())
        return SC_VALTYPE_VALUE;
    else
        return SC_VALTYPE_STRING;
}

bool ScDPItemData::IsEmpty() const
{
    return meType == Empty;
}

bool ScDPItemData::IsValue() const
{
    return meType == Value || meType == GroupValue;
}

rtl::OUString ScDPItemData::GetString() const
{
    if (meType == String)
        return *mpString;

    // TODO: Generate appropriate string.
    return rtl::OUString();
}

double ScDPItemData::GetValue() const
{
    if (meType == Value)
        return mfValue;

    if (meType == GroupValue)
        return maGroupValue.mnValue;

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
    return meType == String;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
