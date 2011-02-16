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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include <stdlib.h>
#include <stdio.h>
#include "bootstrp/sstring.hxx"
#include <osl/mutex.hxx>

#include <tools/stream.hxx>
#include <tools/geninfo.hxx>
#include "bootstrp/prj.hxx"
#include "bootstrp/inimgr.hxx"

#if defined(WNT) || defined(OS2)
#define LIST_DELIMETER ';'
#define PATH_DELIMETER '\\'
#elif defined UNX
#define LIST_DELIMETER ':'
#define PATH_DELIMETER '/'
#endif

Link Star::aDBNotFoundHdl;

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
      while ((aStringBuffer = GetNextLine()) == "\t") ; //solange bis != "\t"
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
    nLine++;

    aFileStream.ReadLine ( aTmpStr );
    if ( aTmpStr.Search( "#" ) == 0 )
        return "\t";
    aTmpStr = aTmpStr.EraseLeadingChars();
    aTmpStr = aTmpStr.EraseTrailingChars();
    while ( aTmpStr.SearchAndReplace(ByteString(' '),ByteString('\t') ) != STRING_NOTFOUND ) ;
    int nLength = aTmpStr.Len();
    BOOL bFound = FALSE;
    ByteString aEraseString;
    for ( USHORT i = 0; i<= nLength; i++)
    {
        if ( aTmpStr.GetChar( i ) == 0x20  && !bFound )
            aTmpStr.SetChar( i, 0x09 );
    }
    return aTmpStr;
}

/*****************************************************************************/
ByteString SimpleConfig::GetCleanedNextLine( BOOL bReadComments )
/*****************************************************************************/
{

    aFileStream.ReadLine ( aTmpStr );
    if ( aTmpStr.Search( "#" ) == 0 )
        {
        if (bReadComments )
            return aTmpStr;
        else
            while ( aTmpStr.Search( "#" ) == 0 )
            {
                aFileStream.ReadLine ( aTmpStr );
            }
        }

    aTmpStr = aTmpStr.EraseLeadingChars();
    aTmpStr = aTmpStr.EraseTrailingChars();
    int nLength = aTmpStr.Len();
    ByteString aEraseString;
    BOOL bFirstTab = TRUE;
    for ( USHORT i = 0; i<= nLength; i++)
    {
        if ( aTmpStr.GetChar( i ) == 0x20 )
            aTmpStr.SetChar( i, 0x09 );

        if ( aTmpStr.GetChar( i ) ==  0x09 )
        {
            if ( bFirstTab )
                bFirstTab = FALSE;
            else
            {
                aTmpStr.SetChar( i, 0x20 );
            }
        }
        else
            bFirstTab = TRUE;

    }
    aTmpStr.EraseAllChars(' ');
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
}

/*****************************************************************************/
CommandData::~CommandData()
/*****************************************************************************/
{
    if ( pDepList )
    {
        for ( size_t i = 0, n = pDepList->size(); i < n; ++i ) {
            delete (*pDepList)[ i ];
        }
        delete pDepList;

        pDepList = NULL;
    }
}

/*****************************************************************************/
ByteString CommandData::GetOSTypeString()
/*****************************************************************************/
{
    ByteString aRetStr;

    switch (nOSType)
    {
        case OS_WIN16 | OS_WIN32 | OS_OS2 | OS_UNX :
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
CommandData* Prj::GetDirectoryList ( USHORT, USHORT )
/*****************************************************************************/
{
    return (CommandData *)NULL;
}

/*****************************************************************************/
CommandData* Prj::GetDirectoryData( ByteString aLogFileName )
/*****************************************************************************/
{
    CommandData *pData = NULL;
    ULONG nObjCount = Count();
    for ( ULONG i=0; i<nObjCount; i++ )
    {
        pData = GetObject(i);
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
    bVisited( FALSE ),
    pPrjInitialDepList(0),
    pPrjDepList(0),
    bHardDependencies( FALSE ),
    bSorted( FALSE )
/*****************************************************************************/
{
}

/*****************************************************************************/
Prj::Prj( ByteString aName ) :
    bVisited( FALSE ),
    aProjectName( aName ),
    pPrjInitialDepList(0),
    pPrjDepList(0),
    bHardDependencies( FALSE ),
    bSorted( FALSE )
/*****************************************************************************/
{
}

/*****************************************************************************/
Prj::~Prj()
/*****************************************************************************/
{
    if ( pPrjDepList )
    {
        for ( size_t i = 0, n = maList.size(); i < n; ++i )
            delete maList[ i ];
        maList.clear();

        for ( size_t i = 0, n = pPrjDepList->size(); i < n; ++i )
            delete (*pPrjDepList)[ i ];
        delete pPrjDepList;
        pPrjDepList = NULL;
    }

    if ( pPrjInitialDepList )
    {
        for ( size_t i = 0, n = pPrjInitialDepList->size(); i < n; ++i )
            delete (*pPrjInitialDepList)[ i ];
        delete pPrjInitialDepList;
        pPrjInitialDepList = NULL;
    }
}

/*****************************************************************************/
void Prj::AddDependencies( ByteString aStr )
/*****************************************************************************/
{

    // needs dirty flag - not expanded
    if ( !pPrjDepList )
        pPrjDepList = new SByteStringList;

    pPrjDepList->PutString( new ByteString(aStr) );

    if ( !pPrjInitialDepList )
        pPrjInitialDepList = new SByteStringList;

    pPrjInitialDepList->PutString( new ByteString(aStr) );
}

/*****************************************************************************/
SByteStringList* Prj::GetDependencies( BOOL bExpanded )
/*****************************************************************************/
{
    if ( bExpanded )
        return pPrjDepList;
    else
        return pPrjInitialDepList;
}



/*****************************************************************************/
BOOL Prj::InsertDirectory ( ByteString aDirName, USHORT aWhat,
                                USHORT aWhatOS, ByteString aLogFileName,
                                const ByteString &rClientRestriction )
/*****************************************************************************/
{
    CommandData* pData = new CommandData();

    pData->SetPath( aDirName );
    pData->SetCommandType( aWhat );
    pData->SetOSType( aWhatOS );
    pData->SetLogFile( aLogFileName );
    pData->SetClientRestriction( rClientRestriction );

    maList.push_back( pData );

    return FALSE;
}

/*****************************************************************************/
//
// removes directory and existing dependencies on it
//
CommandData* Prj::RemoveDirectory ( ByteString aLogFileName )
/*****************************************************************************/
{
    size_t nCountMember = maList.size();
    CommandData* pData;
    CommandData* pDataFound = NULL;
    SByteStringList* pDataDeps;

    for ( size_t i = 0; i < nCountMember; i++ )
    {
        pData = maList[ i ];
        if ( pData->GetLogFile() == aLogFileName )
            pDataFound = pData;
        else
        {
            pDataDeps = pData->GetDependencies();
            if ( pDataDeps )
            {
                ByteString* pString;
                size_t nDataDepsCount = pDataDeps->size();
                for ( size_t j = nDataDepsCount; j > 0; )
                {
                    pString = (*pDataDeps)[ --j ];
                    if ( pString->GetToken( 0, '.') == aLogFileName )
                        pDataDeps->erase( j );
                }
            }
        }
    }

    Remove( pDataFound );

    return pDataFound;
}

//
//  class Star
//

/*****************************************************************************/
Star::Star()
/*****************************************************************************/
{
    // this ctor is only used by StarWriter
}

/*****************************************************************************/
Star::Star(String aFileName, USHORT nMode )
/*****************************************************************************/
                : nStarMode( nMode )
{
    Read( aFileName );
}

/*****************************************************************************/
Star::Star( SolarFileList *pSolarFiles )
/*****************************************************************************/
                : nStarMode( STAR_MODE_MULTIPLE_PARSE )
{
    // this ctor is used by StarBuilder to get the information for the whole workspace
    Read( pSolarFiles );
}

/*****************************************************************************/
Star::Star( GenericInformationList *pStandLst, ByteString &rVersion,
    BOOL bLocal, const char *pSourceRoot )
/*****************************************************************************/
{
    ByteString sPath( rVersion );
    String sSrcRoot;
    if ( pSourceRoot )
        sSrcRoot = String::CreateFromAscii( pSourceRoot );

#ifdef UNX
    sPath += "/settings/UNXSOLARLIST";
#else
    sPath += "/settings/SOLARLIST";
#endif
    GenericInformation *pInfo = pStandLst->GetInfo( sPath, TRUE );

    if( pInfo && pInfo->GetValue().Len()) {
        ByteString sFile( pInfo->GetValue());
        if ( bLocal ) {
            IniManager aIniManager;
            aIniManager.ToLocal( sFile );
        }
        String sFileName( sFile, RTL_TEXTENCODING_ASCII_US );
        nStarMode = STAR_MODE_SINGLE_PARSE;
        Read( sFileName );
    }
    else {
        SolarFileList *pFileList = new SolarFileList();

        sPath = rVersion;
        sPath += "/drives";

        GenericInformation *pInfo2 = pStandLst->GetInfo( sPath, TRUE );
        if ( pInfo2 && pInfo2->GetSubList())  {
            GenericInformationList *pDrives = pInfo2->GetSubList();
            for ( size_t i = 0; i < pDrives->size(); i++ ) {
                GenericInformation *pDrive = (*pDrives)[ i ];
                if ( pDrive ) {
                    DirEntry aEntry;
                    BOOL bOk = FALSE;
                    if ( sSrcRoot.Len()) {
                        aEntry = DirEntry( sSrcRoot );
                        bOk = TRUE;
                    }
                    else {
#ifdef UNX
                        sPath = "UnixVolume";
                        GenericInformation *pUnixVolume = pDrive->GetSubInfo( sPath );
                        if ( pUnixVolume ) {
                            String sRoot( pUnixVolume->GetValue(), RTL_TEXTENCODING_ASCII_US );
                            aEntry = DirEntry( sRoot );
                            bOk = TRUE;
                         }
#else
                        bOk = TRUE;
                        String sRoot( *pDrive, RTL_TEXTENCODING_ASCII_US );
                        sRoot += String::CreateFromAscii( "\\" );
                        aEntry = DirEntry( sRoot );
#endif
                    }
                    if ( bOk ) {
                        sPath = "projects";
                        GenericInformation *pProjectsKey = pDrive->GetSubInfo( sPath, TRUE );
                        if ( pProjectsKey ) {
                            if ( !sSrcRoot.Len()) {
                                sPath = rVersion;
                                sPath += "/settings/PATH";
                                GenericInformation *pPath = pStandLst->GetInfo( sPath, TRUE );
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
                            GenericInformationList *pProjects = pProjectsKey->GetSubList();
                            if ( pProjects ) {
                                String sPrjDir( String::CreateFromAscii( "prj" ));
                                String sSolarFile( String::CreateFromAscii( "build.lst" ));

                                for ( size_t j = 0; j < pProjects->size(); j++ ) {
                                    ByteString sProject( *(*pProjects)[ j ] );
                                    String ssProject( sProject, RTL_TEXTENCODING_ASCII_US );

                                    DirEntry aPrjEntry( aEntry );

                                    aPrjEntry += DirEntry( ssProject );
                                    aPrjEntry += DirEntry( sPrjDir );
                                    aPrjEntry += DirEntry( sSolarFile );

                                    pFileList->push_back( new String( aPrjEntry.GetFull()) );

                                    ByteString sFile( aPrjEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
                                }
                            }
                        }
                    }
                }
            }
        }
        Read( pFileList );
    }
}

/*****************************************************************************/
Star::~Star()
/*****************************************************************************/
{
    for ( size_t i = 0, n = maStarList.size(); i < n; ++i )
        delete maStarList[ i ];
    maStarList.clear();
}

/*****************************************************************************/
void Star::Read( String &rFileName )
/*****************************************************************************/
{
    ByteString aString;
    aFileList.push_back( new String( rFileName ));

    DirEntry aEntry( rFileName );
    aEntry.ToAbs();
    aEntry = aEntry.GetPath().GetPath().GetPath();
    sSourceRoot = aEntry.GetFull();

    while ( !aFileList.empty() )
    {
        StarFile *pFile = new StarFile( *aFileList.front() );
        if ( pFile->Exists())
        {
            SimpleConfig aSolarConfig( *aFileList.front() );
            while (( aString = aSolarConfig.GetNext()) != "" )
                InsertToken (( char * ) aString.GetBuffer());
        }
        delete pFile;
        delete aFileList.front();
        aFileList.erase( aFileList.begin() );
    }
    // resolve all dependencies recursive
    Expand_Impl();
}

/*****************************************************************************/
void Star::Read( SolarFileList *pSolarFiles )
/*****************************************************************************/
{
    while ( !pSolarFiles->empty() )
    {
        ByteString aString;

        StarFile *pFile = new StarFile( *pSolarFiles->front() );
        if ( pFile->Exists())
        {
            SimpleConfig aSolarConfig( *pSolarFiles->front() );
            while (( aString = aSolarConfig.GetNext()) != "" )
                InsertToken (( char * ) aString.GetBuffer());
        }
        delete pFile;
        delete pSolarFiles->front();
        aFileList.erase( aFileList.begin() );
    }
    delete pSolarFiles;

    Expand_Impl();
}

/*****************************************************************************/
String Star::CreateFileName( String sProject )
/*****************************************************************************/
{
    // this method is used to find solarlist parts of nabours (other projects)
    String sPrjDir( String::CreateFromAscii( "prj" ));
    String sSolarFile( String::CreateFromAscii( "build.lst" ));

    DirEntry aEntry( sSourceRoot );
    aEntry += DirEntry( sProject );
    aEntry += DirEntry( sPrjDir );
    aEntry += DirEntry( sSolarFile );

    if ( !aEntry.Exists() && aDBNotFoundHdl.IsSet())
        aDBNotFoundHdl.Call( &sProject );

    return aEntry.GetFull();
}

/*****************************************************************************/
void Star::InsertSolarList( String sProject )
/*****************************************************************************/
{
    // inserts a new solarlist part of another project
    String sFileName( CreateFileName( sProject ));

    for ( SolarFileList::iterator it = aFileList.begin(); it != aFileList.end(); ++it )
        if ( *(*it) == sFileName )
            return;

    ByteString ssProject( sProject, RTL_TEXTENCODING_ASCII_US );
    if ( HasProject( ssProject ))
        return;

    aFileList.push_back( new String( sFileName ) );
}

/*****************************************************************************/
void Star::ExpandPrj_Impl( Prj *pPrj, Prj *pDepPrj )
/*****************************************************************************/
{
    if ( pDepPrj->bVisited )
        return;

    pDepPrj->bVisited = TRUE;

    SByteStringList* pPrjLst = pPrj->GetDependencies();
    SByteStringList* pDepLst = NULL;
    ByteString* pDepend;
    ByteString* pPutStr;
    Prj *pNextPrj = NULL;
    size_t i, nRetPos;

    if ( pPrjLst ) {
        pDepLst = pDepPrj->GetDependencies();
        if ( pDepLst ) {
            for ( i = 0; i < pDepLst->size(); i++ ) {
                pDepend = (*pDepLst)[ i ];
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
    for ( ULONG i = 0; i < Count(); i++ ) {
        for ( ULONG j = 0; j < Count(); j++ )
            GetObject( j )->bVisited = FALSE;

        Prj* pPrj = GetObject( i );
        ExpandPrj_Impl( pPrj, pPrj );
    }
}

/*****************************************************************************/
void Star::InsertToken ( char *yytext )
/*****************************************************************************/
{
    static int i = 0;
    static ByteString aDirName, aWhat, aWhatOS,
        sClientRestriction, aLogFileName, aProjectName, aPrefix, aCommandPara;
    static BOOL bPrjDep = FALSE;
    static BOOL bHardDep = FALSE;
    static USHORT nCommandType, nOSType;
    CommandData* pCmdData;
    static SByteStringList *pStaticDepList;
    Prj* pPrj;

    switch (i)
    {
        case 0:
                aPrefix = yytext;
                pStaticDepList = 0;
                break;
        case 1:
                    aDirName = yytext;
                break;
        case 2:
                if ( !strcmp( yytext, ":" ))
                {
                    bPrjDep = TRUE;
                    bHardDep = FALSE;
                    i = 9;
                }
                else if ( !strcmp( yytext, "::" ))
                {
                    bPrjDep = TRUE;
                    bHardDep = TRUE;
                    i = 9;
                }
                else
                {
                    bPrjDep = FALSE;
                    bHardDep = FALSE;

                    aWhat = yytext;
                    if ( aWhat == "nmake" )
                        nCommandType = COMMAND_NMAKE;
                    else if ( aWhat == "get" )
                        nCommandType = COMMAND_GET;
                    else {
                        ULONG nOffset = aWhat.Copy( 3 ).ToInt32();
                        nCommandType = sal::static_int_cast< USHORT >(
                            COMMAND_USER_START + nOffset - 1);
                    }
                }
                break;
        case 3:
                if ( !bPrjDep )
                {
                    aWhat = yytext;
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
                    aWhatOS = yytext;
                    if ( aWhatOS.GetTokenCount( ',' ) > 1 ) {
                        sClientRestriction = aWhatOS.Copy( aWhatOS.GetToken( 0, ',' ).Len() + 1 );
                        aWhatOS = aWhatOS.GetToken( 0, ',' );
                    }
                    if ( aWhatOS == "all" )
                        nOSType = ( OS_WIN16 | OS_WIN32 | OS_OS2 | OS_UNX );
                    else if ( aWhatOS == "w" )
                        nOSType = ( OS_WIN16 | OS_WIN32 );
                    else if ( aWhatOS == "p" )
                        nOSType = OS_OS2;
                    else if ( aWhatOS == "u" )
                        nOSType = OS_UNX;
                    else if ( aWhatOS == "d" )
                        nOSType = OS_WIN16;
                    else if ( aWhatOS == "n" )
                        nOSType = OS_WIN32;
                    else
                        nOSType = OS_NONE;
                }
                break;
        case 5:
                if ( !bPrjDep )
                {
                    aLogFileName = yytext;
                }
                break;
        default:
                if ( !bPrjDep )
                {
                    ByteString aItem = yytext;
                    if ( aItem == "NULL" )
                    {
                        // Liste zu Ende
                        i = -1;
                    }
                    else
                    {
                        // ggfs. Dependency liste anlegen und ergaenzen
                        if ( !pStaticDepList )
                            pStaticDepList = new SByteStringList;
                        pStaticDepList->PutString( new ByteString( aItem ));
                    }
                }
                else
                {
                    ByteString aItem = yytext;
                    if ( aItem == "NULL" )
                    {
                        // Liste zu Ende
                        i = -1;
                        bPrjDep= FALSE;
                    }
                    else
                    {
                        aProjectName = aDirName.GetToken ( 0, '\\');
                        if ( HasProject( aProjectName ))
                        {
                            pPrj = GetPrj( aProjectName );
                            // Projekt exist. schon, neue Eintraege anhaengen
                        }
                        else
                        {
                            // neues Project anlegen
                            pPrj = new Prj ( aProjectName );
                            pPrj->SetPreFix( aPrefix );
                            Insert(pPrj,LIST_APPEND);
                        }
                        pPrj->AddDependencies( aItem );
                        pPrj->HasHardDependencies( bHardDep );

                        if ( nStarMode == STAR_MODE_RECURSIVE_PARSE ) {
                            String sItem( aItem, RTL_TEXTENCODING_ASCII_US );
                            InsertSolarList( sItem );
                        }
                    }
                }
                break;
    }
    /* Wenn dieses Project noch nicht vertreten ist, in die Liste
       der Solar-Projekte einfuegen */
    if ( i == -1 )
    {
        aProjectName = aDirName.GetToken ( 0, '\\');
        if ( HasProject( aProjectName ))
        {
            pPrj = GetPrj( aProjectName );
            // Projekt exist. schon, neue Eintraege anhaengen
        }
        else
        {
            // neues Project anlegen
            pPrj = new Prj ( aProjectName );
            pPrj->SetPreFix( aPrefix );
            Insert(pPrj,LIST_APPEND);
        }

        pCmdData = new CommandData;
        pCmdData->SetPath( aDirName );
        pCmdData->SetCommandType( nCommandType );
        pCmdData->SetCommandPara( aCommandPara );
        pCmdData->SetOSType( nOSType );
        pCmdData->SetLogFile( aLogFileName );
        pCmdData->SetClientRestriction( sClientRestriction );
        if ( pStaticDepList )
            pCmdData->SetDependencies( pStaticDepList );

        pStaticDepList = 0;
        pPrj->Insert ( pCmdData, LIST_APPEND );
        aDirName ="";
        aWhat ="";
        aWhatOS = "";
        sClientRestriction = "";
        aLogFileName = "";
        nCommandType = 0;
        nOSType = 0;
    }
    i++;

    // und wer raeumt die depLst wieder ab ?
}

/*****************************************************************************/
BOOL Star::HasProject ( ByteString aProjectName )
/*****************************************************************************/
{
    Prj *pPrj;
    int nCountMember;

    nCountMember = Count();

    for ( int i=0; i<nCountMember; i++)
    {
        pPrj = GetObject(i);
        if ( pPrj->GetProjectName().EqualsIgnoreCaseAscii(aProjectName) )
            return TRUE;
    }
    return FALSE;
}

/*****************************************************************************/
Prj* Star::GetPrj ( ByteString aProjectName )
/*****************************************************************************/
{
    Prj* pPrj;
    int nCountMember = Count();
    for ( int i=0;i<nCountMember;i++)
    {
        pPrj = GetObject(i);
        if ( pPrj->GetProjectName().EqualsIgnoreCaseAscii(aProjectName) )
            return pPrj;
    }
    return 0L ;
}

/*****************************************************************************/
ByteString Star::GetPrjName( DirEntry &aPath )
/*****************************************************************************/
{
    ByteString aRetPrj, aDirName;
    ByteString aFullPathName = ByteString( aPath.GetFull(), gsl_getSystemTextEncoding());

    xub_StrLen nToken = aFullPathName.GetTokenCount(PATH_DELIMETER);
    for ( xub_StrLen i=0; i< nToken; i++ )
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


//
//  class StarWriter
//

/*****************************************************************************/
StarWriter::StarWriter( String aFileName, BOOL bReadComments, USHORT nMode )
/*****************************************************************************/
{
    Read ( aFileName, bReadComments, nMode );
}

/*****************************************************************************/
StarWriter::StarWriter( SolarFileList *pSolarFiles, BOOL bReadComments )
/*****************************************************************************/
{
    Read( pSolarFiles, bReadComments );
}

/*****************************************************************************/
StarWriter::StarWriter( GenericInformationList *pStandLst, ByteString &rVersion,
    BOOL bLocal, const char *pSourceRoot )
/*****************************************************************************/
{
    ByteString sPath( rVersion );
    String sSrcRoot;
    if ( pSourceRoot )
        sSrcRoot = String::CreateFromAscii( pSourceRoot );

#ifdef UNX
    sPath += "/settings/UNXSOLARLIST";
#else
    sPath += "/settings/SOLARLIST";
#endif
    GenericInformation *pInfo = pStandLst->GetInfo( sPath, TRUE );

    if( pInfo && pInfo->GetValue().Len()) {
        ByteString sFile( pInfo->GetValue());
        if ( bLocal ) {
            IniManager aIniManager;
            aIniManager.ToLocal( sFile );
        }
        String sFileName( sFile, RTL_TEXTENCODING_ASCII_US );
        nStarMode = STAR_MODE_SINGLE_PARSE;
        Read( sFileName );
    }
    else {
        SolarFileList *pFileList = new SolarFileList();

        sPath = rVersion;
        sPath += "/drives";

        GenericInformation *pInfo2 = pStandLst->GetInfo( sPath, TRUE );
        if ( pInfo2 && pInfo2->GetSubList())  {
            GenericInformationList *pDrives = pInfo2->GetSubList();
            for ( size_t i = 0; i < pDrives->size(); i++ ) {
                GenericInformation *pDrive = (*pDrives)[ i ];
                if ( pDrive ) {
                    DirEntry aEntry;
                    BOOL bOk = FALSE;
                    if ( sSrcRoot.Len()) {
                        aEntry = DirEntry( sSrcRoot );
                        bOk = TRUE;
                    }
                    else {
#ifdef UNX
                        sPath = "UnixVolume";
                        GenericInformation *pUnixVolume = pDrive->GetSubInfo( sPath );
                        if ( pUnixVolume ) {
                            String sRoot( pUnixVolume->GetValue(), RTL_TEXTENCODING_ASCII_US );
                            aEntry = DirEntry( sRoot );
                            bOk = TRUE;
                         }
#else
                        bOk = TRUE;
                        String sRoot( *pDrive, RTL_TEXTENCODING_ASCII_US );
                        sRoot += String::CreateFromAscii( "\\" );
                        aEntry = DirEntry( sRoot );
#endif
                    }
                    if ( bOk ) {
                        sPath = "projects";
                        GenericInformation *pProjectsKey = pDrive->GetSubInfo( sPath, TRUE );
                        if ( pProjectsKey ) {
                            if ( !sSrcRoot.Len()) {
                                sPath = rVersion;
                                sPath += "/settings/PATH";
                                GenericInformation *pPath = pStandLst->GetInfo( sPath, TRUE );
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
                            GenericInformationList *pProjects = pProjectsKey->GetSubList();
                            if ( pProjects ) {
                                String sPrjDir( String::CreateFromAscii( "prj" ));
                                String sSolarFile( String::CreateFromAscii( "build.lst" ));

                                for ( size_t j = 0; j < pProjects->size(); j++ ) {
                                    ByteString sProject( *(*pProjects)[ j ] );
                                    String ssProject( sProject, RTL_TEXTENCODING_ASCII_US );

                                    DirEntry aPrjEntry( aEntry );

                                    aPrjEntry += DirEntry( ssProject );
                                    aPrjEntry += DirEntry( sPrjDir );
                                    aPrjEntry += DirEntry( sSolarFile );

                                    pFileList->push_back( new String( aPrjEntry.GetFull()) );

                                    ByteString sFile( aPrjEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
                                    fprintf( stdout, "%s\n", sFile.GetBuffer());
                                }
                            }
                        }
                    }
                }
            }
        }
        Read( pFileList );
    }
}

/*****************************************************************************/
void StarWriter::CleanUp()
/*****************************************************************************/
{
    Expand_Impl();
}

/*****************************************************************************/
USHORT StarWriter::Read( String aFileName, BOOL bReadComments, USHORT nMode  )
/*****************************************************************************/
{
    nStarMode = nMode;

    ByteString aString;
    aFileList.push_back( new String( aFileName ) );

    DirEntry aEntry( aFileName );
    aEntry.ToAbs();
    aEntry = aEntry.GetPath().GetPath().GetPath();
    sSourceRoot = aEntry.GetFull();

    while ( !aFileList.empty() )
    {
        StarFile *pFile = new StarFile( *aFileList.front() );
        if ( pFile->Exists()) {
            SimpleConfig aSolarConfig( *aFileList.front() );
            while (( aString = aSolarConfig.GetCleanedNextLine( bReadComments )) != "" )
                InsertTokenLine ( aString );
        }
        delete pFile;
        delete aFileList.front();
        aFileList.erase( aFileList.begin() );
    }
    // resolve all dependencies recursive
    Expand_Impl();

    // Die gefundenen Abhaengigkeiten rekursiv aufloesen
    Expand_Impl();
    return 0;
}

/*****************************************************************************/
USHORT StarWriter::Read( SolarFileList *pSolarFiles, BOOL bReadComments )
/*****************************************************************************/
{
    nStarMode = STAR_MODE_MULTIPLE_PARSE;

    // this ctor is used by StarBuilder to get the information for the whole workspace
    while ( !pSolarFiles->empty() )
    {
        ByteString aString;
        StarFile *pFile = new StarFile( *pSolarFiles->front() );
        if ( pFile->Exists()) {
            SimpleConfig aSolarConfig( *pSolarFiles->front() );
            while (( aString = aSolarConfig.GetCleanedNextLine( bReadComments )) != "" )
                InsertTokenLine ( aString );
        }
        delete pFile;
        delete pSolarFiles->front();
        pSolarFiles->erase( pSolarFiles->begin() );
    }
    delete pSolarFiles;

    Expand_Impl();
    return 0;
}

/*****************************************************************************/
USHORT StarWriter::WritePrj( Prj *pPrj, SvFileStream& rStream )
/*****************************************************************************/
{
    ByteString aDataString;
    ByteString aTab('\t');
    ByteString aSpace(' ');
    ByteString aEmptyString("");
    SByteStringList* pCmdDepList;

    CommandData* pCmdData = NULL;
    if ( pPrj->Count() > 0 )
    {
        pCmdData = pPrj->First();
        SByteStringList* pPrjDepList = pPrj->GetDependencies( FALSE );
        if ( pPrjDepList != 0 )
        {
            aDataString = pPrj->GetPreFix();
            aDataString += aTab;
            aDataString += pPrj->GetProjectName();
            aDataString += aTab;
            if ( pPrj->HasHardDependencies())
                aDataString+= ByteString("::");
            else
                aDataString+= ByteString(":");
            aDataString += aTab;
            for ( size_t i = 0; i< pPrjDepList->size(); i++ ) {
                aDataString += *(*pPrjDepList)[ i ];
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
                    USHORT nPathLen = pCmdData->GetPath().Len();
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
                        for ( size_t i = 0; i< pCmdDepList->size(); i++ ) {
                            aDataString += *(*pCmdDepList)[ i ];
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
USHORT StarWriter::Write( String aFileName )
/*****************************************************************************/
{
    SvFileStream aFileStream;

    aFileStream.Open( aFileName, STREAM_WRITE | STREAM_TRUNC);

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
USHORT StarWriter::WriteMultiple( String rSourceRoot )
/*****************************************************************************/
{
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

            SvFileStream aFileStream;
            aFileStream.Open( aEntry.GetFull(), STREAM_WRITE | STREAM_TRUNC);

              WritePrj( pPrj, aFileStream );

            aFileStream.Close();

            pPrj = Next();
        } while ( pPrj );
    }

    return 0;
}

/*****************************************************************************/
void StarWriter::InsertTokenLine ( ByteString& rString )
/*****************************************************************************/
{
    int i = 0;
    ByteString aWhat, aWhatOS,
        sClientRestriction, aLogFileName, aProjectName, aPrefix, aCommandPara;
    static  ByteString aDirName;
    BOOL bPrjDep = FALSE;
    BOOL bHardDep = FALSE;
    USHORT nCommandType = 0;
    USHORT nOSType = 0;
    CommandData* pCmdData;
    SByteStringList *pDepList2 = NULL;
    Prj* pPrj;

    ByteString aEmptyString;
    ByteString aToken = rString.GetToken( 0, '\t' );
    ByteString aCommentString;

    const char* yytext = aToken.GetBuffer();

    while ( !( aToken == aEmptyString ) )
    {
        switch (i)
        {
            case 0:
                    if ( rString.Search( "#" ) == 0 )
                    {
                        i = -1;
                        aCommentString = rString;
                        rString = aEmptyString;
                        if ( Count() == 0 )
                            aDirName = "null_entry" ; //comments at begin of file
                        break;
                    }
                    aPrefix = yytext;
                    pDepList2 = NULL;
                    break;
            case 1:
                        aDirName = yytext;
                    break;
            case 2:
                    if ( !strcmp( yytext, ":" ))
                    {
                        bPrjDep = TRUE;
                        bHardDep = FALSE;
                        i = 9;
                    }
                    else if ( !strcmp( yytext, "::" ))
                    {
                        bPrjDep = TRUE;
                        bHardDep = TRUE;
                        i = 9;
                    }
                    else
                    {
                        bPrjDep = FALSE;
                        bHardDep = FALSE;

                        aWhat = yytext;
                        if ( aWhat == "nmake" )
                            nCommandType = COMMAND_NMAKE;
                        else if ( aWhat == "get" )
                            nCommandType = COMMAND_GET;
                        else {
                            ULONG nOffset = aWhat.Copy( 3 ).ToInt32();
                            nCommandType = sal::static_int_cast< USHORT >(
                                COMMAND_USER_START + nOffset - 1);
                        }
                    }
                    break;
            case 3:
                    if ( !bPrjDep )
                    {
                        aWhat = yytext;
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
                        aWhatOS = yytext;
                        if ( aWhatOS.GetTokenCount( ',' ) > 1 ) {
                            sClientRestriction = aWhatOS.Copy( aWhatOS.GetToken( 0, ',' ).Len() + 1 );
                            aWhatOS = aWhatOS.GetToken( 0, ',' );
                        }
                        aWhatOS = aWhatOS.GetToken( 0, ',' );
                        if ( aWhatOS == "all" )
                            nOSType = ( OS_WIN16 | OS_WIN32 | OS_OS2 | OS_UNX );
                        else if ( aWhatOS == "w" )
                            nOSType = ( OS_WIN16 | OS_WIN32 );
                        else if ( aWhatOS == "p" )
                            nOSType = OS_OS2;
                        else if ( aWhatOS == "u" )
                            nOSType = OS_UNX;
                        else if ( aWhatOS == "d" )
                            nOSType = OS_WIN16;
                        else if ( aWhatOS == "n" )
                            nOSType = OS_WIN32;
                        else
                            nOSType = OS_NONE;
                    }
                    break;
            case 5:
                    if ( !bPrjDep )
                    {
                        aLogFileName = yytext;
                    }
                    break;
            default:
                    if ( !bPrjDep )
                    {
                        ByteString aItem = yytext;
                        if ( aItem == "NULL" )
                        {
                            // Liste zu Ende
                            i = -1;
                        }
                        else
                        {
                            // ggfs. Dependency liste anlegen und ergaenzen
                            if ( !pDepList2 )
                                pDepList2 = new SByteStringList;
                            pDepList2->PutString( new ByteString( aItem ));
                        }
                    }
                    else
                    {
                        ByteString aItem = yytext;
                        if ( aItem == "NULL" )
                        {
                            // Liste zu Ende
                            i = -1;
                            bPrjDep= FALSE;
                        }
                        else
                        {
                            aProjectName = aDirName.GetToken ( 0, '\\');
                            if ( HasProject( aProjectName ))
                            {
                                pPrj = GetPrj( aProjectName );
                                // Projekt exist. schon, neue Eintraege anhaengen
                            }
                            else
                            {
                                // neues Project anlegen
                                pPrj = new Prj ( aProjectName );
                                pPrj->SetPreFix( aPrefix );
                                Insert(pPrj,LIST_APPEND);
                            }
                            pPrj->AddDependencies( aItem );
                            pPrj->HasHardDependencies( bHardDep );

                            if ( nStarMode == STAR_MODE_RECURSIVE_PARSE ) {
                                String sItem( aItem, RTL_TEXTENCODING_ASCII_US );
                                InsertSolarList( sItem );
                            }
                        }

                    }
                    break;
        }
        /* Wenn dieses Project noch nicht vertreten ist, in die Liste
           der Solar-Projekte einfuegen */
        if ( i == -1 )
        {
            aProjectName = aDirName.GetToken ( 0, '\\');
            if ( HasProject( aProjectName ))
            {
                pPrj = GetPrj( aProjectName );
                // Projekt exist. schon, neue Eintraege anhaengen
            }
            else
            {
                // neues Project anlegen
                pPrj = new Prj ( aProjectName );
                pPrj->SetPreFix( aPrefix );
                Insert(pPrj,LIST_APPEND);
            }

            pCmdData = new CommandData;
            pCmdData->SetPath( aDirName );
            pCmdData->SetCommandType( nCommandType );
            pCmdData->SetCommandPara( aCommandPara );
            pCmdData->SetOSType( nOSType );
            pCmdData->SetLogFile( aLogFileName );
            pCmdData->SetComment( aCommentString );
            pCmdData->SetClientRestriction( sClientRestriction );
            if ( pDepList2 )
                pCmdData->SetDependencies( pDepList2 );

            pPrj->Insert ( pCmdData, LIST_APPEND );

        }
        i++;

        rString.Erase(0, aToken.Len()+1);
        aToken = rString.GetToken( 0, '\t' );
        yytext = aToken.GetBuffer();

    }
    // und wer raeumt die depLst wieder ab ?
}

/*****************************************************************************/
BOOL StarWriter::InsertProject ( Prj* )
/*****************************************************************************/
{
    return FALSE;
}

/*****************************************************************************/
Prj* StarWriter::RemoveProject ( ByteString aProjectName )
/*****************************************************************************/
{
    size_t nCountMember = Count();
    Prj* pPrj;
    Prj* pPrjFound = NULL;
    SByteStringList* pPrjDeps;

    for ( size_t i = 0; i < nCountMember; i++ )
    {
        pPrj = GetObject( i );
        if ( pPrj->GetProjectName() == aProjectName )
            pPrjFound = pPrj;
        else
        {
            pPrjDeps = pPrj->GetDependencies( FALSE );
            if ( pPrjDeps )
            {
                ByteString* pString;
                size_t nPrjDepsCount = pPrjDeps->size();
                for ( ULONG j = nPrjDepsCount; j > 0; )
                {
                    pString = (*pPrjDeps)[ --j ];
                    if ( pString->GetToken( 0, '.') == aProjectName )
                        pPrjDeps->erase( j );
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
        bExists = TRUE;
        FileStat aStat( aEntry );
        aDate = aStat.DateModified();
        aTime = aStat.TimeModified();
    }
    else
        bExists = FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
