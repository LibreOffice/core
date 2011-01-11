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
#ifndef _SVX_ZOOMITEM_HXX
#define _SVX_ZOOMITEM_HXX

#include <svl/intitem.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include "svx/svxdllapi.h"

//-------------------------------------------------------------------------

enum SvxZoomType
{
    SVX_ZOOM_PERCENT,       // GetValue() ist kein besonderer prozentualer Wert
    SVX_ZOOM_OPTIMAL,       // GetValue() entspricht der optimalen Gr"o\se
    SVX_ZOOM_WHOLEPAGE,     // GetValue() entspricht der ganzen Seite
    SVX_ZOOM_PAGEWIDTH,      // GetValue() entspricht der Seitenbreite
    SVX_ZOOM_PAGEWIDTH_NOBORDER  // GetValue() pagewidth without border
};

//-------------------------------------------------------------------------

class SVX_DLLPUBLIC SvxZoomItem: public SfxUInt16Item
{
    sal_uInt16                  nValueSet;  // erlaubte Werte (siehe #defines unten)
    SvxZoomType             eType;

public:
    TYPEINFO();

    SvxZoomItem( SvxZoomType eZoomType = SVX_ZOOM_PERCENT,
                 sal_uInt16 nVal = 0, sal_uInt16 nWhich = SID_ATTR_ZOOM );
    SvxZoomItem( const SvxZoomItem& );
    ~SvxZoomItem();

    void                    SetValueSet( sal_uInt16 nValues ) { nValueSet = nValues; }
    sal_uInt16                  GetValueSet() const { return nValueSet; }
    FASTBOOL                IsValueAllowed( sal_uInt16 nValue ) const
                            { return nValue == ( nValue & nValueSet ); }

    SvxZoomType             GetType() const { return eType; }
    void                    SetType( SvxZoomType eNewType )
                            { eType = eNewType; }

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStrm, sal_uInt16 nVersion ) const;
    virtual SvStream&       Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

//------------------------------------------------------------------------

#define SVX_ZOOM_ENABLE_50          0x0001
#define SVX_ZOOM_ENABLE_75          0x0002
#define SVX_ZOOM_ENABLE_100         0x0004
#define SVX_ZOOM_ENABLE_150         0x0008
#define SVX_ZOOM_ENABLE_200         0x0010
#define SVX_ZOOM_ENABLE_OPTIMAL     0x1000
#define SVX_ZOOM_ENABLE_WHOLEPAGE   0x2000
#define SVX_ZOOM_ENABLE_PAGEWIDTH   0x4000
#define SVX_ZOOM_ENABLE_ALL         0x701F

#endif
