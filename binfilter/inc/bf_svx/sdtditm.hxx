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
#ifndef SDTMITM_HXX
#define SDTMITM_HXX

#include <bf_svx/sdmetitm.hxx>

#include <bf_svx/svddef.hxx>
namespace binfilter {

//----------------------------------------------
// class SdrTextLeft/Right/Upper/LowerDistItem
//----------------------------------------------
class SdrTextLeftDistItem: public SdrMetricItem {
public:
    SdrTextLeftDistItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_LEFTDIST,nHgt)  {}
    SdrTextLeftDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_LEFTDIST,rIn) {}
};

class SdrTextRightDistItem: public SdrMetricItem {
public:
    SdrTextRightDistItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_RIGHTDIST,nHgt)  {}
    SdrTextRightDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_RIGHTDIST,rIn) {}
};

class SdrTextUpperDistItem: public SdrMetricItem {
public:
    SdrTextUpperDistItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_UPPERDIST,nHgt)  {}
    SdrTextUpperDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_UPPERDIST,rIn) {}
};

class SdrTextLowerDistItem: public SdrMetricItem {
public:
    SdrTextLowerDistItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_LOWERDIST,nHgt)  {}
    SdrTextLowerDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_LOWERDIST,rIn) {}
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
