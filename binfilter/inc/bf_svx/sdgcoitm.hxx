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

#ifndef _SDGCOITM_HXX
#define _SDGCOITM_HXX

#ifndef _SDPRCITM_HXX
#include <bf_svx/sdprcitm.hxx>
#endif
#ifndef _SVDDEF_HXX
#include <bf_svx/svddef.hxx>
#endif
namespace binfilter {

//-----------------
// SdrGrafRedItem -
//-----------------

class SdrGrafRedItem : public SdrSignedPercentItem
{
public:

                            TYPEINFO();

                            SdrGrafRedItem( short nRedPercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFRED, nRedPercent ) {}
                            SdrGrafRedItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFRED, rIn ) {}

    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*	Create( SvStream& rIn, USHORT nVer ) const;
};

//-------------------
// SdrGrafGreenItem -
//-------------------

class SdrGrafGreenItem : public SdrSignedPercentItem
{
public:

                            TYPEINFO();

                            SdrGrafGreenItem( short nGreenPercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFGREEN, nGreenPercent ) {}
                            SdrGrafGreenItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFGREEN, rIn ) {}

    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*	Create( SvStream& rIn, USHORT nVer ) const;
};

//-------------------
// SdrGrafBlueItem -
//-------------------

class SdrGrafBlueItem : public SdrSignedPercentItem
{
public:

                            TYPEINFO();

                            SdrGrafBlueItem( short nBluePercent = 0 ) : SdrSignedPercentItem( SDRATTR_GRAFBLUE, nBluePercent ) {}
                            SdrGrafBlueItem( SvStream& rIn ) : SdrSignedPercentItem( SDRATTR_GRAFBLUE, rIn ) {}

    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*	Create( SvStream& rIn, USHORT nVer ) const;
};

}//end of namespace binfilter
#endif // _SDGCOITM_HXX
