/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <string>
#include <string.h>

#include <sal/main.h>

#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitInit.h>

using namespace std;

// Using SAL_IMPLEMENT_MAIN means that we can directly use the same
// arguments as would be provided to cppunittester, i.e. ensuring
// that we're operating with the correct directories etc.
SAL_IMPLEMENT_MAIN()
{
    string sPath( getenv("INSTDIR") );
    sPath += "/program";

    LibreOfficeKit* pOffice = lok_init( sPath.c_str() );

    // As this is the first init, pOffice should be NULL.
    // However there's no reason we shouldn't be able to operate
    // even if it isn't the first init.
    if ( pOffice )
    {
        pOffice->pClass->destroy( pOffice );
        pOffice = 0;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
