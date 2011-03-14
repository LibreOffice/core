/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <cntfrm.hxx>
#include <doc.hxx>
#include <pam.hxx>      // fuer GetBodyTxtNode
#include <ndtxt.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <expfld.hxx>
#include <docfld.hxx>   // fuer _SetGetExpFld
#include <unofldmid.h>


using namespace ::com::sun::star;
using ::rtl::OUString;


SwTblFieldType::SwTblFieldType(SwDoc* pDocPtr)
    : SwValueFieldType( pDocPtr, RES_TABLEFLD )
{}


SwFieldType* SwTblFieldType::Copy() const
{
    return new SwTblFieldType(GetDoc());
}



void SwTblField::CalcField( SwTblCalcPara& rCalcPara )
{
    if( rCalcPara.rCalc.IsCalcError() )     // ist schon Fehler gesetzt ?
        return;

    // erzeuge aus den BoxNamen die Pointer
    BoxNmToPtr( rCalcPara.pTbl );
    String sFml( MakeFormel( rCalcPara ));
    SetValue( rCalcPara.rCalc.Calculate( sFml ).GetDouble() );
    ChgValid( !rCalcPara.IsStackOverFlow() );       // ist der Wert wieder gueltig?
}



SwTblField::SwTblField( SwTblFieldType* pInitType, const String& rFormel,
                        sal_uInt16 nType, sal_uLong nFmt )
    : SwValueField( pInitType, nFmt ), SwTableFormula( rFormel ),
    sExpand( '0' ), nSubType(nType)
{
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


String SwTblField::GetFieldName() const
{
    String aStr(GetTyp()->GetName());
    aStr += ' ';
    aStr += const_cast<SwTblField *>(this)->GetCommand();
    return aStr;
}

// suche den TextNode, in dem das Feld steht
const SwNode* SwTblField::GetNodeOfFormula() const
{
    if( !GetTyp()->GetDepends() )
        return 0;

    SwClientIter aIter( *GetTyp() );
    SwClient * pLast = aIter.GoStart();
    if( pLast )     // konnte zum Anfang gesprungen werden ??
        do {
            const SwFmtFld* pFmtFld = (SwFmtFld*)pLast;
            if( this == pFmtFld->GetFld() )
                return (SwTxtNode*)&pFmtFld->GetTxtFld()->GetTxtNode();

        } while( 0 != ( pLast = aIter++ ));
    return 0;
}

String SwTblField::GetCommand()
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
        ? SwTableFormula::GetFormula()
        : String();
}

String SwTblField::Expand() const
{
    String aStr;
    if (nSubType & nsSwExtendedSubType::SUB_CMD)
    {
        aStr = const_cast<SwTblField *>(this)->GetCommand();
    }
    else
    {
        aStr = sExpand;
        if(nSubType & nsSwGetSetExpType::GSE_STRING)
        {
            // es ist ein String
            aStr = sExpand;
            aStr.Erase( 0,1 );
            aStr.Erase( aStr.Len()-1, 1 );
        }
    }
    return aStr;
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

/*--------------------------------------------------------------------
    Beschreibung: Parameter setzen
 --------------------------------------------------------------------*/


String SwTblField::GetPar2() const
{
    return SwTableFormula::GetFormula();
}


void SwTblField::SetPar2(const String& rStr)
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
            rAny <<= rtl::OUString( Expand() );
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
        rAny <<= rtl::OUString(GetExpStr());
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
    String sTmp;
    switch ( nWhichId )
    {
    case FIELD_PROP_PAR2:
        SetFormula( ::GetString( rAny, sTmp ));
        break;
    case FIELD_PROP_BOOL1:
        if(*(sal_Bool*)rAny.getValue())
            nSubType = nsSwGetSetExpType::GSE_FORMULA|nsSwExtendedSubType::SUB_CMD;
        else
            nSubType = nsSwGetSetExpType::GSE_FORMULA;
        break;
    case FIELD_PROP_PAR1:
        ChgExpStr( ::GetString( rAny, sTmp ));
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
