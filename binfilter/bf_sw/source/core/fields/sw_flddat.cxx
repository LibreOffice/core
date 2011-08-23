/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <math.h>

#include <bf_svtools/zforlist.hxx>
#include <com/sun/star/util/DateTime.hpp>

#include <horiornt.hxx>

#include <doc.hxx>
#include <flddat.hxx>
#include <unofldmid.h>
namespace binfilter {

using namespace ::com::sun::star;
/*--------------------------------------------------
    Beschreibung: Datum/Zeit-Typ
 ---------------------------------------------------*/

/*N*/ SwDateTimeFieldType::SwDateTimeFieldType(SwDoc* pDoc)
/*N*/ 	: SwValueFieldType( pDoc, RES_DATETIMEFLD )
/*N*/ {}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ SwFieldType* SwDateTimeFieldType::Copy() const
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 //STRIP001 	SwDateTimeFieldType *pTmp = new SwDateTimeFieldType(GetDoc());
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Datum/Zeit-Feld
 --------------------------------------------------------------------*/

/*N*/ SwDateTimeField::SwDateTimeField(SwDateTimeFieldType* pType, USHORT nSub, ULONG nFmt, USHORT nLng)
/*N*/ 	: SwValueField(pType, nFmt, nLng, 0.0),
/*N*/ 	nSubType(nSub),
/*N*/ 	nOffset(0)
/*N*/ {
/*N*/ 	if (!nFmt)
/*N*/ 	{
/*N*/ 		SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
/*N*/ 		if (nSubType & DATEFLD)
/*N*/ 			ChangeFormat(pFormatter->GetFormatIndex(NF_DATE_SYSTEM_SHORT, GetLanguage()));
/*N*/ 		else
/*?*/ 			ChangeFormat(pFormatter->GetFormatIndex(NF_TIME_HHMMSS, GetLanguage()));
/*N*/ 	}
/*N*/ 	if (IsFixed())
/*N*/ 	{
/*N*/         DateTime aDateTime;
/*N*/         SetDateTime(aDateTime);
/*N*/ 	}
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ String SwDateTimeField::Expand() const
/*N*/ {
/*N*/ 	double fVal;
/*N*/ 
/*N*/ 	if (!(IsFixed()))
/*N*/ 	{
/*N*/         DateTime aDateTime;
/*N*/         fVal = GetDateTime(GetDoc(), aDateTime);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		fVal = GetValue();
/*N*/ 
/*N*/ 	if (nOffset)
/*N*/ 		fVal += (double)(nOffset * 60L) / 86400.0;
/*N*/ 
/*N*/ 	return ExpandValue(fVal, GetFormat(), GetLanguage());
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ SwField* SwDateTimeField::Copy() const
/*N*/ {
/*N*/ 	SwDateTimeField *pTmp =
/*N*/ 		new SwDateTimeField((SwDateTimeFieldType*)GetTyp(), nSubType,
/*N*/ 											GetFormat(), GetLanguage());
/*N*/ 
/*N*/ 	pTmp->SetValue(GetValue());
/*N*/ 	pTmp->SetOffset(nOffset);
/*N*/     pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
/*N*/ 
/*N*/ 	return pTmp;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ USHORT SwDateTimeField::GetSubType() const
/*N*/ {
/*N*/ 	return nSubType;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ void SwDateTimeField::SetSubType(USHORT nType) //SW40.SDW 
/*N*/ {
/*N*/ 	nSubType = nType;
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ void SwDateTimeField::SetDateTime(const DateTime& rDT)
/*N*/ {
/*N*/     SetValue(GetDateTime(GetDoc(), rDT));
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ double SwDateTimeField::GetDateTime(SwDoc* pDoc, const DateTime& rDT)
/*N*/ {
/*N*/ 	SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
/*N*/     Date* pNullDate = pFormatter->GetNullDate();
/*N*/ 
/*N*/     double fResult = rDT - DateTime(*pNullDate);
/*N*/     
/*N*/     return fResult;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ double SwDateTimeField::GetValue() const
/*N*/ {
/*N*/ 	if (IsFixed())
/*N*/ 		return SwValueField::GetValue();
/*N*/ 	else
/*N*/         return GetDateTime(GetDoc(), DateTime());
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ Date SwDateTimeField::GetDate(BOOL bUseOffset) const
/*N*/ {
/*N*/ 	SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
/*N*/ 	Date* pNullDate = pFormatter->GetNullDate();
/*N*/ 
/*N*/ 	long nVal = GetValue();
/*N*/ 
/*N*/ 	if (bUseOffset && nOffset)
/*?*/ 		nVal += nOffset / 60 / 24;
/*N*/ 
/*N*/ 	Date aDate = *pNullDate + nVal;
/*N*/ 
/*N*/ 	return aDate;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ Time SwDateTimeField::GetTime(BOOL bUseOffset) const
/*N*/ {
/*N*/     double fDummy;
/*N*/     double fFract = modf(GetValue(), &fDummy);
/*N*/     DateTime aDT((long)fDummy, 0);
/*N*/     aDT += fFract;
/*N*/     if (bUseOffset)
/*N*/          aDT += Time(0, nOffset);
/*N*/     return (Time)aDT;
/*N*/ }

/*-----------------04.03.98 11:05-------------------

--------------------------------------------------*/
/*N*/ BOOL SwDateTimeField::QueryValue( uno::Any& rVal, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_BOOL1:
/*N*/ 		{
/*N*/ 			BOOL bTmp = IsFixed();
/*N*/ 			rVal.setValue(&bTmp, ::getCppuBooleanType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_BOOL2:
/*N*/ 		{
/*N*/ 			BOOL bTmp = IsDate();
/*N*/ 			rVal.setValue(&bTmp, ::getCppuBooleanType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		rVal <<= (sal_Int32)GetFormat();
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_SUBTYPE:
/*N*/ 		rVal <<= (sal_Int32)nOffset;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_DATE_TIME:
/*N*/ 		{
/*N*/             DateTime aDateTime(GetDate(), GetTime());
/*N*/ 
/*N*/ 			util::DateTime DateTimeValue;
/*N*/ 			DateTimeValue.HundredthSeconds = aDateTime.Get100Sec();
/*N*/     		DateTimeValue.Seconds = aDateTime.GetSec();
/*N*/     		DateTimeValue.Minutes = aDateTime.GetMin();
/*N*/     		DateTimeValue.Hours = aDateTime.GetHour();
/*N*/     		DateTimeValue.Day = aDateTime.GetDay();
/*N*/     		DateTimeValue.Month = aDateTime.GetMonth();
/*N*/     		DateTimeValue.Year = aDateTime.GetYear();
/*N*/ 			rVal <<= DateTimeValue;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	default:
/*N*/         return SwField::QueryValue(rVal, nMId);
/*N*/     }
/*N*/ 	return TRUE;
/*N*/ }
/*-----------------04.03.98 11:05-------------------

--------------------------------------------------*/
/*N*/ BOOL SwDateTimeField::PutValue( const uno::Any& rVal, BYTE nMId )
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Int32 nTmp;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_BOOL1:
/*N*/ 		if(*(sal_Bool*)rVal.getValue())
/*N*/ 			nSubType |= FIXEDFLD;
/*N*/ 		else
/*N*/ 			nSubType &= ~FIXEDFLD;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_BOOL2:
/*N*/ 		nSubType &=  ~(DATEFLD|TIMEFLD);
/*N*/ 		nSubType |= *(sal_Bool*)rVal.getValue() ? DATEFLD : TIMEFLD;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		rVal >>= nTmp;
/*N*/ 		ChangeFormat(nTmp);
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_SUBTYPE:
/*N*/ 		rVal >>= nTmp;
/*N*/ 		nOffset = nTmp;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_DATE_TIME:
/*N*/ 		{
/*N*/ 			util::DateTime aDateTimeValue;
/*N*/ 			if(!(rVal >>= aDateTimeValue))
/*N*/ 				return FALSE;
/*N*/ 			DateTime aDateTime;
/*N*/ 			aDateTime.Set100Sec(aDateTimeValue.HundredthSeconds);
/*N*/     		aDateTime.SetSec(aDateTimeValue.Seconds);
/*N*/     		aDateTime.SetMin(aDateTimeValue.Minutes);
/*N*/     		aDateTime.SetHour(aDateTimeValue.Hours);
/*N*/     		aDateTime.SetDay(aDateTimeValue.Day);
/*N*/     		aDateTime.SetMonth(aDateTimeValue.Month);
/*N*/     		aDateTime.SetYear(aDateTimeValue.Year);
/*N*/             SetDateTime(aDateTime);
/*N*/ 		}
/*N*/ 		break;
/*N*/         default:
/*N*/             return SwField::PutValue(rVal, nMId);
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
