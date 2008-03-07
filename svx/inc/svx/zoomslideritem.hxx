/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoomslideritem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:15:06 $
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
#ifndef _SVX_ZOOMSLIDERITEM_HXX
#define _SVX_ZOOMSLIDERITEM_HXX

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//-------------------------------------------------------------------------

class SVX_DLLPUBLIC SvxZoomSliderItem: public SfxUInt16Item
{
    com::sun::star::uno::Sequence < sal_Int32 > maValues;
    USHORT mnMinZoom;
    USHORT mnMaxZoom;

public:
    TYPEINFO();

    SvxZoomSliderItem( USHORT nCurrentZoom = 100, USHORT nMinZoom = 20, USHORT nMaxZoom = 600, USHORT nWhich = SID_ATTR_ZOOMSLIDER );
    SvxZoomSliderItem( const SvxZoomSliderItem& );
    ~SvxZoomSliderItem();

    void AddSnappingPoint( sal_Int32 nNew );
    const com::sun::star::uno::Sequence < sal_Int32 >& GetSnappingPoints() const;
    USHORT GetMinZoom() const {return mnMinZoom;}
    USHORT GetMaxZoom() const {return mnMaxZoom;}

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStrm, USHORT nVersion ) const;                       // leer
    virtual SvStream&       Store( SvStream& rStrm , USHORT nItemVersion ) const;                   // leer
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const; // leer
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );   // leer
};

//------------------------------------------------------------------------

#endif
