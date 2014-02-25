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
#ifndef INCLUDED_SVX_SDTDITM_HXX
#define INCLUDED_SVX_SDTDITM_HXX

#include <svx/sdmetitm.hxx>
#include <svx/svddef.hxx>


// class SdrTextLeft/Right/Upper/LowerDistItem

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
