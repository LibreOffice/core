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

#ifndef _SVDSOB_HXX
#define _SVDSOB_HXX

#include <com/sun/star/uno/Any.hxx>
#include <tools/stream.hxx>

#ifndef _STRING_H
#include <tools/string.hxx> //wg. memset
#define _STRING_H
#endif
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  Deklaration eines statischen Mengentyps. Die Menge kann die Elemente
  0..255 aufnehmen und verbraucht stets 32 Bytes.
*/

class SVX_DLLPUBLIC SetOfByte
{
protected:
    sal_uInt8 aData[32];

public:
    SetOfByte(sal_Bool bInitVal = sal_False)
    {
        memset(aData, bInitVal ? 0xFF : 0x00, sizeof(aData));
    }

    sal_Bool operator==(const SetOfByte& rCmpSet) const
    {
        return (memcmp(aData, rCmpSet.aData, sizeof(aData)) == 0);
    }

    sal_Bool operator!=(const SetOfByte& rCmpSet) const
    {
        return (memcmp(aData, rCmpSet.aData, sizeof(aData))!=0);
    }

    void Set(sal_uInt8 a)
    {
        aData[a/8] |= 1<<a%8;
    }

    void Clear(sal_uInt8 a)
    {
        aData[a/8] &= ~(1<<a%8);
    }

    void Set(sal_uInt8 a, sal_Bool b)
    {
        if(b)
            Set(a);
        else
            Clear(a);
    }

    sal_Bool IsSet(sal_uInt8 a) const
    {
        return (aData[a/8] & 1<<a%8) != 0;
    }

    void SetAll()
    {
        memset(aData, 0xFF, sizeof(aData));
    }

    void ClearAll()
    {
        memset(aData, 0x00, sizeof(aData));
    }

    sal_Bool IsEmpty() const;
    sal_Bool IsFull() const;

    sal_uInt16 GetSetCount() const;
    sal_uInt8 GetSetBit(sal_uInt16 nNum) const;
    sal_uInt16 GetClearCount() const;
    sal_uInt8 GetClearBit(sal_uInt16 nNum) const;
    void operator&=(const SetOfByte& r2ndSet);
    void operator|=(const SetOfByte& r2ndSet);

    friend inline SvStream& operator<<(SvStream& rOut, const SetOfByte& rSet);
    friend inline SvStream& operator>>(SvStream& rIn, SetOfByte& rSet);

    // initialize this set with a uno sequence of sal_Int8
    void PutValue(const com::sun::star::uno::Any & rAny);

    // returns a uno sequence of sal_Int8
    void QueryValue(com::sun::star::uno::Any & rAny) const;
};

inline SvStream& operator<<(SvStream& rOut, const SetOfByte& rSet)
{
    rOut.Write((char*)rSet.aData,32);
    return rOut;
}

inline SvStream& operator>>(SvStream& rIn, SetOfByte& rSet)
{
    rIn.Read((char*)rSet.aData,32);
    return rIn;
}

#endif // _SVDSOB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
