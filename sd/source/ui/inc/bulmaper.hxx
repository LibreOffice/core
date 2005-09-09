/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bulmaper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:22:54 $
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

#ifndef INCLUDED_SD_BULMAPER_HXX
#define INCLUDED_SD_BULMAPER_HXX

#ifndef _SOLAR_H
#include "tools/solar.h"
#endif

class SfxItemSet;
class SvxBulletItem;
class SvxNumberFormat;
class SvxNumRule;

class SdBulletMapper
{
public:
/*
    static SvxNumberFormat MapBulletToNumBulletItem( const SvxBulletItem& aBullet );
    static void MapNumBulletToBulletItem( const SvxNumberFormat& aNumBullet, SvxBulletItem& aBullet, SfxItemSet& rSet );
    static void MapNumBulletToBulletItem( const SvxNumRule& aNumRule, USHORT nLevel, SvxBulletItem& aBullet, SfxItemSet& rSet );
*/

    static void PreMapNumBulletForDialog( SfxItemSet& rSet );
    static void PostMapNumBulletForDialog( SfxItemSet& rSet );

    static void MapFontsInNumRule( SvxNumRule& aNumRule, const SfxItemSet& rSet );

};

#endif /* INCLUDED_SD_BULMAPER_HXX */
