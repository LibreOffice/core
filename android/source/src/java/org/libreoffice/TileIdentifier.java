/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.graphics.Rect;
import android.graphics.RectF;

import org.mozilla.gecko.gfx.IntSize;

/**
 * Identifies the tile by its position (x and y coordinate on the document), zoom and tile size (currently static)
 */
public class TileIdentifier {
    public final int x;
    public final int y;
    public final float zoom;
    public final IntSize size;

    public TileIdentifier(int x, int y, float zoom, IntSize size) {
        this.x = x;
        this.y = y;
        this.zoom = zoom;
        this.size = size;
    }

    /**
     * Returns a rectangle of the tiles position in scaled coordinates.
     */
    public RectF getRectF() {
        return new RectF(x, y, x + size.width, y + size.height);
    }

    /**
     * Returns a rectangle of the tiles position in non-scaled coordinates (coordinates as the zoom would be 1).
     */
    public RectF getCSSRectF() {
        float cssX = x / zoom;
        float cssY = y / zoom;
        float cssSizeW = size.width / zoom;
        float cssSizeH = size.height / zoom;
        return new RectF(cssX, cssY, cssX + cssSizeW, cssY + cssSizeH);
    }

    /**
     * Returns a integer rectangle of the tiles position in non-scaled and rounded coordinates (coordinates as the zoom would be 1).
     */
    public Rect getCSSRect() {
        float cssX = x / zoom;
        float cssY = y / zoom;
        float sizeW = size.width / zoom;
        float sizeH = size.height / zoom;
        return new Rect(
                (int) cssX, (int) cssY,
                (int) (cssX + sizeW),
                (int) (cssY + sizeH) );
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

    @Override
    public String toString() {
        return String.format("TileIdentifier (%d, %d) z=%f s=(%d, %d)", x, y, zoom, size.width, size.height);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
