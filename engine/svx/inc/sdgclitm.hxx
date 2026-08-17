/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <svl/poolitem.hxx>
#include <svx/svddef.hxx>

class SdrGrafClipPolyPolygonItem final : public SfxPoolItem
{
    basegfx::B2DPolyPolygon maPolyPolygon;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrGrafClipPolyPolygonItem)
    SdrGrafClipPolyPolygonItem();
    explicit SdrGrafClipPolyPolygonItem(basegfx::B2DPolyPolygon aPolyPolygon);

    virtual bool operator==(const SfxPoolItem& rItem) const override;
    virtual SdrGrafClipPolyPolygonItem* Clone(SfxItemPool* pPool = nullptr) const override;

    virtual bool QueryValue(cpo::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const cpo::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;

    const basegfx::B2DPolyPolygon& GetValue() const { return maPolyPolygon; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
