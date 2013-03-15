/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include "sal/main.h"

#include "export.hxx"
#include "stringmerge.hxx"

void WriteUsage()
{
    std::cout
        << "Syntax: stringex [-p Prj] [-r Root] -i FileIn -o FileOut"
        << " [-m DataBase] [-l l1,l2,...]\n"
        << " Prj:      Project\n"
        << " Root:  Path to project root (../.. etc.)\n"
        << " FileIn:   Source files (strings.xml)\n"
        << " FileOut:  Destination file (*.*)\n"
        << " DataBase: Mergedata (*.po)\n"
        << " -l: Restrict the handled languages; l1, l2, ... are elements of"
        << " (de, en-US, ...)\n";
}


SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    HandledArgs aArgs;
    if( !Export::handleArguments(argc, argv, aArgs) )
    {
        WriteUsage();
        return 1;
    }

        StringParser aParser(aArgs.m_sInputFile, Export::sLanguages);
        if( !aParser.isInitialized() )
        {
            return 1;
        }

        if( aArgs.m_bMergeMode || aArgs.m_sPrj.isEmpty() )
        {
            aParser.Merge(
                aArgs.m_sMergeSrc, aArgs.m_sOutputFile );
        }
        else
        {
            aParser.Extract( aArgs.m_sOutputFile );
        }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
