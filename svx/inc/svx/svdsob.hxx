/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdsob.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:27:58 $
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

#ifndef _SVDSOB_HXX
#define _SVDSOB_HXX

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _STRING_H
#include <tools/string.hxx> //wg. memset
#define _STRING_H
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

