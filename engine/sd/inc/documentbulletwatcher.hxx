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

#include <sal/types.h>
#include <svl/lstner.hxx>

class SdDrawDocument;
class SfxStyleSheetBasePool;

/// Drops the bullets a presentation remembers from its style sheets when one of those style
/// sheets changes. A style sheet announces a change to the listeners of that sheet, so every
/// sheet in the pool is listened to, and the pool itself for the sheets it gains and loses.
class SdStyleSheetBulletWatcher final : public SfxListener
{
public:
    explicit SdStyleSheetBulletWatcher(const SdDrawDocument& rDocument);

    /** Listens to the pool and to every style sheet it holds, and returns whether that listening
        was set up afresh. A pool that holds the same number of sheets as the last time is already
        listened to whole, so it is left alone.
     */
    bool Watch(SfxStyleSheetBasePool& rPool);

    virtual void Notify(SfxBroadcaster& rBroadcaster, const SfxHint& rHint) override;

private:
    const SdDrawDocument& mrDocument;
    /// How many style sheets the pool held when the listening was last set up, or -1 for none.
    sal_Int32 mnWatchedSheets = -1;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
