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

#include <sal/config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <osl/diagnose.h>

#include <command.hxx>
#include <globals.hxx>
#include <database.hxx>

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
nullptr };

char CommandLineSyntax[] =
"-fs<slotmap file>\n"
"-fm<makefile target file>\n"
"-help, ?                   @<file> response file\n"
" <filenames>\n";

void Init()
{
    if( !GetIdlApp().pHashTable )
        GetIdlApp().pHashTable      = new SvStringHashTable( 2801 );
    if( !GetIdlApp().pGlobalNames )
        GetIdlApp().pGlobalNames    = new SvGlobalHashNames();
}

bool ReadIdl( SvIdlWorkingBase * pDataBase, const SvCommand & rCommand )
{
    for( size_t n = 0; n < rCommand.aInFileList.size(); ++n )
    {
        OUString aFileName ( rCommand.aInFileList[ n ] );
        pDataBase->AddDepFile(aFileName);
        SvFileStream aStm( aFileName, STREAM_STD_READ | StreamMode::NOCREATE );
        if( aStm.GetError() == SVSTREAM_OK )
        {
            SvTokenStream aTokStm( aStm, aFileName );
            if( !pDataBase->ReadSvIdl( aTokStm, false, rCommand.aPath ) )
                return false;
        }
        else
        {
            const OString aStr(OUStringToOString(aFileName,
                RTL_TEXTENCODING_UTF8));
            fprintf( stderr, "unable to read input file: %s\n", aStr.getStr() );
            return false;
        }
    }
    return true;
}

static bool ResponseFile( StringList * pList, int argc, char ** argv )
{
    // program name
    pList->push_back( OUString::createFromAscii(*argv) );
    for( int i = 1; i < argc; i++ )
    {
        if( '@' == **(argv +i) )
        { // when @, then response file
            SvFileStream aStm( OUString::createFromAscii((*(argv +i)) +1), STREAM_STD_READ | StreamMode::NOCREATE );
            if( aStm.GetError() != SVSTREAM_OK )
                return false;

            OString aStr;
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
                        pList->push_back( OStringToOUString(aStr.copy(nPos, n - nPos), RTL_TEXTENCODING_ASCII_US) );
                }
            }
        }
        else if( argv[ i ] )
            pList->push_back( OUString::createFromAscii( argv[ i ] ) );
    }
    return true;
}

SvCommand::SvCommand( int argc, char ** argv )
    : nVerbosity(1)
{
    StringList aList;

    if( ResponseFile( &aList, argc, argv ) )
    {
        for( size_t i = 1; i < aList.size(); i++ )
        {
            OUString aParam( aList[ i ] );
            sal_Unicode aFirstChar( aParam[0] );
            if( '-' == aFirstChar )
            {
                aParam = aParam.copy( 1 );
                aFirstChar = aParam[0];
                if( aFirstChar == 'F' || aFirstChar == 'f' )
                {
                    aParam = aParam.copy( 1 );
                    aFirstChar = aParam[0];
                    OUString aName( aParam.copy( 1 ) );
                    if( 's' == aFirstChar )
                    { // name of slot output
                        aSlotMapFile = aName;
                    }
                    else if( 'm' == aFirstChar )
                    { // name of info file
                        aTargetFile = aName;
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
                            OUStringToOString(
                                aParam, RTL_TEXTENCODING_UTF8).getStr());
                        exit( -1 );
                    }
                }
                else if( aParam.equalsIgnoreAsciiCase( "help" ) || aParam.equalsIgnoreAsciiCase( "?" ) )
                { // help
                    printf( "%s", CommandLineSyntax );
                }
                else if( aParam.equalsIgnoreAsciiCase( "quiet" ) )
                {
                    nVerbosity = 0;
                }
                else if( aParam.equalsIgnoreAsciiCase( "verbose" ) )
                {
                    nVerbosity = 2;
                }
                else if( aParam.equalsIgnoreAsciiCase( "syntax" ) )
                { // help
                    int j = 0;
                    while(SyntaxStrings[j])
                        printf("%s\n",SyntaxStrings[j++]);
                }
                else if (aParam == "isystem")
                {
                    // ignore "-isystem" and following arg
                    if (i < aList.size())
                    {
                        ++i;
                    }
                }
                else if (aParam.startsWith("isystem"))
                {
                    // ignore args starting with "-isystem"
                }
                else if( aParam.startsWithIgnoreAsciiCase( "i" ) )
                { // define include paths
                    OUString aName( aParam.copy( 1 ) );
                    if( !aPath.isEmpty() )
                        aPath += OUStringLiteral1<SAL_PATHSEPARATOR>();
                    aPath += aName;
                }
                else if( aParam.startsWithIgnoreAsciiCase( "rsc" ) )
                { // first line in *.srs file
                    OSL_ENSURE(false, "does anything use this option, doesn't look like it belong here");
                    if( !aList[ i + 1 ].isEmpty() )
                    {
                        i++;
                    }
                }
                else
                {
                    // temporary compatibility hack
                    printf(
                        "unknown switch: %s\n",
                        OUStringToOString(
                            aParam, RTL_TEXTENCODING_UTF8).getStr());
                    exit( -1 );
                }
            }
            else
            {
                aInFileList.push_back( aParam );
            }
        }
    }
    else
    {
        printf( "%s", CommandLineSyntax );
    }

    aList.clear();

    OString aInc(getenv("INCLUDE"));
    // append include environment variable
    if( aInc.getLength() )
    {
        if( !aPath.isEmpty() )
            aPath += OUStringLiteral1<SAL_PATHSEPARATOR>();
        aPath += OStringToOUString(aInc, RTL_TEXTENCODING_ASCII_US);
    }
}

SvCommand::~SvCommand()
{
    // release String list
    aInFileList.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
