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

#ifdef MAC
#include <stdlib.h>
#endif
#include <float.h>

#include <rtl/math.hxx>
#include <bf_svtools/zformat.hxx>
#include <unofldmid.h>

#include <horiornt.hxx>

#include <errhdl.hxx>
#include <viscrs.hxx>
#include <editsh.hxx>
#include <docfld.hxx>
#include <docufld.hxx>
#include <shellres.hxx>
namespace binfilter {


using namespace ::com::sun::star;

/*N*/ USHORT lcl_GetLanguageOfFormat( USHORT nLng, ULONG nFmt,
/*N*/ 								const SvNumberFormatter& rFormatter )
/*N*/ {
/*N*/ 	if( nLng == LANGUAGE_NONE )	// wegen Bug #60010
/*?*/ 		nLng = LANGUAGE_SYSTEM;
/*N*/ 	else if( nLng == ::binfilter::GetAppLanguage() )
/*N*/ 		switch( rFormatter.GetIndexTableOffset( nFmt ))
/*N*/ 		{
/*?*/ 		case NF_NUMBER_SYSTEM:
/*?*/ 		case NF_DATE_SYSTEM_SHORT:
/*?*/ 		case NF_DATE_SYSTEM_LONG:
/*?*/ 		case NF_DATETIME_SYSTEM_SHORT_HHMM:
/*?*/ 			nLng = LANGUAGE_SYSTEM;
/*?*/ 			break;
/*N*/ 		}
/*N*/ 	return nLng;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Globals
 --------------------------------------------------------------------*/
// Array der Feldname

/*N*/ SvStringsDtor* SwFieldType::pFldNames = 0;

/*N*/ DBG_NAME(SwFieldType)

        // ????? TYP_USRINPFLD,



/*N*/ const String& SwFieldType::GetTypeStr(USHORT nTypeId)
/*N*/ {
/*N*/ 	if( !pFldNames )
/*N*/ 		_GetFldName();
/*N*/ 
/*N*/ 	if( nTypeId < SwFieldType::pFldNames->Count() )
/*N*/ 		return *SwFieldType::pFldNames->GetObject( nTypeId );
/*N*/ 	else
/*?*/ 		return aEmptyStr;
/*N*/ }

/*---------------------------------------------------
 Jedes Feld referenziert einen Feldtypen, der fuer
 jedes Dokument einmalig ist.
 --------------------------------------------------*/

/*N*/ SwFieldType::SwFieldType( USHORT nWhichId )
/*N*/ 	: SwModify(0),
/*N*/ 	nWhich( nWhichId )
/*N*/ {
/*N*/ 	DBG_CTOR( SwFieldType, 0 );
/*N*/ }

/*N*/ #ifdef DBG_UTIL

/*N*/ SwFieldType::~SwFieldType()
/*N*/ {
/*N*/ 	DBG_DTOR( SwFieldType, 0 );
/*N*/ }

/*N*/ #endif

/*N*/ const String& SwFieldType::GetName() const
/*N*/ {
/*N*/ 	return aEmptyStr;
/*N*/ }


/*--------------------------------------------------------------------
    Beschreibung:	Basisklasse aller Felder
                    Felder referenzieren einen Feldtyp
                    Felder sind n-mal vorhanden, Feldtypen nur einmal
 --------------------------------------------------------------------*/

/*N*/ SwField::SwField(SwFieldType* pTyp, sal_uInt32 nFmt, USHORT nLng) :
/*N*/ 	nFormat(nFmt),
/*N*/     nLang(nLng),
/*N*/     bIsAutomaticLanguage(TRUE)
/*N*/ {
/*N*/ 	ASSERT( pTyp, "SwField: ungueltiger SwFieldType" );
/*N*/ 	pType = pTyp;
/*N*/ }

/*N*/ SwField::~SwField()
/*N*/ {
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Statt Umweg ueber den Typ
 --------------------------------------------------------------------*/

/*N*/ #ifdef DBG_UTIL
/*N*/ USHORT SwField::Which() const
/*N*/ {
/*N*/ 	ASSERT(pType, "Kein Typ vorhanden");
/*N*/ 	return pType->Which();
/*N*/ }
/*N*/ #endif

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/




/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



/*--------------------------------------------------------------------
    Beschreibung: liefert den Namen oder den Inhalt
 --------------------------------------------------------------------*/

/*N*/ String SwField::GetCntnt( BOOL bName ) const
/*N*/ {
/*N*/ 	String sRet;
/*N*/ 	if( bName )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 USHORT nTypeId = GetTypeId();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		sRet = Expand();
/*N*/ 	return sRet;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Parameter setzen auslesen
 --------------------------------------------------------------------*/

/*N*/ const String& SwField::GetPar1() const
/*N*/ {
/*N*/ 	return aEmptyStr;
/*N*/ }

/*N*/ String SwField::GetPar2() const
/*N*/ {
/*N*/ 	return aEmptyStr;
/*N*/ }

/*N*/ String SwField::GetFormula() const
/*N*/ {
/*N*/ 	return GetPar2();
/*N*/ }

void SwField::SetPar1(const String& rStr)
{}

void SwField::SetPar2(const String& rStr)
 {}

/*N*/ USHORT SwField::GetSubType() const
/*N*/ {
/*N*/ //	ASSERT(0, "Sorry Not implemented");
/*N*/ 	return 0;
/*N*/ }

void SwField::SetSubType(USHORT nType)
{
//  ASSERT(0, "Sorry Not implemented");
}

/*N*/ BOOL  SwField::QueryValue( uno::Any& rVal, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/     switch( nMId )
/*N*/     {
/*N*/         case FIELD_PROP_BOOL4:
/*N*/         {
/*N*/             BOOL bFixed = !bIsAutomaticLanguage;
/*N*/             rVal.setValue(&bFixed, ::getCppuBooleanType());
/*N*/         }
/*N*/         break;
/*N*/         default:
/*N*/             DBG_ERROR("illegal property");
/*N*/     }
/*N*/     return TRUE;
/*N*/ }
/*N*/ BOOL SwField::PutValue( const uno::Any& rVal, BYTE nMId )
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/     switch( nMId )
/*N*/     {
/*N*/         case FIELD_PROP_BOOL4:
/*N*/         {    
/*N*/             BOOL bFixed;
/*N*/             if(rVal >>= bFixed)
/*N*/                 bIsAutomaticLanguage = !bFixed;
/*N*/         }
/*N*/         break;
/*N*/         default:
/*N*/             DBG_ERROR("illegal property");
/*N*/     }
/*N*/     return TRUE;
/*N*/ }


/*--------------------------------------------------------------------
    Beschreibung: 	neuen Typ setzen
                    (wird fuer das Kopieren zwischen Dokumenten benutzt)
                    muss immer vom gleichen Typ sein.
 --------------------------------------------------------------------*/

SwFieldType* SwField::ChgTyp( SwFieldType* pNewType )
{
    ASSERT( pNewType && pNewType->Which() == pType->Which(),
            "kein Typ oder ungleiche Typen" );

    SwFieldType* pOld = pType;
    pType = pNewType;
    return pOld;
}

    // hat das Feld eine Action auf dem ClickHandler ? (z.B. INetFelder,..)

/*N*/ void SwField::SetLanguage(USHORT nLng)
/*N*/ {
/*N*/ 	nLang = nLng;
/*N*/ }

/*N*/ void SwField::ChangeFormat(sal_uInt32 n)
/*N*/ {
/*N*/ 	nFormat = n;
/*N*/ }

/*N*/ FASTBOOL SwField::IsFixed() const
/*N*/ {
/*N*/ 	FASTBOOL bRet = FALSE;
/*N*/ 	switch( pType->Which() )
/*N*/ 	{
/*N*/ 	case RES_FIXDATEFLD:
/*N*/ 	case RES_FIXTIMEFLD:
/*?*/ 		bRet = TRUE;
/*?*/ 		break;
/*N*/ 
/*N*/ 	case RES_DATETIMEFLD:
/*N*/ 		bRet = 0 != (GetSubType() & FIXEDFLD);
/*N*/ 		break;
/*N*/ 
/*N*/ 	case RES_EXTUSERFLD:
/*N*/ 	case RES_AUTHORFLD:
/*N*/ 		bRet = 0 != (GetFormat() & AF_FIXED);
/*N*/ 		break;
/*N*/ 
/*N*/ 	case RES_FILENAMEFLD:
/*N*/ 		bRet = 0 != (GetFormat() & FF_FIXED);
/*N*/ 		break;
/*N*/ 
/*N*/ 	case RES_DOCINFOFLD:
/*N*/ 		bRet = 0 != (GetSubType() & DI_SUB_FIXED);
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Sortierte Feldliste aller Felder usw.
 --------------------------------------------------------------------*/




/*--------------------------------------------------------------------
    Beschreibung: Felder aus der Liste in sortierter Reihenfolge
 --------------------------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung: 	von der CursorPos das naechste und das Letzte Feld
                    in der Liste anfahren
 --------------------------------------------------------------------*/



/*--------------------------------------------------------------------
    Beschreibung: Numerierung expandieren
 --------------------------------------------------------------------*/

/*N*/ String FormatNumber(USHORT nNum, sal_uInt32 nFormat)
/*N*/ {
/*N*/     if(SVX_NUM_PAGEDESC == nFormat)
/*?*/         return  String::CreateFromInt32( nNum );
/*N*/     SvxNumberType aNumber;
/*N*/ 
/*N*/ 	ASSERT(nFormat != SVX_NUM_NUMBER_NONE, "Falsches Nummern-Format" );
/*N*/ 
/*N*/ 	aNumber.SetNumberingType((sal_Int16)nFormat);
/*N*/ 	return aNumber.GetNumStr(nNum);
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: CTOR SwValueFieldType
 --------------------------------------------------------------------*/

/*N*/ SwValueFieldType::SwValueFieldType( SwDoc* pDocPtr, USHORT nWhichId )
/*N*/ 	: SwFieldType(nWhichId),
/*N*/ 	pDoc(pDocPtr),
/*N*/ 	bUseFormat(TRUE)
/*N*/ {
/*N*/ }


/*--------------------------------------------------------------------
    Beschreibung: Wert formatiert als String zurueckgeben
 --------------------------------------------------------------------*/

/*N*/ String SwValueFieldType::ExpandValue( const double& rVal,
/*N*/ 										sal_uInt32 nFmt, USHORT nLng) const
/*N*/ {
/*N*/ 	if (rVal >= DBL_MAX)		// FehlerString fuer Calculator
/*?*/ 		return ViewShell::GetShellRes()->aCalc_Error;
/*N*/ 
/*N*/ 	String sExpand;
/*N*/ 	SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
/*N*/ 	Color* pCol = 0;
/*N*/ 
/*N*/ 	// wegen Bug #60010
/*N*/ 	USHORT nFmtLng = ::binfilter::lcl_GetLanguageOfFormat( nLng, nFmt, *pFormatter );
/*N*/ 
/*N*/ 	if( nFmt < SV_COUNTRY_LANGUAGE_OFFSET && LANGUAGE_SYSTEM != nFmtLng )
/*N*/ 	{
/*?*/ 		short nType = NUMBERFORMAT_DEFINED;
/*?*/ 		xub_StrLen nDummy;
/*?*/ 
/*?*/ 		const SvNumberformat* pEntry = pFormatter->GetEntry(nFmt);
/*?*/ 
/*?*/ 		if (pEntry && nLng != pEntry->GetLanguage())
/*?*/ 		{
/*?*/ 			sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFmt,
/*?*/ 													(LanguageType)nFmtLng);
/*?*/ 
/*?*/ 			if (nNewFormat == nFmt)
/*?*/ 			{
/*?*/ 				// Warscheinlich benutzerdefiniertes Format
/*?*/ 				String sFmt(pEntry->GetFormatstring());
/*?*/ 
/*?*/ 				pFormatter->PutandConvertEntry(sFmt, nDummy, nType, nFmt,
/*?*/ 										pEntry->GetLanguage(), nFmtLng );
/*?*/ 			}
/*?*/ 			else
/*?*/ 				nFmt = nNewFormat;
/*?*/ 		}
/*?*/ 		ASSERT(pEntry, "Unbekanntes Zahlenformat!");
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pFormatter->IsTextFormat( nFmt ) )
/*N*/ 	{
/*?*/ 		String sValue;
/*?*/ 		DoubleToString(sValue, rVal, nFmtLng);
/*?*/ 		pFormatter->GetOutputString(sValue, nFmt, sExpand, &pCol);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pFormatter->GetOutputString(rVal, nFmt, sExpand, &pCol);
/*N*/ 
/*N*/ 	return sExpand;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ void SwValueFieldType::DoubleToString( String &rValue, const double &rVal,
/*N*/ 										USHORT nLng ) const
/*N*/ {
/*N*/ 	SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
/*N*/ 
/*N*/ 	// wegen Bug #60010
/*N*/ 	if( nLng == LANGUAGE_NONE )	// wegen Bug #60010
/*?*/ 		nLng = LANGUAGE_SYSTEM;
/*N*/ 
/*N*/ 	pFormatter->ChangeIntl( nLng );	// Separator in der richtigen Sprache besorgen
/*N*/ 	rValue = ::rtl::math::doubleToUString( rVal, rtl_math_StringFormat_F, 12,
/*N*/ 									pFormatter->GetDecSep(), TRUE );
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: CTOR SwValueField
 --------------------------------------------------------------------*/

/*N*/ SwValueField::SwValueField( SwValueFieldType* pFldType, sal_uInt32 nFmt,
/*N*/ 							USHORT nLang, const double fVal )
/*N*/ 	: SwField(pFldType, nFmt, nLang),
/*N*/ 	fValue(fVal)
/*N*/ {
/*N*/ }


/*N*/ SwValueField::~SwValueField()
/*N*/ {
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung: 	neuen Typ setzen
                    (wird fuer das Kopieren zwischen Dokumenten benutzt)
                    muss immer vom gleichen Typ sein.
 --------------------------------------------------------------------*/

SwFieldType* SwValueField::ChgTyp( SwFieldType* pNewType )
{
    SwDoc* pNewDoc = ((SwValueFieldType *)pNewType)->GetDoc();
    SwDoc* pDoc    = GetDoc();

    if( pNewDoc && pDoc && pDoc != pNewDoc)
    {
        SvNumberFormatter* pFormatter = pNewDoc->GetNumberFormatter();

        if( pFormatter && pFormatter->HasMergeFmtTbl() &&
            ((SwValueFieldType *)GetTyp())->UseFormat() )
            SetFormat(pFormatter->GetMergeFmtIndex( GetFormat() ));
    }

    return SwField::ChgTyp(pNewType);
}

/*--------------------------------------------------------------------
    Beschreibung: Format aendern
 --------------------------------------------------------------------*/
/*
 Was sollte das denn?
void SwValueField::ChangeFormat(ULONG n)
{
    nFormat = n;
}

/*--------------------------------------------------------------------
    Beschreibung: Format in Office-Sprache ermitteln
 --------------------------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung: Sprache im Format anpassen
 --------------------------------------------------------------------*/

/*N*/ void SwValueField::SetLanguage( USHORT nLng )
/*N*/ {
/*N*/     if( IsAutomaticLanguage() && 
/*N*/             ((SwValueFieldType *)GetTyp())->UseFormat() &&
/*N*/ 		GetFormat() != SAL_MAX_UINT32 )
/*N*/ 	{
/*N*/ 		// wegen Bug #60010
/*N*/ 		SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
/*N*/ 		USHORT nFmtLng = ::binfilter::lcl_GetLanguageOfFormat( nLng, GetFormat(),
/*N*/ 													*pFormatter );
/*N*/ 
/*N*/ 		if( (GetFormat() >= SV_COUNTRY_LANGUAGE_OFFSET ||
/*N*/ 			 LANGUAGE_SYSTEM != nFmtLng ) &&
/*N*/ 			!(Which() == RES_USERFLD && (GetSubType()&SUB_CMD) ) )
/*N*/ 		{
/*N*/ 			const SvNumberformat* pEntry = pFormatter->GetEntry(GetFormat());
/*N*/ 
/*N*/ 			if( pEntry && nFmtLng != pEntry->GetLanguage() )
/*N*/ 			{
/*N*/ 				sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(
/*N*/ 										GetFormat(), (LanguageType)nFmtLng );
/*N*/ 
/*N*/ 				if( nNewFormat == GetFormat() )
/*N*/ 				{
/*?*/ 					// Warscheinlich benutzerdefiniertes Format
/*?*/ 					short nType = NUMBERFORMAT_DEFINED;
/*?*/ 					xub_StrLen nDummy;
/*?*/ 					String sFmt( pEntry->GetFormatstring() );
/*?*/ 					pFormatter->PutandConvertEntry( sFmt, nDummy, nType,
/*?*/ 													nNewFormat,
/*?*/ 													pEntry->GetLanguage(),
/*?*/ 													nFmtLng );
/*N*/ 				}
/*N*/ 				SetFormat( nNewFormat );
/*N*/ 			}
/*N*/ 			ASSERT(pEntry, "Unbekanntes Zahlenformat!");
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SwField::SetLanguage(nLng);
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ double SwValueField::GetValue() const
/*N*/ {
/*N*/ 	return fValue;
/*N*/ }

/*N*/ void SwValueField::SetValue( const double& rVal )
/*N*/ {
/*N*/ 	fValue = rVal;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: SwFormulaField
 --------------------------------------------------------------------*/

/*N*/ SwFormulaField::SwFormulaField( SwValueFieldType* pFldType, sal_uInt32 nFmt, const double fVal)
/*N*/ 	: SwValueField(pFldType, nFmt, LANGUAGE_SYSTEM, fVal)
/*N*/ {
/*N*/ }


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ String SwFormulaField::GetFormula() const
/*N*/ {
/*N*/ 	return sFormula;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ void SwFormulaField::SetFormula(const String& rStr)
/*N*/ {
/*N*/ 	sFormula = rStr;
/*N*/ 
/*N*/ 	sal_uInt32 nFmt(GetFormat());
/*N*/ 
/*N*/ 	if( nFmt && SAL_MAX_UINT32 != nFmt )
/*N*/ 	{
/*N*/ 		xub_StrLen nPos = 0;
/*N*/ 		double fValue;
/*N*/ 		if( SwCalc::Str2Double( rStr, nPos, fValue, GetDoc() ) )
/*N*/ 			SwValueField::SetValue( fValue );
/*N*/ 	}
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFormulaField::SetExpandedFormula( const String& rStr )
{
    sal_uInt32 nFmt(GetFormat());

    if (nFmt && nFmt != SAL_MAX_UINT32 && ((SwValueFieldType *)GetTyp())->UseFormat())
    {
        double fValue;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        if (pFormatter->IsNumberFormat(rStr, nFmt, fValue))
        {
            SwValueField::SetValue(fValue);
            sFormula.Erase();

            ((SwValueFieldType *)GetTyp())->DoubleToString(sFormula, fValue, nFmt);
            return;
        }
    }
    sFormula = rStr;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

String SwFormulaField::GetExpandedFormula() const
{
    sal_uInt32 nFmt(GetFormat());

    if (nFmt && nFmt != SAL_MAX_UINT32 && ((SwValueFieldType *)GetTyp())->UseFormat())
    {
        String sFormattedValue;
        Color* pCol = 0;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        if (pFormatter->IsTextFormat(nFmt))
        {
            String sValue;
            ((SwValueFieldType *)GetTyp())->DoubleToString(sValue, GetValue(), nFmt);
            pFormatter->GetOutputString(sValue, nFmt, sFormattedValue, &pCol);
        }
        else
            pFormatter->GetOutputString(GetValue(), nFmt, sFormattedValue, &pCol);

        return sFormattedValue;
    }
    else
        return GetFormula();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
