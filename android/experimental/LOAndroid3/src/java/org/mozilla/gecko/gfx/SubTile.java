/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

public class SubTile extends SingleTileLayer {
    public boolean markedForRemoval = false;
    public final TileIdentifier id;

    public SubTile(CairoImage mImage, int x, int y, float zoom) {
        super(mImage);
        id = new TileIdentifier(x, y, zoom);
    }

    public void markForRemoval() {
        markedForRemoval = true;
    }

    public static class TileIdentifier {
        public int x;
        public int y;
        public float zoom;

        public TileIdentifier(int x, int y, float zoom) {
            this.x = x;
            this.y = y;
            this.zoom = zoom;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;

            TileIdentifier that = (TileIdentifier) o;

            if (x != that.x) return false;
            if (y != that.y) return false;
            if (Float.compare(that.zoom, zoom) != 0) return false;

            return true;
        }

        @Override
        public int hashCode() {
            int result = x;
            result = 31 * result + y;
            result = 31 * result + (zoom != +0.0f ? Float.floatToIntBits(zoom) : 0);
            return result;
        }
    }
}
