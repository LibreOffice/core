/*************************************************************************
 *
 *  $RCSfile: prj.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: nf $ $Date: 2001-02-14 16:55:59 $
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
#include <stdio.h>
#include <sstring.hxx>

#include "stream.hxx"
#include "geninfo.hxx"
#include "prj.hxx"

#pragma hdrstop

//#define TEST  1

#ifdef MAC
#define putenv(x)
#endif

#if defined(DOS) || defined(WNT) || defined(OS2)
#define LIST_DELIMETER ';'
#else
#ifdef UNX
#define LIST_DELIMETER ':'
#else
#ifdef MAC
#define LIST_DELIMETER ','
#endif
#endif
#endif

#if defined(DOS) || defined(WNT) || defined(OS2) || defined(WIN)
#define PATH_DELIMETER '\\'
#else
#ifdef UNX
#define PATH_DELIMETER '/'
#else
#ifdef MAC
#define PATH_DELIMETER ':'
#endif
#endif
#endif

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
    USHORT iret = 0;
    nLine++;

    aFileStream.ReadLine ( aTmpStr );
    if ( aTmpStr.Search( "#" ) == 0 )
        return "\t";
    aTmpStr = aTmpStr.EraseLeadingChars();
    aTmpStr = aTmpStr.EraseTrailingChars();
    while ( aTmpStr.SearchAndReplace(ByteString(' '),ByteString('\t') ) != STRING_NOTFOUND );
    int nLength = aTmpStr.Len();
    USHORT nPos = 0;
    BOOL bFound = FALSE;
    ByteString aEraseString;
    for ( USHORT i = 0; i<= nLength; i++)
    {
#ifdef MAC
        if ( aTmpStr.GetChar( i ) == '"')
        {
            if ( bFound) bFound = FALSE;
            else bFound = TRUE;
            aTmpStr.SetChar( i, '\t' );
        }
#endif
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
        if (bReadComments )
            return aTmpStr;
        else
            while ( aTmpStr.Search( "#" ) == 0 )
            {
                aFileStream.ReadLine ( aTmpStr );
            }

    aTmpStr = aTmpStr.EraseLeadingChars();
    aTmpStr = aTmpStr.EraseTrailingChars();
//  while ( aTmpStr.SearchAndReplace(String(' '),String('\t') ) != (USHORT)-1 );
    int nLength = aTmpStr.Len();
    USHORT nPos = 0;
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
CommandData* Prj::GetDirectoryList ( USHORT nWhatOS, USHORT nCommand )
/*****************************************************************************/
{
    return (CommandData *)NULL;
}

/*****************************************************************************/
CommandData* Prj::GetDirectoryData( ByteString aLogFileName )
/*****************************************************************************/
{
    CommandData *pData = NULL;
    ULONG nCount = Count();
    for ( ULONG i=0; i<nCount; i++ )
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
    pPrjDepList(0),
    pPrjInitialDepList(0),
    bSorted( FALSE ),
    bHardDependencies( FALSE ),
    bVisited( FALSE )
/*****************************************************************************/
{
}

/*****************************************************************************/
Prj::Prj( ByteString aName ) :
    aProjectName( aName ),
    pPrjDepList(0),
    pPrjInitialDepList(0),
    bSorted( FALSE ),
    bHardDependencies( FALSE ),
    bVisited( FALSE )
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

    Insert( pData );

    return FALSE;
}

/*****************************************************************************/
//
// removes directory and existing dependencies on it
//
CommandData* Prj::RemoveDirectory ( ByteString aLogFileName )
/*****************************************************************************/
{
    ULONG nCount = Count();
    CommandData* pData;
    CommandData* pDataFound = NULL;
    SByteStringList* pDataDeps;

    for ( USHORT i = 0; i < nCount; i++ )
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
                ULONG nDataDepsCount = pDataDeps->Count();
                for ( ULONG j = nDataDepsCount; j > 0; j-- )
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
Star::Star( GenericInformationList *pStandLst, ByteString &rVersion )
/*****************************************************************************/
{
    ByteString sPath( rVersion );
#ifdef UNX
    sPath += "/settings/UNXSOLARLIST";
#else
    sPath += "/settings/SOLARLIST";
#endif
    GenericInformation *pInfo = pStandLst->GetInfo( sPath, TRUE );

    if( pInfo && pInfo->GetValue().Len()) {
        String sFileName( pInfo->GetValue(), RTL_TEXTENCODING_ASCII_US );
        nStarMode = STAR_MODE_SINGLE_PARSE;
        Read( sFileName );
    }
    else {
        SolarFileList *pFileList = new SolarFileList();

        sPath = rVersion;
        sPath += "/drives";

        GenericInformation *pInfo = pStandLst->GetInfo( sPath, TRUE );
        if ( pInfo && pInfo->GetSubList())  {
            GenericInformationList *pDrives = pInfo->GetSubList();
            for ( ULONG i = 0; i < pDrives->Count(); i++ ) {
                GenericInformation *pDrive = pDrives->GetObject( i );
                if ( pDrive ) {
                    DirEntry aEntry;
                    BOOL bOk = FALSE;
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
                    if ( bOk ) {
                        sPath = "projects";
                        GenericInformation *pProjectsKey = pDrive->GetSubInfo( sPath, TRUE );
                        if ( pProjectsKey ) {
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
                            GenericInformationList *pProjects = pProjectsKey->GetSubList();
                            if ( pProjects ) {
                                String sPrjDir( String::CreateFromAscii( "prj" ));
                                String sSolarFile( String::CreateFromAscii( "build.lst" ));

                                for ( ULONG i = 0; i < pProjects->Count(); i++ ) {
                                    ByteString sProject( *pProjects->GetObject( i ));
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
Star::~Star()
/*****************************************************************************/
{
}

/*****************************************************************************/
BOOL Star::NeedsUpdate()
/*****************************************************************************/
{
    for ( ULONG i = 0; i < aLoadedFilesList.Count(); i++ ) {
        DirEntry aEntry( *aLoadedFilesList.GetObject( i ));
        FileStat aStat( aEntry );

        if (( aStat.DateModified() > aDate ) ||
            (( aStat.DateModified() == aDate ) && ( aStat.TimeModified() > aTime )))
            return TRUE;
    }
    return FALSE;
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
        DirEntry aEntry( *aFileList.GetObject(( ULONG ) 0 ));
        if ( aEntry.Exists()) {
            SimpleConfig aSolarConfig( *aFileList.GetObject(( ULONG ) 0 ));
            while (( aString = aSolarConfig.GetNext()) != "" )
                InsertToken (( char * ) aString.GetBuffer());
        }
        aLoadedFilesList.Insert( aFileList.GetObject(( ULONG ) 0 ), LIST_APPEND );
        aFileList.Remove(( ULONG ) 0 );
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

        DirEntry aEntry( *pSolarFiles->GetObject(( ULONG ) 0 ));
        if ( aEntry.Exists()) {
            SimpleConfig aSolarConfig( *pSolarFiles->GetObject(( ULONG ) 0 ));
            while (( aString = aSolarConfig.GetNext()) != "" )
                InsertToken (( char * ) aString.GetBuffer());
        }

        aLoadedFilesList.Insert( pSolarFiles->GetObject(( ULONG ) 0 ),
            LIST_APPEND );
        pSolarFiles->Remove(( ULONG ) 0 );
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

    return aEntry.GetFull();
}

/*****************************************************************************/
void Star::InsertSolarList( String sProject )
/*****************************************************************************/
{
    // inserts a new solarlist part of another project
    String sFileName( CreateFileName( sProject ));

    for ( ULONG i = 0; i < aFileList.Count(); i++ ) {
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

    pDepPrj->bVisited = TRUE;

    SByteStringList* pPrjLst = pPrj->GetDependencies();
    SByteStringList* pDepLst = NULL;
    ByteString* pDepend;
    ByteString* pPutStr;
    Prj *pNextPrj = NULL;
    ULONG i, nRetPos;

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
    static int nCommandType, nOSType;
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
                        nCommandType = COMMAND_USER_START + nOffset - 1;
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
                        nOSType = ( OS_WIN16 | OS_WIN32 | OS_OS2 | OS_UNX | OS_MAC );
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
                    else if ( aWhatOS == "m" )
                        nOSType = OS_MAC;
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
                        aProjectName = aDirName.GetToken ( 0, 0x5c);
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
        aProjectName = aDirName.GetToken ( 0, 0x5c);
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
    int nCount;

    nCount = Count();

    for ( int i=0; i<nCount; i++)
    {
        pPrj = GetObject(i);
        if ( pPrj->GetProjectName().ToLowerAscii() == aProjectName.ToLowerAscii() )
            return TRUE;
    }
    return FALSE;
}

/*****************************************************************************/
Prj* Star::GetPrj ( ByteString aProjectName )
/*****************************************************************************/
{
    Prj* pPrj;
    int nCount = Count();
    for ( int i=0;i<nCount;i++)
    {
        pPrj = GetObject(i);
        if ( pPrj->GetProjectName().ToLowerAscii() == aProjectName.ToLowerAscii() )
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

    USHORT nToken = aFullPathName.GetTokenCount(PATH_DELIMETER);
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
StarWriter::StarWriter( GenericInformationList *pStandLst, ByteString &rVersion )
/*****************************************************************************/
{
    ByteString sPath( rVersion );
#ifdef UNX
    sPath += "/settings/UNXSOLARLIST";
#else
    sPath += "/settings/SOLARLIST";
#endif
    GenericInformation *pInfo = pStandLst->GetInfo( sPath, TRUE );

    if( pInfo && pInfo->GetValue().Len()) {
        String sFileName( pInfo->GetValue(), RTL_TEXTENCODING_ASCII_US );
        nStarMode = STAR_MODE_SINGLE_PARSE;
        Read( sFileName );
    }
    else {
        SolarFileList *pFileList = new SolarFileList();

        sPath = rVersion;
        sPath += "/drives";

        GenericInformation *pInfo = pStandLst->GetInfo( sPath, TRUE );
        if ( pInfo && pInfo->GetSubList())  {
            GenericInformationList *pDrives = pInfo->GetSubList();
            for ( ULONG i = 0; i < pDrives->Count(); i++ ) {
                GenericInformation *pDrive = pDrives->GetObject( i );
                if ( pDrive ) {
                    DirEntry aEntry;
                    BOOL bOk = FALSE;
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
                    if ( bOk ) {
                        sPath = "projects";
                        GenericInformation *pProjectsKey = pDrive->GetSubInfo( sPath, TRUE );
                        if ( pProjectsKey ) {
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
                            GenericInformationList *pProjects = pProjectsKey->GetSubList();
                            if ( pProjects ) {
                                String sPrjDir( String::CreateFromAscii( "prj" ));
                                String sSolarFile( String::CreateFromAscii( "build.lst" ));

                                for ( ULONG i = 0; i < pProjects->Count(); i++ ) {
                                    ByteString sProject( *pProjects->GetObject( i ));
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
USHORT StarWriter::Read( String aFileName, BOOL bReadComments, USHORT nMode  )
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

        DirEntry aEntry( *aFileList.GetObject(( ULONG ) 0 ));
        if ( aEntry.Exists()) {
            SimpleConfig aSolarConfig( *aFileList.GetObject(( ULONG ) 0 ));
            while (( aString = aSolarConfig.GetCleanedNextLine( bReadComments )) != "" )
                InsertTokenLine ( aString );
        }

        aLoadedFilesList.Insert( aFileList.GetObject(( ULONG ) 0 ), LIST_APPEND );
        aFileList.Remove(( ULONG ) 0 );
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
    while(  pSolarFiles->Count()) {
        ByteString aString;

        DirEntry aEntry(  *pSolarFiles->GetObject(( ULONG ) 0 ));
        if ( aEntry.Exists()) {
            SimpleConfig aSolarConfig( *pSolarFiles->GetObject(( ULONG ) 0 ));
            while (( aString = aSolarConfig.GetCleanedNextLine( bReadComments )) != "" )
                InsertTokenLine ( aString );
        }

        aLoadedFilesList.Insert( pSolarFiles->GetObject(( ULONG ) 0 ),
            LIST_APPEND );
        pSolarFiles->Remove(( ULONG ) 0 );
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
    SByteStringList* pPrjDepList;

    CommandData* pCmdData = NULL;
    if ( pPrj->Count() > 0 )
    {
        pCmdData = pPrj->First();
        if ( pPrjDepList = pPrj->GetDependencies( FALSE ))
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
            for ( USHORT i = 0; i< pPrjDepList->Count(); i++ ) {
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
                        for ( USHORT i = 0; i< pCmdDepList->Count(); i++ ) {
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

    aDate = Date();
    aTime = Time();

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

    aDate = Date();
    aTime = Time();

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
    int nCommandType, nOSType;
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
                            nCommandType = COMMAND_USER_START + nOffset - 1;
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
                            nOSType = ( OS_WIN16 | OS_WIN32 | OS_OS2 | OS_UNX | OS_MAC );
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
                        else if ( aWhatOS == "m" )
                            nOSType = OS_MAC;
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
                            aProjectName = aDirName.GetToken ( 0, 0x5c);
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
            aProjectName = aDirName.GetToken ( 0, 0x5c);
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
BOOL StarWriter::InsertProject ( Prj* pNewPrj )
/*****************************************************************************/
{
    return FALSE;
}

/*****************************************************************************/
Prj* StarWriter::RemoveProject ( ByteString aProjectName )
/*****************************************************************************/
{
    ULONG nCount = Count();
    Prj* pPrj;
    Prj* pPrjFound = NULL;
    SByteStringList* pPrjDeps;

    for ( USHORT i = 0; i < nCount; i++ )
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
                ULONG nPrjDepsCount = pPrjDeps->Count();
                for ( ULONG j = nPrjDepsCount; j > 0; j-- )
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

