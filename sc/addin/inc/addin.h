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

#ifdef WNT
#define CALLTYPE        __cdecl
#else
#define CALLTYPE
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
