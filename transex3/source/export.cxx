/*************************************************************************
 *
 *  $RCSfile: export.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nf $ $Date: 2000-11-22 12:57:00 $
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
#include <stdio.h>
#include <stdlib.h>
#include <tools/fsys.hxx>
#include <bootstrp/appdef.hxx>
#include "export.hxx"
#include "wrdtrans.hxx"
#include "tokens.h"

extern "C" { yyerror( char * ); }
extern "C" { YYWarning( char * ); }

Export *pExport = 0L;

// defines to parse command line
#define STATE_NON       0x0001
#define STATE_INPUT     0x0002
#define STATE_OUTPUT    0x0003
#define STATE_PRJ       0x0004
#define STATE_ROOT      0x0005
#define STATE_MERGESRC  0x0006
#define STATE_ERRORLOG  0x0007
#define STATE_BREAKHELP 0x0008
#define STATE_UNMERGE   0x0009

// set of global variables
DECLARE_LIST( FileList, ByteString * );
FileList aInputFileList;
BOOL bEnableExport;
BOOL bMergeMode;
BOOL bErrorLog;
BOOL bBreakWhenHelpText;
BOOL bUnmerge;
ByteString sPrj;
ByteString sPrjRoot;
ByteString sActFileName;
ByteString sOutputFile;
ByteString sMergeSrc;
MergeDataFile *pMergeDataFile;

extern "C" {
// the whole interface to lexer is in this extern "C" section

/*****************************************************************************/
extern char *GetOutputFile( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = FALSE;
    bMergeMode = FALSE;
    bErrorLog = TRUE;
    bBreakWhenHelpText = FALSE;
    bUnmerge = FALSE;
    sPrj = "";
    sPrjRoot = "";
    sActFileName = "";

    USHORT nState = STATE_NON;
    BOOL bInput = FALSE;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        if (( ByteString( argv[ i ]) == "-i" ) || ( argv[ i ] == "-I" )) {
            nState = STATE_INPUT; // next tokens specifies source files
        }
        else if (( ByteString( argv[ i ]) == "-o" ) || ( argv[ i ] == "-O" )) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if (( ByteString( argv[ i ]) == "-p" ) || ( argv[ i ] == "-P" )) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }
        else if (( ByteString( argv[ i ]) == "-r" ) || ( argv[ i ] == "-R" )) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if (( ByteString( argv[ i ]) == "-m" ) || ( argv[ i ] == "-M" )) {
            nState = STATE_MERGESRC; // next token specifies the merge database
        }
        else if (( ByteString( argv[ i ]) == "-e" ) || ( argv[ i ] == "-E" )) {
            nState = STATE_ERRORLOG;
            bErrorLog = FALSE;
        }
        else if (( ByteString( argv[ i ]) == "-b" ) || ( argv[ i ] == "-B" )) {
            nState = STATE_BREAKHELP;
            bBreakWhenHelpText = TRUE;
        }
        else if (( ByteString( argv[ i ]) == "-u" ) || ( argv[ i ] == "-U" )) {
            nState = STATE_UNMERGE;
            bUnmerge = TRUE;
            bMergeMode = TRUE;
        }
        else {
            switch ( nState ) {
                case STATE_NON: {
                    return NULL;    // no valid command line
                }
                break;
                case STATE_INPUT: {
                    aInputFileList.Insert( new ByteString( argv[ i ]), LIST_APPEND );
                    bInput = TRUE; // min. one source file found
                }
                break;
                case STATE_OUTPUT: {
                    sOutputFile = ByteString( argv[ i ]); // the dest. file
                }
                break;
                case STATE_PRJ: {
                    sPrj = ByteString( argv[ i ]);
                    sPrj.ToLowerAscii(); // the project
                }
                break;
                case STATE_ROOT: {
                    sPrjRoot = ByteString( argv[ i ]); // path to project root
                }
                break;
                case STATE_MERGESRC: {
                    sMergeSrc = ByteString( argv[ i ]);
                    bMergeMode = TRUE; // activate merge mode, cause merge database found
                }
                break;
            }
        }
    }

    if ( bInput ) {
        // command line is valid
        bEnableExport = TRUE;
        char *pReturn = new char[ sOutputFile.Len() + 1 ];
        strcpy( pReturn, sOutputFile.GetBuffer());
        return pReturn;
    }

    // command line is not valid
    return NULL;
}

/*****************************************************************************/
int InitExport( char *pOutput )
/*****************************************************************************/
{
    // instanciate Export
    ByteString sOutput( pOutput );

    if ( bMergeMode && !bUnmerge ) {
        // merge mode enabled, so read database
        pExport = new Export(sOutput, bEnableExport, sPrj, sPrjRoot, sMergeSrc);
    }
    else
        // no merge mode, only export
        pExport = new Export( sOutput, bEnableExport, sPrj, sPrjRoot );
    return 1;
}

/*****************************************************************************/
int EndExport()
/*****************************************************************************/
{
    delete pExport;
    return 1;
}

/*****************************************************************************/
extern FILE *GetNextFile()
/*****************************************************************************/
{
    // look for next valid filename in input file list
    while ( aInputFileList.Count()) {
        ByteString sFileName( *(aInputFileList.GetObject( 0 )));
        delete aInputFileList.GetObject(( ULONG ) 0 );
        aInputFileList.Remove(( ULONG ) 0 );

        // able to open file?
        FILE *pFile = fopen( sFileName.GetBuffer(), "r" );
        if ( !pFile )
            fprintf( stderr, "Error: Could not open File %s\n",
                sFileName.GetBuffer());
        else {
            // this is a valid file which can be opened, so
            // create path to project root
            DirEntry aEntry( String( sFileName, RTL_TEXTENCODING_ASCII_US ));
            aEntry.ToAbs();
            ByteString sFullEntry( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
            aEntry += DirEntry( String( "..", RTL_TEXTENCODING_ASCII_US ));
            aEntry += DirEntry( sPrjRoot );
            ByteString sPrjEntry( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );

            // create file name, beginnig with project root
            // (e.g.: source\ui\src\menue.src)
            sActFileName = sFullEntry.Copy( sPrjEntry.Len() + 1 );
            sActFileName.ToLowerAscii();

            fprintf( stdout, "\nProcessing File %s ...\n", sFileName.GetBuffer());
            if ( pExport ) {
                // create instance of class export
                pExport->Init();
            }
            // return the valid file handle
            return pFile;
        }
    }
    // this means the file could not be opened
    return NULL;
}

/*****************************************************************************/
int WorkOnTokenSet( int nTyp, char *pTokenText )
/*****************************************************************************/
{
    // execute token
    pExport->Execute( nTyp, pTokenText );
    return 1;
}
}

extern "C" {
/*****************************************************************************/
int SetError()
/*****************************************************************************/
{
    // set error at global instance of class Export
    pExport->SetError();
    return 1;
}
}

extern "C" {
/*****************************************************************************/
int GetError()
/*****************************************************************************/
{
    // get error at global instance of class Export
    if ( pExport->GetError())
        return 1;
    return FALSE;
}
}

//
// class ResData
//

/*****************************************************************************/
BOOL ResData::SetId( const ByteString &rId, USHORT nLevel )
/*****************************************************************************/
{
    if ( nLevel > nIdLevel )
    {
        nIdLevel = nLevel;
        sId = rId;

        if ( bChild && bChildWithText ) {
            ByteString sError( "ResId after child definition" );
            yyerror( sError.GetBufferAccess());
            sError.ReleaseBufferAccess();
            SetError();
        }

        if ( sId.Len() > 255 ) {
            ByteString sWarning( "LocalId > 255 chars, truncating..." );
            YYWarning( sWarning.GetBufferAccess());
            sWarning.ReleaseBufferAccess();
            sId.Erase( 255 );
            sId.EraseTrailingChars( ' ' );
            sId.EraseTrailingChars( '\t' );
        }

        return TRUE;
    }

    return FALSE;
}

//
// class Export
//

/*****************************************************************************/
Export::Export( const ByteString &rOutput, BOOL bWrite,
                const ByteString &rPrj, const ByteString &rPrjRoot )
/*****************************************************************************/
                : nLevel( 0 ),
                nListIndex( 0 ),
                bDefine( FALSE ),
                bNextMustBeDefineEOL( FALSE ),
                nList( LIST_NON ),
                bError( FALSE ),
                bEnableExport( bWrite ),
                sProject( sPrj ),
                sRoot( sPrjRoot ),
                bMergeMode( bUnmerge ),
                bReadOver( FALSE ),
                aCharSet( RTL_TEXTENCODING_MS_1252 ),
                bDontWriteOutput( FALSE ),
                nListLevel( 0 ),
                pWordTransformer( NULL )
{
    // used when export is enabled

    // open output stream
    if ( bEnableExport ) {
        aOutput.Open( String( rOutput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
        aOutput.SetStreamCharSet( RTL_TEXTENCODING_MS_1252 );
//      aOutput.SetTargetCharSet( RTL_TEXTENCODING_MS_1252 );
    }

    // looking in environment for given transformation file for german text
    ByteString sTransformFile( GetEnv( "TRANSEXTRANSFORM" ));
    if ( sTransformFile.Len()) {
        pWordTransformer = new WordTransformer();
        if( !pWordTransformer->LoadWordlist( sTransformFile )) {
            fprintf( stderr, "ERROR: Wordlist %s does not exist!\n" ,
                sTransformFile.GetBuffer());
            delete pWordTransformer;
            pWordTransformer = NULL;
        }
    }
}

/*****************************************************************************/
Export::Export( const ByteString &rOutput, BOOL bWrite,
                const ByteString &rPrj, const ByteString &rPrjRoot,
                const ByteString &rMergeSource )
/*****************************************************************************/
                : nLevel( 0 ),
                nListIndex( 0 ),
                bDefine( FALSE ),
                bNextMustBeDefineEOL( FALSE ),
                nList( LIST_NON ),
                bError( FALSE ),
                bEnableExport( bWrite ),
                sProject( sPrj ),
                sRoot( sPrjRoot ),
                bMergeMode( TRUE ),
                bReadOver( FALSE ),
                sMergeSrc( rMergeSource ),
                aCharSet( RTL_TEXTENCODING_MS_1252 ),
                bDontWriteOutput( FALSE ),
                nListLevel( 0 ),
                pWordTransformer( NULL )
{
    // used when merge is enabled

    // open output stream
    if ( bEnableExport )
        aOutput.Open( String( rOutput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );

    // looking in environment for given transformation file for german text
    ByteString sTransformFile( GetEnv( "TRANSEXTRANSFORM" ));
    if ( sTransformFile.Len()) {
        pWordTransformer = new WordTransformer();
        if( !pWordTransformer->LoadWordlist( sTransformFile )) {
            fprintf( stderr, "ERROR: Wordlist %s does not exist!\n" ,
                sTransformFile.GetBuffer());
            delete pWordTransformer;
            pWordTransformer = NULL;
        }
    }
}

/*****************************************************************************/
void Export::Init()
/*****************************************************************************/
{
    // resets the internal status, used before parseing another file
    sActPForm = "";
    bDefine = FALSE;
    bNextMustBeDefineEOL = FALSE;
    nLevel = 0;
    nList = LIST_NON;
    nListLang = 0;
    nListIndex = 0;
    while ( aResStack.Count()) {
        delete aResStack.GetObject(( ULONG ) 0 );
        aResStack.Remove(( ULONG ) 0 );
    }
}

/*****************************************************************************/
Export::~Export()
/*****************************************************************************/
{
    // close transformer for german text
    if ( pWordTransformer )
        delete pWordTransformer;

    // close output stream
    if ( bEnableExport )
        aOutput.Close();
    while ( aResStack.Count()) {
        delete aResStack.GetObject(( ULONG ) 0 );
        aResStack.Remove(( ULONG ) 0 );
    }

    if ( bMergeMode && !bUnmerge ) {
        if ( !pMergeDataFile )
            pMergeDataFile = new MergeDataFile( sMergeSrc, bErrorLog, aCharSet );

        pMergeDataFile->WriteErrorLog( sActFileName );
        delete pMergeDataFile;
    }
}

/*****************************************************************************/
int Export::Execute( int nToken, char * pToken )
/*****************************************************************************/
{
    ByteString sToken( pToken );
    ByteString sOrig( sToken );
    BOOL bWriteToMerged = bMergeMode;

    if ( nToken == CONDITION ) {
        ByteString sTestToken( pToken );
        sTestToken.EraseAllChars( '\t' );
        sTestToken.EraseAllChars( ' ' );
        if (( !bReadOver ) && ( sTestToken.Search( "#ifndef__RSC_PARSER" ) == 0 ))
            bReadOver = TRUE;
        else if (( bReadOver ) && ( sTestToken.Search( "#endif" ) == 0 ))
            bReadOver = FALSE;
    }
    if ((( nToken < FILTER_LEVEL ) || ( bReadOver )) &&
        (!(( bNextMustBeDefineEOL ) && ( sOrig == "\n" )))) {
        // this tokens are not mandatory for parsing, so ignore them ...
        if ( bMergeMode )
            WriteToMerged( sOrig ); // ... ore whrite them directly to dest.
        return 0;
    }

    ResData *pResData = NULL;
    if ( nLevel ) {
        // res. exists at cur. level
        pResData = aResStack.GetObject( nLevel-1 );
    }
    else if (( nToken != RESSOURCE ) &&
            ( nToken != RESSOURCEEXPR ) &&
            ( nToken != SMALRESSOURCE ) &&
            ( nToken != LEVELUP ) &&
            ( nToken != RSCDEFINE ) &&
            ( nToken != CONDITION ) &&
            ( nToken != PRAGMA ))
    {
        // no res. exists at cur. level so return
        if ( bMergeMode )
            WriteToMerged( sOrig );
        return 0;
    }

    if ( bDefine ) {
        if (( nToken != EMPTYLINE ) && ( nToken != LEVELDOWN ) && ( nToken != LEVELUP )) {
            // cur. res. defined in macro
            if ( bNextMustBeDefineEOL ) {
                if ( nToken != RSCDEFINELEND ) {
                    // end of macro found, so destroy res.
                    bDefine = FALSE;
                    if ( bMergeMode ) {
                        if ( bDontWriteOutput && bUnmerge ) {
                            bDontWriteOutput = FALSE;
                            bNextMustBeDefineEOL = FALSE;
                            bDefine = TRUE;
                        }
                        MergeRest( pResData );
                    }
                    bNextMustBeDefineEOL = FALSE;
                    char *pTkn = "";
                    Execute( LEVELDOWN, pTkn );
                }
                else {
                    // next line also in macro definition
                    bNextMustBeDefineEOL = FALSE;
                    if ( bMergeMode )
                        WriteToMerged( sOrig );
                    return 1;
                }
            }
            else if (( nToken != LISTASSIGNMENT ) && ( nToken != UIENTRIES )){
                // cur. line has macro line end
                ByteString sTmpLine( sToken );
                sTmpLine.EraseAllChars( '\t' ); sTmpLine.EraseAllChars( ' ' );
                if ( sTmpLine.GetChar(( USHORT )( sTmpLine.Len() - 1 )) != '\\' )
                    bNextMustBeDefineEOL = TRUE;
            }
        }
    }

    BOOL bExecuteDown = FALSE;
    if ( nToken != LEVELDOWN ) {
        USHORT nOpen = 0;
        USHORT nClose = 0;
        BOOL bReadOver = FALSE;

        for ( USHORT i = 0; i < sToken.Len(); i++ ) {
            if ( sToken.GetChar( i ) == '\"' )
                bReadOver = !bReadOver;
            if ( !bReadOver && ( sToken.GetChar( i ) == '{' ))
                nOpen++;
        }

        bReadOver = FALSE;
        for ( i = 0; i < sToken.Len(); i++ ) {
            if ( sToken.GetChar( i ) == '\"' )
                bReadOver = !bReadOver;
            if ( !bReadOver && ( sToken.GetChar( i ) == '}' ))
                nClose++;
        }

        if ( nOpen < nClose )
            bExecuteDown = TRUE;
    }

    switch ( nToken ) {
        case RSCDEFINE:
            bDefine = TRUE; // res. defined in macro
        case RESSOURCE:
        case RESSOURCEEXPR: {
            bDontWriteOutput = FALSE;
            if ( nToken != RSCDEFINE )
                bNextMustBeDefineEOL = FALSE;
            // this is the beginning of a new res.
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack.GetObject( nLevel - 2 )->bChild = TRUE;
            }

            // create new instance for this res. and fill mandatory fields
//          if ( pResData )
//              pResData->bChild = TRUE;

            pResData = new ResData( sActPForm, FullId());
            aResStack.Insert( pResData, LIST_APPEND );
            ByteString sBackup( sToken );
            sToken.EraseAllChars( '\n' );
            sToken.EraseAllChars( '{' );
            while( sToken.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
            sToken.EraseTrailingChars( ' ' );
            pResData->sResTyp = sToken.GetToken( 0, ' ' ).ToLowerAscii();
            ByteString sId( sToken.Copy( pResData->sResTyp.Len() + 1 ));
            ByteString sCondition;
            if ( sId.Search( "#" ) != STRING_NOTFOUND ) {
                // between ResTyp, Id and paranthes is a precomp. condition
                sCondition = "#";
                sCondition += sId.GetToken( 1, '#' );
                sId = sId.GetToken( 0, '#' );
            }
            sId = sId.GetToken( 0, '/' );
            CleanValue( sId );
            sId = sId.EraseAllChars( '\t' );
            pResData->SetId( sId, ID_LEVEL_IDENTIFIER );
            if ( sCondition.Len()) {
                ByteString sEmpty( "" );
                Execute( CONDITION, sEmpty.GetBufferAccess());  // execute the
                                                                  // precomp.
                                                                // condition
                sEmpty.ReleaseBufferAccess();
            }
        }
        break;
        case SMALRESSOURCE: {
            bDontWriteOutput = FALSE;
            // this is the beginning of a new res.
            bNextMustBeDefineEOL = FALSE;
            USHORT nIndex = 0;
/*          if ( pResData ) {
                pResData->nChildIndex++;
                nIndex = pResData->nChildIndex;
            } */
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack.GetObject( nLevel - 2 )->bChild = TRUE;
            }

            // create new instance for this res. and fill mandatory fields
//          if ( pResData )
//              pResData->bChild = TRUE;

            pResData = new ResData( sActPForm, FullId());
            aResStack.Insert( pResData, LIST_APPEND );
            sToken.EraseAllChars( '\n' );
            sToken.EraseAllChars( '{' );
            sToken.EraseAllChars( '\t' );
            sToken.EraseAllChars( ' ' );
            sToken.EraseAllChars( '\\' );
            pResData->sResTyp = sToken.ToLowerAscii();
/*          if ( nIndex )
                pResData->SetId( nIndex, ID_LEVEL_AUTOID ); */
        }
        break;
        case LEVELUP: {
            // push
            if ( nList )
                nListLevel++;
            if ( nList )
                break;

            bDontWriteOutput = FALSE;
            ByteString sLowerTyp;
            if ( pResData )
                sLowerTyp = "unknown";
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack.GetObject( nLevel - 2 )->bChild = TRUE;
            }

//          if ( pResData )
//              pResData->bChild = TRUE;

            ResData *pNewData = new ResData( sActPForm, FullId());
            pNewData->sResTyp = sLowerTyp;
            aResStack.Insert( pNewData, LIST_APPEND );
        }
        break;
        case LEVELDOWN: {
            // pop
            if ( !nList  ) {
                bDontWriteOutput = FALSE;
                if ( nLevel ) {
                    if ( bDefine && (nLevel == 1 )) {
                        bDefine = FALSE;
                        bNextMustBeDefineEOL = FALSE;
                    }
                    WriteData( pResData );
                    delete aResStack.GetObject( nLevel - 1 );
                    aResStack.Remove( nLevel - 1 );
                    nLevel--;
                }
            }
            else {
                if ( !nListLevel ) {
                    if ( bMergeMode )
                        MergeRest( pResData, MERGE_MODE_LIST );
                    nList = LIST_NON;
                }
                else
                    nListLevel--;
            }
        }
        break;
        case ASSIGNMENT: {
            bDontWriteOutput = FALSE;
            // interpret different types of assignement
             ByteString sKey = sToken.GetToken( 0, '=' );
            sKey.EraseAllChars( ' ' );
            sKey.EraseAllChars( '\t' );
            ByteString sValue = sToken.GetToken( 1, '=' );
            CleanValue( sValue );
            if ( sKey.ToUpperAscii() == "IDENTIFIER" ) {
                ByteString sId( sValue.EraseAllChars( '\t' ));
                pResData->SetId( sId.EraseAllChars( ' ' ), ID_LEVEL_IDENTIFIER );
            }
            else if ( sKey == "HELPID" ) {
                pResData->sHelpId = sValue;
            }
            else if ( sKey == "STRINGLIST" ) {
                if ( bUnmerge )
                    ( sOrig.SearchAndReplace( "=", "[ GERMAN ] =" ));
                pResData->bList = TRUE;
                nList = LIST_STRING;
                ByteString sLang( "GERMAN" );
                nListLang = GetLangIndex( sLang );
                nListIndex = 0;
                nListLevel = 0;
            }
            else if ( sKey == "FILTERLIST" ) {
                if ( bUnmerge )
                    ( sOrig.SearchAndReplace( "=", "[ GERMAN ] =" ));
                pResData->bList = TRUE;
                nList = LIST_FILTER;
                ByteString sLang( "GERMAN" );
                nListLang = GetLangIndex( sLang );
                nListIndex = 0;
                nListLevel = 0;
            }
            else if ( sKey == "UIENTRIES" ) {
                if ( bUnmerge )
                    ( sOrig.SearchAndReplace( "=", "[ GERMAN ] =" ));
                pResData->bList = TRUE;
                nList = LIST_UIENTRIES;
                ByteString sLang( "GERMAN" );
                nListLang = GetLangIndex( sLang );
                nListIndex = 0;
                nListLevel = 0;
            }
            if (( sToken.Search( "{" ) != STRING_NOTFOUND ) &&
                ( sToken.GetTokenCount( '{' ) > sToken.GetTokenCount( '}' )))
            {
                char *pTkn = "";
                WorkOnTokenSet( LEVELUP, pTkn );
            }
            if ( bUnmerge && nListLang == GetLangIndex( "GERMAN" ) && ListExists( pResData, nList ))
                bDontWriteOutput = TRUE;
         }
        break;
        case UIENTRIES:
        case LISTASSIGNMENT: {
            bDontWriteOutput = FALSE;
            if ( sToken.Search( "[" ) == STRING_NOTFOUND ) {
                if ( bUnmerge )
                    ( sOrig.SearchAndReplace( "=", "[ GERMAN ] =" ));
                 ByteString sKey = sToken.GetToken( 0, '=' );
                sKey.EraseAllChars( ' ' );
                sKey.EraseAllChars( '\t' );
                ByteString sValue = sToken.GetToken( 1, '=' );
                CleanValue( sValue );
                if ( sKey.ToUpperAscii() ==  "STRINGLIST" ) {
                    pResData->bList = TRUE;
                    nList = LIST_STRING;
                    ByteString sLang( "GERMAN" );
                    nListLang = GetLangIndex( sLang );
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if ( sKey == "FILTERLIST" ) {
                    pResData->bList = TRUE;
                    nList = LIST_FILTER;
                    ByteString sLang( "GERMAN" );
                    nListLang = GetLangIndex( sLang );
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if ( sKey ==  "ITEMLIST" ) {
                    pResData->bList = TRUE;
                    nList = LIST_ITEM;
                    ByteString sLang( "GERMAN" );
                    nListLang = GetLangIndex( sLang );
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if ( sKey ==  "UIENTRIES" ) {
                    pResData->bList = TRUE;
                    nList = LIST_UIENTRIES;
                    ByteString sLang( "GERMAN" );
                    nListLang = GetLangIndex( sLang );
                    nListIndex = 0;
                    nListLevel = 0;
                }
                if ( bUnmerge && nListLang == GetLangIndex( "GERMAN" ) && ListExists( pResData, nList ))
                    bDontWriteOutput = TRUE;
            }
            else {
                // new res. is a String- or FilterList
                ByteString sKey = sToken.GetToken( 0, '[' );
                sKey.EraseAllChars( ' ' );
                sKey.EraseAllChars( '\t' );
                if ( sKey.ToUpperAscii() == "STRINGLIST" )
                    nList = LIST_STRING;
                else if ( sKey == "FILTERLIST" )
                    nList = LIST_FILTER;
                else if ( sKey == "ITEMLIST" )
                    nList = LIST_ITEM;
                else if ( sKey == "UIENTRIES" )
                    nList = LIST_UIENTRIES;
                if ( nList ) {
                    ByteString sLang=sToken.GetToken( 1, '[' ).GetToken( 0, ']' );
                    CleanValue( sLang );
                    nListLang = GetLangIndex( sLang );
                    if (( bUnmerge ) && ( nListLang != GERMAN_INDEX ) && ( nListLang != ENGLISH_INDEX ))
                        bDontWriteOutput = TRUE;
                    nListIndex = 0;
                    nListLevel = 0;
                    if ( bUnmerge && nListLang == GetLangIndex( "GERMAN" ) && ListExists( pResData, nList ))
                        bDontWriteOutput = TRUE;
                }
            }
        }
        break;
        case TEXT:
        case _LISTTEXT:
        case LISTTEXT: {
//          bDontWriteOutput = FALSE;
            // this is an entry for a String- or FilterList
            if ( nList ) {
                SetChildWithText();
                ByteString sEntry( sToken.GetToken( 1, '\"' ));
                sEntry = sEntry.Convert( aCharSet, RTL_TEXTENCODING_MS_1252 );
                InsertListEntry( sEntry, sOrig );
                if ( bMergeMode )
                    PrepareTextToMerge( sOrig, nList, nListLang, pResData );
            }
        }
        break;
        case LONGTEXTLINE:
        case TEXTLINE:
            bDontWriteOutput = FALSE;
            if ( nLevel ) {
                CutComment( sToken );

                // this is a text line!!!
                ByteString sKey = sToken.GetToken( 0, '=' ).GetToken( 0, '[' );
                sKey.EraseAllChars( ' ' );
                sKey.EraseAllChars( '\t' );
                ByteString sText( GetText( sToken, nToken ));
                if ( !bMergeMode )
                    sText = sText.Convert( aCharSet, RTL_TEXTENCODING_MS_1252 );
                ByteString sLang( "GERMAN" );
                if ( sToken.GetToken( 0, '=' ).Search( "[" ) != STRING_NOTFOUND ) {
                     sLang = sToken.GetToken( 0, '=' ).GetToken( 1, '[' ).GetToken( 0, ']' );
                    CleanValue( sLang );
                }
                USHORT nLangIndex = GetLangIndex( sLang );
                ByteString sOrigKey = sKey;
                if ( sText.Len()) {
                    if (( sKey.ToUpperAscii() == "TEXT" ) ||
                        ( sKey == "MESSAGE" ) ||
                        ( sKey == "CUSTOMUNITTEXT" ) ||
                        ( sKey == "SLOTNAME" ) ||
                        ( sKey == "UINAME" ))
                    {
                        if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND )
                            ( sOrig.SearchAndReplace( "=", "[ GERMAN ] =" ));
                        SetChildWithText();
                        if ( nLangIndex == GERMAN_INDEX )
                            pResData->SetId( sText, ID_LEVEL_TEXT );
                        pResData->bText = TRUE;
                        pResData->sTextTyp = sOrigKey;
                        if ( bMergeMode ) {
                            PrepareTextToMerge( sOrig, STRING_TYP_TEXT, nLangIndex, pResData );
                            if ( bUnmerge )
                                pResData->sText[ nLangIndex ] = sText;
                        }
                        else {
                            if ( pResData->sText[ nLangIndex ].Len()) {
                                ByteString sError( "Language " );
                                sError += LangName[ nLangIndex ];
                                sError += " defined twice";
                                YYWarning( sError.GetBufferAccess());
                                sError.ReleaseBufferAccess();
                            }
                            pResData->sText[ nLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "HELPTEXT" ) {
                        if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND )
                            ( sOrig.SearchAndReplace( "=", "[ GERMAN ] =" ));
                        SetChildWithText();
                        pResData->bHelpText = TRUE;
                        if ( bBreakWhenHelpText ) {
                            ByteString sError( "\"HelpText\" found in source\n" );
                            YYWarning( sError.GetBufferAccess());
                            sError.ReleaseBufferAccess();
                            SetError();
                        }
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_HELPTEXT, nLangIndex, pResData );
                            if ( bUnmerge )
                                pResData->sHelpText[ nLangIndex ] = sText;
                        else {
                            if ( pResData->sHelpText[ nLangIndex ].Len()) {
                                ByteString sError( "Language " );
                                sError += LangName[ nLangIndex ];
                                sError += " defined twice";
                                YYWarning( sError.GetBufferAccess());
                                sError.ReleaseBufferAccess();
                            }
                            pResData->sHelpText[ nLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "QUICKHELPTEXT" ) {
                        if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND )
                            ( sOrig.SearchAndReplace( "=", "[ GERMAN ] =" ));
                        SetChildWithText();
                        pResData->bQuickHelpText = TRUE;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_QUICKHELPTEXT, nLangIndex, pResData );
                            if ( bUnmerge )
                                pResData->sQuickHelpText[ nLangIndex ] = sText;
                        else {
                            if ( pResData->sQuickHelpText[ nLangIndex ].Len()) {
                                ByteString sError( "Language " );
                                sError += LangName[ nLangIndex ];
                                sError += " defined twice";
                                YYWarning( sError.GetBufferAccess());
                                sError.ReleaseBufferAccess();
                            }
                            pResData->sQuickHelpText[ nLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "TITLE" ) {
                        if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND )
                            ( sOrig.SearchAndReplace( "=", "[ GERMAN ] =" ));
                        SetChildWithText();
                        pResData->bTitle = TRUE;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_TITLE, nLangIndex, pResData );
                            if ( bUnmerge )
                                pResData->sTitle[ nLangIndex ] = sText;
                        else {
                            if ( pResData->sTitle[ nLangIndex ].Len()) {
                                ByteString sError( "Language " );
                                sError += LangName[ nLangIndex ];
                                sError += " defined twice";
                                YYWarning( sError.GetBufferAccess());
                                sError.ReleaseBufferAccess();
                            }
                            pResData->sTitle[ nLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "ACCESSPATH" ) {
                        pResData->SetId( sText, ID_LEVEL_ACCESSPATH );
                    }
                    else if ( sKey == "FIELDNAME" ) {
                        pResData->SetId( sText, ID_LEVEL_FIELDNAME );
                    }
                }
            }
        break;
        case NEWTEXTINRES: {
            bDontWriteOutput = TRUE;
            // this means something like // ### Achtung : Neuer Text ...
            ByteString sLang( "GERMAN" );
            USHORT nLangIndex = GetLangIndex( sLang );
            ByteString sText = sToken.GetToken( 2, ':' ).GetToken( 0, '*' );
            CleanValue( sText );
            if ( sText.Len())
                pResData->sText[ nLangIndex ] = sText;
        }
        break;
        case APPFONTMAPPING: {
            bDontWriteOutput = FALSE;
            // this is a AppfontMapping, so look if its a definition
            // of field size
            ByteString sKey = sToken.GetToken( 0, '=' );
            sKey.EraseAllChars( ' ' );
            sKey.EraseAllChars( '\t' );
            ByteString sMapping = sToken.GetToken( 1, '=' );
            sMapping = sMapping.GetToken( 1, '(' );
            sMapping = sMapping.GetToken( 0, ')' );
            sMapping.EraseAllChars( ' ' );
            sMapping.EraseAllChars( '\t' );
            if ( sKey.ToUpperAscii() == "SIZE" ) {
                pResData->nWidth = sMapping.GetToken( 0, ',' ).ToInt64();
            }
            else if ( sKey == "POSSIZE" ) {
                pResData->nWidth = sMapping.GetToken( 2, ',' ).ToInt64();
            }
        }
        break;
        case RSCDEFINELEND:
            bDontWriteOutput = FALSE;
        break;
        case CONDITION: {
            bDontWriteOutput = FALSE;
            while( sToken.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
            while( sToken.SearchAndReplace( "  ", " " ) != STRING_NOTFOUND ) {};
            ByteString sCondition = sToken.GetToken( 0, ' ' );
            if ( sCondition == "#ifndef" ) {
                sActPForm = "!defined ";
                sActPForm += sToken.GetToken( 1, ' ' );
            }
            else if ( sCondition == "#ifdef" ) {
                sActPForm = "defined ";
                sActPForm += sToken.GetToken( 1, ' ' );
            }
            else if ( sCondition == "#if" ) {
                sActPForm = sToken.Copy( 4 );
                while ( sActPForm.SearchAndReplace( "||", "\\or" ) != STRING_NOTFOUND ) {};
            }
            else if ( sCondition == "#elif" ) {
                sActPForm = sToken.Copy( 6 );
                while ( sActPForm.SearchAndReplace( "||", "\\or" ) != STRING_NOTFOUND ) {};
            }
            else if ( sCondition == "#else" ) {
                sActPForm = sCondition;
            }
            else if ( sCondition == "#endif" ) {
                sActPForm = "";
            }
            else break;
            if ( nLevel ) {
                WriteData( pResData, TRUE );
                pResData->sPForm = sActPForm;
            }
        }
        break;
        case EMPTYLINE : {
            bDontWriteOutput = FALSE;
            if ( bDefine ) {
                bNextMustBeDefineEOL = FALSE;
                bDefine = FALSE;
                char *pTkn = "";
                while ( nLevel )
                    WorkOnTokenSet( LEVELDOWN, pTkn );
            }
        }
        break;
        case PRAGMA : {
            bDontWriteOutput = FALSE;
            while( sToken.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
            while( sToken.SearchAndReplace( "  ", " " ) != STRING_NOTFOUND ) {};
            sToken.EraseLeadingChars( ' ' );
            sToken.EraseTrailingChars( ' ' );

            ByteString sCharset = sToken.GetToken( 1, ' ' );
            ByteString sSet = sToken.GetToken( 2, ' ' );
            if (( sCharset.ToUpperAscii() == "CHARSET_IBMPC" ) ||
                ( sCharset == "RTL_TEXTENCODING_IBM_850" ) ||
                (( sCharset == "CHARSET" ) && ( sSet.ToUpperAscii() == "IBMPC" )))
            {
                aCharSet = RTL_TEXTENCODING_IBM_850;
            }
            else if (( sCharset == "CHARSET_ANSI" ) ||
                ( sCharset == "RTL_TEXTENCODING_MS_1252" ) ||
                (( sCharset == "CHARSET" ) && ( sSet.ToUpperAscii() == "ANSI" )))
            {
                aCharSet = RTL_TEXTENCODING_MS_1252;
            }
        }
        break;
        case TEXTREFID : {
            bDontWriteOutput = TRUE;
             ByteString sKey = sToken.GetToken( 0, '=' ).EraseAllChars( '\t' ).EraseAllChars( ' ' );
            USHORT nRefId = sToken.GetToken( 1, '=' ).GetToken( 0, ';' ).EraseAllChars( '\t' ).EraseAllChars( ' ' ).ToInt32();
            if (( sKey.ToUpperAscii() == "TEXT" ) ||
                ( sKey == "MESSAGE" ) ||
                ( sKey == "CUSTOMUNITTEXT" ) ||
                ( sKey == "SLOTNAME" ) ||
                ( sKey == "UINAME" ))
                    pResData->nTextRefId = nRefId;
            else if ( sKey == "HELPTEXT" )
                pResData->nHelpTextRefId = nRefId;
            else if ( sKey == "QUICKHELPTEXT" )
                pResData->nQuickHelpTextRefId = nRefId;
            else if ( sKey == "TITLE" )
                pResData->nTitleRefId = nRefId;
        }
    }
    if ( bWriteToMerged ) {
        // the current token must be written to dest. without merging
        WriteToMerged( sOrig );
    }

    if ( bExecuteDown ) {
        char *pTkn = "";
        WorkOnTokenSet( LEVELDOWN, pTkn );
    }

    return 1;
}

/*****************************************************************************/
void Export::CutComment( ByteString &rText )
/*****************************************************************************/
{
    if ( rText.Search( "//" ) != STRING_NOTFOUND ) {
        ByteString sWork( rText );
        sWork.SearchAndReplaceAll( "\\\"", "XX" );
        USHORT i = 0;
        BOOL bInner = FALSE;

        while ( i < sWork.Len() - 1 ) {
            if ( sWork.GetChar( i ) == '\"' )
                bInner = !bInner;
            else if
                (( sWork.GetChar( i ) == '/' ) &&
                ( !bInner ) &&
                ( sWork.GetChar( i + 1 ) == '/' ))
            {
                rText.Erase( i );
                return;
            }
            i++;
        }
    }
}

/*****************************************************************************/
BOOL Export::ListExists( ResData *pResData, USHORT nLst )
/*****************************************************************************/
{
    switch ( nLst ) {
        case LIST_STRING: return pResData->pStringList != NULL; break;
        case LIST_FILTER: return pResData->pFilterList != NULL; break;
        case LIST_ITEM: return pResData->pItemList != NULL; break;
        case LIST_UIENTRIES: return pResData->pUIEntries != NULL; break;
    }
    return FALSE;
}

/*****************************************************************************/
USHORT Export::GetLangIndex( const ByteString &rLang )
/*****************************************************************************/
{
    // translation table: LangName <=> Index
    USHORT nLangIndex = 0;
    ByteString sLang( rLang );

    if ( sLang.ToUpperAscii() == "LANGUAGE_USER1" )
        return COMMENT_INDEX;
    else if ( sLang == "ENGLISH_US" )
        return ENGLISH_US_INDEX;
    else if ( sLang == "PORTUGUESE" )
        return PORTUGUESE_INDEX;
    else if ( sLang == "RUSSIAN" )
        return RUSSIAN_INDEX;
    else if ( sLang == "GREEK" )
        return GREEK_INDEX;
    else if ( sLang == "DUTCH" )
        return DUTCH_INDEX;
    else if ( sLang == "FRENCH" )
        return FRENCH_INDEX;
    else if ( sLang == "SPANISH" )
        return SPANISH_INDEX;
    else if ( sLang == "FINNISH" )
        return FINNISH_INDEX;
    else if ( sLang == "HUNGARIAN" )
        return HUNGARIAN_INDEX;
    else if ( sLang == "ITALIAN" )
        return ITALIAN_INDEX;
    else if ( sLang == "CZECH" )
        return CZECH_INDEX;
    else if ( sLang == "SLOVAK" )
        return SLOVAK_INDEX;
    else if ( sLang == "ENGLISH" )
        return ENGLISH_INDEX;
    else if ( sLang == "DANISH" )
        return DANISH_INDEX;
    else if ( sLang == "SWEDISH" )
        return SWEDISH_INDEX;
    else if ( sLang == "NORWEGIAN" )
        return NORWEGIAN_INDEX;
    else if ( sLang == "POLISH" )
        return POLISH_INDEX;
    else if ( sLang == "GERMAN" )
        return GERMAN_INDEX;
    else if ( sLang == "PORTUGUESE_BRAZILIAN" )
        return PORTUGUESE_BRAZILIAN_INDEX;
    else if ( sLang == "JAPANESE" )
        return JAPANESE_INDEX;
    else if ( sLang == "KOREAN" )
        return KOREAN_INDEX;
    else if ( sLang == "CHINESE_SIMPLIFIED" )
        return CHINESE_SIMPLIFIED_INDEX;
    else if ( sLang == "CHINESE_TRADITIONAL" )
        return CHINESE_TRADITIONAL_INDEX;
    else if ( sLang == "TURKISH" )
        return TURKISH_INDEX;
    else if ( sLang == "ARABIC" )
        return ARABIC_INDEX;
    else if ( sLang == "HEBREW" )
        return HEBREW_INDEX;

    return nLangIndex;
}

/*****************************************************************************/
BOOL Export::WriteData( ResData *pResData, BOOL bCreateNew )
/*****************************************************************************/
{
    if ( bMergeMode ) {
        MergeRest( pResData );
        return TRUE;
    }

    if ( bUnmerge )
        return TRUE;
//  fprintf( stdout, "." );

       // mandatory to export: german and eng. and/or enus
    if (( pResData->sText[ GERMAN_INDEX ].Len() &&
            ( pResData->sText[ ENGLISH_US_INDEX ].Len() ||
                pResData->sText[ ENGLISH_INDEX ].Len())) ||
        ( pResData->sHelpText[ GERMAN_INDEX ].Len() &&
            ( pResData->sHelpText[ ENGLISH_US_INDEX ].Len() ||
                pResData->sHelpText[ ENGLISH_INDEX ].Len())) ||
        ( pResData->sQuickHelpText[ GERMAN_INDEX ].Len() &&
            ( pResData->sQuickHelpText[ ENGLISH_US_INDEX ].Len() ||
                pResData->sQuickHelpText[ ENGLISH_INDEX ].Len()))||
        ( pResData->sTitle[ GERMAN_INDEX ].Len() &&
            ( pResData->sTitle[ ENGLISH_US_INDEX ].Len() ||
                pResData->sTitle[ ENGLISH_INDEX ].Len())))
    {
        ByteString sGID = pResData->sGId;
        ByteString sLID;
        if ( !sGID.Len())
            sGID = pResData->sId;
        else
            sLID = pResData->sId;

        ByteString sXText;
        ByteString sXHText;
        ByteString sXQHText;
        ByteString sXTitle;

        Time aTime;
        ByteString sTimeStamp( ByteString::CreateFromInt64( Date().GetDate()));
        sTimeStamp += " ";
        sTimeStamp += ByteString::CreateFromInt32( aTime.GetHour());
        sTimeStamp += ":";
        sTimeStamp += ByteString::CreateFromInt32( aTime.GetMin());
        sTimeStamp += ":";
        sTimeStamp += ByteString::CreateFromInt32( aTime.GetSec());

        for ( USHORT i = 0; i < LANGUAGES; i++ ) {
            if ( LANGUAGE_ALLOWED( i )) {
                if ( i != COMMENT_INDEX ) {
                    if ( pResData->sText[ i ].Len())
                        sXText = pResData->sText[ i ];
                    else {
                        sXText = pResData->sText[ ENGLISH_US_INDEX ];
                        if ( !sXText.Len())
                            sXText = pResData->sText[ ENGLISH_INDEX ];
                        if ( !sXText.Len())
                            sXText = pResData->sText[ GERMAN_INDEX ];
                    }

                    if ( pResData->sHelpText[ i ].Len())
                        sXHText = pResData->sHelpText[ i ];
                    else {
                        sXHText = pResData->sHelpText[ ENGLISH_US_INDEX ];
                        if ( !sXHText.Len())
                            sXHText = pResData->sHelpText[ ENGLISH_INDEX ];
                        if ( !sXText.Len())
                            sXHText = pResData->sHelpText[ GERMAN_INDEX ];
                    }

                    if ( pResData->sQuickHelpText[ i ].Len())
                        sXQHText = pResData->sQuickHelpText[ i ];
                    else {
                        sXQHText = pResData->sQuickHelpText[ ENGLISH_US_INDEX ];
                        if ( !sXQHText.Len())
                            sXQHText = pResData->sQuickHelpText[ ENGLISH_INDEX ];
                        if ( !sXQHText.Len())
                            sXQHText = pResData->sQuickHelpText[ GERMAN_INDEX ];
                    }

                    if ( pResData->sTitle[ i ].Len())
                        sXTitle = pResData->sTitle[ i ];
                    else {
                        sXTitle = pResData->sTitle[ ENGLISH_US_INDEX ];
                        if ( !sXTitle.Len())
                            sXTitle = pResData->sTitle[ ENGLISH_INDEX ];
                        if ( !sXTitle.Len())
                            sXTitle = pResData->sTitle[ GERMAN_INDEX ];
                    }

                    if ( !sXText.Len())
                        sXText = "-";

                    if ( !sXHText.Len()) {
                        if ( pResData->sHelpText[ GERMAN_INDEX ].Len())
                            sXHText = pResData->sHelpText[ GERMAN_INDEX ];
                        else if ( pResData->sHelpText[ ENGLISH_US_INDEX ].Len())
                            sXHText = pResData->sHelpText[ ENGLISH_US_INDEX ];
                        else if ( pResData->sHelpText[ ENGLISH_INDEX ].Len())
                            sXHText = pResData->sHelpText[ ENGLISH_INDEX ];
                    }
                }
                else
                    sXText = pResData->sText[ i ];

                if ( bEnableExport ) {
                    ByteString sOutput( sProject ); sOutput += "\t";
                    if ( sRoot.Len())
                        sOutput += sActFileName;
                    sOutput += "\t0\t";
                    sOutput += pResData->sResTyp; sOutput += "\t";
                    sOutput += sGID; sOutput += "\t";
                    sOutput += sLID; sOutput += "\t";
                    sOutput += pResData->sHelpId; sOutput += "\t";
                    sOutput += pResData->sPForm; sOutput += "\t";
                    sOutput += ByteString::CreateFromInt64( pResData->nWidth ); sOutput += "\t";
                    sOutput += ByteString::CreateFromInt64( LangId[ i ] ); sOutput += "\t";
                    sOutput += sXText; sOutput += "\t";
                    sOutput += sXHText; sOutput += "\t";
                    sOutput += sXQHText; sOutput += "\t";
                    sOutput += sXTitle; sOutput += "\t";
                    sOutput += sTimeStamp;

                    aOutput.WriteLine( sOutput );
                }

                if ( bCreateNew ) {
                    pResData->sText[ i ] = "";
                    pResData->sHelpText[ i ] = "";
                    pResData->sQuickHelpText[ i ] = "";
                    pResData->sTitle[ i ] = "";
                }
            }
        }
    }
    if ( pResData->pStringList ) {
        ByteString sList( "stringlist" );
        WriteExportList( pResData, pResData->pStringList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pStringList = 0;
    }
    if ( pResData->pFilterList ) {
        ByteString sList( "filterlist" );
        WriteExportList( pResData, pResData->pFilterList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pFilterList = 0;
    }
    if ( pResData->pItemList ) {
        ByteString sList( "itemlist" );
        WriteExportList( pResData, pResData->pItemList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pItemList = 0;
    }
    if ( pResData->pUIEntries ) {
        ByteString sList( "uientries" );
        WriteExportList( pResData, pResData->pUIEntries, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pUIEntries = 0;
    }
    return TRUE;
}

/*****************************************************************************/
BOOL Export::WriteExportList( ResData *pResData, ExportList *pExportList,
                        const ByteString &rTyp, BOOL bCreateNew )
/*****************************************************************************/
{
    ByteString sGID = pResData->sGId;
    if ( !sGID.Len())
        sGID = pResData->sId;
    else {
        sGID += ".";
        sGID += pResData->sId;
        sGID.EraseTrailingChars( '.' );
    }

    Time aTime;
    ByteString sTimeStamp( ByteString::CreateFromInt64( Date().GetDate()));
    sTimeStamp += " ";
    sTimeStamp += ByteString::CreateFromInt32( aTime.GetHour());
    sTimeStamp += ":";
    sTimeStamp += ByteString::CreateFromInt32( aTime.GetMin());
    sTimeStamp += ":";
    sTimeStamp += ByteString::CreateFromInt32( aTime.GetSec());

    for ( ULONG i = 0; i < pExportList->Count(); i++ ) {
        ByteString sLID( ByteString::CreateFromInt64( i + 1 ));
        ExportListEntry *pEntry = pExportList->GetObject( i );
        for ( USHORT j = 0;  j < LANGUAGES; j++ ) {
            if ( LANGUAGE_ALLOWED( j )) {
                // mandatory for export: german and eng. and/or enus
                if ((*pEntry)[ GERMAN_INDEX ].Len() &&
                    ((*pEntry)[ ENGLISH_US_INDEX ].Len() ||
                         (*pEntry)[ ENGLISH_INDEX ].Len()))
                {
                    if ( bEnableExport )
                    {
                        ByteString sText((*pEntry)[ GERMAN_INDEX ] );
                        if ((*pEntry)[ j ].Len())
                            sText = (*pEntry)[ j ];

                        ByteString sOutput( sProject ); sOutput += "\t";
                        if ( sRoot.Len())
                            sOutput += sActFileName;
                        sOutput += "\t0\t";
                        sOutput += rTyp; sOutput += "\t";
                        sOutput += sGID; sOutput += "\t";
                        sOutput += sLID; sOutput += "\t\t";
                        sOutput += pResData->sPForm; sOutput += "\t0\t";
                        sOutput += ByteString::CreateFromInt64( LangId[ j ] ); sOutput += "\t";
                        sOutput += sText; sOutput += "\t\t\t\t";
                        sOutput += sTimeStamp;

                        aOutput.WriteLine( sOutput );
                    }
                }
            }
        }
        if ( bCreateNew )
            delete [] pEntry;
    }
    if ( bCreateNew )
        delete pExportList;

    return TRUE;
}

/*****************************************************************************/
ByteString Export::FullId()
/*****************************************************************************/
{
    ByteString sFull;
    if ( nLevel > 1 ) {
        sFull = aResStack.GetObject( 0 )->sId;
        for ( USHORT i = 1; i < nLevel - 1; i++ ) {
            ByteString sToAdd = aResStack.GetObject( i )->sId;
            if ( sToAdd.Len()) {
                sFull += ".";
                sFull += sToAdd;
            }
        }
    }
    if ( sFull.Len() > 255 ) {
        ByteString sError( "GroupId > 255 chars" );
        yyerror( sError.GetBufferAccess());
        sError.ReleaseBufferAccess();
    }

    return sFull;
}

/*****************************************************************************/
void Export::InsertListEntry( const ByteString &rText, const ByteString &rLine )
/*****************************************************************************/
{
    ResData *pResData = aResStack.GetObject( nLevel-1 );

    ExportList *pList = NULL;
    if ( nList == LIST_STRING ) {
        pList = pResData->pStringList;
        if ( !pList ) {
            pResData->pStringList = new ExportList();
            pList = pResData->pStringList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_FILTER ) {
        pList = pResData->pFilterList;
        if ( !pList ) {
            pResData->pFilterList = new ExportList();
            pList = pResData->pFilterList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_ITEM ) {
        pList = pResData->pItemList;
        if ( !pList ) {
            pResData->pItemList = new ExportList();
            pList = pResData->pItemList;
            nListIndex = 0;
        }
    }
    else if ( nList == LIST_UIENTRIES ) {
        pList = pResData->pUIEntries;
        if ( !pList ) {
            pResData->pUIEntries = new ExportList();
            pList = pResData->pUIEntries;
            nListIndex = 0;
        }
    }
    else
        return;

    if ( nListIndex + 1 > pList->Count()) {
        ExportListEntry *pNew = new ExportListEntry[ LANGUAGES + 2];
        (*pNew)[ LIST_REFID ] = ByteString::CreateFromInt32( REFID_NONE );
        pList->Insert( pNew, LIST_APPEND );
    }
    ExportListEntry *pCurEntry = pList->GetObject( nListIndex );
    (*pCurEntry)[ nListLang ] = rText;
    if ( nListLang == GERMAN_INDEX )
        (*pCurEntry)[ GERMAN_LIST_LINE_INDEX ] = rLine;
    nListIndex++;
}

/*****************************************************************************/
void Export::CleanValue( ByteString &rValue )
/*****************************************************************************/
{
    while ( rValue.Len()) {
        if (( rValue.GetChar( 0 ) == ' ' ) || ( rValue.GetChar( 0 ) == '\t' ))
            rValue = rValue.Copy( 1 );
        else
            break;
    }

    if ( rValue.Len()) {
        for ( USHORT i = rValue.Len() - 1; i > 0; i-- ) {
            if (( rValue.GetChar( i ) == ' ' ) || ( rValue.GetChar( i ) == '\t' ) ||
                ( rValue.GetChar( i ) == '\n' ) || ( rValue.GetChar( i ) == ';' ) ||
                ( rValue.GetChar( i ) == '{' ) || ( rValue.GetChar( i ) == '\\' ))
                rValue.Erase( i );
            else
                break;
        }
    }
}


/*****************************************************************************/
ByteString Export::GetText( const ByteString &rSource, USHORT nToken )
/*****************************************************************************/
#define TXT_STATE_NON   0x000
#define TXT_STATE_TEXT  0x001
#define TXT_STATE_MACRO 0x002
{
    ByteString sReturn;
    switch ( nToken ) {
        case TEXTLINE:
        case LONGTEXTLINE: {
            ByteString sTmp( rSource.Copy( rSource.Search( "=" )));
            CleanValue( sTmp );
            sTmp.EraseAllChars( '\n' );

            while ( sTmp.SearchAndReplace( "\\\\\"", "-=<[BSlashBSlashHKom]>=-\"" )
                != STRING_NOTFOUND ) {};
            while ( sTmp.SearchAndReplace( "\\\"", "-=<[Hochkomma]>=-" )
                != STRING_NOTFOUND ) {};
            while ( sTmp.SearchAndReplace( "\\", "-=<[0x7F]>=-" )
                != STRING_NOTFOUND ) {};
            while ( sTmp.SearchAndReplace( "\\0x7F", "-=<[0x7F]>=-" )
                != STRING_NOTFOUND ) {};

            USHORT nStart = 0;
            USHORT nState = TXT_STATE_MACRO;
//          if ( sTmp[( USHORT )0 ] == '\"' ); {
                nState = TXT_STATE_TEXT;
                nStart = 1;
//          }

            for ( USHORT i = nStart; i < sTmp.GetTokenCount( '\"' ); i++ ) {
                ByteString sToken = sTmp.GetToken( i, '\"' );
                if ( sToken.Len()) {
                    if ( nState == TXT_STATE_TEXT ) {
                        sReturn += sToken;
                        nState = TXT_STATE_MACRO;
                    }
                    else {
                        while( sToken.SearchAndReplace( "\t", " " ) !=
                            STRING_NOTFOUND ) {};
                        while( sToken.SearchAndReplace( "  ", " " ) !=
                            STRING_NOTFOUND ) {};
                        sToken.EraseLeadingChars( ' ' );
                        sToken.EraseTrailingChars( ' ' );
                        if ( sToken.Len()) {
                            sReturn += "\\\" ";
                            sReturn += sToken;
                            sReturn += " \\\"";
                        }
                        nState = TXT_STATE_TEXT;
                    }
                }
            }

            while ( sReturn.SearchAndReplace( "-=<[0x7F]>=-", "" )
                != STRING_NOTFOUND ) {};
            while ( sReturn.SearchAndReplace( "-=<[Hochkomma]>=-", "\"" )
                != STRING_NOTFOUND ) {};
            while ( sReturn.SearchAndReplace( "-=<[BSlashBSlashHKom]>=-", "\\\\" )
                != STRING_NOTFOUND ) {};

            // new
            while ( sReturn.SearchAndReplace( "\\\\", "-=<[BSlashBSlash]>=-" )
                != STRING_NOTFOUND ) {};
            while ( sReturn.SearchAndReplace( "-=<[BSlashBSlash]>=-", "\\" )
                != STRING_NOTFOUND ) {};
            // new
        }
        break;
    }
    return sReturn;
}

/*****************************************************************************/
void Export::WriteToMerged( const ByteString &rText )
/*****************************************************************************/
{
    if ( !bDontWriteOutput || !bUnmerge ) {
        ByteString sText( rText );
        while ( sText.SearchAndReplace( " \n", "\n" ) != STRING_NOTFOUND ) {};
        for ( USHORT i = 0; i < sText.Len(); i++ ) {
            if ( sText.GetChar( i ) != '\n' )
                aOutput.Write( ByteString( sText.GetChar( i )).GetBuffer(), 1 );
            else
                aOutput.WriteLine( ByteString());
        }
    }
}

/*****************************************************************************/
void Export::ConvertMergeContent( ByteString &rText, USHORT nTyp )
/*****************************************************************************/
{
    BOOL bNoOpen = ( rText.Search( "\\\"" ) != 0 );
    ByteString sClose( rText.Copy( rText.Len() - 2 ));
    BOOL bNoClose = ( sClose != "\\\"" );

    ByteString sNew;
    for ( USHORT i = 0; i < rText.Len(); i++ ) {
        ByteString sChar( rText.GetChar( i ));
        if ( sChar == "\\" ) {
            if (( i + 1 ) < rText.Len()) {
                ByteString sNext( rText.GetChar( i + 1 ));
                if ( sNext == "\"" ) {
                    sChar = "\"";
                    i++;
                }
                else if ( sNext == "n" ) {
                    sChar = "\\n";
                    i++;
                }
                else if ( sNext == "t" ) {
                    sChar = "\\t";
                    i++;
                }
                else if ( sNext == "\'" ) {
                    sChar = "\\\'";
                    i++;
                }
                else
                    sChar = "\\\\";
            }
            else {
                sChar = "\\\\";
            }
        }
        else if ( sChar == "\"" ) {
            sChar = "\\\"";
        }
        else if ( sChar == "" ) {
            sChar = "\\0x7F";
        }
        sNew += sChar;
    }

    rText = sNew;

/*
    while ( rText.SearchAndReplace( "\\\"", "-=<[Hochkomma]>=-" )
        != STRING_NOTFOUND ) {};
//  while ( rText.SearchAndReplace( "\\\\", "-=<[BSlashBSlash]>=-" )
//      != STRING_NOTFOUND ) {};
    while ( rText.SearchAndReplace( "\"", "-=<[BSlashHKom}>=-" )
        != STRING_NOTFOUND ) {};
    while ( rText.SearchAndReplace( "\0x7F", "-=<[0x7F]>=-" )
        != STRING_NOTFOUND ) {};
// new
    while ( rText.SearchAndReplace( "\\", "-=<[BSlash]>=-" )
        != STRING_NOTFOUND ) {};
    while ( rText.SearchAndReplace(  "-=<[BSlash]>=-", "\\\\" )
        != STRING_NOTFOUND ) {};
// new
    while ( rText.SearchAndReplace( "-=<[BSlashHKom}>=-", "\\\"" )
        != STRING_NOTFOUND ) {};
//  while ( rText.SearchAndReplace(  "-=<[BSlashBSlash]>=-", "\\\\" )
//      != STRING_NOTFOUND ) {};
    while ( rText.SearchAndReplace( "-=<[Hochkomma]>=-", "\"" )
        != STRING_NOTFOUND ) {};
*/

    if ( bNoOpen ) {
        ByteString sTmp( rText );
        rText = "\"";
        rText += sTmp;
    }
    if ( bNoClose )
        rText += "\"";
}

/*****************************************************************************/
BOOL Export::PrepareTextToMerge( ByteString &rText, USHORT nTyp,
                                USHORT nLangIndex, ResData *pResData )
/*****************************************************************************/
{
    // position to merge in:
    USHORT nStart;
    USHORT nEnd;
    ByteString sOldId = pResData->sId;
    ByteString sOldGId = pResData->sGId;
    ByteString sOldTyp = pResData->sResTyp;

    ByteString sOrigText( rText );

    switch ( nTyp ) {
        case LIST_STRING :
        case LIST_UIENTRIES :
        case LIST_FILTER :
        case LIST_ITEM :
        {
            if ( bUnmerge )
                return TRUE;

            ExportList *pList = NULL;
            switch ( nTyp ) {
                case LIST_STRING : {
                    pResData->sResTyp = "stringlist";
                    pList = pResData->pStringList;
                }
                break;
                case LIST_UIENTRIES : {
                    pResData->sResTyp = "uientries";
                    pList = pResData->pUIEntries;
                }
                break;
                case LIST_FILTER : {
                    pResData->sResTyp = "filterlist";
                    pList = pResData->pFilterList;
                }
                break;
                case LIST_ITEM : {
                    pResData->sResTyp = "itemlist";
                    pList = pResData->pItemList;
                }
                break;
            }
            if ( pList ) {
                ExportListEntry *pCurEntry = pList->GetObject( nListIndex - 1 );
                if ( pCurEntry ) {
                    rText = (*pCurEntry)[ GERMAN_LIST_LINE_INDEX ];
                }
            }

            nStart = rText.Search( "\"" );
            if ( nStart == STRING_NOTFOUND ) {
                rText = sOrigText;
                return FALSE;
            }

            BOOL bFound = FALSE;
            for ( nEnd = nStart + 1; nEnd < rText.Len() && !bFound; nEnd++ ) {
                if ( rText.GetChar( nEnd ) == '\"' )
                    bFound = TRUE;
            }
            if ( !bFound ) {
                rText = sOrigText;
                return FALSE;
            }

            nEnd --;
            sLastListLine = rText;
            pResData->sId = nListIndex;
            if ( pResData->sGId.Len())
                pResData->sGId += ".";
            pResData->sGId += sOldId;
            nTyp = STRING_TYP_TEXT;
        }
        break;
        case STRING_TYP_TEXT :
        case STRING_TYP_HELPTEXT :
        case STRING_TYP_QUICKHELPTEXT :
        case STRING_TYP_TITLE :
        {
            if ( bUnmerge ) {
                if (( nLangIndex != GERMAN_INDEX ) &&
                    ( nLangIndex != ENGLISH_INDEX ))
                {
                    bDontWriteOutput = TRUE;
                }
                return TRUE;
            }

            nStart = rText.Search( "=" );
            if ( nStart == STRING_NOTFOUND ) {
                rText = sOrigText;
                return FALSE;
            }

            nStart++;
            BOOL bFound = FALSE;
            while(( nStart < rText.Len()) && !bFound ) {
                if (( rText.GetChar( nStart ) != ' ' ) && ( rText.GetChar( nStart ) != '\t' ))
                    bFound = TRUE;
                else
                    nStart ++;
            }

            // no start position found
            if ( !bFound ) {
                rText = sOrigText;
                return FALSE;
            }

            // position to end mergeing in
            nEnd = rText.Len() - 1;
            bFound = FALSE;

            while (( nEnd > nStart ) && !bFound ) {
                if (( rText.GetChar( nEnd ) != ' ' ) && ( rText.GetChar( nEnd ) != '\t' ) &&
                    ( rText.GetChar( nEnd ) != '\n' ) && ( rText.GetChar( nEnd ) != ';' ) &&
                    ( rText.GetChar( nEnd ) != '{' ) && ( rText.GetChar( nEnd ) != '\\' ))
                {
                    bFound = TRUE;
                }
                else
                    nEnd --;
            }
        }
        break;
    }

    // search for merge data
    if ( !pMergeDataFile )
        pMergeDataFile = new MergeDataFile( sMergeSrc, bErrorLog, aCharSet );

    PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
    pResData->sId = sOldId;
    pResData->sGId = sOldGId;
    pResData->sResTyp = sOldTyp;

    if ( !pEntrys ) {
        rText = sOrigText;
        return FALSE; // no data found
    }

    ByteString sContent;
    pEntrys->GetText( sContent, nTyp, nLangIndex );
    if ( !sContent.Len() && ( nLangIndex != GERMAN_INDEX )) {
        rText = sOrigText;
        return FALSE; // no data found
    }

    if ( nLangIndex == GERMAN_INDEX ) {
        if ( pWordTransformer ) {
            ByteString sRealContent = rText.Copy( nStart + 1, nEnd - nStart - 1 );
            ByteString sPostFix( rText.Copy( nEnd ));
            rText.Erase( nStart + 1 );

            if ( pWordTransformer->Transform( sRealContent ) != WordTransformer::OK ) {
                ByteString aString;
                for ( USHORT i = 0; i < pWordTransformer->NrOfErrors(); i++ ) {
                    pWordTransformer->GetError( i, &aString );
                    fprintf( stderr, "%s\n" , aString.GetBuffer());
                }
                rText = sOrigText;
                return FALSE;
            }

            rText += sRealContent;
            rText += sPostFix;

            if ( rText != sOrigText )
                return TRUE;
        }
        return FALSE;
    }

    ByteString sPostFix( rText.Copy( ++nEnd ));
    rText.Erase( nStart );

    ConvertMergeContent( sContent, nTyp );

    // merge new res. in text line
    rText += sContent;
    rText += sPostFix;

    return TRUE;
}

/*****************************************************************************/
void Export::MergeRest( ResData *pResData, USHORT nMode )
/*****************************************************************************/
{
    if ( bUnmerge ) {
        CreateRefIds( pResData );
        switch ( nMode ) {
            case MERGE_MODE_NORMAL : {
                if ( pResData->bRestMerged  )
                    return;
                pResData->bRestMerged = TRUE;
                if ( pResData->bText &&
                    pResData->sText[ GERMAN_INDEX ].Len() &&
                    pResData->sText[ ENGLISH_INDEX ].Len())
                {
                    BOOL bAddSemikolon = FALSE;
                    ByteString sOutput;
                    if ( bNextMustBeDefineEOL ) {
                        sOutput += "\t\\\n";
                    }
                    if ( bDefine ) {
                        bDefine = FALSE;
                        bNextMustBeDefineEOL = TRUE;
                    }
                    for ( USHORT j = 0; j < nLevel; j++ )
                        sOutput += "\t";
                    sOutput += pResData->sTextTyp;
                    sOutput += " = ";
                    sOutput += ByteString::CreateFromInt32( pResData->nTextRefId );
                    if ( bDefine )
                        sOutput += ";\\\n";
                    else if ( !bNextMustBeDefineEOL )
                        sOutput += ";\n";
                    else
                        bAddSemikolon = TRUE;
                    WriteToMerged( sOutput );

                    if ( bAddSemikolon ) {
                        ByteString sOutput( " ;" );
                        WriteToMerged( sOutput );
                    }
                }

                if ( pResData->bHelpText &&
                    pResData->sHelpText[ GERMAN_INDEX ].Len() &&
                    pResData->sHelpText[ ENGLISH_INDEX ].Len())
                {
                    BOOL bAddSemikolon = FALSE;
                    ByteString sOutput;
                    if ( bNextMustBeDefineEOL) {
                        sOutput += "\t\\\n";
                    }
                    if ( bDefine ) {
                        bDefine = FALSE;
                        bNextMustBeDefineEOL = TRUE;
                    }
                    for ( USHORT j = 1; j < nLevel; j++ )
                        sOutput += "\t";
                    sOutput += "HelpText";
                    sOutput += " = ";
                    sOutput += ByteString::CreateFromInt32( pResData->nHelpTextRefId );
                    if ( bDefine )
                        sOutput += ";\\\n";
                    else if ( !bNextMustBeDefineEOL )
                        sOutput += ";\n";
                    else
                        bAddSemikolon = TRUE;
                    WriteToMerged( sOutput );

                    if ( bAddSemikolon ) {
                        ByteString sOutput( " ;" );
                        WriteToMerged( sOutput );
                    }
                }

                if ( pResData->bQuickHelpText &&
                    pResData->sQuickHelpText[ GERMAN_INDEX ].Len() &&
                    pResData->sQuickHelpText[ ENGLISH_INDEX ].Len())
                {
                    BOOL bAddSemikolon = FALSE;
                    ByteString sOutput;
                    if ( bNextMustBeDefineEOL) {
                        sOutput += "\t\\\n";
                    }
                    if ( bDefine ) {
                        bDefine = FALSE;
                        bNextMustBeDefineEOL = TRUE;
                    }
                    for ( USHORT j = 1; j < nLevel; j++ )
                        sOutput += "\t";
                    sOutput += "QuickHelpText";
                    sOutput += " = ";
                    sOutput += ByteString::CreateFromInt32( pResData->nQuickHelpTextRefId );
                    if ( bDefine )
                        sOutput += ";\\\n";
                    else if ( !bNextMustBeDefineEOL )
                        sOutput += ";\n";
                    else
                        bAddSemikolon = TRUE;
                    WriteToMerged( sOutput );

                    if ( bAddSemikolon ) {
                        ByteString sOutput( " ;" );
                        WriteToMerged( sOutput );
                    }
                }

                if ( pResData->bTitle &&
                    pResData->sTitle[ GERMAN_INDEX ].Len() &&
                    pResData->sTitle[ ENGLISH_INDEX ].Len())
                {
                    BOOL bAddSemikolon = FALSE;
                    ByteString sOutput;
                    if ( bNextMustBeDefineEOL) {
                        sOutput += "\t\\\n";
                    }
                    if ( bDefine ) {
                        bDefine = FALSE;
                        bNextMustBeDefineEOL = TRUE;
                    }
                    for ( USHORT j = 1; j < nLevel; j++ )
                        sOutput += "\t";
                    sOutput += "Title";
                    sOutput += " = ";
                    sOutput += ByteString::CreateFromInt32( pResData->nTitleRefId );
                    if ( bDefine )
                        sOutput += ";\\\n";
                    else if ( !bNextMustBeDefineEOL )
                        sOutput += ";\n";
                    else
                        bAddSemikolon = TRUE;
                    WriteToMerged( sOutput );

                    if ( bAddSemikolon ) {
                        ByteString sOutput( " ;" );
                        WriteToMerged( sOutput );
                    }
                }
            }
            if ( pResData->bList ) {
                ByteString sSpace;
                for ( USHORT i = 0; i < nLevel-1; i++ )
                    sSpace += "\t";
                for ( USHORT nT = LIST_STRING; nT <= LIST_UIENTRIES; nT++ ) {
                    ByteString sHead( sSpace );
                    ExportList *pList = NULL;
                    switch ( nT ) {
                        case LIST_STRING : sHead += "StringList "; pList = pResData->pStringList; break;
                        case LIST_FILTER : sHead += "FilterList "; pList = pResData->pFilterList; break;
                        case LIST_ITEM : sHead += "ItemList "; pList = pResData->pItemList; break;
                        case LIST_UIENTRIES : sHead += "UIEntries "; pList = pResData->pUIEntries; break;
                    }
                    if ( pList ) {
                        if ( bDefine ) {
                            sHead += "= \\\n";
                            sHead += sSpace;
                            sHead += "\t{\\\n\t\t";
                        }
                        else {
                            sHead += "= \n";
                            sHead += sSpace;
                            sHead += "\t{\n\t\t";
                        }
                        WriteToMerged( sHead );
                        for ( ULONG i = 0; i < pList->Count(); i++ ) {
                            ByteString sLine(( *pList->GetObject( i ))[ GERMAN_LIST_LINE_INDEX ]);
                            if (( nT != LIST_UIENTRIES ) &&
                                (( sLine.Search( "{" ) == STRING_NOTFOUND ) ||
                                ( sLine.Search( "{" ) >= sLine.Search( "\"" ))) &&
                                (( sLine.Search( "<" ) == STRING_NOTFOUND ) ||
                                ( sLine.Search( "<" ) >= sLine.Search( "\"" ))))
                            {
                                sLine.SearchAndReplace( "\"", "< \"" );
                            }

                            USHORT nStart, nEnd;
                            nStart = sLine.Search( "\"" );
                            BOOL bFound = FALSE;
                            for ( nEnd = nStart + 1; nEnd < sLine.Len() && !bFound; nEnd++ ) {
                                if ( sLine.GetChar( nEnd ) == '\"' )
                                    bFound = TRUE;
                            }
                            nEnd--;
                            ByteString sPostFix( sLine.Copy( ++nEnd ));
                            sLine.Erase( nStart );

                            ByteString sText(( *pList->GetObject( i ))[ LIST_REFID ]);

                            // merge new res. in text line
                            sLine += sText;
                            sLine += sPostFix;

                            sText = "\t";
                            sText += sLine;
                            if ( bDefine )
                                sText += " ;\\\n";
                            else
                                sText += " ;\n";
                            sText += sSpace;
                            sText += "\t";
                            WriteToMerged( sText );
                        }
                        if ( pList->Count()) {
                            ByteString sFooter( sSpace.Copy( 1 ));
                            if ( !bDefine )
                                sFooter += "};\n\t";
                            else
                                sFooter += "\n\n";
                            WriteToMerged( sFooter );
                        }
                    }
                }
            }
            break;
        }
        return;
    }

    if ( !pMergeDataFile )
        pMergeDataFile = new MergeDataFile( sMergeSrc, bErrorLog, aCharSet );

    switch ( nMode ) {
        case MERGE_MODE_NORMAL : {
            PFormEntrys *pEntry = pMergeDataFile->GetPFormEntrys( pResData );
            if ( pEntry && pResData->bText ) {
                BOOL bAddSemikolon = FALSE;
                BOOL bFirst = TRUE;
                for ( USHORT i = 0; i < LANGUAGES; i++ ) {
                    ByteString sText;
                    BOOL bText = pEntry->GetText( sText, STRING_TYP_TEXT, i, TRUE );
                    if ( bText && sText.Len() && sText != "-" ) {
                        ByteString sOutput;
                        if ( bNextMustBeDefineEOL)  {
                            if ( bFirst )
                                sOutput += "\t\\\n";
                            else
                                sOutput += ";\t\\\n";
                        }
                        bFirst=FALSE;
                        sOutput += "\t";
                        sOutput += pResData->sTextTyp;
                        if ( i != GERMAN_INDEX ) {
                            sOutput += "[ ";
                            sOutput += LangName[ i ];
                            sOutput += " ] ";
                        }
                        sOutput += "= ";
                           ConvertMergeContent( sText, STRING_TYP_TEXT );
                        sOutput += sText;
                        if ( bDefine )
                            sOutput += ";\\\n";
                        else if ( !bNextMustBeDefineEOL )
                            sOutput += ";\n";
                        else
                            bAddSemikolon = TRUE;
                        for ( USHORT j = 1; j < nLevel; j++ )
                            sOutput += "\t";
                        WriteToMerged( sOutput );
                    }
                }
                if ( bAddSemikolon ) {
                    ByteString sOutput( ";" );
                    WriteToMerged( sOutput );
                }
            }

            if ( pEntry && pResData->bQuickHelpText ) {
                BOOL bAddSemikolon = FALSE;
                BOOL bFirst = TRUE;
                for ( USHORT i = 0; i < LANGUAGES; i++ ) {
                    ByteString sText;
                    BOOL bText = pEntry->GetText( sText, STRING_TYP_QUICKHELPTEXT, i, TRUE );
                    if ( bText && sText.Len() && sText != "-" ) {
                        ByteString sOutput;
                        if ( bNextMustBeDefineEOL)  {
                            if ( bFirst )
                                sOutput += "\t\\\n";
                            else
                                sOutput += ";\t\\\n";
                        }
                        bFirst=FALSE;
                        sOutput += "\t";
                        sOutput += "QuickHelpText";
                        if ( i != GERMAN_INDEX ) {
                            sOutput += "[ ";
                            sOutput += LangName[ i ];
                            sOutput += " ] ";
                        }
                        sOutput += "= ";
                           ConvertMergeContent( sText, STRING_TYP_QUICKHELPTEXT );
                        sOutput += sText;
                        if ( bDefine )
                            sOutput += ";\\\n";
                        else if ( !bNextMustBeDefineEOL )
                            sOutput += ";\n";
                        else
                            bAddSemikolon = TRUE;
                        for ( USHORT j = 1; j < nLevel; j++ )
                            sOutput += "\t";
                        WriteToMerged( sOutput );
                    }
                }
                if ( bAddSemikolon ) {
                    ByteString sOutput( ";" );
                    WriteToMerged( sOutput );
                }
            }

            if ( pEntry && pResData->bTitle ) {
                BOOL bAddSemikolon = FALSE;
                BOOL bFirst = TRUE;
                for ( USHORT i = 0; i < LANGUAGES; i++ ) {
                    ByteString sText;
                    BOOL bText = pEntry->GetText( sText, STRING_TYP_TITLE, i, TRUE );
                    if ( bText && sText.Len() && sText != "-" ) {
                        ByteString sOutput;
                        if ( bNextMustBeDefineEOL)  {
                            if ( bFirst )
                                sOutput += "\t\\\n";
                            else
                                sOutput += ";\t\\\n";
                        }
                        bFirst=FALSE;
                        sOutput += "\t";
                        sOutput += "Title";
                        if ( i != GERMAN_INDEX ) {
                            sOutput += "[ ";
                            sOutput += LangName[ i ];
                            sOutput += " ] ";
                        }
                        sOutput += "= ";
                           ConvertMergeContent( sText, STRING_TYP_TITLE );
                        sOutput += sText;
                        if ( bDefine )
                            sOutput += ";\\\n";
                        else if ( !bNextMustBeDefineEOL )
                            sOutput += ";\n";
                        else
                            bAddSemikolon = TRUE;
                        for ( USHORT j = 1; j < nLevel; j++ )
                            sOutput += "\t";
                        WriteToMerged( sOutput );
                    }
                }
                if ( bAddSemikolon ) {
                    ByteString sOutput( ";" );
                    WriteToMerged( sOutput );
                }
            }
            // looking for left list entrys

            if ( pResData->bList ) {
                ByteString sOldId = pResData->sId;
                ByteString sOldGId = pResData->sGId;
                ByteString sOldTyp = pResData->sResTyp;
                if ( pResData->sGId.Len())
                    pResData->sGId += ".";
                pResData->sGId += sOldId;
                ByteString sSpace;
                for ( USHORT i = 1; i < nLevel-1; i++ )
                    sSpace += "\t";
                for ( USHORT nT = LIST_STRING; nT <= LIST_UIENTRIES; nT++ ) {
                    switch ( nT ) {
                        case LIST_STRING : pResData->sResTyp = "stringlist"; break;
                        case LIST_FILTER : pResData->sResTyp = "filterlist"; break;
                        case LIST_UIENTRIES : pResData->sResTyp = "uientries"; break;
                        case LIST_ITEM : pResData->sResTyp = "itemlist"; break;
                    }
                    for ( USHORT nLang = 0; nLang < LANGUAGES; nLang ++ ) {
                        USHORT nIdx = 1;
                        pResData->sId = nIdx;
                        PFormEntrys *pEntrys;
                        while( pEntrys = pMergeDataFile->GetPFormEntrys( pResData )) {
                            ByteString sText;
                            BOOL bText = pEntrys->GetText( sText, STRING_TYP_TEXT, nLang, TRUE );
                            if ( bText && sText.Len()) {
                                if ( nIdx == 1 ) {
                                    ByteString sHead( sSpace );
                                    switch ( nT ) {
                                        case LIST_STRING : sHead += "StringList "; break;
                                        case LIST_FILTER : sHead += "FilterList "; break;
                                        case LIST_ITEM : sHead += "ItemList "; break;
                                        case LIST_UIENTRIES : sHead += "UIEntries "; break;
                                    }
                                    if ( nIdx != GERMAN_INDEX ) {
                                        sHead += "[ ";
                                        sHead += LangName[ nLang ];
                                        sHead += " ] ";
                                    }
                                    if ( bDefine ) {
                                        sHead += "= \\\n";
                                        sHead += sSpace;
                                        sHead += "\t{\\\n\t\t";
                                    }
                                    else {
                                        sHead += "= \n";
                                        sHead += sSpace;
                                        sHead += "\t{\n\t\t";
                                    }
                                    WriteToMerged( sHead );
                                }
                                ByteString sLine( sLastListLine );
                                if (( nT != LIST_UIENTRIES ) &&
                                    (( sLine.Search( "{" ) == STRING_NOTFOUND ) ||
                                    ( sLine.Search( "{" ) >= sLine.Search( "\"" ))) &&
                                    (( sLine.Search( "<" ) == STRING_NOTFOUND ) ||
                                    ( sLine.Search( "<" ) >= sLine.Search( "\"" ))))
                                {
                                    sLine.SearchAndReplace( "\"", "< \"" );
                                }

                                USHORT nStart, nEnd;
                                nStart = sLine.Search( "\"" );
                                BOOL bFound = FALSE;
                                for ( nEnd = nStart + 1; nEnd < sLine.Len() && !bFound; nEnd++ ) {
                                    if ( sLine.GetChar( nEnd ) == '\"' )
                                        bFound = TRUE;
                                }
                                nEnd --;
                                ByteString sPostFix( sLine.Copy( ++nEnd ));
                                sLine.Erase( nStart );

                                ConvertMergeContent( sText, nT );

                                // merge new res. in text line
                                sLine += sText;
                                sLine += sPostFix;

                                ByteString sText( "\t" );
                                sText += sLine;
                                if ( bDefine )
                                    sText += " ;\\\n";
                                else
                                    sText += " ;\n";
                                sText += sSpace;
                                sText += "\t";
                                WriteToMerged( sText );
                                pResData->sId = ++nIdx;
                            }
                            else
                                break;
                        }
                        if ( nIdx > 1 ) {
                            ByteString sFooter( sSpace.Copy( 1 ));
                            if ( !bDefine )
                                sFooter += "};\n\t";
                            else
                                sFooter += "\n\n";
                            WriteToMerged( sFooter );
                        }
                    }
                }

                pResData->sId = sOldId;
                pResData->sGId = sOldGId;
                pResData->sResTyp = sOldTyp;
            }
        }
        break;
        case MERGE_MODE_LIST : {
            nListIndex++;
            ByteString sLine( sLastListLine );
            if (( nList != LIST_UIENTRIES ) &&
                (( sLine.Search( "{" ) == STRING_NOTFOUND ) ||
                ( sLine.Search( "{" ) >= sLine.Search( "\"" ))) &&
                (( sLine.Search( "<" ) == STRING_NOTFOUND ) ||
                ( sLine.Search( "<" ) >= sLine.Search( "\"" ))))
            {
                sLine.SearchAndReplace( "\"", "< \"" );
            }
            while( PrepareTextToMerge( sLine, nList, nListLang, pResData )) {
                ByteString sText( "\t" );
                sText += sLine;
                sText += "\n";
                for ( USHORT i = 0; i < nLevel; i++ )
                    sText += "\t";
                WriteToMerged( sText );
                nListIndex++;
                sLine = sLastListLine;
            }
        }
        break;
    }
}

/*****************************************************************************/
void Export::SetChildWithText()
/*****************************************************************************/
{
    if ( aResStack.Count() > 1 ) {
        for ( ULONG i = 0; i < aResStack.Count() - 1; i++ ) {
            aResStack.GetObject( i )->bChildWithText = TRUE;
        }
    }
}

