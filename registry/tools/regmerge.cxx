/*************************************************************************
 *
 *  $RCSfile: regmerge.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>
#include <string.h>

#include "registry/registry.h"

#ifndef _RTL_USTRING_HXX_
#include    <rtl/ustring.hxx>
#endif

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

int realargc;
char* realargv[2048];

static void checkCommandFile(char* cmdfile)
{
    FILE    *commandfile;
    char    option[256];

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
                checkCommandFile(option);
            } else
            {
                realargv[realargc]= strdup(option);
                realargc++;
            }

            if (realargc == 2047)
            {
                fprintf(stderr, "ERROR: more than 2048 arguments.\n");
                break;
            }
        }
        fclose(commandfile);
    }
}

static void checkCommandArgs(int argc, char **argv)
{
    realargc = 0;

    for (int i=0; i<argc; i++)
    {
        if (argv[i][0]== '@')
        {
            checkCommandFile(argv[i]);
        } else
        {
            realargv[i]= strdup(argv[i]);
            realargc++;
        }
    }
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

    checkCommandArgs(argc, argv);

    if (realargc < 4)
    {
        fprintf(stderr, "using: regmerge mergefile mergeKeyName regfile_1 ... regfile_n\n");
        fprintf(stderr, "       regmerge @regcmds\n");

        cleanCommandArgs();
        exit(1);
    }

    ::rtl::OUString regName( ::rtl::OUString::createFromAscii(realargv[1]) );
    if (reg_openRegistry(regName.pData, &hReg, REG_READWRITE))
    {
        if (reg_createRegistry(regName.pData, &hReg))
        {
            fprintf(stderr, "open registry \"%s\" failed\n", realargv[1]);
            cleanCommandArgs();
            exit(-1);
        }
    }

    if (!reg_openRootKey(hReg, &hRootKey))
    {
        ::rtl::OUString mergeKeyName( ::rtl::OUString::createFromAscii(realargv[2]) );
        ::rtl::OUString targetRegName;
        RegError _ret = REG_NO_ERROR;
        for (int i = 3; i < realargc; i++)
        {
            targetRegName = ::rtl::OUString::createFromAscii(realargv[i]);
            if (_ret = reg_mergeKey(hRootKey, mergeKeyName.pData, targetRegName.pData, sal_False, sal_True))
            {
                if (_ret == REG_MERGE_CONFLICT)
                {
                    fprintf(stderr, "merging registry \"%s\" under key \"%s\" in registry \"%s\".\n",
                            realargv[i], realargv[2], realargv[1]);
                } else
                {
                    fprintf(stderr, "ERROR: merging registry \"%s\" under key \"%s\" in registry \"%s\" failed.\n",
                            realargv[i], realargv[2], realargv[1]);
                    exit(-2);
                }
            } else
            {
                fprintf(stderr, "merging registry \"%s\" under key \"%s\" in registry \"%s\".\n",
                        realargv[i], realargv[2], realargv[1]);
            }
        }

        if (reg_closeKey(hRootKey))
        {
            fprintf(stderr, "closing root key of registry \"%s\" failed\n", realargv[1]);
            exit(-3);
        }
    } else
    {
        fprintf(stderr, "open root key of registry \"%s\" failed\n", realargv[1]);
        exit(-4);
    }

    if (reg_closeRegistry(hReg))
    {
        fprintf(stderr, "closing registry \"%s\" failed\n", realargv[1]);
        cleanCommandArgs();
        exit(-5);
    }

    cleanCommandArgs();
    return(0);
}


