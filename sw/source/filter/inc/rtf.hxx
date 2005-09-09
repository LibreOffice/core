/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rtf.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:50:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _RTF_HXX
#define _RTF_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

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


