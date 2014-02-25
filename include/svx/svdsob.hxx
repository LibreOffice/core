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

#ifndef INCLUDED_SVX_SVDSOB_HXX
#define INCLUDED_SVX_SVDSOB_HXX

#include <com/sun/star/uno/Any.hxx>
#include <tools/stream.hxx>

#include <svx/svxdllapi.h>


/*
  Deklaration eines statischen Mengentyps. Die Menge kann die Elemente
  0..255 aufnehmen und verbraucht stets 32 Bytes.
*/

class SVX_DLLPUBLIC SetOfByte
{
protected:
    sal_uInt8 aData[32];

public:
    explicit SetOfByte(sal_Bool bInitVal = sal_False)
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

    void operator&=(const SetOfByte& r2ndSet);
    void operator|=(const SetOfByte& r2ndSet);

    friend inline SvStream& WriteSetOfByte(SvStream& rOut, const SetOfByte& rSet);
    friend inline SvStream& ReadSetOfByte(SvStream& rIn, SetOfByte& rSet);

    // initialize this set with a uno sequence of sal_Int8
    void PutValue(const com::sun::star::uno::Any & rAny);

    // returns a uno sequence of sal_Int8
    void QueryValue(com::sun::star::uno::Any & rAny) const;
};

inline SvStream& WriteSetOfByte(SvStream& rOut, const SetOfByte& rSet)
{
    rOut.Write((char*)rSet.aData,32);
    return rOut;
}

inline SvStream& ReadSetOfByte(SvStream& rIn, SetOfByte& rSet)
{
    rIn.Read((char*)rSet.aData,32);
    return rIn;
}

#endif // INCLUDED_SVX_SVDSOB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
