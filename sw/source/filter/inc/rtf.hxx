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
#ifndef _RTF_HXX
#define _RTF_HXX

#include <tools/solar.h>

// Umsetzung einiger FlyFrame-Attribute
class RTFVertOrient
{
    union {
        struct {
            sal_uInt16 nOrient : 4;
            sal_uInt16 nRelOrient : 4;
        } Flags;
        sal_uInt16 nVal;
    } Value;

public:
    RTFVertOrient( sal_uInt16 nValue ) { Value.nVal = nValue; }

    RTFVertOrient( sal_uInt16 nOrient, sal_uInt16 nRelOrient )  {
        Value.Flags.nOrient = nOrient;
        Value.Flags.nRelOrient = nRelOrient;
    }

    sal_uInt16 GetOrient()   const { return Value.Flags.nOrient; }
    sal_uInt16 GetRelation() const { return Value.Flags.nRelOrient; }
    sal_uInt16 GetValue()    const { return Value.nVal; }
};


class RTFHoriOrient
{
    union {
        struct {
            sal_uInt16 nOrient : 4;
            sal_uInt16 nRelAnchor : 4;
            sal_uInt16 nRelOrient : 4;
        } Flags;
        sal_uInt16 nVal;
    } Value;

public:
    RTFHoriOrient( sal_uInt16 nValue ) { Value.nVal = nValue; }

    RTFHoriOrient( sal_uInt16 nOrient, sal_uInt16 nRelOrient ) {
        Value.Flags.nOrient = nOrient;
        Value.Flags.nRelOrient = nRelOrient;
        Value.Flags.nRelAnchor = 0;
    }

    sal_uInt16 GetOrient()   const { return Value.Flags.nOrient; }
    sal_uInt16 GetRelation() const { return Value.Flags.nRelOrient; }
    sal_uInt16 GetValue()    const { return Value.nVal; }
};

class RTFProtect
{
    union {
        struct {
            sal_Bool bCntnt : 1;
            sal_Bool bSize : 1;
            sal_Bool bPos : 1;
        } Flags;
        sal_uInt8 nVal;
    } Value;
public:
    RTFProtect( sal_uInt8 nValue ) { Value.nVal = nValue; }

    RTFProtect( sal_Bool bCntnt, sal_Bool bSize, sal_Bool bPos ) {
        Value.Flags.bCntnt = bCntnt;
        Value.Flags.bSize = bSize;
        Value.Flags.bPos = bPos;
    }

    sal_Bool GetCntnt()     const { return Value.Flags.bCntnt; }
    sal_Bool GetSize()      const { return Value.Flags.bSize; }
    sal_Bool GetPos()       const { return Value.Flags.bPos; }
    sal_uInt16 GetValue()   const { return Value.nVal; }
};


class RTFSurround
{
    union {
        struct {
            sal_uInt16 nGoldCut : 1;
            sal_uInt16 nOrder : 4;
        } Flags;
        sal_uInt8 nVal;
    } Value;
public:
    RTFSurround( sal_uInt8 nValue ) { Value.nVal = nValue; }

    RTFSurround( sal_Bool bGoldCut, sal_uInt8 nOrder ) {
        Value.Flags.nOrder = nOrder;
        Value.Flags.nGoldCut = bGoldCut;
    }

    sal_uInt8 GetOrder()     const { return (sal_uInt8)Value.Flags.nOrder; }
    sal_Bool GetGoldCut()   const { return (sal_Bool)Value.Flags.nGoldCut; }
    sal_uInt16 GetValue()   const { return Value.nVal; }
};

#endif // _RTF_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
