/*************************************************************************
 *
 *  $RCSfile: tblcalc.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:07:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>      // fuer GetBodyTxtNode
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>    // fuer Modify()
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>   // fuer _SetGetExpFld
#endif
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;


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



SwTblField::SwTblField( SwTblFieldType* pType, const String& rFormel,
                        USHORT nType, ULONG nFmt )
    : SwValueField( pType, nFmt ), SwTableFormula( rFormel ),
    nSubType(nType), sExpand( '0' )
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


String SwTblField::GetCntnt(BOOL bName) const
{
    if( bName )
    {
        String aStr(GetTyp()->GetName());
        aStr += ' ';

        USHORT nOldSubType = nSubType;
        SwTblField* pThis = (SwTblField*)this;
        pThis->nSubType |= SUB_CMD;
        aStr += Expand();
        pThis->nSubType = nOldSubType;

        return aStr;
    }
    return Expand();
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


String SwTblField::Expand() const
{
    String aStr;
    if (nSubType & SUB_CMD)
    {
        if( EXTRNL_NAME != GetNameType() )
        {
            const SwNode* pNd = GetNodeOfFormula();
            const SwTableNode* pTblNd = pNd ? pNd->FindTableNode() : 0;
            if( pTblNd )
                ((SwTblField*)this)->PtrToBoxNm( &pTblNd->GetTable() );
        }
        if( EXTRNL_NAME == GetNameType() )
            aStr = SwTableFormula::GetFormula();
    }
    else
    {
        aStr = sExpand;
        if(nSubType & GSE_STRING)
        {
            // es ist ein String
            aStr = sExpand;
            aStr.Erase( 0,1 );
            aStr.Erase( aStr.Len()-1, 1 );
        }
    }
    return aStr;
}

USHORT SwTblField::GetSubType() const
{
    return nSubType;
}

void SwTblField::SetSubType(USHORT nType)
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


/*-----------------04.03.98 10:33-------------------

--------------------------------------------------*/
BOOL SwTblField::QueryValue( uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    BOOL bRet = TRUE;
    switch ( nMId )
    {
    case FIELD_PROP_PAR2:
        {
            USHORT nOldSubType = nSubType;
            SwTblField* pThis = (SwTblField*)this;
            pThis->nSubType |= SUB_CMD;
            rAny <<= rtl::OUString( Expand() );
            pThis->nSubType = nOldSubType;
        }
        break;
    case FIELD_PROP_BOOL1:
        {
            BOOL bFormula = 0 != (SUB_CMD & nSubType);
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
        bRet = sal_False;
    }
    return bRet;
}
/*-----------------04.03.98 10:33-------------------

--------------------------------------------------*/
BOOL SwTblField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    BOOL bRet = TRUE;
    String sTmp;
    switch ( nMId )
    {
    case FIELD_PROP_PAR2:
        SetFormula( ::GetString( rAny, sTmp ));
        break;
    case FIELD_PROP_BOOL1:
        if(*(sal_Bool*)rAny.getValue())
            nSubType = GSE_FORMULA|SUB_CMD;
        else
            nSubType = GSE_FORMULA;
        break;
    case FIELD_PROP_PAR1:
        ChgExpStr( ::GetString( rAny, sTmp ));
        break;
    case FIELD_PROP_FORMAT:
        {
            sal_Int32 nTmp;
            rAny >>= nTmp;
            SetFormat(nTmp);
        }
        break;
    default:
        bRet = sal_False;
    }
    return bRet;
}




