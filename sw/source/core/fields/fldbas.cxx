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

#include <fldbas.hxx>

#include <float.h>
#include <math.h>

#include <libxml/xmlwriter.h>

#include <rtl/math.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <editeng/unolingu.hxx>
#include <unofldmid.h>
#include <doc.hxx>
#include <editsh.hxx>
#include <frame.hxx>
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
#include <calbck.hxx>

using namespace ::com::sun::star;
using namespace nsSwDocInfoSubType;

static sal_uInt16 lcl_GetLanguageOfFormat( sal_uInt16 nLng, sal_uLong nFormat,
                                const SvNumberFormatter& rFormatter )
{
    if( nLng == LANGUAGE_NONE ) // Bug #60010
        nLng = LANGUAGE_SYSTEM;
    else if( nLng == ::GetAppLanguage() )
        switch( rFormatter.GetIndexTableOffset( nFormat ))
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
std::vector<OUString>* SwFieldType::s_pFieldNames = nullptr;

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
    if (!s_pFieldNames)
        _GetFieldName();

    if (nTypeId < SwFieldType::s_pFieldNames->size())
        return (*SwFieldType::s_pFieldNames)[nTypeId];
    return OUString();
}

// each field refences a field type that is unique for each document
SwFieldType::SwFieldType( sal_uInt16 nWhichId )
    : SwModify(nullptr)
    , m_nWhich(nWhichId)
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

void SwFieldTypes::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFieldTypes"));
    sal_uInt16 nCount = size();
    for (sal_uInt16 nType = 0; nType < nCount; ++nType)
    {
        const SwFieldType *pCurType = (*this)[nType];
        SwIterator<SwFormatField, SwFieldType> aIter(*pCurType);
        for (const SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next())
            pFormatField->dumpAsXml(pWriter);
    }
    xmlTextWriterEndElement(pWriter);
}

// Base class for all fields.
// A field (multiple can exist) references a field type (can exists only once)
SwField::SwField(
        SwFieldType* pType,
        sal_uInt32 nFormat,
        sal_uInt16 nLang,
        bool bUseFieldValueCache)
    : m_Cache()
    , m_bUseFieldValueCache( bUseFieldValueCache )
    , m_nLang( nLang )
    , m_bIsAutomaticLanguage( true )
    , m_nFormat( nFormat )
    , m_pType( pType )
{
    assert(m_pType);
}

SwField::~SwField()
{
}

// instead of indirectly via the type

#ifdef DBG_UTIL
sal_uInt16 SwField::Which() const
{
    assert(m_pType);
    return m_pType->Which();
}
#endif

sal_uInt16 SwField::GetTypeId() const
{

    sal_uInt16 nRet;
    switch (m_pType->Which())
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
        nRet = aTypeTab[ m_pType->Which() ];
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
            rVal <<= !m_bIsAutomaticLanguage;
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
                m_bIsAutomaticLanguage = !bFixed;
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
    assert(pNewType && pNewType->Which() == m_pType->Which());

    SwFieldType* pOld = m_pType;
    m_pType = pNewType;
    return pOld;
}

/// Does the field have an action on a ClickHandler? (E.g. INetFields,...)
bool SwField::HasClickHdl() const
{
    bool bRet = false;
    switch (m_pType->Which())
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

void SwField::SetLanguage(sal_uInt16 const nLang)
{
    m_nLang = nLang;
}

void SwField::ChangeFormat(sal_uInt32 const nFormat)
{
    m_nFormat = nFormat;
}

bool SwField::IsFixed() const
{
    bool bRet = false;
    switch (m_pType->Which())
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

SwValueFieldType::SwValueFieldType(SwDoc *const pDoc, sal_uInt16 const nWhichId)
    : SwFieldType(nWhichId)
    , m_pDoc(pDoc)
    , m_bUseFormat(true)
{
}

SwValueFieldType::SwValueFieldType( const SwValueFieldType& rTyp )
    : SwFieldType(rTyp.Which())
    , m_pDoc(rTyp.GetDoc())
    , m_bUseFormat(rTyp.UseFormat())
{
}

/// return value formatted as string
OUString SwValueFieldType::ExpandValue( const double& rVal,
                                        sal_uInt32 nFormat, sal_uInt16 nLng) const
{
    if (rVal >= DBL_MAX) // error string for calculator
        return SwViewShell::GetShellRes()->aCalc_Error;

    OUString sExpand;
    SvNumberFormatter* pFormatter = m_pDoc->GetNumberFormatter();
    Color* pCol = nullptr;

    // Bug #60010
    sal_uInt16 nFormatLng = ::lcl_GetLanguageOfFormat( nLng, nFormat, *pFormatter );

    if( nFormat < SV_COUNTRY_LANGUAGE_OFFSET && LANGUAGE_SYSTEM != nFormatLng )
    {
        short nType = css::util::NumberFormat::DEFINED;
        sal_Int32 nDummy;

        const SvNumberformat* pEntry = pFormatter->GetEntry(nFormat);

        if (pEntry && nLng != pEntry->GetLanguage())
        {
            sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFormat,
                                                    (LanguageType)nFormatLng);

            if (nNewFormat == nFormat)
            {
                // probably user-defined format
                OUString sFormat(pEntry->GetFormatstring());

                pFormatter->PutandConvertEntry(sFormat, nDummy, nType, nFormat,
                                        pEntry->GetLanguage(), nFormatLng );
            }
            else
                nFormat = nNewFormat;
        }
        OSL_ENSURE(pEntry, "unknown number format!");
    }

    if( pFormatter->IsTextFormat( nFormat ) )
    {
        pFormatter->GetOutputString(DoubleToString(rVal, nFormatLng), nFormat,
                                    sExpand, &pCol);
    }
    else
    {
        pFormatter->GetOutputString(rVal, nFormat, sExpand, &pCol);
    }
    return sExpand;
}

OUString SwValueFieldType::DoubleToString(const double &rVal,
                                        sal_uInt32 nFormat) const
{
    SvNumberFormatter* pFormatter = m_pDoc->GetNumberFormatter();
    const SvNumberformat* pEntry = pFormatter->GetEntry(nFormat);

    if (!pEntry)
        return OUString();

    return DoubleToString(rVal, pEntry->GetLanguage());
}

OUString SwValueFieldType::DoubleToString( const double &rVal,
                                        sal_uInt16 nLng ) const
{
    SvNumberFormatter* pFormatter = m_pDoc->GetNumberFormatter();

    // Bug #60010
    if( nLng == LANGUAGE_NONE )
        nLng = LANGUAGE_SYSTEM;

    pFormatter->ChangeIntl( nLng ); // get separator in the correct language
    return ::rtl::math::doubleToUString( rVal, rtl_math_StringFormat_F, 12,
                                    pFormatter->GetDecSep(), true );
}

SwValueField::SwValueField( SwValueFieldType* pFieldType, sal_uInt32 nFormat,
                            sal_uInt16 nLng, const double fVal )
    : SwField(pFieldType, nFormat, nLng)
    , m_fValue(fVal)
{
}

SwValueField::SwValueField( const SwValueField& rField )
    : SwField(rField)
    , m_fValue(rField.GetValue())
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

        if( pFormatter && pFormatter->HasMergeFormatTable() &&
            static_cast<SwValueFieldType *>(GetTyp())->UseFormat() )
            SetFormat(pFormatter->GetMergeFormatIndex( GetFormat() ));
    }

    return SwField::ChgTyp(pNewType);
}

/// get format in office language
sal_uInt32 SwValueField::GetSystemFormat(SvNumberFormatter* pFormatter, sal_uInt32 nFormat)
{
    const SvNumberformat* pEntry = pFormatter->GetEntry(nFormat);
    sal_uInt16 nLng = SvtSysLocale().GetLanguageTag().getLanguageType();

    if (pEntry && nLng != pEntry->GetLanguage())
    {
        sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFormat,
                                                        (LanguageType)nLng);

        if (nNewFormat == nFormat)
        {
            // probably user-defined format
            short nType = css::util::NumberFormat::DEFINED;
            sal_Int32 nDummy;

            OUString sFormat(pEntry->GetFormatstring());

            sal_uInt32 nTempFormat = nFormat;
            pFormatter->PutandConvertEntry(sFormat, nDummy, nType,
                                           nTempFormat, pEntry->GetLanguage(), nLng);
            nFormat = nTempFormat;
        }
        else
            nFormat = nNewFormat;
    }

    return nFormat;
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
        sal_uInt16 nFormatLng = ::lcl_GetLanguageOfFormat( nLng, GetFormat(),
                                                    *pFormatter );

        if( (GetFormat() >= SV_COUNTRY_LANGUAGE_OFFSET ||
             LANGUAGE_SYSTEM != nFormatLng ) &&
            !(Which() == RES_USERFLD && (GetSubType()&nsSwExtendedSubType::SUB_CMD) ) )
        {
            const SvNumberformat* pEntry = pFormatter->GetEntry(GetFormat());

            if( pEntry && nFormatLng != pEntry->GetLanguage() )
            {
                sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(
                                        GetFormat(), (LanguageType)nFormatLng );

                if( nNewFormat == GetFormat() )
                {
                    // probably user-defined format
                    short nType = css::util::NumberFormat::DEFINED;
                    sal_Int32 nDummy;
                    OUString sFormat( pEntry->GetFormatstring() );
                    pFormatter->PutandConvertEntry( sFormat, nDummy, nType,
                                                    nNewFormat,
                                                    pEntry->GetLanguage(),
                                                    nFormatLng );
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
    return m_fValue;
}

void SwValueField::SetValue( const double& rVal )
{
    m_fValue = rVal;
}

SwFormulaField::SwFormulaField( SwValueFieldType* pFieldType, sal_uInt32 nFormat, const double fVal)
    : SwValueField(pFieldType, nFormat, LANGUAGE_SYSTEM, fVal)
{
}

SwFormulaField::SwFormulaField( const SwFormulaField& rField )
    : SwValueField(static_cast<SwValueFieldType *>(rField.GetTyp()), rField.GetFormat(),
                    rField.GetLanguage(), rField.GetValue())
{
}

OUString SwFormulaField::GetFormula() const
{
    return m_sFormula;
}

void SwFormulaField::SetFormula(const OUString& rStr)
{
    m_sFormula = rStr;

    sal_uLong nFormat(GetFormat());

    if( nFormat && SAL_MAX_UINT32 != nFormat )
    {
        sal_Int32 nPos = 0;
        double fTmpValue;
        if( SwCalc::Str2Double( rStr, nPos, fTmpValue, GetDoc() ) )
            SwValueField::SetValue( fTmpValue );
    }
}

void SwFormulaField::SetExpandedFormula( const OUString& rStr )
{
    sal_uInt32 nFormat(GetFormat());

    if (nFormat && nFormat != SAL_MAX_UINT32 && static_cast<SwValueFieldType *>(GetTyp())->UseFormat())
    {
        double fTmpValue;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        if (pFormatter->IsNumberFormat(rStr, nFormat, fTmpValue))
        {
            SwValueField::SetValue(fTmpValue);

            m_sFormula = static_cast<SwValueFieldType *>(GetTyp())->DoubleToString(fTmpValue, nFormat);
            return;
        }
    }
    m_sFormula = rStr;
}

OUString SwFormulaField::GetExpandedFormula() const
{
    sal_uInt32 nFormat(GetFormat());

    if (nFormat && nFormat != SAL_MAX_UINT32 && static_cast<SwValueFieldType *>(GetTyp())->UseFormat())
    {
        OUString sFormattedValue;
        Color* pCol = nullptr;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        if (pFormatter->IsTextFormat(nFormat))
        {
            OUString sTempIn(static_cast<SwValueFieldType *>(GetTyp())->DoubleToString(GetValue(), nFormat));
            pFormatter->GetOutputString(sTempIn, nFormat, sFormattedValue, &pCol);
        }
        else
        {
            pFormatter->GetOutputString(GetValue(), nFormat, sFormattedValue, &pCol);
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

void SwField::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swField"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*this).name()));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
