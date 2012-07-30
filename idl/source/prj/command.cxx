/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <command.hxx>
#include <globals.hxx>
#include <database.hxx>
#include <tools/fsys.hxx>

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
"-fr<resource file>         -fm<makefile target file>\n"
"-fC<c++ source file>       -fH<c++ header file>\n"
"-fc<c source file>         -fh<c header file>\n"
"-rsc <*.srs header line>\n"
"-help, ?                   @<file> response file\n"
" <filenames>\n";

void Init()
{
    if( !IDLAPP->pHashTable )
        IDLAPP->pHashTable      = new SvStringHashTable( 2801 );
    if( !IDLAPP->pGlobalNames )
        IDLAPP->pGlobalNames    = new SvGlobalHashNames();
}

void DeInit()
{
    delete IDLAPP;
}

sal_Bool ReadIdl( SvIdlWorkingBase * pDataBase, const SvCommand & rCommand )
{
    for( size_t n = 0; n < rCommand.aInFileList.size(); ++n )
    {
        String aFileName ( *rCommand.aInFileList[ n ] );
        pDataBase->AddDepFile(aFileName);
        SvFileStream aStm( aFileName, STREAM_STD_READ | STREAM_NOCREATE );
        if( aStm.GetError() == SVSTREAM_OK )
        {
            if( pDataBase->IsBinaryFormat( aStm ) )
            {
                pDataBase->Load( aStm );
                if( aStm.GetError() != SVSTREAM_OK )
                {
                    rtl::OStringBuffer aStr;
                    if( aStm.GetError() == SVSTREAM_FILEFORMAT_ERROR )
                        aStr.append("error: incompatible format, file ");
                    else if( aStm.GetError() == SVSTREAM_WRONGVERSION )
                        aStr.append("error: wrong version, file ");
                    else
                        aStr.append("error during load, file ");
                    aStr.append(rtl::OUStringToOString(aFileName,
                        RTL_TEXTENCODING_UTF8));
                    fprintf( stderr, "%s\n", aStr.getStr() );
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
            const rtl::OString aStr(rtl::OUStringToOString(aFileName,
                RTL_TEXTENCODING_UTF8));
            fprintf( stderr, "unable to read input file: %s\n", aStr.getStr() );
            return sal_False;
        }
    }
    return sal_True;
}

static sal_Bool ResponseFile( StringList * pList, int argc, char ** argv )
{
    // program name
    pList->push_back( new String(rtl::OUString::createFromAscii(*argv) ) );
    for( int i = 1; i < argc; i++ )
    {
        if( '@' == **(argv +i) )
        { // when @, then response file
            SvFileStream aStm( rtl::OUString::createFromAscii((*(argv +i)) +1), STREAM_STD_READ | STREAM_NOCREATE );
            if( aStm.GetError() != SVSTREAM_OK )
                return sal_False;

            rtl::OString aStr;
            while( aStm.ReadLine( aStr ) )
            {
                sal_uInt16 n = 0;
                sal_uInt16 nPos = 1;
                while( n != nPos )
                {
                    while( aStr[n] && isspace( aStr[n] ) )
                        n++;
                    nPos = n;
                    while( aStr[n] && !isspace( aStr[n] ) )
                        n++;
                    if( n != nPos )
                        pList->push_back( new String( rtl::OStringToOUString(aStr.copy(nPos, n - nPos), RTL_TEXTENCODING_ASCII_US) ) );
                }
            }
        }
        else if( argv[ i ] )
            pList->push_back( new String( rtl::OUString::createFromAscii( argv[ i ] ) ) );
    }
    return sal_True;
}

SvCommand::SvCommand( int argc, char ** argv )
    : nVerbosity(1), nFlags( 0 )
{
    StringList aList;

    if( ResponseFile( &aList, argc, argv ) )
    for( size_t i = 1; i < aList.size(); i++ )
    {
        String aParam( *aList[ i ] );
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
                { // name of slot output
                    aSlotMapFile = aName;
                }
                else if( 'l' == aFirstChar )
                { // name of listing
                    aListFile = aName;
                }
                else if( 'i' == aFirstChar )
                {
                }
                else if( 'o' == aFirstChar )
                {
                }
                else if( 'd' == aFirstChar )
                { // name of data set file
                    aDataBaseFile = aName;
                }
                else if( 'D' == aFirstChar )
                {
                }
                else if( 'C' == aFirstChar )
                {
                }
                else if( 'H' == aFirstChar )
                {
                }
                else if( 'c' == aFirstChar )
                {
                }
                else if( 'h' == aFirstChar )
                {
                }
                else if( 't' == aFirstChar )
                {
                }
                else if( 'm' == aFirstChar )
                { // name of info file
                    aTargetFile = aName;
                }
                else if( 'r' == aFirstChar )
                {
                }
                else if( 'z' == aFirstChar )
                { // name of HelpId file
                    aHelpIdFile = aName;
                }
                else if( 'y' == aFirstChar )
                { // name of CSV file
                    aCSVFile = aName;
                }
                else if( 'x' == aFirstChar )
                { // name of IDL file for the CSV file
                    aExportFile = aName;
                }
                else if( 'M' == aFirstChar )
                {
                    m_DepFile = aName;
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
            { // help
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
            { // help
                int j = 0;
                while(SyntaxStrings[j])
                    printf("%s\n",SyntaxStrings[j++]);
            }
            else if( aParam.EqualsIgnoreCaseAscii( "i", 0, 1 ) )
            { // define include paths
                String aName( aParam.Copy( 1 ) );
                if( aPath.Len() )
                    aPath += DirEntry::GetSearchDelimiter();
                aPath += aName;
            }
            else if( aParam.EqualsIgnoreCaseAscii( "rsc", 0, 3 ) )
            { // first line in *.srs file
                OSL_ENSURE(false, "does anything use this option, doesn't look like it belong here");
                if( aList[ i + 1 ] )
                {
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
            aInFileList.push_back( new String( aParam ) );
        }
    }
    else
    {
        printf( "%s", CommandLineSyntax );
    }

    for ( size_t i = 0, n = aList.size(); i < n; ++i )
        delete aList[ i ];
    aList.clear();

    rtl::OString aInc(getenv("INCLUDE"));
    // append include environment variable
    if( aInc.getLength() )
    {
        if( aPath.Len() )
            aPath += DirEntry::GetSearchDelimiter();
        aPath += rtl::OStringToOUString(aInc, RTL_TEXTENCODING_ASCII_US);
    }
}

SvCommand::~SvCommand()
{
    // release String list
    for ( size_t i = 0, n = aInFileList.size(); i < n; ++i )
        delete aInFileList[ i ];
    aInFileList.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
