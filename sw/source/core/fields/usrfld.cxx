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

#include <libxml/xmlwriter.h>

#include <o3tl/any.hxx>

#include <svl/numformat.hxx>
#include <unotools/charclass.hxx>

#include <calbck.hxx>
#include <calc.hxx>
#include <usrfld.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <unofldmid.h>

using namespace ::com::sun::star;

namespace
{
/**
 * Returns the language used for float <-> string conversions in
 * SwUserFieldType.
 */
LanguageType GetFieldTypeLanguage()
{
    return LANGUAGE_SYSTEM;
}
}

// Userfields

SwUserField::SwUserField(SwUserFieldType* pTyp, SwUserType nSub, sal_uInt32 nFormat)
    : SwValueField(pTyp, nFormat),
    m_nSubType(nSub)
{
}

OUString SwUserField::ExpandImpl(SwRootFrame const*const) const
{
    if(!(m_nSubType & SwUserType::Invisible))
        return static_cast<SwUserFieldType*>(GetTyp())->Expand(GetFormat(), m_nSubType, GetLanguage());

    return OUString();
}

std::unique_ptr<SwField> SwUserField::Copy() const
{
    std::unique_ptr<SwField> pTmp(new SwUserField(static_cast<SwUserFieldType*>(GetTyp()), m_nSubType, GetFormat()));
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
    pTmp->SetTitle(GetTitle());
    return pTmp;
}

OUString SwUserField::GetFieldName() const
{
    return SwFieldType::GetTypeStr(SwFieldTypesEnum::User) +
        " " + GetTyp()->GetName().toString() + " = " +
        static_cast<SwUserFieldType*>(GetTyp())->GetContent();
}

double SwUserField::GetValue() const
{
    return static_cast<SwUserFieldType*>(GetTyp())->GetValue();
}

void SwUserField::SetValue( const double& rVal )
{
    static_cast<SwUserFieldType*>(GetTyp())->SetValue(rVal);
}

/// Get name
OUString SwUserField::GetPar1() const
{
    return static_cast<const SwUserFieldType*>(GetTyp())->GetName().toString();
}

/// Get content
OUString SwUserField::GetPar2() const
{
    return static_cast<SwUserFieldType*>(GetTyp())->GetContent(GetFormat());
}

void SwUserField::SetPar2(const OUString& rStr)
{
    static_cast<SwUserFieldType*>(GetTyp())->SetContent(rStr, GetFormat());
}

SwUserType SwUserField::GetSubType() const
{
    return static_cast<SwUserFieldType*>(GetTyp())->GetType() | m_nSubType;
}

void SwUserField::SetSubType(SwUserType nSub)
{
    static_cast<SwUserFieldType*>(GetTyp())->SetType(nSub & SwUserType::LowerMask);
    m_nSubType = nSub & SwUserType::UpperMask;
}

bool SwUserField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL2:
        rAny <<= bool(m_nSubType & SwUserType::ShowCommand);
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= !(m_nSubType & SwUserType::Invisible);
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= static_cast<sal_Int32>(GetFormat());
        break;
    default:
        return SwField::QueryValue(rAny, nWhichId);
    }
    return true;
}

bool SwUserField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        if(*o3tl::doAccess<bool>(rAny))
            m_nSubType &= ~SwUserType::Invisible;
        else
            m_nSubType |= SwUserType::Invisible;
        break;
    case FIELD_PROP_BOOL2:
        if(*o3tl::doAccess<bool>(rAny))
            m_nSubType |= SwUserType::ShowCommand;
        else
            m_nSubType &= ~SwUserType::ShowCommand;
        break;
    case FIELD_PROP_FORMAT:
        {
            sal_Int32 nTmp = 0;
            rAny >>= nTmp;
            SetFormat(nTmp);
        }
        break;
    default:
        return SwField::PutValue(rAny, nWhichId);
    }
    return true;
}

void SwUserField::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwUserField"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nSubType"), BAD_CAST(OString::number(static_cast<sal_uInt16>(m_nSubType)).getStr()));
    SwValueField::dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

SwUserFieldType::SwUserFieldType( SwDoc* pDocPtr, const UIName& aNam )
    : SwValueFieldType( pDocPtr, SwFieldIds::User ),
    m_nValue( 0 ),
    m_nType(SwUserType::String)
{
    m_bValidValue = m_bDeleted = false;
    m_aName = aNam;

    EnableFormat(false); // Do not use a Numberformatter for SwUserType::String
}

OUString SwUserFieldType::Expand(sal_uInt32 nFormat, SwUserType nSubType, LanguageType nLng)
{
    if((m_nType & SwUserType::Expr) && !(nSubType & SwUserType::ShowCommand))
    {
        EnableFormat();
        return ExpandValue(m_nValue, nFormat, nLng);
    }

    EnableFormat(false);    // Do not use a Numberformatter
    return m_aContent;
}

std::unique_ptr<SwFieldType> SwUserFieldType::Copy() const
{
    std::unique_ptr<SwUserFieldType> pTmp(new SwUserFieldType( GetDoc(), m_aName ));
    pTmp->m_aContent      = m_aContent;
    pTmp->m_aContentLang  = m_aContentLang;
    pTmp->m_nType         = m_nType;
    pTmp->m_bValidValue   = m_bValidValue;
    pTmp->m_nValue        = m_nValue;
    pTmp->m_bDeleted      = m_bDeleted;

    return pTmp;
}

UIName SwUserFieldType::GetName() const
{
    return m_aName;
}

void SwUserFieldType::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::SwLegacyModify)
    {
        auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
        if (!pLegacy->m_pOld && !pLegacy->m_pNew)
            m_bValidValue = false;
    }
    else if (rHint.GetId() == SfxHintId::SwUpdateAttr)
    {
        auto pUpdateHint = static_cast<const sw::UpdateAttrHint*>(&rHint);
        if (!pUpdateHint->m_pOld && !pUpdateHint->m_pNew)
            m_bValidValue = false;
    }
    else if (rHint.GetId() == SfxHintId::SwAttrSetChange)
    {
        auto pChangeHint = static_cast<const sw::AttrSetChangeHint*>(&rHint);
        if (!pChangeHint->m_pOld && !pChangeHint->m_pNew)
            m_bValidValue = false;
    }

    CallSwClientNotify(rHint);
    // update input fields that might be connected to the user field
    if (!IsModifyLocked())
    {
        LockModify();
        GetDoc()->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Input)->UpdateFields();
        UnlockModify();
    }
}

void SwUserFieldType::UpdateFields()
{
    m_bValidValue = false;
    CallSwClientNotify(sw::LegacyModifyHint(nullptr, nullptr));
    if (!IsModifyLocked())
    {
        LockModify();
        GetDoc()->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Input)->UpdateFields();
        UnlockModify();
    }
}

double SwUserFieldType::GetValue( SwCalc& rCalc )
{
    if(m_bValidValue)
        return m_nValue;

    if(!rCalc.Push( this ))
    {
        rCalc.SetCalcError( SwCalcError::Syntax );
        return 0;
    }

    // See if we need to temporarily switch rCalc's language: in case it
    // differs from the field type locale.
    const CharClass* pCharClass = rCalc.GetCharClass();
    LanguageTag aCharClassLanguage = pCharClass->getLanguageTag();
    LanguageTag aContentLang(m_aContentLang);

    // for the call of calculate we need the language that was used for putting/setting
    // the m_aContent string, otherwise the aContent could be interpreted wrongly,

    bool bSwitchLanguage = m_aContentLang != aCharClassLanguage.getBcp47();

    if (bSwitchLanguage)
        rCalc.SetCharClass(aContentLang);

    m_nValue = rCalc.Calculate( m_aContent ).GetDouble();

    // we than have to set the proper char class languageTag again

    if (bSwitchLanguage)
        rCalc.SetCharClass(aCharClassLanguage);

    rCalc.Pop();

    if( !rCalc.IsCalcError() )
        m_bValidValue = true;
    else
        m_nValue = 0;

    return m_nValue;
}

OUString SwUserFieldType::GetInputOrDateTime( sal_uInt32 nFormat ) const
{
    return static_cast<const SwValueFieldType*>(this)->GetInputOrDateTime( m_aContent, GetValue(), nFormat);
}

OUString SwUserFieldType::GetContent( sal_uInt32 nFormat ) const
{
    if (nFormat && nFormat != SAL_MAX_UINT32)
    {
        OUString sFormattedValue;
        const Color* pCol = nullptr;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        pFormatter->GetOutputString(GetValue(), nFormat, sFormattedValue, &pCol);
        return sFormattedValue;
    }

    return m_aContent;
}

void SwUserFieldType::SetContent( const OUString& rStr, sal_uInt32 nFormat )
{
    if( m_aContent == rStr )
        return;

    m_aContent = rStr;

    if (nFormat && nFormat != SAL_MAX_UINT32)
    {
        double fValue;

        if (GetDoc()->IsNumberFormat(rStr, nFormat, fValue))
        {
            SetValue(fValue);
            LanguageTag aContentLanguage(GetFieldTypeLanguage());
            m_aContentLang = aContentLanguage.getBcp47();
            m_aContent = DoubleToString(fValue, aContentLanguage.getLanguageType());
        }
    }

    bool bModified = GetDoc()->getIDocumentState().IsModified();
    GetDoc()->getIDocumentState().SetModified();
    if( !bModified )    // Bug 57028
    {
        GetDoc()->GetIDocumentUndoRedo().SetUndoNoResetModified();
    }
}

void SwUserFieldType::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_DOUBLE:
        rAny <<= m_nValue;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= m_aContent;
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= bool(SwUserType::Expr & m_nType);
        break;
    default:
        assert(false);
    }
}

void SwUserFieldType::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_DOUBLE:
        {
            double fVal = 0;
            rAny >>= fVal;
            m_nValue = fVal;
            LanguageTag aContentLanguage(GetFieldTypeLanguage());
            m_aContentLang = aContentLanguage.getBcp47();
            m_aContent = DoubleToString(m_nValue, aContentLanguage.getLanguageType());
        }
        break;
    case FIELD_PROP_PAR2:
        rAny >>= m_aContent;
        break;
    case FIELD_PROP_BOOL1:
        if(*o3tl::doAccess<bool>(rAny))
        {
            m_nType |= SwUserType::Expr;
            m_nType &= ~SwUserType::String;
        }
        else
        {
            m_nType &= ~SwUserType::Expr;
            m_nType |= SwUserType::String;
        }
        break;
    default:
        assert(false);
    }
}

void SwUserFieldType::EnsureValid()
{
    if(IsValid())
        return;
    SwCalc aCalc(*GetDoc());
    GetValue(aCalc);
}

void SwUserFieldType::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwUserFieldType"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nValue"), BAD_CAST(OString::number(m_nValue).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("aContent"), BAD_CAST(m_aContent.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("aContentLang"), BAD_CAST(m_aContentLang.toUtf8().getStr()));
    SwFieldType::dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
