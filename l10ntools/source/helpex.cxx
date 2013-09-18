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

#include "sal/config.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#include <stdio.h>
#include <stdlib.h>

#include "sal/main.h"

#include "helpmerge.hxx"
#include "common.hxx"

#ifndef TESTDRIVER

void WriteUsage()
{
    std::cout
        << " Syntax: Helpex -[m]i FileIn -o FileOut [-m DataBase] [-l Lang]\n"
        << " FileIn + i:   Source file (*.xhp)\n"
        << " FileIn + -mi: File including paths of source files"
        << " (only for merge)\n"
        << " FileOut:  Destination file (*.*) or files (in case of -mi)\n"
        << " DataBase: Mergedata (*.po)\n"
        << " Lang: Restrict the handled languages; one element of\n"
        << " (de, en-US, ...) or all\n";
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {

    bool bMultiMerge = false;
    for (int nIndex = 1; nIndex != argc; ++nIndex)
    {
        if (std::strcmp(argv[nIndex], "-mi") == 0)
        {
            argv[nIndex][1] = 'i';
            argv[nIndex][2] = '\0';
            bMultiMerge = true;
            break;
        }
    }

    common::HandledArgs aArgs;
    if ( !common::handleArguments( argc, argv, aArgs) )
    {
        WriteUsage();
        return 1;
    }
    bool hasNoError = true;

    if ( aArgs.m_bMergeMode )
    {
        if( bMultiMerge )
        {
            std::ifstream aInput( aArgs.m_sInputFile.getStr() );
            if( !aInput.is_open() )
            {
                std::cerr << "Helpex error: cannot open input file\n";
                return 1;
            }
            MergeDataFile* pMergeDataFile = 0;
            if( aArgs.m_sLanguage != "qtz")
            {
                pMergeDataFile = new MergeDataFile(aArgs.m_sMergeSrc, OString(), false, false );
            }
            std::string sTemp;
            aInput >> sTemp;
            while( !aInput.eof() )
            {
                const OString sXhpFile( sTemp.data(), sTemp.length() );
                HelpParser aParser( sXhpFile );
                const OString sOutput(
                    aArgs.m_sOutputFile +
                    sXhpFile.copy( sXhpFile.lastIndexOf('/') ));
                if( !aParser.Merge( aArgs.m_sMergeSrc, sOutput,
                    aArgs.m_sLanguage, pMergeDataFile ))
                {
                    hasNoError = false;
                }
                aInput >> sTemp;
            }
            aInput.close();
            delete pMergeDataFile;
        }
        else
        {
            HelpParser aParser( aArgs.m_sInputFile );
            MergeDataFile* pMergeDataFile = 0;
            if( aArgs.m_sLanguage != "qtz")
            {
                pMergeDataFile = new MergeDataFile(aArgs.m_sMergeSrc, aArgs.m_sInputFile, false, false );
            }
            hasNoError =
                aParser.Merge(
                    aArgs.m_sMergeSrc, aArgs.m_sOutputFile,
                    aArgs.m_sLanguage, pMergeDataFile );
            delete pMergeDataFile;
        }
    }
    else
    {
        HelpParser aParser( aArgs.m_sInputFile );
        hasNoError =
            aParser.CreatePO(
                aArgs.m_sOutputFile, aArgs.m_sInputFile,
                new XMLFile( OString('0') ), "help" );
    }

    if( hasNoError )
        return 0;
    else
        return 1;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
