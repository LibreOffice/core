/*************************************************************************
 *
 *  $RCSfile: addin.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


#if (SUPD>=398)
/* new in StarOffice 5.0 */

extern void CALLTYPE GetParameterDescription( USHORT& nNo, USHORT& nParam,
char* pName, char* pDesc );

#endif

#if (SUPD>=506)
/* new in StarOffice 5.1 */

extern void CALLTYPE SetLanguage( USHORT& nLanguage );

#endif

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

#if (SUPD>=398)
/* new in StarOffice 5.0 */

extern void CALLTYPE GetParameterDescription( USHORT* nNo, USHORT* nParam,
            char* pName, char* pDesc );

#endif

#if (SUPD>=506)
/* new in StarOffice 5.1 */

extern void CALLTYPE SetLanguage( USHORT* nLanguage );

#endif

#endif

#endif
