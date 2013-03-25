/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <cstring>
#include "sal/main.h"

#include "export.hxx"
#include "treemerge.hxx"

void WriteUsage()
{
    std::cout
        << "Syntax: Treex [-r Root] -i FileIn -o FileOut"
        << " [-m DataBase] [-l Lang]\n"
        << " Root:  Path to root of localized xhp files\n"
        << " FileIn:   Source files (*.tree)\n"
        << " FileOut:  Destination file (*.*)\n"
        << " DataBase: Mergedata (*.po)\n"
        << " Lang: Restrict the handled languages; one element of\n"
        << " (de, en-US, ...) or all\n";
}


SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    OString sXHPRoot;
    for (int nIndex = 1; nIndex != argc; ++nIndex)
    {
        if (std::strcmp(argv[nIndex], "-r") == 0)
        {
            sXHPRoot = OString( argv[nIndex + 1] );
            for( int nIndex2 = nIndex+3; nIndex2 < argc; nIndex2 = nIndex2 + 2 )
            {
                argv[nIndex-3] = argv[nIndex-1];
                argv[nIndex-2] = argv[nIndex];
            }
            argc = argc - 2;
            break;
        }
    }
    HandledArgs aArgs;
    if( !Export::handleArguments(argc, argv, aArgs) )
    {
        WriteUsage();
        return 1;
    }

    TreeParser aParser(aArgs.m_sInputFile, aArgs.m_sLanguage );
    if( !aParser.isInitialized() )
    {
        return 1;
    }

    if( aArgs.m_bMergeMode || !sXHPRoot.isEmpty() )
    {
        aParser.Merge( aArgs.m_sMergeSrc, aArgs.m_sOutputFile, sXHPRoot );
    }
    else
    {
        aParser.Extract( aArgs.m_sOutputFile );
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
