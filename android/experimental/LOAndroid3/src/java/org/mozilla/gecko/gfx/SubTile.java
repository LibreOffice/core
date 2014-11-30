/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import org.libreoffice.TileIdentifier;

public class SubTile extends SingleTileLayer {
    public boolean markedForRemoval = false;
    public final TileIdentifier id;

    public SubTile(CairoImage mImage, TileIdentifier id) {
        super(mImage);
        this.id = id;
    }

    public void markForRemoval() {
        markedForRemoval = true;
    }
}
