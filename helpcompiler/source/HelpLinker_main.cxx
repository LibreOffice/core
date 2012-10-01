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

#include <helpcompiler/HelpCompiler.hxx>
#include <helpcompiler/HelpLinker.hxx>
#include <iostream>
#include <sal/main.h>

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
        args.push_back(std::string(argv[i]));
    try
    {
        HelpLinker* pHelpLinker = new HelpLinker();
        pHelpLinker->main( args );
        delete pHelpLinker;
    }
    catch( const HelpProcessingException& e )
    {
        std::cerr << e.m_aErrorMsg;
        exit(1);
    }
    return 0;
}

