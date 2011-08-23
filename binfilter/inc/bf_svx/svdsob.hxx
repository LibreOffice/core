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

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_ 
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _STRING_H
#define _STRING_H
#endif
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  Deklaration eines statischen Mengentyps. Die Menge kann die Elemente
  0..255 aufnehmen und verbraucht stets 32 Bytes.
*/

class SetOfByte {
protected:
    BYTE aData[32];
public:
    SetOfByte(FASTBOOL bInitVal=FALSE)                  { memset(aData,bInitVal ? 0xFF : 0x00,sizeof(aData)); }
    FASTBOOL operator==(const SetOfByte& rCmpSet) const { return (memcmp(aData,rCmpSet.aData,sizeof(aData))==0); }
    FASTBOOL operator!=(const SetOfByte& rCmpSet) const { return (memcmp(aData,rCmpSet.aData,sizeof(aData))!=0); }
    void     Set  (BYTE a)                              { aData[a/8]|=1<<a%8; }
    void     Clear(BYTE a)                              { aData[a/8]&=~(1<<a%8); }
    void     Set  (BYTE a, FASTBOOL b)                  { if (b) Set(a); else Clear(a); }
    FASTBOOL IsSet(BYTE a) const                        { return (aData[a/8]&1<<a%8)!=0; }
    FASTBOOL IsSet(USHORT a) const                      { return (a<=255) && (aData[a/8]&1<<a%8)!=0; }
    void     SetAll()                                   { memset(aData,0xFF,sizeof(aData)); }
    void     ClearAll()                                 { memset(aData,0x00,sizeof(aData)); }
    FASTBOOL IsEmpty() const;
    void     operator&=(const SetOfByte& r2ndSet);
    friend inline SvStream& operator<<(SvStream& rOut, const SetOfByte& rSet);
    friend inline SvStream& operator>>(SvStream& rIn, SetOfByte& rSet);

     void	PutValue( const ::com::sun::star::uno::Any & rAny );		// initialize this set with a uno sequence of sal_Int8
    void	QueryValue( ::com::sun::star::uno::Any & rAny ) const;	// returns a uno sequence of sal_Int8
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

}//end of namespace binfilter
#endif // _SVDSOB_HXX

