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


#ifdef WNT
#include <windows.h>
#endif
#include <osl/process.h>
#include "odma_provider.hxx"

#ifdef WNT
#define SOFFICE "soffice.exe"
#else
#define SOFFICE "soffice"
#endif

/** our main program to convert ODMAIDs to ODMA URLs
*/

#if (defined UNX)
void main( int argc, char * argv[] )
#else
#if (defined GCC)
int _cdecl main( int argc, char * argv[] )
#else
void _cdecl main( int argc, char * argv[] )
#endif
#endif
{
    static ::rtl::OUString sProcess(RTL_CONSTASCII_USTRINGPARAM(SOFFICE));
    if(argc > 1)
    {
        ::rtl::OUString* pArguments = new ::rtl::OUString[argc-1];
        for(int i = 0; i < argc-1; ++i)
        {
            pArguments[i] = ::rtl::OUString::createFromAscii(argv[i+1]);
            if( pArguments[i].matchIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(ODMA_URL_ODMAID)))
            {
                ::rtl::OUString sArgument
                      = ::rtl::OUString(
                          RTL_CONSTASCII_USTRINGPARAM(
                              ODMA_URL_SCHEME ODMA_URL_SHORT "/"));
                sArgument += pArguments[i];
                pArguments[i] = sArgument;
            }
        }

        rtl_uString ** ustrArgumentList = new rtl_uString * [argc-1];
        for (int i = 0; i < argc-1; i++)
            ustrArgumentList[i] = pArguments[i].pData;

        oslProcess  aProcess;

        if ( osl_Process_E_None == osl_executeProcess(
                 sProcess.pData,
                 ustrArgumentList,
                 argc-1,
                 osl_Process_DETACHED,
                 NULL,
                 NULL,
                 NULL,
                 0,
                 &aProcess )
        )
            osl_freeProcessHandle( aProcess );

        delete [] ustrArgumentList;
        delete [] pArguments;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
