/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SXELDITM_HXX
#define _SXELDITM_HXX

#include <bf_svx/svddef.hxx>

#include <bf_svx/sdmetitm.hxx>
namespace binfilter {

class SdrEdgeLineDeltaAnzItem: public SfxUInt16Item {
public:
    SdrEdgeLineDeltaAnzItem(UINT16 nVal=0): SfxUInt16Item(SDRATTR_EDGELINEDELTAANZ,nVal) {}
    SdrEdgeLineDeltaAnzItem(SvStream& rIn): SfxUInt16Item(SDRATTR_EDGELINEDELTAANZ,rIn)  {}
};

class SdrEdgeLine1DeltaItem: public SdrMetricItem {
public:
    SdrEdgeLine1DeltaItem(long nVal=0): SdrMetricItem(SDRATTR_EDGELINE1DELTA,nVal)  {}
    SdrEdgeLine1DeltaItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGELINE1DELTA,rIn) {}
};

class SdrEdgeLine2DeltaItem: public SdrMetricItem {
public:
    SdrEdgeLine2DeltaItem(long nVal=0): SdrMetricItem(SDRATTR_EDGELINE2DELTA,nVal)  {}
    SdrEdgeLine2DeltaItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGELINE2DELTA,rIn) {}
};

class SdrEdgeLine3DeltaItem: public SdrMetricItem {
public:
    SdrEdgeLine3DeltaItem(long nVal=0): SdrMetricItem(SDRATTR_EDGELINE3DELTA,nVal)  {}
    SdrEdgeLine3DeltaItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGELINE3DELTA,rIn) {}
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
