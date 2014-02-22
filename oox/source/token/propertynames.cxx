/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "oox/token/propertynames.hxx"

namespace oox {



PropertyNameVector::PropertyNameVector()
{
    static const sal_Char* sppcPropertyNames[] =
    {
        
#include "propertynames.inc"
        ""
    };

    size_t nArraySize = SAL_N_ELEMENTS(sppcPropertyNames) - 1;
    reserve( nArraySize );
    for( size_t nIndex = 0; nIndex < nArraySize; ++nIndex )
        push_back( OUString::createFromAscii( sppcPropertyNames[ nIndex ] ) );
}



} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
