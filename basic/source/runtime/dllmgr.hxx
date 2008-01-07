/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dllmgr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:44:47 $
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
