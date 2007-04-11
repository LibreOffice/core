/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdtaitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:13:48 $
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
#ifndef _SDTAITM_HXX
#define _SDTAITM_HXX

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <svx/svddef.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//------------------------------
// class SdrTextVertAdjustItem
//------------------------------

// Implementiert werden zunaechst nur TOP und CENTER, neustens auch BOTTOM
enum SdrTextVertAdjust {SDRTEXTVERTADJUST_TOP,      // Kopfbuendig (so wie man es normalerweise kennt)
                        SDRTEXTVERTADJUST_CENTER,   // Zentriert (z.B. fuer die Titeltexte im Draw)
                        SDRTEXTVERTADJUST_BOTTOM,   // Fussbuendig
                        SDRTEXTVERTADJUST_BLOCK    // #103516# support vertical full with supported now
                        /*,SDRTEXTVERTADJUST_STRETCH*/}; // Auch die Buchstaben in der Hoehe verzerren (ni)

class SVX_DLLPUBLIC SdrTextVertAdjustItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrTextVertAdjustItem(SdrTextVertAdjust eAdj=SDRTEXTVERTADJUST_TOP): SfxEnumItem(SDRATTR_TEXT_VERTADJUST,(USHORT)eAdj) {}
    SdrTextVertAdjustItem(SvStream& rIn)                               : SfxEnumItem(SDRATTR_TEXT_VERTADJUST,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*      Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT            GetValueCount() const; // { return 5; }
            SdrTextVertAdjust GetValue() const      { return (SdrTextVertAdjust)SfxEnumItem::GetValue(); }

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual String  GetValueTextByPos(USHORT nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

//------------------------------
// class SdrTextHorzAdjustItem
//------------------------------

enum SdrTextHorzAdjust {SDRTEXTHORZADJUST_LEFT,     // Linksbuendig verankert
                        SDRTEXTHORZADJUST_CENTER,   // Zentriert (z.B. fuer die Titeltexte im Draw)
                        SDRTEXTHORZADJUST_RIGHT,    // Rechtsbuendig verankert
                        SDRTEXTHORZADJUST_BLOCK    // So wie es frueher war: Gesamte Textrahmenbreite verwenden, Absatzformatierung macht wieder Sinn
                        /*,SDRTEXTHORZADJUST_STRETCH*/}; // FitToSize in X-Richtung (ni).

class SVX_DLLPUBLIC SdrTextHorzAdjustItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrTextHorzAdjustItem(SdrTextHorzAdjust eAdj=SDRTEXTHORZADJUST_BLOCK): SfxEnumItem(SDRATTR_TEXT_HORZADJUST,(USHORT)eAdj) {}
    SdrTextHorzAdjustItem(SvStream& rIn)                                 : SfxEnumItem(SDRATTR_TEXT_HORZADJUST,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*      Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT            GetValueCount() const; // { return 5; }
            SdrTextHorzAdjust GetValue() const      { return (SdrTextHorzAdjust)SfxEnumItem::GetValue(); }

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual String  GetValueTextByPos(USHORT nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

#endif
