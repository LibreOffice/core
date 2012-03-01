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

bool ScDPItemData::isDate( sal_uLong nNumType )
{
    return ((nNumType & NUMBERFORMAT_DATE) != 0) ? 1 : 0;
}

ScDPItemData::ScDPItemData() :
    mfValue(0.0), mbFlag(0) {}

ScDPItemData::ScDPItemData(const rtl::OUString & rS, double fV, sal_uInt8 bF) :
    maString(rS), mfValue(fV), mbFlag(bF) {}

ScDPItemData::ScDPItemData(const rtl::OUString& rS, double fV, bool bHV, bool bData) :
    maString(rS), mfValue(fV),
    mbFlag( (MK_VAL*!!bHV) | (MK_DATA*!!bData) | (MK_ERR*!!false) )
{
}

void ScDPItemData::Set(const rtl::OUString& rS, double fVal, sal_uInt8 nFlag)
{
    maString = rS;
    mfValue = fVal;
    mbFlag = nFlag;
}

void ScDPItemData::SetString(const rtl::OUString& rS)
{
    maString = rS;
    mbFlag &= ~(MK_VAL|MK_DATE);
    mbFlag |= MK_DATA;
}

void ScDPItemData::SetErrorString(const rtl::OUString& rS)
{
    SetString(rS);
    mbFlag |= MK_ERR;
}

bool ScDPItemData::IsCaseInsEqual( const ScDPItemData& r ) const
{
    //! pass Transliteration?
    //! inline?
    return IsValue() ? ( r.IsValue() && rtl::math::approxEqual( mfValue, r.mfValue ) ) :
                       ( !r.IsValue() &&
                        ScGlobal::GetpTransliteration()->isEqual( maString, r.maString ) );
}

size_t ScDPItemData::Hash() const
{
    if ( IsValue() )
        return (size_t) rtl::math::approxFloor( mfValue );
    else
        // If we do unicode safe case insensitive hash we can drop
        // ScDPItemData::operator== and use ::IsCasInsEqual
        return rtl_ustr_hashCode_WithLength(maString.getStr(), maString.getLength());
}

bool ScDPItemData::operator==( const ScDPItemData& r ) const
{
    if ( IsValue() )
    {
        if ( IsDate() != r.IsDate() )
            return false;
        else if ( r.IsValue() )
            return rtl::math::approxEqual( mfValue, r.mfValue );
        else
            return false;
    }
    else if ( r.IsValue() )
        return false;
    else
        // need exact equality until we have a safe case insensitive string hash
        return maString == r.maString;
}

sal_Int32 ScDPItemData::Compare( const ScDPItemData& rA,
                                 const ScDPItemData& rB )
{
    if ( rA.IsValue() )
    {
        if ( rB.IsValue() )
        {
            if ( rtl::math::approxEqual( rA.mfValue, rB.mfValue ) )
            {
                if ( rA.IsDate() == rB.IsDate() )
                    return 0;
                else
                    return rA.IsDate() ? 1: -1;
            }
            else if ( rA.mfValue < rB.mfValue )
                return -1;
            else
                return 1;
        }
        else
            return -1;           // values first
    }
    else if ( rB.IsValue() )
        return 1;                // values first
    else
        return ScGlobal::GetCollator()->compareString( rA.maString, rB.maString );
}

sal_uInt8 ScDPItemData::GetType() const
{
    if ( IsHasErr() )
        return SC_VALTYPE_ERROR;
    else if ( !IsHasData() )
        return SC_VALTYPE_EMPTY;
    else if ( IsValue())
        return SC_VALTYPE_VALUE;
    else
        return SC_VALTYPE_STRING;
}

bool ScDPItemData::IsHasData() const
{
    return !!(mbFlag&MK_DATA);
}

bool ScDPItemData::IsHasErr() const
{
    return !!(mbFlag&MK_ERR);
}

bool ScDPItemData::IsValue() const
{
    return !!(mbFlag&MK_VAL);
}

const rtl::OUString& ScDPItemData::GetString() const
{
    return maString;
}

double ScDPItemData::GetValue() const
{
    return mfValue;
}

bool ScDPItemData::HasStringData() const
{
    return IsHasData()&&!IsHasErr()&&!IsValue();
}

bool ScDPItemData::IsDate() const
{
    return !!(mbFlag&MK_DATE);
}

void ScDPItemData::SetDate( bool b )
{
    b ? ( mbFlag |= MK_DATE ) : ( mbFlag &= ~MK_DATE );
}


ScDPItemDataPool::ScDPItemDataPool()
{
}

ScDPItemDataPool::ScDPItemDataPool(const ScDPItemDataPool& r):
    maItems(r.maItems),
    maItemIds(r.maItemIds)
{
}

ScDPItemDataPool::~ScDPItemDataPool()
{
}


const ScDPItemData* ScDPItemDataPool::getData( sal_Int32 nId )
{
    if ( nId >= static_cast<sal_Int32>(maItems.size()) )
        return NULL;
    else
        return &(maItems[nId]);
}

sal_Int32 ScDPItemDataPool::getDataId( const ScDPItemData& aData )
{
    DataHash::const_iterator itr = maItemIds.find( aData),
            itrEnd = maItemIds.end();
    if ( itr == itrEnd )
         // not exist
        return -1;

    else //exist
        return itr->second;

}

sal_Int32 ScDPItemDataPool::insertData( const ScDPItemData& aData )
{
    sal_Int32 nResult = getDataId( aData );

    if( nResult < 0 )
    {
        maItemIds.insert( DataHash::value_type( aData, nResult = maItems.size() ) );
        maItems.push_back( aData );
    }

    return nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
