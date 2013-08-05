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
#ifndef _SYMBOL_LOADER_HXX
#define _SYMBOL_LOADER_HXX
#include <iostream>
#include <osl/module.h>
#include <rtl/ustring.hxx>

#define SYM_MAP(a) { #a, (SymbolFunc *)&a }

typedef void (*SymbolFunc) (void);

struct ApiMap
{
    const char *symName;
    SymbolFunc *refValue;
};

namespace
{
    const char *libNames[] = {
        "libvlc.so.5",
        "libvlccore.so.5"
    };

    template<size_t N>
    bool tryLink( oslModule &aModule, const ApiMap ( &pMap )[N] )
    {
        for (uint i = 0; i < N; ++i)
        {
            SymbolFunc aMethod = ( SymbolFunc )osl_getFunctionSymbol
                ( aModule, OUString::createFromAscii ( pMap[ i ].symName ).pData );
            if ( !aMethod )
                return false;

            *pMap[ i ].refValue = aMethod;
        }

        return true;
    }
}

template<size_t N>
bool InitApiMap( const ApiMap ( &pMap )[N]  )
{
    oslModule aModule;

    for (uint j = 0; j < sizeof(libNames) / sizeof(libNames[0]); ++j)
    {
        aModule = osl_loadModule( OUString::createFromAscii
                                  ( libNames[ j ] ).pData,
                                  SAL_LOADMODULE_DEFAULT );

        if( aModule == NULL)
            continue;

        tryLink( aModule, pMap );

        osl_unloadModule( aModule );
    }

    return false;
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
