/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dllmgr.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _DLLMGR_HXX
#define _DLLMGR_HXX

#define _SVSTDARR_BYTESTRINGSSORT
#include <svl/svarray.hxx>
#ifndef _SVSTDARR_HXX //autogen
#include <svl/svstdarr.hxx>
#endif

// !!! nur zum debuggen fuer infoboxes !!!
//#ifndef _SV_HXX
//#include <sv.hxx>
//#endif

//#ifndef _TOOLS_HXX
//#include <tools.hxx>
//#endif
#define _SVSTDARR_STRINGS
//#ifndef _SVSTDARR_HXX
//#include <svstdarr.hxx>
//#endif
#ifndef _SBERRORS_HXX
#include <basic/sberrors.hxx>
#endif

class SbxArray;
class SbxVariable;

class ImplSbiDll;
class ImplSbiProc;

SV_DECL_PTRARR_SORT(ImplDllArr,ByteStringPtr,5,5)

class SbiDllMgr
{
    ImplDllArr  aDllArr;

    SbiDllMgr( const SbiDllMgr& );

#ifdef _DLLMGR_CXX
    ImplSbiDll*     GetDll( const ByteString& rDllName );
    SbiDllProc      GetProc( ImplSbiDll*, const ByteString& rProcName );

    SbiDllHandle    CreateDllHandle( const ByteString& rDllName );
    void            FreeDllHandle( SbiDllHandle );
    SbiDllProc      GetProcAddr( SbiDllHandle, const ByteString& pProcName );
    SbError         CallProc( SbiDllProc pProc, SbxArray* pArgs,
                              SbxVariable& rResult );
    SbError         CallProcC( SbiDllProc pProc, SbxArray* pArgs,
                              SbxVariable& rResult );
    void*           CreateStack( SbxArray* pArgs, USHORT& rSize );
    void            CheckDllName( ByteString& rName );
#endif

public:
                    SbiDllMgr();
                    ~SbiDllMgr();

    SbError         Call( const char* pFunc, const char* pDll,
                          SbxArray* pArgs, SbxVariable& rResult,
                          BOOL bCDecl );

    void            FreeDll( const ByteString& rDllName );
};



#endif
