/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdgluitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:09:12 $
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

#ifndef _SDGLUITM_HXX
#define _SDGLUITM_HXX

#ifndef _SDPRCITM_HXX
#include <svx/sdprcitm.hxx>
#endif
#ifndef _SVDDEF_HXX
#include <svx/svddef.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//-----------------------
// SdrGrafLuminanceItem -
//-----------------------

class SVX_DLLPUBLIC SdrGrafLuminanceItem : public SdrSignedPercentItem
{
public:

                            TYPEINFO();

                            SdrGrafLuminanceItem( short nLuminancePercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFLUMINANCE, nLuminancePercent ) {}
                            SdrGrafLuminanceItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFLUMINANCE, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, USHORT nVer ) const;
};

//----------------------
// SdrGrafContrastItem -
//----------------------

class SVX_DLLPUBLIC SdrGrafContrastItem : public SdrSignedPercentItem
{
public:

                            TYPEINFO();

                            SdrGrafContrastItem( short nContrastPercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFCONTRAST, nContrastPercent ) {}
                            SdrGrafContrastItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFCONTRAST, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, USHORT nVer ) const;
};

#endif // _SDGLUITM_HXX
