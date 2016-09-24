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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_GETMETRICVAL_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_GETMETRICVAL_HXX

#include <tools/solar.h>

#define CM_1  0         // 1 centimeter     or 1/2 inch
#define CM_05 1         // 0.5 centimeter   or 1/4 inch
#define CM_01 2         // 0.1 centimeter   or 1/20 inch

inline sal_uInt16 GetMetricVal( int n )
{
    sal_uInt16 nVal = 567;      // 1 cm

    if( CM_01 == n )
        nVal /= 10;
    else if( CM_05 == n )
        nVal /= 2;
    return nVal;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
