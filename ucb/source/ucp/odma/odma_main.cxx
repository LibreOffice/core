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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include <osl/process.h>
#include "odma_provider.hxx"

#ifdef WNT
#define SOFFICE "soffice.exe"
#else
#define SOFFICE "soffice"
#endif

/** our main program to convert ODMAIDs to ODMA URLs
*/

#if (defined UNX) || (defined OS2)
void main( int argc, char * argv[] )
#else
void _cdecl main( int argc, char * argv[] )
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
