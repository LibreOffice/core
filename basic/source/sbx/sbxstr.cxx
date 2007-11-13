/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxstr.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-13 15:24:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#include <basic/sbx.hxx>
#include "sbxconv.hxx"
#include "sbxres.hxx"

// AB 29.10.99 Unicode
#ifndef _USE_NO_NAMESPACE
using namespace rtl;
#endif


// Die Konversion eines Items auf String wird ueber die Put-Methoden
// der einzelnen Datentypen abgewickelt, um doppelten Code zu vermeiden.

XubString ImpGetString( const SbxValues* p )
{
    SbxValues aTmp;
    XubString aRes;
    aTmp.eType = SbxSTRING;
    aTmp.pString = &aRes;
    switch( +p->eType )
    {
        case SbxNULL:
            SbxBase::SetError( SbxERR_CONVERSION );
        case SbxEMPTY:
            break;
        case SbxCHAR:
            ImpPutChar( &aTmp, p->nChar ); break;
        case SbxBYTE:
            ImpPutByte( &aTmp, p->nByte ); break;
        case SbxINTEGER:
            ImpPutInteger( &aTmp, p->nInteger ); break;
        case SbxBOOL:
            ImpPutBool( &aTmp, p->nUShort ); break;
        case SbxUSHORT:
            ImpPutUShort( &aTmp, p->nUShort ); break;
        case SbxLONG:
            ImpPutLong( &aTmp, p->nLong ); break;
        case SbxULONG:
            ImpPutULong( &aTmp, p->nULong ); break;
        case SbxSINGLE:
            ImpPutSingle( &aTmp, p->nSingle ); break;
        case SbxDOUBLE:
            ImpPutDouble( &aTmp, p->nDouble ); break;
        case SbxCURRENCY:
            ImpPutCurrency( &aTmp, p->nLong64 ); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpPutDecimal( &aTmp, p->pDecimal ); break;
        case SbxSALINT64:
            ImpPutInt64( &aTmp, p->nInt64 ); break;
        case SbxSALUINT64:
            ImpPutUInt64( &aTmp, p->uInt64 ); break;
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( p->pString )
                aRes = *p->pString;
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                aRes = pVal->GetString();
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxERROR:
            // Hier wird der String "Error n" erzeugt
            aRes = SbxRes( STRING_ERRORMSG );
            aRes += p->nUShort; break;
        case SbxDATE:
            ImpPutDate( &aTmp, p->nDouble ); break;

        case SbxBYREF | SbxCHAR:
            ImpPutChar( &aTmp, *p->pChar ); break;
        case SbxBYREF | SbxBYTE:
            ImpPutByte( &aTmp, *p->pByte ); break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            ImpPutInteger( &aTmp, *p->pInteger ); break;
        case SbxBYREF | SbxLONG:
            ImpPutLong( &aTmp, *p->pLong ); break;
        case SbxBYREF | SbxULONG:
            ImpPutULong( &aTmp, *p->pULong ); break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            ImpPutUShort( &aTmp, *p->pUShort ); break;
        case SbxBYREF | SbxSINGLE:
            ImpPutSingle( &aTmp, *p->pSingle ); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            ImpPutDouble( &aTmp, *p->pDouble ); break;
        case SbxBYREF | SbxCURRENCY:
            ImpPutCurrency( &aTmp, *p->pLong64 ); break;
        case SbxBYREF | SbxSALINT64:
            ImpPutInt64( &aTmp, *p->pnInt64 ); break;
        case SbxBYREF | SbxSALUINT64:
            ImpPutUInt64( &aTmp, *p->puInt64 ); break;
        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
    return aRes;
}

// AB 10.4.97, neue Funktion fuer SbxValue::GetCoreString()
XubString ImpGetCoreString( const SbxValues* p )
{
    // Vorerst nur fuer double
    if( ( p->eType & (~SbxBYREF) ) == SbxDOUBLE )
    {
        SbxValues aTmp;
        XubString aRes;
        aTmp.eType = SbxSTRING;
        aTmp.pString = &aRes;
        if( p->eType == SbxDOUBLE )
            ImpPutDouble( &aTmp, p->nDouble, /*bCoreString=*/TRUE );
        else
            ImpPutDouble( &aTmp, *p->pDouble, /*bCoreString=*/TRUE );
        return aRes;
    }
    else
        return ImpGetString( p );
}

void ImpPutString( SbxValues* p, const XubString* n )
{
    SbxValues aTmp;
    aTmp.eType = SbxSTRING;
    XubString* pTmp = NULL;
    // Sicherheitshalber, falls ein NULL-Ptr kommt
    if( !n )
        n = pTmp = new XubString;
    aTmp.pString = (XubString*) n;
    switch( +p->eType )
    {
        case SbxCHAR:
            p->nChar = ImpGetChar( &aTmp ); break;
        case SbxBYTE:
            p->nByte = ImpGetByte( &aTmp ); break;
        case SbxINTEGER:
        case SbxBOOL:
            p->nInteger = ImpGetInteger( &aTmp ); break;
        case SbxLONG:
            p->nLong = ImpGetLong( &aTmp ); break;
        case SbxULONG:
            p->nULong = ImpGetULong( &aTmp ); break;
        case SbxERROR:
        case SbxUSHORT:
            p->nUShort = ImpGetUShort( &aTmp ); break;
        case SbxSINGLE:
            p->nSingle = ImpGetSingle( &aTmp ); break;
        case SbxDATE:
            p->nDouble = ImpGetDate( &aTmp ); break;
        case SbxDOUBLE:
            p->nDouble = ImpGetDouble( &aTmp ); break;
        case SbxULONG64:
            p->nLong64 = ImpGetCurrency( &aTmp ); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            releaseDecimalPtr( p->pDecimal );
            p->pDecimal = ImpGetDecimal( &aTmp ); break;
        case SbxSALINT64:
            p->nInt64 = ImpGetInt64( &aTmp ); break;
        case SbxSALUINT64:
            p->uInt64 = ImpGetUInt64( &aTmp ); break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( n->Len() )
            {
                if( !p->pString )
                    p->pString = new XubString;
                *p->pString = *n;
            }
            else
                delete p->pString, p->pString = NULL;
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutString( *n );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            *p->pChar = ImpGetChar( p ); break;
        case SbxBYREF | SbxBYTE:
            *p->pByte = ImpGetByte( p ); break;
        case SbxBYREF | SbxINTEGER:
            *p->pInteger = ImpGetInteger( p ); break;
        case SbxBYREF | SbxBOOL:
            *p->pUShort = sal::static_int_cast< UINT16 >( ImpGetBool( p ) );
            break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            *p->pUShort = ImpGetUShort( p ); break;
        case SbxBYREF | SbxLONG:
            *p->pLong = ImpGetLong( p ); break;
        case SbxBYREF | SbxULONG:
            *p->pULong = ImpGetULong( p ); break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = ImpGetSingle( p ); break;
        case SbxBYREF | SbxDATE:
            *p->pDouble = ImpGetDate( p ); break;
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = ImpGetDouble( p ); break;
        case SbxBYREF | SbxCURRENCY:
            *p->pLong64 = ImpGetCurrency( p ); break;
        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
    delete pTmp;
}

