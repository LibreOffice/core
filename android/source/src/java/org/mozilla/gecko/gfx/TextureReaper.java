/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.opengl.GLES20;
import android.util.Log;

import java.util.ArrayList;

/**
 * Manages a list of dead tiles, so we don't leak resources.
 */
public class TextureReaper {
    private static TextureReaper sSharedInstance;
    private ArrayList<Integer> mDeadTextureIDs = new ArrayList<Integer>();
    private static final String LOGTAG = TextureReaper.class.getSimpleName();

    private TextureReaper() {
    }

    public static TextureReaper get() {
        if (sSharedInstance == null) {
            sSharedInstance = new TextureReaper();
        }
        return sSharedInstance;
    }

    public void add(int[] textureIDs) {
        for (int textureID : textureIDs) {
            add(textureID);
        }
    }

    public synchronized void add(int textureID) {
        mDeadTextureIDs.add(textureID);
    }

    public synchronized void  reap() {
        int numTextures = mDeadTextureIDs.size();
        // Adreno 200 will generate INVALID_VALUE if len == 0 is passed to glDeleteTextures,
        // even though it's not supposed to.
        if (numTextures == 0)
            return;

        int[] deadTextureIDs = new int[numTextures];
        for (int i = 0; i < numTextures; i++) {
            Integer id = mDeadTextureIDs.get(i);
            if (id == null) {
                deadTextureIDs[i] = 0;
                Log.e(LOGTAG, "Dead texture id is null");
            } else {
                deadTextureIDs[i] = mDeadTextureIDs.get(i);
            }
        }
        mDeadTextureIDs.clear();

        GLES20.glDeleteTextures(deadTextureIDs.length, deadTextureIDs, 0);
    }
}