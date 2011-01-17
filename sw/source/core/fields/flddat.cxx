/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

// #include <math.h>
#include <tools/datetime.hxx>
#include <svl/zforlist.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <doc.hxx>
#include <fldbas.hxx>
#include <flddat.hxx>
#include <unofldmid.h>

using namespace ::com::sun::star;
/*--------------------------------------------------
    Beschreibung: Datum/Zeit-Typ
 ---------------------------------------------------*/

SwDateTimeFieldType::SwDateTimeFieldType(SwDoc* pInitDoc)
    : SwValueFieldType( pInitDoc, RES_DATETIMEFLD )
{}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFieldType* SwDateTimeFieldType::Copy() const
{
    SwDateTimeFieldType *pTmp = new SwDateTimeFieldType(GetDoc());
    return pTmp;
}

/*--------------------------------------------------------------------
    Beschreibung: Datum/Zeit-Feld
 --------------------------------------------------------------------*/

SwDateTimeField::SwDateTimeField(SwDateTimeFieldType* pInitType, sal_uInt16 nSub, sal_uLong nFmt, sal_uInt16 nLng)
    : SwValueField(pInitType, nFmt, nLng, 0.0),
    nSubType(nSub),
    nOffset(0)
{
    if (!nFmt)
    {
        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
        if (nSubType & DATEFLD)
            ChangeFormat(pFormatter->GetFormatIndex(NF_DATE_SYSTEM_SHORT, GetLanguage()));
        else
            ChangeFormat(pFormatter->GetFormatIndex(NF_TIME_HHMMSS, GetLanguage()));
    }
    if (IsFixed())
    {
        DateTime aDateTime;
        SetDateTime(aDateTime);
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

String SwDateTimeField::Expand() const
{
    double fVal;

    if (!(IsFixed()))
    {
        DateTime aDateTime;
        fVal = GetDateTime(GetDoc(), aDateTime);
    }
    else
        fVal = GetValue();

    if (nOffset)
        fVal += (double)(nOffset * 60L) / 86400.0;

    return ExpandValue(fVal, GetFormat(), GetLanguage());
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwField* SwDateTimeField::Copy() const
{
    SwDateTimeField *pTmp =
        new SwDateTimeField((SwDateTimeFieldType*)GetTyp(), nSubType,
                                            GetFormat(), GetLanguage());

    pTmp->SetValue(GetValue());
    pTmp->SetOffset(nOffset);
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());

    return pTmp;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_uInt16 SwDateTimeField::GetSubType() const
{
    return nSubType;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDateTimeField::SetSubType(sal_uInt16 nType)
{
    nSubType = nType;
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDateTimeField::SetPar2(const String& rStr)
{
    nOffset = rStr.ToInt32();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

String SwDateTimeField::GetPar2() const
{
    if (nOffset)
        return String::CreateFromInt32(nOffset);
    else
        return aEmptyStr;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDateTimeField::SetDateTime(const DateTime& rDT)
{
    SetValue(GetDateTime(GetDoc(), rDT));
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

double SwDateTimeField::GetDateTime(SwDoc* pDoc, const DateTime& rDT)
{
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
    Date* pNullDate = pFormatter->GetNullDate();

    double fResult = rDT - DateTime(*pNullDate);

    return fResult;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

double SwDateTimeField::GetValue() const
{
    if (IsFixed())
        return SwValueField::GetValue();
    else
        return GetDateTime(GetDoc(), DateTime());
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

Date SwDateTimeField::GetDate(sal_Bool bUseOffset) const
{
    SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
    Date* pNullDate = pFormatter->GetNullDate();

    long nVal = static_cast<long>( GetValue() );

    if (bUseOffset && nOffset)
        nVal += nOffset / 60 / 24;

    Date aDate = *pNullDate + nVal;

    return aDate;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

Time SwDateTimeField::GetTime(sal_Bool bUseOffset) const
{
    double fDummy;
    double fFract = modf(GetValue(), &fDummy);
    DateTime aDT((long)fDummy, 0);
    aDT += fFract;
    if (bUseOffset)
         aDT += Time(0, nOffset);
    return (Time)aDT;
}

/*-----------------04.03.98 11:05-------------------

--------------------------------------------------*/
sal_Bool SwDateTimeField::QueryValue( uno::Any& rVal, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        {
            sal_Bool bTmp = IsFixed();
            rVal.setValue(&bTmp, ::getCppuBooleanType());
        }
        break;
    case FIELD_PROP_BOOL2:
        {
            sal_Bool bTmp = IsDate();
            rVal.setValue(&bTmp, ::getCppuBooleanType());
        }
        break;
    case FIELD_PROP_FORMAT:
        rVal <<= (sal_Int32)GetFormat();
        break;
    case FIELD_PROP_SUBTYPE:
        rVal <<= (sal_Int32)nOffset;
        break;
    case FIELD_PROP_DATE_TIME:
        {
            DateTime aDateTime(GetDate(), GetTime());

            util::DateTime DateTimeValue;
            DateTimeValue.HundredthSeconds = aDateTime.Get100Sec();
            DateTimeValue.Seconds = aDateTime.GetSec();
            DateTimeValue.Minutes = aDateTime.GetMin();
            DateTimeValue.Hours = aDateTime.GetHour();
            DateTimeValue.Day = aDateTime.GetDay();
            DateTimeValue.Month = aDateTime.GetMonth();
            DateTimeValue.Year = aDateTime.GetYear();
            rVal <<= DateTimeValue;
        }
        break;
    default:
        return SwField::QueryValue(rVal, nWhichId);
    }
    return sal_True;
}
/*-----------------04.03.98 11:05-------------------

--------------------------------------------------*/
sal_Bool SwDateTimeField::PutValue( const uno::Any& rVal, sal_uInt16 nWhichId )
{
    sal_Int32 nTmp = 0;
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        if(*(sal_Bool*)rVal.getValue())
            nSubType |= FIXEDFLD;
        else
            nSubType &= ~FIXEDFLD;
        break;
    case FIELD_PROP_BOOL2:
        nSubType &=  ~(DATEFLD|TIMEFLD);
        nSubType |= *(sal_Bool*)rVal.getValue() ? DATEFLD : TIMEFLD;
        break;
    case FIELD_PROP_FORMAT:
        rVal >>= nTmp;
        ChangeFormat(nTmp);
        break;
    case FIELD_PROP_SUBTYPE:
        rVal >>= nTmp;
        nOffset = nTmp;
        break;
    case FIELD_PROP_DATE_TIME:
        {
            util::DateTime aDateTimeValue;
            if(!(rVal >>= aDateTimeValue))
                return sal_False;
            DateTime aDateTime;
            aDateTime.Set100Sec(aDateTimeValue.HundredthSeconds);
            aDateTime.SetSec(aDateTimeValue.Seconds);
            aDateTime.SetMin(aDateTimeValue.Minutes);
            aDateTime.SetHour(aDateTimeValue.Hours);
            aDateTime.SetDay(aDateTimeValue.Day);
            aDateTime.SetMonth(aDateTimeValue.Month);
            aDateTime.SetYear(aDateTimeValue.Year);
            SetDateTime(aDateTime);
        }
        break;
        default:
            return SwField::PutValue(rVal, nWhichId);
    }
    return sal_True;
}

