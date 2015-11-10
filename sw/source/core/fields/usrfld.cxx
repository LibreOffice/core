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

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>

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

// Userfields

SwUserField::SwUserField(SwUserFieldType* pTyp, sal_uInt16 nSub, sal_uInt32 nFormat)
    : SwValueField(pTyp, nFormat),
    nSubType(nSub)
{
}

OUString SwUserField::Expand() const
{
    if(!(nSubType & nsSwExtendedSubType::SUB_INVISIBLE))
        return static_cast<SwUserFieldType*>(GetTyp())->Expand(GetFormat(), nSubType, GetLanguage());

    return OUString();
}

SwField* SwUserField::Copy() const
{
    SwField* pTmp = new SwUserField(static_cast<SwUserFieldType*>(GetTyp()), nSubType, GetFormat());
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
    return static_cast<SwUserFieldType*>(GetTyp())->GetType() | nSubType;
}

void SwUserField::SetSubType(sal_uInt16 nSub)
{
    static_cast<SwUserFieldType*>(GetTyp())->SetType(nSub & 0x00ff);
    nSubType = nSub & 0xff00;
}

bool SwUserField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL2:
        rAny <<= 0 != (nSubType & nsSwExtendedSubType::SUB_CMD);
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= 0 == (nSubType & nsSwExtendedSubType::SUB_INVISIBLE);
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= (sal_Int32)GetFormat();
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
        if(*static_cast<sal_Bool const *>(rAny.getValue()))
            nSubType &= (~nsSwExtendedSubType::SUB_INVISIBLE);
        else
            nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
        break;
    case FIELD_PROP_BOOL2:
        if(*static_cast<sal_Bool const *>(rAny.getValue()))
            nSubType |= nsSwExtendedSubType::SUB_CMD;
        else
            nSubType &= (~nsSwExtendedSubType::SUB_CMD);
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

SwUserFieldType::SwUserFieldType( SwDoc* pDocPtr, const OUString& aNam )
    : SwValueFieldType( pDocPtr, RES_USERFLD ),
    nValue( 0 ),
    nType(nsSwGetSetExpType::GSE_STRING)
{
    bValidValue = bDeleted = false;
    aName = aNam;

    if (nType & nsSwGetSetExpType::GSE_STRING)
        EnableFormat(false);    // Do not use a Numberformatter
}

OUString SwUserFieldType::Expand(sal_uInt32 nFormat, sal_uInt16 nSubType, sal_uInt16 nLng)
{
    if((nType & nsSwGetSetExpType::GSE_EXPR) && !(nSubType & nsSwExtendedSubType::SUB_CMD))
    {
        EnableFormat();
        return ExpandValue(nValue, nFormat, nLng);
    }

    EnableFormat(false);    // Do not use a Numberformatter
    return aContent;
}

SwFieldType* SwUserFieldType::Copy() const
{
    SwUserFieldType *pTmp = new SwUserFieldType( GetDoc(), aName );
    pTmp->aContent      = aContent;
    pTmp->nType         = nType;
    pTmp->bValidValue   = bValidValue;
    pTmp->nValue        = nValue;
    pTmp->bDeleted      = bDeleted;

    return pTmp;
}

OUString SwUserFieldType::GetName() const
{
    return aName;
}

void SwUserFieldType::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( !pOld && !pNew )
        ChgValid( false );

    NotifyClients( pOld, pNew );

    // update input fields that might be connected to the user field
    if ( !IsModifyLocked() )
    {
        LockModify();
        GetDoc()->getIDocumentFieldsAccess().GetSysFieldType( RES_INPUTFLD )->UpdateFields();
        UnlockModify();
    }
}

double SwUserFieldType::GetValue( SwCalc& rCalc )
{
    if(bValidValue)
        return nValue;

    if(!rCalc.Push( this ))
    {
        rCalc.SetCalcError( CALC_SYNTAX );
        return 0;
    }
    nValue = rCalc.Calculate( aContent ).GetDouble();
    rCalc.Pop();

    if( !rCalc.IsCalcError() )
        bValidValue = true;
    else
        nValue = 0;

    return nValue;
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

    return aContent;
}

void SwUserFieldType::SetContent( const OUString& rStr, sal_uInt32 nFormat )
{
    if( aContent != rStr )
    {
        aContent = rStr;

        if (nFormat && nFormat != SAL_MAX_UINT32)
        {
            double fValue;

            SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

            if (pFormatter->IsNumberFormat(rStr, nFormat, fValue))
            {
                SetValue(fValue);
                aContent = DoubleToString(fValue, nFormat);
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

bool SwUserFieldType::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_DOUBLE:
        rAny <<= (double) nValue;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= aContent;
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= 0 != (nsSwGetSetExpType::GSE_EXPR&nType);
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwUserFieldType::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_DOUBLE:
        {
            double fVal = 0;
            rAny >>= fVal;
            nValue = fVal;

            // The following line is in fact wrong, since the language is unknown (is part of the
            // field) and, thus, aContent should also belong to the field. Each field can have a
            // different language, but the same content with just different formatting.
            aContent = DoubleToString(nValue, static_cast<sal_uInt32>(LANGUAGE_SYSTEM));
        }
        break;
    case FIELD_PROP_PAR2:
        rAny >>= aContent;
        break;
    case FIELD_PROP_BOOL1:
        if(*static_cast<sal_Bool const *>(rAny.getValue()))
        {
            nType |= nsSwGetSetExpType::GSE_EXPR;
            nType &= ~nsSwGetSetExpType::GSE_STRING;
        }
        else
        {
            nType &= ~nsSwGetSetExpType::GSE_EXPR;
            nType |= nsSwGetSetExpType::GSE_STRING;
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
