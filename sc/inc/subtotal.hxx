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

#ifndef SC_SUBTOTAL_HXX
#define SC_SUBTOTAL_HXX

#include "global.hxx"

class SubTotal
{
public:
    static bool SafePlus( double& fVal1, double fVal2);
    static bool SafeMult( double& fVal1, double fVal2);
    static bool SafeDiv( double& fVal1, double fVal2);
};


struct ScFunctionData                   // to calculate single functions
{
    ScSubTotalFunc  eFunc;
    double          nVal;
    long            nCount;
    bool            bError;

    ScFunctionData( ScSubTotalFunc eFn ) :
        eFunc(eFn), nVal(0.0), nCount(0), bError(false) {}
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
