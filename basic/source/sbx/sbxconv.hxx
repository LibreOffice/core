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

#ifndef _SBXCONV_HXX
#define _SBXCONV_HXX

#include "sbxdec.hxx"

class SbxArray;

// SBXSCAN.CXX
extern void ImpCvtNum( double nNum, short nPrec, ::rtl::OUString& rRes, sal_Bool bCoreString=sal_False );
extern SbxError ImpScan
    ( const ::rtl::OUString& rSrc, double& nVal, SbxDataType& rType, sal_uInt16* pLen,
      sal_Bool bAllowIntntl=sal_False, sal_Bool bOnlyIntntl=sal_False );

// with advanced evaluation (International, "TRUE"/"FALSE")
extern sal_Bool ImpConvStringExt( ::rtl::OUString& rSrc, SbxDataType eTargetType );

// SBXINT.CXX

double      ImpRound( double );
sal_Int16       ImpGetInteger( const SbxValues* );
void        ImpPutInteger( SbxValues*, sal_Int16 );

sal_Int64   ImpGetInt64( const SbxValues* );
void        ImpPutInt64( SbxValues*, sal_Int64 );
sal_uInt64  ImpGetUInt64( const SbxValues* );
void        ImpPutUInt64( SbxValues*, sal_uInt64 );

sal_Int64   ImpDoubleToSalInt64 ( double d );
sal_uInt64  ImpDoubleToSalUInt64( double d );
double      ImpSalInt64ToDouble ( sal_Int64 n );
double      ImpSalUInt64ToDouble( sal_uInt64 n );

// SBXLNG.CXX

sal_Int32   ImpGetLong( const SbxValues* );
void    ImpPutLong( SbxValues*, sal_Int32 );

// SBXSNG.CXX

float   ImpGetSingle( const SbxValues* );
void    ImpPutSingle( SbxValues*, float );

// SBXDBL.CXX

double  ImpGetDouble( const SbxValues* );
void    ImpPutDouble( SbxValues*, double, sal_Bool bCoreString=sal_False );

// SBXCURR.CXX

sal_Int64   ImpGetCurrency( const SbxValues* );
void        ImpPutCurrency( SbxValues*, const sal_Int64 );

inline  sal_Int64   ImpDoubleToCurrency( double d )
                    {   if (d > 0) return (sal_Int64)( d * CURRENCY_FACTOR + 0.5);
                              else return (sal_Int64)( d * CURRENCY_FACTOR - 0.5);
                    }

inline  double      ImpCurrencyToDouble( const sal_Int64 r )
                    { return (double)r / (double)CURRENCY_FACTOR; }


// SBXDEC.CXX

SbxDecimal* ImpCreateDecimal( SbxValues* p );
SbxDecimal* ImpGetDecimal( const SbxValues* p );
void ImpPutDecimal( SbxValues* p, SbxDecimal* pDec );

// SBXDATE.CXX

double  ImpGetDate( const SbxValues* );
void    ImpPutDate( SbxValues*, double );

// SBXSTR.CXX

::rtl::OUString     ImpGetString( const SbxValues* );
::rtl::OUString     ImpGetCoreString( const SbxValues* );
void    ImpPutString( SbxValues*, const ::rtl::OUString* );

// SBXCHAR.CXX

sal_Unicode ImpGetChar( const SbxValues* );
void        ImpPutChar( SbxValues*, sal_Unicode );

// SBXBYTE.CXX
sal_uInt8   ImpGetByte( const SbxValues* );
void    ImpPutByte( SbxValues*, sal_uInt8 );

// SBXUINT.CXX

sal_uInt16  ImpGetUShort( const SbxValues* );
void    ImpPutUShort( SbxValues*, sal_uInt16 );

// SBXULNG.CXX

sal_uInt32  ImpGetULong( const SbxValues* );
void    ImpPutULong( SbxValues*, sal_uInt32 );

// SBXBOOL.CXX

enum SbxBOOL ImpGetBool( const SbxValues* );
void    ImpPutBool( SbxValues*, sal_Int16 );

// ByteArray <--> String
SbxArray* StringToByteArray(const ::rtl::OUString& rStr);
::rtl::OUString ByteArrayToString(SbxArray* pArr);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
