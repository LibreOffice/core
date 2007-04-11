/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxallitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:31:32 $
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
#ifndef _SXALLITM_HXX
#define _SXALLITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <svx/sdmetitm.hxx>
#endif

//------------------------------
// class SdrAllPositionXItem
//------------------------------
class SdrAllPositionXItem: public SdrMetricItem {
public:
    SdrAllPositionXItem(long nPosX=0): SdrMetricItem(SDRATTR_ALLPOSITIONX,nPosX) {}
    SdrAllPositionXItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLPOSITIONX,rIn)  {}
};

//------------------------------
// class SdrAllPositionYItem
//------------------------------
class SdrAllPositionYItem: public SdrMetricItem {
public:
    SdrAllPositionYItem(long nPosY=0): SdrMetricItem(SDRATTR_ALLPOSITIONY,nPosY) {}
    SdrAllPositionYItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLPOSITIONY,rIn)  {}
};

//------------------------------
// class SdrAllSizeWidthItem
//------------------------------
class SdrAllSizeWidthItem: public SdrMetricItem {
public:
    SdrAllSizeWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_ALLSIZEWIDTH,nWdt)  {}
    SdrAllSizeWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLSIZEWIDTH,rIn) {}
};

//------------------------------
// class SdrAllSizeWidthItem
//------------------------------
class SdrAllSizeHeightItem: public SdrMetricItem {
public:
    SdrAllSizeHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_ALLSIZEHEIGHT,nHgt)  {}
    SdrAllSizeHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLSIZEHEIGHT,rIn) {}
};

#endif
