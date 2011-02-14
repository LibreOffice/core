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
#include "precompiled_l10ntools.hxx"
#include <stdio.h>
#include <stdlib.h>
#include <tools/fsys.hxx>
#include "export.hxx"
#include "tokens.h"
#include "utf8conv.hxx"
#include <iostream>

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
DECLARE_LIST( FileList, ByteString * )
FileList aInputFileList;
sal_Bool bEnableExport;
sal_Bool bMergeMode;
sal_Bool bErrorLog;
sal_Bool bBreakWhenHelpText;
sal_Bool bUnmerge;
sal_Bool bUTF8;
ByteString sPrj;
ByteString sPrjRoot;
ByteString sActFileName;
ByteString sOutputFile;
ByteString sMergeSrc;
ByteString sTempFile;
ByteString sFile;
MergeDataFile *pMergeDataFile;
FILE *pTempFile;


ByteString sStrBuffer;
bool bMarcro = false;

extern "C" {
// the whole interface to lexer is in this extern "C" section


/*****************************************************************************/
extern char *GetOutputFile( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = sal_False;
    bMergeMode = sal_False;
    bErrorLog = sal_True;
    bBreakWhenHelpText = sal_False;
    bUnmerge = sal_False;
    bUTF8 = sal_True;
    sPrj = "";
    sPrjRoot = "";
    sActFileName = "";
    Export::sLanguages = "";
    Export::sForcedLanguages = "";
    sTempFile = "";
    pTempFile = NULL;
    sal_uInt16 nState = STATE_NON;
    sal_Bool bInput = sal_False;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        ByteString sSwitch( argv[ i ] );

        if (sSwitch == "-i"  || sSwitch == "-I" ) {
            nState = STATE_INPUT; // next tokens specifies source files
        }
        else if (sSwitch == "-o"  || sSwitch == "-O" ) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if (sSwitch == "-p"  || sSwitch == "-P" ) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }

        else if (sSwitch == "-r"  || sSwitch == "-R" ) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if (sSwitch == "-m"  || sSwitch == "-M" ) {
            nState = STATE_MERGESRC; // next token specifies the merge database
        }
        else if (sSwitch == "-e"  || sSwitch == "-E" ) {
            nState = STATE_ERRORLOG;
            bErrorLog = sal_False;
        }
        else if (sSwitch == "-b"  || sSwitch == "-B" ) {
            nState = STATE_BREAKHELP;
            bBreakWhenHelpText = sal_True;
        }
        else if (sSwitch == "-u"  || sSwitch == "-U" ) {
            nState = STATE_UNMERGE;
            bUnmerge = sal_True;
            bMergeMode = sal_True;
        }
        else if ( sSwitch.ToUpperAscii() == "-UTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = sal_True;
        }
        else if ( sSwitch.ToUpperAscii() == "-NOUTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = sal_False;
        }
        else if ( sSwitch == "-l"  || sSwitch == "-L" ) {
            nState = STATE_LANGUAGES;
        }
        else {
            switch ( nState ) {
                case STATE_NON: {
                    return NULL;    // no valid command line
                }
                case STATE_INPUT: {
                    aInputFileList.Insert( new ByteString( argv[ i ]), LIST_APPEND );
                    bInput = sal_True; // min. one source file found
                }
                break;
                case STATE_OUTPUT: {
                    sOutputFile = ByteString( argv[ i ]); // the dest. file
                }
                break;
                case STATE_PRJ: {
                    sPrj = ByteString( argv[ i ]);
                }
                break;
                case STATE_ROOT: {
                    sPrjRoot = ByteString( argv[ i ]); // path to project root
                }
                break;
                case STATE_MERGESRC: {
                    sMergeSrc = ByteString( argv[ i ]);
                    bMergeMode = sal_True; // activate merge mode, cause merge database found
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
        bEnableExport = sal_True;
        char *pReturn = new char[ sOutputFile.Len() + 1 ];
        strcpy( pReturn, sOutputFile.GetBuffer());  // #100211# - checked
        return pReturn;
    }

    // command line is not valid
    return NULL;
}
/*****************************************************************************/
int InitExport( char *pOutput , char* pFilename )
/*****************************************************************************/
{
    // instanciate Export
    ByteString sOutput( pOutput );
    ByteString sFilename( pFilename );

    if ( bMergeMode && !bUnmerge ) {
        // merge mode enabled, so read database
        pExport = new Export(sOutput, bEnableExport, sPrj, sPrjRoot, sMergeSrc , sFilename );
    }
    else
        // no merge mode, only export
        pExport = new Export( sOutput, bEnableExport, sPrj, sPrjRoot , sFilename );
    return 1;
}

/*****************************************************************************/
int EndExport()
/*****************************************************************************/
{
    delete pExport;
    return 1;
}

extern const char* getFilename()
{
    return (*(aInputFileList.GetObject( 0 ))).GetBuffer();
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
        delete aInputFileList.GetObject(( sal_uLong ) 0 );
        aInputFileList.Remove(( sal_uLong ) 0 );

        if ( sFileName == "" ) {
            fprintf( stderr, "ERROR: Could not precompile File %s\n",
                sOrigFile.GetBuffer());
            return GetNextFile();
        }

        sTempFile = sFileName;
        Export::RemoveUTF8ByteOrderMarkerFromFile( sFileName );

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

int Parse( int nTyp, const char *pTokenText ){
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
    return sal_False;
}
}

//
// class ResData
//

void ResData::Dump(){
    printf("**************\nResData\n");
    printf("sPForm = %s , sResTyp = %s , sId = %s , sGId = %s , sHelpId = %s\n",sPForm.GetBuffer()
        ,sResTyp.GetBuffer(),sId.GetBuffer(),sGId.GetBuffer(),sHelpId.GetBuffer());

    ByteString a("*pStringList");
    ByteString b("*pUIEntries");
    ByteString c("*pFilterList");
    ByteString d("*pItemList");
    ByteString e("*pPairedList");
    ByteString f("sText");

    Export::DumpMap( f , sText );

    if( pStringList )   Export::DumpExportList( a , *pStringList );
    if( pUIEntries )    Export::DumpExportList( b , *pUIEntries );
    if( pFilterList )   Export::DumpExportList( c , *pFilterList );
    if( pItemList )     Export::DumpExportList( d , *pItemList );
    if( pPairedList )   Export::DumpExportList( e , *pPairedList );
    printf("\n");
}

void ResData::addFallbackData( ByteString& sId_in , const ByteString& sText_in ){
    //printf(" ResData::addFallbackData ( sId = %s , sText = %s )\n", sId_in.GetBuffer() , sText_in.GetBuffer() );
    aFallbackData[ sId_in ] = sText_in;
}
bool ResData::getFallbackData( ByteString& sId_in , ByteString& sText_inout ){
    sText_inout = aFallbackData[ sId_in ];
    //printf("ResData::getFallbackData( sId = %s , return sText = %s \n" , sId_in.GetBuffer(), sText_inout.GetBuffer());
    return sText_inout.Len() > 0;
}

void ResData::addMergedLanguage( ByteString& sLang ){
    aMergedLanguages[ sLang ]=ByteString("1");
}
bool ResData::isMerged( ByteString& sLang ){
    return aMergedLanguages[ sLang ].Equals("1");
}

/*****************************************************************************/
sal_Bool ResData::SetId( const ByteString &rId, sal_uInt16 nLevel )
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

        return sal_True;
    }

    return sal_False;
}

//
// class Export
//

/*****************************************************************************/
Export::Export( const ByteString &rOutput, sal_Bool bWrite,
                const ByteString &rPrj, const ByteString &rPrjRoot , const ByteString& rFile )
/*****************************************************************************/
                :
                pWordTransformer( NULL ),
                aCharSet( RTL_TEXTENCODING_MS_1252 ),
                bDefine( sal_False ),
                bNextMustBeDefineEOL( sal_False ),
                nLevel( 0 ),
                nList( LIST_NON ),
                nListIndex( 0 ),
                nListLevel( 0 ),
                bSkipFile( false ),
                sProject( sPrj ),
                sRoot( sPrjRoot ),
                bEnableExport( bWrite ),
                bMergeMode( bUnmerge ),
                bError( sal_False ),
                bReadOver( sal_False ),
                bDontWriteOutput( sal_False ),
                sFilename( rFile )
{
    pParseQueue = new ParserQueue( *this );
    (void) rPrj;
    (void) rPrjRoot;
    (void) rFile;

    if( !isInitialized ) InitLanguages();
    // used when export is enabled

    // open output stream
    if ( bEnableExport ) {
        aOutput.Open( String( rOutput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
        if( !aOutput.IsOpen() ) {
            printf("ERROR : Can't open file %s\n",rOutput.GetBuffer());
            exit ( -1 );
        }
        aOutput.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );

        aOutput.SetLineDelimiter( LINEEND_CRLF );
    }
}

/*****************************************************************************/
Export::Export( const ByteString &rOutput, sal_Bool bWrite,
                const ByteString &rPrj, const ByteString &rPrjRoot,
                const ByteString &rMergeSource , const ByteString& rFile )
/*****************************************************************************/
                :
                pWordTransformer( NULL ),
                aCharSet( RTL_TEXTENCODING_MS_1252 ),
                bDefine( sal_False ),
                bNextMustBeDefineEOL( sal_False ),
                nLevel( 0 ),
                nList( LIST_NON ),
                nListIndex( 0 ),
                nListLevel( 0 ),
                bSkipFile( false ),
                sProject( sPrj ),
                sRoot( sPrjRoot ),
                bEnableExport( bWrite ),
                bMergeMode( sal_True ),
                sMergeSrc( rMergeSource ),
                bError( sal_False ),
                bReadOver( sal_False ),
                bDontWriteOutput( sal_False ),
                sFilename( rFile )
{
    (void) rPrj;
    (void) rPrjRoot;
    (void) rFile;
    pParseQueue = new ParserQueue( *this );
    if( !isInitialized ) InitLanguages( bMergeMode );
    // used when merge is enabled

    // open output stream
    if ( bEnableExport ) {
        aOutput.Open( String( rOutput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
        aOutput.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );
        aOutput.SetLineDelimiter( LINEEND_CRLF );
    }

}

/*****************************************************************************/
void Export::Init()
/*****************************************************************************/
{
    // resets the internal status, used before parseing another file
    sActPForm = "";
    bDefine = sal_False;
    bNextMustBeDefineEOL = sal_False;
    nLevel = 0;
    nList = LIST_NON;
    nListLang = ByteString( String::CreateFromAscii(""),RTL_TEXTENCODING_ASCII_US );
    nListIndex = 0;
    while ( aResStack.Count()) {
        delete aResStack.GetObject(( sal_uLong ) 0 );
        aResStack.Remove(( sal_uLong ) 0 );
    }
}

/*****************************************************************************/
Export::~Export()
/*****************************************************************************/
{
    if( pParseQueue )
        delete pParseQueue;
    // close output stream
    if ( bEnableExport )
        aOutput.Close();
    while ( aResStack.Count()) {
        delete aResStack.GetObject(( sal_uLong ) 0 );
        aResStack.Remove(( sal_uLong ) 0 );
    }

    if ( bMergeMode && !bUnmerge ) {
        if ( !pMergeDataFile )
            pMergeDataFile = new MergeDataFile( sMergeSrc,sFile , bErrorLog, aCharSet);//, bUTF8 );

        //pMergeDataFile->WriteErrorLog( sActFileName );
        delete pMergeDataFile;
    }
}

/*****************************************************************************/
int Export::Execute( int nToken, const char * pToken )
/*****************************************************************************/
{

    ByteString sToken( pToken );
    ByteString sOrig( sToken );
/*  printf("+---------------\n");
    printf("sToken = %s\n",sToken.GetBuffer());
    printf("nToken = %d\n",nToken);
    printf("+---------------\n"); */
    sal_Bool bWriteToMerged = bMergeMode;

    if ( nToken == CONDITION ) {
        ByteString sTestToken( pToken );
        sTestToken.EraseAllChars( '\t' );
        sTestToken.EraseAllChars( ' ' );
        if (( !bReadOver ) && ( sTestToken.Search( "#ifndef__RSC_PARSER" ) == 0 ))
            bReadOver = sal_True;
        else if (( bReadOver ) && ( sTestToken.Search( "#endif" ) == 0 ))
            bReadOver = sal_False;
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
                    bDefine = sal_False;
                    if ( bMergeMode ) {
                        /*if ( bDontWriteOutput && bUnmerge ) {
                            bDontWriteOutput = sal_False;
                            bNextMustBeDefineEOL = sal_False;
                            bDefine = sal_True;
                        }*/
                        MergeRest( pResData );
                    }
                    bNextMustBeDefineEOL = sal_False;
                    Execute( LEVELDOWN, "" );
                }
                else {
                    // next line also in macro definition
                    bNextMustBeDefineEOL = sal_False;
                    if ( bMergeMode )
                        WriteToMerged( sOrig , false );
                    return 1;
                }
            }
            else if (( nToken != LISTASSIGNMENT ) && ( nToken != UIENTRIES )){
                // cur. line has macro line end
                ByteString sTmpLine( sToken );
                sTmpLine.EraseAllChars( '\t' ); sTmpLine.EraseAllChars( ' ' );
                #if 0
                // impossible, unsigned is never negative
                if( sTmpLine.Len() < 0 ){
                    if ( sTmpLine.GetChar(( sal_uInt16 )( sTmpLine.Len() - 1 )) != '\\' )
                        bNextMustBeDefineEOL = sal_True;
                }
                #endif
            }
        }
    }

    sal_Bool bExecuteDown = sal_False;
    if ( nToken != LEVELDOWN ) {
        sal_uInt16 nOpen = 0;
        sal_uInt16 nClose = 0;
        sal_Bool bReadOver1 = sal_False;
        sal_uInt16 i = 0;
        for ( i = 0; i < sToken.Len(); i++ ) {
            if ( sToken.GetChar( i ) == '\"' )
                bReadOver1 = !bReadOver1;
            if ( !bReadOver1 && ( sToken.GetChar( i ) == '{' ))
                nOpen++;
        }

        bReadOver1 = sal_False;
        for ( i = 0; i < sToken.Len(); i++ ) {
            if ( sToken.GetChar( i ) == '\"' )
                bReadOver1 = !bReadOver1;
            if ( !bReadOver1 && ( sToken.GetChar( i ) == '}' ))
                nClose++;
        }

        if ( nOpen < nClose )
            bExecuteDown = sal_True;
    }
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
            bDefine = sal_True; // res. defined in macro

        case RESSOURCE:
        case RESSOURCEEXPR: {
            bDontWriteOutput = sal_False;
            if ( nToken != RSCDEFINE )
                bNextMustBeDefineEOL = sal_False;
            // this is the beginning of a new res.
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack.GetObject( nLevel - 2 )->bChild = sal_True;
            }

            // create new instance for this res. and fill mandatory fields

            pResData = new ResData( sActPForm, FullId() , sFilename );
            aResStack.Insert( pResData, LIST_APPEND );
            ByteString sBackup( sToken );
            sToken.EraseAllChars( '\n' );
            sToken.EraseAllChars( '\r' );
            sToken.EraseAllChars( '{' );
            while( sToken.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
            sToken.EraseTrailingChars( ' ' );
            ByteString sT =  sToken.GetToken( 0, ' ' );
            pResData->sResTyp = sT.ToLowerAscii();
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
            bDontWriteOutput = sal_False;
            // this is the beginning of a new res.
            bNextMustBeDefineEOL = sal_False;
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack.GetObject( nLevel - 2 )->bChild = sal_True;
            }

            // create new instance for this res. and fill mandatory fields

            pResData = new ResData( sActPForm, FullId() , sFilename );
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

            bDontWriteOutput = sal_False;
            ByteString sLowerTyp;
            if ( pResData )
                sLowerTyp = "unknown";
            nLevel++;
            if ( nLevel > 1 ) {
                aResStack.GetObject( nLevel - 2 )->bChild = sal_True;
            }

            ResData *pNewData = new ResData( sActPForm, FullId() , sFilename );
            pNewData->sResTyp = sLowerTyp;
            aResStack.Insert( pNewData, LIST_APPEND );
        }
        break;
        case LEVELDOWN: {
            // pop
            if ( !nList  ) {
                bDontWriteOutput = sal_False;
                if ( nLevel ) {
                    if ( bDefine && (nLevel == 1 )) {
                        bDefine = sal_False;
                        bNextMustBeDefineEOL = sal_False;
                    }
                    WriteData( pResData );
                    delete aResStack.GetObject( nLevel - 1 );
                    aResStack.Remove( nLevel - 1 );
                    nLevel--;
                }
            }
            else {
                if ( bDefine )
                    bNextMustBeDefineEOL = sal_True;
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
            bDontWriteOutput = sal_False;
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
                //if ( bUnmerge ){
                //  ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                //}

                pResData->bList = sal_True;
                nList = LIST_STRING;
                //ByteString sLang("en-US" , RTL_TEXTENCODING_ASCII_US );
                nListLang = SOURCE_LANGUAGE;
                nListIndex = 0;
                nListLevel = 0;
            }
            else if ( sKey == "FILTERLIST" ) {
                //if ( bUnmerge ){
                //  ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                //}
                pResData->bList = sal_True;
                nList = LIST_FILTER;
                //ByteString sLang("en-US" , RTL_TEXTENCODING_ASCII_US );
                nListLang = SOURCE_LANGUAGE;
                nListIndex = 0;
                nListLevel = 0;
            }
            else if ( sKey == "UIENTRIES" ) {
                //if ( bUnmerge ){
                //  ( sOrig.SearchAndReplace( "=", "[ de ] =" ));}
                pResData->bList = sal_True;
                nList = LIST_UIENTRIES;
                //ByteString sLang("en-US" , RTL_TEXTENCODING_ASCII_US );
                nListLang = SOURCE_LANGUAGE;
                nListIndex = 0;
                nListLevel = 0;
            }
            if (( sToken.Search( "{" ) != STRING_NOTFOUND ) &&
                ( sToken.GetTokenCount( '{' ) > sToken.GetTokenCount( '}' )))
            {
                //WorkOnTokenSet( LEVELUP, pTkn );
                Parse( LEVELUP, "" );
            }
            //if ( bUnmerge && ( nListLang.EqualsIgnoreCaseAscii("de") || nListLang.EqualsIgnoreCaseAscii("en-US") ) && ListExists( pResData, nList ))
            //  bDontWriteOutput = sal_True;
         }
        break;
        case UIENTRIES:
        case LISTASSIGNMENT: {
            bDontWriteOutput = sal_False;
            ByteString sTmpToken( sToken);
            sTmpToken.EraseAllChars(' ');
            sal_uInt16 nPos = 0;
            //nPos = sTmpToken.ToLowerAscii().Search("[de]=");
            nPos = sTmpToken.ToLowerAscii().Search("[en-us]=");
            if( nPos != STRING_NOTFOUND ) {
                //if ( bUnmerge ){
                //  ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                //}
                ByteString sKey = sTmpToken.Copy( 0 , nPos );
                sKey.EraseAllChars( ' ' );
                sKey.EraseAllChars( '\t' );
                ByteString sValue = sToken.GetToken( 1, '=' );
                CleanValue( sValue );
                if ( sKey.ToUpperAscii() ==  "STRINGLIST" ) {
                    pResData->bList = sal_True;
                    nList = LIST_STRING;
                    //ByteString sLang("en-US" , RTL_TEXTENCODING_ASCII_US );
                    nListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if ( sKey == "FILTERLIST" ) {
                    pResData->bList = sal_True;
                    nList = LIST_FILTER;
                    //ByteString sLang("en-US" , RTL_TEXTENCODING_ASCII_US );
                    nListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                // PairedList
                else if ( sKey ==  "PAIREDLIST" ) {
                    pResData->bList = sal_True;
                    nList = LIST_PAIRED;
                    //ByteString sLang("en-US" , RTL_TEXTENCODING_ASCII_US );
                    nListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }

                else if ( sKey ==  "ITEMLIST" ) {
                    pResData->bList = sal_True;
                    nList = LIST_ITEM;
                    //ByteString sLang("en-US" , RTL_TEXTENCODING_ASCII_US );
                    nListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                else if ( sKey ==  "UIENTRIES" ) {
                    pResData->bList = sal_True;
                    nList = LIST_UIENTRIES;
                    //ByteString sLang("en-US" , RTL_TEXTENCODING_ASCII_US );
                    nListLang = SOURCE_LANGUAGE;
                    nListIndex = 0;
                    nListLevel = 0;
                }
                /*if ( bUnmerge && ( nListLang.EqualsIgnoreCaseAscii( "de" )
                    || nListLang.EqualsIgnoreCaseAscii("en-US" ) )
                    && ListExists( pResData, nList ))
                    bDontWriteOutput = sal_True;*/
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
                else if ( sKey == "PAIREDLIST" )
                    nList = LIST_PAIRED;                // abcd
                else if ( sKey == "ITEMLIST" )
                    nList = LIST_ITEM;
                else if ( sKey == "UIENTRIES" )
                    nList = LIST_UIENTRIES;
                if ( nList ) {
                    ByteString sLang=sToken.GetToken( 1, '[' ).GetToken( 0, ']' );
                    CleanValue( sLang );
                    nListLang = sLang;
                    /*if (( bUnmerge ) && ( !nListLang.EqualsIgnoreCaseAscii("de")) && ( !nListLang.EqualsIgnoreCaseAscii("en-US")))
                        bDontWriteOutput = sal_True;*/
                    nListIndex = 0;
                    nListLevel = 0;
                    /*if ( bUnmerge && nListLang.EqualsIgnoreCaseAscii("de")  && ListExists( pResData, nList ) )
                        bDontWriteOutput = sal_True;*/
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
            bDontWriteOutput = sal_False;
            if ( nLevel ) {
                CutComment( sToken );

                // this is a text line!!!
                ByteString sKey = sToken.GetToken( 0, '=' ).GetToken( 0, '[' );
                sKey.EraseAllChars( ' ' );
                sKey.EraseAllChars( '\t' );
                ByteString sText( GetText( sToken, nToken ));
                if ( !bMergeMode )
                    sText = sText.Convert( aCharSet, RTL_TEXTENCODING_MS_1252 );
                ByteString sLang;
                if ( sToken.GetToken( 0, '=' ).Search( "[" ) != STRING_NOTFOUND ) {
                     sLang = sToken.GetToken( 0, '=' ).GetToken( 1, '[' ).GetToken( 0, ']' );
                    CleanValue( sLang );
                }
                ByteString nLangIndex = sLang;
                ByteString sOrigKey = sKey;
                if ( sText.Len() && sLang.Len() ) {
                    if (( sKey.ToUpperAscii() == "TEXT" ) ||
                        ( sKey == "MESSAGE" ) ||
                        ( sKey == "CUSTOMUNITTEXT" ) ||
                        ( sKey == "SLOTNAME" ) ||
                        ( sKey == "UINAME" ))
                    {
                        //if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND )
                        //  ( sOrig.SearchAndReplace( "=", "[ de ] =" ));

                        SetChildWithText();
                        //if ( nLangIndex.EqualsIgnoreCaseAscii("en-US") )
                        if ( Export::isSourceLanguage( nLangIndex ) )
                            pResData->SetId( sText, ID_LEVEL_TEXT );

                        pResData->bText = sal_True;
                        pResData->sTextTyp = sOrigKey;
                        if ( bMergeMode ) {
                            PrepareTextToMerge( sOrig, STRING_TYP_TEXT, nLangIndex, pResData );
                            //if ( bUnmerge )
                            //  pResData->sText[ nLangIndex ] = sText;
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
                        //if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND ){
                        //  ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                        //  }
                        SetChildWithText();
                        pResData->bHelpText = sal_True;
                        if ( bBreakWhenHelpText ) {
                            ByteString sError( "\"HelpText\" found in source\n" );
                            YYWarning( sError.GetBufferAccess());
                            sError.ReleaseBufferAccess();
                            SetError();
                        }
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_HELPTEXT, nLangIndex, pResData );
                            //if ( bUnmerge )
                            //  pResData->sHelpText[ nLangIndex ] = sText;
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
                        //if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND ){
                        //  ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                        //  }
                        SetChildWithText();
                        pResData->bQuickHelpText = sal_True;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_QUICKHELPTEXT, nLangIndex, pResData );
                            //if ( bUnmerge )
                            //  pResData->sQuickHelpText[ nLangIndex ] = sText;
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
                        //if ( bUnmerge && sToken.GetToken( 0, '=' ).Search( "[" ) == STRING_NOTFOUND ){
                        //  ( sOrig.SearchAndReplace( "=", "[ de ] =" ));
                        //  }
                        SetChildWithText();
                        pResData->bTitle = sal_True;
                        if ( bMergeMode )
                            PrepareTextToMerge( sOrig, STRING_TYP_TITLE, nLangIndex, pResData );
                            //if ( bUnmerge )
                            //  pResData->sTitle[ nLangIndex ] = sText;
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
            bDontWriteOutput = sal_True;
            // this means something like // ### Achtung : Neuer Text ...
            /*ByteString sLang( "GERMAN" );
            ByteString sText = sToken.GetToken( 2, ':' ).GetToken( 0, '*' );
            CleanValue( sText );
            if ( sText.Len())
                pResData->sText[ sLang ] = sText;*/
        }
        break;
        case APPFONTMAPPING: {
            bDontWriteOutput = sal_False;
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
                pResData->nWidth = ( sal_uInt16 ) sMapping.GetToken( 0, ',' ).ToInt64();
            }
            else if ( sKey == "POSSIZE" ) {
                pResData->nWidth = ( sal_uInt16 ) sMapping.GetToken( 2, ',' ).ToInt64();
            }
        }
        break;
        case RSCDEFINELEND:
            bDontWriteOutput = sal_False;
        break;
        case CONDITION: {
            bDontWriteOutput = sal_False;
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
                WriteData( pResData, sal_True );
                pResData->sPForm = sActPForm;
            }
        }
        break;
        case EMPTYLINE : {
            bDontWriteOutput = sal_False;
            if ( bDefine ) {
                bNextMustBeDefineEOL = sal_False;
                bDefine = sal_False;
                while ( nLevel )
                    Parse( LEVELDOWN, "" );
                    //WorkOnTokenSet( LEVELDOWN, pTkn );
            }
        }
        break;
        case PRAGMA : {
            bDontWriteOutput = sal_False;
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
            bDontWriteOutput = sal_True;
             /*ByteString sK = sToken.GetToken( 0, '=' );
            ByteString sKey = sK.EraseAllChars( '\t' ).EraseAllChars( ' ' );
            ByteString sT = sToken.GetToken( 1, '=' ).GetToken( 0, ';' );
            sal_uInt16 nRefId = ( sal_uInt16 ) sT.EraseAllChars( '\t' ).EraseAllChars( ' ' ).ToInt32();
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
                pResData->nTitleRefId = nRefId;*/
        }
        }
    if ( bWriteToMerged ) {
        // the current token must be written to dest. without merging

        if( bDefine && sOrig.Len() > 2 ){
            for( sal_uInt16 n = 0 ; n < sOrig.Len() ; n++ ){
                if( sOrig.GetChar( n ) == '\n' && sOrig.GetChar( n-1 ) != '\\'){
                    sOrig.Insert('\\' , n++ );
                }
            }
        }
        WriteToMerged( sOrig , false);
    }

    if ( bExecuteDown ) {
        Parse( LEVELDOWN, "" );
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
        sal_uInt16 i = 0;
        sal_Bool bInner = sal_False;

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

void Export::UnmergeUTF8( ByteString& sOrig ){
    sal_uInt16 nPos1 = sOrig.Search('\"');
    sal_uInt16 nPos2 = sOrig.SearchBackward('\"');
    if( nPos1 > 0 && nPos2 > 0 && nPos1 < nPos2){
        ByteString sPart = sOrig.Copy(nPos1+1 , nPos2-1);
        ByteString sPartUTF8 = sPart;
        sPartUTF8.Convert( RTL_TEXTENCODING_MS_1252 , RTL_TEXTENCODING_UTF8 );
        sOrig.SearchAndReplace( sPart , sPartUTF8 );
    }
}

/*****************************************************************************/
sal_Bool Export::ListExists( ResData *pResData, sal_uInt16 nLst )
/*****************************************************************************/
{
    switch ( nLst ) {
        case LIST_STRING: return pResData->pStringList != NULL;
        case LIST_FILTER: return pResData->pFilterList != NULL;
        case LIST_ITEM: return pResData->pItemList != NULL;
        case LIST_PAIRED: return pResData->pPairedList != NULL;
        case LIST_UIENTRIES: return pResData->pUIEntries != NULL;
    }
    return sal_False;
}

/*****************************************************************************/
sal_Bool Export::WriteData( ResData *pResData, sal_Bool bCreateNew )
/*****************************************************************************/
{
    if ( bMergeMode ) {
        MergeRest( pResData );
        return sal_True;
    }

    if ( bUnmerge )
        return sal_True;

/*    ByteStringHashMap::iterator pos3 = pResData->sText.begin();
    ByteStringHashMap::iterator end3 = pResData->sText.end();
    for(;pos3!=end3;++pos3){

        printf("[%s]=%s\n", pos3->first.GetBuffer(), pos3->second.GetBuffer() );
    }*/
       // mandatory to export: en-US

     if (( //pResData->sText[ ByteString("de") ].Len() &&
        ( pResData->sText[ SOURCE_LANGUAGE ].Len()))
        ||
        ( //pResData->sHelpText[ ByteString("de") ].Len() &&
        (  pResData->sHelpText[ SOURCE_LANGUAGE ].Len()))
        ||
        ( //pResData->sQuickHelpText[ ByteString("de") ].Len() &&
        (  pResData->sQuickHelpText[ SOURCE_LANGUAGE ].Len()))
         ||
        ( //pResData->sTitle[ ByteString("de") ].Len() &&
        (  pResData->sTitle[ SOURCE_LANGUAGE ].Len())))

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

        for( unsigned int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];
                if ( !sCur.EqualsIgnoreCaseAscii("x-comment") ){
                    if ( pResData->sText[ sCur ].Len())
                        sXText = pResData->sText[ sCur ];
                    else {
                        sXText = pResData->sText[ SOURCE_LANGUAGE ];
                        /*if ( !sXText.Len())
                            sXText = pResData->sText[ ByteString("en") ];
                        if ( !sXText.Len())
                            sXText = pResData->sText[ ByteString("de") ];*/
                    }

                    if ( pResData->sHelpText[ sCur ].Len())
                        sXHText = pResData->sHelpText[ sCur ];
                    else {
                        sXHText = pResData->sHelpText[ SOURCE_LANGUAGE ];
                        /*if ( !sXHText.Len())
                            sXHText = pResData->sHelpText[ ByteString("en") ];
                        if ( !sXText.Len())
                            sXHText = pResData->sHelpText[ ByteString("de") ];*/
                    }

                    if ( pResData->sQuickHelpText[ sCur ].Len())
                        sXQHText = pResData->sQuickHelpText[ sCur ];
                    else {
                        sXQHText = pResData->sQuickHelpText[ SOURCE_LANGUAGE ];
                        /*if ( !sXQHText.Len())
                            sXQHText = pResData->sQuickHelpText[ ByteString("en") ];
                        if ( !sXQHText.Len())
                            sXQHText = pResData->sQuickHelpText[ ByteString("de") ];*/
                    }

                    if ( pResData->sTitle[ sCur ].Len())
                        sXTitle = pResData->sTitle[ sCur ];
                    else {
                        sXTitle = pResData->sTitle[ SOURCE_LANGUAGE ];
                        /*if ( !sXTitle.Len())
                            sXTitle = pResData->sTitle[ ByteString("en") ];
                        if ( !sXTitle.Len())
                            sXTitle = pResData->sTitle[ ByteString("de") ];*/
                    }

                    if ( !sXText.Len())
                        sXText = "-";

                    if ( !sXHText.Len()) {
                        /*if ( pResData->sHelpText[ ByteString("de") ].Len())
                            sXHText = pResData->sHelpText[ ByteString("de") ];*/
                        if ( pResData->sHelpText[ SOURCE_LANGUAGE ].Len())
                            sXHText = pResData->sHelpText[ SOURCE_LANGUAGE ];
                        /*else if ( pResData->sHelpText[ ByteString("en") ].Len())
                            sXHText = pResData->sHelpText[ ByteString("en") ];*/
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
                    sOutput += pResData->sHelpId; sOutput   += "\t";
                    sOutput += pResData->sPForm; sOutput    += "\t";
                    sOutput += ByteString::CreateFromInt64( pResData->nWidth ); sOutput += "\t";
                    sOutput += sCur; sOutput += "\t";


                    sOutput += sXText; sOutput  += "\t";
                    sOutput += sXHText; sOutput += "\t";
                    sOutput += sXQHText; sOutput+= "\t";
                    sOutput += sXTitle; sOutput += "\t";
                    sOutput += sTimeStamp;

                 // if( !sCur.EqualsIgnoreCaseAscii("de") ||( sCur.EqualsIgnoreCaseAscii("de") && !Export::isMergingGermanAllowed( sProject ) ) )
                    aOutput.WriteLine( sOutput );
                }

                if ( bCreateNew ) {
                    pResData->sText[ sCur ]         = "";
                    pResData->sHelpText[ sCur ]     = "";
                    pResData->sQuickHelpText[ sCur ]= "";
                    pResData->sTitle[ sCur ]        = "";
                }
            }
    }
    FillInFallbacks( pResData );
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
    if ( pResData->pPairedList ) {
        ByteString sList( "pairedlist" );
        WriteExportList( pResData, pResData->pPairedList, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pItemList = 0;
    }
    if ( pResData->pUIEntries ) {
        ByteString sList( "uientries" );
        WriteExportList( pResData, pResData->pUIEntries, sList, bCreateNew );
        if ( bCreateNew )
            pResData->pUIEntries = 0;
    }
    return sal_True;
}
ByteString Export::GetPairedListID( const ByteString& sText ){
// < "STRING" ; IDENTIFIER ; > ;
    ByteString sIdent = sText.GetToken( 1, ';' );
    sIdent.ToUpperAscii();
    while( sIdent.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
    sIdent.EraseTrailingChars( ' ' );
    sIdent.EraseLeadingChars( ' ' );
    return sIdent;
}
ByteString Export::GetPairedListString( const ByteString& sText ){
// < "STRING" ; IDENTIFIER ; > ;
    ByteString sString = sText.GetToken( 0, ';' );
    while( sString.SearchAndReplace( "\t", " " ) != STRING_NOTFOUND ) {};
    sString.EraseTrailingChars( ' ' );
    ByteString s1 = sString.Copy( sString.Search( '\"' )+1 );
    sString = s1.Copy( 0 , s1.SearchBackward( '\"' ) );
    sString.EraseTrailingChars( ' ' );
    sString.EraseLeadingChars( ' ' );
    return sString;
}
ByteString Export::StripList( const ByteString& sText ){
    ByteString s1 = sText.Copy( sText.Search( '\"' ) + 1 );
    return s1.Copy( 0 , s1.SearchBackward( '\"' ) );
}

/*****************************************************************************/
sal_Bool Export::WriteExportList( ResData *pResData, ExportList *pExportList,
                        const ByteString &rTyp, sal_Bool bCreateNew )
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
    for ( sal_uLong i = 0; pExportList != NULL && i < pExportList->Count(); i++ ) {
        ExportListEntry *pEntry = pExportList->GetObject( i );
                // mandatory for export: german and eng. and/or enus
        //ByteString a("Export::WriteExportList::pEntry");
        //Export::DumpMap( a,  *pEntry );

        ByteString sLID( ByteString::CreateFromInt64( i + 1 ));
        for( unsigned int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];
            if ( //1 )
                  //(*pEntry)[ ByteString("de") ].Len() &&
                     (*pEntry)[ SOURCE_LANGUAGE ].Len() )
                    //||
                     //     (*pEntry)[ ByteString("en") ].Len()))
                {
                    if ( bEnableExport )
                    {
                        ByteString sText((*pEntry)[ SOURCE_LANGUAGE ] );

                        // Strip PairList Line String
                        if( rTyp.EqualsIgnoreCaseAscii("pairedlist") ){
                            sLID = GetPairedListID( sText );
                            if ((*pEntry)[ sCur ].Len())
                                sText = (*pEntry)[ sCur ];
                            sText = GetPairedListString( sText );
                        }
                        else{
                            //if ((*pEntry)[ sCur ].Len()){
                            //  if( sCur.EqualsIgnoreCaseAscii("de") ){
                            //      sText = StripList( (*pEntry)[ sCur ] );
                            //  }
                            //  else
                                    sText = StripList( (*pEntry)[ sCur ] );
                                    if( sText == "\\\"" )
                                        sText = "\"";
                            //}
                        }

                        ByteString sOutput( sProject ); sOutput += "\t";
                        if ( sRoot.Len())
                            sOutput += sActFileName;
                        sOutput += "\t0\t";
                        sOutput += rTyp; sOutput += "\t";
                        sOutput += sGID; sOutput += "\t";
                        sOutput += sLID; sOutput += "\t\t";
                        sOutput += pResData->sPForm; sOutput += "\t0\t";
                        sOutput += sCur; sOutput += "\t";

                        sOutput += sText; sOutput += "\t\t\t\t";
                        sOutput += sTimeStamp;

                        //if( !sCur.EqualsIgnoreCaseAscii("de") ||( sCur.EqualsIgnoreCaseAscii("de") && !Export::isMergingGermanAllowed( sProject ) ) )
                        aOutput.WriteLine( sOutput );

                    }
                }
        }
        if ( bCreateNew )
            delete [] pEntry;
    }
    if ( bCreateNew )
        delete pExportList;

    return sal_True;
}

/*****************************************************************************/
ByteString Export::FullId()
/*****************************************************************************/
{
    ByteString sFull;
    if ( nLevel > 1 ) {
        sFull = aResStack.GetObject( 0 )->sId;
        for ( sal_uInt16 i = 1; i < nLevel - 1; i++ ) {
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
    else if ( nList == LIST_PAIRED ) {
        pList = pResData->pPairedList;
        if ( !pList ) {
            pResData->pPairedList = new ExportList();
            pList = pResData->pPairedList;
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

    // For paired list use the line to set proper lid
    if( nList == LIST_PAIRED ){
        (*pCurEntry)[ nListLang ] = rLine;
    }else
        (*pCurEntry)[ nListLang ] = rText;

    // Remember en-US fallback string, so each list has the same amount of elements
    //if ( nListLang.EqualsIgnoreCaseAscii("en-US")  ) {
    if ( Export::isSourceLanguage( nListLang ) ) {
        if( nList == LIST_PAIRED ){
            const ByteString sPlist("pairedlist");
            ByteString sKey = MergeDataFile::CreateKey( sPlist , pResData->sId , GetPairedListID( rLine ) , sFilename );
            pResData->addFallbackData( sKey , rText );
        }
        // new fallback
        else{
            const ByteString sPlist("list");
            ByteString a( pResData->sGId );
            a.Append( "." );
            a.Append( pResData->sId );
            sal_Int64 x = nListIndex+1;
            ByteString b( ByteString::CreateFromInt64( x ) );
            ByteString sKey = MergeDataFile::CreateKey( sPlist , a , b  , sFilename );
            pResData->addFallbackData( sKey , rText );
        }
        // new fallback
    }

    //if ( nListLang.EqualsIgnoreCaseAscii("en-US")  ) {
    if ( Export::isSourceLanguage( nListLang ) ) {
        if( nList == LIST_PAIRED ){
            (*pCurEntry)[ SOURCE_LANGUAGE ] = rLine;
        }
        else
            (*pCurEntry)[ SOURCE_LANGUAGE ] = rLine;

        pList->NewSourceLanguageListEntry();
    }

    //printf("Export::InsertListEntry ResData.id = %s ResData.ListData = %s\n",pResData->sId.GetBuffer() ,(*pCurEntry)[ nListLang ].GetBuffer());
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
        for ( sal_uInt16 i = rValue.Len() - 1; i > 0; i-- ) {
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
ByteString Export::GetText( const ByteString &rSource, int nToken )
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

            sal_uInt16 nStart = 0;
            sal_uInt16 nState = TXT_STATE_MACRO;

            nState = TXT_STATE_TEXT;
            nStart = 1;


            for ( sal_uInt16 i = nStart; i < sTmp.GetTokenCount( '\"' ); i++ ) {
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
    //printf("%s\n",rText.GetBuffer() );

    #if 0
    // statement has no effect
    if( pParseQueue->bMflag && !bSDFContent ) pParseQueue->bMflag;
    #endif

    if ( !bDontWriteOutput || !bUnmerge ) {
        ByteString sText( rText );
        while ( sText.SearchAndReplace( " \n", "\n" ) != STRING_NOTFOUND ) {};
        if( pParseQueue->bNextIsM && bSDFContent && sText.Len() > 2 ){
            for( sal_uInt16 n = 0 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );

                }
            }
        }
        else if( pParseQueue->bLastWasM && sText.Len() > 2 ){
            for( sal_uInt16 n = 0 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );
                }
                if( sText.GetChar( n ) == '\n' )pParseQueue->bMflag=true;
            }
        }
        else if( pParseQueue->bCurrentIsM && bSDFContent && sText.Len() > 2 ){
            for( sal_uInt16 n = 0 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );
                    pParseQueue->bMflag=true;
                }
            }
        }
        else if( pParseQueue->bMflag ){
            for( sal_uInt16 n = 1 ; n < sText.Len() ; n++ ){
                if( sText.GetChar( n ) == '\n' && sText.GetChar( n-1 ) != '\\'){
                    sText.Insert('\\' , n++ );
                }
            }
        }
        for ( sal_uInt16 i = 0; i < sText.Len(); i++ ) {
            if ( sText.GetChar( i ) != '\n' ){
                aOutput.Write( ByteString( sText.GetChar( i )).GetBuffer(), 1 );

            }
            else{
                aOutput.WriteLine( ByteString());
            }

        }
    }
}

/*****************************************************************************/
void Export::ConvertMergeContent( ByteString &rText )
/*****************************************************************************/
{
    sal_Bool bNoOpen = ( rText.Search( "\\\"" ) != 0 );
    ByteString sClose( rText.Copy( rText.Len() - 2 ));
    sal_Bool bNoClose = ( sClose != "\\\"" );
    ByteString sNew;
    for ( sal_uInt16 i = 0; i < rText.Len(); i++ ) {
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

    if ( bNoOpen ) {
        ByteString sTmp( rText );
        rText = "\"";
        rText += sTmp;
    }
    if ( bNoClose )
        rText += "\"";
}

/*****************************************************************************/
sal_Bool Export::PrepareTextToMerge( ByteString &rText, sal_uInt16 nTyp,
                                ByteString &nLangIndex, ResData *pResData )
/*****************************************************************************/
{
    // position to merge in:
    sal_uInt16 nStart = 0;
    sal_uInt16 nEnd = 0;
    ByteString sOldId = pResData->sId;
    ByteString sOldGId = pResData->sGId;
    ByteString sOldTyp = pResData->sResTyp;

    ByteString sOrigText( rText );

    switch ( nTyp ) {
        case LIST_STRING :
        case LIST_UIENTRIES :
        case LIST_FILTER :
        case LIST_PAIRED:
        case LIST_ITEM :
        {
            if ( bUnmerge )
                return sal_True;

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
                case LIST_PAIRED : {
                    pResData->sResTyp = "pairedlist";
                    pList = pResData->pPairedList;
                }
                break;

            }
            if ( pList ) {
                ExportListEntry *pCurEntry = pList->GetObject( nListIndex - 1 );
                if ( pCurEntry ) {
                    //printf("%s\n",Export::DumpMap( "pCurEntry", *pCurEntry ).GetBuffer() );
                    //ByteString a("pCurEntry");
                    //Export::DumpMap( a , *pCurEntry );
                    rText = (*pCurEntry)[ SOURCE_LANGUAGE ];
                    if( nTyp == LIST_PAIRED ){
                        pResData->addMergedLanguage( nLangIndex );
                    }
                }
            }

            nStart = rText.Search( "\"" );
            if ( nStart == STRING_NOTFOUND ) {
                rText = sOrigText;
                return sal_False;
            }

            sal_Bool bFound = sal_False;
            for ( nEnd = nStart + 1; nEnd < rText.Len() && !bFound; nEnd++ ) {
                if ( rText.GetChar( nEnd ) == '\"' )
                    bFound = sal_True;
            }
            if ( !bFound ) {
                rText = sOrigText;
                return sal_False;
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
            if ( pResData->sResTyp.EqualsIgnoreCaseAscii( "pairedlist" ) ){
               pResData->sId = GetPairedListID( sLastListLine );
            }
            else pResData->sId = ByteString::CreateFromInt32( nListIndex );

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
            /*if ( bUnmerge ) {
                if (( nLangIndex != ByteString("de") ) &&
                    ( nLangIndex != ByteString("en-US") ))
                {
                    bDontWriteOutput = sal_True;
                }
                return sal_True;
            }*/

            nStart = rText.Search( "=" );
            if ( nStart == STRING_NOTFOUND ) {
                rText = sOrigText;
                return sal_False;
            }

            nStart++;
            sal_Bool bFound = sal_False;
            while(( nStart < rText.Len()) && !bFound ) {
                if (( rText.GetChar( nStart ) != ' ' ) && ( rText.GetChar( nStart ) != '\t' ))
                    bFound = sal_True;
                else
                    nStart ++;
            }

            // no start position found
            if ( !bFound ) {
                rText = sOrigText;
                return sal_False;
            }

            // position to end mergeing in
            nEnd = rText.Len() - 1;
            bFound = sal_False;

            while (( nEnd > nStart ) && !bFound ) {
                if (( rText.GetChar( nEnd ) != ' ' ) && ( rText.GetChar( nEnd ) != '\t' ) &&
                    ( rText.GetChar( nEnd ) != '\n' ) && ( rText.GetChar( nEnd ) != ';' ) &&
                    ( rText.GetChar( nEnd ) != '{' ) && ( rText.GetChar( nEnd ) != '\\' ))
                {
                    bFound = sal_True;
                }
                else
                    nEnd --;
            }
        }
        break;
    }

    // search for merge data
    if ( !pMergeDataFile ){
        pMergeDataFile = new MergeDataFile( sMergeSrc, sFile , bErrorLog, aCharSet);//, bUTF8 );

        // Init Languages
        ByteString sTmp = Export::sLanguages;
        if( sTmp.ToUpperAscii().Equals("ALL") )
            SetLanguages( pMergeDataFile->GetLanguages() );
        else if( !isInitialized )InitLanguages();

    }
//  printf("*************DUMPING****************\n");
//  printf("%s\n",pMergeDataFile->Dump().GetBuffer());
//  printf("*************DUMPING****************\n");

//  printf("Dumping ResData\n");
//  pResData->Dump();
    PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
    //printf("Dumping pEntrys\n");
    //if( pEntrys ) pEntrys->Dump();
    pResData->sId = sOldId;
    pResData->sGId = sOldGId;
    pResData->sResTyp = sOldTyp;

    if ( !pEntrys ) {
        rText = sOrigText;
        return sal_False; // no data found
    }

    ByteString sContent;
    pEntrys->GetTransex3Text( sContent, nTyp, nLangIndex );
    //if ( !sContent.Len() && ( ! nLangIndex.EqualsIgnoreCaseAscii("en-US") )) {
    if ( !sContent.Len() && ( ! Export::isSourceLanguage( nLangIndex ) )) {
        rText = sOrigText;
        return sal_False; // no data found
    }

    //if ( nLangIndex.EqualsIgnoreCaseAscii("en-US") ) {
    if ( Export::isSourceLanguage( nLangIndex ) ) {
        return sal_False;
    }

    ByteString sPostFix( rText.Copy( ++nEnd ));
    rText.Erase( nStart );

    //ConvertMergeContent( sContent, nTyp );
    ConvertMergeContent( sContent );



    //printf("Merged %s\n",nLangIndex.GetBuffer());
    // merge new res. in text line
    rText += sContent;
    rText += sPostFix;

    return sal_True;
}

/*****************************************************************************/
void Export::MergeRest( ResData *pResData, sal_uInt16 nMode )
/*****************************************************************************/
{
    //if ( bUnmerge ) { return;}

    //pResData->Dump();

    if ( !pMergeDataFile ){
        pMergeDataFile = new MergeDataFile( sMergeSrc, sFile ,bErrorLog, aCharSet);//, bUTF8 );

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

                sal_Bool bAddSemikolon = sal_False;
                sal_Bool bFirst = sal_True;
                ByteString sCur;
                ByteString sTmp = Export::sLanguages;

                for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                    sCur = aLanguages[ n ];

                    ByteString sText;
                    sal_Bool bText = pEntry->GetTransex3Text( sText, STRING_TYP_TEXT, sCur , sal_True );
                    if ( bText && sText.Len() && sText != "-" ) {
                        ByteString sOutput;
                        if ( bNextMustBeDefineEOL)  {
                            if ( bFirst )
                                sOutput += "\t\\\n";
                            else
                                sOutput += ";\t\\\n";
                        }
                        bFirst=sal_False;
                        sOutput += "\t";
                        sOutput += pResData->sTextTyp;
                        //if ( !sCur.EqualsIgnoreCaseAscii("en-US")) {
                        if ( ! Export::isSourceLanguage( sCur ) ) {
                            sOutput += "[ ";
                            sOutput += sCur;
                            sOutput += " ] ";
                        }
                        sOutput += "= ";
                        ConvertMergeContent( sText );
                        sOutput += sText;

                        if ( bDefine && bWriteNoSlash )
                            sOutput += ";\n";

                        if ( bDefine )
                            sOutput += ";\\\n";
                        else if ( !bNextMustBeDefineEOL )
                            sOutput += ";\n";
                        else
                            bAddSemikolon = sal_True;
                        for ( sal_uInt16 j = 1; j < nLevel; j++ )
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
                sal_Bool bAddSemikolon = sal_False;
                sal_Bool bFirst = sal_True;
                ByteString sCur;

                for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                    sCur = aLanguages[ n ];

                    ByteString sText;
                    sal_Bool bText = pEntry->GetTransex3Text( sText, STRING_TYP_QUICKHELPTEXT, sCur, sal_True );
                    if ( bText && sText.Len() && sText != "-" ) {
                        ByteString sOutput;
                        if ( bNextMustBeDefineEOL)  {
                            if ( bFirst )
                                sOutput += "\t\\\n";
                            else
                                sOutput += ";\t\\\n";
                        }
                        bFirst=sal_False;
                        sOutput += "\t";
                        sOutput += "QuickHelpText";
                        //if ( !sCur.EqualsIgnoreCaseAscii("en-US") ) {
                        if ( ! Export::isSourceLanguage( sCur ) ) {
                            sOutput += "[ ";
                            sOutput += sCur;
                            sOutput += " ] ";
                        }
                        sOutput += "= ";
                        ConvertMergeContent( sText );
                        sOutput += sText;
                        if ( bDefine )
                            sOutput += ";\\\n";
                        else if ( !bNextMustBeDefineEOL )
                            sOutput += ";\n";
                        else
                            bAddSemikolon = sal_True;
                        for ( sal_uInt16 j = 1; j < nLevel; j++ )
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
                sal_Bool bAddSemikolon = sal_False;
                sal_Bool bFirst = sal_True;
                ByteString sCur;

                for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                    sCur = aLanguages[ n ];

                ByteString sText;
                    sal_Bool bText = pEntry->GetTransex3Text( sText, STRING_TYP_TITLE, sCur, sal_True );
                    if ( bText && sText.Len() && sText != "-" ) {
                        ByteString sOutput;
                        if ( bNextMustBeDefineEOL)  {
                            if ( bFirst )
                                sOutput += "\t\\\n";
                            else
                                sOutput += ";\t\\\n";
                        }
                        bFirst=sal_False;
                        sOutput += "\t";
                        sOutput += "Title";
                        //if ( !sCur.EqualsIgnoreCaseAscii("en-US") ) {
                        if ( ! Export::isSourceLanguage( sCur ) ) {
                            sOutput += "[ ";
                            sOutput += sCur;
                            sOutput += " ] ";
                        }
                        sOutput += "= ";
                        ConvertMergeContent( sText );
                        sOutput += sText;
                        if ( bDefine )
                            sOutput += ";\\\n";
                        else if ( !bNextMustBeDefineEOL )
                            sOutput += ";\n";
                        else
                            bAddSemikolon = sal_True;
                        for ( sal_uInt16 j = 1; j < nLevel; j++ )
                            sOutput += "\t";
                        WriteToMerged( sOutput ,true );
                    }
                }
                if ( bAddSemikolon ) {
                    ByteString sOutput( ";" );
                    WriteToMerged( sOutput ,false);
                }
            }
            // Merge Lists

            if ( pResData->bList ) {
                //printf("Dumping ResData\n");
                //pResData->Dump();

                bool bPairedList = false;
                ByteString sOldId = pResData->sId;
                ByteString sOldGId = pResData->sGId;
                ByteString sOldTyp = pResData->sResTyp;
                if ( pResData->sGId.Len())
                    pResData->sGId += ".";
                pResData->sGId += sOldId;
                ByteString sSpace;
                for ( sal_uInt16 i = 1; i < nLevel-1; i++ )
                    sSpace += "\t";
                for ( sal_uInt16 nT = LIST_STRING; nT <= LIST_UIENTRIES; nT++ ) {
                    ExportList *pList = NULL;
                    switch ( nT ) {
                        case LIST_STRING : pResData->sResTyp = "stringlist"; pList = pResData->pStringList; bPairedList = false; break;
                        case LIST_FILTER : pResData->sResTyp = "filterlist"; pList = pResData->pFilterList; bPairedList = false; break;
                        case LIST_UIENTRIES : pResData->sResTyp = "uientries"; pList = pResData->pUIEntries;bPairedList = false; break;
                        case LIST_ITEM : pResData->sResTyp = "itemlist"; pList = pResData->pItemList;       bPairedList = false; break;
                        case LIST_PAIRED : pResData->sResTyp = "pairedlist"; pList = pResData->pPairedList; bPairedList = true;  break;
                    }
                    ByteString sCur;
                    for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                        sCur = aLanguages[ n ];
                        sal_uInt16 nIdx = 1;

                        // Set matching pairedlist identifier
                        if( bPairedList && pResData->pPairedList && ( nIdx == 1 ) ){
                            ExportListEntry* pListE = ( ExportListEntry* ) pResData->pPairedList->GetObject( nIdx-1 );
                            pResData->sId = GetPairedListID ( (*pListE)[ SOURCE_LANGUAGE ] );
                        }
                        else
                            pResData->sId = ByteString("1");

                        PFormEntrys *pEntrys;
                        sal_uLong nLIndex = 0;
                        sal_uLong nMaxIndex = 0;
                        if ( pList )
                            nMaxIndex = pList->GetSourceLanguageListEntryCount();
                        pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
                        while( pEntrys  && ( nLIndex < nMaxIndex )) {
                            //printf("Lang %s, List Index %d\n",sCur.GetBuffer(),(int)nLIndex);
                            ByteString sText;
                            sal_Bool bText;
                            bText = pEntrys->GetTransex3Text( sText, STRING_TYP_TEXT, sCur, sal_True );
                            if( !bText )
                                bText = pEntrys->GetTransex3Text( sText , STRING_TYP_TEXT, SOURCE_LANGUAGE , sal_False );

                            // Use fallback, if data is missing in sdf file
                            //if( !bText && pResData->sResTyp.Equals( "pairedlist" ) ){
                            if( !bText && bPairedList ){
                                if( pResData->isMerged( sCur ) ) break;
                                const ByteString sPlist("pairedlist");
                                ByteString sKey = MergeDataFile::CreateKey( sPlist , pResData->sGId , pResData->sId , sFilename );
                                bText = pResData->getFallbackData( sKey , sText );
                            }else if ( !bText ){// new fallback
                                if( pResData->isMerged( sCur ) ) break;
                                const ByteString sPlist("list");
                                ByteString sKey = MergeDataFile::CreateKey( sPlist , pResData->sGId , pResData->sId , sFilename );
                                bText = pResData->getFallbackData( sKey , sText );
                            } // new fallback

                            if ( bText && sText.Len()) {
                                //if( pEntrys ) pEntrys->Dump();
                                if ( nIdx == 1 ) {
                                    ByteString sHead;
                                    if ( bNextMustBeDefineEOL )
                                        sHead = "\\\n\t";
                                    sHead += sSpace;
                                    switch ( nT ) {
                                        case LIST_STRING : sHead += "StringList "; break;
                                        case LIST_FILTER : sHead += "FilterList "; break;
                                        case LIST_ITEM : sHead += "ItemList "; break;
                                        case LIST_PAIRED : sHead += "PairedList "; break;
                                        case LIST_UIENTRIES : sHead += "UIEntries "; break;
                                    }
                                      sHead += "[ ";
                                    sHead += sCur;
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
                                    sLine = ( *pList->GetObject( nLIndex ))[ SOURCE_LANGUAGE ];
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

                                sal_uInt16 nStart, nEnd;
                                nStart = sLine.Search( "\"" );

                                ByteString sPostFix;
                                if( !bPairedList ){
                                    nEnd = sLine.SearchBackward( '\"' );
                                    sPostFix = ByteString( sLine.Copy( ++nEnd ));
                                    sLine.Erase( nStart );
                                }


                                ConvertMergeContent( sText );

                                // merge new res. in text line
                                if( bPairedList ){
                                    sLine = MergePairedList( sLine , sText );
                                }
                                else{
                                    sLine += sText;
                                    sLine += sPostFix;
                                }

                                ByteString sText1( "\t" );
                                sText1 += sLine;
                                if ( bDefine || bNextMustBeDefineEOL )
                                    sText1 += " ;\\\n";
                                else
                                    sText1 += " ;\n";
                                sText1 += sSpace;
                                sText1 += "\t";
                                //printf("Writing '%s'\n",sText1.GetBuffer());
                                WriteToMerged( sText1 ,true );

                                // Set matching pairedlist identifier
                                if ( bPairedList ){
                                    nIdx++;
                                    ExportListEntry* pListE = ( ExportListEntry* ) pResData->pPairedList->GetObject( ( nIdx ) -1 );
                                    if( pListE ){
                                        pResData->sId = GetPairedListID ( (*pListE)[ SOURCE_LANGUAGE ] );
                                    }
                                }
                                else
                                    pResData->sId = ByteString::CreateFromInt32( ++nIdx );
                            }
                            else
                                break;
                            nLIndex ++;
                            PFormEntrys *oldEntry = pEntrys;
                            pEntrys = pMergeDataFile->GetPFormEntrys( pResData ); // <--- game over
                            if( !pEntrys )
                                pEntrys = oldEntry;
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
                // PairedList
                case LIST_STRING : pList = pResData->pStringList; break;
                case LIST_FILTER : pList = pResData->pFilterList; break;
                case LIST_UIENTRIES : pList = pResData->pUIEntries; break;
                case LIST_ITEM : pList = pResData->pItemList; break;
                case LIST_PAIRED : pList = pResData->pPairedList; break;

            }

            nListIndex++;
            sal_uLong nMaxIndex = 0;
            if ( pList )
                nMaxIndex = pList->GetSourceLanguageListEntryCount();
            ByteString sLine;
            if ( pList && pList->GetObject( nListIndex ))
                sLine = ( *pList->GetObject( nListIndex ))[ SOURCE_LANGUAGE ];
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
                for ( sal_uInt16 i = 0; i < nLevel; i++ )
                    sText += "\t";
                WriteToMerged( sText ,false );
                nListIndex++;
                if ( pList && pList->GetObject( nListIndex ))
                    sLine = ( *pList->GetObject( nListIndex ))[ SOURCE_LANGUAGE ];
                if ( !sLine.Len())
                    sLine = sLastListLine;
                sLine += " ;";
            }
        }
        break;
    }
    pParseQueue->bMflag = false;
}

ByteString Export::MergePairedList( ByteString& sLine , ByteString& sText ){
// < "xy" ; IDENTIFIER ; >
    ByteString sPre  = sLine.Copy( 0 , sLine.Search('\"') );
    ByteString sPost = sLine.Copy( sLine.SearchBackward('\"') + 1 , sLine.Len() );
    sPre.Append( sText );
    sPre.Append( sPost );
    return sPre;
}

/*****************************************************************************/
void Export::SetChildWithText()
/*****************************************************************************/
{
    if ( aResStack.Count() > 1 ) {
        for ( sal_uLong i = 0; i < aResStack.Count() - 1; i++ ) {
            aResStack.GetObject( i )->bChildWithText = sal_True;
        }
    }
}

void ParserQueue::Push( const QueueEntry& aEntry ){
//    printf("nTyp = %d ",aEntry.nTyp);
    sal_uInt16 nLen = aEntry.sLine.Len();

    if( !bStart ){
        aQueueCur->push( aEntry );
        if( nLen > 1 && aEntry.sLine.GetChar( nLen-1 ) == '\n' )
            bStart = true;
        else if ( aEntry.nTyp != IGNOREDTOKENS ){
            if( nLen > 1 && ( aEntry.sLine.GetChar( nLen-1 ) == '\\') ){
                // Next is Macro
                bCurrentIsM = true;
             }else{
                // Next is no Macro
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
            }
            else{
                // Next is no Macro
                bNextIsM = false;
            }
        }else if( nLen > 2 && aEntry.sLine.GetChar( nLen-1 ) == '\n' ){
            if( aEntry.nTyp != IGNOREDTOKENS ){
                if( nLen > 2 && ( aEntry.sLine.GetChar( nLen-2  ) == '\\') ){
                    // Next is Macro
                    bNextIsM = true;
                }
                else{
                    // Next is no Macro
                    bNextIsM = false;
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
        :
          bCurrentIsM( false ),
          bNextIsM( false ) ,
          bLastWasM( false ),
          bMflag( false ) ,
          aExport( aExportObj ) ,
          bStart( false ) ,
          bStartNext( false )
{
          aQueueNext = new std::queue<QueueEntry>;
          aQueueCur  = new std::queue<QueueEntry>;
}


ParserQueue::~ParserQueue(){
    if( aQueueNext )    delete aQueueNext;
    if( aQueueCur )     delete aQueueCur;
}
