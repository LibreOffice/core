/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 David Ostrovsky <d.ostrovsky@gmx.de> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifdef AIX
#    undef _THREAD_SAFE
#endif

#include "HelpCompiler.hxx"
#include "l10ntools/HelpLinker.hxx"

#include <map>

#include <string.h>
#include <limits.h>

#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/functions.h>
#include <libxslt/extensions.h>

#include <sal/main.h>
#include <sal/types.h>
#include <osl/time.h>
#include <rtl/bootstrap.hxx>

#include <expat.h>

#define DBHELP_ONLY

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

