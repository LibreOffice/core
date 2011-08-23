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

#include <vcl/svapp.hxx>
#include <bf_svtools/zforlist.hxx>
#include <tools/errcode.hxx>
#include <tools/color.hxx>
#include <i18npool/lang.h>
#include "sbx.hxx"
#include "sbxconv.hxx"
#include "math.h"

#include <comphelper/processfactory.hxx>

namespace binfilter {

double ImpGetDate( const SbxValues* p )
{
    double nRes;
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
            nRes = (double) p->nLong; break;
        case SbxULONG:
            nRes = (double) p->nULong; break;
        case SbxSINGLE:
            nRes = p->nSingle; break;
        case SbxDATE:
        case SbxDOUBLE:
            nRes = p->nDouble; break;
        case SbxULONG64:
            nRes = ImpUINT64ToDouble( p->nULong64 ); break;
        case SbxLONG64:
            nRes = ImpINT64ToDouble( p->nLong64 ); break;
        case SbxCURRENCY:
            nRes = ImpCurrencyToDouble( p->nLong64 ); break;
        case SbxSALINT64:
            nRes = static_cast< double >(p->nInt64); break;
        case SbxSALUINT64:
            nRes = ImpSalUInt64ToDouble( p->uInt64 ); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            if( p->pDecimal )
                p->pDecimal->getDouble( nRes );
            else
                nRes = 0.0;
            break;
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pString )
                nRes = 0;
            else
            {
#ifndef DOS
                LanguageType eLangType = GetpApp()->GetSettings().GetLanguage();

                SvNumberFormatter* pFormatter;
                com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > 
                    xFactory = comphelper::getProcessServiceFactory();
                pFormatter = new SvNumberFormatter( xFactory, eLangType );

                sal_uInt32 nIndex;
                xub_StrLen nCheckPos = 0;
                short nType = 127;

                // Standard-Vorlagen des Formatters haben nur zweistellige
                // Jahreszahl. Deshalb eigenes Format registrieren

                // HACK, da der Numberformatter in PutandConvertEntry die Platzhalter
                // fuer Monat, Tag, Jahr nicht entsprechend der Systemeinstellung
                // austauscht. Problem: Print Year(Date) unter engl. BS
                // siehe auch basic\source\runtime\runtime.cxx

                SvtSysLocale aSysLocale;
                DateFormat eDate = aSysLocale.GetLocaleData().getDateFormat();
                String aDateStr;
                switch( eDate )
                {
                    case MDY: aDateStr.AssignAscii( "MM.TT.JJJJ" ); break;
                    case DMY: aDateStr.AssignAscii( "TT.MM.JJJJ" ); break;
                    case YMD: aDateStr.AssignAscii( "JJJJ.MM.TT" ); break;
                    default:  aDateStr.AssignAscii( "MM.TT.JJJJ" );
                }

                String aStr( aDateStr );
                aStr.AppendAscii( " HH:MM:SS" );

                pFormatter->PutandConvertEntry( aStr, nCheckPos,	nType,
                    nIndex, LANGUAGE_GERMAN, eLangType );
                BOOL bSuccess = pFormatter->IsNumberFormat( *p->pString, nIndex, nRes );
                if ( bSuccess )
                {
                    short nType_ = pFormatter->GetType( nIndex );
                    if(!(nType_ & (	NUMBERFORMAT_DATETIME | NUMBERFORMAT_DATE |
                                    NUMBERFORMAT_TIME | NUMBERFORMAT_DEFINED )))
                        bSuccess = FALSE;
                }

                if ( !bSuccess )
                {
                    SbxBase::SetError( SbxERR_CONVERSION ); nRes = 0;
                }

                delete pFormatter;
#else
                SbxBase::SetError( SbxERR_CONVERSION ); nRes = 0;
#endif
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetDate();
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
        case SbxBYREF | SbxULONG:
            nRes = *p->pULong; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            nRes = *p->pUShort; break;
        case SbxBYREF | SbxSINGLE:
            nRes = *p->pSingle; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            nRes = *p->pDouble; break;
        case SbxBYREF | SbxULONG64:
            nRes = ImpUINT64ToDouble( *p->pULong64 ); break;
        case SbxBYREF | SbxLONG64:
            nRes = ImpINT64ToDouble( *p->pLong64 ); break;
        case SbxBYREF | SbxCURRENCY:
            nRes = ImpCurrencyToDouble( *p->pLong64 ); break;
        case SbxBYREF | SbxSALINT64:
            nRes = static_cast< double >(*p->pnInt64); break;
        case SbxBYREF | SbxSALUINT64:
            nRes = ImpSalUInt64ToDouble( *p->puInt64 ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION ); nRes = 0;
    }
    return nRes;
}

void ImpPutDate( SbxValues* p, double n )
{
    SbxValues aTmp;

start:
    switch( +p->eType )
    {
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;

        // ab hier wird getestet
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxINTEGER:
        case SbxBOOL:
            aTmp.pInteger = &p->nInteger; goto direct;
        case SbxLONG:
            aTmp.pLong = &p->nLong; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        case SbxSINGLE:
            aTmp.pSingle = &p->nSingle; goto direct;
        case SbxULONG64:
            aTmp.pULong64 = &p->nULong64; goto direct;
        case SbxLONG64:
        case SbxCURRENCY:
            aTmp.pLong64 = &p->nLong64; goto direct;
        case SbxSALINT64:
            aTmp.pnInt64 = &p->nInt64; goto direct;
        case SbxSALUINT64:
            aTmp.puInt64 = &p->uInt64; goto direct;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            SbxDecimal* pDec = ImpCreateDecimal( p );
            if( !pDec->setDouble( n ) )
                SbxBase::SetError( SbxERR_OVERFLOW ); 
            break;
            }
        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
#ifndef DOS
        {
            if( !p->pString )
                p->pString = new XubString;
            Color* pColor;

            LanguageType eLangType = GetpApp()->GetSettings().GetLanguage();
            SvNumberFormatter* pFormatter;
            com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > 
                xFactory = comphelper::getProcessServiceFactory();
            pFormatter = new SvNumberFormatter( xFactory, eLangType );

            sal_uInt32 nIndex;
            xub_StrLen nCheckPos = 0;
            short nType;

            SvtSysLocale aSysLocale;
            DateFormat eDate = aSysLocale.GetLocaleData().getDateFormat();
            String aStr;
            // ist der ganzzahlige Teil 0, wollen wir kein Jahr!
            if( n <= -1.0 || n >= 1.0 )
            {
                // Time only if != 00:00:00
                if( floor( n ) == n )
                {
                    switch( eDate )
                    {
                        case MDY: aStr.AssignAscii( "MM.TT.JJJJ" ); break;
                        case DMY: aStr.AssignAscii( "TT.MM.JJJJ" ); break;
                        case YMD: aStr.AssignAscii( "JJJJ.MM.TT" ); break;
                        default:  aStr.AssignAscii( "MM.TT.JJJJ" );
                    }
                }
                else
                {
                    switch( eDate )
                    {
                        case MDY: aStr.AssignAscii( "MM.TT.JJJJ HH:MM:SS" ); break;
                        case DMY: aStr.AssignAscii( "TT.MM.JJJJ HH:MM:SS" ); break;
                        case YMD: aStr.AssignAscii( "JJJJ.MM.TT HH:MM:SS" ); break;
                        default:  aStr.AssignAscii( "MM.TT.JJJJ HH:MM:SS" );
                    }
                }
            }
            else
                aStr.AppendAscii( "HH:MM:SS" );

            pFormatter->PutandConvertEntry( aStr,
                nCheckPos,
                nType,
                nIndex,
                LANGUAGE_GERMAN,
                eLangType );
            pFormatter->GetOutputString( n, nIndex, *p->pString, &pColor );
            delete pFormatter;
#endif
            break;
#ifndef DOS
        }
#endif
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutDate( n );
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
            if( n > SbxMAXLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXLNG;
            }
            else if( n < SbxMINLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINLNG;
            }
            *p->pLong = (INT32) n; break;
        case SbxBYREF | SbxULONG:
            if( n > SbxMAXULNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXULNG;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pULong = (UINT32) n; break;
        case SbxBYREF | SbxSINGLE:
            if( n > SbxMAXSNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXSNG;
            }
            else if( n < SbxMINSNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINSNG;
            }
            *p->pSingle = (float) n; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = ImpDoubleToSalInt64( n ); break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = ImpDoubleToSalUInt64( n ); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = (double) n; break;
        case SbxBYREF | SbxCURRENCY:
            if( n > SbxMAXCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXCURR;
            }
            else if( n < SbxMINCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINCURR;
            }
            *p->pLong64 = ImpDoubleToCurrency( n ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
