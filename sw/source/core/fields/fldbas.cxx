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
#include <authfld.hxx>

#include <math.h>

using namespace ::com::sun::star;
using namespace nsSwDocInfoSubType;

static sal_uInt16 lcl_GetLanguageOfFormat( sal_uInt16 nLng, sal_uLong nFmt,
                                const SvNumberFormatter& rFormatter )
{
    if( nLng == LANGUAGE_NONE ) // Bug #60010
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

// Globals

/// field names
std::vector<OUString>* SwFieldType::pFldNames = 0;

namespace
{

    const sal_uInt16 aTypeTab[] = {
    /* RES_DBFLD            */      TYP_DBFLD,
    /* RES_USERFLD          */      TYP_USERFLD,
    /* RES_FILENAMEFLD      */      TYP_FILENAMEFLD,
    /* RES_DBNAMEFLD        */      TYP_DBNAMEFLD,
    /* RES_DATEFLD          */      TYP_DATEFLD,
    /* RES_TIMEFLD          */      TYP_TIMEFLD,
    /* RES_PAGENUMBERFLD    */      TYP_PAGENUMBERFLD,  // dynamic
    /* RES_AUTHORFLD        */      TYP_AUTHORFLD,
    /* RES_CHAPTERFLD       */      TYP_CHAPTERFLD,
    /* RES_DOCSTATFLD       */      TYP_DOCSTATFLD,
    /* RES_GETEXPFLD        */      TYP_GETFLD,         // dynamic
    /* RES_SETEXPFLD        */      TYP_SETFLD,         // dynamic
    /* RES_GETREFFLD        */      TYP_GETREFFLD,
    /* RES_HIDDENTXTFLD     */      TYP_HIDDENTXTFLD,
    /* RES_POSTITFLD        */      TYP_POSTITFLD,
    /* RES_FIXDATEFLD       */      TYP_FIXDATEFLD,
    /* RES_FIXTIMEFLD       */      TYP_FIXTIMEFLD,
    /* RES_REGFLD           */      0,                  // old (no change since 2000)
    /* RES_VARREGFLD        */      0,                  // old (no change since 2000)
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
    /* RES_DATETIMEFLD      */      0,                  // dynamic
    /* RES_AUTHORITY        */      TYP_AUTHORITY,
    /* RES_COMBINED_CHARS   */      TYP_COMBINED_CHARS,
    /* RES_DROPDOWN         */      TYP_DROPDOWN
    };

}

OUString SwFieldType::GetTypeStr(sal_uInt16 nTypeId)
{
    if( !pFldNames )
        _GetFldName();

    if( nTypeId < SwFieldType::pFldNames->size() )
        return (*SwFieldType::pFldNames)[nTypeId];
    return OUString();
}

// each field refences a field type that is unique for each document
SwFieldType::SwFieldType( sal_uInt16 nWhichId )
    : SwModify(0),
    nWhich( nWhichId )
{
}

OUString SwFieldType::GetName() const
{
    return OUString();
}

bool SwFieldType::QueryValue( uno::Any&, sal_uInt16 ) const
{
    return false;
}
bool SwFieldType::PutValue( const uno::Any& , sal_uInt16 )
{
    return false;
}

// Base class for all fields.
// A field (multiple can exist) references a field type (can exists only once)
SwField::SwField(
    SwFieldType* pTyp,
    sal_uInt32 nFmt,
    sal_uInt16 nLng,
    bool bUseFieldValueCache )
    : m_Cache()
    , m_bUseFieldValueCache( bUseFieldValueCache )
    , nLang( nLng )
    , bIsAutomaticLanguage( true )
    , nFormat( nFmt )
    , pType( pTyp )
{
    OSL_ENSURE( pTyp, "SwField: no SwFieldType" );
}

SwField::~SwField()
{
}

// instead of indirectly via the type

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
        else if( static_cast<const SwSetExpField*>(this)->GetInputFlag() )
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

/// get name or content
OUString SwField::GetFieldName() const
{
    sal_uInt16 nTypeId = GetTypeId();
    if (RES_DATETIMEFLD == GetTyp()->Which())
    {
        nTypeId = static_cast<sal_uInt16>(
            ((GetSubType() & DATEFLD) != 0) ? TYP_DATEFLD : TYP_TIMEFLD);
    }
    OUString sRet = SwFieldType::GetTypeStr( nTypeId );
    if (IsFixed())
    {
        sRet += " " + OUString(SwViewShell::GetShellRes()->aFixedStr);
    }
    return sRet;
}

OUString SwField::GetPar1() const
{
    return OUString();
}

OUString SwField::GetPar2() const
{
    return OUString();
}

OUString SwField::GetFormula() const
{
    return GetPar2();
}

void SwField::SetPar1(const OUString& )
{}

void SwField::SetPar2(const OUString& )
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
            rVal <<= !bIsAutomaticLanguage;
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
            bool bFixed = false;
            if(rVal >>= bFixed)
                bIsAutomaticLanguage = !bFixed;
        }
        break;
        default:
            OSL_FAIL("illegal property");
    }
    return true;
}

/** Set a new type
 *
 * This is needed/used for copying between documents.
 * Needs to be always of the same type.
 * @param pNewType The new type.
 * @return The old type.
 */
SwFieldType* SwField::ChgTyp( SwFieldType* pNewType )
{
    OSL_ENSURE( pNewType && pNewType->Which() == pType->Which(),
            "no or different type" );

    SwFieldType* pOld = pType;
    pType = pNewType;
    return pOld;
}

/// Does the field have an action on a ClickHandler? (E.g. INetFields,...)
bool SwField::HasClickHdl() const
{
    bool bRet = false;
    switch( pType->Which() )
    {
    case RES_INTERNETFLD:
    case RES_JUMPEDITFLD:
    case RES_GETREFFLD:
    case RES_MACROFLD:
    case RES_INPUTFLD:
    case RES_DROPDOWN :
        bRet = true;
        break;

    case RES_SETEXPFLD:
        bRet = static_cast<const SwSetExpField*>(this)->GetInputFlag();
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

bool SwField::IsFixed() const
{
    bool bRet = false;
    switch( pType->Which() )
    {
    case RES_FIXDATEFLD:
    case RES_FIXTIMEFLD:
        bRet = true;
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

OUString SwField::ExpandField(bool const bCached, ToxAuthorityField eField) const
{
    if ( m_bUseFieldValueCache )
    {
        if (!bCached) // #i85766# do not expand fields in clipboard documents
        {
            if (GetTypeId() == TYP_AUTHORITY)
            {
                const SwAuthorityField* pAuthorityField = static_cast<const SwAuthorityField*>(this);
                m_Cache = pAuthorityField->ConditionalExpand(eField);
            }
            else
                m_Cache = Expand();
        }
        return m_Cache;
    }

    return Expand();
}

OUString SwField::ExpandCitation(ToxAuthorityField eField) const
{
    const SwAuthorityField* pAuthorityField = static_cast<const SwAuthorityField*>(this);
        return (pAuthorityField ? pAuthorityField->ExpandCitation(eField) : OUString());
}

SwField * SwField::CopyField() const
{
    SwField *const pNew = Copy();
    // #i85766# cache expansion of source (for clipboard)
    // use this->cache, not this->Expand(): only text formatting calls Expand()
    pNew->m_Cache = m_Cache;
    pNew->m_bUseFieldValueCache = m_bUseFieldValueCache;

    return pNew;
}

/// expand numbering
OUString FormatNumber(sal_uInt32 nNum, sal_uInt32 nFormat)
{
    if(SVX_NUM_PAGEDESC == nFormat)
        return  OUString::number( nNum );
    SvxNumberType aNumber;

    OSL_ENSURE(nFormat != SVX_NUM_NUMBER_NONE, "wrong number format" );

    aNumber.SetNumberingType((sal_Int16)nFormat);
    return aNumber.GetNumStr(nNum);
}

SwValueFieldType::SwValueFieldType( SwDoc* pDocPtr, sal_uInt16 nWhichId )
    : SwFieldType(nWhichId),
    pDoc(pDocPtr),
    bUseFormat(true)
{
}

SwValueFieldType::SwValueFieldType( const SwValueFieldType& rTyp )
    : SwFieldType(rTyp.Which()),
    pDoc(rTyp.GetDoc()),
    bUseFormat(rTyp.UseFormat())
{
}

/// return value formatted as string
OUString SwValueFieldType::ExpandValue( const double& rVal,
                                        sal_uInt32 nFmt, sal_uInt16 nLng) const
{
    if (rVal >= DBL_MAX) // error string for calculator
        return SwViewShell::GetShellRes()->aCalc_Error;

    OUString sExpand;
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
    Color* pCol = 0;

    // Bug #60010
    sal_uInt16 nFmtLng = ::lcl_GetLanguageOfFormat( nLng, nFmt, *pFormatter );

    if( nFmt < SV_COUNTRY_LANGUAGE_OFFSET && LANGUAGE_SYSTEM != nFmtLng )
    {
        short nType = NUMBERFORMAT_DEFINED;
        sal_Int32 nDummy;

        const SvNumberformat* pEntry = pFormatter->GetEntry(nFmt);

        if (pEntry && nLng != pEntry->GetLanguage())
        {
            sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFmt,
                                                    (LanguageType)nFmtLng);

            if (nNewFormat == nFmt)
            {
                // probably user-defined format
                OUString sFmt(pEntry->GetFormatstring());

                pFormatter->PutandConvertEntry(sFmt, nDummy, nType, nFmt,
                                        pEntry->GetLanguage(), nFmtLng );
            }
            else
                nFmt = nNewFormat;
        }
        OSL_ENSURE(pEntry, "unknown number format!");
    }

    if( pFormatter->IsTextFormat( nFmt ) )
    {
        pFormatter->GetOutputString(DoubleToString(rVal, nFmtLng), nFmt,
                                    sExpand, &pCol);
    }
    else
    {
        pFormatter->GetOutputString(rVal, nFmt, sExpand, &pCol);
    }
    return sExpand;
}

OUString SwValueFieldType::DoubleToString(const double &rVal,
                                        sal_uInt32 nFmt) const
{
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
    const SvNumberformat* pEntry = pFormatter->GetEntry(nFmt);

    if (!pEntry)
        return OUString();

    return DoubleToString(rVal, pEntry->GetLanguage());
}

OUString SwValueFieldType::DoubleToString( const double &rVal,
                                        sal_uInt16 nLng ) const
{
    SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();

    // Bug #60010
    if( nLng == LANGUAGE_NONE )
        nLng = LANGUAGE_SYSTEM;

    pFormatter->ChangeIntl( nLng ); // get separator in the correct language
    return ::rtl::math::doubleToUString( rVal, rtl_math_StringFormat_F, 12,
                                    pFormatter->GetDecSep(), true );
}

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

/** Set a new type
 *
 * This is needed/used for copying between documents.
 * Needs to be always of the same type.
 * @param pNewType The new type.
 * @return The old type.
 */
SwFieldType* SwValueField::ChgTyp( SwFieldType* pNewType )
{
    SwDoc* pNewDoc = static_cast<SwValueFieldType *>(pNewType)->GetDoc();
    SwDoc* pDoc    = GetDoc();

    if( pNewDoc && pDoc && pDoc != pNewDoc)
    {
        SvNumberFormatter* pFormatter = pNewDoc->GetNumberFormatter();

        if( pFormatter && pFormatter->HasMergeFmtTbl() &&
            static_cast<SwValueFieldType *>(GetTyp())->UseFormat() )
            SetFormat(pFormatter->GetMergeFmtIndex( GetFormat() ));
    }

    return SwField::ChgTyp(pNewType);
}

/// get format in office language
sal_uInt32 SwValueField::GetSystemFormat(SvNumberFormatter* pFormatter, sal_uInt32 nFmt)
{
    const SvNumberformat* pEntry = pFormatter->GetEntry(nFmt);
    sal_uInt16 nLng = SvtSysLocale().GetLanguageTag().getLanguageType();

    if (pEntry && nLng != pEntry->GetLanguage())
    {
        sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFmt,
                                                        (LanguageType)nLng);

        if (nNewFormat == nFmt)
        {
            // probably user-defined format
            short nType = NUMBERFORMAT_DEFINED;
            sal_Int32 nDummy;

            OUString sFmt(pEntry->GetFormatstring());

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

/// set language of the format
void SwValueField::SetLanguage( sal_uInt16 nLng )
{
    if( IsAutomaticLanguage() &&
            static_cast<SwValueFieldType *>(GetTyp())->UseFormat() &&
        GetFormat() != SAL_MAX_UINT32 )
    {
        // Bug #60010
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
                    // probably user-defined format
                    short nType = NUMBERFORMAT_DEFINED;
                    sal_Int32 nDummy;
                    OUString sFmt( pEntry->GetFormatstring() );
                    pFormatter->PutandConvertEntry( sFmt, nDummy, nType,
                                                    nNewFormat,
                                                    pEntry->GetLanguage(),
                                                    nFmtLng );
                }
                SetFormat( nNewFormat );
            }
            OSL_ENSURE(pEntry, "unknown number format!");
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

SwFormulaField::SwFormulaField( SwValueFieldType* pFldType, sal_uInt32 nFmt, const double fVal)
    : SwValueField(pFldType, nFmt, LANGUAGE_SYSTEM, fVal)
{
}

SwFormulaField::SwFormulaField( const SwFormulaField& rFld )
    : SwValueField(static_cast<SwValueFieldType *>(rFld.GetTyp()), rFld.GetFormat(),
                    rFld.GetLanguage(), rFld.GetValue())
{
}

OUString SwFormulaField::GetFormula() const
{
    return sFormula;
}

void SwFormulaField::SetFormula(const OUString& rStr)
{
    sFormula = rStr;

    sal_uLong nFmt(GetFormat());

    if( nFmt && SAL_MAX_UINT32 != nFmt )
    {
        sal_Int32 nPos = 0;
        double fTmpValue;
        if( SwCalc::Str2Double( rStr, nPos, fTmpValue, GetDoc() ) )
            SwValueField::SetValue( fTmpValue );
    }
}

void SwFormulaField::SetExpandedFormula( const OUString& rStr )
{
    sal_uInt32 nFmt(GetFormat());

    if (nFmt && nFmt != SAL_MAX_UINT32 && static_cast<SwValueFieldType *>(GetTyp())->UseFormat())
    {
        double fTmpValue;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        if (pFormatter->IsNumberFormat(rStr, nFmt, fTmpValue))
        {
            SwValueField::SetValue(fTmpValue);

            sFormula = static_cast<SwValueFieldType *>(GetTyp())->DoubleToString(fTmpValue, nFmt);
            return;
        }
    }
    sFormula = rStr;
}

OUString SwFormulaField::GetExpandedFormula() const
{
    sal_uInt32 nFmt(GetFormat());

    if (nFmt && nFmt != SAL_MAX_UINT32 && static_cast<SwValueFieldType *>(GetTyp())->UseFormat())
    {
        OUString sFormattedValue;
        Color* pCol = 0;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        if (pFormatter->IsTextFormat(nFmt))
        {
            OUString sTempIn(static_cast<SwValueFieldType *>(GetTyp())->DoubleToString(GetValue(), nFmt));
            pFormatter->GetOutputString(sTempIn, nFmt, sFormattedValue, &pCol);
        }
        else
        {
            pFormatter->GetOutputString(GetValue(), nFmt, sFormattedValue, &pCol);
        }
        return sFormattedValue;
    }
    else
        return GetFormula();
}

OUString SwField::GetDescription() const
{
    return SW_RES(STR_FIELD);
}

bool SwField::IsClickable() const
{
    switch (Which())
    {
    case RES_JUMPEDITFLD:
    case RES_MACROFLD:
    case RES_GETREFFLD:
    case RES_INPUTFLD:
    case RES_SETEXPFLD:
    case RES_DROPDOWN:
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
