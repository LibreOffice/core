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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
