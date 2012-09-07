/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <fstream>
#include <string>

#include <stdio.h>

#include <rtl/strbuf.hxx>
#include "sal/main.h"
#include "helper.hxx"
#include "tagtest.hxx"
#include "gsicheck.hxx"

namespace {

rtl::OString addSuffix(
    rtl::OString const & pathname, rtl::OString const & suffix)
{
    sal_Int32 n = pathname.lastIndexOf('.');
    if (n == -1) {
        fprintf(
            stderr,
            ("Error: pathname \"%s\" does not contain dot to add suffix in"
             " front of\n"),
            pathname.getStr());
        exit(EXIT_FAILURE);
    }
    return pathname.replaceAt(n, 0, suffix);
}

}

void Help()
{
    fprintf( stdout, "\n" );
    fprintf( stdout, "gsicheck checks the syntax of tags in SDF-Files\n" );
    fprintf( stdout, "         checks for inconsistencies and malicious UTF8 encoding\n" );
    fprintf( stdout, "         checks tags in Online Help\n" );
    fprintf( stdout, "         relax GID/LID length to %s\n",
        rtl::OString::valueOf(static_cast<sal_Int32>(MAX_GID_LID_LEN)).getStr() );
    fprintf( stdout, "\n" );
    fprintf( stdout, "Syntax: gsicheck [ -c ] [-f] [ -we ] [ -wef ErrorFilename ] [ -wc ]\n" );
    fprintf( stdout, "                 [ -wcf CorrectFilename ] [ -s | -t ] [ -l LanguageID ]\n" );
    fprintf( stdout, "                 [ -r ReferenceFile ] filename\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "-c    Add context to error message (Print the line containing the error)\n" );
    fprintf( stdout, "-f    try to fix errors. See also -wf -wff \n" );
    fprintf( stdout, "-wf   Write File containing all fixed parts\n" );
    fprintf( stdout, "-wff  Same as above but give own filename\n" );
    fprintf( stdout, "-we   Write File containing all errors\n" );
    fprintf( stdout, "-wef  Same as above but give own filename\n" );
    fprintf( stdout, "-wc   Write File containing all correct parts\n" );
    fprintf( stdout, "-wcf  Same as above but give own filename\n" );
    fprintf( stdout, "-s    Check only source language. Should be used before handing out to vendor.\n" );
    fprintf( stdout, "-t    Check only Translation language(s). Should be used before merging.\n" );
    fprintf( stdout, "-e    disable encoding checks. E.g.: double questionmark \'??\' which may be the\n" );
    fprintf( stdout, "      result of false conversions\n" );
    fprintf( stdout, "-l    ISO language code of the source language.\n" );
    fprintf( stdout, "      Default is en-US. Use \"\" (empty string) or 'none'\n" );
    fprintf( stdout, "      to disable source language dependent checks\n" );
    fprintf( stdout, "-r    Reference filename to check that source language entries\n" );
    fprintf( stdout, "      have not been changed\n" );
    fprintf( stdout, "\n" );
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    sal_Bool bError = sal_False;
    sal_Bool bPrintContext = sal_False;
    sal_Bool bCheckSourceLang = sal_False;
    sal_Bool bCheckTranslationLang = sal_False;
    sal_Bool bWriteError = sal_False;
    sal_Bool bWriteCorrect = sal_False;
    sal_Bool bWriteFixed = sal_False;
    sal_Bool bFixTags = sal_False;
    sal_Bool bAllowSuspicious = sal_False;
    rtl::OString aErrorFilename;
    rtl::OString aCorrectFilename;
    rtl::OString aFixedFilename;
    sal_Bool bFileHasError = sal_False;
    rtl::OString aSourceLang( "en-US" );     // English is default
    rtl::OString aFilename;
    rtl::OString aReferenceFilename;
    sal_Bool bReferenceFile = sal_False;
    for ( int i = 1 ; i < argc ; i++ )
    {
        if ( *argv[ i ] == '-' )
        {
            switch (*(argv[ i ]+1))
            {
                case 'c':bPrintContext = sal_True;
                    break;
                case 'w':
                    {
                        if ( (*(argv[ i ]+2)) == 'e' )
                        {
                            if ( (*(argv[ i ]+3)) == 'f' )
                                if ( (i+1) < argc )
                                {
                                    aErrorFilename = argv[i + 1];
                                    bWriteError = sal_True;
                                    i++;
                                }
                                else
                                {
                                    fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                                    bError = sal_True;
                                }
                            else
                                   bWriteError = sal_True;
                        }
                        else if ( (*(argv[ i ]+2)) == 'c' )
                            if ( (*(argv[ i ]+3)) == 'f' )
                                if ( (i+1) < argc )
                                {
                                    aCorrectFilename = argv[i + 1];
                                    bWriteCorrect = sal_True;
                                    i++;
                                }
                                else
                                {
                                    fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                                    bError = sal_True;
                                }
                            else
                                   bWriteCorrect = sal_True;
                        else if ( (*(argv[ i ]+2)) == 'f' )
                            if ( (*(argv[ i ]+3)) == 'f' )
                                if ( (i+1) < argc )
                                {
                                    aFixedFilename = argv[i + 1];
                                    bWriteFixed = sal_True;
                                    bFixTags = sal_True;
                                    i++;
                                }
                                else
                                {
                                    fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                                    bError = sal_True;
                                }
                            else
                            {
                                   bWriteFixed = sal_True;
                                bFixTags = sal_True;
                            }
                        else
                        {
                            fprintf( stderr, "\nERROR: Unknown Switch %s!\n\n", argv[ i ] );
                            bError = sal_True;
                        }
                    }
                    break;
                case 's':bCheckSourceLang = sal_True;
                    break;
                case 't':bCheckTranslationLang = sal_True;
                    break;
                case 'l':
                    {
                        if ( (i+1) < argc )
                        {
                            aSourceLang = argv[ i+1 ];
                            if ( aSourceLang.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("none")) )
                                aSourceLang = rtl::OString();
                            i++;
                        }
                        else
                        {
                            fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                            bError = sal_True;
                        }
                    }
                    break;
                case 'r':
                    {
                        if ( (i+1) < argc )
                        {
                            aReferenceFilename = argv[ i+1 ];
                            bReferenceFile = sal_True;
                            i++;
                        }
                        else
                        {
                            fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                            bError = sal_True;
                        }
                    }
                    break;
                case 'f':
                    {
                        bFixTags = sal_True;
                    }
                    break;
                case 'e':
                    {
                        bAllowSuspicious = sal_True;
                    }
                    break;
                default:
                    fprintf( stderr, "\nERROR: Unknown Switch %s!\n\n", argv[ i ] );
                    bError = sal_True;
            }
        }
        else
        {
            if  (aFilename.isEmpty())
                aFilename = argv[i];
            else
            {
                fprintf( stderr, "\nERROR: Only one filename may be specified!\n\n");
                bError = sal_True;
            }
        }
    }


    if (aFilename.isEmpty() || bError)
    {
        Help();
        exit ( 0 );
    }

    if ( !aSourceLang.isEmpty() && !LanguageOK( aSourceLang ) )
    {
        fprintf( stderr, "\nERROR: The Language '%s' is invalid!\n\n", aSourceLang.getStr() );
        Help();
        exit ( 1 );
    }

    if ( bCheckSourceLang && bCheckTranslationLang )
    {
        fprintf( stderr, "\nERROR: The Options -s and -t are mutually exclusive.\nUse only one of them.\n\n" );
        Help();
        exit ( 1 );
    }



    std::ifstream aGSI(aFilename.getStr());
    if (!aGSI.is_open()) {
        fprintf( stderr, "\nERROR: Could not open GSI-File %s!\n\n", aFilename.getStr() );
        exit ( 3 );
    }

    std::ifstream aReferenceGSI;
    if ( bReferenceFile )
    {
        aReferenceGSI.open(aReferenceFilename.getStr());
        if (!aReferenceGSI.is_open()) {
            fprintf( stderr, "\nERROR: Could not open Input-File %s!\n\n", aFilename.getStr() );
            exit ( 3 );
        }
    }

    LazyStream aOkOut;
    if ( bWriteCorrect )
    {
        if (aCorrectFilename.isEmpty())
        {
            aCorrectFilename = addSuffix(
                aFilename, rtl::OString(RTL_CONSTASCII_STRINGPARAM("_ok")));
        }
        aOkOut.SetFileName(aCorrectFilename);
    }

    LazyStream aErrOut;
    if ( bWriteError )
    {
        if (aErrorFilename.isEmpty())
        {
            aErrorFilename = addSuffix(
                aFilename, rtl::OString(RTL_CONSTASCII_STRINGPARAM("_err")));
        }
        aErrOut.SetFileName(aErrorFilename);
    }

    LazyStream aFixOut;
    if ( bWriteFixed )
    {
        if (aFixedFilename.isEmpty())
        {
            aFixedFilename = addSuffix(
                aFilename, rtl::OString(RTL_CONSTASCII_STRINGPARAM("_fix")));
        }
        aFixOut.SetFileName(aFixedFilename);
    }


    GSILine* pReferenceLine = NULL;
    std::size_t nReferenceLine = 0;

    GSILine* pGSILine = NULL;
    rtl::OString aOldId("No Valid ID");   // just set to something which can never be an ID
    GSIBlock *pBlock = NULL;
    std::size_t nLine = 0;

    while (!aGSI.eof())
    {
        std::string s;
        std::getline(aGSI, s);
        nLine++;

        pGSILine = new GSILine(rtl::OString(s.data(), s.length()), nLine );
        sal_Bool bDelete = sal_True;

        if ( !pGSILine->data_.isEmpty() )
        {
            if ( FORMAT_UNKNOWN == pGSILine->GetLineFormat() )
            {
                PrintError( "Format of line is unknown. Ignoring!", "Line format", pGSILine->data_.copy( 0,40 ), bPrintContext, pGSILine->GetLineNumber() );
                pGSILine->NotOK();
                if ( bWriteError )
                {
                    bFileHasError = sal_True;
                    aErrOut.LazyOpen();
                    aErrOut << pGSILine->data_.getStr();
                }
            }
            else if ( pGSILine->GetLineType().equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("res-comment")) )
            {   // ignore comment lines, but write them to Correct Items File
                if ( bWriteCorrect )
                {
                    aOkOut.LazyOpen();
                    aOkOut << pGSILine->data_.getStr() << '\n';
                }
            }
            else
            {
                rtl::OString aId = pGSILine->GetUniqId();
                if ( aId != aOldId )
                {
                    if ( pBlock )
                    {
                        bFileHasError |= pBlock->CheckSyntax( nLine, !aSourceLang.isEmpty(), bFixTags );

                        if ( bWriteError )
                            pBlock->WriteError( aErrOut, !aSourceLang.isEmpty() );
                        if ( bWriteCorrect )
                            pBlock->WriteCorrect( aOkOut, !aSourceLang.isEmpty() );
                        if ( bWriteFixed )
                            pBlock->WriteFixed( aFixOut );

                        delete pBlock;
                    }
                    pBlock = new GSIBlock( bPrintContext, bCheckSourceLang, bCheckTranslationLang, bReferenceFile, bAllowSuspicious );

                    aOldId = aId;

                    // find corresponding line in reference file
                    if ( bReferenceFile )
                    {
                        sal_Bool bContinueSearching = sal_True;
                        while ( ( !aReferenceGSI.eof() || pReferenceLine ) && bContinueSearching )
                        {
                            if ( !pReferenceLine )
                            {
                                std::string s2;
                                std::getline(aReferenceGSI, s2);
                                nReferenceLine++;
                                pReferenceLine = new GSILine(
                                    rtl::OString(s2.data(), s2.length()),
                                    nReferenceLine);
                            }
                            if ( pReferenceLine->GetLineFormat() != FORMAT_UNKNOWN )
                            {
                                if ( pReferenceLine->GetUniqId() == aId && pReferenceLine->GetLanguageId() == aSourceLang )
                                {
                                    pBlock->SetReferenceLine( pReferenceLine );
                                    pReferenceLine = NULL;
                                }
                                else if ( pReferenceLine->GetUniqId() > aId )
                                {
                                    bContinueSearching = sal_False;
                                }
                                else
                                {
                                    if ( pReferenceLine->GetUniqId() < aId  && pReferenceLine->GetLanguageId() == aSourceLang )
                                        PrintError( "No Entry in source file found. Entry has been removed from source file", "File format", "", bPrintContext, pGSILine->GetLineNumber(), pReferenceLine->GetUniqId() );
                                    delete pReferenceLine;
                                    pReferenceLine = NULL;
                                }
                            }
                            else
                            {
                                delete pReferenceLine;
                                pReferenceLine = NULL;
                            }

                        }
                    }

                }
                pBlock->InsertLine( pGSILine, aSourceLang );
                bDelete = sal_False;
            }
        }
        if ( bDelete )
            delete pGSILine;
    }
    if ( pBlock )
    {
        bFileHasError |= pBlock->CheckSyntax( nLine, !aSourceLang.isEmpty(), bFixTags );

        if ( bWriteError )
            pBlock->WriteError( aErrOut, !aSourceLang.isEmpty() );
        if ( bWriteCorrect )
            pBlock->WriteCorrect( aOkOut, !aSourceLang.isEmpty() );
        if ( bWriteFixed )
            pBlock->WriteFixed( aFixOut );

        delete pBlock;
    }
    aGSI.close();

    if ( bWriteError )
        aErrOut.close();
    if ( bWriteCorrect )
        aOkOut.close();
    if ( bWriteFixed )
        aFixOut.close();
    if ( bFileHasError )
        return 55;
    else
        return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
