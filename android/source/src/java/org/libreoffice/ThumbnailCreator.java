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
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.widget.ImageView;

import java.lang.ref.WeakReference;

/**
 * Create thumbnails for the parts of the document.
 */
public class ThumbnailCreator {
    private static final String LOG_TAG = ThumbnailCreator.class.getSimpleName();
    private static final int THUMBNAIL_SIZE = 256;

    private static boolean needsThumbnailCreation(int partNumber, ImageView imageView) {
        ThumbnailCreationTask thumbnailCreationTask = currentThumbnailCreationTask(imageView);

        if (thumbnailCreationTask == null) {
            return true;
        }

        if (thumbnailCreationTask.partNumber != partNumber) {
            thumbnailCreationTask.cancel();
            return true;
        } else {
            return false;
        }
    }

    private static ThumbnailCreationTask currentThumbnailCreationTask(ImageView imageView) {
        if (imageView == null) {
            return null;
        }
        Drawable drawable = imageView.getDrawable();
        if (drawable instanceof ThumbnailDrawable) {
            return ((ThumbnailDrawable) drawable).thumbnailCreationTask.get();
        } else {
            return null;
        }
    }

    public void createThumbnail(int partNumber, ImageView imageView) {
        if (needsThumbnailCreation(partNumber, imageView)) {
            ThumbnailCreationTask task = new ThumbnailCreationTask(imageView, partNumber);
            ThumbnailDrawable thumbnailDrawable = new ThumbnailDrawable(task);
            imageView.setImageDrawable(thumbnailDrawable);
            imageView.setMinimumHeight(THUMBNAIL_SIZE);
            LOKitShell.sendThumbnailEvent(task);
        }
    }

    static class ThumbnailDrawable extends ColorDrawable {
        public final WeakReference<ThumbnailCreationTask> thumbnailCreationTask;

        public ThumbnailDrawable(ThumbnailCreationTask thumbnailCreationTask) {
            super(Color.WHITE);
            this.thumbnailCreationTask = new WeakReference<ThumbnailCreationTask>(thumbnailCreationTask);
        }
    }

    class ThumbnailCreationTask{
        private final WeakReference<ImageView> imageViewReference;
        private int partNumber;
        private boolean cancelled = false;

        public ThumbnailCreationTask(ImageView imageView, int partNumber) {
            imageViewReference = new WeakReference<ImageView>(imageView);
            this.partNumber = partNumber;
        }

        public void cancel() {
            cancelled = true;
        }

        public Bitmap getThumbnail(TileProvider tileProvider) {
            int currentPart = tileProvider.getCurrentPartNumber();
            tileProvider.changePart(partNumber);
            final Bitmap bitmap = tileProvider.thumbnail(THUMBNAIL_SIZE);
            tileProvider.changePart(currentPart);
            return bitmap;
        }

        private void changeBitmap(Bitmap bitmap) {
            if (cancelled) {
                bitmap = null;
            }

            if (imageViewReference == null) {
                return;
            }
            ImageView imageView = imageViewReference.get();
            ThumbnailCreationTask thumbnailCreationTask = currentThumbnailCreationTask(imageView);
            if (this == thumbnailCreationTask) {
                imageView.setImageBitmap(bitmap);
            }
        }

        public void applyBitmap(final Bitmap bitmap) {
            // run on UI thread
            LibreOfficeMainActivity.mAppContext.mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    changeBitmap(bitmap);
                }
            });
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
