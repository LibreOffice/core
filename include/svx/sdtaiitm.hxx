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
#ifndef INCLUDED_SVX_SDTAIITM_HXX
#define INCLUDED_SVX_SDTAIITM_HXX

#include <svx/sdynitm.hxx>
#include <svx/svddef.hxx>

class SVXCORE_DLLPUBLIC SdrTextAniStartInsideItem final : public SdrYesNoItem {
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrTextAniStartInsideItem)
    SdrTextAniStartInsideItem(bool bOn=false): SdrYesNoItem(SDRATTR_TEXT_ANISTARTINSIDE,bOn) {}
    virtual ~SdrTextAniStartInsideItem() override;
    virtual SdrTextAniStartInsideItem* Clone(SfxItemPool* pPool=nullptr) const override;

    SdrTextAniStartInsideItem(SdrTextAniStartInsideItem const &) = default;
    SdrTextAniStartInsideItem(SdrTextAniStartInsideItem &&) = default;
    SdrTextAniStartInsideItem & operator =(SdrTextAniStartInsideItem const &) = delete; // due to SdrYesNoItem
    SdrTextAniStartInsideItem & operator =(SdrTextAniStartInsideItem &&) = delete; // due to SdrYesNoItem
};

class SVXCORE_DLLPUBLIC SdrTextAniStopInsideItem final : public SdrYesNoItem {
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrTextAniStopInsideItem)
    SdrTextAniStopInsideItem(bool bOn=false): SdrYesNoItem(SDRATTR_TEXT_ANISTOPINSIDE,bOn) {}
    virtual ~SdrTextAniStopInsideItem() override;
    virtual SdrTextAniStopInsideItem* Clone(SfxItemPool* pPool=nullptr) const override;

    SdrTextAniStopInsideItem(SdrTextAniStopInsideItem const &) = default;
    SdrTextAniStopInsideItem(SdrTextAniStopInsideItem &&) = default;
    SdrTextAniStopInsideItem & operator =(SdrTextAniStopInsideItem const &) = delete; // due to SdrYesNoItem
    SdrTextAniStopInsideItem & operator =(SdrTextAniStopInsideItem &&) = delete; // due to SdrYesNoItem
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
