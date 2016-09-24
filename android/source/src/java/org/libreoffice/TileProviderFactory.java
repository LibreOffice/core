/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;


import org.libreoffice.kit.LibreOfficeKit;
import org.mozilla.gecko.gfx.GeckoLayerClient;

/**
 * Create a desired instance of TileProvider.
 */
public class TileProviderFactory {

    private TileProviderFactory() {
    }

    public static void initialize() {
        LibreOfficeKit.initializeLibrary();
    }

    public static TileProvider create(GeckoLayerClient layerClient, InvalidationHandler invalidationHandler, String filename) {
         return new LOKitTileProvider(layerClient, invalidationHandler, filename);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
