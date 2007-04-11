/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxoneitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:38:14 $
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
#ifndef _SXONEITM_HXX
#define _SXONEITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <svx/sdmetitm.hxx>
#endif

//------------------------------
// class SdrOnePositionXItem
//------------------------------
class SdrOnePositionXItem: public SdrMetricItem {
public:
    SdrOnePositionXItem(long nPosX=0): SdrMetricItem(SDRATTR_ONEPOSITIONX,nPosX) {}
    SdrOnePositionXItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONEPOSITIONX,rIn)  {}
};

//------------------------------
// class SdrOnePositionYItem
//------------------------------
class SdrOnePositionYItem: public SdrMetricItem {
public:
    SdrOnePositionYItem(long nPosY=0): SdrMetricItem(SDRATTR_ONEPOSITIONY,nPosY) {}
    SdrOnePositionYItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONEPOSITIONY,rIn)  {}
};

//------------------------------
// class SdrOneSizeWidthItem
//------------------------------
class SdrOneSizeWidthItem: public SdrMetricItem {
public:
    SdrOneSizeWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_ONESIZEWIDTH,nWdt)  {}
    SdrOneSizeWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONESIZEWIDTH,rIn) {}
};

//------------------------------
// class SdrOneSizeWidthItem
//------------------------------
class SdrOneSizeHeightItem: public SdrMetricItem {
public:
    SdrOneSizeHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_ONESIZEHEIGHT,nHgt)  {}
    SdrOneSizeHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONESIZEHEIGHT,rIn) {}
};

#endif
