/*************************************************************************
 *
 *  $RCSfile: command.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
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
#include <string.h>
#include <ctype.h>

#include <command.hxx>
#include <globals.hxx>
#include <database.hxx>

#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif

#pragma hdrstop

/*************************************************************************
|*
|*    Syntaxbeschreibung
|*
*************************************************************************/
char* SyntaxStrings[] = {
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
"\t\tCachable*, Volatile",
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
"\t\tPlugComm",
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
void Init( const SvCommand & rCommand )
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
BOOL ReadIdl( SvIdlWorkingBase * pDataBase, const SvCommand & rCommand )
{
    for( USHORT n = 0; n < rCommand.aInFileList.Count(); n++ )
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
                    return FALSE;
                }
            }
            else
            {
                SvTokenStream aTokStm( aStm, aFileName );
                if( !pDataBase->ReadSvIdl( aTokStm, FALSE, rCommand.aPath ) )
                    return FALSE;
            }
        }
        else
            return FALSE;
    }
    return TRUE;
}

/*************************************************************************
|*
|*    SvCommand::SvCommand()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.12.94
|*    Letzte Aenderung  MM 15.12.94
|*
*************************************************************************/
SvCommand::SvCommand()
    : nFlags( 0 )
{
    DirEntry aEntry;
    aPath = aEntry.GetFull(); //Immer im Aktuellen Pfad suchen
}

/*************************************************************************
|*
|*    SvCommand::SvCommand()
|*
|*    Beschreibung
|*
*************************************************************************/
static BOOL ResponseFile( StringList * pList, int argc, char ** argv )
{
    // Programmname
    pList->Insert( new String( String::CreateFromAscii(*argv) ), LIST_APPEND );
    for( int i = 1; i < argc; i++ )
    {
        if( '@' == **(argv +i) )
        { // wenn @, dann Response-Datei
            SvFileStream aStm( String::CreateFromAscii((*(argv +i)) +1), STREAM_STD_READ | STREAM_NOCREATE );
            if( aStm.GetError() != SVSTREAM_OK )
                return FALSE;

            ByteString aStr;
            while( aStm.ReadLine( aStr ) )
            {
                USHORT n = 0;
                USHORT nPos = 1;
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
    return TRUE;
}

/*************************************************************************
|*    SvCommand::SvCommand()
|*
|*    Beschreibung
*************************************************************************/
SvCommand::SvCommand( int argc, char ** argv )
    : nFlags( 0 )
{
    StringList aList;

    if( ResponseFile( &aList, argc, argv ) )
    for( ULONG i = 1; i < aList.Count(); i++ )
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
                    printf( "unknown switch: %s\n", aParam.GetBuffer() );
                    exit( -1 );
                }
            }
            else if( aParam.EqualsIgnoreCaseAscii( "help" ) || aParam.EqualsIgnoreCaseAscii( "?" ) )
            { // Hilfe
                printf( CommandLineSyntax );
            }
            else if( aParam.EqualsIgnoreCaseAscii( "syntax" ) )
            { // Hilfe
                int i = 0;
                while(SyntaxStrings[i])
                    printf("%s\n",SyntaxStrings[i++]);
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
#if SUPD>589
                // temporary compatibility hack
                printf( "unknown switch: %s\n", aParam.GetBuffer() );
                exit( -1 );
#endif
            }
        }
        else
        {
            aInFileList.Insert( new String( aParam ), LIST_APPEND );
        }
    }
    else
    {
                printf( CommandLineSyntax );
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

