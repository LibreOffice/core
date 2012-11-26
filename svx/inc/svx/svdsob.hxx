/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVDSOB_HXX
#define _SVDSOB_HXX

#include <com/sun/star/uno/Any.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx> //wg. memset
#include "svx/svxdllapi.h"
#include <string.h>

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
    SetOfByte(bool bInitVal = false)
    {
        memset(aData, bInitVal ? 0xFF : 0x00, sizeof(aData));
    }

    bool operator==(const SetOfByte& rCmpSet) const
    {
        return (memcmp(aData, rCmpSet.aData, sizeof(aData)) == 0);
    }

    bool operator!=(const SetOfByte& rCmpSet) const
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

    void Set(sal_uInt8 a, bool b)
    {
        if(b)
        {
            Set(a);
        }
        else
        {
            Clear(a);
    }
    }

    bool IsSet(sal_uInt8 a) const
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

    bool IsEmpty() const;
    bool IsFull() const;

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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // _SVDSOB_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
