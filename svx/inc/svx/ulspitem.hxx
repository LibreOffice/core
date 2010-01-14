/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ulspitem.hxx,v $
 * $Revision: 1.4 $
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
#ifndef _SVX_ULSPITEM_HXX
#define _SVX_ULSPITEM_HXX

// include ---------------------------------------------------------------

#include <svl/poolitem.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include "svx/svxdllapi.h"

namespace rtl
{
    class OUString;
}

// class SvxULSpaceItem --------------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt den oberen und unteren Rand einer Seite oder Absatz.
*/

#define ULSPACE_16_VERSION  ((USHORT)0x0001)

class SVX_DLLPUBLIC SvxULSpaceItem : public SfxPoolItem
{
    USHORT nUpper;  //Oberer Rand
    USHORT nLower;  //Unterer Rand
    USHORT nPropUpper, nPropLower;      // relativ oder absolut (=100%)
public:
    TYPEINFO();

    SvxULSpaceItem( const USHORT nId  );
    SvxULSpaceItem( const USHORT nUp, const USHORT nLow,
                    const USHORT nId  );
    inline SvxULSpaceItem& operator=( const SvxULSpaceItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion ) const;
    virtual USHORT           GetVersion( USHORT nFileVersion ) const;
    virtual int              ScaleMetrics( long nMult, long nDiv );
    virtual int              HasMetrics() const;

    inline void SetUpper( const USHORT nU, const USHORT nProp = 100 );
    inline void SetLower( const USHORT nL, const USHORT nProp = 100 );

    void SetUpperValue( const USHORT nU ) { nUpper = nU; }
    void SetLowerValue( const USHORT nL ) { nLower = nL; }
    void SetPropUpper( const USHORT nU ) { nPropUpper = nU; }
    void SetPropLower( const USHORT nL ) { nPropLower = nL; }

    USHORT GetUpper() const { return nUpper; }
    USHORT GetLower() const { return nLower; }
    USHORT GetPropUpper() const { return nPropUpper; }
    USHORT GetPropLower() const { return nPropLower; }
};

inline SvxULSpaceItem &SvxULSpaceItem::operator=( const SvxULSpaceItem &rCpy )
{
    nUpper = rCpy.GetUpper();
    nLower = rCpy.GetLower();
    nPropUpper = rCpy.GetPropUpper();
    nPropLower = rCpy.GetPropLower();
    return *this;
}

inline void SvxULSpaceItem::SetUpper( const USHORT nU, const USHORT nProp )
{
    nUpper = USHORT((ULONG(nU) * nProp ) / 100); nPropUpper = nProp;
}
inline void SvxULSpaceItem::SetLower( const USHORT nL, const USHORT nProp )
{
    nLower = USHORT((ULONG(nL) * nProp ) / 100); nPropLower = nProp;
}

#endif

