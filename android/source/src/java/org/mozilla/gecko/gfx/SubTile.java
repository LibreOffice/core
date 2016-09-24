/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Region;
import android.graphics.RegionIterator;
import android.opengl.GLES20;
import android.util.Log;

import org.libreoffice.TileIdentifier;

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;

public class SubTile extends Layer {
    private static String LOGTAG = SubTile.class.getSimpleName();
    public final TileIdentifier id;

    private final RectF mBounds;
    private final RectF mTextureBounds;
    private final RectF mViewport;
    private final Rect mIntBounds;
    private final Rect mSubRect;
    private final RectF mSubRectF;
    private final Region mMaskedBounds;
    private final Rect mCropRect;
    private final RectF mObjRectF;
    private final float[] mCoords;

    public boolean markedForRemoval = false;

    private CairoImage mImage;
    private IntSize mSize;
    private int[] mTextureIDs;
    private boolean mDirtyTile;

    public SubTile(TileIdentifier id) {
        super();
        this.id = id;

        mBounds = new RectF();
        mTextureBounds = new RectF();
        mViewport = new RectF();
        mIntBounds = new Rect();
        mSubRect = new Rect();
        mSubRectF = new RectF();
        mMaskedBounds = new Region();
        mCropRect = new Rect();
        mObjRectF = new RectF();
        mCoords = new float[20];

        mImage = null;
        mTextureIDs = null;
        mSize = new IntSize(0, 0);
        mDirtyTile = false;
    }

    public void setImage(CairoImage image) {
        if (image.getSize().isPositive()) {
            this.mImage = image;
        }
    }

    public void refreshTileMetrics() {
        setPosition(id.getCSSRect());
    }

    public void markForRemoval() {
        markedForRemoval = true;
    }

    protected int getTextureID() {
        return mTextureIDs[0];
    }

    protected boolean initialized() {
        return mTextureIDs != null;
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            destroyImage();
            cleanTexture(false);
        } finally {
            super.finalize();
        }
    }

    private void cleanTexture(boolean immediately) {
        if (mTextureIDs != null) {
            TextureReaper.get().add(mTextureIDs);
            mTextureIDs = null;
            if (immediately) {
                TextureReaper.get().reap();
            }
        }
    }

    public void destroy() {
        try {
            destroyImage();
            cleanTexture(false);
        } catch (Exception ex) {
            Log.e(LOGTAG, "Error clearing buffers: ", ex);
        }
    }

    public void destroyImage() {
        if (mImage != null) {
            mImage.destroy();
            mImage = null;
        }
    }

    /**
     * Invalidates the entire buffer so that it will be uploaded again. Only valid inside a
     * transaction.
     */
    public void invalidate() {
        if (!inTransaction()) {
            throw new RuntimeException("invalidate() is only valid inside a transaction");
        }
        if (mImage == null) {
            return;
        }
        mDirtyTile = true;
    }

    /**
     * Remove the texture if the image is of different size than the current uploaded texture.
     */
    private void validateTexture() {
        IntSize textureSize = mImage.getSize().nextPowerOfTwo();

        if (!textureSize.equals(mSize)) {
            mSize = textureSize;
            cleanTexture(true);
        }
    }

    @Override
    protected void performUpdates(RenderContext context) {
        super.performUpdates(context);
        if (mImage == null && !mDirtyTile) {
            return;
        }
        validateTexture();
        uploadNewTexture();
        mDirtyTile = false;
    }

    private void uploadNewTexture() {
        ByteBuffer imageBuffer = mImage.getBuffer();
        if (imageBuffer == null) {
            return;
        }

        if (mTextureIDs == null) {
            mTextureIDs = new int[1];
            GLES20.glGenTextures(mTextureIDs.length, mTextureIDs, 0);
        }

        int cairoFormat = mImage.getFormat();
        CairoGLInfo glInfo = new CairoGLInfo(cairoFormat);

        bindAndSetGLParameters();

        IntSize bufferSize = mImage.getSize();

        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, glInfo.internalFormat,
                mSize.width, mSize.height, 0, glInfo.format, glInfo.type, imageBuffer);

        destroyImage();
    }

    private void bindAndSetGLParameters() {
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextureIDs[0]);

        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
    }

    @Override
    public void draw(RenderContext context) {
        // mTextureIDs may be null here during startup if Layer.java's draw method
        // failed to acquire the transaction lock and call performUpdates.
        if (!initialized())
            return;

        mViewport.set(context.viewport);

        mBounds.set(getBounds(context));
        mTextureBounds.set(mBounds);

        mBounds.roundOut(mIntBounds);
        mMaskedBounds.set(mIntBounds);

        // XXX Possible optimisation here, form this array so we can draw it in
        //     a single call.
        RegionIterator iterator = new RegionIterator(mMaskedBounds);
        while (iterator.next(mSubRect)) {
            // Compensate for rounding errors at the edge of the tile caused by
            // the roundOut above
            mSubRectF.set(Math.max(mBounds.left, (float) mSubRect.left),
                    Math.max(mBounds.top, (float) mSubRect.top),
                    Math.min(mBounds.right, (float) mSubRect.right),
                    Math.min(mBounds.bottom, (float) mSubRect.bottom));

            // This is the left/top/right/bottom of the rect, relative to the
            // bottom-left of the layer, to use for texture coordinates.
            mCropRect.set(Math.round(mSubRectF.left - mBounds.left),
                    Math.round(mBounds.bottom - mSubRectF.top),
                    Math.round(mSubRectF.right - mBounds.left),
                    Math.round(mBounds.bottom - mSubRectF.bottom));

            mObjRectF.set(mSubRectF.left - mViewport.left,
                    mViewport.bottom - mSubRectF.bottom,
                    mSubRectF.right - mViewport.left,
                    mViewport.bottom - mSubRectF.top);

            fillRectCoordBuffer(mCoords, mObjRectF, mViewport.width(), mViewport.height(), mCropRect, mTextureBounds.width(), mTextureBounds.height());

            FloatBuffer coordBuffer = context.coordBuffer;
            int positionHandle = context.positionHandle;
            int textureHandle = context.textureHandle;

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, getTextureID());

            // Make sure we are at position zero in the buffer
            coordBuffer.position(0);
            coordBuffer.put(mCoords);

            // Unbind any the current array buffer so we can use client side buffers
            GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);

            // Vertex coordinates are x,y,z starting at position 0 into the buffer.
            coordBuffer.position(0);
            GLES20.glVertexAttribPointer(positionHandle, 3, GLES20.GL_FLOAT, false, 20, coordBuffer);

            // Texture coordinates are texture_x, texture_y starting at position 3 into the buffer.
            coordBuffer.position(3);
            GLES20.glVertexAttribPointer(textureHandle, 2, GLES20.GL_FLOAT, false, 20, coordBuffer);
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        }
    }
}