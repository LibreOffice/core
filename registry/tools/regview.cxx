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
#include "precompiled_registry.hxx"

#include "registry/registry.h"
#include "fileurl.hxx"

#include "rtl/ustring.hxx"

#include <stdio.h>
#include <string.h>

using rtl::OUString;
using namespace registry::tools;

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    RegHandle       hReg;
    RegKeyHandle    hRootKey, hKey;

    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "using: regview registryfile [keyName]\n");
        exit(1);
    }

    OUString regName( convertToFileUrl(argv[1], strlen(argv[1])) );
    if (reg_openRegistry(regName.pData, &hReg, REG_READONLY))
    {
        fprintf(stderr, "open registry \"%s\" failed\n", argv[1]);
        exit(1);
    }

    if (!reg_openRootKey(hReg, &hRootKey))
    {
        if (argc == 3)
        {
            OUString keyName( OUString::createFromAscii(argv[2]) );
            if (!reg_openKey(hRootKey, keyName.pData, &hKey))
            {
                if (reg_dumpRegistry(hKey))
                {
                    fprintf(stderr, "dumping registry \"%s\" failed\n", argv[1]);
                }

                if (reg_closeKey(hKey))
                {
                    fprintf(stderr, "closing key \"%s\" of registry \"%s\" failed\n",
                            argv[2], argv[1]);
                }
            }
            else
            {
                fprintf(stderr, "key \"%s\" not exists in registry \"%s\"\n",
                        argv[2], argv[1]);
            }
        }
        else
        {
            if (reg_dumpRegistry(hRootKey))
            {
                fprintf(stderr, "dumping registry \"%s\" failed\n", argv[1]);
            }
        }

        if (reg_closeKey(hRootKey))
        {
            fprintf(stderr, "closing root key of registry \"%s\" failed\n", argv[1]);
        }
    }
    else
    {
        fprintf(stderr, "open root key of registry \"%s\" failed\n", argv[1]);
    }

    if (reg_closeRegistry(hReg))
    {
        fprintf(stderr, "closing registry \"%s\" failed\n", argv[1]);
        exit(1);
    }

    return(0);
}


