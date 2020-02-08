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

#include <o3tl/any.hxx>

#include <calc.hxx>
#include <calbck.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <expfld.hxx>
#include <unofldmid.h>

using namespace ::com::sun::star;

SwTableFieldType::SwTableFieldType(SwDoc* pDocPtr)
    : SwValueFieldType( pDocPtr, SwFieldIds::Table )
{}

std::unique_ptr<SwFieldType> SwTableFieldType::Copy() const
{
    return std::make_unique<SwTableFieldType>(GetDoc());
}

void SwTableField::CalcField( SwTableCalcPara& rCalcPara )
{
    if( rCalcPara.m_rCalc.IsCalcError() ) // stop if there is already an error set
        return;

    // create pointers from box name
    BoxNmToPtr( rCalcPara.m_pTable );
    OUString sFormula( MakeFormula( rCalcPara ));
    SetValue( rCalcPara.m_rCalc.Calculate( sFormula ).GetDouble() );
    ChgValid( !rCalcPara.IsStackOverflow() ); // is the value again valid?
}

SwTableField::SwTableField( SwTableFieldType* pInitType, const OUString& rFormel,
                        sal_uInt16 nType, sal_uLong nFormat )
    : SwValueField( pInitType, nFormat ), SwTableFormula( rFormel ),
    m_nSubType(nType)
{
    m_sExpand = "0";
}

std::unique_ptr<SwField> SwTableField::Copy() const
{
    std::unique_ptr<SwTableField> pTmp(new SwTableField( static_cast<SwTableFieldType*>(GetTyp()),
                                        SwTableFormula::GetFormula(), m_nSubType, GetFormat() ));
    pTmp->m_sExpand     = m_sExpand;
    pTmp->SwValueField::SetValue(GetValue());
    pTmp->SwTableFormula::operator=( *this );
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
    return std::unique_ptr<SwField>(pTmp.release());
}

OUString SwTableField::GetFieldName() const
{
    return GetTyp()->GetName() + " " + const_cast<SwTableField *>(this)->GetCommand();
}

/// search TextNode containing this field
const SwNode* SwTableField::GetNodeOfFormula() const
{
    auto pFormat = GetTyp()->FindFormatForField(this);
    return pFormat ? &pFormat->GetTextField()->GetTextNode() : nullptr;
}

OUString SwTableField::GetCommand()
{
    if (EXTRNL_NAME != GetNameType())
    {
        SwNode const*const pNd = GetNodeOfFormula();
        SwTableNode const*const pTableNd = pNd ? pNd->FindTableNode() : nullptr;
        if (pTableNd)
        {
            PtrToBoxNm( &pTableNd->GetTable() );
        }
    }
    return (EXTRNL_NAME == GetNameType())
        ? SwTableFormula::GetFormula()
        : OUString();
}

OUString SwTableField::ExpandImpl(SwRootFrame const*const) const
{
    if (m_nSubType & nsSwExtendedSubType::SUB_CMD)
    {
        return const_cast<SwTableField *>(this)->GetCommand();
    }

    if(m_nSubType & nsSwGetSetExpType::GSE_STRING)
    {
        // it is a string
        return m_sExpand.copy(1, m_sExpand.getLength()-2);
    }

    return m_sExpand;
}

sal_uInt16 SwTableField::GetSubType() const
{
    return m_nSubType;
}

void SwTableField::SetSubType(sal_uInt16 nType)
{
    m_nSubType = nType;
}

void SwTableField::SetValue( const double& rVal )
{
    SwValueField::SetValue(rVal);
    m_sExpand = static_cast<SwValueFieldType*>(GetTyp())->ExpandValue(rVal, GetFormat(), GetLanguage());
}

OUString SwTableField::GetPar2() const
{
    return SwTableFormula::GetFormula();
}

void SwTableField::SetPar2(const OUString& rStr)
{
    SetFormula( rStr );
}

bool SwTableField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    bool bRet = true;
    switch ( nWhichId )
    {
    case FIELD_PROP_PAR2:
        {
            sal_uInt16 nOldSubType = m_nSubType;
            SwTableField* pThis = const_cast<SwTableField*>(this);
            pThis->m_nSubType |= nsSwExtendedSubType::SUB_CMD;
            rAny <<= ExpandImpl(nullptr);
            pThis->m_nSubType = nOldSubType;
        }
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= 0 != (nsSwExtendedSubType::SUB_CMD & m_nSubType);
        break;
    case FIELD_PROP_PAR1:
        rAny <<= m_sExpand;
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= static_cast<sal_Int32>(GetFormat());
        break;
    default:
        bRet = false;
    }
    return bRet;
}

bool SwTableField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    bool bRet = true;
    switch ( nWhichId )
    {
    case FIELD_PROP_PAR2:
        {
            OUString sTmp;
            rAny >>= sTmp;
            SetFormula( sTmp );
        }
        break;
    case FIELD_PROP_BOOL1:
        if(*o3tl::doAccess<bool>(rAny))
            m_nSubType = nsSwGetSetExpType::GSE_FORMULA|nsSwExtendedSubType::SUB_CMD;
        else
            m_nSubType = nsSwGetSetExpType::GSE_FORMULA;
        break;
    case FIELD_PROP_PAR1:
        {
            OUString sTmp;
            rAny >>= sTmp;
            ChgExpStr( sTmp );
        }
        break;
    case FIELD_PROP_FORMAT:
        {
            sal_Int32 nTmp = 0;
            rAny >>= nTmp;
            SetFormat(nTmp);
        }
        break;
    default:
        bRet = false;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
