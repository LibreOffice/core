/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _RTF_HXX
#define _RTF_HXX

#include <tools/solar.h>

// Umsetzung einiger FlyFrame-Attribute
class RTFVertOrient
{
    union {
        struct {
            sal_uInt16 nOrient : 4;
            sal_uInt16 nRelOrient : 1;
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
            sal_uInt16 nRelOrient : 1;
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


