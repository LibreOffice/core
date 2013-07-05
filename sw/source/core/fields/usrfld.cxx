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
#include <IDocumentUndoRedo.hxx>
#include <editsh.hxx>
#include <dpage.hxx>
#include <unofldmid.h>

using namespace ::com::sun::star;

// Userfields

SwUserField::SwUserField(SwUserFieldType* pTyp, sal_uInt16 nSub, sal_uInt32 nFmt)
    : SwValueField(pTyp, nFmt),
    nSubType(nSub)
{
}

String SwUserField::Expand() const
{
    String sStr;
    if(!(nSubType & nsSwExtendedSubType::SUB_INVISIBLE))
        sStr = ((SwUserFieldType*)GetTyp())->Expand(GetFormat(), nSubType, GetLanguage());

    return sStr;
}

SwField* SwUserField::Copy() const
{
    SwField* pTmp = new SwUserField((SwUserFieldType*)GetTyp(), nSubType, GetFormat());
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
    return pTmp;
}

String SwUserField::GetFieldName() const
{
    String aStr(SwFieldType::GetTypeStr(TYP_USERFLD));
    aStr += ' ';
    aStr += GetTyp()->GetName();
    aStr.AppendAscii(" = ");
    aStr += static_cast<SwUserFieldType*>(GetTyp())->GetContent();
    return aStr;
}

double SwUserField::GetValue() const
{
    return ((SwUserFieldType*)GetTyp())->GetValue();
}

void SwUserField::SetValue( const double& rVal )
{
    ((SwUserFieldType*)GetTyp())->SetValue(rVal);
}

/// Get name
const OUString& SwUserField::GetPar1() const
{
    return ((const SwUserFieldType*)GetTyp())->GetName();
}

/// Get content
OUString SwUserField::GetPar2() const
{
    return ((SwUserFieldType*)GetTyp())->GetContent(GetFormat());
}

void SwUserField::SetPar2(const OUString& rStr)
{
    ((SwUserFieldType*)GetTyp())->SetContent(rStr, GetFormat());
}

sal_uInt16 SwUserField::GetSubType() const
{
    return ((SwUserFieldType*)GetTyp())->GetType() | nSubType;
}

void SwUserField::SetSubType(sal_uInt16 nSub)
{
    ((SwUserFieldType*)GetTyp())->SetType(nSub & 0x00ff);
    nSubType = nSub & 0xff00;
}

bool SwUserField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL2:
        {
            sal_Bool bTmp = 0 != (nSubType & nsSwExtendedSubType::SUB_CMD);
            rAny.setValue(&bTmp, ::getBooleanCppuType());
        }
        break;
    case FIELD_PROP_BOOL1:
        {
            sal_Bool bTmp = 0 == (nSubType & nsSwExtendedSubType::SUB_INVISIBLE);
            rAny.setValue(&bTmp, ::getBooleanCppuType());
        }
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
        if(*(sal_Bool*) rAny.getValue())
            nSubType &= (~nsSwExtendedSubType::SUB_INVISIBLE);
        else
            nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
        break;
    case FIELD_PROP_BOOL2:
        if(*(sal_Bool*) rAny.getValue())
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

SwUserFieldType::SwUserFieldType( SwDoc* pDocPtr, const String& aNam )
    : SwValueFieldType( pDocPtr, RES_USERFLD ),
    nValue( 0 ),
    nType(nsSwGetSetExpType::GSE_STRING)
{
    bValidValue = bDeleted = false;
    aName = aNam;

    if (nType & nsSwGetSetExpType::GSE_STRING)
        EnableFormat(sal_False);    // Do not use a Numberformatter
}

String SwUserFieldType::Expand(sal_uInt32 nFmt, sal_uInt16 nSubType, sal_uInt16 nLng)
{
    String aStr(aContent);
    if((nType & nsSwGetSetExpType::GSE_EXPR) && !(nSubType & nsSwExtendedSubType::SUB_CMD))
    {
        EnableFormat(sal_True);
        aStr = ExpandValue(nValue, nFmt, nLng);
    }
    else
        EnableFormat(sal_False);    // Do not use a Numberformatter

    return aStr;
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

const OUString& SwUserFieldType::GetName() const
{
    return aName;
}

void SwUserFieldType::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( !pOld && !pNew )
        ChgValid( sal_False );

    NotifyClients( pOld, pNew );
    // update input fields that might be connected to the user field
    GetDoc()->GetSysFldType( RES_INPUTFLD )->UpdateFlds();
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

String SwUserFieldType::GetContent( sal_uInt32 nFmt )
{
    if (nFmt && nFmt != SAL_MAX_UINT32)
    {
        OUString sFormattedValue;
        Color* pCol = 0;

        SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

        pFormatter->GetOutputString(GetValue(), nFmt, sFormattedValue, &pCol);
        return sFormattedValue;
    }
    else
        return aContent;
}

void SwUserFieldType::SetContent( const String& rStr, sal_uInt32 nFmt )
{
    if( aContent != rStr )
    {
        aContent = rStr;

        if (nFmt && nFmt != SAL_MAX_UINT32)
        {
            double fValue;

            SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();

            if (pFormatter->IsNumberFormat(rStr, nFmt, fValue))
            {
                SetValue(fValue);
                aContent.Erase();
                DoubleToString(aContent, fValue, nFmt);
            }
        }

        sal_Bool bModified = GetDoc()->IsModified();
        GetDoc()->SetModified();
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
        rAny <<= OUString(aContent);
        break;
    case FIELD_PROP_BOOL1:
        {
            sal_Bool bExpression = 0 != (nsSwGetSetExpType::GSE_EXPR&nType);
            rAny.setValue(&bExpression, ::getBooleanCppuType());
        }
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
            // differnt language, but the same content with just different formatting.
            DoubleToString(aContent, nValue, (sal_uInt16)LANGUAGE_SYSTEM);
        }
        break;
    case FIELD_PROP_PAR2:
        ::GetString( rAny, aContent );
        break;
    case FIELD_PROP_BOOL1:
        if(*(sal_Bool*)rAny.getValue())
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
