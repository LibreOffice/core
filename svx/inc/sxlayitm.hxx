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
#ifndef INCLUDED_SVX_INC_SXLAYITM_HXX
#define INCLUDED_SVX_INC_SXLAYITM_HXX

#include <svx/svddef.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svx/svdtypes.hxx>

class SdrLayerIdItem final : public SfxInt16Item {
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrLayerIdItem)
    SdrLayerIdItem(SdrLayerID nId):
        SfxInt16Item(SDRATTR_LAYERID,nId.get())  {}
    SdrLayerID GetValue() const { return SdrLayerID(SfxInt16Item::GetValue()); }
    virtual SdrLayerIdItem* Clone(SfxItemPool* pPool=nullptr) const override;
};

class SdrLayerNameItem final : public SfxStringItem {
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrLayerNameItem)
    SdrLayerNameItem()
        : SfxStringItem(SDRATTR_LAYERNAME) {}
    SdrLayerNameItem(const OUString& rStr)
        : SfxStringItem(SDRATTR_LAYERNAME, rStr) {}
    virtual SdrLayerNameItem* Clone(SfxItemPool* pPool=nullptr) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
