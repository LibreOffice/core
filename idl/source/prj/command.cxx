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
#include "precompiled_idl.hxx"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <command.hxx>
#include <globals.hxx>
#include <database.hxx>
#include <tools/fsys.hxx>

/*************************************************************************
|*
|*    Syntaxbeschreibung
|*
*************************************************************************/
char const * SyntaxStrings[] = {
"basic-type:",
"\tvoid|        char|       int|        float|      double|",
"\tUINT16|      INT16|      UINT32|     INT32|      BOOL|",
"\tBYTE|        String|     SbxObject",
"",
"{ import \"filename\" }\n",
"module definition:",
"module",
"\tunique id range (ask MM)",
"modul-name",
"'['",
"\tSlotIdFile( \"filename\" )",
"']'",
"'{'",
"\t{ include \"filename\" }\n",

"\titem definition:",
"\titem type item-name;\n",

"\ttype definition:",
"\tstruct | union identifier",
"\t'{'",
"\t\t{ type idetifier }",
"\t'}'",
"\t|",
"\tenum identifier",
"\t'{'",
"\t\t{ identifier, }",
"\t'}'",
"\t|",
"\ttypedef type identifier\n",

"\titem-method:",
"\titem identifier item-method-args\n",

"\titem-method-args:",
"\t( { item parameter-name SLOT_ID } )\n",

"\tslot definition:",
"\titem identifier SLOT_ID [ item-method-args ]",
"\t'['\n",

"\t\titem-method-args",
"\t\tAccelConfig, MenuConfig, StatusBarConfig, ToolbarConfig",
"\t\tAutomation*",
"\t\tAutoUpdate",
// "\t\tCachable*, Volatile",
"\t\tContainer",
"\t\tDefault        = Identifier",
"\t\tExecMethod     = Identifier",
"\t\tExport*",
"\t\tFastCall",
"\t\tGet, Set",
"\t\tGroupId        = Identifier",
"\t\tHasCoreId",
"\t\tHasDialog",
"\t\tIsCollection",
"\t\tImageRotation",
"\t\tImageReflection",
"\t\tPseudoPrefix   = Identifier",
"\t\tPseudoSlots",
"\t\tReadOnly",
"\t\tReadOnlyDoc*",
"\t\tRecordPerSet*, RecordPerItem, RecordManual, NoRecord",
"\t\tRecordAbsolute",
"\t\tStateMethod    = Identifier",
"\t\tSynchron*, Asynchron",
"\t\tToggle",
"\t']'\n",

"\tinterface definition:",
"\tshell | interface identifier ':' interface",
"\t'{'",
"\t\t{ slot }",
"\t'}'\n",
"---syntax example is sfx.idl---\n",
NULL };

char CommandLineSyntax[] =
"-fs<slotmap file>          -fl<listing file>\n"
"-fo<odl file>              -fd<data base file>\n"
"-fi<item implementation>   -ft<type library file> (not OLE)\n"
"-fr<ressource file>        -fm<makefile target file>\n"
"-fC<c++ source file>       -fH<c++ header file>\n"
"-fc<c source file>         -fh<c header file>\n"
"-rsc <*.srs header line>\n"
"-help, ?                   @<file> response file\n"
" <filenames>\n";

/*************************************************************************
|*
|*    Init()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.12.94
|*    Letzte Aenderung  MM 15.12.94
|*
*************************************************************************/
void Init()
{
    if( !IDLAPP->pHashTable )
        IDLAPP->pHashTable      = new SvStringHashTable( 2801 );
    if( !IDLAPP->pGlobalNames )
        IDLAPP->pGlobalNames    = new SvGlobalHashNames();
}

/*************************************************************************
|*
|*    DeInit()
|*
|*    Beschreibung
|*
*************************************************************************/
void DeInit()
{
    delete IDLAPP;
}

/*************************************************************************
|*
|*    DeInit()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool ReadIdl( SvIdlWorkingBase * pDataBase, const SvCommand & rCommand )
{
    for( sal_uInt16 n = 0; n < rCommand.aInFileList.Count(); n++ )
    {
        String aFileName ( *rCommand.aInFileList.GetObject( n ) );
        SvFileStream aStm( aFileName, STREAM_STD_READ | STREAM_NOCREATE );
        if( aStm.GetError() == SVSTREAM_OK )
        {
            if( pDataBase->IsBinaryFormat( aStm ) )
            {
                pDataBase->Load( aStm );
                if( aStm.GetError() != SVSTREAM_OK )
                {
                    ByteString aStr;
                    if( aStm.GetError() == SVSTREAM_FILEFORMAT_ERROR )
                        aStr = "error: incompatible format, file ";
                    else if( aStm.GetError() == SVSTREAM_WRONGVERSION )
                        aStr = "error: wrong version, file ";
                    else
                        aStr = "error during load, file ";
                    aStr += ByteString( aFileName, RTL_TEXTENCODING_UTF8 );
                    fprintf( stderr, "%s\n", aStr.GetBuffer() );
                    return sal_False;
                }
            }
            else
            {
                SvTokenStream aTokStm( aStm, aFileName );
                if( !pDataBase->ReadSvIdl( aTokStm, sal_False, rCommand.aPath ) )
                    return sal_False;
            }
        }
        else
        {
            const ByteString aStr( aFileName, RTL_TEXTENCODING_UTF8 );
            fprintf( stderr, "unable to read input file: %s\n", aStr.GetBuffer() );
            return sal_False;
        }
    }
    return sal_True;
}

/*************************************************************************
|*
|*    SvCommand::SvCommand()
|*
|*    Beschreibung
|*
*************************************************************************/
static sal_Bool ResponseFile( StringList * pList, int argc, char ** argv )
{
    // Programmname
    pList->Insert( new String( String::CreateFromAscii(*argv) ), LIST_APPEND );
    for( int i = 1; i < argc; i++ )
    {
        if( '@' == **(argv +i) )
        { // wenn @, dann Response-Datei
            SvFileStream aStm( String::CreateFromAscii((*(argv +i)) +1), STREAM_STD_READ | STREAM_NOCREATE );
            if( aStm.GetError() != SVSTREAM_OK )
                return sal_False;

            ByteString aStr;
            while( aStm.ReadLine( aStr ) )
            {
                sal_uInt16 n = 0;
                sal_uInt16 nPos = 1;
                while( n != nPos )
                {
                    while( aStr.GetChar(n) && isspace( aStr.GetChar(n) ) )
                        n++;
                    nPos = n;
                    while( aStr.GetChar(n) && !isspace( aStr.GetChar(n) ) )
                        n++;
                    if( n != nPos )
                        pList->Insert( new String( String::CreateFromAscii( aStr.Copy( nPos, n - nPos ).GetBuffer() ) ), LIST_APPEND );
                }
            }
        }
        else if( argv[ i ] )
            pList->Insert( new String( String::CreateFromAscii( argv[ i ] ) ), LIST_APPEND );
    }
    return sal_True;
}

/*************************************************************************
|*    SvCommand::SvCommand()
|*
|*    Beschreibung
*************************************************************************/
SvCommand::SvCommand( int argc, char ** argv )
    : nVerbosity(1), nFlags( 0 )
{
    StringList aList;

    if( ResponseFile( &aList, argc, argv ) )
    for( sal_uLong i = 1; i < aList.Count(); i++ )
    {
        String aParam( *aList.GetObject( i ) );
        sal_Unicode aFirstChar( aParam.GetChar(0) );
        if( '-' == aFirstChar )
        {
            aParam.Erase( 0, 1 );
            aFirstChar = aParam.GetChar(0);
            if( aFirstChar == 'F' || aFirstChar == 'f' )
            {
                aParam.Erase( 0, 1 );
                aFirstChar = aParam.GetChar(0);
                String aName( aParam.Copy( 1 ) );
                if( 's' == aFirstChar )
                { // Name der Slot-Ausgabe
                    aSlotMapFile = aName;
                }
                else if( 'l' == aFirstChar )
                { // Name der Listing
                    aListFile = aName;
                }
                else if( 'i' == aFirstChar )
                { // Name der Item-Datei
//                    aSfxItemFile = aName;
                }
                else if( 'o' == aFirstChar )
                { // Name der ODL-Datei
//                    aODLFile = aName;
                }
                else if( 'd' == aFirstChar )
                { // Name der Datenbasis-Datei
                    aDataBaseFile = aName;
                }
                else if( 'D' == aFirstChar )
                { // Name der Docu-Datei f"ur das API
//                    aDocuFile = aName;
                }
                else if( 'C' == aFirstChar )
                { // Name der cxx-Datei
//                    aCxxFile = aName;
                }
                else if( 'H' == aFirstChar )
                { // Name der hxx-Datei
//                    aHxxFile = aName;
                }
                else if( 'c' == aFirstChar )
                { // Name der C-Header-Datei
//                    aCSourceFile = aName;
                }
                else if( 'h' == aFirstChar )
                { // Name der C-Header-Datei
//                    aCHeaderFile = aName;
                }
                else if( 't' == aFirstChar )
                { // Name der Info-Datei
//                    aCallingFile = aName;
                }
                else if( 'm' == aFirstChar )
                { // Name der Info-Datei
                    aTargetFile = aName;
                }
                else if( 'r' == aFirstChar )
                { // Name der Resource-Datei
//                    aSrcFile = aName;
                }
                else if( 'z' == aFirstChar )
                { // Name der HelpId-Datei
                    aHelpIdFile = aName;
                }
                else if( 'y' == aFirstChar )
                { // Name der CSV-Datei
                    aCSVFile = aName;
                }
                else if( 'x' == aFirstChar )
                { // Name der IDL-Datei fuer die CSV-Datei
                    aExportFile = aName;
                }
                else
                {
                    printf(
                        "unknown switch: %s\n",
                        rtl::OUStringToOString(
                            aParam, RTL_TEXTENCODING_UTF8).getStr());
                    exit( -1 );
                }
            }
            else if( aParam.EqualsIgnoreCaseAscii( "help" ) || aParam.EqualsIgnoreCaseAscii( "?" ) )
            { // Hilfe
                printf( "%s", CommandLineSyntax );
            }
            else if( aParam.EqualsIgnoreCaseAscii( "quiet" ) )
            {
                nVerbosity = 0;
            }
            else if( aParam.EqualsIgnoreCaseAscii( "verbose" ) )
            {
                nVerbosity = 2;
            }
            else if( aParam.EqualsIgnoreCaseAscii( "syntax" ) )
            { // Hilfe
                int j = 0;
                while(SyntaxStrings[j])
                    printf("%s\n",SyntaxStrings[j++]);
            }
            else if( aParam.EqualsIgnoreCaseAscii( "i", 0, 1 ) )
            { // Include-Pfade definieren
                String aName( aParam.Copy( 1 ) );
                if( aPath.Len() )
                    aPath += DirEntry::GetSearchDelimiter();
                aPath += aName;
            }
            else if( aParam.EqualsIgnoreCaseAscii( "rsc", 0, 3 ) )
            { // erste Zeile im *.srs File
                if( aList.GetObject( i +1 ) )
                {
                    aSrsLine = ByteString( *aList.GetObject( i +1 ), RTL_TEXTENCODING_UTF8 );
                    i++;
                }
            }
            else
            {
                // temporary compatibility hack
                printf(
                    "unknown switch: %s\n",
                    rtl::OUStringToOString(
                        aParam, RTL_TEXTENCODING_UTF8).getStr());
                exit( -1 );
            }
        }
        else
        {
            aInFileList.Insert( new String( aParam ), LIST_APPEND );
        }
    }
    else
    {
        printf( "%s", CommandLineSyntax );
    }

    String * pStr = aList.First();
    while( pStr )
    {
        delete pStr;
        pStr = aList.Next();
    }

    ByteString aInc( getenv( "INCLUDE" ) );
    // Include Environmentvariable anhaengen
    if( aInc.Len() )
    {
        if( aPath.Len() )
            aPath += DirEntry::GetSearchDelimiter();
        aPath += String::CreateFromAscii( aInc.GetBuffer() );
    }
}

/*************************************************************************
|*
|*    SvCommand::~SvCommand()
|*
|*    Beschreibung
|*
*************************************************************************/
SvCommand::~SvCommand()
{
    // ByteString Liste freigeben
    String * pStr;
    while( NULL != (pStr = aInFileList.Remove()) )
        delete pStr;
}

