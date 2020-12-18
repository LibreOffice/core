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

#include <vcl/BitmapInfoAccess.hxx>

#include <salbmp.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

BitmapInfoAccess::BitmapInfoAccess(Bitmap& rBitmap, BitmapAccessMode nMode)
    : mpBuffer(nullptr)
    , mnAccessMode(nMode)
{
    std::shared_ptr<SalBitmap> xImpBmp = rBitmap.ImplGetSalBitmap();

    if (!xImpBmp)
        return;

    if (mnAccessMode == BitmapAccessMode::Write)
    {
        xImpBmp->DropScaledCache();

        if (xImpBmp.use_count() > 2)
        {
            xImpBmp.reset();
            rBitmap.ImplMakeUnique();
            xImpBmp = rBitmap.ImplGetSalBitmap();
        }
    }

    mpBuffer = xImpBmp->AcquireBuffer(mnAccessMode);

    if (!mpBuffer)
    {
        std::shared_ptr<SalBitmap> xNewImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
        if (xNewImpBmp->Create(*xImpBmp, rBitmap.GetBitCount()))
        {
            xImpBmp = xNewImpBmp;
            rBitmap.ImplSetSalBitmap(xImpBmp);
            mpBuffer = xImpBmp->AcquireBuffer(mnAccessMode);
        }
    }

    maBitmap = rBitmap;
}

BitmapInfoAccess::~BitmapInfoAccess()
{
    std::shared_ptr<SalBitmap> xImpBmp = maBitmap.ImplGetSalBitmap();

    if (mpBuffer && xImpBmp)
    {
        xImpBmp->ReleaseBuffer(mpBuffer, mnAccessMode);
    }
}

sal_uInt16 BitmapInfoAccess::GetBestPaletteIndex(const BitmapColor& rBitmapColor) const
{
    return (HasPalette() ? mpBuffer->maPalette.GetBestIndex(rBitmapColor) : 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
