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

#include <libxml/xmlwriter.h>

#include <rtl/math.hxx>
#include <comphelper/string.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <o3tl/enumarray.hxx>
#include <osl/diagnose.h>
#include <unofldmid.h>
#include <doc.hxx>
#include <fmtfld.hxx>
#include <docufld.hxx>
#include <expfld.hxx>
#include <shellres.hxx>
#include <calc.hxx>
#include <strings.hrc>
#include <docary.hxx>
#include <authfld.hxx>
#include <calbck.hxx>
#include <viewsh.hxx>
#include <hints.hxx>
#include <unofield.hxx>
#include <dbfld.hxx>
#include <chpfld.hxx>
#include <flddat.hxx>
#include <usrfld.hxx>

using namespace ::com::sun::star;

static LanguageType lcl_GetLanguageOfFormat(LanguageType nLng, sal_uLong nFormat)
{
    if( nLng == LANGUAGE_NONE ) // Bug #60010
        nLng = LANGUAGE_SYSTEM;
    else if( nLng == ::GetAppLanguage() )
        switch( SvNumberFormatter::GetIndexTableOffset( nFormat ))
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
    /* SwFieldIds::Database      */      SwFieldTypesEnum::Database,
    /* SwFieldIds::User          */      SwFieldTypesEnum::User,
    /* SwFieldIds::Filename      */      SwFieldTypesEnum::Filename,
    /* SwFieldIds::DatabaseName  */      SwFieldTypesEnum::DatabaseName,
    /* SwFieldIds::Date          */      SwFieldTypesEnum::Date,
    /* SwFieldIds::Time          */      SwFieldTypesEnum::Time,
    /* SwFieldIds::PageNumber    */      SwFieldTypesEnum::PageNumber,  // dynamic
    /* SwFieldIds::Author        */      SwFieldTypesEnum::Author,
    /* SwFieldIds::Chapter       */      SwFieldTypesEnum::Chapter,
    /* SwFieldIds::DocStat       */      SwFieldTypesEnum::DocumentStatistics,
    /* SwFieldIds::GetExp        */      SwFieldTypesEnum::Get,         // dynamic
    /* SwFieldIds::SetExp        */      SwFieldTypesEnum::Set,         // dynamic
    /* SwFieldIds::GetRef        */      SwFieldTypesEnum::GetRef,
    /* SwFieldIds::HiddenText    */      SwFieldTypesEnum::HiddenText,
    /* SwFieldIds::Postit        */      SwFieldTypesEnum::Postit,
    /* SwFieldIds::FixDate       */      SwFieldTypesEnum::FixedDate,
    /* SwFieldIds::FixTime       */      SwFieldTypesEnum::FixedTime,
    /* SwFieldIds::Reg           */      SwFieldTypesEnum::Begin,         // old (no change since 2000)
    /* SwFieldIds::VarReg        */      SwFieldTypesEnum::Begin,         // old (no change since 2000)
    /* SwFieldIds::SetRef        */      SwFieldTypesEnum::SetRef,
    /* SwFieldIds::Input         */      SwFieldTypesEnum::Input,
    /* SwFieldIds::Macro         */      SwFieldTypesEnum::Macro,
    /* SwFieldIds::Dde           */      SwFieldTypesEnum::DDE,
    /* SwFieldIds::Table         */      SwFieldTypesEnum::Formel,
    /* SwFieldIds::HiddenPara    */      SwFieldTypesEnum::HiddenParagraph,
    /* SwFieldIds::DocInfo       */      SwFieldTypesEnum::DocumentInfo,
    /* SwFieldIds::TemplateName  */      SwFieldTypesEnum::TemplateName,
    /* SwFieldIds::DbNextSet     */      SwFieldTypesEnum::DatabaseNextSet,
    /* SwFieldIds::DbNumSet      */      SwFieldTypesEnum::DatabaseNumberSet,
    /* SwFieldIds::DbSetNumber   */      SwFieldTypesEnum::DatabaseSetNumber,
    /* SwFieldIds::ExtUser       */      SwFieldTypesEnum::ExtendedUser,
    /* SwFieldIds::RefPageSet    */      SwFieldTypesEnum::SetRefPage,
    /* SwFieldIds::RefPageGet    */      SwFieldTypesEnum::GetRefPage,
    /* SwFieldIds::Internet      */      SwFieldTypesEnum::Internet,
    /* SwFieldIds::JumpEdit      */      SwFieldTypesEnum::JumpEdit,
    /* SwFieldIds::Script        */      SwFieldTypesEnum::Script,
    /* SwFieldIds::DateTime      */      SwFieldTypesEnum::Begin,         // dynamic
    /* SwFieldIds::TableOfAuthorities*/  SwFieldTypesEnum::Authority,
    /* SwFieldIds::CombinedChars */      SwFieldTypesEnum::CombinedChars,
    /* SwFieldIds::Dropdown      */      SwFieldTypesEnum::Dropdown,
    /* SwFieldIds::ParagraphSignature */ SwFieldTypesEnum::ParagraphSignature
    };

}

const OUString & SwFieldType::GetTypeStr(SwFieldTypesEnum nTypeId)
{
    if (!s_pFieldNames)
        GetFieldName_();

    return (*SwFieldType::s_pFieldNames)[static_cast<int>(nTypeId)];
}

// each field references a field type that is unique for each document
SwFieldType::SwFieldType( SwFieldIds nWhichId )
    : sw::BroadcastingModify()
    , m_nWhich(nWhichId)
{
}

UIName SwFieldType::GetName() const
{
    return UIName();
}

void SwFieldType::QueryValue( uno::Any&, sal_uInt16 ) const
{
}
void SwFieldType::PutValue( const uno::Any& , sal_uInt16 )
{
}

void SwFieldType::PrintHiddenPara()
{
    SwClientNotify(*this, sw::PrintHiddenParaHint());
}

void SwFieldType::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    std::vector<SwFormatField*> vFields;
    GatherFields(vFields);
    if(!vFields.size())
        return;
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFieldType"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*this).name()));
    for(const auto pFormatField: vFields)
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatField"));
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", pFormatField);
        (void)xmlTextWriterEndElement(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);
}

SwFormatField* SwFieldType::FindFormatForField(const SwField* pField) const {
    SwFormatField* pFormat = nullptr;
    CallSwClientNotify(sw::FindFormatForFieldHint(pField, pFormat));
    return pFormat;
}

SwFormatField* SwFieldType::FindFormatForPostItId(sal_uInt32 nPostItId) const {
    SwFormatField* pFormat = nullptr;
    CallSwClientNotify(sw::FindFormatForPostItIdHint(nPostItId, pFormat));
    return pFormat;
}

void SwFieldType::CollectPostIts(std::vector<SwFormatField*>& rvFormatFields, IDocumentRedlineAccess const& rIDRA, const bool bHideRedlines)
{
    CallSwClientNotify(sw::CollectPostItsHint(rvFormatFields, rIDRA, bHideRedlines));
}

bool SwFieldType::HasHiddenInformationNotes() const
{
    bool bHasHiddenInformationNotes = false;
    CallSwClientNotify(sw::HasHiddenInformationNotesHint(bHasHiddenInformationNotes));
    return bHasHiddenInformationNotes;
}

void SwFieldType::GatherNodeIndex(std::vector<SwNodeOffset>& rvNodeIndex)
{
    CallSwClientNotify(sw::GatherNodeIndexHint(rvNodeIndex));
}

void SwFieldType::GatherRefFields(std::vector<SwGetRefField*>& rvRFields, const ReferencesSubtype nTyp)
{
    CallSwClientNotify(sw::GatherRefFieldsHint(rvRFields, nTyp));
}

void SwFieldType::GatherFields(std::vector<SwFormatField*>& rvFields, bool bCollectOnlyInDocNodes) const
{
    CallSwClientNotify(sw::GatherFieldsHint(rvFields, bCollectOnlyInDocNodes));
}

void SwFieldType::GatherDdeTables(std::vector<SwDDETable*>& rvTables) const
{
    CallSwClientNotify(sw::GatherDdeTablesHint(rvTables));
}

void SwFieldType::UpdateDocPos(const SwTwips nDocPos)
{
    CallSwClientNotify(sw::DocPosUpdate(nDocPos));
}
void SwFieldType::UpdateFields()
{
    CallSwClientNotify(sw::LegacyModifyHint(nullptr, nullptr));
};

void SwFieldType::SetXObject(rtl::Reference<SwXFieldMaster> const& xFieldMaster)
{
    m_wXFieldMaster = xFieldMaster.get();
}

void SwFieldTypes::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFieldTypes"));
    sal_uInt16 nCount = size();
    for (sal_uInt16 nType = 0; nType < nCount; ++nType)
        (*this)[nType]->dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}



// Base class for all fields.
// A field (multiple can exist) references a field type (can exists only once)
SwField::SwField(
        SwFieldType* pType,
        LanguageType nLang,
        bool bUseFieldValueCache)
    : m_pType( pType )
    , m_nLang( nLang )
    , m_bUseFieldValueCache( bUseFieldValueCache )
    , m_bIsAutomaticLanguage( true )
{
    assert(m_pType);
}

SwField::~SwField()
{
}

SwFieldTypesEnum SwField::GetTypeId() const
{

    SwFieldTypesEnum nRet;
    switch (m_pType->Which())
    {
    case SwFieldIds::DateTime:
        {
            auto pDateTimeField = static_cast<const SwDateTimeField*>(this);
            SwDateTimeSubType nSubType = pDateTimeField->GetSubType();
            if (nSubType & SwDateTimeSubType::Fixed)
                nRet = nSubType & SwDateTimeSubType::Date ? SwFieldTypesEnum::FixedDate : SwFieldTypesEnum::FixedTime;
            else
                nRet = nSubType & SwDateTimeSubType::Date ? SwFieldTypesEnum::Date : SwFieldTypesEnum::Time;
        }
        break;
    case SwFieldIds::GetExp:
        {
            auto pGetExpField = static_cast<const SwGetExpField*>(this);
            nRet = SwGetSetExpType::Formula & pGetExpField->GetSubType() ? SwFieldTypesEnum::Formel : SwFieldTypesEnum::Get;
        }
        break;

    case SwFieldIds::HiddenText:
        {
            auto pHiddenTextField = static_cast<const SwHiddenTextField*>(this);
            nRet = pHiddenTextField->GetSubType();
        }
        break;

    case SwFieldIds::SetExp:
        {
            auto pSetExpField = static_cast<const SwSetExpField*>(this);
            if( SwGetSetExpType::Sequence & pSetExpField->GetSubType() )
                nRet = SwFieldTypesEnum::Sequence;
            else if( pSetExpField->GetInputFlag() )
                nRet = SwFieldTypesEnum::SetInput;
            else
                nRet = SwFieldTypesEnum::Set;
        }
        break;

    case SwFieldIds::PageNumber:
        {
            auto pPageNumberField = static_cast<const SwPageNumberField*>(this);
            SwPageNumSubType nSubType = pPageNumberField->GetSubType();
            if( SwPageNumSubType::Next == nSubType )
                nRet = SwFieldTypesEnum::NextPage;
            else if( SwPageNumSubType::Previous == nSubType )
                nRet = SwFieldTypesEnum::PreviousPage;
            else
                nRet = SwFieldTypesEnum::PageNumber;
        }
        break;

    default:
        nRet = aTypeTab[ m_pType->Which() ];
    }
    return nRet;
}

/// get name or content
OUString SwField::GetFieldName() const
{
    SwFieldTypesEnum nTypeId = GetTypeId();
    if (SwFieldIds::DateTime == GetTyp()->Which())
    {
        auto pDateTimeField = static_cast<const SwDateTimeField*>(this);
        nTypeId =
            (pDateTimeField->GetSubType() & SwDateTimeSubType::Date) ? SwFieldTypesEnum::Date : SwFieldTypesEnum::Time;
    }
    OUString sRet = SwFieldType::GetTypeStr( nTypeId );
    if (IsFixed())
    {
        sRet += " " + SwViewShell::GetShellRes()->aFixedStr;
    }
    return sRet;
}

/// Helpers for those places still passing untyped format ids around for SwField
sal_uInt32 SwField::GetUntypedFormat() const
{
    switch (m_pType->Which())
    {
    case SwFieldIds::PageNumber:
        return static_cast<const SwPageNumberField*>(this)->GetFormat();
    case SwFieldIds::JumpEdit:
        return static_cast<sal_uInt32>(static_cast<const SwJumpEditField*>(this)->GetFormat());
    case SwFieldIds::DocStat:
        return static_cast<const SwDocStatField*>(this)->GetFormat();
    case SwFieldIds::TemplateName:
        return static_cast<sal_uInt32>(static_cast<const SwTemplNameField*>(this)->GetFormat());
    case SwFieldIds::Chapter:
        return static_cast<sal_uInt32>(static_cast<const SwChapterField*>(this)->GetFormat());
    case SwFieldIds::Filename:
        return static_cast<sal_uInt32>(static_cast<const SwFileNameField*>(this)->GetFormat());
    case SwFieldIds::Author:
        return static_cast<sal_uInt32>(static_cast<const SwAuthorField*>(this)->GetFormat());
    case SwFieldIds::ExtUser:
        return static_cast<sal_uInt32>(static_cast<const SwExtUserField*>(this)->GetFormat());
    case SwFieldIds::DbNextSet:
    case SwFieldIds::DbNumSet:
    case SwFieldIds::DatabaseName:
    case SwFieldIds::DbSetNumber:
        return static_cast<const SwDBNameInfField*>(this)->GetFormat();
    case SwFieldIds::RefPageGet:
        return static_cast<const SwRefPageGetField*>(this)->GetFormat();
    case SwFieldIds::GetRef:
        return static_cast<sal_uInt16>(static_cast<const SwGetRefField*>(this)->GetFormat());
    default: break;
    }
    if (auto p = dynamic_cast<const SwValueField*>(this))
        return p->GetFormat();
    return 0;
}

/// Helpers for those places still passing untyped format ids around for SwField
void SwField::SetUntypedFormat(sal_uInt32 n)
{
    switch (m_pType->Which())
    {
    case SwFieldIds::PageNumber:
        static_cast<SwPageNumberField*>(this)->SetFormat(static_cast<SvxNumType>(n));
        return;
    case SwFieldIds::JumpEdit:
        static_cast<SwJumpEditField*>(this)->SetFormat(static_cast<SwJumpEditFormat>(n));
        return;
    case SwFieldIds::DocStat:
        static_cast<SwDocStatField*>(this)->SetFormat(static_cast<SvxNumType>(n));
        return;
    case SwFieldIds::TemplateName:
        static_cast<SwTemplNameField*>(this)->SetFormat(static_cast<SwFileNameFormat>(n));
        return;
    case SwFieldIds::Chapter:
        static_cast<SwChapterField*>(this)->SetFormat(static_cast<SwChapterFormat>(n));
        return;
    case SwFieldIds::Filename:
        static_cast<SwFileNameField*>(this)->SetFormat(static_cast<SwFileNameFormat>(n));
        return;
    case SwFieldIds::Author:
        static_cast<SwAuthorField*>(this)->SetFormat(static_cast<SwAuthorFormat>(n));
        return;
    case SwFieldIds::ExtUser:
        static_cast<SwExtUserField*>(this)->SetFormat(static_cast<SwAuthorFormat>(n));
        return;
    case SwFieldIds::DbNextSet:
    case SwFieldIds::DbNumSet:
    case SwFieldIds::DatabaseName:
    case SwFieldIds::DbSetNumber:
        static_cast<SwDBNameInfField*>(this)->SetFormat(n);
        return;
    case SwFieldIds::RefPageGet:
        static_cast<SwRefPageGetField*>(this)->SetFormat(static_cast<SvxNumType>(n));
        return;
    case SwFieldIds::GetRef:
        static_cast<SwGetRefField*>(this)->SetFormat(static_cast<RefFieldFormat>(n));
        return;
    default: break;
    }
    if (auto p2 = dynamic_cast<SwValueField*>(this))
        p2->SetFormat(n);
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

// for code that is still passing around untyped values
sal_uInt16 SwField::GetUntypedSubType() const
{
    switch (m_pType->Which())
    {
    case SwFieldIds::User:
        return static_cast<sal_uInt16>(static_cast<const SwUserField*>(this)->GetSubType());
    case SwFieldIds::GetRef:
        return static_cast<sal_uInt16>(static_cast<const SwGetRefField*>(this)->GetSubType());
    case SwFieldIds::DateTime:
        return static_cast<sal_uInt16>(static_cast<const SwDateTimeField*>(this)->GetSubType());
    case SwFieldIds::Table:
        return static_cast<sal_uInt16>(static_cast<const SwTableField*>(this)->GetSubType());
    case SwFieldIds::Input:
        return static_cast<sal_uInt16>(static_cast<const SwInputField*>(this)->GetSubType());
    case SwFieldIds::GetExp:
        return static_cast<sal_uInt16>(static_cast<const SwGetExpField*>(this)->GetSubType());
    case SwFieldIds::SetExp:
        return static_cast<sal_uInt16>(static_cast<const SwSetExpField*>(this)->GetSubType());
    case SwFieldIds::ExtUser:
        return static_cast<sal_uInt32>(static_cast<const SwExtUserField*>(this)->GetSubType());
    case SwFieldIds::DocInfo:
        return static_cast<sal_uInt16>(static_cast<const SwDocInfoField*>(this)->GetSubType());
    case SwFieldIds::HiddenText:
        return static_cast<sal_uInt16>(static_cast<const SwHiddenTextField*>(this)->GetSubType());
    case SwFieldIds::DocStat:
        return static_cast<sal_uInt16>(static_cast<const SwDocStatField*>(this)->GetSubType());
    case SwFieldIds::PageNumber:
        return static_cast<sal_uInt16>(static_cast<const SwPageNumberField*>(this)->GetSubType());
    case SwFieldIds::DbNextSet:
    case SwFieldIds::DbNumSet:
    case SwFieldIds::DatabaseName:
    case SwFieldIds::DbSetNumber:
        return static_cast<sal_uInt16>(static_cast<const SwDBNameInfField*>(this)->GetSubType());
    case SwFieldIds::Database:
        return static_cast<sal_uInt16>(static_cast<const SwDBField*>(this)->GetSubType());
    default: break;
    }
    return 0;
}

void SwField::SetUntypedSubType(sal_uInt16 n)
{
    switch (m_pType->Which())
    {
    case SwFieldIds::User:
        static_cast<SwUserField*>(this)->SetSubType(static_cast<SwUserType>(n));
        break;
    case SwFieldIds::GetRef:
        static_cast<SwGetRefField*>(this)->SetSubType(static_cast<ReferencesSubtype>(n));
        break;
    case SwFieldIds::DateTime:
        static_cast<SwDateTimeField*>(this)->SetSubType(static_cast<SwDateTimeSubType>(n));
        break;
    case SwFieldIds::Table:
        static_cast<SwTableField*>(this)->SetSubType(static_cast<SwTableFieldSubType>(n));
        break;
    case SwFieldIds::Input:
        static_cast<SwInputField*>(this)->SetSubType(static_cast<SwInputFieldSubType>(n));
        break;
    case SwFieldIds::GetExp:
        static_cast<SwGetExpField*>(this)->SetSubType(static_cast<SwGetSetExpType>(n));
        break;
    case SwFieldIds::SetExp:
        static_cast<SwSetExpField*>(this)->SetSubType(static_cast<SwGetSetExpType>(n));
        break;
    case SwFieldIds::ExtUser:
        static_cast<SwExtUserField*>(this)->SetSubType(static_cast<SwExtUserSubType>(n));
        break;
    case SwFieldIds::DocInfo:
        static_cast<SwDocInfoField*>(this)->SetSubType(static_cast<SwDocInfoSubType>(n));
        break;
    case SwFieldIds::HiddenText:
        static_cast<SwHiddenTextField*>(this)->SetSubType(static_cast<SwFieldTypesEnum>(n));
        break;
    case SwFieldIds::DocStat:
        static_cast<SwDocStatField*>(this)->SetSubType(static_cast<SwDocStatSubType>(n));
        break;
    case SwFieldIds::PageNumber:
        static_cast<SwPageNumberField*>(this)->SetSubType(static_cast<SwPageNumSubType>(n));
        break;
    case SwFieldIds::DbNextSet:
    case SwFieldIds::DbNumSet:
    case SwFieldIds::DatabaseName:
    case SwFieldIds::DbSetNumber:
        static_cast<SwDBNameInfField*>(this)->SetSubType(static_cast<SwDBFieldSubType>(n));
        break;
    case SwFieldIds::Database:
        static_cast<SwDBField*>(this)->SetSubType(static_cast<SwDBFieldSubType>(n));
        break;
    default:
        assert(n == 0 && "trying to set a subtype on something I don't know about");
        break;
    }
}

bool  SwField::QueryValue( uno::Any& rVal, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
        case FIELD_PROP_BOOL4:
            rVal <<= !m_bIsAutomaticLanguage;
        break;
        case FIELD_PROP_TITLE:
        {
            rVal <<= m_aTitle;
        }
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
        case FIELD_PROP_TITLE:
        {
            OUString aTitle;
            if (rVal >>= aTitle)
            {
                m_aTitle = aTitle;
            }
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
    case SwFieldIds::Dropdown:
    case SwFieldIds::TableOfAuthorities:
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
        bRet = bool(static_cast<const SwDateTimeField*>(this)->GetSubType() & SwDateTimeSubType::Fixed);
        break;

    case SwFieldIds::ExtUser:
        bRet = bool(static_cast<const SwExtUserField*>(this)->GetFormat() & SwAuthorFormat::Fixed);
        break;
    case SwFieldIds::Author:
        bRet = bool(static_cast<const SwAuthorField*>(this)->GetFormat() & SwAuthorFormat::Fixed);
        break;

    case SwFieldIds::Filename:
        bRet = bool(static_cast<const SwFileNameField*>(this)->GetFormat() & SwFileNameFormat::Fixed);
        break;

    case SwFieldIds::DocInfo:
        bRet = bool(static_cast<const SwDocInfoField*>(this)->GetSubType() & SwDocInfoSubType::SubFixed);
        break;
    default: break;
    }
    return bRet;
}

OUString
SwField::ExpandField(bool const bCached, SwRootFrame const*const pLayout) const
{
    if ( m_bUseFieldValueCache )
    {
        if (!bCached) // #i85766# do not expand fields in clipboard documents
        {
            if (GetTypeId() == SwFieldTypesEnum::Authority)
            {
                const SwAuthorityField* pAuthorityField = static_cast<const SwAuthorityField*>(this);
                m_Cache = pAuthorityField->ConditionalExpandAuthIdentifier(pLayout);
            }
            else
                m_Cache = ExpandImpl(pLayout);
        }
        return m_Cache;
    }

    return ExpandImpl(pLayout);
}

std::unique_ptr<SwField> SwField::CopyField() const
{
    std::unique_ptr<SwField> pNew = Copy();
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

SwFieldTypesEnum SwFieldTypeFromString(std::u16string_view rString)
{
    if (rString == u"Date")
        return SwFieldTypesEnum::Date;
    if (rString == u"Time")
        return SwFieldTypesEnum::Time;
    if (rString == u"Filename")
        return SwFieldTypesEnum::Filename;
    if (rString == u"DatabaseName")
        return SwFieldTypesEnum::DatabaseName;
    if (rString == u"Chapter")
        return SwFieldTypesEnum::Chapter;
    if (rString == u"PageNumber")
        return SwFieldTypesEnum::PageNumber;
    if (rString == u"DocumentStatistics")
        return SwFieldTypesEnum::DocumentStatistics;
    if (rString == u"Author")
        return SwFieldTypesEnum::Author;
    if (rString == u"Set")
        return SwFieldTypesEnum::Set;
    if (rString == u"Get")
        return SwFieldTypesEnum::Get;
    if (rString == u"Formel")
        return SwFieldTypesEnum::Formel;
    if (rString == u"HiddenText")
        return SwFieldTypesEnum::HiddenText;
    if (rString == u"SetRef")
        return SwFieldTypesEnum::SetRef;
    return SwFieldTypesEnum::Unknown;
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
    const Color* pCol = nullptr;

    // Bug #60010
    LanguageType nFormatLng = ::lcl_GetLanguageOfFormat( nLng, nFormat );

    if( nFormat < SV_COUNTRY_LANGUAGE_OFFSET && LANGUAGE_SYSTEM != nFormatLng )
    {
        const SvNumberformat* pEntry = pFormatter->GetEntry(nFormat);

        if (pEntry && nLng != pEntry->GetLanguage())
        {
            sal_uInt32 nNewFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nFormat,
                                                    nFormatLng);
            if (nNewFormat == nFormat)
            {
                // probably user-defined format
                OUString sFormat(pEntry->GetFormatstring());
                sal_Int32 nDummy;
                SvNumFormatType nType = SvNumFormatType::DEFINED;

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

OUString SwValueFieldType::GetInputOrDateTime( const OUString& rInput, const double& rVal, sal_uInt32 nFormat ) const
{
    if (nFormat && nFormat != SAL_MAX_UINT32 && UseFormat())
    {
        SvNumberFormatter* pFormatter = m_pDoc->GetNumberFormatter();
        const SvNumberformat* pEntry = pFormatter->GetEntry(nFormat);
        if (pEntry && (pEntry->GetType() & SvNumFormatType::DATETIME))
            return pFormatter->GetInputLineString( rVal, nFormat );
    }
    return rInput;
}

SwValueField::SwValueField( SwValueFieldType* pFieldType, sal_uInt32 nFormat,
                            LanguageType nLng, const double fVal )
    : SwField(pFieldType, nLng)
    , m_fValue(fVal)
    , m_nFormat(nFormat)
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

void SwValueField::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwValueField"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_fValue"), BAD_CAST(OString::number(m_fValue).getStr()));
    SwField::dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
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
        LanguageType nFormatLng = ::lcl_GetLanguageOfFormat( nLng, GetFormat() );

        if( (GetFormat() >= SV_COUNTRY_LANGUAGE_OFFSET ||
             LANGUAGE_SYSTEM != nFormatLng ) &&
            !(Which() == SwFieldIds::User && (static_cast<const SwUserField*>(this)->GetSubType() & SwUserType::ShowCommand) ) )
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
        // Uses the SwCalc document locale.
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

            // Will get reinterpreted by SwCalc when updating fields, so use
            // the proper locale.
            m_sFormula = static_cast<SwValueFieldType *>(GetTyp())->DoubleToString( fTmpValue,
                    SwCalc::GetDocAppScriptLang( *GetDoc()));
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
        const Color* pCol = nullptr;

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

OUString SwFormulaField::GetInputOrDateTime() const
{
    // GetFormula() leads to problems with date formats because only the
    // number string without formatting is returned (additionally that may or
    // may not use a localized decimal separator due to the convoluted handling
    // of "formula"). It must be used for expressions though because otherwise
    // with GetPar2() only the value calculated by SwCalc would be displayed
    // (instead of test2 = test + 1).
    // Force a formatted edit value for date+time formats, assuming they are
    // not editable calculated expressions if the formula doesn't contain
    // arithmetic operators or assignment.

    const OUString aFormula( GetFormula());

    if (comphelper::string::indexOfAny( aFormula, u"=+-*/", 0) == -1)
        return static_cast<SwValueFieldType*>(GetTyp())->GetInputOrDateTime( aFormula, GetValue(), GetFormat());

    return aFormula;
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
    case SwFieldIds::TableOfAuthorities:
        return true;
    default: break;
    }
    return false;
}

void SwField::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwField"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*this).name()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_nLang"), BAD_CAST(OString::number(m_nLang.get()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_aTitle"), BAD_CAST(m_aTitle.toUtf8().getStr()));

    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
