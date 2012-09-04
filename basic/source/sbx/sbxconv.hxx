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

#ifndef _SBXCONV_HXX
#define _SBXCONV_HXX

#include "sbxdec.hxx"

class SbxArray;

// SBXSCAN.CXX
extern void ImpCvtNum( double nNum, short nPrec, ::rtl::OUString& rRes, bool bCoreString=false );
extern SbxError ImpScan
    ( const ::rtl::OUString& rSrc, double& nVal, SbxDataType& rType, sal_uInt16* pLen,
      bool bAllowIntntl=false, bool bOnlyIntntl=false );

// with advanced evaluation (International, "TRUE"/"FALSE")
extern bool ImpConvStringExt( ::rtl::OUString& rSrc, SbxDataType eTargetType );

void ImpGetIntntlSep( sal_Unicode& rcDecimalSep, sal_Unicode& rcThousandSep );

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
void    ImpPutDouble( SbxValues*, double, bool bCoreString=false );

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
