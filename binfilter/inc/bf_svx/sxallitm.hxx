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
#ifndef _SXALLITM_HXX
#define _SXALLITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <bf_svx/sdmetitm.hxx>
#endif
namespace binfilter {

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

}//end of namespace binfilter
#endif
