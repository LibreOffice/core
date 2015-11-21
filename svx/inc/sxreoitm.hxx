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
#ifndef INCLUDED_SVX_INC_SXREOITM_HXX
#define INCLUDED_SVX_INC_SXREOITM_HXX

#include <svx/svddef.hxx>

#include <svx/sxfiitm.hxx>

class SdrResizeXOneItem: public SdrFractionItem {
public:
    SdrResizeXOneItem(): SdrFractionItem(SDRATTR_RESIZEXONE,Fraction(1,1)) {}
    SdrResizeXOneItem(const Fraction& rFact): SdrFractionItem(SDRATTR_RESIZEXONE,rFact) {}
    SdrResizeXOneItem(SvStream& rIn): SdrFractionItem(SDRATTR_RESIZEXONE,rIn)    {}
    SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrResizeXOneItem(*this);
    }
};

class SdrResizeYOneItem: public SdrFractionItem {
public:
    SdrResizeYOneItem(): SdrFractionItem(SDRATTR_RESIZEYONE,Fraction(1,1)) {}
    SdrResizeYOneItem(const Fraction& rFact): SdrFractionItem(SDRATTR_RESIZEYONE,rFact) {}
    SdrResizeYOneItem(SvStream& rIn): SdrFractionItem(SDRATTR_RESIZEYONE,rIn)    {}
    SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrResizeYOneItem(*this);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
