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

#include <stdio.h>
#include <stdlib.h>

#include "sal/main.h"

#include "helpmerge.hxx"

#ifndef TESTDRIVER

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    HandledArgs aArgs;
    if ( !Export::handleArguments( argc, argv, aArgs) ) {
        Export::writeUsage("helpex","xhp");
        return 1;
    }
    //sal_uInt32 startfull = Export::startMessure();
    bool hasNoError = true;

    if ( !aArgs.m_sOutputFile.isEmpty() ){
        HelpParser aParser( aArgs.m_sInputFile );

        if ( aArgs.m_bMergeMode )
        {
            //sal_uInt64 startreadloc = Export::startMessure();
            MergeDataFile aMergeDataFile( aArgs.m_sMergeSrc, aArgs.m_sInputFile, false );

            hasNoError = aParser.Merge( aArgs.m_sMergeSrc, aArgs.m_sOutputFile , Export::sLanguages , aMergeDataFile );
        }
        else
            hasNoError =
                aParser.CreateSDF(
                    aArgs.m_sOutputFile, aArgs.m_sPrj, aArgs.m_sPrjRoot,
                    aArgs.m_sInputFile, new XMLFile( OUString('0') ), "help" );
    }
    else
        std::cerr << "helpex ERROR: Wrong input parameters!\n";

    if( hasNoError )
        return 0;
    else
        return 1;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
