/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.util;

import java.lang.ref.WeakReference;
import java.util.Arrays;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.widget.ImageView;

public final class ImageLoader {
    private final Resources mResources;
    private final Bitmap mLoadingImage;

    public ImageLoader(Resources aResources, int aLoadingImageResourceId) {
        mResources = aResources;
        mLoadingImage = BitmapFactory.decodeResource(mResources, aLoadingImageResourceId);
    }

    public void loadImage(ImageView aImageView, byte[] aImageBytes) {
        if (isSameImageLoading(aImageView, aImageBytes)) {
            return;
        }

        if (isImageLoadingCancellationRequired(aImageView, aImageBytes)) {
            cancelImageLoading(aImageView);
        }

        startImageLoading(aImageView, aImageBytes);
    }

    private boolean isSameImageLoading(ImageView aImageView, byte[] aImageBytes) {
        if (!isImageLoading(aImageView)) {
            return false;
        }

        ImageLoadingTask aImageLoadingTask = getImageLoadingTask(aImageView);

        return Arrays.equals(aImageBytes, aImageLoadingTask.getImageBytes());
    }

    private boolean isImageLoading(ImageView aImageView) {
        ImageLoadingTask aImageLoadingTask = getImageLoadingTask(aImageView);

        return aImageLoadingTask != null;
    }

    private ImageLoadingTask getImageLoadingTask(ImageView aImageView) {
        if (aImageView == null) {
            return null;
        }

        Drawable aImageDrawable = aImageView.getDrawable();

        if (!(aImageDrawable instanceof AsyncDrawable)) {
            return null;
        }

        AsyncDrawable aAsyncImageDrawable = (AsyncDrawable) aImageDrawable;

        return aAsyncImageDrawable.getImageLoadingTask();
    }

    private boolean isImageLoadingCancellationRequired(ImageView aImageView, byte[] aImageBytes) {
        return isImageLoading(aImageView) && !isSameImageLoading(aImageView, aImageBytes);
    }

    private void cancelImageLoading(ImageView aImageView) {
        ImageLoadingTask aImageLoadingTask = getImageLoadingTask(aImageView);

        aImageLoadingTask.cancel(true);
    }

    private void startImageLoading(ImageView aImageView, byte[] aImageBytes) {
        ImageLoadingTask aImageLoadingTask = new ImageLoadingTask(aImageView, aImageBytes);
        AsyncDrawable aAsyncDrawable = new AsyncDrawable(mResources, mLoadingImage, aImageLoadingTask);

        aImageView.setImageDrawable(aAsyncDrawable);
        aImageLoadingTask.execute();
    }

    private static class ImageLoadingTask extends AsyncTask<Void, Void, Bitmap> {
        private final WeakReference<ImageView> mImageViewReference;
        private final byte[] mImageBytes;

        public ImageLoadingTask(ImageView aImageView, byte[] aImageBytes) {
            mImageViewReference = new WeakReference<ImageView>(aImageView);
            mImageBytes = aImageBytes;
        }

        public byte[] getImageBytes() {
            return mImageBytes;
        }

        @Override
        protected Bitmap doInBackground(Void... aParameters) {
            return BitmapFactory.decodeByteArray(mImageBytes, 0, mImageBytes.length);
        }

        @Override
        protected void onPostExecute(Bitmap aBitmap) {
            super.onPostExecute(aBitmap);

            if (isCancelled()) {
                return;
            }

            if (aBitmap == null) {
                return;
            }

            if (mImageViewReference == null) {
                return;
            }

            if (getImageView() == null) {
                return;
            }

            getImageView().setImageBitmap(aBitmap);
        }

        private ImageView getImageView() {
            return mImageViewReference.get();
        }
    }

    private static class AsyncDrawable extends BitmapDrawable {
        private final WeakReference<ImageLoadingTask> mImageLoadingTaskReference;

        public AsyncDrawable(Resources aResources, Bitmap aBitmap, ImageLoadingTask aImageLoadingTask) {
            super(aResources, aBitmap);

            mImageLoadingTaskReference = new WeakReference<ImageLoadingTask>(aImageLoadingTask);
        }

        public ImageLoadingTask getImageLoadingTask() {
            return mImageLoadingTaskReference.get();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
