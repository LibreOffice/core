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

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/charclass.hxx>

#include <svx/svdmodel.hxx>

#include <calbck.hxx>
#include <calc.hxx>
#include <usrfld.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <editsh.hxx>
#include <dpage.hxx>
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

SwUserField::SwUserField(SwUserFieldType* pTyp, sal_uInt16 nSub, sal_uInt32 nFormat)
    : SwValueField(pTyp, nFormat),
    m_nSubType(nSub)
{
}

OUString SwUserField::ExpandImpl(SwRootFrame const*const) const
{
    if(!(m_nSubType & nsSwExtendedSubType::SUB_INVISIBLE))
        return static_cast<SwUserFieldType*>(GetTyp())->Expand(GetFormat(), m_nSubType, GetLanguage());

    return OUString();
}

std::unique_ptr<SwField> SwUserField::Copy() const
{
    std::unique_ptr<SwField> pTmp(new SwUserField(static_cast<SwUserFieldType*>(GetTyp()), m_nSubType, GetFormat()));
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
    return pTmp;
}

OUString SwUserField::GetFieldName() const
{
    return SwFieldType::GetTypeStr(TYP_USERFLD) +
        " " + GetTyp()->GetName() + " = " +
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
    return static_cast<const SwUserFieldType*>(GetTyp())->GetName();
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

sal_uInt16 SwUserField::GetSubType() const
{
    return static_cast<SwUserFieldType*>(GetTyp())->GetType() | m_nSubType;
}

void SwUserField::SetSubType(sal_uInt16 nSub)
{
    static_cast<SwUserFieldType*>(GetTyp())->SetType(nSub & 0x00ff);
    m_nSubType = nSub & 0xff00;
}

bool SwUserField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL2:
        rAny <<= 0 != (m_nSubType & nsSwExtendedSubType::SUB_CMD);
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= 0 == (m_nSubType & nsSwExtendedSubType::SUB_INVISIBLE);
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
            m_nSubType &= (~nsSwExtendedSubType::SUB_INVISIBLE);
        else
            m_nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
        break;
    case FIELD_PROP_BOOL2:
        if(*o3tl::doAccess<bool>(rAny))
            m_nSubType |= nsSwExtendedSubType::SUB_CMD;
        else
            m_nSubType &= (~nsSwExtendedSubType::SUB_CMD);
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwUserField"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nSubType"), BAD_CAST(OString::number(m_nSubType).getStr()));
    SwValueField::dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

SwUserFieldType::SwUserFieldType( SwDoc* pDocPtr, const OUString& aNam )
    : SwValueFieldType( pDocPtr, SwFieldIds::User ),
    m_nValue( 0 ),
    m_nType(nsSwGetSetExpType::GSE_STRING)
{
    m_bValidValue = m_bDeleted = false;
    m_aName = aNam;

    EnableFormat(false); // Do not use a Numberformatter for nsSwGetSetExpType::GSE_STRING
}

OUString SwUserFieldType::Expand(sal_uInt32 nFormat, sal_uInt16 nSubType, LanguageType nLng)
{
    if((m_nType & nsSwGetSetExpType::GSE_EXPR) && !(nSubType & nsSwExtendedSubType::SUB_CMD))
    {
        EnableFormat();
        return ExpandValue(m_nValue, nFormat, nLng);
    }

    EnableFormat(false);    // Do not use a Numberformatter
    return m_aContent;
}

SwFieldType* SwUserFieldType::Copy() const
{
    SwUserFieldType *pTmp = new SwUserFieldType( GetDoc(), m_aName );
    pTmp->m_aContent      = m_aContent;
    pTmp->m_nType         = m_nType;
    pTmp->m_bValidValue   = m_bValidValue;
    pTmp->m_nValue        = m_nValue;
    pTmp->m_bDeleted      = m_bDeleted;

    return pTmp;
}

OUString SwUserFieldType::GetName() const
{
    return m_aName;
}

void SwUserFieldType::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( !pOld && !pNew )
        m_bValidValue = false;

    NotifyClients( pOld, pNew );

    // update input fields that might be connected to the user field
    if ( !IsModifyLocked() )
    {
        LockModify();
        GetDoc()->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::Input )->UpdateFields();
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
    CharClass* pCharClass = rCalc.GetCharClass();
    LanguageTag aCalcLanguage = pCharClass->getLanguageTag();
    LanguageTag aFieldTypeLanguage(GetFieldTypeLanguage());
    bool bSwitchLanguage = aCalcLanguage != aFieldTypeLanguage;
    if (bSwitchLanguage)
        pCharClass->setLanguageTag(aFieldTypeLanguage);

    m_nValue = rCalc.Calculate( m_aContent ).GetDouble();

    if (bSwitchLanguage)
        pCharClass->setLanguageTag(aCalcLanguage);

    rCalc.Pop();

    if( !rCalc.IsCalcError() )
        m_bValidValue = true;
    else
        m_nValue = 0;

    return m_nValue;
}

OUString SwUserFieldType::GetContent( sal_uInt32 nFormat )
{
    if (nFormat && nFormat != SAL_MAX_UINT32)
    {
        OUString sFormattedValue;
        Color* pCol = nullptr;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        pFormatter->GetOutputString(GetValue(), nFormat, sFormattedValue, &pCol);
        return sFormattedValue;
    }

    return m_aContent;
}

void SwUserFieldType::SetContent( const OUString& rStr, sal_uInt32 nFormat )
{
    if( m_aContent != rStr )
    {
        m_aContent = rStr;

        if (nFormat && nFormat != SAL_MAX_UINT32)
        {
            double fValue;

            if (GetDoc()->IsNumberFormat(rStr, nFormat, fValue))
            {
                SetValue(fValue);
                m_aContent = DoubleToString(fValue, nFormat);
            }
        }

        bool bModified = GetDoc()->getIDocumentState().IsModified();
        GetDoc()->getIDocumentState().SetModified();
        if( !bModified )    // Bug 57028
        {
            GetDoc()->GetIDocumentUndoRedo().SetUndoNoResetModified();
        }
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
        rAny <<= 0 != (nsSwGetSetExpType::GSE_EXPR&m_nType);
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

            m_aContent = DoubleToString(m_nValue, static_cast<sal_uInt16>(GetFieldTypeLanguage()));
        }
        break;
    case FIELD_PROP_PAR2:
        rAny >>= m_aContent;
        break;
    case FIELD_PROP_BOOL1:
        if(*o3tl::doAccess<bool>(rAny))
        {
            m_nType |= nsSwGetSetExpType::GSE_EXPR;
            m_nType &= ~nsSwGetSetExpType::GSE_STRING;
        }
        else
        {
            m_nType &= ~nsSwGetSetExpType::GSE_EXPR;
            m_nType |= nsSwGetSetExpType::GSE_STRING;
        }
        break;
    default:
        assert(false);
    }
}

void SwUserFieldType::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwUserFieldType"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nValue"), BAD_CAST(OString::number(m_nValue).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("aContent"), BAD_CAST(m_aContent.toUtf8().getStr()));
    SwFieldType::dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
