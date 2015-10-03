/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.ui;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Color;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import java.io.File;
import java.util.Stack;

public class FolderIconView extends View{
    private String LOGTAG = "FolderIconView";

    private Paint mPaintBlack;
    private Paint mPaintGray;
    private Paint mPaintShadow;

    private File dir;

    public FolderIconView(Context context) {
        super(context);
        initialisePaints();
    }
    public FolderIconView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initialisePaints();
    }
    public FolderIconView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initialisePaints();
    }

    private void initialisePaints() {
        mPaintBlack = new Paint();
        mPaintBlack.setColor(Color.DKGRAY);//Can also use parseColor(String "#aarrggbb")
        mPaintBlack.setAntiAlias(true);

        mPaintGray = new Paint();
        mPaintGray.setColor(Color.GRAY);//Can also use parseColor(String "#aarrggbb")
        mPaintGray.setAntiAlias(true);

        mPaintShadow = new Paint();
        mPaintShadow.setColor(Color.parseColor("#88888888"));
        mPaintShadow.setAntiAlias(true);
    }

    public void setDir(File dir) {
        this.dir = dir;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        Log.d(LOGTAG, "onDraw");
        //float width = (float)canvas.getWidth();
        //float height = (float)canvas.getHeight();
        float width = (float) this.getWidth();
        float height = (float) this.getHeight();
        float centerX = width*0.5f;// centered on horz axis
        float centerY = height*0.5f;
        float outerRadius = 0.8f*0.5f*width;
        float innerRadius = 0.7f*0.5f*width;
        float thumbHeight = outerRadius*1.25f;
        float thumbWidth = thumbHeight*(float)(1/Math.sqrt(2));
        float DZx = 0.2f*outerRadius;
        float DZy = 0.2f*outerRadius;
        //Bitmap blankPage = BitmapFactory.decodeResource(getResources(), R.drawable.page);
        Log.i(LOGTAG, Float.toString(width) + "x" + Float.toString(height));
        canvas.drawCircle(centerX, centerY, outerRadius, mPaintGray);
        canvas.drawCircle(centerX, centerY, innerRadius, mPaintBlack);
        //Either get thumbs from directory or use generic page images
        //For now just get the first 4 thumbs -> add some checks later
        if (dir == null)
            return;//TODO
        File[] contents = dir.listFiles();//TODO consider filtering thumbs to match grid.
        if (contents == null)
            // dir is not a directory,
            // or user does not have permissions to read it
            return;
        Stack<Bitmap> thumbs = new Stack<Bitmap>();
        BitmapFactory factory = new BitmapFactory();
        for (File file : contents) {
            if (!FileUtilities.isThumbnail(file))
                continue;
            thumbs.push(BitmapFactory.decodeFile(file.getAbsolutePath()));//TODO switch to push for semantics
            if (thumbs.size() > 3)
                break;
        }
        /*while(thumbs.size() < 4) {// padd out with blanks?
            thumbs.push(blankPage);
        }*/
        Log.i(LOGTAG, Integer.toString(thumbs.size()));
        //should handle empty folders better
        //  options:
        //      don't show?
        //      show generic LO icons for writer etc
        //      Show a generic blank page icon
        if (thumbs.isEmpty())
            return;
        /*float left = centerX ;//+ 0.25f*outerRadius;
        float top = centerY - 0.5f*outerRadius;
        float right = left + thumbs.get(0).getWidth()*0.4f;
        float bottom = top + thumbs.get(0).getHeight()*0.4f;
        RectF dest = new RectF(left, top, right, bottom);
        RectF shadowBox = new RectF(dest);
        shadowBox.inset(-1, -1);
        int size = thumbs.size();
        for (int i = 1; i <= size; i++) {
            canvas.drawRect(shadowBox, mPaintShadow);
            canvas.drawBitmap(thumbs.pop(), null, dest, null);
            dest.offset(-outerRadius*0.2f, outerRadius*0.1f);
            shadowBox.offset(-outerRadius*0.2f, outerRadius*0.1f);
        }*/
        float left;
        float top;
        float right;
        float bottom;
        RectF dest;
        RectF shadowBox;
        int size;
        switch(thumbs.size()) {
            case 0:
                break;
            case 1:
                left = centerX - 0.5f*thumbWidth;
                top = centerY - 0.5f*thumbHeight;
                right = left + thumbWidth;
                bottom = top + thumbHeight;
                dest = new RectF(left, top, right, bottom);
                shadowBox = new RectF(dest);
                shadowBox.inset(-1, -1);
                canvas.drawRect(shadowBox, mPaintShadow);
                canvas.drawBitmap(thumbs.pop(), null, dest, null);
                break;
            case 2:
                left = centerX - 0.5f*thumbWidth + 0.5f*DZx;
                top = centerY - 0.5f*thumbHeight - 0.5f*DZy;
                right = left + thumbWidth;
                bottom = top + thumbHeight;
                dest = new RectF(left, top, right, bottom);
                shadowBox = new RectF(dest);
                shadowBox.inset(-1, -1);
                size = thumbs.size();
                for (int i = 1; i <= size; i++) {
                    canvas.drawRect(shadowBox, mPaintShadow);
                    canvas.drawBitmap(thumbs.pop(), null, dest, null);
                    dest.offset(-DZx, DZy);
                    shadowBox.offset(-DZx, DZy);
                }
                break;
            case 3:
                left = centerX - 0.5f*thumbWidth + DZx;
                top = centerY - 0.5f*thumbHeight - DZy;
                right = left + thumbWidth;
                bottom = top + thumbHeight;
                dest = new RectF(left, top, right, bottom);
                shadowBox = new RectF(dest);
                shadowBox.inset(-1, -1);
                size = thumbs.size();
                for (int i = 1; i <= size; i++) {
                    canvas.drawRect(shadowBox, mPaintShadow);
                    canvas.drawBitmap(thumbs.pop(), null, dest, null);
                    dest.offset(-DZx, DZy);
                    shadowBox.offset(-DZx, DZy);
                }
                break;
            case 4:
                left = centerX - 0.5f*thumbWidth + 1.5f*DZx;
                top = centerY - 0.5f*thumbHeight - 1.5f*DZy;
                right = left + thumbWidth;
                bottom = top + thumbHeight;
                dest = new RectF(left, top, right, bottom);
                shadowBox = new RectF(dest);
                shadowBox.inset(-1, -1);
                size = thumbs.size();
                for (int i = 1; i <= size; i++) {
                    canvas.drawRect(shadowBox, mPaintShadow);
                    canvas.drawBitmap(thumbs.pop(), null, dest, null);
                    dest.offset(-DZx, DZy);
                    shadowBox.offset(-DZx, DZy);
                }
                break;
            default:
                break;
        }
        //test

        return;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
