/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: addin.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:58:15 $
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

#ifndef _ADDIN_H
#define _ADDIN_H

#ifndef _SOLAR_H

#ifndef TRUE
#define TRUE            1
#endif
#ifndef FALSE
#define FALSE           0
#endif

#undef NULL
#define NULL            0

typedef unsigned char       BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      USHORT;
typedef unsigned long       ULONG;

#endif

#ifndef SUPD
/* enable all new interface features */
#define SUPD 9999
#endif

/* all character string returns are limited to 255+1 chars */
#define SO_CHARSTRING_MAX       256
#define SO_StringCopy( dst, src ) \
    (strncpy( dst, src, SO_CHARSTRING_MAX ), dst[SO_CHARSTRING_MAX-1] = '\0')

typedef enum
{
    PTR_DOUBLE,
    PTR_STRING,
    PTR_DOUBLE_ARR,
    PTR_STRING_ARR,
    PTR_CELL_ARR,
    NONE
} ParamType;

#ifndef WIN
#ifdef WNT
#define CALLTYPE        __cdecl
#else
#define CALLTYPE
#endif
#else
#define PASCAL          _pascal
#define FAR             _far
#define CALLTYPE        FAR PASCAL
#endif


#ifdef __cplusplus

extern "C" {

typedef void (CALLTYPE* AdvData)( double& nHandle, void* pData );

extern void CALLTYPE GetFunctionCount( USHORT& nCount );

extern void CALLTYPE GetFunctionData( USHORT&    nNo,
                   char*      pFuncName,
                   USHORT&    nParamCount,
                   ParamType* peType,
                   char*      pInternalName );

extern void CALLTYPE IsAsync( USHORT& nNo, ParamType* peType );

extern void CALLTYPE Advice( USHORT& nNo, AdvData& pfCallback );

extern void CALLTYPE Unadvice( double& nHandle );


/* new in StarOffice 5.0 */

extern void CALLTYPE GetParameterDescription( USHORT& nNo, USHORT& nParam,
char* pName, char* pDesc );

/* new in StarOffice 5.1 */

extern void CALLTYPE SetLanguage( USHORT& nLanguage );


};

#else

typedef void (CALLTYPE* AdvData)( double* nHandle, void* pData );

extern void CALLTYPE GetFunctionCount( USHORT * nCount );

extern void CALLTYPE GetFunctionData( USHORT * nNo,
                   char *      pFuncName,
                   USHORT *    nParamCount,
                   ParamType *  peType,
                   char *      pInternalName );

extern void CALLTYPE IsAsync( USHORT * nNo, ParamType * peType );

extern void CALLTYPE Advice( USHORT * nNo, AdvData * pfCallback );

extern void CALLTYPE Unadvice( double * nHandle );

/* new in StarOffice 5.0 */

extern void CALLTYPE GetParameterDescription( USHORT* nNo, USHORT* nParam,
            char* pName, char* pDesc );


/* new in StarOffice 5.1 */

extern void CALLTYPE SetLanguage( USHORT* nLanguage );


#endif

#endif
