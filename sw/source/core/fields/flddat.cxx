/*************************************************************************
 *
 *  $RCSfile: flddat.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:05:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#include <math.h>

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif

using namespace ::com::sun::star;
/*--------------------------------------------------
    Beschreibung: Datum/Zeit-Typ
 ---------------------------------------------------*/

SwDateTimeFieldType::SwDateTimeFieldType(SwDoc* pDoc)
    : SwValueFieldType( pDoc, RES_DATETIMEFLD )
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

SwDateTimeField::SwDateTimeField(SwDateTimeFieldType* pType, USHORT nSub, ULONG nFmt, USHORT nLng)
    : SwValueField(pType, nFmt, nLng, 0.0),
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

USHORT SwDateTimeField::GetSubType() const
{
    return nSubType;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDateTimeField::SetSubType(USHORT nType)
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

Date SwDateTimeField::GetDate(BOOL bUseOffset) const
{
    SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
    Date* pNullDate = pFormatter->GetNullDate();

    long nVal = GetValue();

    if (bUseOffset && nOffset)
        nVal += nOffset / 60 / 24;

    Date aDate = *pNullDate + nVal;

    return aDate;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

Time SwDateTimeField::GetTime(BOOL bUseOffset) const
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
BOOL SwDateTimeField::QueryValue( uno::Any& rVal, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_BOOL1:
        {
            BOOL bTmp = IsFixed();
            rVal.setValue(&bTmp, ::getCppuBooleanType());
        }
        break;
    case FIELD_PROP_BOOL2:
        {
            BOOL bTmp = IsDate();
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
        return SwField::QueryValue(rVal, nMId);
    }
    return TRUE;
}
/*-----------------04.03.98 11:05-------------------

--------------------------------------------------*/
BOOL SwDateTimeField::PutValue( const uno::Any& rVal, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    sal_Int32 nTmp;
    switch( nMId )
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
                return FALSE;
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
            return SwField::PutValue(rVal, nMId);
    }
    return TRUE;
}

