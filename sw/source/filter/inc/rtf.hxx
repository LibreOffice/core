/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
            USHORT nOrient : 4;
            USHORT nRelOrient : 1;
        } Flags;
        USHORT nVal;
    } Value;

public:
    RTFVertOrient( USHORT nValue ) { Value.nVal = nValue; }

    RTFVertOrient( USHORT nOrient, USHORT nRelOrient )  {
        Value.Flags.nOrient = nOrient;
        Value.Flags.nRelOrient = nRelOrient;
    }

    USHORT GetOrient()   const { return Value.Flags.nOrient; }
    USHORT GetRelation() const { return Value.Flags.nRelOrient; }
    USHORT GetValue()    const { return Value.nVal; }
};


class RTFHoriOrient
{
    union {
        struct {
            USHORT nOrient : 4;
            USHORT nRelAnchor : 4;
            USHORT nRelOrient : 1;
        } Flags;
        USHORT nVal;
    } Value;

public:
    RTFHoriOrient( USHORT nValue ) { Value.nVal = nValue; }

    RTFHoriOrient( USHORT nOrient, USHORT nRelOrient ) {
        Value.Flags.nOrient = nOrient;
        Value.Flags.nRelOrient = nRelOrient;
        Value.Flags.nRelAnchor = 0;
    }

    USHORT GetOrient()   const { return Value.Flags.nOrient; }
    USHORT GetRelation() const { return Value.Flags.nRelOrient; }
    USHORT GetValue()    const { return Value.nVal; }
};

class RTFProtect
{
    union {
        struct {
            BOOL bCntnt : 1;
            BOOL bSize : 1;
            BOOL bPos : 1;
        } Flags;
        BYTE nVal;
    } Value;
public:
    RTFProtect( BYTE nValue ) { Value.nVal = nValue; }

    RTFProtect( BOOL bCntnt, BOOL bSize, BOOL bPos ) {
        Value.Flags.bCntnt = bCntnt;
        Value.Flags.bSize = bSize;
        Value.Flags.bPos = bPos;
    }

    BOOL GetCntnt()     const { return Value.Flags.bCntnt; }
    BOOL GetSize()      const { return Value.Flags.bSize; }
    BOOL GetPos()       const { return Value.Flags.bPos; }
    USHORT GetValue()   const { return Value.nVal; }
};


class RTFSurround
{
    union {
        struct {
            USHORT nGoldCut : 1;
            USHORT nOrder : 4;
        } Flags;
        BYTE nVal;
    } Value;
public:
    RTFSurround( BYTE nValue ) { Value.nVal = nValue; }

    RTFSurround( BOOL bGoldCut, BYTE nOrder ) {
        Value.Flags.nOrder = nOrder;
        Value.Flags.nGoldCut = bGoldCut;
    }

    BYTE GetOrder()     const { return (BYTE)Value.Flags.nOrder; }
    BOOL GetGoldCut()   const { return (BOOL)Value.Flags.nGoldCut; }
    USHORT GetValue()   const { return Value.nVal; }
};

#endif // _RTF_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
