/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reginfo.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 08:53:24 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "reginfo.hxx"

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#define MAXREGVALUE 200

// *****************************************************************************
#if defined(WIN) || defined(WNT)

#include <tools/svwin.h>

#define DBG_HDL DBG_ASSERT(pImp->bValidGroup, "Keine Gruppe gesetzt"); \
    if( !pImp->bValidGroup ) return

struct RegInfo_Impl
{
    HKEY aGroupHdl;
    BOOL bValidGroup;
};

RegInfo::RegInfo()
{
    pImp=new RegInfo_Impl;
    pImp->bValidGroup = FALSE;
}

RegInfo::~RegInfo()
{
    if(pImp->bValidGroup)
        RegCloseKey( pImp->aGroupHdl );
    delete pImp;
}

String RegInfo::GetKeyName( USHORT nKey ) const
{
    DBG_HDL String::EmptyString();
    char aBuffer[MAXREGVALUE];
    RegEnumKey( pImp->aGroupHdl, nKey, aBuffer, MAXREGVALUE );
    return String( UniString::CreateFromAscii(aBuffer) );
}

USHORT RegInfo::GetKeyCount() const
{
    DBG_HDL 0;
#ifdef WNT
    DWORD nKeys;
    DWORD Dum1=10, Dum2, Dum3, Dum4, Dum5, Dum6, Dum7;
    char s[10];
    FILETIME aDumFileTime;
    RegQueryInfoKey( pImp->aGroupHdl, s, &Dum1, 0, &nKeys, &Dum2, &Dum3,
                    &Dum4, &Dum5, &Dum6, &Dum7, &aDumFileTime );
    return (USHORT) nKeys;
#else
    char aBuffer[MAXREGVALUE];
    USHORT n=0;
    while(RegEnumKey(
        pImp->aGroupHdl, n, aBuffer, MAXREGVALUE) == ERROR_SUCCESS)
        n++;
    return n;
#endif
}

inline String MakeAppGroupString_Impl( const String &rGroup )
{
    String aGroup( UniString::CreateFromAscii("SvAppGroups\\") );
    aGroup+=rGroup;
    return aGroup;
}

void RegInfo::SetAppGroup( const String& rGroup )
{
    aCurrentGroup = MakeAppGroupString_Impl(rGroup);
    if( pImp->bValidGroup )
    {
        RegCloseKey( pImp->aGroupHdl );
        pImp->bValidGroup = FALSE;
    }
    ByteString aBStr( aCurrentGroup, osl_getThreadTextEncoding() );
    RegCreateKey( HKEY_CLASSES_ROOT, aBStr.GetBuffer(), &pImp->aGroupHdl );
    pImp->bValidGroup = TRUE;
}

void RegInfo::DeleteAppGroup( const String &rGroup )
{
    String aOldGroup = aCurrentGroup;
    SetAppGroup( rGroup );
    DBG_HDL;
    USHORT nMax = GetKeyCount();
    for( USHORT n = nMax; n--; )
    {
        String aKey( GetKeyName( n ));
        DeleteKey( aKey );
    }
    RegCloseKey( pImp->aGroupHdl );

    ByteString aBStr( rGroup, osl_getThreadTextEncoding() );
    RegDeleteKey( HKEY_CLASSES_ROOT, aBStr.GetBuffer() );
    pImp->bValidGroup = FALSE;
    if( rGroup != aOldGroup )
        SetAppGroup( aOldGroup );
}

BOOL ReadKey_Impl( const String& rKey,
                  HKEY aHdl, String& rResult )
{
    char s[MAXREGVALUE];
    LONG aLen=MAXREGVALUE;

    ByteString aBStr( rKey, osl_getThreadTextEncoding() );
    LONG nRes = RegQueryValue( aHdl, aBStr.GetBuffer(), s, &aLen);
    if(nRes == ERROR_SUCCESS)
    {
        rResult = UniString::CreateFromAscii(s);
        return TRUE;
    }
    else
        return FALSE;
}

String  RegInfo::ReadKey( const String& rKey ) const
{
    DBG_HDL String::EmptyString();
    String aRes;
    if(ReadKey_Impl( rKey, pImp->aGroupHdl, aRes))
        return aRes;
    else
        return String::EmptyString();
}

String  RegInfo::ReadKey( const String& rKey, const String &rDefault ) const
{
    DBG_HDL String::EmptyString();
    String aRes;
    if(ReadKey_Impl( rKey, pImp->aGroupHdl, aRes))
        return aRes;
    else
        return rDefault;
}

void RegInfo::WriteKey( const String& rKey, const String& rValue )
{
    DBG_HDL;
    ByteString aBStr( rKey, osl_getThreadTextEncoding() );
    ByteString aBStr1( rValue, osl_getThreadTextEncoding() );
    RegSetValue( pImp->aGroupHdl, aBStr.GetBuffer(), REG_SZ, aBStr1.GetBuffer(), 0);
}


void RegInfo::DeleteKey( const String& rKey )
{
    DBG_HDL;
    ByteString aBStr( rKey, osl_getThreadTextEncoding() );
    RegDeleteKey( pImp->aGroupHdl, aBStr.GetBuffer() );
}

// *****************************************************************************
#elif defined(OS2)

#define INCL_WINSHELLDATA
#include <tools/svpm.h>

struct RegInfo_Impl
{
    char *pKeyList;
    String aCurrentApp;
    void BuildKeyList( const String &rGroup );
};

void RegInfo_Impl::BuildKeyList( const String &rGroup )
{
    USHORT nLen = 0;
    do
    {
        nLen+=1000;
        delete[] pKeyList;
        pKeyList = new char[nLen];
        *(int *)pKeyList = 0;
    }
    while( PrfQueryProfileString(
        HINI_USERPROFILE, rGroup,
        0, 0, pKeyList, nLen) == nLen);
}


RegInfo::RegInfo()
{
    pImp=new RegInfo_Impl;
    pImp->pKeyList = 0;
}

RegInfo::~RegInfo()
{
    delete[] pImp->pKeyList;
    delete pImp;
}

inline String MakeAppGroupString_Impl( const String &rGroup )
{
    String aGroup("SvAppGroups:");
    aGroup+=rGroup;
    return aGroup;
}

String RegInfo::GetKeyName( USHORT nKey ) const
{
    if( !pImp->pKeyList )
        pImp->BuildKeyList(pImp->aCurrentApp);

    const char *pc=pImp->pKeyList;
    for( USHORT n=0; n<nKey; n++ )
        while(*pc++);

    return String(pc);
}

USHORT RegInfo::GetKeyCount() const
{
    if( !pImp->pKeyList )
        pImp->BuildKeyList( pImp->aCurrentApp);

    const char *pc=pImp->pKeyList;
    USHORT nRet=0;
    while(*pc)
    {
        while(*pc++);
        nRet++;
    }
    return nRet;
}

void RegInfo::SetAppGroup( const String& rGroup )
{
    delete[] pImp->pKeyList;
    pImp->pKeyList = 0;
    aCurrentGroup = rGroup;
    pImp->aCurrentApp = MakeAppGroupString_Impl( rGroup );
}

void RegInfo::DeleteAppGroup( const String &rGroup )
{
    PrfWriteProfileString(
        HINI_USERPROFILE, MakeAppGroupString_Impl( rGroup ), 0, 0);
}


String  RegInfo::ReadKey( const String& rKey ) const
{
    char *pBuffer= new char[MAXREGVALUE];
    *pBuffer=0;
    PrfQueryProfileString(
        HINI_USERPROFILE, pImp->aCurrentApp, rKey, 0, pBuffer, MAXREGVALUE);
    String aRet(pBuffer);
    delete[] pBuffer;
    return aRet;
}


String  RegInfo::ReadKey( const String& rKey, const String &rDefault ) const
{
    char *pBuffer= new char[MAXREGVALUE];
    *pBuffer=0;
    PrfQueryProfileString(
        HINI_USERPROFILE, pImp->aCurrentApp, rKey,  rDefault, pBuffer, MAXREGVALUE);
    String aRet(pBuffer);
    delete[] pBuffer;
    return aRet;
}


void RegInfo::WriteKey( const String& rKey, const String& rValue )
{
    PrfWriteProfileString(
        HINI_USERPROFILE, pImp->aCurrentApp, rKey, rValue);
}

void RegInfo::DeleteKey( const String& rKey )
{
    PrfWriteProfileString(
        HINI_USERPROFILE, pImp->aCurrentApp, rKey, 0);
}

// *****************************************************************************

#else

RegInfo::RegInfo()
{
}


RegInfo::~RegInfo()
{
}

String RegInfo::GetKeyName( USHORT ) const
{
    return String::EmptyString();
}

USHORT RegInfo::GetKeyCount() const
{
    return 0;
}

void RegInfo::SetAppGroup( const String& )
{
    return ;
}

void RegInfo::DeleteAppGroup( const String& )
{
    return;
}

String  RegInfo::ReadKey( const String& ) const
{
    return String::EmptyString();
}

String  RegInfo::ReadKey( const String&, const String& ) const
{
    return String::EmptyString();
}

void RegInfo::WriteKey( const String&, const String& )
{
    return;
}

void RegInfo::DeleteKey( const String& )
{
    return;
}

#endif
