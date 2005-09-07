/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxconv.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:47:37 $
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

#ifndef _SBXCONV_HXX
#define _SBXCONV_HXX

#include "sbxdec.hxx"

// SBXSCAN.CXX
extern void ImpCvtNum( double nNum, short nPrec, String& rRes, BOOL bCoreString=FALSE );
extern SbxError ImpScan
    ( const String& rSrc, double& nVal, SbxDataType& rType, USHORT* pLen,
      BOOL bAllowIntntl=FALSE, BOOL bOnlyIntntl=FALSE );

// mit erweiterter Auswertung (International, "TRUE"/"FALSE")
extern BOOL ImpConvStringExt( String& rSrc, SbxDataType eTargetType );

// SBXINT.CXX

double      ImpRound( double );
INT16       ImpGetInteger( const SbxValues* );
void        ImpPutInteger( SbxValues*, INT16 );
sal_Int64   ImpGetInt64( const SbxValues* );
void        ImpPutInt64( SbxValues*, sal_Int64 );
sal_uInt64  ImpGetUInt64( const SbxValues* );
void        ImpPutUInt64( SbxValues*, sal_uInt64 );

sal_Int64   ImpDoubleToSalInt64( double d );
sal_uInt64  ImpDoubleToSalUInt64( double d );
double      ImpSalUInt64ToDouble( sal_uInt64 n );

// SBXLNG.CXX

INT32   ImpGetLong( const SbxValues* );
void    ImpPutLong( SbxValues*, INT32 );

// SBXSNG.CXX

float   ImpGetSingle( const SbxValues* );
void    ImpPutSingle( SbxValues*, float );

// SBXDBL.CXX

double  ImpGetDouble( const SbxValues* );
void    ImpPutDouble( SbxValues*, double, BOOL bCoreString=FALSE );

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
INT32     ImpGetCurrLong( const SbxValues* );
void      ImpPutCurrLong( SbxValues*, INT32 );
INT32     ImpDoubleToCurrLong( double );
double    ImpCurrLongToDouble( INT32 );
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

String  ImpGetString( const SbxValues* );
String  ImpGetCoreString( const SbxValues* );
void    ImpPutString( SbxValues*, const String* );

// SBXCHAR.CXX

sal_Unicode ImpGetChar( const SbxValues* );
void    ImpPutChar( SbxValues*, sal_Unicode );

// SBXBYTE.CXX
BYTE    ImpGetByte( const SbxValues* );
void    ImpPutByte( SbxValues*, BYTE );

// SBXUINT.CXX

UINT16  ImpGetUShort( const SbxValues* );
void    ImpPutUShort( SbxValues*, UINT16 );

// SBXULNG.CXX

UINT32  ImpGetULong( const SbxValues* );
void    ImpPutULong( SbxValues*, UINT32 );

// SBXBOOL.CXX

INT16   ImpGetBool( const SbxValues* );
void    ImpPutBool( SbxValues*, INT16 );

#endif
