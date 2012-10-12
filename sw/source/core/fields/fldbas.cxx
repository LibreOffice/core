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


#include <float.h>
#include <rtl/math.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <editeng/unolingu.hxx>
#include <unofldmid.h>
#include <doc.hxx>
#include <editsh.hxx>
#include <frame.hxx>
#include <fldbas.hxx>
#include <flddat.hxx>
#include <ndtxt.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <pam.hxx>
#include <docfld.hxx>
#include <swtable.hxx>
#include <docufld.hxx>
#include <expfld.hxx>
#include <shellres.hxx>
#include <calc.hxx>
#include <comcore.hrc>
#include <docary.hxx>

#include <math.h>

using namespace ::com::sun::star;
using namespace nsSwDocInfoSubType;

static sal_uInt16 lcl_GetLanguageOfFormat( sal_uInt16 nLng, sal_uLong nFmt,
                                const SvNumberFormatter& rFormatter )
{
    if( nLng == LANGUAGE_NONE ) // wegen Bug #60010
        nLng = LANGUAGE_SYSTEM;
    else if( nLng == ::GetAppLanguage() )
        switch( rFormatter.GetIndexTableOffset( nFmt ))
        {
        case NF_NUMBER_SYSTEM:
        case NF_DATE_SYSTEM_SHORT:
        case NF_DATE_SYSTEM_LONG:
        case NF_DATETIME_SYSTEM_SHORT_HHMM:
            nLng = LANGUAGE_SYSTEM;
            break;
        default: break;
        }
    return nLng;
}

/*--------------------------------------------------------------------
    Beschreibung: Globals
 --------------------------------------------------------------------*/
// Array der Feldname

std::vector<String>* SwFieldType::pFldNames = 0;

    sal_uInt16 aTypeTab[] = {
    /* RES_DBFLD            */      TYP_DBFLD,
    /* RES_USERFLD          */      TYP_USERFLD,
    /* RES_FILENAMEFLD      */      TYP_FILENAMEFLD,
    /* RES_DBNAMEFLD        */      TYP_DBNAMEFLD,
    /* RES_DATEFLD          */      TYP_DATEFLD,
    /* RES_TIMEFLD          */      TYP_TIMEFLD,
    /* RES_PAGENUMBERFLD    */      TYP_PAGENUMBERFLD,  // dynamisch
    /* RES_AUTHORFLD        */      TYP_AUTHORFLD,
    /* RES_CHAPTERFLD       */      TYP_CHAPTERFLD,
    /* RES_DOCSTATFLD       */      TYP_DOCSTATFLD,
    /* RES_GETEXPFLD        */      TYP_GETFLD,         // dynamisch
    /* RES_SETEXPFLD        */      TYP_SETFLD,         // dynamisch
    /* RES_GETREFFLD        */      TYP_GETREFFLD,
    /* RES_HIDDENTXTFLD     */      TYP_HIDDENTXTFLD,
    /* RES_POSTITFLD        */      TYP_POSTITFLD,
    /* RES_FIXDATEFLD       */      TYP_FIXDATEFLD,
    /* RES_FIXTIMEFLD       */      TYP_FIXTIMEFLD,
    /* RES_REGFLD           */      0,                  // alt
    /* RES_VARREGFLD        */      0,                  // alt
    /* RES_SETREFFLD        */      TYP_SETREFFLD,
    /* RES_INPUTFLD         */      TYP_INPUTFLD,
    /* RES_MACROFLD         */      TYP_MACROFLD,
    /* RES_DDEFLD           */      TYP_DDEFLD,
    /* RES_TABLEFLD         */      TYP_FORMELFLD,
    /* RES_HIDDENPARAFLD    */      TYP_HIDDENPARAFLD,
    /* RES_DOCINFOFLD       */      TYP_DOCINFOFLD,
    /* RES_TEMPLNAMEFLD     */      TYP_TEMPLNAMEFLD,
    /* RES_DBNEXTSETFLD     */      TYP_DBNEXTSETFLD,
    /* RES_DBNUMSETFLD      */      TYP_DBNUMSETFLD,
    /* RES_DBSETNUMBERFLD   */      TYP_DBSETNUMBERFLD,
    /* RES_EXTUSERFLD       */      TYP_EXTUSERFLD,
    /* RES_REFPAGESETFLD    */      TYP_SETREFPAGEFLD,
    /* RES_REFPAGEGETFLD    */      TYP_GETREFPAGEFLD,
    /* RES_INTERNETFLD      */      TYP_INTERNETFLD,
    /* RES_JUMPEDITFLD      */      TYP_JUMPEDITFLD,
    /* RES_SCRIPTFLD        */      TYP_SCRIPTFLD,
    /* RES_DATETIMEFLD      */      0,                  // dynamisch
    /* RES_AUTHORITY        */      TYP_AUTHORITY,
    /* RES_COMBINED_CHARS   */      TYP_COMBINED_CHARS,
    /* RES_DROPDOWN         */      TYP_DROPDOWN
    };
        // ????? TYP_USRINPFLD,



const String& SwFieldType::GetTypeStr(sal_uInt16 nTypeId)
{
    if( !pFldNames )
        _GetFldName();

    if( nTypeId < SwFieldType::pFldNames->size() )
        return (*SwFieldType::pFldNames)[nTypeId];
    else
        return aEmptyStr;
}

/*---------------------------------------------------
 Jedes Feld referenziert einen Feldtypen, der fuer
 jedes Dokument einmalig ist.
 --------------------------------------------------*/

SwFieldType::SwFieldType( sal_uInt16 nWhichId )
    : SwModify(0),
    nWhich( nWhichId )
{
}

namespace
{
    rtl::OUString aEmptyOUStr;
}

const rtl::OUString& SwFieldType::GetName() const
{
    return aEmptyOUStr;
}

bool SwFieldType::QueryValue( uno::Any&, sal_uInt16 ) const
{
    return false;
}
bool SwFieldType::PutValue( const uno::Any& , sal_uInt16 )
{
    return false;
}

/*--------------------------------------------------------------------
    Beschreibung:   Basisklasse aller Felder
                    Felder referenzieren einen Feldtyp
                    Felder sind n-mal vorhanden, Feldtypen nur einmal
 --------------------------------------------------------------------*/

SwField::SwField(SwFieldType* pTyp, sal_uInt32 nFmt, sal_uInt16 nLng) :
    nLang(nLng),
    bIsAutomaticLanguage(sal_True),
    nFormat(nFmt)
{
    OSL_ENSURE( pTyp, "SwField: no SwFieldType" );
    pType = pTyp;
}

SwField::~SwField()
{
}

/*--------------------------------------------------------------------
    Beschreibung: Statt Umweg ueber den Typ
 --------------------------------------------------------------------*/

#ifdef DBG_UTIL
sal_uInt16 SwField::Which() const
{
    OSL_ENSURE(pType, "SwField: No FieldType");
    return pType->Which();
}
#endif

sal_uInt16 SwField::GetTypeId() const
{

    sal_uInt16 nRet;
    switch( pType->Which() )
    {
    case RES_DATETIMEFLD:
        if (GetSubType() & FIXEDFLD)
            nRet = static_cast<sal_uInt16>(GetSubType() & DATEFLD ? TYP_FIXDATEFLD : TYP_FIXTIMEFLD);
        else
            nRet = static_cast<sal_uInt16>(GetSubType() & DATEFLD ? TYP_DATEFLD : TYP_TIMEFLD);
        break;
    case RES_GETEXPFLD:
        nRet = static_cast<sal_uInt16>(nsSwGetSetExpType::GSE_FORMULA & GetSubType() ? TYP_FORMELFLD : TYP_GETFLD);
        break;

    case RES_HIDDENTXTFLD:
        nRet = GetSubType();
        break;

    case RES_SETEXPFLD:
        if( nsSwGetSetExpType::GSE_SEQ & GetSubType() )
            nRet = TYP_SEQFLD;
        else if( ((SwSetExpField*)this)->GetInputFlag() )
            nRet = TYP_SETINPFLD;
        else
            nRet = TYP_SETFLD;
        break;

    case RES_PAGENUMBERFLD:
        nRet = GetSubType();
        if( PG_NEXT == nRet )
            nRet = TYP_NEXTPAGEFLD;
        else if( PG_PREV == nRet )
            nRet = TYP_PREVPAGEFLD;
        else
            nRet = TYP_PAGENUMBERFLD;
        break;

    default:
        nRet = aTypeTab[ pType->Which() ];
    }
    return nRet;
}


/*--------------------------------------------------------------------
    Beschreibung: liefert den Namen oder den Inhalt
 --------------------------------------------------------------------*/

String SwField::GetFieldName() const
{
    sal_uInt16 nTypeId = GetTypeId();
    if (RES_DATETIMEFLD == GetTyp()->Which())
    {
        nTypeId = static_cast<sal_uInt16>(
            ((GetSubType() & DATEFLD) != 0) ? TYP_DATEFLD : TYP_TIMEFLD);
    }
    String sRet = SwFieldType::GetTypeStr( nTypeId );
    if (IsFixed())
    {
        sRet += ' ';
        sRet += ViewShell::GetShellRes()->aFixedStr;
    }
    return sRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Parameter setzen auslesen
 --------------------------------------------------------------------*/

const rtl::OUString& SwField::GetPar1() const
{
    return aEmptyOUStr;
}

rtl::OUString SwField::GetPar2() const
{
    return rtl::OUString();
}

String SwField::GetFormula() const
{
    return GetPar2();
}

void SwField::SetPar1(const rtl::OUString& )
{}

void SwField::SetPar2(const rtl::OUString& )
{}

sal_uInt16 SwField::GetSubType() const
{
    return 0;
}

void SwField::SetSubType(sal_uInt16 )
{
}

bool  SwField::QueryValue( uno::Any& rVal, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
        case FIELD_PROP_BOOL4:
        {
            sal_Bool bFixed = !bIsAutomaticLanguage;
            rVal.setValue(&bFixed, ::getCppuBooleanType());
        }
        break;
        default:
            OSL_FAIL("illegal property");
    }
    return true;
}
bool SwField::PutValue( const uno::Any& rVal, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
        case FIELD_PROP_BOOL4:
        {
            sal_Bool bFixed = sal_False;
            if(rVal >>= bFixed)
                bIsAutomaticLanguage = !bFixed;
        }
        break;
        default:
            OSL_FAIL("illegal property");
    }
    return true;
}


/*--------------------------------------------------------------------
    Beschreibung:   neuen Typ setzen
                    (wird fuer das Kopieren zwischen Dokumenten benutzt)
                    muss immer vom gleichen Typ sein.
 --------------------------------------------------------------------*/

SwFieldType* SwField::ChgTyp( SwFieldType* pNewType )
{
    OSL_ENSURE( pNewType && pNewType->Which() == pType->Which(),
            "kein Typ oder ungleiche Typen" );

    SwFieldType* pOld = pType;
    pType = pNewType;
    return pOld;
}

    // hat das Feld eine Action auf dem ClickHandler ? (z.B. INetFelder,..)
sal_Bool SwField::HasClickHdl() const
{
    sal_Bool bRet = sal_False;
    switch( pType->Which() )
    {
    case RES_INTERNETFLD:
    case RES_JUMPEDITFLD:
    case RES_GETREFFLD:
    case RES_MACROFLD:
    case RES_INPUTFLD:
    case RES_DROPDOWN :
        bRet = sal_True;
        break;

    case RES_SETEXPFLD:
        bRet = ((SwSetExpField*)this)->GetInputFlag();
        break;
    }
    return bRet;
}

void SwField::SetLanguage(sal_uInt16 nLng)
{
    nLang = nLng;
}

void SwField::ChangeFormat(sal_uInt32 n)
{
    nFormat = n;
}

sal_Bool SwField::IsFixed() const
{
    sal_Bool bRet = sal_False;
    switch( pType->Which() )
    {
    case RES_FIXDATEFLD:
    case RES_FIXTIMEFLD:
        bRet = sal_True;
        break;

    case RES_DATETIMEFLD:
        bRet = 0 != (GetSubType() & FIXEDFLD);
        break;

    case RES_EXTUSERFLD:
    case RES_AUTHORFLD:
        bRet = 0 != (GetFormat() & AF_FIXED);
        break;

    case RES_FILENAMEFLD:
        bRet = 0 != (GetFormat() & FF_FIXED);
        break;

    case RES_DOCINFOFLD:
        bRet = 0 != (GetSubType() & DI_SUB_FIXED);
        break;
    }
    return bRet;
}

String SwField::ExpandField(bool const bCached) const
{
    if (!bCached) // #i85766# do not expand fields in clipboard documents
    {
        m_Cache = Expand();
    }
    return m_Cache;
}

SwField * SwField::CopyField() const
{
    SwField *const pNew = Copy();
    // #i85766# cache expansion of source (for clipboard)
    // use this->cache, not this->Expand(): only text formatting calls Expand()
    pNew->m_Cache = m_Cache;
    return pNew;
}

/*--------------------------------------------------------------------
    Beschreibung: Numerierung expandieren
 --------------------------------------------------------------------*/

String FormatNumber(sal_uInt32 nNum, sal_uInt32 nFormat)
{
    if(SVX_NUM_PAGEDESC == nFormat)
        return  String::CreateFromInt32( nNum );
    SvxNumberType aNumber;

    OSL_ENSURE(nFormat != SVX_NUM_NUMBER_NONE, "Falsches Nummern-Format" );

    aNumber.SetNumberingType((sal_Int16)nFormat);
    return aNumber.GetNumStr(nNum);
}

/*--------------------------------------------------------------------
    Beschreibung: CTOR SwValueFieldType
 --------------------------------------------------------------------*/

SwValueFieldType::SwValueFieldType( SwDoc* pDocPtr, sal_uInt16 nWhichId )
    : SwFieldType(nWhichId),
    pDoc(pDocPtr),
    bUseFormat(sal_True)
{
}

SwValueFieldType::SwValueFieldType( const SwValueFieldType& rTyp )
    : SwFieldType(rTyp.Which()),
    pDoc(rTyp.GetDoc()),
    bUseFormat(rTyp.UseFormat())
{
}

/*--------------------------------------------------------------------
    Beschreibung: Wert formatiert als String zurueckgeben
 --------------------------------------------------------------------*/

String SwValueFieldType::ExpandValue( const double& rVal,
                                        sal_uInt32 nFmt, sal_uInt16 nLng) const
{
    if (rVal >= DBL_MAX)        // FehlerString fuer Calculator
        return ViewShell::GetShellRes()->aCalc_Error;

    String sExpand;
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
    Color* pCol = 0;

    // wegen Bug #60010
    sal_uInt16 nFmtLng = ::lcl_GetLanguageOfFormat( nLng, nFmt, *pFormatter );

    if( nFmt < SV_COUNTRY_LANGUAGE_OFFSET && LANGUAGE_SYSTEM != nFmtLng )
    {
        short nType = NUMBERFORMAT_DEFINED;
        xub_StrLen nDummy;

        const SvNumberformat* pEntry = pFormatter->GetEntry(nFmt);

        if (pEntry && nLng != pEntry->GetLanguage())
        {
            sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFmt,
                                                    (LanguageType)nFmtLng);

            if (nNewFormat == nFmt)
            {
                // Warscheinlich benutzerdefiniertes Format
                String sFmt(pEntry->GetFormatstring());

                pFormatter->PutandConvertEntry(sFmt, nDummy, nType, nFmt,
                                        pEntry->GetLanguage(), nFmtLng );
            }
            else
                nFmt = nNewFormat;
        }
        OSL_ENSURE(pEntry, "Unbekanntes Zahlenformat!");
    }

    if( pFormatter->IsTextFormat( nFmt ) )
    {
        String sValue;
        DoubleToString(sValue, rVal, nFmtLng);
        pFormatter->GetOutputString(sValue, nFmt, sExpand, &pCol);
    }
    else
        pFormatter->GetOutputString(rVal, nFmt, sExpand, &pCol);

    return sExpand;
}

void SwValueFieldType::DoubleToString( String &rValue, const double &rVal,
                                        sal_uInt32 nFmt) const
{
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
    const SvNumberformat* pEntry = pFormatter->GetEntry(nFmt);

    if (pEntry)
        DoubleToString(rValue, rVal, pEntry->GetLanguage());
}

void SwValueFieldType::DoubleToString( String &rValue, const double &rVal,
                                        sal_uInt16 nLng ) const
{
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();

    // wegen Bug #60010
    if( nLng == LANGUAGE_NONE ) // wegen Bug #60010
        nLng = LANGUAGE_SYSTEM;

    pFormatter->ChangeIntl( nLng ); // Separator in der richtigen Sprache besorgen
    rValue = ::rtl::math::doubleToUString( rVal, rtl_math_StringFormat_F, 12,
                                    pFormatter->GetDecSep(), true );
}

/*--------------------------------------------------------------------
    Beschreibung: CTOR SwValueField
 --------------------------------------------------------------------*/

SwValueField::SwValueField( SwValueFieldType* pFldType, sal_uInt32 nFmt,
                            sal_uInt16 nLng, const double fVal )
    : SwField(pFldType, nFmt, nLng),
    fValue(fVal)
{
}

SwValueField::SwValueField( const SwValueField& rFld )
    : SwField(rFld),
    fValue(rFld.GetValue())
{
}

SwValueField::~SwValueField()
{
}
/*--------------------------------------------------------------------
    Beschreibung:   neuen Typ setzen
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
    Beschreibung: Format in Office-Sprache ermitteln
 --------------------------------------------------------------------*/

sal_uInt32 SwValueField::GetSystemFormat(SvNumberFormatter* pFormatter, sal_uInt32 nFmt)
{
    const SvNumberformat* pEntry = pFormatter->GetEntry(nFmt);
    sal_uInt16 nLng = SvxLocaleToLanguage( SvtSysLocale().GetLocaleData().getLocale() );

    if (pEntry && nLng != pEntry->GetLanguage())
    {
        sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFmt,
                                                        (LanguageType)nLng);

        if (nNewFormat == nFmt)
        {
            // Warscheinlich benutzerdefiniertes Format
            short nType = NUMBERFORMAT_DEFINED;
            xub_StrLen nDummy;

            String sFmt(pEntry->GetFormatstring());

            sal_uInt32 nFormat = nFmt;
            pFormatter->PutandConvertEntry(sFmt, nDummy, nType,
                                nFormat, pEntry->GetLanguage(), nLng);
            nFmt = nFormat;
        }
        else
            nFmt = nNewFormat;
    }

    return nFmt;
}

/*--------------------------------------------------------------------
    Beschreibung: Sprache im Format anpassen
 --------------------------------------------------------------------*/

void SwValueField::SetLanguage( sal_uInt16 nLng )
{
    if( IsAutomaticLanguage() &&
            ((SwValueFieldType *)GetTyp())->UseFormat() &&
        GetFormat() != SAL_MAX_UINT32 )
    {
        // wegen Bug #60010
        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
        sal_uInt16 nFmtLng = ::lcl_GetLanguageOfFormat( nLng, GetFormat(),
                                                    *pFormatter );

        if( (GetFormat() >= SV_COUNTRY_LANGUAGE_OFFSET ||
             LANGUAGE_SYSTEM != nFmtLng ) &&
            !(Which() == RES_USERFLD && (GetSubType()&nsSwExtendedSubType::SUB_CMD) ) )
        {
            const SvNumberformat* pEntry = pFormatter->GetEntry(GetFormat());

            if( pEntry && nFmtLng != pEntry->GetLanguage() )
            {
                sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(
                                        GetFormat(), (LanguageType)nFmtLng );

                if( nNewFormat == GetFormat() )
                {
                    // Warscheinlich benutzerdefiniertes Format
                    short nType = NUMBERFORMAT_DEFINED;
                    xub_StrLen nDummy;
                    String sFmt( pEntry->GetFormatstring() );
                    pFormatter->PutandConvertEntry( sFmt, nDummy, nType,
                                                    nNewFormat,
                                                    pEntry->GetLanguage(),
                                                    nFmtLng );
                }
                SetFormat( nNewFormat );
            }
            OSL_ENSURE(pEntry, "Unbekanntes Zahlenformat!");
        }
    }

    SwField::SetLanguage(nLng);
}

double SwValueField::GetValue() const
{
    return fValue;
}

void SwValueField::SetValue( const double& rVal )
{
    fValue = rVal;
}

/*--------------------------------------------------------------------
    Beschreibung: SwFormulaField
 --------------------------------------------------------------------*/

SwFormulaField::SwFormulaField( SwValueFieldType* pFldType, sal_uInt32 nFmt, const double fVal)
    : SwValueField(pFldType, nFmt, LANGUAGE_SYSTEM, fVal)
{
}

SwFormulaField::SwFormulaField( const SwFormulaField& rFld )
    : SwValueField((SwValueFieldType *)rFld.GetTyp(), rFld.GetFormat(),
                    rFld.GetLanguage(), rFld.GetValue())
{
}

String SwFormulaField::GetFormula() const
{
    return sFormula;
}

void SwFormulaField::SetFormula(const String& rStr)
{
    sFormula = rStr;

    sal_uLong nFmt(GetFormat());

    if( nFmt && SAL_MAX_UINT32 != nFmt )
    {
        xub_StrLen nPos = 0;
        double fTmpValue;
        if( SwCalc::Str2Double( rStr, nPos, fTmpValue, GetDoc() ) )
            SwValueField::SetValue( fTmpValue );
    }
}

void SwFormulaField::SetExpandedFormula( const String& rStr )
{
    sal_uInt32 nFmt(GetFormat());

    if (nFmt && nFmt != SAL_MAX_UINT32 && ((SwValueFieldType *)GetTyp())->UseFormat())
    {
        double fTmpValue;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        if (pFormatter->IsNumberFormat(rStr, nFmt, fTmpValue))
        {
            SwValueField::SetValue(fTmpValue);
            sFormula.Erase();

            ((SwValueFieldType *)GetTyp())->DoubleToString(sFormula, fTmpValue, nFmt);
            return;
        }
    }
    sFormula = rStr;
}

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

String SwField::GetDescription() const
{
    return SW_RES(STR_FIELD);
}

sal_uInt16 SwFldTypes::GetPos(const SwFieldType* pFieldType) const
{
    const_iterator it = std::find(begin(), end(), pFieldType);
    return it == end() ? USHRT_MAX : it - begin();
}

SwFldTypes::~SwFldTypes()
{
    for(const_iterator it = begin(); it != end(); ++it)
        delete *it;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
