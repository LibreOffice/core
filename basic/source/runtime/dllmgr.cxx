/*************************************************************************
 *
 *  $RCSfile: dllmgr.cxx,v $
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

#include <stdlib.h>
#ifdef OS2
#define INCL_DOSMODULEMGR
#include <tools/svpm.h>
#endif

#if defined( WIN ) || defined( WNT )
#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _SBXVAR_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

#if defined(WIN)
typedef HINSTANCE SbiDllHandle;
typedef FARPROC SbiDllProc;
#elif defined(WNT)
typedef HMODULE SbiDllHandle;
typedef int(*SbiDllProc)();
#elif defined(OS2)
typedef HMODULE SbiDllHandle;
typedef PFN SbiDllProc;

#else
typedef void* SbiDllHandle;
typedef void* SbiDllProc;
#endif

#define _DLLMGR_CXX
#include "dllmgr.hxx"
#include <sberrors.hxx>

#ifndef CDECL
#ifdef WNT
//#define CDECL __cdecl
#define CDECL
#else
#ifdef WTC
#define CDECL cdecl
#else
#if defined(ICC) && defined(OS2)
#define CDECL _System
#else
#define CDECL
#endif
#endif
#endif
#endif

extern "C" {
#if defined(INTEL) && (defined(WIN) || defined(WNT) || defined(OS2))

extern INT16 CDECL CallINT( SbiDllProc, char *stack, short nstack);
extern INT32 CDECL CallLNG( SbiDllProc, char *stack, short nstack);
#ifndef WNT
extern float CDECL CallSNG( SbiDllProc, char *stack, short nstack);
#endif
extern double CDECL CallDBL( SbiDllProc, char *stack, short nstack);
extern char* CDECL CallSTR( SbiDllProc, char *stack, short nstack);
// extern CallFIX( SbiDllProc, char *stack, short nstack);

#else

INT16 CallINT( SbiDllProc, char *, short ) { return 0; }
INT32 CallLNG( SbiDllProc, char *, short ) { return 0; }
float CallSNG( SbiDllProc, char *, short ) { return 0; }
double CallDBL( SbiDllProc, char *, short) { return 0; }
char* CallSTR( SbiDllProc, char *, short ) { return 0; }
#endif
}

SV_IMPL_OP_PTRARR_SORT(ImplDllArr,ByteStringPtr)

/* mit Optimierung An stuerzt unter Win95 folgendes Makro ab:
declare Sub MessageBeep Lib "user32" (ByVal long)
sub main
    MessageBeep( 1 )
end sub
*/
#if defined (WNT) && defined (MSC)
//#pragma optimize ("", off)
#endif

//
// ***********************************************************************
//

class ImplSbiProc : public ByteString
{
    SbiDllProc pProc;
    ImplSbiProc();
    ImplSbiProc( const ImplSbiProc& );

public:
    ImplSbiProc( const ByteString& rName, SbiDllProc pFunc )
        : ByteString( rName ) { pProc = pFunc;  }
    SbiDllProc GetProc() const { return pProc; }
};

//
// ***********************************************************************
//

class ImplSbiDll : public ByteString
{
    ImplDllArr      aProcArr;
    SbiDllHandle    hDLL;

    ImplSbiDll( const ImplSbiDll& );
public:
    ImplSbiDll( const ByteString& rName, SbiDllHandle hHandle )
        : ByteString( rName ) { hDLL = hHandle; }
    ~ImplSbiDll();
    SbiDllHandle GetHandle() const { return hDLL; }
    SbiDllProc GetProc( const ByteString& rName ) const;
    void InsertProc( const ByteString& rName, SbiDllProc pProc );
};

ImplSbiDll::~ImplSbiDll()
{
    USHORT nCount = aProcArr.Count();
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        ImplSbiProc* pProc = (ImplSbiProc*)aProcArr.GetObject( nCur );
        delete pProc;
    }
}

SbiDllProc ImplSbiDll::GetProc( const ByteString& rName ) const
{
    USHORT nPos;
    BOOL bRet = aProcArr.Seek_Entry( (ByteStringPtr)&rName, &nPos );
    if( bRet )
    {
        ImplSbiProc* pImplProc = (ImplSbiProc*)aProcArr.GetObject(nPos);
        return pImplProc->GetProc();
    }
    return (SbiDllProc)0;
}

void ImplSbiDll::InsertProc( const ByteString& rName, SbiDllProc pProc )
{
    DBG_ASSERT(aProcArr.Seek_Entry((ByteStringPtr)&rName,0)==0,"InsertProc: Already in table");
    ImplSbiProc* pImplProc = new ImplSbiProc( rName, pProc );
    aProcArr.Insert( (ByteStringPtr)pImplProc );
}


//
// ***********************************************************************
//

SbiDllMgr::SbiDllMgr( const SbiDllMgr& )
{
}

SbiDllMgr::SbiDllMgr()
{
}

SbiDllMgr::~SbiDllMgr()
{
    USHORT nCount = aDllArr.Count();
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        ImplSbiDll* pDll = (ImplSbiDll*)aDllArr.GetObject( nCur );
        FreeDllHandle( pDll->GetHandle() );
        delete pDll;
    }
}

void SbiDllMgr::FreeDll( const ByteString& rDllName )
{
    USHORT nPos;
    BOOL bRet = aDllArr.Seek_Entry( (ByteStringPtr)&rDllName, &nPos );
    if( bRet )
    {
        ImplSbiDll* pDll = (ImplSbiDll*)aDllArr.GetObject(nPos);
        FreeDllHandle( pDll->GetHandle() );
        delete pDll;
        aDllArr.Remove( nPos, 1 );
    }
}


ImplSbiDll* SbiDllMgr::GetDll( const ByteString& rDllName )
{
    USHORT nPos;
    ImplSbiDll* pDll = 0;
    BOOL bRet = aDllArr.Seek_Entry( (ByteStringPtr)&rDllName, &nPos );
    if( bRet )
        pDll = (ImplSbiDll*)aDllArr.GetObject(nPos);
    else
    {
        SbiDllHandle hDll = CreateDllHandle( rDllName );
        if( hDll )
        {
            pDll = new ImplSbiDll( rDllName, hDll );
            aDllArr.Insert( (ByteStringPtr)pDll );
        }
    }
    return pDll;
}

SbiDllProc SbiDllMgr::GetProc( ImplSbiDll* pDll, const ByteString& rProcName )
{
    DBG_ASSERT(pDll,"GetProc: No dll-ptr");
    SbiDllProc pProc;
    pProc = pDll->GetProc( rProcName );
    if( !pProc )
    {
        pProc = GetProcAddr( pDll->GetHandle(), rProcName );
        if( pProc )
            pDll->InsertProc( rProcName, pProc );
    }
    return pProc;
}


SbError SbiDllMgr::Call( const char* pProcName, const char* pDllName,
    SbxArray* pArgs, SbxVariable& rResult, BOOL bCDecl )
{
    DBG_ASSERT(pProcName&&pDllName,"Call: Bad parms");
    SbError nSbErr = 0;
    ByteString aDllName( pDllName );
    CheckDllName( aDllName );
    ImplSbiDll* pDll = GetDll( aDllName );
    if( pDll )
    {
        SbiDllProc pProc = GetProc( pDll, pProcName );
        if( pProc )
        {
            if( bCDecl )
                nSbErr = CallProcC( pProc, pArgs, rResult );
            else
                nSbErr = CallProc( pProc, pArgs, rResult );
        }
        else
            nSbErr = SbERR_PROC_UNDEFINED;
    }
    else
        nSbErr = SbERR_BAD_DLL_LOAD;
    return nSbErr;
}

//  ***********************************************************************
//  ******************* abhaengige Implementationen ***********************
//  ***********************************************************************

void SbiDllMgr::CheckDllName( ByteString& rDllName )
{
#if defined(WIN) || defined(WNT) // || defined(OS2)
    if( rDllName.Search('.') == STRING_NOTFOUND )
        rDllName += ".DLL";
#endif
}


SbiDllHandle SbiDllMgr::CreateDllHandle( const ByteString& rDllName )
{
#if defined(MAC) || defined(UNX)
    SbiDllHandle hLib=0;
#else
    SbiDllHandle hLib;
#endif

#if defined(WIN)
    hLib = LoadLibrary( (const char*)rDllName );
    if( (ULONG)hLib < 32 )
        hLib = 0;

#elif defined(WNT)
    hLib = LoadLibrary( rDllName.GetBuffer() );
    if( !(ULONG)hLib  )
    {
#ifdef DBG_UTIL
        ULONG nLastErr = GetLastError();
#endif
        hLib = 0;
    }

#elif defined(OS2)
    char cErr[ 100 ];
    if( DosLoadModule( (PSZ) cErr, 100, (const char*)rDllName, &hLib ) )
        hLib = 0;
#endif
    return hLib;
}

void SbiDllMgr::FreeDllHandle( SbiDllHandle hLib )
{
#if defined(WIN) || defined(WNT)
    if( hLib )
        FreeLibrary ((HINSTANCE) hLib);
#elif defined(OS2)
    if( hLib )
        DosFreeModule( (HMODULE) hLib );
#endif
}

SbiDllProc SbiDllMgr::GetProcAddr(SbiDllHandle hLib, const ByteString& rProcName)
{
    char buf1 [128];
    char buf2 [128];

    SbiDllProc pProc = 0;
    short nOrd = 0;

    // Ordinal?
    if( rProcName.GetBuffer()[0] == '@' )
        nOrd = atoi( rProcName.GetBuffer()+1 );

    // Moegliche Parameter weg:
    strcpy( buf1, rProcName.GetBuffer() );
    char *p = strchr( buf1, '#' );
    if( p )
        *p = 0;
    strcpy( buf2, "_" );
    strcat( buf2, buf1 );

#if defined(WIN) || defined(WNT)
    if( nOrd > 0 )
        pProc = (SbiDllProc)GetProcAddress( hLib, (char*)(long) nOrd );
    else
    {
        // 2. mit Parametern:
        pProc = (SbiDllProc)GetProcAddress ( hLib, rProcName.GetBuffer() );
        // 3. nur der Name:
        if (!pProc)
            pProc = (SbiDllProc)GetProcAddress( hLib, buf1 );
        // 4. der Name mit Underline vorweg:
        if( !pProc )
            pProc = (SbiDllProc)GetProcAddress( hLib, buf2 );
    }

#elif defined(OS2)
    PSZ pp;
    APIRET rc;
    // 1. Ordinal oder mit Parametern:
    rc = DosQueryProcAddr( hLib, nOrd, pp = (char*)rProcName.GetStr(), &pProc );
    // 2. nur der Name:
    if( rc )
        rc = DosQueryProcAddr( hLib, 0, pp = (PSZ)buf1, &pProc );
    // 3. der Name mit Underline vorweg:
    if( rc )
        rc = DosQueryProcAddr( hLib, 0, pp = (PSZ)buf2, &pProc );
    if( rc )
        pProc = NULL;
    else
    {
        // 16-bit oder 32-bit?
        ULONG nInfo = 0;
        if( DosQueryProcType( hLib, nOrd, pp, &nInfo ) )
            nInfo = 0;;
    }
#endif
    return pProc;
}

SbError SbiDllMgr::CallProc( SbiDllProc pProc, SbxArray* pArgs,
  SbxVariable& rResult )
{
//  ByteString aStr("Calling DLL at ");
//  aStr += (ULONG)pProc;
//  InfoBox( 0, aStr ).Execute();
    INT16 nInt16; int nInt; INT32 nInt32; float nSingle; double nDouble;
    char* pStr;

    USHORT nSize;
    char* pStack = (char*)CreateStack( pArgs, nSize );
    switch( rResult.GetType() )
    {
        case SbxINTEGER:
            nInt16 = CallINT(pProc, pStack, (short)nSize );
            rResult.PutInteger( nInt16 );
            break;

        case SbxUINT:
        case SbxUSHORT:
            nInt16 = (INT16)CallINT(pProc, pStack, (short)nSize );
            rResult.PutUShort( (USHORT)nInt16 );
            break;

        case SbxERROR:
            nInt16 = (INT16)CallINT(pProc, pStack, (short)nSize );
            rResult.PutErr( (USHORT)nInt16 );
            break;

        case SbxINT:
            nInt = CallINT(pProc, pStack, (short)nSize );
            rResult.PutInt( nInt );
            break;

        case SbxLONG:
            nInt32 = CallLNG(pProc, pStack, (short)nSize );
            rResult.PutLong( nInt32 );
            break;

        case SbxULONG:
            nInt32 = CallINT(pProc, pStack, (short)nSize );
            rResult.PutULong( (ULONG)nInt32 );
            break;

#ifndef WNT
        case SbxSINGLE:
            nSingle = CallSNG(pProc, pStack, (short)nSize );
            rResult.PutSingle( nSingle );
            break;
#endif

        case SbxDOUBLE:
#ifdef WNT
        case SbxSINGLE:
#endif
            nDouble = CallDBL(pProc, pStack, (short)nSize );
            rResult.PutDouble( nDouble );
            break;

        case SbxDATE:
            nDouble = CallDBL(pProc, pStack, (short)nSize );
            rResult.PutDate( nDouble );
            break;

        case SbxCHAR:
        case SbxBYTE:
        case SbxBOOL:
            nInt16 = CallINT(pProc, pStack, (short)nSize );
            rResult.PutByte( (BYTE)nInt16 );
            break;

        case SbxSTRING:
        case SbxLPSTR:
            pStr = CallSTR(pProc, pStack, (short)nSize );
            rResult.PutString( String::CreateFromAscii( pStr ) );
            break;

        case SbxNULL:
        case SbxEMPTY:
            nInt16 = CallINT(pProc, pStack, (short)nSize );
            // Rueckgabe nur zulaessig, wenn variant!
            if( !rResult.IsFixed() )
                rResult.PutInteger( nInt16 );
            break;

        case SbxCURRENCY:
        case SbxOBJECT:
        case SbxDATAOBJECT:
        default:
            CallINT(pProc, pStack, (short)nSize );
            break;
    }
    delete pStack;

    if( pArgs )
    {
        // die Laengen aller uebergebenen Strings anpassen
        USHORT nCount = pArgs->Count();
        for( USHORT nCur = 1; nCur < nCount; nCur++ )
        {
            SbxVariable* pVar = pArgs->Get( nCur );
            BOOL bIsString = ( pVar->GetType() == SbxSTRING ) ||
                             ( pVar->GetType() == SbxLPSTR  );

            if( pVar->GetFlags() & SBX_REFERENCE )
            {
                pVar->ResetFlag( SBX_REFERENCE );   // Sbx moechte es so
                if( bIsString )
                {
                    ByteString aByteStr( (char*)pVar->GetUserData() );
                    String aStr( aByteStr, gsl_getSystemTextEncoding() );
                    pVar->PutString( aStr );
                }
            }
            if( bIsString )
            {
                delete (char*)(pVar->GetUserData());
                pVar->SetUserData( 0 );
            }
        }
    }
    return 0;
}

SbError SbiDllMgr::CallProcC( SbiDllProc pProc, SbxArray* pArgs,
    SbxVariable& rResult )
{
    DBG_ERROR("C calling convention not supported");
    return (USHORT)SbERR_BAD_ARGUMENT;
}

void* SbiDllMgr::CreateStack( SbxArray* pArgs, USHORT& rSize )
{
    if( !pArgs )
    {
        rSize = 0;
        return 0;
    }
    char* pStack = new char[ 2048 ];
    char* pTop = pStack;
    USHORT nCount = pArgs->Count();
    // erstes Element ueberspringen
#ifndef WIN
    for( USHORT nCur = 1; nCur < nCount; nCur++ )
#else
    // unter 16-Bit Windows anders rum (OS/2 ?????)
    for( USHORT nCur = nCount-1; nCur >= 1; nCur-- )
#endif
    {
        SbxVariable* pVar = pArgs->Get( nCur );
        // AB 22.1.1996, Referenz
        if( pVar->GetFlags() & SBX_REFERENCE )  // Es ist eine Referenz
        {
            switch( pVar->GetType() )
            {
                case SbxINTEGER:
                case SbxUINT:
                case SbxINT:
                case SbxUSHORT:
                case SbxLONG:
                case SbxULONG:
                case SbxSINGLE:
                case SbxDOUBLE:
                case SbxCHAR:
                case SbxBYTE:
                case SbxBOOL:
                    *((void**)pTop) = (void*)&(pVar->aData);
                    pTop += sizeof( void* );
                    break;

                case SbxSTRING:
                case SbxLPSTR:
                    {
                    USHORT nLen = 256;
                    ByteString rStr( pVar->GetString(), gsl_getSystemTextEncoding() );
                    if( rStr.Len() > 255 )
                        nLen = rStr.Len() + 1;

                    char* pStr = new char[ nLen ];
                    strcpy( pStr, rStr.GetBuffer() );
                    // ist nicht so sauber, aber wir sparen ein Pointerarray
                    DBG_ASSERT(sizeof(UINT32)>=sizeof(char*),"Gleich krachts im Basic");
                    pVar->SetUserData( (UINT32)pStr );
                    *((const char**)pTop) = pStr;
                    pTop += sizeof( char* );
                    }
                    break;

                case SbxNULL:
                case SbxEMPTY:
                case SbxERROR:
                case SbxDATE:
                case SbxCURRENCY:
                case SbxOBJECT:
                case SbxDATAOBJECT:
                default:
                    break;
            }
        }
        else
        {
            // ByVal
            switch( pVar->GetType() )
            {
                case SbxINTEGER:
                case SbxUINT:
                case SbxINT:
                case SbxUSHORT:
                    *((INT16*)pTop) = pVar->GetInteger();
                    pTop += sizeof( INT16 );
                    break;

                case SbxLONG:
                case SbxULONG:
                    *((INT32*)pTop) = pVar->GetLong();
                    pTop += sizeof( INT32 );
                    break;

                case SbxSINGLE:
                    *((float*)pTop) = pVar->GetSingle();
                    pTop += sizeof( float );
                    break;

                case SbxDOUBLE:
                    *((double*)pTop) = pVar->GetDouble();
                    pTop += sizeof( double );
                    break;

                case SbxSTRING:
                case SbxLPSTR:
                    {
                    char* pStr = new char[ pVar->GetString().Len() + 1 ];
                    ByteString aByteStr( pVar->GetString(), gsl_getSystemTextEncoding() );
                    strcpy( pStr, aByteStr.GetBuffer() );
                    // ist nicht so sauber, aber wir sparen ein Pointerarray
                    DBG_ASSERT(sizeof(UINT32)>=sizeof(char*),"Gleich krachts im Basic");
                    pVar->SetUserData( (UINT32)pStr );
                    *((const char**)pTop) = pStr;
                    pTop += sizeof( char* );
                    }
                    break;

                case SbxCHAR:
                case SbxBYTE:
                case SbxBOOL:
                    *((BYTE*)pTop) = pVar->GetByte();
                    pTop += sizeof( BYTE );
                    break;

                case SbxNULL:
                case SbxEMPTY:
                case SbxERROR:
                case SbxDATE:
                case SbxCURRENCY:
                case SbxOBJECT:
                case SbxDATAOBJECT:
                default:
                    break;
            }
        }
    }
    rSize = (USHORT)((ULONG)pTop - (ULONG)pStack);
    return pStack;
}




