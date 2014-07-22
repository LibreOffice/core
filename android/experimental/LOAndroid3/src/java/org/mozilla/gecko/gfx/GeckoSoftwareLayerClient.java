/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Android code.
 *
 * The Initial Developer of the Original Code is Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2009-2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Patrick Walton <pcwalton@mozilla.com>
 *   Chris Lord <chrislord.net@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

package org.mozilla.gecko.gfx;

import android.content.Context;
import android.util.Log;

import java.util.List;


/**
 * Transfers a software-rendered Gecko to an ImageLayer so that it can be rendered by our
 * compositor.
 *
 * TODO: Throttle down Gecko's priority when we pan and zoom.
 */
public class GeckoSoftwareLayerClient extends GeckoLayerClient {
    private static final String LOGTAG = "GeckoSoftwareLayerClient";

    private static final IntSize TILE_SIZE = new IntSize(256, 256);

    public GeckoSoftwareLayerClient(Context context) {
        super(context);
    }

    public void setLayerController(LayerController layerController) {
        super.setLayerController(layerController);

        layerController.setRoot(mTileLayer);

        if (mGeckoViewport != null) {
            layerController.setViewportMetrics(mGeckoViewport);
        }

        sendResizeEventIfNecessary(false);
    }

    @Override
    protected boolean setupLayer() {
        Log.i(LOGTAG, "Creating MultiTileLayer");
        if(mTileLayer == null) {
            mTileLayer = new MultiTileLayer(TILE_SIZE);
            mLayerController.setRoot(mTileLayer);
        }

        // Force a resize event to be sent because the results of this
        // are different depending on what tile system we're using
        sendResizeEventIfNecessary(true);

        return false;
    }

    @Override
    protected void updateLayerAfterDraw() {
        if (mTileLayer instanceof MultiTileLayer) {
            ((MultiTileLayer)mTileLayer).invalidate();
        }
    }


    @Override
    protected IntSize getTileSize() {
        return TILE_SIZE;
    }

    public void addTile(SubTile tile) {
        if (mTileLayer instanceof MultiTileLayer) {
            ((MultiTileLayer)mTileLayer).addTile(tile);
        }
    }

    public List<SubTile> getTiles() {
        if (mTileLayer instanceof MultiTileLayer) {
            return ((MultiTileLayer)mTileLayer).getTiles();
        }
        return null;
    }
}

