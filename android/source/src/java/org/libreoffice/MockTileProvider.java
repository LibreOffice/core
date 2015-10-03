/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.graphics.Bitmap;
import android.graphics.PointF;
import android.view.KeyEvent;

import org.mozilla.gecko.gfx.BufferedCairoImage;
import org.mozilla.gecko.gfx.CairoImage;
import org.mozilla.gecko.gfx.GeckoLayerClient;
import org.mozilla.gecko.gfx.IntSize;

public class MockTileProvider implements TileProvider {
    private static final int TILE_SIZE = 256;
    private final GeckoLayerClient mLayerClient;
    private final String inputFile;

    public MockTileProvider(GeckoLayerClient layerClient, String input) {
        mLayerClient = layerClient;
        this.inputFile = input;

        for (int i = 0; i < 5; i++) {
            String partName = "Part " + i;
            final DocumentPartView partView = new DocumentPartView(i, partName);
            LibreOfficeMainActivity.mAppContext.mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    LibreOfficeMainActivity.mAppContext.getDocumentPartViewListAdapter().add(partView);
                }
            });
        }
        LibreOfficeMainActivity.mAppContext.mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                LibreOfficeMainActivity.mAppContext.getDocumentPartViewListAdapter().notifyDataSetChanged();
            }
        });
    }

    @Override
    public int getPageWidth() {
        return 549;
    }

    @Override
    public int getPageHeight() {
        return 630 * 5;
    }

    @Override
    public boolean isReady() {
        return true;
    }

    @Override
    public CairoImage createTile(float x, float y, IntSize tileSize, float zoom) {
        int tiles = (int) (getPageWidth() / TILE_SIZE) + 1;
        int tileNumber = (int) ((y / TILE_SIZE) * tiles + (x / TILE_SIZE));
        tileNumber %= 9;
        tileNumber += 1; // 0 to 1 based numbering

        String imageName = "d" + tileNumber;
        Bitmap bitmap = mLayerClient.getView().getDrawable(imageName);

        return new BufferedCairoImage(bitmap);
    }

    @Override
    public void rerenderTile(CairoImage image, float x, float y, IntSize tileSize, float zoom) {
    }

    @Override
    public Bitmap thumbnail(int size) {
        return mLayerClient.getView().getDrawable("dummy_page");
    }

    @Override
    public void close() {
    }

    @Override
    public boolean isTextDocument() {
        return true;
    }

    @Override
    public boolean isSpreadsheet() {
        return false;
    }

    @Override
    public void sendKeyEvent(KeyEvent keyEvent) {
    }

    @Override
    public void mouseButtonDown(PointF documentCoordinate, int numberOfClicks) {
    }

    @Override
    public void mouseButtonUp(PointF documentCoordinate, int numberOfClicks) {
    }

    @Override
    public void postUnoCommand(String command) {
    }

    @Override
    public void setTextSelectionStart(PointF documentCoordinate) {
    }

    @Override
    public void setTextSelectionEnd(PointF documentCoordinate) {
    }

    @Override
    public void setTextSelectionReset(PointF documentCoordinate) {
    }

    @Override
    public void setGraphicSelectionStart(PointF documentCoordinate) {
    }

    @Override
    public void setGraphicSelectionEnd(PointF documentCoordinate) {
    }

    @Override
    public void changePart(int partIndex) {
    }

    @Override
    public int getCurrentPartNumber() {
        return 0;
    }

    @Override
    public int getPartsCount() {
        return 0;
    }

    @Override
    public void onSwipeLeft() {
    }

    @Override
    public void onSwipeRight() {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
