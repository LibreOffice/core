/*************************************************************************
 *
 *  $RCSfile: reginfo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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
#ifdef MAC
#include "mac_start.h"

#ifndef __TYPES__
  #include <Types.h>
#endif

#ifndef __RESOURCES__
  #include <Resources.h>
#endif

#ifndef __FILES__
  #include <Files.h>
#endif

#ifndef __ERRORS__
  #include <Errors.h>
#endif

#ifndef __FOLDERS__
  #include <Folders.h>
#endif

#ifndef __SCRIPT__
  #include <script.h>
#endif

#ifndef __FINDER__
  #include <finder.h>
#endif
#include "mac_end.h"
#endif

#include "reginfo.hxx"

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

String aEmptyString;

#define MAXREGVALUE 200

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
    DBG_HDL aEmptyString;
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
    DBG_HDL aEmptyString;
    String aRes;
    if(ReadKey_Impl( rKey, pImp->aGroupHdl, aRes))
        return aRes;
    else
        return aEmptyString;
}

String  RegInfo::ReadKey( const String& rKey, const String &rDefault ) const
{
    DBG_HDL aEmptyString;
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

#elif defined(MAC)

/*  Idee:

    In eine Resource SVðð (SV<Apfel><Apfel>) sind die Resource IDs
    der Gruppen Ÿber den Namen der Resource zugŠnglich.

    †ber die aktuelle Gruppe sind die Values Ÿber den Namen der Resource
    als Key zugŠnglich ...
*/

// Wir tuen so, als wŠren es "normale" SV-Resourcen

#define     kAPPInfo 'SVðð'

//****************************************************

#pragma options align=mac68k

typedef struct
{
    OSType nResType;    //  kAPPInfo + k
}
    SVGroupInfo, *SVGroupInfoPtr, **SVGroupInfoHdl;

//****************************************************

typedef struct
{
    USHORT nValueLen;
    char   aValue[1];   // LŠnge wie in nLen
}
    SVKeyInfo, *SVKeyInfoPtr, **SVKeyInfoHdl;

//****************************************************

#pragma options align=reset

//****************************************************

struct RegInfo_Impl
{
    short   nResFile;
    OSType  nCurrentGroup;
    String  aCurrentName;

    SVGroupInfoHdl  GetNamedInfo(const String& rName);

    BOOL            FindInfoFile( const String& rName, FSSpec *pResult );
};

//****************************************************

SVGroupInfoHdl RegInfo_Impl::GetNamedInfo(const String& rName)
{
    String aName(rName);
    short nOldRes = CurResFile();
    UseResFile(nResFile);

    SVGroupInfoHdl hResult = (SVGroupInfoHdl) Get1NamedResource(kAPPInfo,rName.GetPascalStr());

    UseResFile(nOldRes);

    return hResult;
}

//****************************************************

BOOL RegInfo_Impl::FindInfoFile( const String& rName, FSSpec *pResult )
{
    OSErr   nErr;
    long    nDirID;
    short   nVRefNum;
    BOOL    bResult = FALSE;

    nErr = FindFolder( kOnSystemDisk, kPreferencesFolderType,
                       kCreateFolder, &nVRefNum, &nDirID );

    if ( nErr == noErr )
    {
        nErr = FSMakeFSSpec( nVRefNum, nDirID, rName.GetPascalStr(), pResult );
        if ( nErr == fnfErr )
        {
            FSpCreateResFile( pResult, 'SDsv', 'DATA', smRoman );
            nErr = FSMakeFSSpec( nVRefNum, nDirID, rName.GetPascalStr(), pResult );
        }
        if ( nErr == noErr )
        {
            FInfo aInfo;
            nErr = FSpGetFInfo( pResult, &aInfo );
            if ( ( nErr == noErr ) && ! ( aInfo.fdFlags & kIsInvisible ) )
            {
                aInfo.fdFlags |= kIsInvisible;
                nErr = FSpSetFInfo( pResult, &aInfo );
            }
            bResult = TRUE;
        }
    }

    return bResult;
}

//****************************************************

RegInfo::RegInfo()
{
    FSSpec  aFile;

    if ( !pImp->FindInfoFile( "svdbt.dll", &aFile ) )
    {
        pImp = NULL;
        return;
    }

    pImp=new RegInfo_Impl;

    short nOldRes = CurResFile();
    pImp->nResFile = FSpOpenResFile( &aFile, fsRdWrPerm );
    pImp->nCurrentGroup = 0L;
    UseResFile( nOldRes );

    if ( pImp->nResFile < 0 )
    {
        delete pImp;
        pImp = NULL;
    }
}

//****************************************************

RegInfo::~RegInfo()
{
    if (pImp)
    {
        CloseResFile( pImp->nResFile );
        delete pImp;
    }
}

//****************************************************

String RegInfo::GetKeyName( USHORT nKey ) const
{
    String aResult;

    if (!pImp || !pImp -> nCurrentGroup)
        return aResult;

    short nOldRes = CurResFile();
    UseResFile(pImp -> nResFile);

    SVKeyInfoHdl hKeyInfo = (SVKeyInfoHdl) Get1IndResource(pImp -> nCurrentGroup,nKey + 1);

    if (hKeyInfo)
    {
        Str255  aName;
        ResType nType;
        short   nMacKey;

        GetResInfo((Handle) hKeyInfo,&nMacKey,&nType,aName);
        if (ResError() == noErr)
            aResult = String((char*) &aName[1], aName[0]);
    }

    UseResFile(nOldRes);
    return aResult;
}

//****************************************************

USHORT RegInfo::GetKeyCount() const
{
    USHORT nResult = 0;

    if (pImp -> nCurrentGroup)
    {
        short nOldRes = CurResFile();
        UseResFile(pImp -> nResFile);

        nResult = Count1Resources(pImp -> nCurrentGroup);
        UseResFile(nOldRes);
    }
    return nResult;
}

//****************************************************

void RegInfo::SetAppGroup( const String& rGroup )
{
    if (!pImp)
        return;

    pImp->aCurrentName = rGroup;
    SVGroupInfoHdl hGroup = pImp -> GetNamedInfo(rGroup);
    if (hGroup)
    {
        pImp->nCurrentGroup = (*hGroup)-> nResType;
        ReleaseResource((Handle) hGroup);
    }
    else
        pImp->nCurrentGroup = 0L;
}

//****************************************************

void RegInfo::DeleteAppGroup( const String &rGroup )
{
    SVGroupInfoHdl hToDelete = pImp -> GetNamedInfo(rGroup);
    if (hToDelete)
    {
        if ((*hToDelete)-> nResType == pImp->nCurrentGroup)
        {
            pImp->nCurrentGroup = 0L;
            pImp->aCurrentName.Erase();
        }
        RemoveResource((Handle) hToDelete);
    }
}

//****************************************************

String  RegInfo::ReadKey( const String& rKey ) const
{
    String aResult;

    if (!pImp || !pImp -> nCurrentGroup)
        return aResult;

    short nOldRes = CurResFile();
    UseResFile(pImp -> nResFile);

    SVKeyInfoHdl hKeyInfo = (SVKeyInfoHdl) Get1NamedResource(pImp->nCurrentGroup,rKey.GetPascalStr());

    if (hKeyInfo)
    {
        HLock((Handle) hKeyInfo);
        aResult = String((*hKeyInfo)->aValue, (*hKeyInfo)->nValueLen);
        HUnlock((Handle) hKeyInfo);

        ReleaseResource((Handle) hKeyInfo);
    }

    UseResFile(nOldRes);
    return aResult;
}

//****************************************************

String  RegInfo::ReadKey( const String& rKey, const String &rDefault ) const
{
    String aResult = ReadKey(rKey);
    if (!aResult.Len())
        return rDefault;
    else
        return aResult;
}

//****************************************************

void RegInfo::WriteKey( const String& rKey, const String& rValue )
{
    if (!pImp)
        return;

    short nOldRes = CurResFile();
    UseResFile(pImp -> nResFile);

    // Wenn wir noch keine Resource zur aktuellen Gruppe haben ...
    if (!pImp -> nCurrentGroup)
    {
        // ... dann muessen wir hier eine Anlegen

        if (!pImp -> aCurrentName.Len())
            return; // Kein aktueller Name ???

        short nNewId = Unique1ID(kAPPInfo);

        SVGroupInfoHdl hNewInfo = (SVGroupInfoHdl) NewHandle(sizeof(SVGroupInfo));

        // Die Neue Resource liegt im Bereich Ÿber "SVðð"
        pImp -> nCurrentGroup =  kAPPInfo + nNewId;

        (*(hNewInfo))-> nResType = pImp -> nCurrentGroup;
        DBG_ASSERT(CurResFile() == pImp -> nResFile,"wrong ResFile");
        AddResource((Handle) hNewInfo ,kAPPInfo, nNewId, pImp -> aCurrentName.GetPascalStr());
        ReleaseResource((Handle) hNewInfo);
    }

    SVKeyInfoHdl hKeyInfo = (SVKeyInfoHdl) Get1NamedResource(pImp -> nCurrentGroup,rKey.GetPascalStr());
    int nNewSize = rValue.Len() + sizeof(USHORT);

    // Haben wir zu diesem Key schon eine Resource ?
    if (hKeyInfo)
    {
        int nSize = GetHandleSize((Handle) hKeyInfo);
        OSErr nMemErr = noErr;
        if (nSize < nNewSize)
        {
            SetHandleSize((Handle) hKeyInfo,nNewSize);
            nMemErr = MemError();
        }

        if (nMemErr == noErr)
        {
            (*hKeyInfo)->nValueLen = rValue.Len();
            BlockMoveData(rValue.GetStr(), (*hKeyInfo)->aValue, rValue.Len());
            ChangedResource((Handle) hKeyInfo);
        }
        else    // Handle konnte nicht groesser werden
            RemoveResource((Handle) hKeyInfo);
    }
    else
    {
        // Resource zu diesemm Key neu anlegen

        hKeyInfo = (SVKeyInfoHdl) NewHandle(nNewSize);
        if (hKeyInfo)
        {
            short nNewId = Unique1ID(pImp -> nCurrentGroup);

            (*hKeyInfo)->nValueLen = rValue.Len();
            BlockMoveData(rValue.GetStr(), (*hKeyInfo)->aValue, rValue.Len());

            AddResource((Handle) hKeyInfo ,pImp -> nCurrentGroup, nNewId, rKey.GetPascalStr());
        }
    }

    UseResFile(nOldRes);
}

//****************************************************

void RegInfo::DeleteKey( const String& rKey )
{
    // Wenn wir noch keine  aktuellen Gruppe haben ...
    if (!pImp -> nCurrentGroup)
        return;

    String aKey(rKey);

    short nOldRes = CurResFile();
    UseResFile(pImp -> nResFile);

    SVKeyInfoHdl hKeyInfo = (SVKeyInfoHdl) Get1NamedResource(pImp -> nCurrentGroup,rKey.GetPascalStr());

    if (hKeyInfo)
        RemoveResource((Handle) hKeyInfo);

    UseResFile(nOldRes);
}

//****************************************************

#else

RegInfo::RegInfo()
{
}


RegInfo::~RegInfo()
{
}

String RegInfo::GetKeyName( USHORT nKey ) const
{
    return aEmptyString;
}

USHORT RegInfo::GetKeyCount() const
{
    return 0;
}

void RegInfo::SetAppGroup( const String& rGroup )
{
    return ;
}

void RegInfo::DeleteAppGroup( const String &rGroup )
{
    return;
}

String  RegInfo::ReadKey( const String& rKey ) const
{
    return aEmptyString;
}

String  RegInfo::ReadKey( const String& rKey, const String &rDefault ) const
{
    return aEmptyString;
}

void RegInfo::WriteKey( const String& rKey, const String& rValue )
{
    return;
}

void RegInfo::DeleteKey( const String& rKey )
{
    return;
}

#endif
