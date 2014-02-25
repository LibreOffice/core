/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_SDTMFITM_HXX
#define INCLUDED_SVX_SDTMFITM_HXX

#include <svx/sdmetitm.hxx>
#include <svx/svddef.hxx>


// class SdrTextMinFrameHeighItem

class SdrTextMinFrameHeightItem: public SdrMetricItem {
public:
    SdrTextMinFrameHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT,nHgt)  {}
    SdrTextMinFrameHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT,rIn) {}
};


// class SdrTextMaxFrameHeightItem

class SdrTextMaxFrameHeightItem: public SdrMetricItem {
public:
    SdrTextMaxFrameHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEHEIGHT,nHgt)  {}
    SdrTextMaxFrameHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEHEIGHT,rIn) {}
};


// class SdrTextMinFrameWidthItem

class SdrTextMinFrameWidthItem: public SdrMetricItem {
public:
    SdrTextMinFrameWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH,nWdt)  {}
    SdrTextMinFrameWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH,rIn) {}
};


// class SdrTextMaxFrameWidthItem

class SdrTextMaxFrameWidthItem: public SdrMetricItem {
public:
    SdrTextMaxFrameWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEWIDTH,nWdt)  {}
    SdrTextMaxFrameWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_TEXT_MAXFRAMEWIDTH,rIn) {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
