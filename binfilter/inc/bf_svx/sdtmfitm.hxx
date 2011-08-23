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
#ifndef _SDTMFITM_HXX
#define _SDTMFITM_HXX

#include <bf_svx/sdmetitm.hxx>

#include <bf_svx/svddef.hxx>
namespace binfilter {

//------------------------------
// class SdrTextMinFrameHeighItem
//------------------------------
class SdrTextMinFrameHeightItem: public SdrMetricItem {
public:
    SdrTextMinFrameHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT,nHgt)  {}
    SdrTextMinFrameHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT,rIn) {}
};

//------------------------------
// class SdrTextMaxFrameHeightItem
//------------------------------
class SdrTextMaxFrameHeightItem: public SdrMetricItem {
public:
    SdrTextMaxFrameHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEHEIGHT,nHgt)  {}
    SdrTextMaxFrameHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEHEIGHT,rIn) {}
};

//------------------------------
// class SdrTextMinFrameWidthItem
//------------------------------
class SdrTextMinFrameWidthItem: public SdrMetricItem {
public:
    SdrTextMinFrameWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH,nWdt)  {}
    SdrTextMinFrameWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH,rIn) {}
};

//------------------------------
// class SdrTextMaxFrameWidthItem
//------------------------------
class SdrTextMaxFrameWidthItem: public SdrMetricItem {
public:
    SdrTextMaxFrameWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEWIDTH,nWdt)  {}
    SdrTextMaxFrameWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEWIDTH,rIn) {}
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
