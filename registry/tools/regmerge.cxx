/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: regmerge.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_registry.hxx"

#include <stdio.h>
#include <string.h>

#include "registry/registry.h"
#include    <rtl/ustring.hxx>
#include    <rtl/alloc.h>
#include <osl/process.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.hxx>

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

using namespace ::rtl;
using namespace ::osl;

sal_Bool isFileUrl(const OString& fileName)
{
    if (fileName.indexOf("file://") == 0 )
        return sal_True;
    return sal_False;
}

OUString convertToFileUrl(const OString& fileName)
{
    if ( isFileUrl(fileName) )
    {
        return OStringToOUString(fileName, osl_getThreadTextEncoding());
    }

    OUString uUrlFileName;
    OUString uFileName(fileName.getStr(), fileName.getLength(), osl_getThreadTextEncoding());
    if ( fileName.indexOf('.') == 0 || fileName.indexOf(SEPARATOR) < 0 )
    {
        OUString uWorkingDir;
        if (osl_getProcessWorkingDir(&uWorkingDir.pData) != osl_Process_E_None)
        {
            OSL_ASSERT(false);
        }
        if (FileBase::getAbsoluteFileURL(uWorkingDir, uFileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    } else
    {
        if (FileBase::getFileURLFromSystemPath(uFileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    }

    return uUrlFileName;
}

int realargc;
char* realargv[2048];

static void dumpHelp()
{
    fprintf(stderr, "using: regmerge [-v|--verbose] mergefile mergeKeyName regfile_1 ... regfile_n\n");
    fprintf(stderr, "       regmerge @regcmds\nOptions:\n");
    fprintf(stderr, "  -v, --verbose : verbose output on stdout.\n");
    fprintf(stderr, "  mergefile     : specifies the merged registry file. If this file doesn't exists,\n");
    fprintf(stderr, "                  it is created.\n");
    fprintf(stderr, "  mergeKeyName  : specifies the merge key, everything is merged under this key.\n");
    fprintf(stderr, "                  If this key doesn't exists, it is created.\n");
    fprintf(stderr, "  regfile_1..n  : specifies one or more registry files which are merged.\n");
}

static bool checkCommandFile(char* cmdfile)
{
    FILE    *commandfile;
    char    option[256];
    bool    bVerbose = false;

    commandfile = fopen(cmdfile+1, "r");
    if( commandfile == NULL )
    {
        fprintf(stderr, "ERROR: Can't open command file \"%s\"\n", cmdfile);
    } else
    {
        while ( fscanf(commandfile, "%s", option) != EOF )
        {
            if (option[0]== '@')
            {
                bool bRet = checkCommandFile(option);
                // ensure that the option will not be overwritten
                if ( !bRet )
                    bVerbose = bRet;
            } else {
                if (option[0]== '-') {
                    if (strncmp(option, "-v", 2)  == 0 ||
                        strncmp(option, "--verbose", 9) == 0)
                    {
                        bVerbose = true;
                    } else {
                        fprintf(stderr, "ERROR: unknown option \"%s\"\n", option);
                        dumpHelp();
                        exit(-1);
                    }
                }else
                {
                    realargv[realargc]= strdup(option);
                    realargc++;
                }
            }
            if (realargc == 2047)
            {
                fprintf(stderr, "ERROR: more than 2048 arguments.\n");
                break;
            }
        }
        fclose(commandfile);
    }

    return bVerbose;
}

static bool checkCommandArgs(int argc, char **argv)
{
    bool bVerbose = false;

    realargc = 0;

    for (int i=0; i<argc; i++)
    {
        if (argv[i][0]== '@')
        {
            bool bRet = checkCommandFile(argv[i]);
            // ensure that the option will not be overwritten
            if ( !bRet )
                bVerbose = bRet;
        } else {
            if (argv[i][0]== '-') {
                if (strncmp(argv[i], "-v", 2)  == 0 ||
                    strncmp(argv[i], "--verbose", 9) == 0)
                {
                    bVerbose = true;
                } else {
                    fprintf(stderr, "ERROR: unknown option \"%s\"\n", argv[i]);
                    dumpHelp();
                    exit(-1);
                }
            } else
            {
                realargv[realargc]= strdup(argv[i]);
                realargc++;
            }
        }
    }

    return bVerbose;
}

static void cleanCommandArgs()
{
    for (int i=0; i<realargc; i++)
    {
        free(realargv[i]);
    }
}



#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    RegHandle       hReg;
    RegKeyHandle    hRootKey;
    bool            bVerbose = checkCommandArgs(argc, argv);

    if (realargc < 4)
    {
        dumpHelp();
        cleanCommandArgs();
        exit(1);
    }

    ::rtl::OUString regName( convertToFileUrl(realargv[1]) );
    if (reg_openRegistry(regName.pData, &hReg, REG_READWRITE))
    {
        if (reg_createRegistry(regName.pData, &hReg))
        {
            if (bVerbose)
                fprintf(stderr, "open registry \"%s\" failed\n", realargv[1]);
            cleanCommandArgs();
            exit(-1);
        }
    }

    if (!reg_openRootKey(hReg, &hRootKey))
    {
        ::rtl::OUString mergeKeyName( ::rtl::OUString::createFromAscii(realargv[2]) );
        ::rtl::OUString targetRegName;
        for (int i = 3; i < realargc; i++)
        {
            targetRegName = convertToFileUrl(realargv[i]);
            RegError _ret = reg_mergeKey(
                hRootKey, mergeKeyName.pData, targetRegName.pData, sal_False,
                bVerbose);
            if (_ret)
            {
                if (_ret == REG_MERGE_CONFLICT)
                {
                    if (bVerbose)
                        fprintf(stderr, "merging registry \"%s\" under key \"%s\" in registry \"%s\".\n",
                                realargv[i], realargv[2], realargv[1]);
                } else
                {
                    if (bVerbose)
                        fprintf(stderr, "ERROR: merging registry \"%s\" under key \"%s\" in registry \"%s\" failed.\n",
                                realargv[i], realargv[2], realargv[1]);
                    exit(-2);
                }
            } else
            {
                if (bVerbose)
                    fprintf(stderr, "merging registry \"%s\" under key \"%s\" in registry \"%s\".\n",
                            realargv[i], realargv[2], realargv[1]);
            }
        }

        if (reg_closeKey(hRootKey))
        {
            if (bVerbose)
                fprintf(stderr, "closing root key of registry \"%s\" failed\n",
                        realargv[1]);
            exit(-3);
        }
    } else
    {
        if (bVerbose)
            fprintf(stderr, "open root key of registry \"%s\" failed\n",
                    realargv[1]);
        exit(-4);
    }

    if (reg_closeRegistry(hReg))
    {
        if (bVerbose)
            fprintf(stderr, "closing registry \"%s\" failed\n", realargv[1]);
        cleanCommandArgs();
        exit(-5);
    }

    cleanCommandArgs();
    return(0);
}


