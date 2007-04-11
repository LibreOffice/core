/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xflboxy.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:51:52 $
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

#ifndef _SVX_XFLBOXY_HXX
#define _SVX_XFLBOXY_HXX

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

/*************************************************************************
|*
|*
|*
\************************************************************************/

class XFillBmpPosOffsetXItem : public SfxUInt16Item
{
public:
                            TYPEINFO();
                            XFillBmpPosOffsetXItem( USHORT nOffPosX = 0 );
                            XFillBmpPosOffsetXItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, USHORT nVer ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
};


/*************************************************************************
|*
|*
|*
\************************************************************************/

class XFillBmpPosOffsetYItem : public SfxUInt16Item
{
public:
                            TYPEINFO();
                            XFillBmpPosOffsetYItem( USHORT nOffPosY = 0 );
                            XFillBmpPosOffsetYItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, USHORT nVer ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
};

#endif
