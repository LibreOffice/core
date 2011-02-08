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
#ifndef _SDTAGITM_HXX
#define _SDTAGITM_HXX

#include <svx/sdooitm.hxx>
#include <svx/svddef.hxx>

//------------------------------------
// class SdrTextAutoGrowHeightItem
//------------------------------------
class SdrTextAutoGrowHeightItem: public SdrOnOffItem {
public:
    SdrTextAutoGrowHeightItem(sal_Bool bAuto=sal_True): SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT,bAuto) {}
    SdrTextAutoGrowHeightItem(SvStream& rIn)  : SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT,rIn)   {}
};

//------------------------------------
// class SdrTextAutoGrowWidthItem
//------------------------------------
class SdrTextAutoGrowWidthItem: public SdrOnOffItem {
public:
    SdrTextAutoGrowWidthItem(sal_Bool bAuto=sal_False): SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH,bAuto) {}
    SdrTextAutoGrowWidthItem(SvStream& rIn)   : SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH,rIn)   {}
};

#endif
