/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "dpglobal.hxx"
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
    mnNumFormat( 0 ), mfValue(0.0), mbFlag(0) {}

ScDPItemData::ScDPItemData(sal_uLong nNF, const String & rS, double fV, sal_uInt8 bF) :
    mnNumFormat(nNF), maString(rS), mfValue(fV), mbFlag(bF) {}

ScDPItemData::ScDPItemData(const String& rS, double fV, bool bHV, const sal_uLong nNumFormatP, bool bData) :
    mnNumFormat( nNumFormatP ), maString(rS), mfValue(fV),
    mbFlag( (MK_VAL*!!bHV) | (MK_DATA*!!bData) | (MK_ERR*!!false) | (MK_DATE*!!isDate( mnNumFormat ) ) )
{
}

ScDPItemData::ScDPItemData(ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nDocTab, bool bLabel) :
    mnNumFormat( 0 ), mfValue(0.0), mbFlag( 0 )
{
    String aDocStr;
    pDoc->GetString( nCol, nRow, nDocTab, aDocStr );

    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

    ScAddress aPos( nCol, nRow, nDocTab );
    ScBaseCell* pCell = pDoc->GetCell( aPos );

    if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA && ((ScFormulaCell*)pCell)->GetErrCode() )
    {
        SetString ( aDocStr );
        mbFlag |= MK_ERR;
    }
    else if ( pDoc->HasValueData( nCol, nRow, nDocTab ) )
    {
        double fVal = pDoc->GetValue(ScAddress(nCol, nRow, nDocTab));
        sal_uLong nFormat = NUMBERFORMAT_NUMBER;
        if ( pFormatter )
            nFormat = pFormatter->GetType( pDoc->GetNumberFormat( ScAddress( nCol, nRow, nDocTab ) ) );
        maString = aDocStr;
        mfValue = fVal;
        mbFlag |= MK_VAL|MK_DATA;
        mnNumFormat = pDoc->GetNumberFormat( ScAddress( nCol, nRow, nDocTab ) );
        isDate( nFormat ) ? ( mbFlag |= MK_DATE ) : (mbFlag &= ~MK_DATE);
    }
    else if (bLabel || pDoc->HasData(nCol, nRow, nDocTab))
    {
        if (bLabel && !aDocStr.Len())
        {
            // Replace an empty label string with column name.
            rtl::OUStringBuffer aBuf;
            aBuf.append(ScGlobal::GetRscString(STR_COLUMN));
            aBuf.append(sal_Unicode(' '));

            ScAddress aColAddr(nCol, 0, 0);
            rtl::OUString aColStr;
            aColAddr.Format(aColStr, SCA_VALID_COL, NULL);
            aBuf.append(aColStr);
            aDocStr = aBuf.makeStringAndClear();
        }

        SetString(aDocStr);
    }
}

void ScDPItemData::SetString( const String& rS )
{
    maString = rS;
    mbFlag &= ~(MK_VAL|MK_DATE);
    mnNumFormat = 0;
    mbFlag |= MK_DATA;
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
        return rtl_ustr_hashCode_WithLength( maString.GetBuffer(), maString.Len() );
}

bool ScDPItemData::operator==( const ScDPItemData& r ) const
{
    if ( IsValue() )
    {
        if( (HasDatePart() != r.HasDatePart())  || (HasDatePart() && mnDatePart != r.mnDatePart) )
            return false;

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

String ScDPItemData::GetString() const
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

bool ScDPItemData::HasDatePart() const
{
    return !!(mbFlag&MK_DATEPART);
}

void ScDPItemData::SetDate( bool b )
{
    b ? ( mbFlag |= MK_DATE ) : ( mbFlag &= ~MK_DATE );
}

namespace ScDPGlobal
{
    Rectangle operator *( const Rectangle &rLeft, const std::pair<double,double> & rRight )
    {
        Rectangle rcResult( rLeft );
        rcResult.Bottom() = rcResult.Top() + static_cast<long>( rcResult.GetHeight() * rRight.second );
        rcResult.Right() = rcResult.Left() + static_cast<long>( rcResult.GetWidth() * rRight.first);
        return rcResult;
    }

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
