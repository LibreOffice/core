/*************************************************************************
 *
 *  $RCSfile: xrmmerge.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:26:15 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
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
#include <tools/string.hxx>
#include <tools/fsys.hxx>

// local includes
#include "export.hxx"
#include "xrmmerge.hxx"
#include "utf8conv.hxx"
#include "tokens.h"

extern "C" { int yyerror( char * ); }
extern "C" { int YYWarning( char * ); }

// defines to parse command line
#define STATE_NON       0x0001
#define STATE_INPUT     0x0002
#define STATE_OUTPUT    0x0003
#define STATE_PRJ       0x0004
#define STATE_ROOT      0x0005
#define STATE_MERGESRC  0x0006
#define STATE_ERRORLOG  0x0007
#define STATE_UTF8      0x000B
#define STATE_LANGUAGES 0x000C
#define STATE_ISOCODE99 0x000D

// set of global variables
BOOL bEnableExport;
BOOL bMergeMode;
BOOL bErrorLog;
BOOL bUTF8;
bool bQuiet;
ByteString sPrj;
ByteString sPrjRoot;
ByteString sInputFileName;
ByteString sActFileName;
ByteString sOutputFile;
ByteString sMergeSrc;
XRMResParser *pParser = NULL;

extern "C" {
// the whole interface to lexer is in this extern "C" section

/*****************************************************************************/
extern char *GetOutputFile( int argc, char* argv[])
/*****************************************************************************/
{
    bEnableExport = FALSE;
    bMergeMode = FALSE;
    bErrorLog = TRUE;
    bUTF8 = TRUE;
    sPrj = "";
    sPrjRoot = "";
    sInputFileName = "";
    sActFileName = "";
    Export::sLanguages = "";
    bQuiet = false;
    USHORT nState = STATE_NON;
    BOOL bInput = FALSE;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        if ( ByteString( argv[ i ] ).ToUpperAscii() == "-I" ) {
            nState = STATE_INPUT; // next token specifies source file
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-O" ) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-P" ) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-R" ) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-M" ) {
            nState = STATE_MERGESRC; // next token specifies the merge database
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-QQ" ) {
            bQuiet = true;
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-E" ) {
            nState = STATE_ERRORLOG;
            bErrorLog = FALSE;
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-UTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = TRUE;
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-NOUTF8" ) {
            nState = STATE_UTF8;
            bUTF8 = FALSE;
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-L" ) {
            nState = STATE_LANGUAGES;
        }
        else if ( ByteString( argv[ i ] ).ToUpperAscii() == "-ISO99" ) {
            nState = STATE_ISOCODE99;
        }
        else {
            switch ( nState ) {
                case STATE_NON: {
                    return NULL;    // no valid command line
                }
                break;
                case STATE_INPUT: {
                    sInputFileName = argv[ i ];
                    bInput = TRUE; // source file found
                }
                break;
                case STATE_OUTPUT: {
                    sOutputFile = argv[ i ]; // the dest. file
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
                case STATE_ISOCODE99: {
                    Export::sIsoCode99 = ByteString( argv[ i ]);
                }
                break;
            }
        }
    }

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
int InitXrmExport( char *pOutput )
/*****************************************************************************/
{
    // instanciate Export
    ByteString sOutput( pOutput );
    Export::InitLanguages( false );

    if ( bMergeMode )
        pParser = new XRMResMerge( sMergeSrc, sOutputFile, bErrorLog );
      else if ( sOutputFile.Len()) {
        pParser = new XRMResExport( sOutputFile, sPrj, sActFileName );
    }
    else
        pParser = new XRMResParser();

    return 1;
}

int isQuiet(){
    if( bQuiet )    return 1;
    else            return 0;
}
/*****************************************************************************/
int EndXrmExport()
/*****************************************************************************/
{
    delete pParser;
    return 1;
}

/*****************************************************************************/
extern FILE *GetXrmFile()
/*****************************************************************************/
{
    // look for valid filename
    if ( sInputFileName.Len()) {

        // able to open file?
        FILE *pFile = fopen( sInputFileName.GetBuffer(), "r" );
        if ( !pFile )
            fprintf( stderr, "Error: Could not open file %s\n",
                sInputFileName.GetBuffer());
        else {
            // this is a valid file which can be opened, so
            // create path to project root
            DirEntry aEntry( String( sInputFileName, RTL_TEXTENCODING_ASCII_US ));
            aEntry.ToAbs();
            ByteString sFullEntry( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
            aEntry += DirEntry( String( "..", RTL_TEXTENCODING_ASCII_US ));
            aEntry += DirEntry( sPrjRoot );
            ByteString sPrjEntry( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );

            // create file name, beginnig with project root
            // (e.g.: source\ui\src\menue.src)
            sActFileName = sFullEntry.Copy( sPrjEntry.Len() + 1 );
//          sActFileName.ToLowerAscii();

            if( !bQuiet )
                fprintf( stdout, "\nProcessing File %s ...\n", sInputFileName.GetBuffer());

            sActFileName.SearchAndReplaceAll( "/", "\\" );

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
    pParser->Execute( nTyp, pTokenText );

    return 1;
}

/*****************************************************************************/
int SetError()
/*****************************************************************************/
{
    pParser->SetError();
    return 1;
}
}

extern "C" {
/*****************************************************************************/
int GetError()
/*****************************************************************************/
{
    return pParser->GetError();
}
}

//
// class XRMResParser
//


/*****************************************************************************/
XRMResParser::XRMResParser()
/*****************************************************************************/
                : bError( FALSE ),
                bText( FALSE )
{
    aLanguages = Export::GetLanguages();
}

/*****************************************************************************/
XRMResParser::~XRMResParser()
/*****************************************************************************/
{
}

/*****************************************************************************/
int XRMResParser::Execute( int nToken, char * pToken )
/*****************************************************************************/
{
    ByteString rToken( pToken );

    switch ( nToken ) {
        case XRM_README_START:
            sLID = "";
            sGID = GetAttribute( rToken, "name" );
        break;

        case XRM_README_END:
            sGID = "";
        break;

        case XRM_SECTION_START:
            sLID = "";
            sGID += ".";
            sGID += GetAttribute( rToken, "id" );
            sLocalized = "X:";
        break;

        case XRM_SECTION_END:
            sGID = sGID.GetToken( 0, '.' );
            break;

        case XRM_PARAGRAPH_START:
            sLID = "";
            sGID += ".";
            sGID += GetAttribute( rToken, "id" );
            if ( GetAttribute( rToken, "localized" ) == "false" )
                sLocalized += "0";
            else
                sLocalized += "1";
        break;

        case XRM_PARAGRAPH_END: {
            if ( sLID.Len())
                EndOfText( sCurrentOpenTag, sCurrentCloseTag );
            ByteString sTmp = sGID;
            sGID = "";
            for ( USHORT i = 0; i + 1 < sTmp.GetTokenCount( '.' ); i++ ) {
                if ( sGID.Len())
                    sGID += ".";
                sGID += sTmp.GetToken( i, '.' );
            }
            sLocalized = sLocalized.Copy( 0, sLocalized.Len() - 1 );
           }
        break;

        case XRM_TEXT_START:
            if ( sLocalized.GetChar( sLocalized.Len() - 1 ) == '1' ) {
                ByteString sNewLID = GetAttribute( rToken, "id" );
                if ( sNewLID != sLID ) {
                    EndOfText( sCurrentOpenTag, sCurrentCloseTag );
                    sLID = sNewLID;
                }
                bText = TRUE;
                sCurrentText = "";
                sCurrentOpenTag = rToken;
                Output( rToken );
            }
        break;

        case XRM_TEXT_END: {
            if ( sLocalized.GetChar( sLocalized.Len() - 1 ) == '1' ) {
                sCurrentCloseTag = rToken;

                ByteString sLang = GetAttribute( sCurrentOpenTag, "xml:lang" );
                if( sLang.EqualsIgnoreCaseAscii("de") ){
                //if ( Export::GetLangByIsoLang( sLang ) != GERMAN ) {
                     ULONG nLen = 0;
                    while ( sCurrentText.Len() != nLen )
                    {
                        nLen = sCurrentText.Len();
                        sCurrentText.SearchAndReplaceAll( "\n\t", "\n" );
                        sCurrentText.SearchAndReplaceAll( "\n ", "\n" );
                    }
                    sCurrentText.SearchAndReplaceAll( "\n", " " );
                    sCurrentCloseTag = rToken;
                }
                WorkOnText( sCurrentOpenTag, sCurrentText );
                Output( sCurrentText );

//              fprintf( stdout, "%s %s\n", sGID.GetBuffer(), sLID.GetBuffer());
//              fprintf( stdout, "%s\n\n", sCurrentText.GetBuffer());

                bText = FALSE;
            }
        }
        break;

        case XRM_LIST_START:
            sLID = "";
        break;

        case XRM_LIST_END:
            if ( sLID.Len())
                EndOfText( sCurrentOpenTag, sCurrentCloseTag );
        break;

        default:
            if ( bText ) {
                sCurrentText += rToken;
            }
        break;
    }

    if ( !bText )
        Output( rToken );

    return 0;
}

/*****************************************************************************/
ByteString XRMResParser::GetAttribute( const ByteString &rToken, const ByteString &rAttribute )
/*****************************************************************************/
{
    ByteString sTmp( rToken );
    sTmp.SearchAndReplaceAll( "\t", " " );

    ByteString sSearch( " " );
    sSearch += rAttribute;
    sSearch += "=";
    int nPos = sTmp.Search( sSearch );

    if ( nPos != STRING_NOTFOUND ) {
        sTmp = sTmp.Copy( nPos );
        ByteString sId = sTmp.GetToken( 1, '\"' );
        return sId;
    }
    return "";
}

/*****************************************************************************/
void XRMResParser::Output( const ByteString& rOutput )
/*****************************************************************************/
{
}

/*****************************************************************************/
void XRMResParser::WorkOnText(
    const ByteString &rOpenTag,
    ByteString &rText
)
/*****************************************************************************/
{
}

/*****************************************************************************/
void XRMResParser::EndOfText(
    const ByteString &rOpenTag,
    const ByteString &rCloseTag
)
/*****************************************************************************/
{
}

/*****************************************************************************/
void XRMResParser::Error( const ByteString &rError )
/*****************************************************************************/
{
    yyerror(( char * ) rError.GetBuffer());
}

/*****************************************************************************/
void XRMResParser::ConvertStringToDBFormat( ByteString &rString )
/*****************************************************************************/
{
    ByteString sResult;
    do {
        sResult = rString;
        rString.EraseLeadingChars( _LF );
    //  rString.EraseLeadingChars( ' ' );
        rString.EraseLeadingChars( '\t' );
    //  rString.EraseTrailingChars( ' ' );
        rString.EraseTrailingChars( '\t' );
    } while ( sResult != rString );

    rString.SearchAndReplaceAll( "\t", "\\t" );
}

/*****************************************************************************/
void XRMResParser::ConvertStringToXMLFormat( ByteString &rString )
/*****************************************************************************/
{
    rString.SearchAndReplaceAll( "\\t", "\t" );
}



//
// class XRMResOutputParser
//

/*****************************************************************************/
XRMResOutputParser::XRMResOutputParser ( const ByteString &rOutputFile )
/*****************************************************************************/
{
    aLanguages = Export::GetLanguages();
    pOutputStream =
        new SvFileStream(
            String( rOutputFile, RTL_TEXTENCODING_ASCII_US ),
            STREAM_STD_WRITE | STREAM_TRUNC
        );
    pOutputStream->SetStreamCharSet( RTL_TEXTENCODING_UTF8 );
    if ( !pOutputStream->IsOpen()) {
        ByteString sError( "Unable to open output file: " );
        sError += rOutputFile;
        Error( sError );
        delete pOutputStream;
        pOutputStream = NULL;
    }
}

/*****************************************************************************/
XRMResOutputParser::~XRMResOutputParser()
/*****************************************************************************/
{
    if ( pOutputStream ) {
        pOutputStream->Close();
        delete pOutputStream;
    }
}

//
// class XMLResExport
//

/*****************************************************************************/
XRMResExport::XRMResExport(
    const ByteString &rOutputFile, const ByteString &rProject,
    const ByteString &rFilePath )
/*****************************************************************************/
                : XRMResOutputParser( rOutputFile ),
                pResData( NULL ),
                sPrj( rProject ),
                sPath( rFilePath )
{
    aLanguages = Export::GetLanguages();
}

/*****************************************************************************/
XRMResExport::~XRMResExport()
/*****************************************************************************/
{
    delete pResData;
}


/*****************************************************************************/
void XRMResExport::WorkOnText(
    const ByteString &rOpenTag,
    ByteString &rText
)
/*****************************************************************************/
{
    ByteString sLang( GetAttribute( rOpenTag, "xml:lang" ));
    //USHORT nLang = Export::GetLangByIsoLang( sLang );
    //USHORT nLangIndex = Export::GetLangIndex( nLang );

    //if ( LANGUAGE_ALLOWED( nLangIndex )) {
        if ( !pResData ) {
            ByteString sPlatform( "" );
            pResData = new ResData( sPlatform, GetGID());
            pResData->sId = GetLID();
        }

        //pResData->sText[ nLangIndex ] =
        //  UTF8Converter::ConvertFromUTF8( rText, Export::GetCharSet( nLang ));
        pResData->sText[ sLang ] = rText;
        //ConvertStringToDBFormat( pResData->sText[ nLangIndex ] );
        ConvertStringToDBFormat( pResData->sText[ sLang ] );
    //}
}

/*****************************************************************************/
void XRMResExport::EndOfText(
    const ByteString &rOpenTag,
    const ByteString &rCloseTag
)
/*****************************************************************************/
{
    if ( pResData && pOutputStream ) {

        char cSearch = 0x00;
        ByteString sSearch( cSearch );

/*      if (( !pResData->sText[ ENGLISH_INDEX ].Len()) &&
            ( !pResData->sText[ ENGLISH_US_INDEX ].Len()))*/
         if ( !pResData->sText[ ByteString("en-US") ].Len() )
            pResData->sText[ ByteString("en-US") ] = pResData->sText[ ByteString("de") ];

        Export::FillInFallbacks( pResData );

        ByteString sTimeStamp( Export::GetTimeStamp());
        ByteString sCur;
        for( long int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];

        //for ( ULONG i = 0; i < LANGUAGES; i++ ) {
        //  if ( LANGUAGE_ALLOWED( i )) {
                //ByteString sAct = pResData->sText[ i ];
            ByteString sAct = pResData->sText[ sCur ];
/*              if ( !sAct.Len() && i ) {
                    if ( pResData->sText[ ENGLISH_US_INDEX ].Len())
                        sAct = pResData->sText[ ENGLISH_US_INDEX ];
                    else
                        sAct = pResData->sText[ ENGLISH_INDEX ];
                }*/

                Export::UnquotHTML( sAct );
                sAct.EraseAllChars( 0x0A );

                ByteString sOutput( sPrj ); sOutput += "\t";
                sOutput += sPath;
                sOutput += "\t0\t";
                sOutput += "readmeitem\t";
                sOutput += pResData->sGId; sOutput += "\t";
                sOutput += pResData->sId; sOutput += "\t\t\t0\t";
                //sOutput += ByteString::CreateFromInt64( Export::LangId[ i ] );
                sOutput += sCur;
                sOutput += "\t";

//                if( sCur.EqualsIgnoreCaseAscii("de") ){
//                   sAct = UTF8Converter::ConvertToUTF8( sAct, RTL_TEXTENCODING_MS_1252 );
//                }

                sOutput += sAct; sOutput += "\t\t\t\t";
                sOutput += sTimeStamp;

//              if ( bUTF8 )
//                  sOutput = UTF8Converter::ConvertToUTF8( sOutput, Export::GetCharSet( Export::LangId[ i ] ));

                sOutput.SearchAndReplaceAll( sSearch, "_" );
                //if( sCur.EqualsIgnoreCaseAscii("de") ){
               //    sOutput = UTF8Converter::ConvertToUTF8( sOutput , RTL_TEXTENCODING_MS_1252 );
                //}

                pOutputStream->WriteLine( sOutput );
            }
        //}
    }
    delete pResData;
    pResData = NULL;
}

//
// class XRMResMerge
//

/*****************************************************************************/
XRMResMerge::XRMResMerge(
    const ByteString &rMergeSource, const ByteString &rOutputFile,
    BOOL bErrorLog )
/*****************************************************************************/
                : XRMResOutputParser( rOutputFile ),
                pMergeDataFile( NULL ),
                pResData( NULL )
{
    if ( rMergeSource.Len())
        pMergeDataFile = new MergeDataFile(
            rMergeSource, sInputFileName , bErrorLog, RTL_TEXTENCODING_MS_1252, bUTF8 );
    if( Export::sLanguages.EqualsIgnoreCaseAscii("ALL") ){
        Export::SetLanguages( pMergeDataFile->GetLanguages() );
        aLanguages = pMergeDataFile->GetLanguages();
    }
    else aLanguages = Export::GetLanguages();
}

/*****************************************************************************/
XRMResMerge::~XRMResMerge()
/*****************************************************************************/
{
    delete pMergeDataFile;
    delete pResData;
}

/*****************************************************************************/
void XRMResMerge::WorkOnText(
    const ByteString &rOpenTag,
    ByteString &rText
)
/*****************************************************************************/
{
    ByteString sLang( GetAttribute( rOpenTag, "xml:lang" ));
    //USHORT nLang = Export::GetLangByIsoLang( sLang );
    //USHORT nLangIndex = Export::GetLangIndex( nLang );

    if ( pMergeDataFile ) {
        if ( !pResData ) {
            ByteString sPlatform( "" );
            pResData = new ResData( sPlatform, GetGID());
            pResData->sId = GetLID();
            pResData->sResTyp = "readmeitem";
        }

    //  if ( LANGUAGE_ALLOWED( nLangIndex )) {
            PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
            if ( pEntrys ) {
                ByteString sContent;
/*              if (( nLang != GERMAN ) &&
                    ( nLang != ENGLISH ) &&*/
                if (( !sLang.EqualsIgnoreCaseAscii("de") ) &&
                    ( !sLang.EqualsIgnoreCaseAscii("en-US") ) &&
                    ( pEntrys->GetText(
                        //sContent, STRING_TYP_TEXT, Export::GetLangIndex( nLang ))) &&
                        sContent, STRING_TYP_TEXT, sLang )) &&
                    ( sContent != "-" ) && ( sContent.Len()))

                {
//                  rText = UTF8Converter::ConvertToUTF8(
//                      sContent, Export::GetCharSet( nLang )) ;
                    rText = sContent;
                    ConvertStringToXMLFormat( rText );
                    Export::QuotHTML( rText );
                }
            }
//      }
    }
}

/*****************************************************************************/
void XRMResMerge::Output( const ByteString& rOutput )
/*****************************************************************************/
{
    if ( pOutputStream )
        pOutputStream->Write( rOutput.GetBuffer(), rOutput.Len());
}

/*****************************************************************************/
void XRMResMerge::EndOfText(
    const ByteString &rOpenTag,
    const ByteString &rCloseTag
)
/*****************************************************************************/
{
    if ( pMergeDataFile && pResData ) {
        PFormEntrys *pEntrys = pMergeDataFile->GetPFormEntrys( pResData );
        if ( pEntrys ) {
            ByteString sCur;
            for( long int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];
//            for ( ULONG nIndex = 0; nIndex < LANGUAGES; nIndex++ ) {
                ByteString sContent;
/*              if (( nIndex != GERMAN_INDEX ) &&
                    ( nIndex != ENGLISH_INDEX ) &&
                    ( LANGUAGE_ALLOWED( nIndex )) && */
                if (( !sCur.EqualsIgnoreCaseAscii("de") &&
                      !sCur.EqualsIgnoreCaseAscii("en-US") )&&
                    ( pEntrys->GetText(
                        //sContent, STRING_TYP_TEXT, nIndex, TRUE )) &&
                        sContent, STRING_TYP_TEXT, sCur, TRUE )) &&
                    ( sContent != "-" ) && ( sContent.Len()))
                {
//                  ByteString sText = UTF8Converter::ConvertToUTF8(
//                      sContent, Export::GetCharSet( Export::LangId[ nIndex ]));
                    ByteString sText( sContent );
                    Export::QuotHTML( sText );

                    ByteString sAdditionalLine( "\t" );
                    sAdditionalLine += rOpenTag;
                    ByteString sSearch = "xml:lang=\"";
                    ByteString sReplace( sSearch );

                    sSearch += GetAttribute( rOpenTag, "xml:lang" );
                    //sReplace += Export::GetIsoLangByIndex( nIndex );
                    sReplace += sCur;

                    sAdditionalLine.SearchAndReplace( sSearch, sReplace );

                    sAdditionalLine += sText;
                    sAdditionalLine += rCloseTag;
                    sAdditionalLine += "\n";

                    for ( USHORT i = 0; i + 1 < GetGID().GetTokenCount( '.' ); i++ )
                        sAdditionalLine += "\t";

                    Output( sAdditionalLine );
                }
            }
        }
    }
    delete pResData;
    pResData = NULL;
}

