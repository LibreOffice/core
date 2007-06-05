/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: odma_main.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-06-05 18:10:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef ODMA_PROVIDER_HXX
#include "odma_provider.hxx"
#endif

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

