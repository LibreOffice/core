/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

package org.libreoffice.ui;

import org.libreoffice.R;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class PageView extends View{
    private Bitmap bmp;
    private Paint mPaintBlack;
    private static final String LOGTAG = "PageView";

    public PageView(Context context ) {
        super(context);
        bmp = BitmapFactory.decodeResource(getResources(), R.drawable.dummy_page);
        initialise();
    }
    public PageView(Context context, AttributeSet attrs) {
        super(context, attrs);
        bmp = BitmapFactory.decodeResource(getResources(), R.drawable.dummy_page);
        Log.d(LOGTAG, bmp.toString());
        initialise();
    }
    public PageView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        bmp = BitmapFactory.decodeResource(getResources(), R.drawable.dummy_page);//load a "page"
        initialise();
    }

    private void initialise(){
        mPaintBlack = new Paint();
        mPaintBlack.setARGB(255, 0, 0, 0);
        Log.d(LOGTAG, " Doing some set-up");
    }

    public void setBitmap(Bitmap bmp){
        this.bmp = bmp;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        Log.d(LOGTAG, "Draw");
        Log.d(LOGTAG, Integer.toString(bmp.getHeight()));
        if( bmp != null ){
            int horizontalMargin = (int) (canvas.getWidth()*0.1);
            //int verticalMargin = (int) (canvas.getHeight()*0.1);
            int verticalMargin = horizontalMargin;
            canvas.drawBitmap(bmp, new Rect(0, 0, bmp.getWidth(), bmp.getHeight()),
                    new Rect(horizontalMargin,verticalMargin,canvas.getWidth()-horizontalMargin,
                        canvas.getHeight()-verticalMargin),
                    mPaintBlack);//
        }
        if( bmp == null)
            canvas.drawText(getContext().getString(R.string.bmp_null), 100, 100, new Paint());
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
