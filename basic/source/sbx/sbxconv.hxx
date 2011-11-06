/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SBXCONV_HXX
#define _SBXCONV_HXX

#include "sbxdec.hxx"

class SbxArray;

// SBXSCAN.CXX
extern void ImpCvtNum( double nNum, short nPrec, ::rtl::OUString& rRes, sal_Bool bCoreString=sal_False );
extern SbxError ImpScan
    ( const ::rtl::OUString& rSrc, double& nVal, SbxDataType& rType, sal_uInt16* pLen,
      sal_Bool bAllowIntntl=sal_False, sal_Bool bOnlyIntntl=sal_False );

// mit erweiterter Auswertung (International, "sal_True"/"sal_False")
extern sal_Bool ImpConvStringExt( ::rtl::OUString& rSrc, SbxDataType eTargetType );

// SBXINT.CXX

double      ImpRound( double );
sal_Int16       ImpGetInteger( const SbxValues* );
void        ImpPutInteger( SbxValues*, sal_Int16 );
sal_Int64   ImpGetInt64( const SbxValues* );
void        ImpPutInt64( SbxValues*, sal_Int64 );
sal_uInt64  ImpGetUInt64( const SbxValues* );
void        ImpPutUInt64( SbxValues*, sal_uInt64 );

sal_Int64   ImpDoubleToSalInt64( double d );
sal_uInt64  ImpDoubleToSalUInt64( double d );
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

#if FALSE
// SBX64.CXX

SbxINT64  ImpGetINT64( const SbxValues* );
void      ImpPutINT64( SbxValues*, const SbxINT64& );
SbxUINT64 ImpGetUINT64( const SbxValues* );
void      ImpPutUINT64( SbxValues*, const SbxUINT64& );
#endif

// SBXCURR.CXX

SbxUINT64 ImpDoubleToUINT64( double );
double    ImpUINT64ToDouble( const SbxUINT64& );
SbxINT64  ImpDoubleToINT64( double );
double    ImpINT64ToDouble( const SbxINT64& );

#if TRUE
sal_Int32     ImpGetCurrLong( const SbxValues* );
void      ImpPutCurrLong( SbxValues*, sal_Int32 );
sal_Int32     ImpDoubleToCurrLong( double );
double    ImpCurrLongToDouble( sal_Int32 );
#endif

SbxINT64  ImpGetCurrency( const SbxValues* );
void      ImpPutCurrency( SbxValues*, const SbxINT64& );
inline
SbxINT64  ImpDoubleToCurrency( double d )
          { return ImpDoubleToINT64( d * CURRENCY_FACTOR ); }
inline
double    ImpCurrencyToDouble( const SbxINT64 &r )
          { return ImpINT64ToDouble( r ) / CURRENCY_FACTOR; }


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
void    ImpPutChar( SbxValues*, sal_Unicode );

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

// ByteArry <--> String
SbxArray* StringToByteArray(const ::rtl::OUString& rStr);
::rtl::OUString ByteArrayToString(SbxArray* pArr);

#endif
