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
#ifndef _SVX_ZOOMSLIDERITEM_HXX
#define _SVX_ZOOMSLIDERITEM_HXX

#include <svl/intitem.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <com/sun/star/uno/Sequence.hxx>
#include "svx/svxdllapi.h"

//-------------------------------------------------------------------------

class SVX_DLLPUBLIC SvxZoomSliderItem: public SfxUInt16Item
{
    com::sun::star::uno::Sequence < sal_Int32 > maValues;
    sal_uInt16 mnMinZoom;
    sal_uInt16 mnMaxZoom;

public:
    TYPEINFO();

    SvxZoomSliderItem( sal_uInt16 nCurrentZoom = 100, sal_uInt16 nMinZoom = 20, sal_uInt16 nMaxZoom = 600, sal_uInt16 nWhich = SID_ATTR_ZOOMSLIDER );
    SvxZoomSliderItem( const SvxZoomSliderItem& );
    ~SvxZoomSliderItem();

    void AddSnappingPoint( sal_Int32 nNew );
    const com::sun::star::uno::Sequence < sal_Int32 >& GetSnappingPoints() const;
    sal_uInt16 GetMinZoom() const {return mnMinZoom;}
    sal_uInt16 GetMaxZoom() const {return mnMaxZoom;}

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStrm, sal_uInt16 nVersion ) const;                       // leer
    virtual SvStream&       Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const;                   // leer
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const; // leer
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );   // leer
};

//------------------------------------------------------------------------

#endif
