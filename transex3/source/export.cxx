/*************************************************************************
 *
 *  $RCSfile: export.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:51:32 $
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
#include "utf8conv.hxx"

extern "C" { int yyerror( char * ); }
extern "C" { int YYWarning( char * ); }

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
#define STATE_UTF8      0x000A
#define STATE_LANGUAGES 0X000B

// set of global variables
DECLARE_LIST( FileList, ByteString * );
FileList aInputFileList;
BOOL bEnableExport;
BOOL bMergeMode;
BOOL bErrorLog;
BOOL bBreakWhenHelpText;
BOOL bUnmerge;
BOOL bUTF8;
bool bQuiet;
ByteString sPrj;
ByteString sPrjRoot;
ByteString sActFileName;
ByteString sOutputFile;
ByteString sMergeSrc;
ByteString sTempFile;
ByteString sFile;
MergeDataFile *pMergeDataFile;
FILE *pTempFile;

/*int nTypBuffer = 0;
int nTypCurrent = 0;
bool bNextIsMacro = false;
bool bLastWasMacro = false;*/
ByteString sStrBuffer;
bool bMarcro = false;
/*bool bStart = true;
int nNextTyp = 0;*/

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
    bUTF8 = TRUE;
    sPrj = "";
    sPrjRoot = "";
    sActFileName = "";
    Export::sLanguages = "";
    sTempFile = "";
    pTempFile = NULL;
    bQuiet = false;
    USHORT nState = STATE_NON;
    BOOL bInput = FALSE;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        if (( ByteString( argv[ i ]) == "-i" ) || ( ByteString( argv[ i ] ) == "-I" )) {
            nState = STATE_INPUT; // next tokens specifies source files
        }
        else if (( ByteString( argv[ i ]) == "-o" ) || ( ByteString( argv[ i ] ) == "-O" )) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if (( ByteString( argv[ i ]) == "-p" ) || ( ByteString( argv[ i ] ) == "-P" )) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }
        else if (( ByteString( argv[ i ]) == "-qq" ) || ( ByteString( argv[ i ] ) == "-QQ" )) {
            bQuiet = true;
        }

        else if (( ByteString( argv[ i ]) == "-r" ) || ( ByteString( argv[ i ] ) == "-R" )) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if (( ByteString( argv[ i ]) == "-m" ) || ( ByteString( argv[ i ] ) == "-M" )) {
            nState = STATE_MERGESRC; // next token specifies the merge database
        }
        else if (( ByteString( argv[ i ]) == "-e" ) || ( ByteString( argv[ i ] ) == "-E" )) {
            nState = STATE_ERRORLOG;
            bErrorLog = FALSE;
        }
        else if (( ByteString( argv[ i ] ) == "-b" ) || ( ByteString( argv[ i ] ) == "-B" )) {
            nState = STATE_BREAKHELP;
            bBreakWhenHelpText = TRUE;
        }
        else if (( ByteString( argv[ i ]) == "-u" ) || ( ByteString( argv[ i ] ) == "-U" )) {
            nState = STATE_UNMERGE;
            bUnmerge = TRUE;
            bMergeMode = TRUE;
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-UTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = TRUE;
        }
        else if ( ByteString( argv[ i ]).ToUpperAscii() == "-NOUTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = FALSE;
        }
        else if (( ByteString( argv[ i ]) == "-l" ) || ( ByteString( argv[ i ]) == "-L" )) {
            nState = STATE_LANGUAGES;
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
//                  sPrj.ToLowerAscii(); // the project
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
                case STATE_LANGUAGES: {
                    Export::sLanguages = ByteString( argv[ i ]);
                }
                break;
            }
        }
    }
    if( bUnmerge ) sMergeSrc = ByteString();
    if ( bInput ) {
        // command line is valid
        bEnableExport = TRUE;
        char *pReturn = new char[ sOutputFile.Len() + 1 ];
        strcpy( pReturn, sOutputFile.GetBuffer());  // #100211# - checked
        return pReturn;
    }

    // command line is not valid
    return NULL;
}
/*****************************************************************************/
int isQuiet(){
/*****************************************************************************/
   if( bQuiet ) return 1;
   else         return 0;
}
/*****************************************************************************/
int InitExport( char *pOutput , char *pFileName )
/*****************************************************************************/
{
    // instanciate Export
    ByteString sOutput( pOutput );


    if ( bMergeMode && !bUnmerge ) {
        // merge mode enabled, so read database
        pExport = new Export(sOutput, bEnableExport, sPrj, sPrjRoot, sMergeSrc , ByteString( pFileName ) );
    }
    else
        // no merge mode, only export
        pExport = new Export( sOutput, bEnableExport, sPrj, sPrjRoot ,ByteString( pFileName ) );
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
    if ( sTempFile.Len()) {
        fclose( pTempFile );
        String sTemp( sTempFile, RTL_TEXTENCODING_ASCII_US );
        DirEntry aTemp( sTemp );
        aTemp.Kill();
    }

    while ( aInputFileList.Count()) {
        ByteString sFileName( *(aInputFileList.GetObject( 0 )));

        ByteString sOrigFile( sFileName );

        sFileName = Export::GetNativeFile( sFileName );
        delete aInputFileList.GetObject(( ULONG ) 0 );
        aInputFileList.Remove(( ULONG ) 0 );

        if ( sFileName == "" ) {
            fprintf( stderr, "ERROR: Could not precompile File %s\n",
                sOrigFile.GetBuffer());
            return GetNextFile();
        }

        sTempFile = sFileName;

        // able to open file?
        FILE *pFile = fopen( sFileName.GetBuffer(), "r" );
        if ( !pFile )
            fprintf( stderr, "Error: Could not open File %s\n",
                sFileName.GetBuffer());
        else {
            pTempFile = pFile;

            // this is a valid file which can be opened, so
            // create path to project root
            DirEntry aEntry( String( sOrigFile, RTL_TEXTENCODING_ASCII_US ));
            aEntry.ToAbs();
            ByteString sFullEntry( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
            aEntry += DirEntry( String( "..", RTL_TEXTENCODING_ASCII_US ));
            aEntry += DirEntry( sPrjRoot );
            ByteString sPrjEntry( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );

            // create file name, beginnig with project root
            // (e.g.: source\ui\src\menue.src)
            sActFileName = sFullEntry.Copy( sPrjEntry.Len() + 1 );
//          sActFileName.ToLowerAscii();

            if( !bQuiet ) fprintf( stdout, "\nProcessing File %s ...\n", sOrigFile.GetBuffer());

            sActFileName.SearchAndReplaceAll( "/", "\\" );
            sFile = sActFileName;

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

int Parse( int nTyp, char *pTokenText ){
    pExport->Execute( nTyp , pTokenText );
    return 1;
}
void Close(){
    pExport->pParseQueue->Close();
}
/*****************************************************************************/
int WorkOnTokenSet( int nTyp, char *pTokenText )
/*****************************************************************************/
{

    pExport->pParseQueue->Push( QueueEntry( nTyp , ByteString( pTokenText ) ) );
    //Parse( nTyp , pTokenText );
    return 1;
}

} // extern

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
                const ByteString &rPrj, const ByteString &rPrjRoot , const ByteString& rFile )
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
                pWordTransformer( NULL ),
                bSkipFile( false )
{
    pParseQueue = new ParserQueue( *this );

    if( !isInitialized ) InitLanguages();
    // used when export is enabled

    // open output stream
    if ( bEnableExport ) {
        aOutput.Open( String( rOutput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
        if( !aOutput.IsOpen() ) {
            printf("ERROR : Can't open file %s\n",rOutput.GetBuffer());
            exit ( -1 );
        }
        aOutput.SetStreamCharSet( RTL_TEXTENCODING_MS_1252 );
        aOutput.SetLineDelimiter( LINEEND_CRLF );
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
                const ByteString &rMergeSource , const ByteString& rFile )
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
                pWordTransformer( NULL ),
                bSkipFile( false )

{
    pParseQueue = new ParserQueue( *this );
    if( !isInitialized ) InitLanguages( bMergeMode );
    // used when merge is enabled

    // open output stream
    if ( bEnableExport ) {
        aOutput.Open( String( rOutput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
        aOutput.SetStreamCharSet( RTL_TEXTENCODING_MS_1252 );
        aOutput.SetLineDelimiter( LINEEND_CRLF );
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
void Export::Init()
/*****************************************************************************/
{
    // resets the internal status, used before parseing another file
    sActPForm = "";
    bDefine = FALSE;
    bNextMustBeDefineEOL = FALSE;
    nLevel = 0;
    nList = LIST_NON;
    nListLang = ByteString( String::CreateFromAscii(""),RTL_TEXTENCODING_ASCII_US );
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
    if( pParseQueue )
        delete pParseQueue;
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
            pMergeDataFile = new MergeDataFile( sMergeSrc,sFile , bErrorLog, aCharSet, bUTF8 );

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
/*  printf("+---------------\n");
    printf("sToken = %s\n",sToken.GetBuffer());
    printf("nToken = %d\n",nToken);
    printf("+---------------\n");*/
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
            WriteToMerged( sOrig , false ); // ... ore whrite them directly to dest.
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
            ( nToken != NORMDEFINE ) &&
            ( nToken != RSCDEFINE ) &&
            ( nToken != CONDITION ) &&
            ( nToken != PRAGMA ))
    {
        // no res. exists at cur. level so return
        if ( bMergeMode )
            WriteToMerged( sOrig , false );
        return 0;
    }
    // #define NO_LOCALIZE_EXPORT
    if( bSkipFile ){
        if ( bMergeMode ) {
            WriteToMerged( sOrig , false );
        }
        return 1;
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
                        WriteToMerged( sOrig , false );
                    return 1;
                }
            }
            else if (( nToken != LISTASSIGNMENT ) && ( nToken != UIENTRIES )){
                // cur. line has macro line end
                ByteString sTmpLine( sToken );
                sTmpLine.EraseAllChars( '\t' ); sTmpLine.EraseAllChars( ' ' );
                if( sTmpLine.Len() < 0 ){
                    if ( sTmpLine.GetChar(( USHORT )( sTmpLine.Len() - 1 )) != '\\' )
                        bNextMustBeDefineEOL = TRUE;
                }
            }
        }
    }

    BOOL bExecuteDown = FALSE;
    if ( nToken != LEVELDOWN ) {
        USHORT nOpen = 0;
        USHORT nClose = 0;
        BOOL bReadOver = FALSE;
        USHORT i = 0;
        for ( i = 0; i < sToken.Len(); i++ ) {
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
//    printf("sTOKEN = '%s'\n",sToken.GetBuffer());
    switch ( nToken ) {

        case NORMDEFINE:
                        //printf("sToken = '%s'",sToken.GetBuffer());
                        while( sToken.SearchAndReplace( "\r", " " ) != STRING_NOTFOUND ) {};
                        while( sToken.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
                        while( sToken.SearchAndReplace( "  ", " " ) != STRING_NOTFOUND ) {};
                        if( sToken.EqualsIgnoreCaseAscii( "#define NO_LOCALIZE_EXPORT" ) ){
                            bSkipFile = true;
                            return 0;
                        }
                        if ( bMergeMode )
                          WriteToMerged( sOrig , false );

                        return 0;


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

            pResData = new ResData( sActPForm, FullId());
            aResStack.Insert( pResData, LIST_APPEND );
            ByteString sBackup( sToken );
            sToken.EraseAllChars( '\n' );
            sToken.EraseAllChars( '\r' );
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
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack.GetObject( nLevel - 2 )->bChild = TRUE;
            }

            // create new instance for this res. and fill mandatory fields

            pResData = new ResData( sActPForm, FullId());
            aResStack.Insert( pResData, LIST_APPEND );
            sToken.EraseAllChars( '\n' );
            sToken.EraseAllChars( '\r' );
            sToken.EraseAllChars( '{' );
            sToken.EraseAllChars( '\t' );
            sToken.EraseAllChars( ' ' );
            sToken.EraseAllChars( '\\' );
            pResData->sResTyp = sToken.ToLowerAscii();
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
                if ( bDefine )
                    bNextMustBeDefineEOL = TRUE;
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
                if ( bUnmerge ){
                    ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                }

                pResData->bList = TRUE;
                nList = LIST_STRING;
                ByteString sLang("de" , RTL_TEXTENCODING_ASCII_US );
                nListLang = sLang;
                nListIndex = 0;
                nListLevel = 0;
            }
            else if ( sKey == "FILTERLIST" ) {
                if ( bUnmerge ){
                    ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                }
                pResData->bList = TRUE;
                nList = LIST_FILTER;
                ByteString sLang("de" , RTL_TEXTENCODING_ASCII_US );
                nListLang = sLang;
                nListIndex = 0;
                nListLevel = 0;
            }
            else if ( sKey == "UIENTRIES" ) {
                if ( bUnmerge ){
                    ( sOrig.SearchAndReplace( "=", "[ de ] =" ));}
                pResData->bList = TRUE;
                nList = LIST_UIENTRIES;
                ByteString sLang("de" , RTL_TEXTENCODING_ASCII_US );
                nListLang = sLang;
                nListIndex = 0;
                nListLevel = 0;
            }
            if (( sToken.Search( "{" ) != STRING_NOTFOUND ) &&
                ( sToken.GetTokenCount( '{' ) > sToken.GetTokenCount( '}' )))
            {
                char *pTkn = "";
                //WorkOnTokenSet( LEVELUP, pTkn );
                Parse( LEVELUP, pTkn );
            }
            if ( bUnmerge && ( nListLang.EqualsIgnoreCaseAscii("de") || nListLang.EqualsIgnoreCaseAscii("en-US") ) && ListExists( pResData, nList ))
                bDontWriteOutput = TRUE;
         }
        break;
        case UIENTRIES:
        case LISTASSIGNMENT: {
            bDontWriteOutput = FALSE;
            ByteString sTmpToken( sToken);
            sTmpToken.EraseAllChars(' ');
            int nPos = 0;
            nPos = sTmpToken.ToLowerAscii().Search("[de]=");
            if( nPos != STRING_NOTFOUND ) {
                if ( bUnmerge ){
                    ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                }
                ByteString sKey = sTmpToken.Copy( 0 , nPos );
                sKey.EraseAllChars( ' ' );
                sKey.EraseAllChars( '\t' );
                ByteString sValue = sToken.GetToken( 1, '=' );
                CleanValue( sValue );
                if ( sKey.ToUpperAscii() ==  "STRINGLIST" ) {
                    pResData->bList = TRUE;
                    nList = LIST_STRING;
                    ByteString sLang("de" , RTL_TEXTENCODING_ASCII_US );
                    nListLang = sLang;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if ( sKey == "FILTERLIST" ) {
                    pResData->bList = TRUE;
                    nList = LIST_FILTER;
                    ByteString sLang("de" , RTL_TEXTENCODING_ASCII_US );
                    nListLang = sLang;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if ( sKey ==  "ITEMLIST" ) {
                    pResData->bList = TRUE;
                    nList = LIST_ITEM;
                    ByteString sLang("de" , RTL_TEXTENCODING_ASCII_US );
                    nListLang = sLang;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if ( sKey ==  "UIENTRIES" ) {
                    pResData->bList = TRUE;
                    nList = LIST_UIENTRIES;
                    ByteString sLang("de" , RTL_TEXTENCODING_ASCII_US );
                    nListLang = sLang;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                if ( bUnmerge && ( nListLang.EqualsIgnoreCaseAscii( "de" )
                    || nListLang.EqualsIgnoreCaseAscii("en-US" ) )
                    && ListExists( pResData, nList ))
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
                    nListLang = sLang;
                    if (( bUnmerge ) && ( !nListLang.EqualsIgnoreCaseAscii("de")) && ( !nListLang.EqualsIgnoreCaseAscii("en-US")))
                        bDontWriteOutput = TRUE;
                    nListIndex = 0;
                    nListLevel = 0;
                    if ( bUnmerge && nListLang.EqualsIgnoreCaseAscii("de")  && ListExists( pResData, nList ) )
                        bDontWriteOutput = TRUE;
                }
            }
        }
        break;
        case TEXT:
        case _LISTTEXT:
        case LISTTEXT: {
            // this is an entry for a String- or FilterList
            if ( nList ) {
                SetChildWithText();
                ByteString sEntry( sToken.GetToken( 1, '\"' ));
                if ( sToken.GetTokenCount( '\"' ) > 3 )
                    sEntry += "\"";
                if ( sEntry == "\\\"" )
                    sEntry = "\"";
                //sEntry = sEntry.Convert( aCharSet, RTL_TEXTENCODING_MS_1252 );
                //sEntry = sEntry.Convert( RTL_TEXTENCODING_MS_1252, RTL_TEXTENCODING_UTF8 );
                InsertListEntry( sEntry, sOrig );
                if ( bMergeMode && ( sEntry != "\"" )) {
                    PrepareTextToMerge( sOrig, nList, nListLang, pResData );
                }
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
                ByteString sLang( "de" );
                if ( sToken.GetToken( 0, '=' ).Search( "[" ) != STRING_NOTFOUND ) {
                     sLang = sToken.GetToken( 0, '=' ).GetToken( 1, '[' ).GetToken( 0, ']' );
                    CleanValue( sLang );
                }
                ByteString nLangIndex = sLang;
                ByteString sOrigKey = sKey;
                if ( sText.Len()) {


                    if (( sKey.ToUpperAscii() == "TEXT" ) ||
                        ( sKey == "MESSAGE" ) ||
                        ( sKey == "CUSTOMUNITTEXT" ) ||
                        ( sKey == "SLOTNAME" ) ||
                        ( sKey == "UINAME" ))
                    {
                        if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND )
                            ( sOrig.SearchAndReplace( "=", "[ de ] =" ));

                        SetChildWithText();
                        if ( nLangIndex.EqualsIgnoreCaseAscii("de") )
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
                                sError += nLangIndex;
                                sError += " defined twice";
                            }
                            pResData->sText[ nLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "HELPTEXT" ) {
                        if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND ){
                            ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                            }
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
                                sError += nLangIndex;
                                sError += " defined twice";
                            }
                            pResData->sHelpText[ nLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "QUICKHELPTEXT" ) {
                        if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND ){
                            ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                            }
                        SetChildWithText();
                        pResData->bQuickHelpText = TRUE;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_QUICKHELPTEXT, nLangIndex, pResData );
                            if ( bUnmerge )
                                pResData->sQuickHelpText[ nLangIndex ] = sText;
                        else {
                            if ( pResData->sQuickHelpText[ nLangIndex ].Len()) {
                                ByteString sError( "Language " );
                                sError += nLangIndex;
                                sError += " defined twice";
                            }
                            pResData->sQuickHelpText[ nLangIndex ] = sText;
                        }
                    }
                    else if ( sKey == "TITLE" ) {
                        if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND ){
                            ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                            }
                        SetChildWithText();
                        pResData->bTitle = TRUE;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_TITLE, nLangIndex, pResData );
                            if ( bUnmerge )
                                pResData->sTitle[ nLangIndex ] = sText;
                        else {
                            if ( pResData->sTitle[ nLangIndex ].Len()) {
                                ByteString sError( "Language " );
                                sError += nLangIndex;
                                sError += " defined twice";
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
            //USHORT nLangIndex = GetLangIndex( sLang );
            ByteString sText = sToken.GetToken( 2, ':' ).GetToken( 0, '*' );
            CleanValue( sText );
            if ( sText.Len())
            //  pResData->sText[ nLangIndex ] = sText;
                pResData->sText[ sLang ] = sText;
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
                pResData->nWidth = ( USHORT ) sMapping.GetToken( 0, ',' ).ToInt64();
            }
            else if ( sKey == "POSSIZE" ) {
                pResData->nWidth = ( USHORT ) sMapping.GetToken( 2, ',' ).ToInt64();
            }
        }
        break;
        case RSCDEFINELEND:
            bDontWriteOutput = FALSE;
        break;
        case CONDITION: {
            bDontWriteOutput = FALSE;
            while( sToken.SearchAndReplace( "\r", " " ) != STRING_NOTFOUND ) {};
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
                    Parse( LEVELDOWN, pTkn );
                    //WorkOnTokenSet( LEVELDOWN, pTkn );
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
            USHORT nRefId = ( USHORT ) sToken.GetToken( 1, '=' ).GetToken( 0, ';' ).EraseAllChars( '\t' ).EraseAllChars( ' ' ).ToInt32();
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

        if( bDefine && sOrig.Len() > 2 ){
            for( int n = 0 ; n < sOrig.Len() ; n++ ){
                if( sOrig.GetChar( n ) == '\n' && sOrig.GetChar( n-1 ) != '\\'){
                    sOrig.Insert('\\' , n++ );
                }
            }
        }
        WriteToMerged( sOrig , false);
    }

    if ( bExecuteDown ) {
        char *pTkn = "";
        Parse( LEVELDOWN, pTkn );
        //WorkOnTokenSet( LEVELDOWN, pTkn );
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
USHORT GetLangIndex( USHORT nLangId )
/*****************************************************************************/
{
    // translation table: LangName <=> Index
    USHORT nLangIndex = 0;
// remove me
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

       // mandatory to export: german and eng. and/or enus

     if (( pResData->sText[ ByteString("de") ].Len() &&
        ( pResData->sText[ ByteString("en") ].Len() || pResData->sText[ ByteString("en-US") ].Len()))
        ||
        ( pResData->sHelpText[ ByteString("de") ].Len() &&
        ( pResData->sHelpText[ ByteString("en") ].Len() || pResData->sHelpText[ ByteString("en-US") ].Len()))
        ||
        ( pResData->sQuickHelpText[ ByteString("de") ].Len() &&
        ( pResData->sQuickHelpText[ ByteString("en") ].Len() || pResData->sQuickHelpText[ ByteString("en-US") ].Len()))
         ||
        ( pResData->sTitle[ ByteString("de") ].Len() &&
        ( pResData->sTitle[ ByteString("en") ].Len() || pResData->sTitle[ ByteString("en-US") ].Len())))

       {
        FillInFallbacks( pResData );

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

        ByteString sTimeStamp( Export::GetTimeStamp());
        ByteString sCur;

        for( long int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];
                if ( !sCur.EqualsIgnoreCaseAscii("x-comment") ){
                    if ( pResData->sText[ sCur ].Len())
                        sXText = pResData->sText[ sCur ];
                    else {
                        sXText = pResData->sText[ ByteString("en-US") ];
                        if ( !sXText.Len())
                            sXText = pResData->sText[ ByteString("en") ];
                        if ( !sXText.Len())
                            sXText = pResData->sText[ ByteString("de") ];
                    }

                    if ( pResData->sHelpText[ sCur ].Len())
                        sXHText = pResData->sHelpText[ sCur ];
                    else {
                        sXHText = pResData->sHelpText[ ByteString("en-US") ];
                        if ( !sXHText.Len())
                            sXHText = pResData->sHelpText[ ByteString("en") ];
                        if ( !sXText.Len())
                            sXHText = pResData->sHelpText[ ByteString("de") ];
                    }

                    if ( pResData->sQuickHelpText[ sCur ].Len())
                        sXQHText = pResData->sQuickHelpText[ sCur ];
                    else {
                        sXQHText = pResData->sQuickHelpText[ ByteString("en-US") ];
                        if ( !sXQHText.Len())
                            sXQHText = pResData->sQuickHelpText[ ByteString("en") ];
                        if ( !sXQHText.Len())
                            sXQHText = pResData->sQuickHelpText[ ByteString("de") ];
                    }

                    if ( pResData->sTitle[ sCur ].Len())
                        sXTitle = pResData->sTitle[ sCur ];
                    else {
                        sXTitle = pResData->sTitle[ ByteString("en-US") ];
                        if ( !sXTitle.Len())
                            sXTitle = pResData->sTitle[ ByteString("en") ];
                        if ( !sXTitle.Len())
                            sXTitle = pResData->sTitle[ ByteString("de") ];
                    }

                    if ( !sXText.Len())
                        sXText = "-";

                    if ( !sXHText.Len()) {
                        if ( pResData->sHelpText[ ByteString("de") ].Len())
                            sXHText = pResData->sHelpText[ ByteString("de") ];
                        else if ( pResData->sHelpText[ ByteString("en-US") ].Len())
                            sXHText = pResData->sHelpText[ ByteString("en-US") ];
                        else if ( pResData->sHelpText[ ByteString("en") ].Len())
                            sXHText = pResData->sHelpText[ ByteString("en") ];
                    }
                }
                else
                    sXText = pResData->sText[ sCur ];

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
                    sOutput += sCur; sOutput += "\t";


/*                  if ( bUTF8 ) {
                        sXText = UTF8Converter::ConvertToUTF8( sXText, GetCharSet( LangId[ i ] ));
                        sXHText = UTF8Converter::ConvertToUTF8( sXHText, GetCharSet( LangId[ i ] ));
                        sXQHText = UTF8Converter::ConvertToUTF8( sXQHText, GetCharSet( LangId[ i ] ));
                        sXTitle = UTF8Converter::ConvertToUTF8( sXTitle, GetCharSet( LangId[ i ] ));
                    }*/

                    sOutput += sXText; sOutput += "\t";
                    sOutput += sXHText; sOutput += "\t";
                    sOutput += sXQHText; sOutput += "\t";
                    sOutput += sXTitle; sOutput += "\t";
                    sOutput += sTimeStamp;

                    aOutput.WriteLine( sOutput );
                }

                if ( bCreateNew ) {
                    pResData->sText[ sCur ] = "";
                    pResData->sHelpText[ sCur ] = "";
                    pResData->sQuickHelpText[ sCur ] = "";
                    pResData->sTitle[ sCur ] = "";
                }
            }
        //}
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

    ByteString sTimeStamp( Export::GetTimeStamp());
    ByteString sCur;
    for ( ULONG i = 0; i < pExportList->Count(); i++ ) {
        ByteString sLID( ByteString::CreateFromInt64( i + 1 ));
        ExportListEntry *pEntry = pExportList->GetObject( i );
                // mandatory for export: german and eng. and/or enus
        for( long int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];

            if ((*pEntry)[ ByteString("de") ].Len() &&
                    ((*pEntry)[ ByteString("en-US") ].Len() ||
                         (*pEntry)[ ByteString("en") ].Len()))
                {
                    if ( bEnableExport )
                    {
                        ByteString sText((*pEntry)[ ByteString("de") ] );
                        if ((*pEntry)[ sCur ].Len())
                            sText = (*pEntry)[ sCur ];

                        ByteString sOutput( sProject ); sOutput += "\t";
                        if ( sRoot.Len())
                            sOutput += sActFileName;
                        sOutput += "\t0\t";
                        sOutput += rTyp; sOutput += "\t";
                        sOutput += sGID; sOutput += "\t";
                        sOutput += sLID; sOutput += "\t\t";
                        sOutput += pResData->sPForm; sOutput += "\t0\t";
                        //sOutput += ByteString::CreateFromInt64( LangId[ j ] ); sOutput += "\t";
                        sOutput += sCur; sOutput += "\t";

                        /*if ( bUTF8 )
                            sText = UTF8Converter::ConvertToUTF8( sText, GetCharSet( LangId[ j ] ));
                        */
                        sOutput += sText; sOutput += "\t\t\t\t";
                        sOutput += sTimeStamp;

                        aOutput.WriteLine( sOutput );
                    }
                }
//          }
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
        printf("GroupID = %s\n",sFull.GetBuffer());
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
        ExportListEntry *pNew = new ExportListEntry();
        (*pNew)[ LIST_REFID ] = ByteString::CreateFromInt32( REFID_NONE );
        pList->Insert( pNew, LIST_APPEND );
    }
    ExportListEntry *pCurEntry = pList->GetObject( nListIndex );
    (*pCurEntry)[ nListLang ] = rText;
    if ( nListLang.EqualsIgnoreCaseAscii("de") ) {
        (*pCurEntry)[ GERMAN_LIST_LINE_INDEX ] = rLine;
        pList->NewGermanEntry();
    }
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
                ( rValue.GetChar( i ) == '{' ) || ( rValue.GetChar( i ) == '\\' ) ||
                ( rValue.GetChar( i ) == '\r' ))
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
            sTmp.EraseAllChars( '\r' );

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

            nState = TXT_STATE_TEXT;
            nStart = 1;


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


            while ( sReturn.SearchAndReplace( "\\\\", "-=<[BSlashBSlash]>=-" )
                != STRING_NOTFOUND ) {};
            while ( sReturn.SearchAndReplace( "-=<[BSlashBSlash]>=-", "\\" )
                != STRING_NOTFOUND ) {};

        }
        break;
    }
    return sReturn;
}

/*****************************************************************************/
void Export::WriteToMerged( const ByteString &rText , bool bSDFContent )
/*****************************************************************************/
{
    static ByteString SLASH  ('\\');
    static ByteString RETURN ('\n');

    if( pParseQueue->bMflag && !bSDFContent ) pParseQueue->bMflag;

    if ( !bDontWriteOutput || !bUnmerge ) {
        ByteString sText( rText );
        while ( sText.SearchAndReplace( " \n", "\n" ) != STRING_NOTFOUND ) {};
        if( pParseQueue->bNextIsM && bSDFContent && sText.Len() > 2 ){
            for( int n = 0 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );

                }
            }
        }
        else if( pParseQueue->bLastWasM && sText.Len() > 2 ){
            for( int n = 0 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );
                }
                if( sText.GetChar( n ) == '\n' )pParseQueue->bMflag=true;
            }
        }
        else if( pParseQueue->bCurrentIsM && bSDFContent && sText.Len() > 2 ){
            for( int n = 0 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );
                    pParseQueue->bMflag=true;
                }
            }
        }
        else if( pParseQueue->bMflag ){
            for( int n = 1 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );
                }
            }
        }
        for ( USHORT i = 0; i < sText.Len(); i++ ) {
            if ( sText.GetChar( i ) != '\n' )
                aOutput.Write( ByteString( sText.GetChar( i )).GetBuffer(), 1 );
            else{
                aOutput.WriteLine( ByteString());
            }
        }
/*        if( rText.Len() ){
          printf("============================\n");
          printf("CurMacro = %d , NextMacro = %d , hasSdfData = %d , bLastWasMacro = %d , bMflag = %d \n",pParseQueue->bCurrentIsM,pParseQueue->bNextIsM,bSDFContent , pParseQueue->bLastWasM , pParseQueue->bMflag);

          ByteString x(rText);
          x.SearchAndReplaceAll('\n','#');
          printf("'%s'\n",x.GetBuffer());

          ByteString y(sText);
          y.SearchAndReplaceAll('\n','#');
          printf("'%s'\n",y.GetBuffer());
        }
*/
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
                                //USHORT nLangIndex, ResData *pResData )
                                ByteString &nLangIndex, ResData *pResData )
/*****************************************************************************/
{
    // position to merge in:
    USHORT nStart = 0;
    USHORT nEnd = 0;
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
            if (( sLastListLine.Search( ">" ) != STRING_NOTFOUND ) &&
                ( sLastListLine.Search( "<" ) == STRING_NOTFOUND ))
            {
                ByteString sTmp = sLastListLine;
                sLastListLine = "<";
                sLastListLine += sTmp;
            }
            pResData->sId = ByteString::CreateFromInt32( nListIndex );
            //pResData->sId = nListIndex;//ByteString::CreateFromInt32( nListIndex );
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
                if (( nLangIndex != ByteString("de") ) &&
                    //( nLangIndex != ENGLISH_INDEX ))
                    ( nLangIndex != ByteString("en-US") ))
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
    if ( !pMergeDataFile ){
        pMergeDataFile = new MergeDataFile( sMergeSrc, sFile , bErrorLog, aCharSet, bUTF8 );

        // Init Languages
        ByteString sTmp = Export::sLanguages;
        if( sTmp.ToUpperAscii().Equals("ALL") )
            SetLanguages( pMergeDataFile->GetLanguages() );
        else if( !isInitialized )InitLanguages();

    }
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
    if ( !sContent.Len() && ( !nLangIndex.EqualsIgnoreCaseAscii("de") )) {
        rText = sOrigText;
        return FALSE; // no data found
    }

    if ( nLangIndex.EqualsIgnoreCaseAscii("de") ) {
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
    if ( bUnmerge ) { return;}


    if ( !pMergeDataFile ){
        pMergeDataFile = new MergeDataFile( sMergeSrc, sFile ,bErrorLog, aCharSet, bUTF8 );

        // Init Languages
        ByteString sTmp = Export::sLanguages;
        if( sTmp.ToUpperAscii().Equals("ALL") )
            SetLanguages( pMergeDataFile->GetLanguages() );
        else if( !isInitialized )InitLanguages();

    }
    switch ( nMode ) {
        case MERGE_MODE_NORMAL : {
            PFormEntrys *pEntry = pMergeDataFile->GetPFormEntrys( pResData );

            bool bWriteNoSlash = false;
           if ( pEntry && pResData->bText ) {

                BOOL bAddSemikolon = FALSE;
                BOOL bFirst = TRUE;
                ByteString sCur;
                ByteString sTmp = Export::sLanguages;

                int nSize = aLanguages.size();
                for( long int n = 0; n < aLanguages.size(); n++ ){
                    sCur = aLanguages[ n ];

                    ByteString sText;
                    BOOL bText = pEntry->GetText( sText, STRING_TYP_TEXT, sCur , TRUE );
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
                        if ( !sCur.EqualsIgnoreCaseAscii("de")) {
                            sOutput += "[ ";
                            sOutput += sCur;
                            sOutput += " ] ";
                        }
                        sOutput += "= ";
                           ConvertMergeContent( sText, STRING_TYP_TEXT );
                        sOutput += sText;

                        if ( bDefine && bWriteNoSlash )
                            sOutput += ";\n";

                        if ( bDefine )
                            sOutput += ";\\\n";
                        else if ( !bNextMustBeDefineEOL )
                            sOutput += ";\n";
                        else
                            bAddSemikolon = TRUE;
                        for ( USHORT j = 1; j < nLevel; j++ )
                            sOutput += "\t";
                        WriteToMerged( sOutput , true );
                    }
                }


                if ( bAddSemikolon ) {
                    ByteString sOutput( ";" );
                    WriteToMerged( sOutput , false );
                }
            }

            if ( pEntry && pResData->bQuickHelpText ) {
                BOOL bAddSemikolon = FALSE;
                BOOL bFirst = TRUE;
                ByteString sCur;

                for( long int n = 0; n < aLanguages.size(); n++ ){
                    sCur = aLanguages[ n ];

                    ByteString sText;
                    //BOOL bText = pEntry->GetText( sText, STRING_TYP_QUICKHELPTEXT, i, TRUE );
                    BOOL bText = pEntry->GetText( sText, STRING_TYP_QUICKHELPTEXT, sCur, TRUE );
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
                        //if ( i != ByteString("de") ) {
                        if ( !sCur.EqualsIgnoreCaseAscii("de") ) {
                            sOutput += "[ ";
                            sOutput += sCur;
                            //sOutput += LangName[ i ];
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
                        WriteToMerged( sOutput ,true );
                    }
                }
                if ( bAddSemikolon ) {
                    ByteString sOutput( ";" );
                    WriteToMerged( sOutput , false );
                }
            }

            if ( pEntry && pResData->bTitle ) {
                BOOL bAddSemikolon = FALSE;
                BOOL bFirst = TRUE;
                ByteString sCur;

                for( long int n = 0; n < aLanguages.size(); n++ ){
                    sCur = aLanguages[ n ];

                ByteString sText;
                    //BOOL bText = pEntry->GetText( sText, STRING_TYP_TITLE, i, TRUE );
                    BOOL bText = pEntry->GetText( sText, STRING_TYP_TITLE, sCur, TRUE );
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
                        //if ( i != ByteString("de") ) {
                        if ( !sCur.EqualsIgnoreCaseAscii("de") ) {
                            sOutput += "[ ";
                            sOutput += sCur;
                            //sOutput += LangName[ i ];
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
                        WriteToMerged( sOutput ,true );
                    }
                }
                if ( bAddSemikolon ) {
                    ByteString sOutput( ";" );
                    WriteToMerged( sOutput ,false);
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
                    ExportList *pList = NULL;
                    switch ( nT ) {
                        case LIST_STRING : pResData->sResTyp = "stringlist"; pList = pResData->pStringList; break;
                        case LIST_FILTER : pResData->sResTyp = "filterlist"; pList = pResData->pFilterList; break;
                        case LIST_UIENTRIES : pResData->sResTyp = "uientries"; pList = pResData->pUIEntries; break;
                        case LIST_ITEM : pResData->sResTyp = "itemlist"; pList = pResData->pItemList; break;
                    }
                    ByteString sCur;
                    for( long int n = 0; n < aLanguages.size(); n++ ){
                        sCur = aLanguages[ n ];
                        USHORT nIdx = 1;
                        //pResData->sId = ByteString::CreateFromInt32( nIdx );
                        pResData->sId = ByteString("1");
                        PFormEntrys *pEntrys;
                        ULONG nLIndex = 0;
                        ULONG nMaxIndex = 0;
                        if ( pList )
                            nMaxIndex = pList->GetGermanEntryCount();
                        while(( pEntrys = pMergeDataFile->GetPFormEntrys( pResData )) && ( nLIndex < nMaxIndex )) {
                            ByteString sText;
                            //BOOL bText = pEntrys->GetText( sText, STRING_TYP_TEXT, nLang, TRUE );
                            BOOL bText = pEntrys->GetText( sText, STRING_TYP_TEXT, sCur, TRUE );
                            if ( bText && sText.Len()) {
                                if ( nIdx == 1 ) {
                                    ByteString sHead;
                                    if ( bNextMustBeDefineEOL )
                                        sHead = "\\\n\t";
                                    sHead += sSpace;
                                    switch ( nT ) {
                                        case LIST_STRING : sHead += "StringList "; break;
                                        case LIST_FILTER : sHead += "FilterList "; break;
                                        case LIST_ITEM : sHead += "ItemList "; break;
                                        case LIST_UIENTRIES : sHead += "UIEntries "; break;
                                    }
                                    //if ( nIdx != GERMAN_INDEX ) {
                                      sHead += "[ ";
                                    //sHead += LangName[ nLang ];
                                    sHead += sCur;//nLang;
                                    sHead += " ] ";
                                    //}
                                    if ( bDefine || bNextMustBeDefineEOL ) {
                                        sHead += "= \\\n";
                                        sHead += sSpace;
                                        sHead += "\t{\\\n\t";
                                    }
                                    else {
                                        sHead += "= \n";
                                        sHead += sSpace;
                                        sHead += "\t{\n\t";
                                    }
                                    WriteToMerged( sHead , true);
                                }
                                ByteString sLine;
                                if ( pList && pList->GetObject( nLIndex ))
                                    sLine = ( *pList->GetObject( nLIndex ))[ GERMAN_LIST_LINE_INDEX ];
                                if ( !sLine.Len())
                                    sLine = sLastListLine;

                                if ( sLastListLine.Search( "<" ) != STRING_NOTFOUND ) {
                                    if (( nT != LIST_UIENTRIES ) &&
                                        (( sLine.Search( "{" ) == STRING_NOTFOUND ) ||
                                        ( sLine.Search( "{" ) >= sLine.Search( "\"" ))) &&
                                        (( sLine.Search( "<" ) == STRING_NOTFOUND ) ||
                                        ( sLine.Search( "<" ) >= sLine.Search( "\"" ))))
                                    {
                                        sLine.SearchAndReplace( "\"", "< \"" );
                                    }
                                }

                                USHORT nStart, nEnd;
                                nStart = sLine.Search( "\"" );
                                BOOL bFound = FALSE;
/*                              for ( nEnd = nStart + 1; nEnd < sLine.Len() && !bFound; nEnd++ ) {
                                    if ( sLine.GetChar( nEnd ) == '\"' )
                                        bFound = TRUE;
                                }*/
                                nEnd = sLine.SearchBackward( '\"' );
                                //nEnd --;
                                ByteString sPostFix( sLine.Copy( ++nEnd ));
                                sLine.Erase( nStart );

                                ConvertMergeContent( sText, nT );

                                // merge new res. in text line
                                sLine += sText;
                                sLine += sPostFix;

                                ByteString sText( "\t" );
                                sText += sLine;
                                if ( bDefine || bNextMustBeDefineEOL )
                                    sText += " ;\\\n";
                                else
                                    sText += " ;\n";
                                sText += sSpace;
                                sText += "\t";
                                WriteToMerged( sText ,true );
                                pResData->sId = ByteString::CreateFromInt32( ++nIdx );
                            }
                            else
                                break;
                            nLIndex ++;
                        }
                        if ( nIdx > 1 ) {
                            ByteString sFooter( sSpace.Copy( 1 ));
                            if ( bNextMustBeDefineEOL )
                                sFooter += "};";
                            else if ( !bDefine )
                                sFooter += "};\n\t";
                            else
                                sFooter += "\n\n";
                            WriteToMerged( sFooter ,true );
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
            ExportList *pList = NULL;
            switch ( nList ) {
                case LIST_STRING : pList = pResData->pStringList; break;
                case LIST_FILTER : pList = pResData->pFilterList; break;
                case LIST_UIENTRIES : pList = pResData->pUIEntries; break;
                case LIST_ITEM : pList = pResData->pItemList; break;
            }

            nListIndex++;
            ULONG nMaxIndex = 0;
            if ( pList )
                nMaxIndex = pList->GetGermanEntryCount();
            ByteString sLine;
            if ( pList && pList->GetObject( nListIndex ))
                sLine = ( *pList->GetObject( nListIndex ))[ GERMAN_LIST_LINE_INDEX ];
            if ( !sLine.Len())
                sLine = sLastListLine;

            if ( sLastListLine.Search( "<" ) != STRING_NOTFOUND ) {
                if (( nList != LIST_UIENTRIES ) &&
                    (( sLine.Search( "{" ) == STRING_NOTFOUND ) ||
                    ( sLine.Search( "{" ) >= sLine.Search( "\"" ))) &&
                    (( sLine.Search( "<" ) == STRING_NOTFOUND ) ||
                    ( sLine.Search( "<" ) >= sLine.Search( "\"" ))))
                {
                    sLine.SearchAndReplace( "\"", "< \"" );
                }
            }
            while( PrepareTextToMerge( sLine, nList, nListLang, pResData ) && ( nListIndex <= nMaxIndex )) {
                ByteString sText( "\t" );
                sText += sLine;
                sText += " ;";
                sText += "\n";
                for ( USHORT i = 0; i < nLevel; i++ )
                    sText += "\t";
                WriteToMerged( sText ,false );
                nListIndex++;
                if ( pList && pList->GetObject( nListIndex ))
                    sLine = ( *pList->GetObject( nListIndex ))[ GERMAN_LIST_LINE_INDEX ];
                if ( !sLine.Len())
                    sLine = sLastListLine;
                sLine += " ;";
            }
        }
        break;
    }
    pParseQueue->bMflag = false;
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

void ParserQueue::Push( const QueueEntry& aEntry ){
//    printf("nTyp = %d ",aEntry.nTyp);
    int nLen = aEntry.sLine.Len();
    if( !bStart ){
        aQueueCur->push( aEntry );
        if( nLen > 1 && aEntry.sLine.GetChar( nLen-1 ) == '\n' )
            bStart = true;
        else if ( aEntry.nTyp != IGNOREDTOKENS ){
            if( nLen > 1 && ( aEntry.sLine.GetChar( nLen-1 ) == '\\') ){
                // Next is Macro
//                ByteString x(aEntry.sLine);
//                x.SearchAndReplaceAll('\n','#');
//                printf("Next is Macro\n'%s'\n",x.GetBuffer());
                bCurrentIsM = true;
             }else{
                // Next is no Macro
 //               ByteString x(aEntry.sLine);
 //               x.SearchAndReplaceAll('\n','#');
 //               printf("Next is NO Macro\n'%s'\n",x.GetBuffer());
                bCurrentIsM = false;
             }
        }
    }
    else{
        aQueueNext->push( aEntry );
        if( nLen > 1 && aEntry.sLine.GetChar( nLen-1 ) != '\n' ){
            if( nLen > 1 && ( aEntry.sLine.GetChar( nLen-1  ) == '\\') ){
                // Next is Macro
                bNextIsM = true;
   //                 ByteString x(aEntry.sLine);
   //                 x.SearchAndReplaceAll('\n','#');
   //                 printf("Next is Macro\n'%s'\n",x.GetBuffer());
            }
            else{
                // Next is no Macro
                bNextIsM = false;
  //                     ByteString x(aEntry.sLine);
  //                     x.SearchAndReplaceAll('\n','#');
  //                     printf("Next is NO Macro\n'%s'\n",x.GetBuffer());
            }
            //}
        }else if( nLen > 2 && aEntry.sLine.GetChar( nLen-1 ) == '\n' ){
            if( aEntry.nTyp != IGNOREDTOKENS ){
                if( nLen > 2 && ( aEntry.sLine.GetChar( nLen-2  ) == '\\') ){
                    // Next is Macro
                    bNextIsM = true;
  //                  ByteString x(aEntry.sLine);
  //                  x.SearchAndReplaceAll('\n','#');
  //                  printf("Next is Macro\n'%s'\n",x.GetBuffer());
                }
                else{
                    // Next is no Macro
                    bNextIsM = false;
  //                  ByteString x(aEntry.sLine);
  //                  x.SearchAndReplaceAll('\n','#');
  //                  printf("Next is NO Macro\n'%s'\n",x.GetBuffer());
                }
            }
            // Pop current
            Pop( *aQueueCur );
            bLastWasM = bCurrentIsM;
            // next -> current
            bCurrentIsM = bNextIsM;
            aQref = aQueueCur;
            aQueueCur = aQueueNext;
            aQueueNext = aQref;

        }

        else{
            // Pop current
            Pop( *aQueueCur );
            bLastWasM = bCurrentIsM;
            // next -> current
            bCurrentIsM = bNextIsM;
            aQref = aQueueCur;
            aQueueCur = aQueueNext;
            aQueueNext = aQref;
        }
    }
}

void ParserQueue::Close(){
    // Pop current
    Pop( *aQueueCur );
    // next -> current
    bLastWasM = bCurrentIsM;
    bCurrentIsM = bNextIsM;
    aQref = aQueueCur;
    aQueueCur = aQueueNext;
    aQueueNext = aQref;
    bNextIsM = false;
    Pop( *aQueueNext );
};
void ParserQueue::Pop( std::queue<QueueEntry>& aQueue ){
    while( !aQueue.empty() ){
        QueueEntry aEntry = aQueue.front();
        aQueue.pop();
        aExport.Execute( aEntry.nTyp , (char*) aEntry.sLine.GetBuffer() );
    }
}
ParserQueue::ParserQueue( Export& aExportObj )
        : aExport( aExportObj ) ,
          bCurrentIsM( false ),
          bNextIsM( false ) ,
          bStart( false ) ,
          bStartNext( false ) ,
          bMflag( false ) ,
          bLastWasM( false ){
            aQueueNext = new std::queue<QueueEntry>;
            aQueueCur  = new std::queue<QueueEntry>;
}


ParserQueue::~ParserQueue(){
    if( aQueueNext )    delete aQueueNext;
    if( aQueueCur )     delete aQueueCur;
}
