/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testproc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:10:55 $
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
#include "precompiled_sal.hxx"


#include <osl/process.h>
#include <stdio.h>


#if defined WNT
    #define MAIN _cdecl main
#else
    #define MAIN main
#endif

void MAIN (void)
{
    oslProcess Process;
    oslProcessError  ProcessError;
    sal_uInt32 nArgCount;
    sal_uInt32 index;
    rtl_uString* ustrExeFile=0;
    rtl_uString* ustrArg=0;

    nArgCount=osl_getCommandArgCount();

    fprintf(stderr,"Arg Count == %i\n\n",nArgCount);

    for ( index = 0 ; index <= nArgCount+1 ; index++ )
    {
        fprintf(stderr,"Getting Arg No . %i\n",index);

        osl_getCommandArg(index,&ustrArg);

        fprintf(stderr,"done ...\n\n",index);
    }

    ProcessError = osl_getExecutableFile(&ustrExeFile);

/*
    osl_executeProcess( "e:\\test\\os2de202.exe",
                        NULL,
                        osl_Process_NORMAL,
                        0L,
                        NULL,
                        NULL,
                        NULL,
                        &Process );*/
}


