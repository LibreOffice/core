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
#ifndef _SXONEITM_HXX
#define _SXONEITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <bf_svx/sdmetitm.hxx>
#endif
namespace binfilter {

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

}//end of namespace binfilter
#endif
