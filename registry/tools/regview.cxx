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


#include "regapi.hxx"
#include "fileurl.hxx"

#include "rtl/ustring.hxx"

#include <stdio.h>
#include <string.h>

using namespace registry::tools;

#if (defined UNX)
int main( int argc, char * argv[] )
#else
int __cdecl main( int argc, char * argv[] )
#endif
{
    try
    {
        RegHandle       hReg;
        RegKeyHandle    hRootKey, hKey;

        if (argc < 2 || argc > 3)
        {
            fprintf(stderr, "using: regview registryfile [keyName]\n");
            exit(1);
        }

        OUString regName( convertToFileUrl(argv[1], strlen(argv[1])) );
        if (reg_openRegistry(regName.pData, &hReg, RegAccessMode::READONLY) != RegError::NO_ERROR)
        {
            fprintf(stderr, "open registry \"%s\" failed\n", argv[1]);
            exit(1);
        }

        if (reg_openRootKey(hReg, &hRootKey) == RegError::NO_ERROR)
        {
            if (argc == 3)
            {
                OUString keyName( OUString::createFromAscii(argv[2]) );
                if (reg_openKey(hRootKey, keyName.pData, &hKey) == RegError::NO_ERROR)
                {
                    if (reg_dumpRegistry(hKey) != RegError::NO_ERROR)
                    {
                        fprintf(stderr, "dumping registry \"%s\" failed\n", argv[1]);
                    }

                    if (reg_closeKey(hKey) != RegError::NO_ERROR)
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
                if (reg_dumpRegistry(hRootKey) != RegError::NO_ERROR)
                {
                    fprintf(stderr, "dumping registry \"%s\" failed\n", argv[1]);
                }
            }

            if (reg_closeKey(hRootKey) != RegError::NO_ERROR)
            {
                fprintf(stderr, "closing root key of registry \"%s\" failed\n", argv[1]);
            }
        }
        else
        {
            fprintf(stderr, "open root key of registry \"%s\" failed\n", argv[1]);
        }

        if (reg_closeRegistry(hReg) != RegError::NO_ERROR)
        {
            fprintf(stderr, "closing registry \"%s\" failed\n", argv[1]);
            exit(1);
        }

        return 0;
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "failure: \"%s\"\n", e.what());
        return 1;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
