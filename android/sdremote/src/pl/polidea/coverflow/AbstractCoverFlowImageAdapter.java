/*
 * Copyright (c) 2011, Polidea
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 *  Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the distribution.
 *  Neither the name of the Polidea nor the names of its contributors may be used to endorse or promote products derived
 *  from this software without specific prior written permission.
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 *  BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 *  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package pl.polidea.coverflow;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;

/**
 * This class is an adapter that provides base, abstract class for images
 * adapter.
 *
 */
public abstract class AbstractCoverFlowImageAdapter extends BaseAdapter {

    /** The Constant TAG. */
    private static final String TAG = AbstractCoverFlowImageAdapter.class
                    .getSimpleName();

    /** The width. */
    private float width = 0;

    /** The height. */
    private float height = 0;

    public AbstractCoverFlowImageAdapter() {
        super();
    }

    /**
     * Set width for all pictures.
     *
     * @param width
     *            picture height
     */
    public synchronized void setWidth(final float width) {
        this.width = width;
    }

    /**
     * Set height for all pictures.
     *
     * @param height
     *            picture height
     */
    public synchronized void setHeight(final float height) {
        this.height = height;
    }

    @Override
    public final Bitmap getItem(final int position) {
        return createBitmap(position);
    }

    /**
     * Creates new bitmap for the position specified.
     *
     * @param position
     *            position
     * @return Bitmap created
     */
    protected abstract Bitmap createBitmap(int position);

    /*
     * (non-Javadoc)
     *
     * @see android.widget.Adapter#getItemId(int)
     */
    @Override
    public final synchronized long getItemId(final int position) {
        return position;
    }

    /*
     * (non-Javadoc)
     *
     * @see android.widget.Adapter#getView(int, android.view.View,
     * android.view.ViewGroup)
     */
    @Override
    public final synchronized ImageView getView(final int position,
                    final View convertView, final ViewGroup parent) {
        ImageView imageView;
        if (convertView == null) {
            final Context context = parent.getContext();
            Log.v(TAG, "Creating Image view at position: " + position + ":"
                            + this);
            imageView = new ImageView(context);
            imageView.setLayoutParams(new CoverFlow.LayoutParams((int) width,
                            (int) height));
        } else {
            Log.v(TAG, "Reusing view at position: " + position + ":" + this);
            imageView = (ImageView) convertView;
        }
        imageView.setImageBitmap(getItem(position));
        return imageView;
    }

}
