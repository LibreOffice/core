/*************************************************************************
 *
 *  $RCSfile: usrfld.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:07:33 $
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

#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif


#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _CALC_HXX
#include <calc.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DPAGE_HXX
#include <dpage.hxx>
#endif
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;
/*--------------------------------------------------------------------
    Beschreibung: Benutzerfelder
 --------------------------------------------------------------------*/

SwUserField::SwUserField(SwUserFieldType* pTyp, sal_uInt16 nSub, sal_uInt32 nFmt)
    : SwValueField(pTyp, nFmt),
    nSubType(nSub)
{
}

String SwUserField::Expand() const
{
    String sStr;
    if(!(nSubType & SUB_INVISIBLE))
        sStr = ((SwUserFieldType*)GetTyp())->Expand(GetFormat(), nSubType, GetLanguage());

    return sStr;
}

SwField* SwUserField::Copy() const
{
    SwField* pTmp = new SwUserField((SwUserFieldType*)GetTyp(), nSubType, GetFormat());
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
    return pTmp;
}

String SwUserField::GetCntnt(sal_Bool bName) const
{
    if ( bName )
    {   String aStr(SwFieldType::GetTypeStr(TYP_USERFLD));
        aStr += ' ';
        aStr += GetTyp()->GetName();
        aStr.AppendAscii(" = ");
        aStr += ((SwUserFieldType*)GetTyp())->GetContent();
        return aStr;
    }
    return Expand();
}

double SwUserField::GetValue() const
{
    return ((SwUserFieldType*)GetTyp())->GetValue();
}

void SwUserField::SetValue( const double& rVal )
{
    ((SwUserFieldType*)GetTyp())->SetValue(rVal);
}

/*--------------------------------------------------------------------
    Beschreibung: Name
 --------------------------------------------------------------------*/

const String& SwUserField::GetPar1() const
{
    return ((SwUserFieldType*)GetTyp())->GetName();
}

/*--------------------------------------------------------------------
    Beschreibung: Content
 --------------------------------------------------------------------*/

String SwUserField::GetPar2() const
{
    return ((SwUserFieldType*)GetTyp())->GetContent(GetFormat());
}

void SwUserField::SetPar2(const String& rStr)
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

/*-----------------09.03.98 08:04-------------------

--------------------------------------------------*/
BOOL SwUserField::QueryValue( uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_BOOL2:
        {
            BOOL bTmp = 0 != (nSubType & SUB_CMD);
            rAny.setValue(&bTmp, ::getBooleanCppuType());
        }
        break;
    case FIELD_PROP_BOOL1:
        {
            BOOL bTmp = 0 == (nSubType & SUB_INVISIBLE);
            rAny.setValue(&bTmp, ::getBooleanCppuType());
        }
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= (sal_Int32)GetFormat();
        break;
    default:
        return SwField::QueryValue(rAny, nMId);
    }
    return sal_True;
}
/*-----------------09.03.98 08:04-------------------

--------------------------------------------------*/
sal_Bool SwUserField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_BOOL1:
        if(*(sal_Bool*) rAny.getValue())
            nSubType &= (~SUB_INVISIBLE);
        else
            nSubType |= SUB_INVISIBLE;
        break;
    case FIELD_PROP_BOOL2:
        if(*(sal_Bool*) rAny.getValue())
            nSubType |= SUB_CMD;
        else
            nSubType &= (~SUB_CMD);
        break;
    case FIELD_PROP_FORMAT:
        {
            sal_Int32 nTmp;
            rAny >>= nTmp;
            SetFormat(nTmp);
        }
        break;
    default:
        return SwField::PutValue(rAny, nMId);
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung: Benutzerfeldtypen
 --------------------------------------------------------------------*/

SwUserFieldType::SwUserFieldType( SwDoc* pDocPtr, const String& aNam )
    : SwValueFieldType( pDocPtr, RES_USERFLD ),
    nType(GSE_STRING),
    nValue( 0 )
{
    bValidValue = bDeleted = sal_False;
    aName = aNam;

    if (nType & GSE_STRING)
        EnableFormat(sal_False);    // Numberformatter nicht einsetzen
}

String SwUserFieldType::Expand(sal_uInt32 nFmt, sal_uInt16 nSubType, sal_uInt16 nLng)
{
    String aStr(aContent);
    if((nType & GSE_EXPR) && !(nSubType & SUB_CMD))
    {
        EnableFormat(sal_True);
        aStr = ExpandValue(nValue, nFmt, nLng);
    }
    else
        EnableFormat(sal_False);    // Numberformatter nicht einsetzen

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

const String& SwUserFieldType::GetName() const
{
    return aName;
}

void SwUserFieldType::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    if( !pOld && !pNew )
        ChgValid( sal_False );

    SwModify::Modify( pOld, pNew );
    // und ggfs. am UserFeld haengende InputFelder updaten!
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
    rCalc.Pop( this );

    if( !rCalc.IsCalcError() )
        bValidValue = sal_True;
    else
        nValue = 0;

    return nValue;
}

String SwUserFieldType::GetContent( sal_uInt32 nFmt )
{
    if (nFmt && nFmt != ULONG_MAX)
    {
        String sFormattedValue;
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

        if (nFmt && nFmt != ULONG_MAX)
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

        // der SdrPage (und damit den VCControls) sagen, das sich was getan hat
        if( GetDoc()->GetDrawModel() && GetDepends() )
            ((SwDPage*)GetDoc()->GetDrawModel()->GetPage( 0 ))->
                    UpdateLinkData( aName, aContent );

        sal_Bool bModified = GetDoc()->IsModified();
        GetDoc()->SetModified();
        if( !bModified )    // Bug 57028
            GetDoc()->SetUndoNoResetModified();
    }
}

void SwUserFieldType::CtrlSetContent( const String& rStr )
{
    if( aContent != rStr )
    {
        aContent = rStr;
        bValidValue = sal_False;

        sal_Bool bModified = GetDoc()->IsModified();
        GetDoc()->SetModified();
        if( !bModified )    // Bug 57028
            GetDoc()->SetUndoNoResetModified();

        // dann mal alle Feldern updaten
        if( GetDepends() )
        {
            SwEditShell* pSh = GetDoc()->GetEditShell();
            if( pSh )
                pSh->StartAllAction();

            Modify( 0, 0 );
            GetDoc()->UpdateUsrFlds();
            GetDoc()->UpdateExpFlds();

            GetDoc()->SetModified();
            if( pSh )
                pSh->EndAllAction();
        }
    }
}
/*-----------------04.03.98 17:05-------------------

--------------------------------------------------*/
BOOL SwUserFieldType::QueryValue( uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_DOUBLE:
        rAny <<= (double) nValue;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= rtl::OUString(aContent);
        break;
    case FIELD_PROP_BOOL1:
        {
            BOOL bExpression = 0 != (GSE_EXPR&nType);
            rAny.setValue(&bExpression, ::getBooleanCppuType());
        }
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}
/*-----------------04.03.98 17:05-------------------

--------------------------------------------------*/
BOOL SwUserFieldType::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_DOUBLE:
        {
            double fVal;
            rAny >>= fVal;
            nValue = fVal;

            // Folgende Zeile ist eigentlich falsch, da die Sprache unbekannt ist
            // (haengt am Feld) und aContent daher auch eigentlich ans Feld gehoeren
            // muesste. Jedes Feld kann eine andere Sprache, aber den gleichen Inhalt
            // haben, nur die Formatierung ist unterschiedlich.
            DoubleToString(aContent, nValue, (sal_uInt16)LANGUAGE_SYSTEM);
        }
        break;
    case FIELD_PROP_PAR2:
        ::GetString( rAny, aContent );
        break;
    case FIELD_PROP_BOOL1:
        if(*(sal_Bool*)rAny.getValue())
        {
            nType |= GSE_EXPR;
            nType &= ~GSE_STRING;
        }
        else
        {
            nType &= ~GSE_EXPR;
            nType |= GSE_STRING;
        }
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}



