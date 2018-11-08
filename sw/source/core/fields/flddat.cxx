/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <o3tl/temporary.hxx>
#include <tools/datetime.hxx>
#include <svl/zforlist.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <doc.hxx>
#include <fldbas.hxx>
#include <flddat.hxx>
#include <unofldmid.h>

using namespace ::com::sun::star;

SwDateTimeFieldType::SwDateTimeFieldType(SwDoc* pInitDoc)
    : SwValueFieldType( pInitDoc, SwFieldIds::DateTime )
{}

SwFieldType* SwDateTimeFieldType::Copy() const
{
    SwDateTimeFieldType *pTmp = new SwDateTimeFieldType(GetDoc());
    return pTmp;
}

SwDateTimeField::SwDateTimeField(SwDateTimeFieldType* pInitType, sal_uInt16 nSub, sal_uLong nFormat, LanguageType nLng)
    : SwValueField(pInitType, nFormat, nLng, 0.0),
    nSubType(nSub),
    nOffset(0)
{
    if (!nFormat)
    {
        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
        if (nSubType & DATEFLD)
            ChangeFormat(pFormatter->GetFormatIndex(NF_DATE_SYSTEM_SHORT, GetLanguage()));
        else
            ChangeFormat(pFormatter->GetFormatIndex(NF_TIME_HHMMSS, GetLanguage()));
    }
    if (IsFixed())
    {
        DateTime aDateTime( DateTime::SYSTEM );
        SetDateTime(aDateTime);
    }
}

OUString SwDateTimeField::ExpandImpl(SwRootFrame const*const) const
{
    double fVal;

    if (!(IsFixed()))
    {
        DateTime aDateTime( DateTime::SYSTEM );
        fVal = GetDateTime(GetDoc(), aDateTime);
    }
    else
        fVal = GetValue();

    if (nOffset)
        fVal += nOffset * ( 60 / 86400.0 );

    return ExpandValue(fVal, GetFormat(), GetLanguage());
}

SwField* SwDateTimeField::Copy() const
{
    SwDateTimeField *pTmp =
        new SwDateTimeField(static_cast<SwDateTimeFieldType*>(GetTyp()), nSubType,
                                            GetFormat(), GetLanguage());

    pTmp->SetValue(GetValue());
    pTmp->SetOffset(nOffset);
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());

    return pTmp;
}

sal_uInt16 SwDateTimeField::GetSubType() const
{
    return nSubType;
}

void SwDateTimeField::SetSubType(sal_uInt16 nType)
{
    nSubType = nType;
}

void SwDateTimeField::SetPar2(const OUString& rStr)
{
    nOffset = rStr.toInt32();
}

OUString SwDateTimeField::GetPar2() const
{
    if (nOffset)
        return OUString::number(nOffset);
    return OUString();
}

void SwDateTimeField::SetDateTime(const DateTime& rDT)
{
    SetValue(GetDateTime(GetDoc(), rDT));
}

double SwDateTimeField::GetDateTime(SwDoc* pDoc, const DateTime& rDT)
{
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
    const Date& rNullDate = pFormatter->GetNullDate();

    double fResult = rDT - DateTime(rNullDate);

    return fResult;
}

double SwDateTimeField::GetValue() const
{
    if (IsFixed())
        return SwValueField::GetValue();
    else
        return GetDateTime(GetDoc(), DateTime( DateTime::SYSTEM ));
}

Date SwDateTimeField::GetDate() const
{
    SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
    const Date& rNullDate = pFormatter->GetNullDate();

    long nVal = static_cast<long>( GetValue() );

    Date aDate = rNullDate + nVal;

    return aDate;
}

tools::Time SwDateTimeField::GetTime() const
{
    double fFract = modf(GetValue(), &o3tl::temporary(double()));
    DateTime aDT( DateTime::EMPTY );
    aDT.AddTime(fFract);
    return static_cast<tools::Time>(aDT);
}

bool SwDateTimeField::QueryValue( uno::Any& rVal, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        rVal <<= IsFixed();
        break;
    case FIELD_PROP_BOOL2:
        rVal <<= (nSubType & DATEFLD) != 0;
        break;
    case FIELD_PROP_FORMAT:
        rVal <<= static_cast<sal_Int32>(GetFormat());
        break;
    case FIELD_PROP_SUBTYPE:
        rVal <<= static_cast<sal_Int32>(nOffset);
        break;
    case FIELD_PROP_DATE_TIME:
        {
            DateTime aDateTime(GetDate(), GetTime());
            rVal <<= aDateTime.GetUNODateTime();
        }
        break;
    default:
        return SwField::QueryValue(rVal, nWhichId);
    }
    return true;
}

bool SwDateTimeField::PutValue( const uno::Any& rVal, sal_uInt16 nWhichId )
{
    sal_Int32 nTmp = 0;
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        if(*o3tl::doAccess<bool>(rVal))
            nSubType |= FIXEDFLD;
        else
            nSubType &= ~FIXEDFLD;
        break;
    case FIELD_PROP_BOOL2:
        nSubType &=  ~(DATEFLD|TIMEFLD);
        nSubType |= *o3tl::doAccess<bool>(rVal) ? DATEFLD : TIMEFLD;
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
                return false;
            DateTime aDateTime( DateTime::EMPTY );
            aDateTime.SetNanoSec(aDateTimeValue.NanoSeconds);
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
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
