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
#include <o3tl/enumarray.hxx>
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
#include <strings.hrc>
#include <docary.hxx>
#include <authfld.hxx>
#include <calbck.hxx>

using namespace ::com::sun::star;
using namespace nsSwDocInfoSubType;

static LanguageType lcl_GetLanguageOfFormat( LanguageType nLng, sal_uLong nFormat,
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

    const o3tl::enumarray<SwFieldIds,SwFieldTypesEnum> aTypeTab {
    /* SwFieldIds::Database      */      TYP_DBFLD,
    /* SwFieldIds::User          */      TYP_USERFLD,
    /* SwFieldIds::Filename      */      TYP_FILENAMEFLD,
    /* SwFieldIds::DatabaseName  */      TYP_DBNAMEFLD,
    /* SwFieldIds::Date          */      TYP_DATEFLD,
    /* SwFieldIds::Time          */      TYP_TIMEFLD,
    /* SwFieldIds::PageNumber    */      TYP_PAGENUMBERFLD,  // dynamic
    /* SwFieldIds::Author        */      TYP_AUTHORFLD,
    /* SwFieldIds::Chapter       */      TYP_CHAPTERFLD,
    /* SwFieldIds::DocStat       */      TYP_DOCSTATFLD,
    /* SwFieldIds::GetExp        */      TYP_GETFLD,         // dynamic
    /* SwFieldIds::SetExp        */      TYP_SETFLD,         // dynamic
    /* SwFieldIds::GetRef        */      TYP_GETREFFLD,
    /* SwFieldIds::HiddenText    */      TYP_HIDDENTXTFLD,
    /* SwFieldIds::Postit        */      TYP_POSTITFLD,
    /* SwFieldIds::FixDate       */      TYP_FIXDATEFLD,
    /* SwFieldIds::FixTime       */      TYP_FIXTIMEFLD,
    /* SwFieldIds::Reg           */      TYP_BEGIN,         // old (no change since 2000)
    /* SwFieldIds::VarReg        */      TYP_BEGIN,         // old (no change since 2000)
    /* SwFieldIds::SetRef        */      TYP_SETREFFLD,
    /* SwFieldIds::Input         */      TYP_INPUTFLD,
    /* SwFieldIds::Macro         */      TYP_MACROFLD,
    /* SwFieldIds::Dde           */      TYP_DDEFLD,
    /* SwFieldIds::Table         */      TYP_FORMELFLD,
    /* SwFieldIds::HiddenPara    */      TYP_HIDDENPARAFLD,
    /* SwFieldIds::DocInfo       */      TYP_DOCINFOFLD,
    /* SwFieldIds::TemplateName  */      TYP_TEMPLNAMEFLD,
    /* SwFieldIds::DbNextSet     */      TYP_DBNEXTSETFLD,
    /* SwFieldIds::DbNumSet      */      TYP_DBNUMSETFLD,
    /* SwFieldIds::DbSetNumber   */      TYP_DBSETNUMBERFLD,
    /* SwFieldIds::ExtUser       */      TYP_EXTUSERFLD,
    /* SwFieldIds::RefPageSet    */      TYP_SETREFPAGEFLD,
    /* SwFieldIds::RefPageGet    */      TYP_GETREFPAGEFLD,
    /* SwFieldIds::Internet      */      TYP_INTERNETFLD,
    /* SwFieldIds::JumpEdit      */      TYP_JUMPEDITFLD,
    /* SwFieldIds::Script        */      TYP_SCRIPTFLD,
    /* SwFieldIds::DateTime      */      TYP_BEGIN,         // dynamic
    /* SwFieldIds::TableOfAuthorities*/  TYP_AUTHORITY,
    /* SwFieldIds::CombinedChars */      TYP_COMBINED_CHARS,
    /* SwFieldIds::Dropdown      */      TYP_DROPDOWN,
    /* SwFieldIds::ParagraphSignature */ TYP_PARAGRAPHSIGFLD
    };

}

OUString SwFieldType::GetTypeStr(sal_uInt16 nTypeId)
{
    if (!s_pFieldNames)
        GetFieldName_();

    if (nTypeId < SwFieldType::s_pFieldNames->size())
        return (*SwFieldType::s_pFieldNames)[nTypeId];
    return OUString();
}

// each field references a field type that is unique for each document
SwFieldType::SwFieldType( SwFieldIds nWhichId )
    : SwModify(nullptr)
    , m_nWhich(nWhichId)
{
}

OUString SwFieldType::GetName() const
{
    return OUString();
}

void SwFieldType::QueryValue( uno::Any&, sal_uInt16 ) const
{
}
void SwFieldType::PutValue( const uno::Any& , sal_uInt16 )
{
}

void SwFieldTypes::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFieldTypes"));
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
        LanguageType nLang,
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
SwFieldIds SwField::Which() const
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
    case SwFieldIds::DateTime:
        if (GetSubType() & FIXEDFLD)
            nRet = static_cast<sal_uInt16>(GetSubType() & DATEFLD ? TYP_FIXDATEFLD : TYP_FIXTIMEFLD);
        else
            nRet = static_cast<sal_uInt16>(GetSubType() & DATEFLD ? TYP_DATEFLD : TYP_TIMEFLD);
        break;
    case SwFieldIds::GetExp:
        nRet = static_cast<sal_uInt16>(nsSwGetSetExpType::GSE_FORMULA & GetSubType() ? TYP_FORMELFLD : TYP_GETFLD);
        break;

    case SwFieldIds::HiddenText:
        nRet = GetSubType();
        break;

    case SwFieldIds::SetExp:
        if( nsSwGetSetExpType::GSE_SEQ & GetSubType() )
            nRet = TYP_SEQFLD;
        else if( static_cast<const SwSetExpField*>(this)->GetInputFlag() )
            nRet = TYP_SETINPFLD;
        else
            nRet = TYP_SETFLD;
        break;

    case SwFieldIds::PageNumber:
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
    if (SwFieldIds::DateTime == GetTyp()->Which())
    {
        nTypeId = static_cast<sal_uInt16>(
            ((GetSubType() & DATEFLD) != 0) ? TYP_DATEFLD : TYP_TIMEFLD);
    }
    OUString sRet = SwFieldType::GetTypeStr( nTypeId );
    if (IsFixed())
    {
        sRet += " " + SwViewShell::GetShellRes()->aFixedStr;
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
            assert(false);
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
            assert(false);
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
    case SwFieldIds::Internet:
    case SwFieldIds::JumpEdit:
    case SwFieldIds::GetRef:
    case SwFieldIds::Macro:
    case SwFieldIds::Input:
    case SwFieldIds::Dropdown :
        bRet = true;
        break;

    case SwFieldIds::SetExp:
        bRet = static_cast<const SwSetExpField*>(this)->GetInputFlag();
        break;
    default: break;
    }
    return bRet;
}

void SwField::SetLanguage(LanguageType const nLang)
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
    case SwFieldIds::FixDate:
    case SwFieldIds::FixTime:
        bRet = true;
        break;

    case SwFieldIds::DateTime:
        bRet = 0 != (GetSubType() & FIXEDFLD);
        break;

    case SwFieldIds::ExtUser:
    case SwFieldIds::Author:
        bRet = 0 != (GetFormat() & AF_FIXED);
        break;

    case SwFieldIds::Filename:
        bRet = 0 != (GetFormat() & FF_FIXED);
        break;

    case SwFieldIds::DocInfo:
        bRet = 0 != (GetSubType() & DI_SUB_FIXED);
        break;
    default: break;
    }
    return bRet;
}

OUString
SwField::ExpandField(bool const bCached, SwRootFrame const*const ) const
{
    if ( m_bUseFieldValueCache )
    {
        if (!bCached) // #i85766# do not expand fields in clipboard documents
        {
            if (GetTypeId() == TYP_AUTHORITY)
            {
                const SwAuthorityField* pAuthorityField = static_cast<const SwAuthorityField*>(this);
                m_Cache = pAuthorityField->ConditionalExpandAuthIdentifier();
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
OUString FormatNumber(sal_uInt32 nNum, SvxNumType nFormat, LanguageType nLang)
{
    if(SVX_NUM_PAGEDESC == nFormat)
        return  OUString::number( nNum );
    SvxNumberType aNumber;

    OSL_ENSURE(nFormat != SVX_NUM_NUMBER_NONE, "wrong number format" );

    aNumber.SetNumberingType(nFormat);

    if (nLang == LANGUAGE_NONE)
        return aNumber.GetNumStr(nNum);
    else
        return aNumber.GetNumStr(nNum, LanguageTag::convertToLocale(nLang));
}

SwValueFieldType::SwValueFieldType(SwDoc *const pDoc, SwFieldIds const nWhichId)
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
                                        sal_uInt32 nFormat, LanguageType nLng) const
{
    if (rVal >= DBL_MAX) // error string for calculator
        return SwViewShell::GetShellRes()->aCalc_Error;

    OUString sExpand;
    SvNumberFormatter* pFormatter = m_pDoc->GetNumberFormatter();
    Color* pCol = nullptr;

    // Bug #60010
    LanguageType nFormatLng = ::lcl_GetLanguageOfFormat( nLng, nFormat, *pFormatter );

    if( nFormat < SV_COUNTRY_LANGUAGE_OFFSET && LANGUAGE_SYSTEM != nFormatLng )
    {
        SvNumFormatType nType = SvNumFormatType::DEFINED;
        sal_Int32 nDummy;

        const SvNumberformat* pEntry = pFormatter->GetEntry(nFormat);

        if (pEntry && nLng != pEntry->GetLanguage())
        {
            sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFormat,
                                                    nFormatLng);

            if (nNewFormat == nFormat)
            {
                // probably user-defined format
                OUString sFormat(pEntry->GetFormatstring());

                pFormatter->PutandConvertEntry(sFormat, nDummy, nType, nFormat,
                                        pEntry->GetLanguage(), nFormatLng, false);
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
                                        LanguageType nLng ) const
{
    SvNumberFormatter* pFormatter = m_pDoc->GetNumberFormatter();

    // Bug #60010
    if( nLng == LANGUAGE_NONE )
        nLng = LANGUAGE_SYSTEM;

    pFormatter->ChangeIntl( nLng ); // get separator in the correct language
    return ::rtl::math::doubleToUString( rVal, rtl_math_StringFormat_F, 12,
                                    pFormatter->GetNumDecimalSep()[0], true );
}

SwValueField::SwValueField( SwValueFieldType* pFieldType, sal_uInt32 nFormat,
                            LanguageType nLng, const double fVal )
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
    LanguageType nLng = SvtSysLocale().GetLanguageTag().getLanguageType();

    if (pEntry && nLng != pEntry->GetLanguage())
    {
        sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFormat,
                                                        nLng);

        if (nNewFormat == nFormat)
        {
            // probably user-defined format
            SvNumFormatType nType = SvNumFormatType::DEFINED;
            sal_Int32 nDummy;

            OUString sFormat(pEntry->GetFormatstring());

            sal_uInt32 nTempFormat = nFormat;
            pFormatter->PutandConvertEntry(sFormat, nDummy, nType,
                                           nTempFormat, pEntry->GetLanguage(), nLng, true);
            nFormat = nTempFormat;
        }
        else
            nFormat = nNewFormat;
    }

    return nFormat;
}

/// set language of the format
void SwValueField::SetLanguage( LanguageType nLng )
{
    if( IsAutomaticLanguage() &&
            static_cast<SwValueFieldType *>(GetTyp())->UseFormat() &&
        GetFormat() != SAL_MAX_UINT32 )
    {
        // Bug #60010
        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
        LanguageType nFormatLng = ::lcl_GetLanguageOfFormat( nLng, GetFormat(),
                                                    *pFormatter );

        if( (GetFormat() >= SV_COUNTRY_LANGUAGE_OFFSET ||
             LANGUAGE_SYSTEM != nFormatLng ) &&
            !(Which() == SwFieldIds::User && (GetSubType()&nsSwExtendedSubType::SUB_CMD) ) )
        {
            const SvNumberformat* pEntry = pFormatter->GetEntry(GetFormat());

            if( pEntry && nFormatLng != pEntry->GetLanguage() )
            {
                sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(
                                        GetFormat(), nFormatLng );

                if( nNewFormat == GetFormat() )
                {
                    // probably user-defined format
                    SvNumFormatType nType = SvNumFormatType::DEFINED;
                    sal_Int32 nDummy;
                    OUString sFormat( pEntry->GetFormatstring() );
                    pFormatter->PutandConvertEntry( sFormat, nDummy, nType,
                                                    nNewFormat,
                                                    pEntry->GetLanguage(),
                                                    nFormatLng, false);
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

        if (GetDoc()->IsNumberFormat(rStr, nFormat, fTmpValue))
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
    return SwResId(STR_FIELD);
}

bool SwField::IsClickable() const
{
    switch (Which())
    {
    case SwFieldIds::JumpEdit:
    case SwFieldIds::Macro:
    case SwFieldIds::GetRef:
    case SwFieldIds::Input:
    case SwFieldIds::SetExp:
    case SwFieldIds::Dropdown:
        return true;
    default: break;
    }
    return false;
}

void SwField::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwField"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*this).name()));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
