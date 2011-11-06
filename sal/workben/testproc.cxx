/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


