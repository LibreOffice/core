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
#include <stdlib.h>
#include <stdio.h>
#include <soldep/sstring.hxx>
#include <vos/mutex.hxx>

#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <tools/stream.hxx>
#include <tools/geninfo.hxx>
#include <soldep/prj.hxx>
//#include <bootstrp/inimgr.hxx>

#ifndef MACOSX
#pragma hdrstop
#endif

//#define TEST  1

#if defined(WNT) || defined(OS2)
#define LIST_DELIMETER ';'
#else
#ifdef UNX
#define LIST_DELIMETER ':'
#else
#endif
#endif

#if defined(WNT) || defined(OS2)
#define PATH_DELIMETER '\\'
#else
#ifdef UNX
#define PATH_DELIMETER '/'
#else
#endif
#endif

//static const char * XML_ALL  =    "all";

//
//  class SimpleConfig
//

/*****************************************************************************/
SimpleConfig::SimpleConfig( String aSimpleConfigFileName )
/*****************************************************************************/
{
    nLine = 0;
    aFileName = aSimpleConfigFileName;
    aFileStream.Open ( aFileName, STREAM_READ );
}

/*****************************************************************************/
SimpleConfig::SimpleConfig( DirEntry& rDirEntry )
/*****************************************************************************/
{
    nLine = 0;
    aFileName = rDirEntry.GetFull();
    aFileStream.Open ( aFileName, STREAM_READ );
}

/*****************************************************************************/
SimpleConfig::~SimpleConfig()
/*****************************************************************************/
{
    aFileStream.Close ();
}

/*****************************************************************************/
ByteString SimpleConfig::GetNext()
/*****************************************************************************/
{
    ByteString aString;

    if ( aStringBuffer =="" )
      while ((aStringBuffer = GetNextLine()) == "\t"); //solange bis != "\t"
    if ( aStringBuffer =="" )
        return ByteString();

    aString = aStringBuffer.GetToken(0,'\t');
    aStringBuffer.Erase(0, aString.Len()+1);

    aStringBuffer.EraseLeadingChars( '\t' );

    return aString;
}

/*****************************************************************************/
ByteString  SimpleConfig::GetNextLine()
/*****************************************************************************/
{
    ByteString aSecStr;
    sal_Bool bStreamOk;
//  sal_uInt16 iret = 0;
    nLine++;

    bStreamOk = aFileStream.ReadLine ( aTmpStr );
    if ( (aTmpStr.Search( "#" ) == 0) )
        return "\t";
    aTmpStr = aTmpStr.EraseLeadingChars();
    aTmpStr = aTmpStr.EraseTrailingChars();
    while ( aTmpStr.SearchAndReplace(ByteString(' '),ByteString('\t') ) != STRING_NOTFOUND );
    int nLength = aTmpStr.Len();
    if ( bStreamOk && (nLength == 0) )
        return "\t";
//  sal_uInt16 nPos = 0;
    sal_Bool bFound = sal_False;
    ByteString aEraseString;
    for ( sal_uInt16 i = 0; i<= nLength; i++)
    {
        if ( aTmpStr.GetChar( i ) == 0x20  && !bFound )
            aTmpStr.SetChar( i, 0x09 );
    }
    return aTmpStr;
}

/*****************************************************************************/
ByteString SimpleConfig::GetCleanedNextLine( sal_Bool bReadComments )
/*****************************************************************************/
{
    sal_Bool bStreamOk;
    sal_Bool bReadNextLine = sal_True;
    while (bReadNextLine)
    {
        bStreamOk = aFileStream.ReadLine ( aTmpStr );
        if (!bStreamOk)
            return ByteString();

        ByteString sTab = "\t";
        ByteString sDoubleTab = "\t\t";
        ByteString sSpace = " ";
        xub_StrLen nIndex = 0;

        aTmpStr.SearchAndReplaceAll(sSpace, sTab);
        while ( (nIndex = aTmpStr.SearchAndReplace(sDoubleTab, sTab, nIndex )) != STRING_NOTFOUND );

        aTmpStr = aTmpStr.EraseLeadingAndTrailingChars('\t'); // remove tabs

        if ( aTmpStr.Search( "#" ) == 0 )
        {
            if (bReadComments )
                return aTmpStr;
        }
        else if (aTmpStr != ByteString::EmptyString())
            bReadNextLine = sal_False;
    }

    return aTmpStr;
}


//
//  class CommandData
//

/*****************************************************************************/
CommandData::CommandData()
/*****************************************************************************/
{
    nOSType = 0;
    nCommand = 0;
    pDepList = 0;
    pCommandList = 0;
}

/*****************************************************************************/
CommandData::~CommandData()
/*****************************************************************************/
{
    if ( pDepList )
    {
        ByteString *pString = pDepList->First();
        while ( pString )
        {
            delete pString;
            pString = pDepList->Next();
        }
        delete pDepList;

        pDepList = NULL;
    }
    if ( pCommandList )
    {
        ByteString *pString = pCommandList->First();
        while ( pString )
        {
            delete pString;
            pString = pCommandList->Next();
        }
        delete pCommandList;

        pCommandList = NULL;
    }
}

/*****************************************************************************/
ByteString CommandData::GetOSTypeString()
/*****************************************************************************/
{
    ByteString aRetStr;

    switch (nOSType)
    {
        case OS_WIN16 | OS_WIN32 | OS_OS2 | OS_UNX | OS_MAC :
            aRetStr = "all";
            break;
        case OS_WIN32 | OS_WIN16 :
            aRetStr = "w";
            break;
        case OS_OS2 :
            aRetStr = "p";
            break;
        case OS_UNX :
            aRetStr = "u";
            break;
        case OS_WIN16 :
            aRetStr = "d";
            break;
        case OS_WIN32 :
            aRetStr = "n";
            break;
        case OS_MAC :
            aRetStr = "m";
            break;
        default :
            aRetStr = "none";
    }

    return aRetStr;
}

/*****************************************************************************/
ByteString CommandData::GetCommandTypeString()
/*****************************************************************************/
{
    ByteString aRetStr;

    switch (nCommand)
    {
        case COMMAND_NMAKE :
            aRetStr = "nmake";
            break;
        case COMMAND_GET :
            aRetStr = "get";
            break;
        default :
            aRetStr = "usr";
            aRetStr += ByteString::CreateFromInt64( nCommand + 1 - COMMAND_USER_START );

    }

    return aRetStr;
}

/*****************************************************************************/
void CommandData::AddCommand(ByteString* pCommand)
/*****************************************************************************/
{
    if (!pCommandList)
        pCommandList = new SByteStringList();
    pCommandList->Insert(pCommand, LIST_APPEND);
}

/*****************************************************************************/
CommandData& CommandData::operator>>  ( SvStream& rStream )
/*****************************************************************************/
{
    rStream << aPrj;
    rStream << aLogFileName;
    rStream << aInpath;
    rStream << aUpd;
    rStream << aUpdMinor;
    rStream << aProduct;
    rStream << aCommand;
    rStream << aPath;
    rStream << aPrePath;
    rStream << aPreFix;
    rStream << aCommandPara;
    rStream << aComment;
    rStream << sClientRestriction;

    rStream << nOSType;
    rStream << nCommand;
    rStream << nDepth;

    if (pDepList)
    {
        rStream << sal_True;
        *pDepList >> rStream;
    }
    else
        rStream << sal_False;

    if (pCommandList)
    {
        rStream << sal_True;
        *pCommandList >> rStream;
    }
    else
        rStream << sal_False;

    return *this;
}

/*****************************************************************************/
CommandData& CommandData::operator<<  ( SvStream& rStream )
/*****************************************************************************/
{
    rStream >> aPrj;
    rStream >> aLogFileName;
    rStream >> aInpath;
    rStream >> aUpd;
    rStream >> aUpdMinor;
    rStream >> aProduct;
    rStream >> aCommand;
    rStream >> aPath;
    rStream >> aPrePath;
    rStream >> aPreFix;
    rStream >> aCommandPara;
    rStream >> aComment;
    rStream >> sClientRestriction;

    rStream >> nOSType;
    rStream >> nCommand;
    rStream >> nDepth;

    sal_Bool bDepList;
    rStream >> bDepList;
    if (pDepList)
        pDepList->CleanUp();
    if (bDepList)
    {
        if (!pDepList)
            pDepList = new SByteStringList();
        *pDepList << rStream;
    }
    else
    {
        if (pDepList)
        DELETEZ (pDepList);
    }

    sal_Bool bCommandList;
    rStream >> bCommandList;
    if (pCommandList)
        pCommandList->CleanUp();
    if (bCommandList)
    {
        if (!pCommandList)
            pCommandList = new SByteStringList();
        *pCommandList << rStream;
    }
    else
    {
        if (pCommandList)
            DELETEZ (pCommandList);
    }

    return *this;
}



//
// class DepInfo
//

/*****************************************************************************/
DepInfo::~DepInfo()
/*****************************************************************************/
{
    RemoveProject();

    if ( pModeList )
    {
        ByteString *pString = pModeList->First();
        while ( pString )
        {
            delete pString;
            pString = pModeList->Next();
        }
        delete pModeList;

        pModeList = NULL;
    }
}

/*****************************************************************************/
void DepInfo::SetProject (ByteString* pStr)
/*****************************************************************************/
{
    RemoveProject();
    pProject = pStr;
}

/*****************************************************************************/
void DepInfo::RemoveProject ()
/*****************************************************************************/
{
    if (pProject)
    {
        delete pProject;
        pProject = NULL;
    }
}

/*****************************************************************************/
DepInfo& DepInfo::operator<<  ( SvStream& rStream )
/*****************************************************************************/
{
    RemoveProject();
    pProject = new ByteString();
    rStream >> *pProject;

    sal_Bool bModeList;
    rStream >> bModeList;
    if (pModeList)
        pModeList->CleanUp();
    if (bModeList)
    {
        if (!pModeList)
            pModeList = new SByteStringList();
        *pModeList << rStream;
    }
    else
        DELETEZ (pModeList);

    rStream >> bAllModes;
    return *this;
}

/*****************************************************************************/
DepInfo& DepInfo::operator>>  ( SvStream& rStream )
/*****************************************************************************/
{
    rStream << *pProject;
    if (pModeList)
    {
        rStream << sal_True;
        *pModeList >> rStream;
    }
    else
        rStream << sal_False;
    rStream << bAllModes;

    return *this;
}

//
// class SDepInfoList
//

/*****************************************************************************/
SDepInfoList::SDepInfoList()
/*****************************************************************************/
                                : pAllModeList(0)
{
}

/*****************************************************************************/
SDepInfoList::~SDepInfoList()
/*****************************************************************************/
{
    if (pAllModeList)
        delete pAllModeList;
}

/*****************************************************************************/
sal_uIntPtr SDepInfoList::IsString( ByteString* pStr )
/*****************************************************************************/
{
    sal_uIntPtr nRet = NOT_THERE;
    if ( (nRet = GetPrevString( pStr )) != 0 )
    {
        ByteString* pString = GetObject( nRet )->GetProject();
        if ( *pString == *pStr )
            return nRet;
        else
            return NOT_THERE;
    }
    else
    {
        ByteString* pString = GetObject( 0 )->GetProject();
        if ( pString && (*pString == *pStr) )
            return 0;
        else
            return NOT_THERE;
    }
    return nRet;
}

/*****************************************************************************/
sal_uIntPtr SDepInfoList::GetPrevString( ByteString* pStr )
/*****************************************************************************/
{
    sal_uIntPtr nRet = 0;
    sal_Bool bFound = sal_False;
    sal_uIntPtr nCount_l = Count();
    sal_uIntPtr nUpper = nCount_l;
    sal_uIntPtr nLower = 0;
    sal_uIntPtr nCurrent = nUpper / 2;
    sal_uIntPtr nRem = 0;
    ByteString* pString;

    do
    {
        if ( (nCurrent == nLower) || (nCurrent == nUpper) )
            return nLower;
        pString = GetObject( nCurrent )->GetProject();
        int nResult =  pStr->CompareTo( *pString );
        if ( nResult == COMPARE_LESS )
        {
            nUpper = nCurrent;
            nCurrent = (nCurrent + nLower) /2;
        }
        else if ( nResult == COMPARE_GREATER )
        {
            nLower = nCurrent;
            nCurrent = (nUpper + nCurrent) /2;
        }
        else if ( nResult == COMPARE_EQUAL )
            return nCurrent;
        if ( nRem == nCurrent )
            return nCurrent;
        nRem = nCurrent;
    }
    while ( !bFound );
    return nRet;
}

/*****************************************************************************/
void SDepInfoList::PutModeString( DepInfo* pInfoList, ByteString* pStr )
/*****************************************************************************/
{
    SByteStringList* pList = pInfoList->GetModeList();
    if (!pList)
    {
        pList = new SByteStringList;
        pInfoList->SetModeList(pList);

    }

    if (pList)
    {
        // check if string exists
        ByteString *pString = pList->First();
        while ( pString )
        {
            if (*pString == *pStr)
            {
                delete pStr;
                return;
            }
            pString = pList->Next();
        }
        pList->PutString( pStr );
    }
    else
        delete pStr;
}

/*****************************************************************************/
sal_uIntPtr SDepInfoList::PutString( ByteString* pStr)
/*****************************************************************************/
{
    return PutString( pStr, NULL);
}

/**************************************************************************
*
*   Sortiert einen ByteString in die Liste ein und gibt die Position,
*   an der einsortiert wurde, zurueck
*
**************************************************************************/

sal_uIntPtr SDepInfoList::PutString( ByteString* pStr, ByteString* pModeStr)
{
    if (pAllModeList)
    {
        delete pAllModeList;
        pAllModeList = NULL;
    }

    sal_uIntPtr nPos = GetPrevString ( pStr );
    if ( Count() )
    {
        {
            DepInfo* pInfo = GetObject( 0 );
            ByteString* pString = pInfo->GetProject();
            if ( pString->CompareTo( *pStr ) == COMPARE_GREATER )
            {
                pInfo = new DepInfo;
                if (pInfo)
                {
                    pInfo->SetProject(pStr);
                    if (pModeStr)
                        PutModeString(pInfo, pModeStr);
                    else
                        pInfo->SetAllModes();
                    Insert( pInfo, (sal_uIntPtr)0 );
                }
                return (sal_uIntPtr)0;
            }
        }
        ByteString* pString = GetObject( nPos )->GetProject();
        if ( *pStr != *pString )
        {
            DepInfo* pInfo = new DepInfo;
            if (pInfo)
            {
                pInfo->SetProject(pStr);
                if (pModeStr)
                    PutModeString(pInfo, pModeStr);
                else
                    pInfo->SetAllModes();
                Insert( pInfo, nPos+1 );
            }
            return ( nPos +1 );
        }
        else
        {
            delete pStr;
            DepInfo* pInfo = GetObject( nPos );
            if (pModeStr)
                PutModeString(pInfo, pModeStr);
            else
                pInfo->SetAllModes();
            return ( nPos +1 );
        }
    }
    else
    {
        DepInfo* pInfo = new DepInfo;
        if (pInfo)
        {
            pInfo->SetProject(pStr);
            if (pModeStr)
                PutModeString(pInfo, pModeStr);
            else
                pInfo->SetAllModes();
            Insert( pInfo);
            return (sal_uIntPtr)0;
        }
    }

    delete pStr;
    if (pModeStr)
        delete pModeStr;
    return NOT_THERE;
}

/*****************************************************************************/
ByteString* SDepInfoList::RemoveString( const ByteString& rName )
/*****************************************************************************/
{
    sal_uIntPtr i;
    ByteString* pReturn;
    if (pAllModeList)
    {
        delete pAllModeList;
        pAllModeList = NULL;
    }

    for( i = 0 ; i < Count(); i++ )
    {
        if ( rName == *(GetObject( i )->GetProject()) )
        {
            pReturn = new ByteString(*(GetObject(i)->GetProject()));
            DepInfo* pInfo;
            pInfo = GetObject(i);
            delete pInfo;
            Remove(i);
            return pReturn;
        }
    }

    return NULL;
}

/*****************************************************************************/
SByteStringList* SDepInfoList::GetAllDepModes()
/*****************************************************************************/
{
    if (pAllModeList)
            return pAllModeList;

    DepInfo *pInfo = First();
    while ( pInfo )
    {
        if (!pInfo->IsAllModes() && pInfo->GetModeList())
        {
            if (!pAllModeList)
                pAllModeList = new SByteStringList();
            ByteString* pStr = pInfo->GetModeList()->First();
            while (pStr)
            {
                if (pAllModeList->IsString(pStr) == NOT_THERE)
                    pAllModeList->PutString(pStr);
                pStr = pInfo->GetModeList()->Next();
            }
        }
        pInfo = Next();
    }
    return pAllModeList;
}

/*****************************************************************************/
SDepInfoList& SDepInfoList::operator<< ( SvStream& rStream )
/*****************************************************************************/
{
    sal_uIntPtr nCount_l;
    rStream >> nCount_l;
    for ( sal_uInt16 i = 0; i < nCount_l; i++ ) {
        DepInfo* pDepInfo = new DepInfo();
        *pDepInfo << rStream;
        Insert (pDepInfo, LIST_APPEND);
    }
    return *this;
}

/*****************************************************************************/
SDepInfoList& SDepInfoList::operator>> ( SvStream& rStream )
/*****************************************************************************/
{
    sal_uIntPtr nCount_l = Count();
    rStream << nCount_l;
    DepInfo* pDepInfo = First();
    while (pDepInfo) {
        *pDepInfo >> rStream;
        pDepInfo = Next();
    }

    return *this;
}

/*****************************************************************************/
CommandData* Prj::GetDirectoryList ( sal_uInt16 nWhatOS, sal_uInt16 nCommand )
/*****************************************************************************/
{
    return (CommandData *)NULL;
}

/*****************************************************************************/
CommandData* Prj::GetDirectoryData( ByteString aLogFileName )
/*****************************************************************************/
{
    PrjList* pPrjList = GetCommandDataList ();
    CommandData *pData = NULL;
    sal_uIntPtr nCount_l = pPrjList->Count();
    for ( sal_uIntPtr i=0; i<nCount_l; i++ )
    {
        pData = pPrjList->GetObject(i);
        if ( pData->GetLogFile() == aLogFileName )
            return pData;
    }
    return NULL;
}

//
//  class Prj
//

/*****************************************************************************/
Prj::Prj() :
    pPrjInitialDepList(0),
    pPrjDepList(0),
    pPrjDepInfoList(0),
    bSorted( sal_False ),
    bHardDependencies( sal_False ),
    bFixedDependencies( sal_False ),
    bVisited( sal_False ),
    bIsAvailable( sal_True ),
    pTempCommandDataList (0),
    bTempCommandDataListPermanent (sal_False),
    bError (sal_False)
/*****************************************************************************/
{
}

/*****************************************************************************/
Prj::Prj( ByteString aName ) :
    aProjectName( aName ),
    pPrjInitialDepList(0),
    pPrjDepList(0),
    pPrjDepInfoList(0),
    bSorted( sal_False ),
    bHardDependencies( sal_False ),
    bFixedDependencies( sal_False ),
    bVisited( sal_False ),
    bIsAvailable( sal_True ),
    pTempCommandDataList (0),
    bTempCommandDataListPermanent (sal_False),
    bError (sal_False)
/*****************************************************************************/
{
}

/*****************************************************************************/
Prj::~Prj()
/*****************************************************************************/
{
    pPrjDepList = RemoveStringList (pPrjDepList);
    pPrjInitialDepList = RemoveStringList (pPrjInitialDepList);
    pPrjDepInfoList = RemoveDepInfoList (pPrjDepInfoList);
}

/*****************************************************************************/
SByteStringList* Prj::RemoveStringList(SByteStringList* pStringList )
/*****************************************************************************/
{
    if ( pStringList )
    {
        ByteString *pString = pStringList->First();
        while ( pString )
        {
            delete pString;
            pString = pStringList->Next();
        }

        delete pStringList;

        pStringList = NULL;
    }
    return pStringList;
}

/*****************************************************************************/
SDepInfoList* Prj::RemoveDepInfoList(SDepInfoList* pInfoList )
/*****************************************************************************/
{
    if ( pInfoList )
    {
        DepInfo *pInfo = pInfoList->First();
        while ( pInfo )
        {
            delete pInfo;
            pInfo = pInfoList->Next();
        }

        delete pInfoList;

        pInfoList = NULL;
    }
    return pInfoList;
}

/*****************************************************************************/
void Prj::AddDependencies( ByteString aStr )
/*****************************************************************************/
{

    if ( !pPrjDepInfoList )
        pPrjDepInfoList = new SDepInfoList;

    pPrjDepInfoList->PutString( new ByteString(aStr) );
}

/*****************************************************************************/
void Prj::AddDependencies( ByteString aStr, ByteString aModeStr )
/*****************************************************************************/
{

    // needs dirty flag - not expanded
    if ( !pPrjDepInfoList )
        pPrjDepInfoList = new SDepInfoList;

    pPrjDepInfoList->PutString( new ByteString(aStr), new ByteString(aModeStr) );
}

/*****************************************************************************/
SByteStringList* Prj::GetDependencies( sal_Bool bExpanded )
/*****************************************************************************/
{
    if ( bExpanded )
    {
        if (!pPrjDepList)
            SetMode (NULL);
        return pPrjDepList;
    }
    else
    {
        if (!pPrjInitialDepList)
            SetMode (NULL);
        return pPrjInitialDepList;
    }
}

/*****************************************************************************/
void Prj::SetMode(SByteStringList* pModList)
/*****************************************************************************/
{
    pPrjDepList = RemoveStringList (pPrjDepList);
    pPrjInitialDepList = RemoveStringList (pPrjInitialDepList);

    if (!pPrjDepInfoList)
        return;

    pPrjDepList = new SByteStringList;
    pPrjInitialDepList = new SByteStringList;

    DepInfo *pInfo = pPrjDepInfoList->First();
    while ( pInfo )
    {
        if (pInfo->IsAllModes() || !pInfo->GetModeList() || !pModList)
        {
            pPrjDepList->PutString( new ByteString((ByteString) *(pInfo->GetProject())));
            pPrjInitialDepList->PutString( new ByteString((ByteString) *(pInfo->GetProject())));
            //pPrjDepList->PutString( pInfo->GetProject());
            //pPrjInitialDepList->PutString( pInfo->GetProject());
        }
        else
        {
            sal_Bool bStringFound = sal_False;
            SByteStringList * pDepList = pInfo->GetModeList();
            ByteString *pModString = pDepList->First();
            while ( pModString )
            {
                ByteString *pDefModString = pModList->First();
                while ( pDefModString )
                {
                    if (*pDefModString == *pModString)
                    {
                        pPrjDepList->PutString( new ByteString((ByteString) *(pInfo->GetProject())));
                        pPrjInitialDepList->PutString( new ByteString((ByteString) *(pInfo->GetProject())));
                        //pPrjDepList->PutString( pInfo->GetProject());
                        //pPrjInitialDepList->PutString( pInfo->GetProject());
                        bStringFound = sal_True;
                        break;
                    }
                    pDefModString = pModList->Next();
                }
                if (bStringFound)
                    break;
                pModString = pDepList->Next();
            }

        }

        pInfo = pPrjDepInfoList->Next();
    }
}

/*****************************************************************************/
sal_Bool Prj::InsertDirectory ( ByteString aDirName, sal_uInt16 aWhat,
                                sal_uInt16 aWhatOS, ByteString aLogFileName,
                                const ByteString &rClientRestriction )
/*****************************************************************************/
{
    CommandData* pData = new CommandData();

    pData->SetPath( aDirName );
    pData->SetCommandType( aWhat );
    pData->SetOSType( aWhatOS );
    pData->SetLogFile( aLogFileName );
    pData->SetClientRestriction( rClientRestriction );

    PrjList* pPrjList = GetCommandDataList ();
    pPrjList->Insert( pData );

    return sal_False;
}

/*****************************************************************************/
//
// removes directory and existing dependencies on it
//
CommandData* Prj::RemoveDirectory ( ByteString aLogFileName )
/*****************************************************************************/
{
    PrjList* pPrjList = GetCommandDataList ();
    sal_uIntPtr nCount_l = pPrjList->Count();
    CommandData* pData;
    CommandData* pDataFound = NULL;
    SByteStringList* pDataDeps;

    for ( sal_uInt16 i = 0; i < nCount_l; i++ )
    {
        pData = pPrjList->GetObject( i );
        if ( pData->GetLogFile() == aLogFileName )
            pDataFound = pData;
        else
        {
            pDataDeps = pData->GetDependencies();
            if ( pDataDeps )
            {
                ByteString* pString;
                sal_uIntPtr nDataDepsCount = pDataDeps->Count();
                for ( sal_uIntPtr j = nDataDepsCount; j > 0; j-- )
                {
                    pString = pDataDeps->GetObject( j - 1 );
                    if ( pString->GetToken( 0, '.') == aLogFileName )
                        pDataDeps->Remove( pString );
                }
            }
        }
    }

    Remove( pDataFound );

    return pDataFound;
}

/*****************************************************************************/
void Prj::ExtractDependencies()
/*****************************************************************************/
{
    sal_uIntPtr nPos = 0;
    CommandData* pData = GetObject(nPos);
    while (pData)
    {
        SByteStringList* pDepList = pData->GetDependencies();
        if (pDepList)
        {
            ByteString * pDepStr = pDepList->First();
            while (pDepStr)
            {
                CommandData* pSearchData = First();
                while (pSearchData)
                {
                    if ((*pDepStr == pSearchData->GetPath()) && (pData->GetOSType() & pSearchData->GetOSType()))
                    {
                        *pDepStr = pSearchData->GetLogFile();
                        break;
                    }

                    pSearchData = Next();
                }

                pDepStr = pDepList->Next();
            }
        }
        nPos ++;
        pData = GetObject(nPos);
    }
}

/*****************************************************************************/
PrjList* Prj::GetCommandDataList ()
/*****************************************************************************/
{
    if (pTempCommandDataList)
        return pTempCommandDataList;
    else
        return (PrjList*)this;
}

/*****************************************************************************/
void Prj::RemoveTempCommandDataList()
/*****************************************************************************/
{
    if (pTempCommandDataList)
    {
        delete pTempCommandDataList; // this list remove the elements by itself
        pTempCommandDataList = NULL;
    }
}

/*****************************************************************************/
void Prj::GenerateTempCommandDataList()
/*****************************************************************************/
{
    if (pTempCommandDataList)
        RemoveTempCommandDataList();
    pTempCommandDataList = new PrjList();
    CommandData* pCommandData = First();
    while (pCommandData) {
        SvMemoryStream* pStream = new SvMemoryStream();
        *pCommandData >> *pStream;
        CommandData* pNewCommandData = new CommandData();
        pStream->Seek( STREAM_SEEK_TO_BEGIN );
        *pNewCommandData << *pStream;
        pTempCommandDataList->Insert(pNewCommandData, LIST_APPEND);
        delete pStream;
        pCommandData = Next();
    }
}

/*****************************************************************************/
void Prj::GenerateEmptyTempCommandDataList()
/*****************************************************************************/
{
    if (pTempCommandDataList)
        RemoveTempCommandDataList();
    pTempCommandDataList = new PrjList();
}

/*****************************************************************************/
Prj& Prj::operator>>  ( SvStream& rStream )
/*****************************************************************************/
{
    rStream << bVisited;
    rStream << aProjectName;
    rStream << aProjectPrefix;
    rStream << bHardDependencies;
    rStream << bFixedDependencies;
    rStream << bSorted;
    rStream << bIsAvailable;
    rStream << bError;

    if (pPrjDepInfoList)
    {
        rStream << sal_True;
        *pPrjDepInfoList >> rStream;
    }
    else
        rStream << sal_False;

    sal_uIntPtr nCount_l = Count();
    rStream << nCount_l;

    CommandData* pData = First();
    while (pData) {
        *pData >> rStream;
        pData = Next();
    }

    return *this;
}

/*****************************************************************************/
Prj& Prj::operator<<  ( SvStream& rStream )
/*****************************************************************************/
{
       rStream >> bVisited;
    rStream >> aProjectName;
    rStream >> aProjectPrefix;
    rStream >> bHardDependencies;
    rStream >> bFixedDependencies;
    rStream >> bSorted;
    rStream >> bIsAvailable;
    rStream >> bError;

    sal_Bool bDepList;
    rStream >> bDepList;
    DELETEZ (pPrjDepInfoList);
    if (bDepList)
    {
        pPrjDepInfoList = new SDepInfoList();
        *pPrjDepInfoList << rStream;
    }

    sal_uIntPtr nCount_l;
    rStream >> nCount_l;

    for ( sal_uInt16 i = 0; i < nCount_l; i++ ) {
        CommandData* pData = new CommandData();
        *pData << rStream;
        Insert (pData, LIST_APPEND);
    }

    return *this;
}


//
//  class Star
//

/*****************************************************************************/
Star::Star()
/*****************************************************************************/
                : pDepMode (NULL),
                pAllDepMode (NULL)
{
    // this ctor is only used by StarWriter
}

/*****************************************************************************/
Star::Star(String aFileName, sal_uInt16 nMode )
/*****************************************************************************/
                : nStarMode( nMode ),
                sFileName( aFileName ),
                pDepMode (NULL),
                pAllDepMode (NULL)
{
    Read( aFileName );
}

/*****************************************************************************/
Star::Star(SolarFileList *pSolarFiles )
/*****************************************************************************/
                : nStarMode( STAR_MODE_MULTIPLE_PARSE ),
                pDepMode (NULL),
                pAllDepMode (NULL)
{
    // this ctor is used by StarBuilder to get the information for the whole workspace
    Read( pSolarFiles );
}

/*****************************************************************************/
Star::Star(GenericInformationList *pStandLst, ByteString &rVersion )
/*****************************************************************************/
                : pDepMode (NULL),
                pAllDepMode (NULL)
{
    UpdateFileList (pStandLst, rVersion, sal_True );
}

/*****************************************************************************/
void Star::UpdateFileList( GenericInformationList *pStandLst, ByteString &rVersion,
    sal_Bool bRead )
/*****************************************************************************/
{
    sSourceRoot=String::CreateFromAscii(""); // clear old SourceRoot
    ByteString sPath( rVersion );

#ifdef UNX
    sPath += "/settings/UNXSOLARLIST";
#else
    sPath += "/settings/SOLARLIST";
#endif
    GenericInformation *pInfo = pStandLst->GetInfo( sPath, sal_True );

    if( pInfo && pInfo->GetValue().Len()) {
        ByteString sFile( pInfo->GetValue());
        String sFileName_l( sFile, RTL_TEXTENCODING_ASCII_US );
        nStarMode = STAR_MODE_SINGLE_PARSE;
        if (bRead)
            Read( sFileName_l );
    }
    else {
        SolarFileList *pFileList = new SolarFileList();

        sPath = rVersion;
        sPath += "/drives";

        GenericInformation *pInfo_l = pStandLst->GetInfo( sPath, sal_True );
        if ( pInfo_l && pInfo_l->GetSubList())  {
            GenericInformationList *pDrives = pInfo_l->GetSubList();
            for ( sal_uIntPtr i = 0; i < pDrives->Count(); i++ ) {
                GenericInformation *pDrive = pDrives->GetObject( i );
                if ( pDrive ) {
                    DirEntry aEntry;
                    sal_Bool bOk = sal_False;
                    if ( sSourceRoot.Len()) {
                        aEntry = DirEntry( sSourceRoot );
                        bOk = sal_True;
                    }
                    else {
#ifdef UNX
                        sPath = "UnixVolume";
                        GenericInformation *pUnixVolume = pDrive->GetSubInfo( sPath );
                        if ( pUnixVolume ) {
                            String sRoot( pUnixVolume->GetValue(), RTL_TEXTENCODING_ASCII_US );
                            aEntry = DirEntry( sRoot );
                            bOk = sal_True;
                         }
#else
                        bOk = sal_True;
                        String sRoot( *pDrive, RTL_TEXTENCODING_ASCII_US );
                        sRoot += String::CreateFromAscii( "\\" );
                        aEntry = DirEntry( sRoot );
#endif
                    }
                    if ( bOk ) {
                        sPath = "projects";
                        GenericInformation *pProjectsKey = pDrive->GetSubInfo( sPath, sal_True );
                        if ( pProjectsKey ) {
                            if ( !sSourceRoot.Len()) {
                                sPath = rVersion;
                                sPath += "/settings/PATH";
                                GenericInformation *pPath = pStandLst->GetInfo( sPath, sal_True );
                                if( pPath ) {
                                    ByteString sAddPath( pPath->GetValue());
#ifdef UNX
                                    sAddPath.SearchAndReplaceAll( "\\", "/" );
#else
                                    sAddPath.SearchAndReplaceAll( "/", "\\" );
#endif
                                    String ssAddPath( sAddPath, RTL_TEXTENCODING_ASCII_US );
                                    aEntry += DirEntry( ssAddPath );
                                }
                            }
                            sPath = rVersion;
                            sPath += "/settings/SHORTPATH";
                            GenericInformation *pShortPath = pStandLst->GetInfo( sPath, sal_True );
                            sal_Bool bShortPath = sal_False;
                            if (pShortPath && (pShortPath->GetValue() == "_TRUE"))
                                bShortPath = sal_True;
                            sSourceRoot = aEntry.GetFull();
                            GenericInformationList *pProjects = pProjectsKey->GetSubList();
                            if ( pProjects ) {
                                GenericInformation * pProject = pProjects->First();
                                while (pProject) {
                                    String sLocalSourceRoot = sSourceRoot;
                                    ByteString sProject( *pProject );
                                    String ssProject( sProject, RTL_TEXTENCODING_ASCII_US );

                                    ByteString aDirStr ("Directory");
                                    GenericInformation * pDir = pProject->GetSubInfo (aDirStr);
                                    if (pDir) {
                                        ByteString aDir = pDir->GetValue();
                                        DirEntry aRootEntry;
                                        if (bShortPath)
                                            aRootEntry = aEntry + DirEntry(aDir);
                                        else
                                            aRootEntry = aEntry.GetPath() + DirEntry(aDir);
                                        sLocalSourceRoot = aRootEntry.GetFull();
                                    }

                                    String aBuildListPath = CreateFileName(ssProject, sLocalSourceRoot);

                                    pFileList->Insert( new String( aBuildListPath ), LIST_APPEND );
                                    ByteString sFile( aBuildListPath, RTL_TEXTENCODING_ASCII_US );
                                    pProject = pProjects->Next();
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!CheckFileLoadList(pFileList))
        {
            ClearAvailableDeps();
            ClearCurrentDeps();
            ClearLoadedFilesList();
            RemoveAllPrj();
            bRead = sal_True; // read new list because old list is deleted
        }

        if (bRead)
            Read( pFileList );
        else
            GenerateFileLoadList( pFileList );
    }
}

/*****************************************************************************/
void Star::FullReload( GenericInformationList *pStandLst, ByteString &rVersion,
    sal_Bool bRead )
/*****************************************************************************/
{
    ClearAvailableDeps();
    ClearCurrentDeps();
    ClearLoadedFilesList();
    RemoveAllPrj();
    UpdateFileList( pStandLst, rVersion, bRead );
}

/*****************************************************************************/
sal_Bool Star::CheckFileLoadList(SolarFileList *pSolarFiles)
/*****************************************************************************/
{
    sal_Bool bRet = sal_True;
    if (aLoadedFilesList.Count() == 0)
        return bRet;
    StarFile * pLoadFile = aLoadedFilesList.First();
    while (pLoadFile)
    {
        sal_Bool bIsAvailable = sal_False;
        String * pFile = pSolarFiles->First();
        while (pFile)
        {
            if (*pFile == pLoadFile->GetName())
            {
                bIsAvailable = sal_True;
                break;
            }
            pFile = pSolarFiles->Next();
        }
        if (!bIsAvailable)
        {
            bRet = sal_False;
            break;
        }
        pLoadFile = aLoadedFilesList.Next();
    }
       return bRet;
}

/*****************************************************************************/
Star::~Star()
/*****************************************************************************/
{
    ClearAvailableDeps();
    ClearCurrentDeps();
    ClearLoadedFilesList();
    RemoveAllPrj();
}

/*****************************************************************************/
void Star::GenerateFileLoadList( SolarFileList *pSolarFiles )
/*****************************************************************************/
{
    SolarFileList* pNewSolarFiles = NULL;
    while(  pSolarFiles->Count()) {
        StarFile *pFile = new StarFile( *pSolarFiles->GetObject(( sal_uIntPtr ) 0 ));
        aMutex.acquire();
        sal_uIntPtr nPos = SearchFileEntry(&aLoadedFilesList, pFile);
        if ( nPos == LIST_ENTRY_NOTFOUND )
        {
            if (!pNewSolarFiles)
                pNewSolarFiles = new SolarFileList();

            pNewSolarFiles->Insert(new String(pFile->GetName()), LIST_APPEND );
        }
        aMutex.release();
        delete pSolarFiles->Remove(( sal_uIntPtr ) 0 );
        delete pFile;
    }
    delete pSolarFiles;
    if (pNewSolarFiles)
        Read (pNewSolarFiles);
}

/*****************************************************************************/
SolarFileList* Star::NeedsFilesForUpdate()
/*****************************************************************************/
{
    aMutex.acquire();
    SolarFileList* pPrjList = NULL;
    for ( sal_uIntPtr i = 0; i < aLoadedFilesList.Count(); i++ )
        if ( aLoadedFilesList.GetObject( i )->NeedsUpdate()) {
            if (!pPrjList)
                pPrjList = new SolarFileList();

            pPrjList->Insert(new String (aLoadedFilesList.GetObject( i )->GetName()), LIST_APPEND);
        }

    aMutex.release();
    return pPrjList;
}

/*****************************************************************************/
sal_Bool Star::NeedsUpdate()
/*****************************************************************************/
{
    aMutex.acquire();
    for ( sal_uIntPtr i = 0; i < aLoadedFilesList.Count(); i++ )
        if ( aLoadedFilesList.GetObject( i )->NeedsUpdate()) {
            aMutex.release();
            return sal_True;
        }

    aMutex.release();
    return sal_False;
}

/*****************************************************************************/
void Star::Read( String &rFileName )
/*****************************************************************************/
{
    ClearAvailableDeps ();
    ByteString aString;
    aFileList.Insert( new String( rFileName ));

    DirEntry aEntry( rFileName );
    aEntry.ToAbs();
    aEntry = aEntry.GetPath().GetPath().GetPath();
    sSourceRoot = aEntry.GetFull();

    while( aFileList.Count()) {
        String ssFileName = *aFileList.GetObject(( sal_uIntPtr ) 0 );
        StarFile* pFile = ReadBuildlist (ssFileName);
        aMutex.acquire();
        ReplaceFileEntry (&aLoadedFilesList, pFile);
        //aLoadedFilesList.Insert( pFile, LIST_APPEND );
        aMutex.release();
        aFileList.Remove(( sal_uIntPtr ) 0 );
    }
    // resolve all dependencies recursive
    Expand_Impl();
}

/*****************************************************************************/
sal_uIntPtr Star::SearchFileEntry( StarFileList *pStarFiles, StarFile* pFile )
/*****************************************************************************/
{
    StarFile *pSearchFile;
    sal_uIntPtr nCount_l;

    nCount_l = pStarFiles->Count();

    for ( sal_uIntPtr i=0; i<nCount_l; i++)
    {
        pSearchFile = pStarFiles->GetObject(i);
        if ( pSearchFile->GetName() == pFile->GetName() )
        {
            return i;
        }
    }
    return LIST_ENTRY_NOTFOUND;
}

/*****************************************************************************/
void Star::ReplaceFileEntry( StarFileList *pStarFiles, StarFile* pFile )
/*****************************************************************************/
{
    sal_uIntPtr nPos = SearchFileEntry(pStarFiles, pFile);
    if ( nPos != LIST_ENTRY_NOTFOUND )
    {
        StarFile* pTmpStarFile = pStarFiles->GetObject(nPos);
        delete pTmpStarFile;
        pStarFiles->Replace(pFile, nPos);
        return;
    }
    pStarFiles->Insert( pFile,  LIST_APPEND );
}

/*****************************************************************************/
void Star::Read( SolarFileList *pSolarFiles )
/*****************************************************************************/
{
    ClearAvailableDeps ();
    while(  pSolarFiles->Count()) {
        ByteString aString;

        String ssFileName = *pSolarFiles->GetObject(( sal_uIntPtr ) 0 );
        StarFile *pFile = ReadBuildlist ( ssFileName);

        if ( pFile->Exists()) {
            DirEntry aEntry( pFile->GetName() );
            DirEntry aEntryPrj = aEntry.GetPath().GetPath();
            if (aEntryPrj.GetExtension() != String::CreateFromAscii( "" ))
            {
                aEntryPrj.CutExtension();
                ByteString aPrjName = ByteString( aEntryPrj.GetName(), gsl_getSystemTextEncoding());
                Prj* pPrj = GetPrj(aPrjName);
                if (pPrj)
                    pPrj->IsAvailable (sal_False);
            }

        }

        aMutex.acquire();
        ReplaceFileEntry (&aLoadedFilesList, pFile);
        //aLoadedFilesList.Insert( pFile,   LIST_APPEND );
        aMutex.release();
        delete pSolarFiles->Remove(( sal_uIntPtr ) 0 );
    }
    delete pSolarFiles;

    Expand_Impl();
}

/*****************************************************************************/
String Star::CreateFileName( String& rProject, String& rSourceRoot )
/*****************************************************************************/
{
    // this method is used to find solarlist parts of nabours (other projects)
    String sPrjDir( String::CreateFromAscii( "prj" ));
    String sBuildList( String::CreateFromAscii( "build.lst" ));
//  String sXmlBuildList( String::CreateFromAscii( "build.xlist" ));

    DirEntry aEntry( rSourceRoot );
    aEntry += DirEntry( rProject );

    // if this project not exists, maybe it's a not added project of a CWS

    if ( !aEntry.Exists() ) {
        aEntry.SetExtension(String::CreateFromAscii( "lnk" ));
        if ( !aEntry.Exists() )
            aEntry.CutExtension();

        aEntry.SetExtension(String::CreateFromAscii( "link" ));
        if ( !aEntry.Exists() )
            aEntry.CutExtension();
    }

    aEntry += DirEntry( sPrjDir );

//  DirEntry aPossibleEntry(aEntry);
//  aPossibleEntry += DirEntry( sXmlBuildList );

    aEntry += DirEntry( sBuildList );

    DirEntry& aActualEntry = aEntry;
/*
    if (aPossibleEntry.Exists()) {
        aActualEntry = aPossibleEntry;
    } else */
    if ( !aActualEntry.Exists() && aDBNotFoundHdl.IsSet())
        aDBNotFoundHdl.Call( &rProject );
    return aActualEntry.GetFull();
}

/*****************************************************************************/
void Star::InsertSolarList( String sProject )
/*****************************************************************************/
{
    // inserts a new solarlist part of another project
    String sFileName_l( CreateFileName( sProject, sSourceRoot ));

    for ( sal_uIntPtr i = 0; i < aFileList.Count(); i++ ) {
        if (( *aFileList.GetObject( i )) == sFileName_l )
            return;
    }

    ByteString ssProject( sProject, RTL_TEXTENCODING_ASCII_US );
    if ( HasProject( ssProject ))
        return;

    aFileList.Insert( new String( sFileName_l ), LIST_APPEND );
}

/*****************************************************************************/
void Star::ExpandPrj_Impl( Prj *pPrj, Prj *pDepPrj )
/*****************************************************************************/
{
    if ( pDepPrj->bVisited )
        return;

    pDepPrj->bVisited = sal_True;

    SByteStringList* pPrjLst = pPrj->GetDependencies();
    SByteStringList* pDepLst = NULL;
    ByteString* pDepend;
    ByteString* pPutStr;
    Prj *pNextPrj = NULL;
    sal_uIntPtr i, nRetPos;

    if ( pPrjLst ) {
        pDepLst = pDepPrj->GetDependencies();
        if ( pDepLst ) {
            for ( i = 0; i < pDepLst->Count(); i++ ) {
                pDepend = pDepLst->GetObject( i );
                pPutStr = new ByteString( *pDepend );
                nRetPos = pPrjLst->PutString( pPutStr );
                if( nRetPos == NOT_THERE )
                    delete pPutStr;
                pNextPrj = GetPrj( *pDepend );
                if ( pNextPrj ) {
                    ExpandPrj_Impl( pPrj, pNextPrj );
                }
            }
        }
    }
}

/*****************************************************************************/
void Star::Expand_Impl()
/*****************************************************************************/
{
    for ( sal_uIntPtr i = 0; i < Count(); i++ ) {
        for ( sal_uIntPtr j = 0; j < Count(); j++ )
            GetObject( j )->bVisited = sal_False;

        Prj* pPrj = GetObject( i );
        pPrj->SetMode(pDepMode); // DepList für Mode initialisieren
        ExpandPrj_Impl( pPrj, pPrj );
    }
}

/*****************************************************************************/
StarFile* Star::ReadBuildlist (const String& rFilename, sal_Bool bReadComments, sal_Bool bExtendAlias)
/*****************************************************************************/
{
    ByteString sFileName_l(rFilename, RTL_TEXTENCODING_ASCII_US);
    StarFile *pFile = new StarFile( rFilename );
    if ( pFile->Exists()) {
        SimpleConfig aSolarConfig( rFilename );
        DirEntry aEntry(rFilename);
        ByteString sProjectName (aEntry.GetPath().GetPath().GetName(), RTL_TEXTENCODING_ASCII_US);
        Prj* pPrj = GetPrj (sProjectName); // 0, if Prj not found
        if (pPrj)
        {
            Remove(pPrj); // Project exist, remove old Project and read again
            DELETEZ (pPrj); // delete and set pPrj to 0
        }
        ByteString aString;
        while (( aString = aSolarConfig.GetCleanedNextLine( bReadComments )) != ByteString::EmptyString() )
            InsertTokenLine ( aString, &pPrj, sProjectName, bExtendAlias );
    }
    return pFile;
}

/*****************************************************************************/
void Star::InsertTokenLine ( const ByteString& rTokenLine, Prj** ppPrj, const ByteString& rProjectName, const sal_Bool bExtendAlias )
/*****************************************************************************/
{
    int i = 0;
    ByteString aWhat, aWhatOS,
        sClientRestriction, aLogFileName, aProjectName, aPrefix, aCommandPara;
    ByteString aDirName;
    sal_Bool bPrjDep = sal_False;
    sal_Bool bHardDep = sal_False;
    sal_Bool bFixedDep = sal_False;
    sal_Bool bNewProject = sal_False;
    int nCommandType=0, nOSType=0;
    Prj* pPrj = *ppPrj;
    CommandData* pCmdData;
    SByteStringList *pDepList = NULL;
    ByteString aCommentString;
    ByteString sToken;
    ByteString sStringBuffer = rTokenLine;

    while (sStringBuffer != ByteString::EmptyString())
    {
        ByteString sToken = sStringBuffer.GetToken(0,'\t');
        sStringBuffer.Erase(0, sToken.Len()+1);

        switch (i)
        {
            case 0:
                    if ( sToken.Search( "#" ) == 0 )
                    {
                        i = -1;
                        aCommentString = sToken;
                        sStringBuffer = ByteString::EmptyString();
                        if ( Count() == 0 )
                            aDirName = "null_entry" ; //comments at begin of file
                    }
                    else
                    {
                        aPrefix = sToken;
                        pDepList = 0;
                    }
                    break;
            case 1:
                    aDirName = sToken;
                    aProjectName = aDirName.GetToken ( 0, 0x5c);
                    if (aProjectName != rProjectName)
                        sStringBuffer = ByteString::EmptyString(); // something is wrong, ignore line
                    break;
            case 2:
                    if ( sToken.CompareTo(":") == COMPARE_EQUAL )
                    {
                        bPrjDep = sal_True;
                        bHardDep = sal_False;
                        bFixedDep = sal_False;
                        i = 9;
                    }
                    else if ( sToken.CompareTo("::") == COMPARE_EQUAL )
                    {
                        bPrjDep = sal_True;
                        bHardDep = sal_True;
                        bFixedDep = sal_False;
                        i = 9;
                    }
                    else if ( sToken.CompareTo(":::") == COMPARE_EQUAL )
                    {
                        bPrjDep = sal_True;
                        bHardDep = sal_True;
                        bFixedDep = sal_True;
                        i = 9;
                    }
                    else
                    {
                        bPrjDep = sal_False;
                        bHardDep = sal_False;
                        bFixedDep = sal_False;

                        aWhat = sToken;
                        nCommandType = GetJobType(aWhat);
                    }
                    if (bPrjDep)
                    {
                        if (pPrj)
                            sStringBuffer = ByteString::EmptyString(); // definition more than once or not first line, ignore line
                    }
                    break;
            case 3:
                    if ( !bPrjDep )
                    {
                        aWhat = sToken;
                        if ( aWhat == "-" )
                        {
                            aCommandPara = ByteString();
                        }
                        else
                            aCommandPara = aWhat;
                    }
                    break;
            case 4:
                    if ( !bPrjDep )
                    {
                        aWhatOS = sToken;
                        if ( aWhatOS.GetTokenCount( ',' ) > 1 ) {
                            sClientRestriction = aWhatOS.Copy( aWhatOS.GetToken( 0, ',' ).Len() + 1 );
                            aWhatOS = aWhatOS.GetToken( 0, ',' );
                        }
                        nOSType = GetOSType (aWhatOS);
                    }
                    break;
            case 5:
                    if ( !bPrjDep )
                    {
                        if (bExtendAlias)
                            aLogFileName = (ByteString(aProjectName).Append("_")).Append(sToken);
                        else
                            aLogFileName = sToken;

                    }
                    break;
            default:
                    if ( !bPrjDep )
                    {
                        ByteString aItem = sToken;
                        if ( aItem == "NULL" )
                        {
                            // Liste zu Ende
                            i = -1;
                        }
                        else
                        {
                            // ggfs. Dependency liste anlegen und ergaenzen
                            if ( !pDepList )
                                pDepList = new SByteStringList;
                            ByteString* pStr;
                            if (bExtendAlias)
                                pStr = new ByteString ((ByteString (aProjectName).Append("_")).Append(aItem));
                            else
                                pStr = new ByteString (aItem);
                            pDepList->PutString( pStr );
                        }
                    }
                    else
                    {
                        ByteString aItem = sToken;
                        if ( aItem == "NULL" )
                        {
                            // Liste zu Ende
                            i = -1;
                            bPrjDep= sal_False;
                        }
                        else
                        {
                            ByteString sMode;
                            sal_Bool bHasModes = sal_False;
                            if (aItem.Search(":") != STRING_NOTFOUND)
                            {
                                sMode = aItem.GetToken ( 0, ':');
                                aItem = aItem.GetToken ( 1, ':');
                                bHasModes = sal_True;
                            }
                            if (!pPrj)
                            {
                                // neues Project anlegen
                                pPrj = new Prj ( aProjectName );
                                pPrj->SetPreFix( aPrefix );
                                bNewProject = sal_True;
                            }
                            if (bHasModes)
                                pPrj->AddDependencies( aItem, sMode );
                            else
                                pPrj->AddDependencies( aItem );
                            pPrj->HasHardDependencies( bHardDep );
                            pPrj->HasFixedDependencies( bFixedDep );
                        }
                    }
                    break;
        }
        if ( i == -1 )
            break;
        i++;
    }
    /* Wenn dieses Project noch nicht vertreten ist, in die Liste
       der Solar-Projekte einfuegen */
    if ( i == -1 )
    {
        if (!pPrj)
        {
            // neues Project anlegen
            pPrj = new Prj ( aProjectName );
            pPrj->SetPreFix( aPrefix );
            bNewProject = sal_True;
        }

        if (bNewProject)
            Insert(pPrj,LIST_APPEND);

        pCmdData = new CommandData;
        pCmdData->SetPath( aDirName );
        pCmdData->SetCommandType( nCommandType );
        pCmdData->SetCommandPara( aCommandPara );
        pCmdData->SetOSType( nOSType );
        pCmdData->SetLogFile( aLogFileName );
        pCmdData->SetComment( aCommentString );
        pCmdData->SetClientRestriction( sClientRestriction );
        if ( pDepList )
            pCmdData->SetDependencies( pDepList );

        pDepList = 0;
        pPrj->Insert ( pCmdData, LIST_APPEND );

        // und wer raeumt die depLst wieder ab ?
        // CommandData macht das
    }
    else
    {
        if (!pPrj)
        {
            // new project to set the error flag
            pPrj = new Prj ( rProjectName );
            pPrj->SetPreFix( aPrefix );
            bNewProject = sal_True;
        }
        if (pPrj)
        {
            pPrj->SetError();
            if (bNewProject)
                Insert(pPrj,LIST_APPEND); // add project even if there is a buildlist error
        }
        if ( pDepList )
            delete pDepList;
    }
    *ppPrj = pPrj;
}

/*****************************************************************************/
sal_Bool Star::HasProject ( ByteString aProjectName )
/*****************************************************************************/
{
    Prj *pPrj;
    int nCount_l;

    nCount_l = Count();

    for ( int i=0; i<nCount_l; i++)
    {
        pPrj = GetObject(i);
        if ( pPrj->GetProjectName().ToLowerAscii() == aProjectName.ToLowerAscii() )
            return sal_True;
    }
    return sal_False;
}

/*****************************************************************************/
Prj* Star::GetPrj ( ByteString aProjectName )
/*****************************************************************************/
{
    Prj* pPrj;
    int nCount_l = Count();
    for ( int i=0;i<nCount_l;i++)
    {
        pPrj = GetObject(i);
        if ( pPrj->GetProjectName().ToLowerAscii() == aProjectName.ToLowerAscii() )
            return pPrj;
    }
//  return (Prj*)NULL;
    return 0L ;
}

/*****************************************************************************/
sal_Bool Star::RemovePrj ( Prj* pPrj )
/*****************************************************************************/
{
    sal_uIntPtr nPos = GetPos(pPrj);
    if (nPos != LIST_ENTRY_NOTFOUND) {
        delete pPrj;
        Remove(nPos);
        return sal_True;
    }
    return sal_False;
}

/*****************************************************************************/
void Star::RemoveAllPrj ()
/*****************************************************************************/
{
    Prj* pPrj = First();
    while (pPrj)
    {
        delete pPrj;
        pPrj = Next();
    }
    Clear();
}

/*****************************************************************************/
ByteString Star::GetPrjName( DirEntry &aPath )
/*****************************************************************************/
{
    ByteString aRetPrj, aDirName;
    ByteString aFullPathName = ByteString( aPath.GetFull(), gsl_getSystemTextEncoding());

    sal_uInt16 nToken = aFullPathName.GetTokenCount(PATH_DELIMETER);
    for ( int i=0; i< nToken; i++ )
    {
        aDirName = aFullPathName.GetToken( i, PATH_DELIMETER );
        if ( HasProject( aDirName ))
        {
            aRetPrj = aDirName;
            break;
        }
    }

    return aRetPrj;
}

/*****************************************************************************/
void Star::ClearAvailableDeps ()
/*****************************************************************************/
{
    if ( pAllDepMode )
    {
        ByteString *pString = pAllDepMode->First();
        while ( pString )
        {
            delete pString;
            pString = pAllDepMode->Next();
        }
        delete pAllDepMode;
        pAllDepMode = NULL;
    }
}

/*****************************************************************************/
void Star::ClearLoadedFilesList ()
/*****************************************************************************/
{
    StarFile *pStarFile = aLoadedFilesList.First();
    while ( pStarFile )
    {
        delete pStarFile;
        pStarFile = aLoadedFilesList.Next();
    }
    aLoadedFilesList.Clear();
}

/*****************************************************************************/
void Star::ClearCurrentDeps ()
/*****************************************************************************/
{
    if ( pDepMode )
    {
        ByteString *pString = pDepMode->First();
        while ( pString )
        {
            delete pString;
            pString = pDepMode->Next();
        }
        delete pDepMode;
        pDepMode = NULL;
    }
}

/*****************************************************************************/
SByteStringList* Star::GetAvailableDeps ()
/*****************************************************************************/
{
    if ( pAllDepMode )
        return pAllDepMode;

    Prj *pPrj;
    ByteString* pStr;
    pPrj = First();
    while (pPrj)
    {
        SByteStringList* pModeList = NULL;
        if (pPrj->GetModeAndDependencies() && (pModeList = pPrj->GetModeAndDependencies()->GetAllDepModes()))
        {
            pStr = pModeList->First();
            while (pStr)
            {
                if ( !pAllDepMode )
                    pAllDepMode = new SByteStringList();

                if (pAllDepMode->IsString(pStr) == NOT_THERE)
                    pAllDepMode->PutString(new ByteString(*pStr));

                pStr = pModeList->Next();
            }
        }
        pPrj = Next();
    }
    return pAllDepMode;
}

/*****************************************************************************/
void Star::SetCurrentDeps (SByteStringList* pDepList)
/*****************************************************************************/
{
    ClearCurrentDeps();

    if (pDepList)
    {
        pDepMode = new SByteStringList();
        ByteString *pString = pDepList->First();
        while ( pString )
        {
            ByteString* pStr = new ByteString (*pString);
            if (pDepMode->PutString(pStr) == NOT_THERE)
                delete pStr; // String is not in List
            pString = pDepList->Next();
        }
    }
    Expand_Impl();
}

///*****************************************************************************/
//void Star::ReadXmlBuildList(const ByteString& sBuildLstPath) {
///*****************************************************************************/
//  if (mpXmlBuildList) {
//      Prj* pPrj = NULL;
//
//      try {
//          mpXmlBuildList->loadXMLFile(sBuildLstPath);
//      }
//      catch (XmlBuildListException) {
//          DirEntry aDirEntry (sBuildLstPath);
//          String ssPrjName = aDirEntry.GetPath().GetPath().GetBase();
//          ByteString sPrjName = ByteString(ssPrjName, RTL_TEXTENCODING_ASCII_US);
//          pPrj = GetPrj( sPrjName );
//          if (pPrj)
//          {
//              //remove old Project
//              RemovePrj (pPrj);
//          }
//          return;
//      }
//
//      try {
//          ByteString sProjectName = mpXmlBuildList->getModuleName();
//          pPrj = GetPrj( sProjectName );
//          if (pPrj)
//          {
//              //remove old Project
//              RemovePrj (pPrj);
//          }
//
//          // insert new Project
//          pPrj = new Prj ( sProjectName );
//          pPrj->SetPreFix( sProjectName ); // use ProjectName as Prefix
//          Insert(pPrj,LIST_APPEND);
//
//          // get global dependencies
//          FullByteStringListWrapper aProducts = mpXmlBuildList->getProducts();
//          ByteString aDepType = ByteString(DEP_MD_ALWAYS_STR);
//          if (mpXmlBuildList->hasModuleDepType(aProducts, aDepType))
//              pPrj->HasHardDependencies( sal_True );
//
//          aDepType = ByteString(DEP_MD_FORCE_STR);
//          if (mpXmlBuildList->hasModuleDepType(aProducts, aDepType))
//          {
//              pPrj->HasHardDependencies( sal_True );
//              pPrj->HasFixedDependencies( sal_True );
//          }
//
//          // modul dependencies
//          ByteString sModulDepType = ByteString();
//          FullByteStringListWrapper aModulDeps = mpXmlBuildList->getModuleDependencies(aProducts, sModulDepType);
//          ByteString * pModulDep = aModulDeps.First();
//          while (pModulDep)
//          {
//              FullByteStringListWrapper aModulProducts = mpXmlBuildList->getModuleProducts(*pModulDep);
//              ByteString *pModulePoduct = aModulProducts.First();
//              while (pModulePoduct)
//              {
//                  if (*pModulePoduct == XML_ALL)
//                      pPrj->AddDependencies( *pModulDep );
//                  else
//                      pPrj->AddDependencies( *pModulDep, *pModulePoduct);
//
//                  pModulePoduct = aModulProducts.Next();
//              }
//              pModulDep = aModulDeps.Next();
//          }
//
//          // job dirs
//          ByteString sJobType = ByteString();
//          ByteString sJobPlatforms = ByteString();
//          FullByteStringListWrapper aJobDirs = mpXmlBuildList->getJobDirectories(sJobType, sJobPlatforms); // all dirs
//          ByteString* pJobDir = aJobDirs.First();
//          while (pJobDir)
//          {
//              FullByteStringListWrapper aJobPlatforms = mpXmlBuildList->getJobPlatforms (*pJobDir);
//              ByteString* pJobPlatform = aJobPlatforms.First();
//              while (pJobPlatform)
//              {
//                  ByteString sJobRestriction = ByteString();
//                  FullByteStringListWrapper aJobReq = mpXmlBuildList->getJobBuildReqs (*pJobDir, *pJobPlatform);
//                  // nur ein Req pro Platform wird zur Zeit unterstützt
//                  // mehr geht wegen der Struktur zur Zeit nicht!
//                  // lese sie trotzdem kommasepariert ein, wenn nötig
//                  if (aJobReq.Count() > 0)
//                  {
//                      ByteString* pRestriction = aJobReq.First();
//                      sJobRestriction = ByteString (*pRestriction);
//                      pRestriction = aJobReq.Next();
//                      while (pRestriction)
//                      {
//                          sJobRestriction += ByteString (",");
//                          sJobRestriction += ByteString (*pRestriction);
//                          pRestriction = aJobReq.Next();
//                      }
//                  }
//
//                  FullByteStringListWrapper aJobTypes = mpXmlBuildList->getJobTypes (*pJobDir);
//                  ByteString * pJobType = aJobTypes.First();
//                  while(pJobType)
//                  {
//                      FullByteStringListWrapper aDirDependencies = mpXmlBuildList->getDirDependencies(*pJobDir, *pJobType, *pJobPlatform);
//                      SByteStringList *pDepList = NULL;
//                      if (aDirDependencies.Count() > 0)
//                      {
//                          pDepList = new SByteStringList;
//                          ByteString* pDirDep = aDirDependencies.First();
//                          while (pDirDep)
//                          {
//                              ByteString sFullDir = sProjectName;
//                              sFullDir += *pDirDep;
//                              sFullDir.SearchAndReplaceAll('/', '\\');
//                              *pDirDep = sFullDir;
//                              pDepList->PutString(pDirDep);   // String wird übergeben
//                              aDirDependencies.Remove();      // Zeiger aus alter Liste löschen
//                              pDirDep = aDirDependencies.First();
//                          }
//                      }
//                      // insert CommandData
//                      CommandData * pCmdData = new CommandData;
//                      ByteString sRequiredPath = sProjectName;
//                      sRequiredPath += *pJobDir;
//                      sRequiredPath.SearchAndReplaceAll('/', '\\');
//                      pCmdData->SetPath(sRequiredPath);
//                      pCmdData->SetCommandType( GetJobType(*pJobType) );
//                      pCmdData->SetCommandPara( ByteString() );
//                      pCmdData->SetOSType( GetOSType(*pJobPlatform) );
//                      ByteString sLogFileName = sProjectName;
//                      sLogFileName += ByteString::CreateFromInt64( pPrj->Count() );
//                      pCmdData->SetLogFile( sLogFileName );
//                      pCmdData->SetClientRestriction( sJobRestriction );
//                      if ( pDepList )
//                          pCmdData->SetDependencies( pDepList );
//
//                      pPrj->Insert ( pCmdData, LIST_APPEND );
//
//                      pJobType = aJobTypes.Next();
//                  }
//
//                  pJobPlatform = aJobPlatforms.Next();
//              }
//
//              pJobDir = aJobDirs.Next();
//          }
//          pPrj->ExtractDependencies();
//      }
//      catch (XmlBuildListException) {
//          if (pPrj)
//          {
//              RemovePrj (pPrj);
//              delete pPrj;
//          }
//
//      }
//  }
//}

/*****************************************************************************/
int Star::GetOSType ( ByteString& aWhatOS ) {
/*****************************************************************************/
    int nOSType = OS_NONE;
    if ( aWhatOS == "all" )
        nOSType = ( OS_WIN16 | OS_WIN32 | OS_OS2 | OS_UNX | OS_MAC );
    else if ( aWhatOS == "w" || aWhatOS == "wnt" )
        nOSType = ( OS_WIN16 | OS_WIN32 );
    else if ( aWhatOS == "p" )
        nOSType = OS_OS2;
    else if ( aWhatOS == "u" || aWhatOS == "unx" )
        nOSType = OS_UNX;
    else if ( aWhatOS == "d" )
        nOSType = OS_WIN16;
    else if ( aWhatOS == "n" )
        nOSType = OS_WIN32;
    else if ( aWhatOS == "m" || aWhatOS == "mac" )
        nOSType = OS_MAC;
    return nOSType;

};

/*****************************************************************************/
int Star::GetJobType ( ByteString& JobType ) {
/*****************************************************************************/
    int nCommandType = 0;
    if ( JobType == "nmake" || JobType == "make")
        nCommandType = COMMAND_NMAKE;
    else if ( JobType == "get" )
        nCommandType = COMMAND_GET;
    else {
        sal_uIntPtr nOffset = JobType.Copy( 3 ).ToInt32();
        nCommandType = COMMAND_USER_START + nOffset - 1;
    }
    return nCommandType;
};

/*****************************************************************************/
void Star::PutPrjIntoStream (SByteStringList* pPrjNameList, SvStream* pStream)
/*****************************************************************************/
{
    aMutex.acquire();
    *pStream << sal_False; // not full Star / only some Projects

    sal_uIntPtr nCount_l = pPrjNameList->Count();
    *pStream << nCount_l;
    ByteString* pStr = pPrjNameList->First();
    while (pStr) {
        Prj* pPrj = GetPrj (*pStr);
        *pPrj >> *pStream;
        pStr = pPrjNameList->Next();
    }
    aMutex.release();
}

/*****************************************************************************/
Star& Star::operator>>  ( SvStream& rStream )
/*****************************************************************************/
{
    aMutex.acquire();
    rStream << sal_True; // full Star
    rStream << nStarMode;
    if (pDepMode)
    {
        rStream << sal_True;
        *pDepMode >> rStream;
    }
    else
        rStream << sal_False;

    sal_uIntPtr nCount_l = Count();
    rStream << nCount_l;
    Prj* pPrj = First();
    while (pPrj) {
        *pPrj >> rStream;
        pPrj = Next();
    }
    aMutex.release();

    return *this;
}

/*****************************************************************************/
Star& Star::operator<<  ( SvStream& rStream )
/*****************************************************************************/
{
    aMutex.acquire();
    sal_Bool bFullList;
    rStream >> bFullList;
    if (bFullList)
    {
        rStream >> nStarMode;
        sal_Bool bDepMode;
        rStream >> bDepMode;
        if (pDepMode)
            pDepMode->CleanUp();
        if (bDepMode)
        {
            if (!pDepMode)
                pDepMode = new SByteStringList();
            *pDepMode << rStream;
        }
        else
            DELETEZ (pDepMode);

    }
    sal_uIntPtr nCount_l;
    rStream >> nCount_l;
    for ( sal_uInt16 i = 0; i < nCount_l; i++ ) {
        Prj* pPrj = new Prj();
        *pPrj << rStream;
        pPrj->SetMode(pDepMode);
        if (HasProject (pPrj->GetProjectName())) {
            Prj* pTmpPrj = GetPrj( pPrj->GetProjectName() );
            Replace (pPrj, pTmpPrj);
            delete pTmpPrj;
        }
        else
            Insert (pPrj, LIST_APPEND);
    }
    Expand_Impl();
    aMutex.release();
    return *this;
}



//
//  class StarWriter
//

/*****************************************************************************/
StarWriter::StarWriter( String aFileName, sal_Bool bReadComments, sal_uInt16 nMode )
/*****************************************************************************/
                : Star ()
{
    sFileName = aFileName;
    Read ( aFileName, bReadComments, nMode );
}

/*****************************************************************************/
StarWriter::StarWriter( SolarFileList *pSolarFiles, sal_Bool bReadComments )
/*****************************************************************************/
                : Star ()
{
    Read( pSolarFiles, bReadComments );
}

/*****************************************************************************/
StarWriter::StarWriter( GenericInformationList *pStandLst, ByteString &rVersion,
    ByteString &rMinor, sal_Bool bReadComments )
/*****************************************************************************/
                : Star ()
{
    ByteString sPath( rVersion );

#ifdef UNX
    sPath += "/settings/UNXSOLARLIST";
#else
    sPath += "/settings/SOLARLIST";
#endif
    GenericInformation *pInfo_l = pStandLst->GetInfo( sPath, sal_True );

    if( pInfo_l && pInfo_l->GetValue().Len()) {
        ByteString sFile( pInfo_l->GetValue());
        String sFileName_l( sFile, RTL_TEXTENCODING_ASCII_US );
        nStarMode = STAR_MODE_SINGLE_PARSE;
        Read( sFileName_l, bReadComments );
    }
    else {
        SolarFileList *pFileList = new SolarFileList();

        sPath = rVersion;
        sPath += "/drives";

        GenericInformation *pInfo_k = pStandLst->GetInfo( sPath, sal_True );
        if ( pInfo_k && pInfo_k->GetSubList())  {
            GenericInformationList *pDrives = pInfo_k->GetSubList();
            for ( sal_uIntPtr i = 0; i < pDrives->Count(); i++ ) {
                GenericInformation *pDrive = pDrives->GetObject( i );
                if ( pDrive ) {
                    DirEntry aEntry;
                    sal_Bool bOk = sal_False;
                    if ( sSourceRoot.Len()) {
                        aEntry = DirEntry( sSourceRoot );
                        bOk = sal_True;
                    }
                    else {
#ifdef UNX
                        sPath = "UnixVolume";
                        GenericInformation *pUnixVolume = pDrive->GetSubInfo( sPath );
                        if ( pUnixVolume ) {
                            String sRoot( pUnixVolume->GetValue(), RTL_TEXTENCODING_ASCII_US );
                            aEntry = DirEntry( sRoot );
                            bOk = sal_True;
                         }
#else
                        bOk = sal_True;
                        String sRoot( *pDrive, RTL_TEXTENCODING_ASCII_US );
                        sRoot += String::CreateFromAscii( "\\" );
                        aEntry = DirEntry( sRoot );
#endif
                    }
                    if ( bOk ) {
                        sPath = "projects";
                        GenericInformation *pProjectsKey = pDrive->GetSubInfo( sPath, sal_True );
                        if ( pProjectsKey ) {
                            if ( !sSourceRoot.Len()) {
                                sPath = rVersion;
                                sPath += "/settings/PATH";
                                GenericInformation *pPath = pStandLst->GetInfo( sPath, sal_True );
                                if( pPath ) {
                                    ByteString sAddPath( pPath->GetValue());
#ifdef UNX
                                    sAddPath.SearchAndReplaceAll( "\\", "/" );
#else
                                    sAddPath.SearchAndReplaceAll( "/", "\\" );
#endif
                                     //If Minor has been set add it to path
                                    if (rMinor.Len()>0) {
                                        sAddPath += ".";
                                        sAddPath += rMinor;
                                    }
                                    String ssAddPath( sAddPath, RTL_TEXTENCODING_ASCII_US );

                                    aEntry += DirEntry( ssAddPath );
                                }
                            }
                            sPath = rVersion;
                            sPath += "/settings/SHORTPATH";
                            GenericInformation *pShortPath = pStandLst->GetInfo( sPath, sal_True );
                            sal_Bool bShortPath = sal_False;
                            if (pShortPath && (pShortPath->GetValue() == "_TRUE"))
                                bShortPath = sal_True;
                            sSourceRoot = aEntry.GetFull();
                            GenericInformationList *pProjects = pProjectsKey->GetSubList();
                            if ( pProjects ) {
                                String sPrjDir( String::CreateFromAscii( "prj" ));
                                String sSolarFile( String::CreateFromAscii( "build.lst" ));

                                GenericInformation * pProject = pProjects->First();
                                while (pProject) {
                                    ByteString sProject( *pProject);
                                    String ssProject( sProject, RTL_TEXTENCODING_ASCII_US );

                                    DirEntry aPrjEntry( aEntry );

                                    ByteString aDirStr ("Directory");
                                    GenericInformation * pDir = pProject->GetSubInfo (aDirStr);
                                    if (pDir) {
                                        ByteString aDir = pDir->GetValue();
                                        if (bShortPath)
                                            aPrjEntry = aEntry;
                                        else
                                            aPrjEntry = aEntry.GetPath();
                                        aPrjEntry += DirEntry(aDir);
                                    }

                                    aPrjEntry += DirEntry( ssProject );
                                    aPrjEntry += DirEntry( sPrjDir );
                                    aPrjEntry += DirEntry( sSolarFile );

                                    pFileList->Insert( new String( aPrjEntry.GetFull()), LIST_APPEND );

                                    ByteString sFile( aPrjEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
                                    fprintf( stdout, "%s\n", sFile.GetBuffer());
                                    pProject = pProjects->Next();
                                }
                            }
                        }
                    }
                }
            }
        }
        Read( pFileList, bReadComments );
    }
}

/*****************************************************************************/
void StarWriter::CleanUp()
/*****************************************************************************/
{
    Expand_Impl();
}

/*****************************************************************************/
sal_uInt16 StarWriter::Read( String aFileName, sal_Bool bReadComments, sal_uInt16 nMode  )
/*****************************************************************************/
{
    sFileName = aFileName;

    nStarMode = nMode;

    ByteString aString;
    aFileList.Insert( new String( aFileName ));

    DirEntry aEntry( aFileName );
    aEntry.ToAbs();
    aEntry = aEntry.GetPath().GetPath().GetPath();
    sSourceRoot = aEntry.GetFull();

    while( aFileList.Count()) {
        String ssFileName = *aFileList.GetObject(( sal_uIntPtr ) 0 );
        StarFile* pFile = ReadBuildlist (ssFileName, bReadComments, sal_False);
        aMutex.acquire();
        aLoadedFilesList.Insert( pFile, LIST_APPEND );
        aMutex.release();
        delete aFileList.Remove(( sal_uIntPtr ) 0 );
    }
    // resolve all dependencies recursive
    Expand_Impl();

    // Die gefundenen Abhaengigkeiten rekursiv aufloesen
    Expand_Impl();
    return 0;
}

/*****************************************************************************/
sal_uInt16 StarWriter::Read( SolarFileList *pSolarFiles, sal_Bool bReadComments )
/*****************************************************************************/
{
    nStarMode = STAR_MODE_MULTIPLE_PARSE;

    // this ctor is used by StarBuilder to get the information for the whole workspace
    while(  pSolarFiles->Count()) {
        ByteString aString;
        String ssFileName = *pSolarFiles->GetObject(( sal_uIntPtr ) 0 );
        StarFile* pFile = ReadBuildlist(ssFileName, bReadComments, sal_False);
        aMutex.acquire();
        aLoadedFilesList.Insert( pFile, LIST_APPEND );
        aMutex.release();
        delete pSolarFiles->Remove(( sal_uIntPtr ) 0 );
    }
    delete pSolarFiles;

    Expand_Impl();
    return 0;
}

/*****************************************************************************/
sal_uInt16 StarWriter::WritePrj( Prj *pPrj, SvFileStream& rStream )
/*****************************************************************************/
{
    ByteString aDataString;
    ByteString aTab('\t');
    ByteString aSpace(' ');
    ByteString aEmptyString("");
    SByteStringList* pCmdDepList;
    SByteStringList* pPrjDepList;

    CommandData* pCmdData = NULL;
    if ( pPrj->Count() > 0 )
    {
        pCmdData = pPrj->First();
        if ( (pPrjDepList = pPrj->GetDependencies( sal_False )) )
        {
            aDataString = pPrj->GetPreFix();
            aDataString += aTab;
            aDataString += pPrj->GetProjectName();
            aDataString += aTab;
            if ( pPrj->HasFixedDependencies())
                aDataString+= ByteString(":::");
            else if ( pPrj->HasHardDependencies())
                aDataString+= ByteString("::");
            else
                aDataString+= ByteString(":");
            aDataString += aTab;
            for ( sal_uInt16 i = 0; i< pPrjDepList->Count(); i++ ) {
                aDataString += *pPrjDepList->GetObject( i );
                aDataString += aSpace;
            }
            aDataString+= "NULL";

            rStream.WriteLine( aDataString );

            pCmdData = pPrj->Next();
        }
        if ( pCmdData ) {
            do
            {
                if (( aDataString = pCmdData->GetComment()) == aEmptyString )
                {
                    aDataString = pPrj->GetPreFix();
                    aDataString += aTab;

                    aDataString+= pCmdData->GetPath();
                    aDataString += aTab;
                    sal_uInt16 nPathLen = pCmdData->GetPath().Len();
                    if ( nPathLen < 40 )
                        for ( int i = 0; i < 9 - pCmdData->GetPath().Len() / 4 ; i++ )
                            aDataString += aTab;
                    else
                        for ( int i = 0; i < 12 - pCmdData->GetPath().Len() / 4 ; i++ )
                            aDataString += aTab;
                    aDataString += pCmdData->GetCommandTypeString();
                    aDataString += aTab;
                    if ( pCmdData->GetCommandType() == COMMAND_GET )
                        aDataString += aTab;
                    if ( pCmdData->GetCommandPara() == aEmptyString )
                        aDataString+= ByteString("-");
                    else
                        aDataString+= pCmdData->GetCommandPara();
                    aDataString += aTab;
                    aDataString+= pCmdData->GetOSTypeString();
                    if ( pCmdData->GetClientRestriction().Len()) {
                        aDataString += ByteString( "," );
                        aDataString += pCmdData->GetClientRestriction();
                    }
                    aDataString += aTab;
                    aDataString += pCmdData->GetLogFile();
                    aDataString += aSpace;

                    pCmdDepList = pCmdData->GetDependencies();
                    if ( pCmdDepList )
                        for ( sal_uInt16 i = 0; i< pCmdDepList->Count(); i++ ) {
                            aDataString += *pCmdDepList->GetObject( i );
                            aDataString += aSpace;
                    }
                    aDataString += "NULL";
                }

                rStream.WriteLine( aDataString );

                pCmdData = pPrj->Next();
            } while ( pCmdData );
        }
    }
    return 0;
}

/*****************************************************************************/
sal_uInt16 StarWriter::Write( String aFileName )
/*****************************************************************************/
{
    sFileName = aFileName;

    FileStat::SetReadOnlyFlag( DirEntry( aFileName ), sal_False );

    SvFileStream aFileStream;

    aFileStream.Open( aFileName, STREAM_WRITE | STREAM_TRUNC);
    if ( !aFileStream.IsOpen() && aFileIOErrorHdl.IsSet()) {
        String sError( String::CreateFromAscii( "Error: Unable to open \"" ));
        sError += aFileName;
        sError += String::CreateFromAscii( "for writing!" );
        aFileIOErrorHdl.Call( &sError );
    }

    if ( Count() > 0 )
    {
        Prj* pPrj = First();
        do
        {
            WritePrj( pPrj, aFileStream );
            pPrj = Next();
        } while ( pPrj );
    }

    aFileStream.Close();

    return 0;
}

/*****************************************************************************/
sal_uInt16 StarWriter::WriteMultiple( String rSourceRoot )
/*****************************************************************************/
{
    sSourceRoot = rSourceRoot;

    if ( Count() > 0 )
    {
        String sPrjDir( String::CreateFromAscii( "prj" ));
        String sSolarFile( String::CreateFromAscii( "build.lst" ));

        Prj* pPrj = First();
        do
        {
            String sName( pPrj->GetProjectName(), RTL_TEXTENCODING_ASCII_US );

            DirEntry aEntry( rSourceRoot );
            aEntry += DirEntry( sName );
            aEntry += DirEntry( sPrjDir );
            aEntry += DirEntry( sSolarFile );

            FileStat::SetReadOnlyFlag( aEntry, sal_False );

            SvFileStream aFileStream;
            aFileStream.Open( aEntry.GetFull(), STREAM_WRITE | STREAM_TRUNC);

            if ( !aFileStream.IsOpen() && aFileIOErrorHdl.IsSet()) {
                String sError( String::CreateFromAscii( "Error: Unable to open \"" ));
                sError += aEntry.GetFull();
                sError += String::CreateFromAscii( "for writing!" );
                aFileIOErrorHdl.Call( &sError );
            }

              WritePrj( pPrj, aFileStream );

            aFileStream.Close();

            pPrj = Next();
        } while ( pPrj );
    }

    return 0;
}

/*****************************************************************************/
void StarWriter::InsertTokenLine ( const ByteString& rTokenLine )
/*****************************************************************************/
{
    ByteString sProjectName = rTokenLine.GetToken(1,'\t');
    Prj* pPrj = GetPrj (sProjectName); // 0, if Prj not found;
    Star::InsertTokenLine ( rTokenLine, &pPrj, sProjectName, sal_False );
}

/*****************************************************************************/
sal_Bool StarWriter::InsertProject ( Prj* /*pNewPrj*/ )
/*****************************************************************************/
{
    return sal_False;
}

/*****************************************************************************/
Prj* StarWriter::RemoveProject ( ByteString aProjectName )
/*****************************************************************************/
{
    sal_uIntPtr nCount_l = Count();
    Prj* pPrj;
    Prj* pPrjFound = NULL;
    SByteStringList* pPrjDeps;

    for ( sal_uInt16 i = 0; i < nCount_l; i++ )
    {
        pPrj = GetObject( i );
        if ( pPrj->GetProjectName() == aProjectName )
            pPrjFound = pPrj;
        else
        {
            pPrjDeps = pPrj->GetDependencies( sal_False );
            if ( pPrjDeps )
            {
                ByteString* pString;
                sal_uIntPtr nPrjDepsCount = pPrjDeps->Count();
                for ( sal_uIntPtr j = nPrjDepsCount; j > 0; j-- )
                {
                    pString = pPrjDeps->GetObject( j - 1 );
                    if ( pString->GetToken( 0, '.') == aProjectName )
                        pPrjDeps->Remove( pString );
                }
            }
        }
    }

    Remove( pPrjFound );

    return pPrjFound;
}

//
// class StarFile
//

/*****************************************************************************/
StarFile::StarFile( const String &rFile )
/*****************************************************************************/
                : aFileName( rFile )
{
    DirEntry aEntry( aFileName );
    if ( aEntry.Exists()) {
        bExists = sal_True;
        FileStat aStat( aEntry );
        aDate = aStat.DateModified();
        aTime = aStat.TimeModified();
        aDateCreated = aStat.DateCreated();
        aTimeCreated = aStat.TimeCreated();
    }
    else
        bExists = sal_False;
}

/*****************************************************************************/
sal_Bool StarFile::NeedsUpdate()
/*****************************************************************************/
{
    DirEntry aEntry( aFileName );
    if ( aEntry.Exists()) {
        if ( !bExists ) {
            bExists = sal_True;
            return sal_True;
        }
        FileStat aStat( aEntry );
        if (( aStat.DateModified() != aDate ) || ( aStat.TimeModified() != aTime )
            || ( aStat.DateCreated() != aDateCreated ) || ( aStat.TimeCreated() != aTimeCreated ))
            return sal_True;
    }
    return sal_False;
}
