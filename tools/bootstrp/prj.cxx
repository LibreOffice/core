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
#include <vos/mutex.hxx>

#include <tools/stream.hxx>
#include <tools/geninfo.hxx>
#include "bootstrp/prj.hxx"
#include "bootstrp/inimgr.hxx"

//#define TEST  1

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
//  while ( aTmpStr.SearchAndReplace(String(' '),String('\t') ) != (sal_uInt16)-1 );
    int nLength = aTmpStr.Len();
    ByteString aEraseString;
    sal_Bool bFirstTab = sal_True;
    for ( sal_uInt16 i = 0; i<= nLength; i++)
    {
        if ( aTmpStr.GetChar( i ) == 0x20 )
            aTmpStr.SetChar( i, 0x09 );

        if ( aTmpStr.GetChar( i ) ==  0x09 )
        {
            if ( bFirstTab )
                bFirstTab = sal_False;
            else
            {
                aTmpStr.SetChar( i, 0x20 );
            }
        }
        else
            bFirstTab = sal_True;

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
        ByteString *pString = pDepList->First();
        while ( pString )
        {
            delete pString;
            pString = pDepList->Next();
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
CommandData* Prj::GetDirectoryList ( sal_uInt16, sal_uInt16 )
/*****************************************************************************/
{
    return (CommandData *)NULL;
}

/*****************************************************************************/
CommandData* Prj::GetDirectoryData( ByteString aLogFileName )
/*****************************************************************************/
{
    CommandData *pData = NULL;
    sal_uIntPtr nObjCount = Count();
    for ( sal_uIntPtr i=0; i<nObjCount; i++ )
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
    bVisited( sal_False ),
    pPrjInitialDepList(0),
    pPrjDepList(0),
    bHardDependencies( sal_False ),
    bSorted( sal_False )
/*****************************************************************************/
{
}

/*****************************************************************************/
Prj::Prj( ByteString aName ) :
    bVisited( sal_False ),
    aProjectName( aName ),
    pPrjInitialDepList(0),
    pPrjDepList(0),
    bHardDependencies( sal_False ),
    bSorted( sal_False )
/*****************************************************************************/
{
}

/*****************************************************************************/
Prj::~Prj()
/*****************************************************************************/
{
    if ( pPrjDepList )
    {
        ByteString *pString = pPrjDepList->First();
        while ( pString )
        {
            delete pString;
            pString = pPrjDepList->Next();
        }
        delete pPrjDepList;

        pPrjDepList = NULL;
    }

    if ( pPrjInitialDepList )
    {
        ByteString *pString = pPrjInitialDepList->First();
        while ( pString )
        {
            delete pString;
            pString = pPrjInitialDepList->Next();
        }
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
SByteStringList* Prj::GetDependencies( sal_Bool bExpanded )
/*****************************************************************************/
{
    if ( bExpanded )
        return pPrjDepList;
    else
        return pPrjInitialDepList;
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

    Insert( pData );

    return sal_False;
}

/*****************************************************************************/
//
// removes directory and existing dependencies on it
//
CommandData* Prj::RemoveDirectory ( ByteString aLogFileName )
/*****************************************************************************/
{
    sal_uIntPtr nCountMember = Count();
    CommandData* pData;
    CommandData* pDataFound = NULL;
    SByteStringList* pDataDeps;

    for ( sal_uInt16 i = 0; i < nCountMember; i++ )
    {
        pData = GetObject( i );
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
Star::Star(String aFileName, sal_uInt16 nMode )
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
    sal_Bool bLocal, const char *pSourceRoot )
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
    GenericInformation *pInfo = pStandLst->GetInfo( sPath, sal_True );

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

        GenericInformation *pInfo2 = pStandLst->GetInfo( sPath, sal_True );
        if ( pInfo2 && pInfo2->GetSubList())  {
            GenericInformationList *pDrives = pInfo2->GetSubList();
            for ( sal_uIntPtr i = 0; i < pDrives->Count(); i++ ) {
                GenericInformation *pDrive = pDrives->GetObject( i );
                if ( pDrive ) {
                    DirEntry aEntry;
                    sal_Bool bOk = sal_False;
                    if ( sSrcRoot.Len()) {
                        aEntry = DirEntry( sSrcRoot );
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
                            if ( !sSrcRoot.Len()) {
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
                            GenericInformationList *pProjects = pProjectsKey->GetSubList();
                            if ( pProjects ) {
                                String sPrjDir( String::CreateFromAscii( "prj" ));
                                String sSolarFile( String::CreateFromAscii( "build.lst" ));

                                for ( sal_uIntPtr j = 0; j < pProjects->Count(); j++ ) {
                                    ByteString sProject( *pProjects->GetObject( j ));
                                    String ssProject( sProject, RTL_TEXTENCODING_ASCII_US );

                                    DirEntry aPrjEntry( aEntry );

                                    aPrjEntry += DirEntry( ssProject );
                                    aPrjEntry += DirEntry( sPrjDir );
                                    aPrjEntry += DirEntry( sSolarFile );

                                    pFileList->Insert( new String( aPrjEntry.GetFull()), LIST_APPEND );

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
    ByteString aString;
    aFileList.Insert( new String( rFileName ));

    DirEntry aEntry( rFileName );
    aEntry.ToAbs();
    aEntry = aEntry.GetPath().GetPath().GetPath();
    sSourceRoot = aEntry.GetFull();

    while( aFileList.Count()) {
        StarFile *pFile = new StarFile( *aFileList.GetObject(( sal_uIntPtr ) 0 ));
        if ( pFile->Exists()) {
            SimpleConfig aSolarConfig( *aFileList.GetObject(( sal_uIntPtr ) 0 ));
            while (( aString = aSolarConfig.GetNext()) != "" )
                InsertToken (( char * ) aString.GetBuffer());
        }
        aMutex.acquire();
        aLoadedFilesList.Insert( pFile, LIST_APPEND );
        aMutex.release();
        aFileList.Remove(( sal_uIntPtr ) 0 );
    }
    // resolve all dependencies recursive
    Expand_Impl();
}

/*****************************************************************************/
void Star::Read( SolarFileList *pSolarFiles )
/*****************************************************************************/
{
    while(  pSolarFiles->Count()) {
        ByteString aString;

        StarFile *pFile = new StarFile( *pSolarFiles->GetObject(( sal_uIntPtr ) 0 ));
        if ( pFile->Exists()) {
            SimpleConfig aSolarConfig( *pSolarFiles->GetObject(( sal_uIntPtr ) 0 ));
            while (( aString = aSolarConfig.GetNext()) != "" )
                InsertToken (( char * ) aString.GetBuffer());
        }

        aMutex.acquire();
        aLoadedFilesList.Insert( pFile, LIST_APPEND );
        aMutex.release();
        delete pSolarFiles->Remove(( sal_uIntPtr ) 0 );
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

    for ( sal_uIntPtr i = 0; i < aFileList.Count(); i++ ) {
        if (( *aFileList.GetObject( i )) == sFileName )
            return;
    }

    ByteString ssProject( sProject, RTL_TEXTENCODING_ASCII_US );
    if ( HasProject( ssProject ))
        return;

    aFileList.Insert( new String( sFileName ), LIST_APPEND );
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
    static sal_Bool bPrjDep = sal_False;
    static sal_Bool bHardDep = sal_False;
    static sal_uInt16 nCommandType, nOSType;
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
                    bPrjDep = sal_True;
                    bHardDep = sal_False;
                    i = 9;
                }
                else if ( !strcmp( yytext, "::" ))
                {
                    bPrjDep = sal_True;
                    bHardDep = sal_True;
                    i = 9;
                }
                else
                {
                    bPrjDep = sal_False;
                    bHardDep = sal_False;

                    aWhat = yytext;
                    if ( aWhat == "nmake" )
                        nCommandType = COMMAND_NMAKE;
                    else if ( aWhat == "get" )
                        nCommandType = COMMAND_GET;
                    else {
                        sal_uIntPtr nOffset = aWhat.Copy( 3 ).ToInt32();
                        nCommandType = sal::static_int_cast< sal_uInt16 >(
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
                        bPrjDep= sal_False;
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
sal_Bool Star::HasProject ( ByteString aProjectName )
/*****************************************************************************/
{
    Prj *pPrj;
    int nCountMember;

    nCountMember = Count();

    for ( int i=0; i<nCountMember; i++)
    {
        pPrj = GetObject(i);
        if ( pPrj->GetProjectName().EqualsIgnoreCaseAscii(aProjectName) )
            return sal_True;
    }
    return sal_False;
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
//  return (Prj*)NULL;
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
StarWriter::StarWriter( String aFileName, sal_Bool bReadComments, sal_uInt16 nMode )
/*****************************************************************************/
{
    Read ( aFileName, bReadComments, nMode );
}

/*****************************************************************************/
StarWriter::StarWriter( SolarFileList *pSolarFiles, sal_Bool bReadComments )
/*****************************************************************************/
{
    Read( pSolarFiles, bReadComments );
}

/*****************************************************************************/
StarWriter::StarWriter( GenericInformationList *pStandLst, ByteString &rVersion,
    sal_Bool bLocal, const char *pSourceRoot )
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
    GenericInformation *pInfo = pStandLst->GetInfo( sPath, sal_True );

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

        GenericInformation *pInfo2 = pStandLst->GetInfo( sPath, sal_True );
        if ( pInfo2 && pInfo2->GetSubList())  {
            GenericInformationList *pDrives = pInfo2->GetSubList();
            for ( sal_uIntPtr i = 0; i < pDrives->Count(); i++ ) {
                GenericInformation *pDrive = pDrives->GetObject( i );
                if ( pDrive ) {
                    DirEntry aEntry;
                    sal_Bool bOk = sal_False;
                    if ( sSrcRoot.Len()) {
                        aEntry = DirEntry( sSrcRoot );
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
                            if ( !sSrcRoot.Len()) {
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
                            GenericInformationList *pProjects = pProjectsKey->GetSubList();
                            if ( pProjects ) {
                                String sPrjDir( String::CreateFromAscii( "prj" ));
                                String sSolarFile( String::CreateFromAscii( "build.lst" ));

                                for ( sal_uIntPtr j = 0; j < pProjects->Count(); j++ ) {
                                    ByteString sProject( *pProjects->GetObject( j ));
                                    String ssProject( sProject, RTL_TEXTENCODING_ASCII_US );

                                    DirEntry aPrjEntry( aEntry );

                                    aPrjEntry += DirEntry( ssProject );
                                    aPrjEntry += DirEntry( sPrjDir );
                                    aPrjEntry += DirEntry( sSolarFile );

                                    pFileList->Insert( new String( aPrjEntry.GetFull()), LIST_APPEND );

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
sal_uInt16 StarWriter::Read( String aFileName, sal_Bool bReadComments, sal_uInt16 nMode  )
/*****************************************************************************/
{
    nStarMode = nMode;

    ByteString aString;
    aFileList.Insert( new String( aFileName ));

    DirEntry aEntry( aFileName );
    aEntry.ToAbs();
    aEntry = aEntry.GetPath().GetPath().GetPath();
    sSourceRoot = aEntry.GetFull();

    while( aFileList.Count()) {

        StarFile *pFile = new StarFile( *aFileList.GetObject(( sal_uIntPtr ) 0 ));
        if ( pFile->Exists()) {
            SimpleConfig aSolarConfig( *aFileList.GetObject(( sal_uIntPtr ) 0 ));
            while (( aString = aSolarConfig.GetCleanedNextLine( bReadComments )) != "" )
                InsertTokenLine ( aString );
        }

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

        StarFile *pFile = new StarFile(  *pSolarFiles->GetObject(( sal_uIntPtr ) 0 ));
        if ( pFile->Exists()) {
            SimpleConfig aSolarConfig( *pSolarFiles->GetObject(( sal_uIntPtr ) 0 ));
            while (( aString = aSolarConfig.GetCleanedNextLine( bReadComments )) != "" )
                InsertTokenLine ( aString );
        }

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

    CommandData* pCmdData = NULL;
    if ( pPrj->Count() > 0 )
    {
        pCmdData = pPrj->First();
        SByteStringList* pPrjDepList = pPrj->GetDependencies( sal_False );
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
sal_uInt16 StarWriter::WriteMultiple( String rSourceRoot )
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
    sal_Bool bPrjDep = sal_False;
    sal_Bool bHardDep = sal_False;
    sal_uInt16 nCommandType = 0;
    sal_uInt16 nOSType = 0;
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
                        bPrjDep = sal_True;
                        bHardDep = sal_False;
                        i = 9;
                    }
                    else if ( !strcmp( yytext, "::" ))
                    {
                        bPrjDep = sal_True;
                        bHardDep = sal_True;
                        i = 9;
                    }
                    else
                    {
                        bPrjDep = sal_False;
                        bHardDep = sal_False;

                        aWhat = yytext;
                        if ( aWhat == "nmake" )
                            nCommandType = COMMAND_NMAKE;
                        else if ( aWhat == "get" )
                            nCommandType = COMMAND_GET;
                        else {
                            sal_uIntPtr nOffset = aWhat.Copy( 3 ).ToInt32();
                            nCommandType = sal::static_int_cast< sal_uInt16 >(
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
                            bPrjDep= sal_False;
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
sal_Bool StarWriter::InsertProject ( Prj* )
/*****************************************************************************/
{
    return sal_False;
}

/*****************************************************************************/
Prj* StarWriter::RemoveProject ( ByteString aProjectName )
/*****************************************************************************/
{
    sal_uIntPtr nCountMember = Count();
    Prj* pPrj;
    Prj* pPrjFound = NULL;
    SByteStringList* pPrjDeps;

    for ( sal_uInt16 i = 0; i < nCountMember; i++ )
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
        if (( aStat.DateModified() > aDate ) ||
            (( aStat.DateModified() == aDate ) && ( aStat.TimeModified() > aTime )))
            return sal_True;
    }
    return sal_False;
}

