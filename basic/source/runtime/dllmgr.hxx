/*************************************************************************
 *
 *  $RCSfile: dllmgr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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

#ifndef _DLLMGR_HXX
#define _DLLMGR_HXX

#define _SVSTDARR_BYTESTRINGSSORT
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _SVSTDARR_HXX //autogen
#include <svtools/svstdarr.hxx>
#endif

// !!! nur zum debuggen fuer infoboxes !!!
//#ifndef _SV_HXX
//#include <sv.hxx>
//#endif

//#ifndef _TOOLS_HXX
//#include <tools.hxx>
//#endif
#if SUPD > 340
#define _SVSTDARR_STRINGS
//#ifndef _SVSTDARR_HXX
//#include <svstdarr.hxx>
//#endif
#else
//#include <svmem.hxx>
#endif
#ifndef _SBERRORS_HXX
#include <sberrors.hxx>
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
