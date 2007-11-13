/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxlng.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-13 15:23:32 $
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

INT32 ImpGetLong( const SbxValues* p )
{
    SbxValues aTmp;
    INT32 nRes;
start:
    switch( +p->eType )
    {
        case SbxNULL:
            SbxBase::SetError( SbxERR_CONVERSION );
        case SbxEMPTY:
            nRes = 0; break;
        case SbxCHAR:
            nRes = p->nChar; break;
        case SbxBYTE:
            nRes = p->nByte; break;
        case SbxINTEGER:
        case SbxBOOL:
            nRes = p->nInteger; break;
        case SbxERROR:
        case SbxUSHORT:
            nRes = p->nUShort; break;
        case SbxLONG:
            nRes = p->nLong; break;
        case SbxULONG:
            if( p->nULong > SbxMAXLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXLNG;
            }
            else
                nRes = (INT32) p->nULong;
            break;
        case SbxSINGLE:
            if( p->nSingle > SbxMAXLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXLNG;
            }
            else if( p->nSingle < SbxMINLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINLNG;
            }
            else
                nRes = (INT32) ImpRound( p->nSingle );
            break;
        case SbxDATE:
        case SbxDOUBLE:
        case SbxLONG64:
        case SbxULONG64:
        case SbxSALINT64:
        case SbxSALUINT64:
        case SbxCURRENCY:
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            double dVal;
            if( p->eType == SbxCURRENCY )
                dVal = ImpCurrencyToDouble( p->nLong64 );
            else if( p->eType == SbxLONG64 )
                dVal = ImpINT64ToDouble( p->nLong64 );
            else if( p->eType == SbxULONG64 )
                dVal = ImpUINT64ToDouble( p->nULong64 );
            else if( p->eType == SbxSALINT64 )
                dVal = static_cast< double >(p->nInt64);
            else if( p->eType == SbxSALUINT64 )
                dVal = ImpSalUInt64ToDouble( p->uInt64 );
            else if( p->eType == SbxDECIMAL )
            {
                dVal = 0.0;
                if( p->pDecimal )
                    p->pDecimal->getDouble( dVal );
            }
            else
                dVal = p->nDouble;

            if( dVal > SbxMAXLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXLNG;
            }
            else if( dVal < SbxMINLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINLNG;
            }
            else
                nRes = (INT32) ImpRound( dVal );
            break;
            }
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pString )
                nRes = 0;
            else
            {
                double d;
                SbxDataType t;
                if( ImpScan( *p->pString, d, t, NULL ) != SbxERR_OK )
                    nRes = 0;
                else if( d > SbxMAXLNG )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXLNG;
                }
                else if( d < SbxMINLNG )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINLNG;
                }
                else
                    nRes = (INT32) ImpRound( d );
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetLong();
            else
            {
                SbxBase::SetError( SbxERR_NO_OBJECT ); nRes = 0;
            }
            break;
        }

        case SbxBYREF | SbxCHAR:
            nRes = *p->pChar; break;
        case SbxBYREF | SbxBYTE:
            nRes = *p->pByte; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            nRes = *p->pInteger; break;
        case SbxBYREF | SbxLONG:
            nRes = *p->pLong; break;

        // ab hier muss getestet werden
        case SbxBYREF | SbxULONG:
            aTmp.nULong = *p->pULong; goto ref;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            aTmp.nUShort = *p->pUShort; goto ref;
        case SbxBYREF | SbxSINGLE:
            aTmp.nSingle = *p->pSingle; goto ref;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            aTmp.nDouble = *p->pDouble; goto ref;
        case SbxBYREF | SbxSALINT64:
            aTmp.nInt64 = *p->pnInt64; goto ref;
        case SbxBYREF | SbxSALUINT64:
            aTmp.uInt64 = *p->puInt64; goto ref;
        case SbxBYREF | SbxULONG64:
            aTmp.nULong64 = *p->pULong64; goto ref;
        case SbxBYREF | SbxLONG64:
        case SbxBYREF | SbxCURRENCY:
            aTmp.nLong64 = *p->pLong64; goto ref;
        ref:
            aTmp.eType = SbxDataType( p->eType & 0x0FFF );
            p = &aTmp; goto start;

        default:
            SbxBase::SetError( SbxERR_CONVERSION ); nRes = 0;
    }
    return nRes;
}

void ImpPutLong( SbxValues* p, INT32 n )
{
    SbxValues aTmp;

start:
    switch( +p->eType )
    {
        // Ab hier muss getestet werden
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxINTEGER:
        case SbxBOOL:
            aTmp.pInteger = &p->nInteger; goto direct;
        case SbxULONG64:
            aTmp.pULong64 = &p->nULong64; goto direct;
        case SbxLONG64:
        case SbxCURRENCY:
            aTmp.pLong64 = &p->nLong64; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxSALUINT64:
            aTmp.puInt64 = &p->uInt64; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort;
        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        // ab hier nicht mehr
        case SbxLONG:
            p->nLong = n; break;
        case SbxSINGLE:
            p->nSingle = (float) n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;
        case SbxSALINT64:
            p->nInt64 = n; break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setLong( n );
            break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pString )
                p->pString = new XubString;
            ImpCvtNum( (double) n, 0, *p->pString );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutLong( n );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            if( n > SbxMAXCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXCHAR;
            }
            else if( n < SbxMINCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINCHAR;
            }
            *p->pChar = (xub_Unicode) n; break;
        case SbxBYREF | SbxBYTE:
            if( n > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXBYTE;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pByte = (BYTE) n; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            if( n > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXINT;
            }
            else if( n < SbxMININT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMININT;
            }
            *p->pInteger = (INT16) n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            if( n > SbxMAXUINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXUINT;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pUShort = (UINT16) n; break;
        case SbxBYREF | SbxLONG:
            *p->pLong = n; break;
        case SbxBYREF | SbxULONG:
            if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pULong = (UINT32) n; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); *p->puInt64 = 0;
            }
            else
                *p->puInt64 = n;
            break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = (float) n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = (double) n; break;
        case SbxBYREF | SbxCURRENCY:
            double d;
            if( n > SbxMAXCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); d = SbxMAXCURR;
            }
            else if( n < SbxMINCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); d = SbxMINCURR;
            }
            else
            {
                d = n;
            }
            *p->pLong64 = ImpDoubleToCurrency( d ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

