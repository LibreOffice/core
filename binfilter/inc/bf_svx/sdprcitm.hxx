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
#ifndef _SDPRCITM_HXX
#define _SDPRCITM_HXX

#ifndef _SFXINTITEM_HXX //autogen
#include <bf_svtools/intitem.hxx>
#endif
namespace binfilter {

//------------------------------------------------------------
// class SdrPercentItem
// Ganze Prozente von 0
//------------------------------------------------------------

class SdrPercentItem : public SfxUInt16Item
{
public:
    TYPEINFO();
    SdrPercentItem(): SfxUInt16Item() {}
    SdrPercentItem(USHORT nId, UINT16 nVal=0): SfxUInt16Item(nId,nVal) {}
    SdrPercentItem(USHORT nId, SvStream& rIn):  SfxUInt16Item(nId,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem* Create(SvStream& rIn, USHORT nVer) const;


#ifdef SDR_ISPOOLABLE
    virtual int IsPoolable() const;
#endif
};

//------------------------------------------------------------
// class SdrSignedPercentItem
// Ganze Prozente von +/-
//------------------------------------------------------------

class SdrSignedPercentItem : public SfxInt16Item
{
public:
    TYPEINFO();
    SdrSignedPercentItem(): SfxInt16Item() {}
    SdrSignedPercentItem( USHORT nId, INT16 nVal = 0 ) : SfxInt16Item( nId,nVal ) {}
    SdrSignedPercentItem( USHORT nId, SvStream& rIn ) : SfxInt16Item( nId,rIn ) {}


#ifdef SDR_ISPOOLABLE
    virtual int IsPoolable() const;
#endif
};

}//end of namespace binfilter
#endif
