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
#ifndef _SDTAITM_HXX
#define _SDTAITM_HXX

#include <bf_svtools/eitem.hxx>

#include <bf_svx/svddef.hxx>
namespace binfilter {

//------------------------------
// class SdrTextVertAdjustItem
//------------------------------

// Implementiert werden zunaechst nur TOP und CENTER, neustens auch BOTTOM
enum SdrTextVertAdjust {SDRTEXTVERTADJUST_TOP,      // Kopfbuendig (so wie man es normalerweise kennt)
                        SDRTEXTVERTADJUST_CENTER,   // Zentriert (z.B. fuer die Titeltexte im Draw)
                        SDRTEXTVERTADJUST_BOTTOM,   // Fussbuendig
                        SDRTEXTVERTADJUST_BLOCK    // #103516# support vertical full with supported now
                        /*,SDRTEXTVERTADJUST_STRETCH*/}; // Auch die Buchstaben in der Hoehe verzerren (ni)

class SdrTextVertAdjustItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrTextVertAdjustItem(SdrTextVertAdjust eAdj=SDRTEXTVERTADJUST_TOP): SfxEnumItem(SDRATTR_TEXT_VERTADJUST,eAdj) {}
    SdrTextVertAdjustItem(SvStream& rIn)                               : SfxEnumItem(SDRATTR_TEXT_VERTADJUST,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*      Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT            GetValueCount() const; // { return 5; }
            SdrTextVertAdjust GetValue() const      { return (SdrTextVertAdjust)SfxEnumItem::GetValue(); }

    virtual	bool              QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool              PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

};

//------------------------------
// class SdrTextHorzAdjustItem
//------------------------------

enum SdrTextHorzAdjust {SDRTEXTHORZADJUST_LEFT,     // Linksbuendig verankert
                        SDRTEXTHORZADJUST_CENTER,   // Zentriert (z.B. fuer die Titeltexte im Draw)
                        SDRTEXTHORZADJUST_RIGHT,    // Rechtsbuendig verankert
                        SDRTEXTHORZADJUST_BLOCK    // So wie es frueher war: Gesamte Textrahmenbreite verwenden, Absatzformatierung macht wieder Sinn
                        /*,SDRTEXTHORZADJUST_STRETCH*/}; // FitToSize in X-Richtung (ni).

class SdrTextHorzAdjustItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrTextHorzAdjustItem(SdrTextHorzAdjust eAdj=SDRTEXTHORZADJUST_BLOCK): SfxEnumItem(SDRATTR_TEXT_HORZADJUST,eAdj) {}
    SdrTextHorzAdjustItem(SvStream& rIn)                                 : SfxEnumItem(SDRATTR_TEXT_HORZADJUST,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*      Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT            GetValueCount() const; // { return 5; }
            SdrTextHorzAdjust GetValue() const      { return (SdrTextHorzAdjust)SfxEnumItem::GetValue(); }

    virtual	bool             QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool             PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual String  GetValueTextByPos(USHORT nPos) const;
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
