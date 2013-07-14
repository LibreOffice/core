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

#include <switerator.hxx>
#include <cntfrm.hxx>
#include <doc.hxx>
#include <pam.hxx>      // for GetBodyTxtNode
#include <ndtxt.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <expfld.hxx>
#include <docfld.hxx>   // for _SetGetExpFld
#include <unofldmid.h>

using namespace ::com::sun::star;

SwTblFieldType::SwTblFieldType(SwDoc* pDocPtr)
    : SwValueFieldType( pDocPtr, RES_TABLEFLD )
{}

SwFieldType* SwTblFieldType::Copy() const
{
    return new SwTblFieldType(GetDoc());
}

void SwTblField::CalcField( SwTblCalcPara& rCalcPara )
{
    if( rCalcPara.rCalc.IsCalcError() ) // stop if there is already an error set
        return;

    // create pointers from box name
    BoxNmToPtr( rCalcPara.pTbl );
    String sFml( MakeFormula( rCalcPara ));
    SetValue( rCalcPara.rCalc.Calculate( sFml ).GetDouble() );
    ChgValid( !rCalcPara.IsStackOverFlow() ); // is the value again valid?
}

SwTblField::SwTblField( SwTblFieldType* pInitType, const OUString& rFormel,
                        sal_uInt16 nType, sal_uLong nFmt )
    : SwValueField( pInitType, nFmt ), SwTableFormula( rFormel ),
    nSubType(nType)
{
    sExpand = OUString('0');
}

SwField* SwTblField::Copy() const
{
    SwTblField* pTmp = new SwTblField( (SwTblFieldType*)GetTyp(),
                                        SwTableFormula::GetFormula(), nSubType, GetFormat() );
    pTmp->sExpand     = sExpand;
    pTmp->SwValueField::SetValue(GetValue());
    pTmp->SwTableFormula::operator=( *this );
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
    return pTmp;
}

OUString SwTblField::GetFieldName() const
{
    return GetTyp()->GetName() + " " + const_cast<SwTblField *>(this)->GetCommand();
}

/// search TextNode containing this field
const SwNode* SwTblField::GetNodeOfFormula() const
{
    if( !GetTyp()->GetDepends() )
        return 0;

    SwIterator<SwFmtFld,SwFieldType> aIter( *GetTyp() );
    for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
            if( this == pFmtFld->GetFld() )
                return (SwTxtNode*)&pFmtFld->GetTxtFld()->GetTxtNode();
    return 0;
}

OUString SwTblField::GetCommand()
{
    if (EXTRNL_NAME != GetNameType())
    {
        SwNode const*const pNd = GetNodeOfFormula();
        SwTableNode const*const pTblNd = (pNd) ? pNd->FindTableNode() : 0;
        if (pTblNd)
        {
            PtrToBoxNm( &pTblNd->GetTable() );
        }
    }
    return (EXTRNL_NAME == GetNameType())
        ? OUString(SwTableFormula::GetFormula())
        : OUString();
}

OUString SwTblField::Expand() const
{
    if (nSubType & nsSwExtendedSubType::SUB_CMD)
    {
        return const_cast<SwTblField *>(this)->GetCommand();
    }

    if(nSubType & nsSwGetSetExpType::GSE_STRING)
    {
        // es ist ein String
        return sExpand.copy(1, sExpand.getLength()-2);
    }

    return sExpand;
}

sal_uInt16 SwTblField::GetSubType() const
{
    return nSubType;
}

void SwTblField::SetSubType(sal_uInt16 nType)
{
    nSubType = nType;
}


void SwTblField::SetValue( const double& rVal )
{
    SwValueField::SetValue(rVal);
    sExpand = ((SwValueFieldType*)GetTyp())->ExpandValue(rVal, GetFormat(), GetLanguage());
}

OUString SwTblField::GetPar2() const
{
    return SwTableFormula::GetFormula();
}

void SwTblField::SetPar2(const OUString& rStr)
{
    SetFormula( rStr );
}

bool SwTblField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    bool bRet = true;
    switch ( nWhichId )
    {
    case FIELD_PROP_PAR2:
        {
            sal_uInt16 nOldSubType = nSubType;
            SwTblField* pThis = (SwTblField*)this;
            pThis->nSubType |= nsSwExtendedSubType::SUB_CMD;
            rAny <<= Expand();
            pThis->nSubType = nOldSubType;
        }
        break;
    case FIELD_PROP_BOOL1:
        {
            sal_Bool bFormula = 0 != (nsSwExtendedSubType::SUB_CMD & nSubType);
            rAny.setValue(&bFormula, ::getBooleanCppuType());
        }
        break;
    case FIELD_PROP_PAR1:
        rAny <<= GetExpStr();
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= (sal_Int32)GetFormat();
        break;
    default:
        bRet = false;
    }
    return bRet;
}

bool SwTblField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
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
        if(*(sal_Bool*)rAny.getValue())
            nSubType = nsSwGetSetExpType::GSE_FORMULA|nsSwExtendedSubType::SUB_CMD;
        else
            nSubType = nsSwGetSetExpType::GSE_FORMULA;
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
