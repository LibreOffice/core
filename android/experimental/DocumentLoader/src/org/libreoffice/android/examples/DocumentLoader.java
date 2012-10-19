// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This is just a testbed for ideas and implementations. (Still, it might turn
// out to be somewhat useful as such while waiting for "real" apps.)

// Important points:

// Everything that might take a long time should be done asynchronously:
//  - loading the document (loadComponentFromURL())
//  - counting number of pages (getRendererCount())
//  - rendering a page (render())

// Unclear whether pages can be rendered in parallel. Probably best to
// serialize all the above in the same worker thread. We use
// AsyncTask.SERIAL_EXECUTOR below.

// While a page is loading ideally should display some animated spinner (but
// for now just a static "please wait" text).

// ===

// How should we handle re-rendering at higher resolution when zooming in, and
// then panning around?

// Hopefully when LO is asked to render just a part of a page (i.e. the
// MapMode of the device rendered to causes significant parts of the page to
// be outside the device) the code is clever enough to quickly skip stuff that
// will be clipped. But I don't hold my breath.

// How could we do it?

// 1/ Re-render just the zoomed-in area. Probably not a good idea, as probably
// the user will almost immediately also pan a bit or zoom out a bit, which
// would cause a re-render.

// 2/ Some kind of tiled approach. Initially just one tile for the whole
// page. When zooming in, at some point (2x?) split the currently visible
// tiles into four sub-tiles, each initially displaying the same resolution as
// the parent tile. Start asynchronous rendering of visible sub-tiles at
// double resolution. Keep the "parent" rendered bitmap but don't keep bitmaps
// that go out of view. (Except perhaps for some caching.) When zooming out,
// at some point (0.5x?) merge four sub-tiles back into one. Hmm. Is this the
// right approach?

// In any case, also this rendering at higher resolution should be done
// asynchronously, of course. If the user zooms in and pans around, the
// existing bitmap will be shown scaled (and ugly) until required rendering
// has finished and then the affected tiles are replaced with
// higher-resolution ones.

package org.libreoffice.android.examples;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.Matrix;
import android.graphics.PixelFormat;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.TranslateAnimation;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.ViewFlipper;
import android.widget.ViewSwitcher;

import junit.framework.Assert;

import com.polites.android.GestureImageView;

import com.sun.star.awt.Size;
import com.sun.star.awt.XBitmap;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XToolkit2;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.view.XRenderable;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.libreoffice.android.Bootstrap;

public class DocumentLoader
    extends Activity
{
    private static final String TAG = "DocumentLoader";

    // Size of a small virtual (bitmap) device used to find out page count and
    // page sizes
    private static final int SMALLSIZE = 128;

    // We pre-render this many pages preceding and succeeding the currently
    // viewed one, i.e. the total number of rendered pages kept is
    // PAGECACHE_PLUSMINUS*2+1.
    private static final int PAGECACHE_PLUSMINUS = 2;
    private static final int PAGECACHE_SIZE = PAGECACHE_PLUSMINUS*2 + 1;

    long timingOverhead;
    XComponentContext context;
    XMultiComponentFactory mcf;
    XComponentLoader componentLoader;
    XToolkit2 toolkit;
    XDevice dummySmallDevice;
    Object doc;
    int pageCount;
    XRenderable renderable;

    GestureDetector.OnGestureListener gestureListener;
    GestureDetector gestureDetector;

    ViewGroup.LayoutParams matchParent;

    ViewFlipper flipper;

    Bundle extras;

    class GestureListener
        extends GestureDetector.SimpleOnGestureListener
    {
        @Override
        public boolean onFling(MotionEvent event1,
                               MotionEvent event2,
                               float velocityX,
                               float velocityY)
        {
            Log.i(TAG, "onFling: " + event1 + " " + event2);
            if (event1.getX() - event2.getX() > 120) {
                if (((PageViewer)flipper.getCurrentView()).currentPageNumber == pageCount-1)
                    return false;

                Animation inFromRight = new TranslateAnimation(Animation.RELATIVE_TO_SELF, 1, Animation.RELATIVE_TO_SELF, 0,
                                                               Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0);
                int duration = Math.abs((int)((float)flipper.getWidth()/velocityX*1000f));
                inFromRight.setDuration(duration);
                flipper.setInAnimation(inFromRight);

                Animation outToLeft = new TranslateAnimation(Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, -1,
                                                             Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0);
                outToLeft.setDuration(duration);
                flipper.setOutAnimation(outToLeft);

                flipper.showNext();

                ((PageViewer)flipper.getChildAt((flipper.getDisplayedChild() + PAGECACHE_PLUSMINUS) % PAGECACHE_SIZE)).display(((PageViewer)flipper.getCurrentView()).currentPageNumber + PAGECACHE_PLUSMINUS);
                return true;
            } else if (event2.getX() - event1.getX() > 120) {
                if (((PageViewer)flipper.getCurrentView()).currentPageNumber == 0)
                    return false;

                Animation inFromLeft = new TranslateAnimation(Animation.RELATIVE_TO_SELF, -1, Animation.RELATIVE_TO_SELF, 0,
                                                              Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0);
                int duration = Math.abs((int)((float)flipper.getWidth()/velocityX*1000f));
                inFromLeft.setDuration(duration);
                flipper.setInAnimation(inFromLeft);

                Animation outToRight = new TranslateAnimation(Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 1,
                                                              Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0);
                outToRight.setDuration(duration);
                flipper.setOutAnimation(outToRight);

                flipper.showPrevious();

                ((PageViewer)flipper.getChildAt((flipper.getDisplayedChild() + PAGECACHE_SIZE - PAGECACHE_PLUSMINUS) % PAGECACHE_SIZE)).display(((PageViewer)flipper.getCurrentView()).currentPageNumber - PAGECACHE_PLUSMINUS);

                return true;
            }
            return false;
        }
    }

    class MyXController
        implements XController
    {

        XFrame frame;
        XModel model;

        public void attachFrame(XFrame frame)
        {
            Log.i(TAG, "attachFrame");
            this.frame = frame;
        }

        public boolean attachModel(XModel model)
        {
            Log.i(TAG, "attachModel");
            this.model = model;
            return true;
        }

        public boolean suspend(boolean doSuspend)
        {
            Log.i(TAG, "suspend");
            return false;
        }

        public Object getViewData()
        {
            Log.i(TAG, "getViewData");
            return null;
        }

        public void restoreViewData(Object data)
        {
            Log.i(TAG, "restoreViewData");
        }

        public XModel getModel()
        {
            Log.i(TAG, "getModel");
            return model;
        }

        public XFrame getFrame()
        {
            Log.i(TAG, "getFrame");
            return frame;
        }

        public void dispose()
        {
            Log.i(TAG, "dispose");
        }

        public void addEventListener(XEventListener listener)
        {
            Log.i(TAG, "addEventListener");
        }

        public void removeEventListener(XEventListener listener)
        {
            Log.i(TAG, "removeEventListener");
        }
    }

    static int zoomLevel(float scale)
    {
        if (scale <= 1)
            return 0;

        int result = 1;
        int power = 2;

        while (scale > power) {
            result++;
            power *= 2;
        }
        return result;
    }

    static int scaleOfZoom(int zoom)
    {
        int result = 1;

        while (zoom > 0) {
            result *= 2;
            zoom--;
        }

        return result;
    }

    static int setColorAlpha(int color,
                             double alpha)
    {
        return Color.argb((int)(alpha*255), Color.red(color), Color.green(color), Color.blue(color));
    }

    // Each (Gesture)ImageView is showing an object of this subclass of
    // BitmapDrawable, the root of a quadtree of higher-resolution partial
    // page bitmaps. Obviously these should be rendered asynchronously on
    // demand but that code is not here yet. And anyway, rendering partial
    // pages won't work until I have figured out why offsetting VirtualDevice
    // has no effect.
    class QuadTree
        extends BitmapDrawable
    {
        final int pageNumber;
        final int level;
        final int location;
        final int w, h;

        static final int NW = 0;
        static final int NE = 1;
        static final int SE = 2;
        static final int SW = 3;

        QuadTree sub[] = new QuadTree[4];

        QuadTree(Bitmap bm,
                 int level,
                 int pageNumber,
                 int location)
        {
            super(bm);

            w = getIntrinsicWidth();
            h = getIntrinsicHeight();

            this.pageNumber = pageNumber;
            this.level = level;
            this.location = location;

            // I spent several days wondering why nothing showed up for the
            // sub-tiles, desperately tweaking stuff left and right, until I
            // found out I need to call setBounds()... (For the level 0
            // drawable the GestureImageView handles calling setBounds(), but
            // it doesn't hurt to do it here for all levels.)
            setBounds(0, 0, w, h);

            // Just for testing until properly doing this asynchronously and
            // with insert()
            if (level == 0) {
                // Don't do it anyway for now

                // 1) offsetting of VirtualDevice doesn't seem to work so this
                // would work for the NW sub-tile only anyway.

                // 2) LO renders Windows-style "Y grows upwards" bitmaps, thus
                // the imageView.setScaleY(-1) below in
                // PageLoadTask.onPostExecute(), but that means that the
                // scaling and translation stuff here in QuadTree needs to be
                // twiddled for Y coordinates. (Just try: comment out the
                // createSub(NW) call below. If you also comment out the
                // setScaleY() call, the sub-tile will show up in the correct
                // location, otherwise not. Or something like that. It's hard
                // to try to write up sevral nights of desperate hacking back
                // and forth... which in the end turned out to be just chasing
                // the wrong ducks. If that is how the metaphor goes?)

                // So probably should write a native method to reflect the
                // rendered byte buffer in the Y direction, and call that
                // after rendering, instead? Or maybe the tweaks needed aren't
                // that large anyway, and I just am mixing up my memory of the
                // trouble caused by that with the trouble caused by not
                // calling setBounds()...

                //createSub(NW);
                //createSub(NE);
                //createSub(SE);
                //createSub(SW);
            }
        }

        QuadTree(QuadTree rhs,
                 Bitmap bm)
        {
            this(bm, rhs.level, rhs.pageNumber, rhs.location);

            sub = rhs.sub;
        }

        QuadTree(Bitmap bm,
                 int pageNumber)
        {
            this(bm, 0, pageNumber, 0);
        }

        QuadTree(int level,
                 int pageNumber,
                 int location)
        {
            this.level = level;
            this.pageNumber = pageNumber;
            this.location = location;
            w = -1;
            h = -1;
        }

        void createSub(int q)
        {
            if (true) {
                ByteBuffer bb = renderPage(pageNumber, level+1, (location<<2)+q);
                Bitmap subbm = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
                subbm.copyPixelsFromBuffer(bb);
                sub[q] = new QuadTree(subbm, level+1, pageNumber, (location<<2)+q);
            } else {
                // Test... just use transparent single colour subtiles
                Bitmap subbm = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
                int color = 0;
                switch (q) {
                case NW: color = Color.RED; break;
                case NE: color = Color.GREEN; break;
                case SE: color = Color.BLUE; break;
                case SW: color = Color.YELLOW; break;
                }
                subbm.eraseColor(setColorAlpha(color, 0.5));
                sub[q] = new QuadTree(subbm, 1, pageNumber, (location<<2)+q);
            }
        }

        int quadrantOf(int x,
                       int y)
        {
            if (x < 0 || y < 0 || x >= w || y >= h)
                return -1;

            if (x < w/2 && y < h/2)
                return NW;
            if (x >= w/2 && y < h/2)
                return NE;
            if (x >= w/2  && y >= h/2)
                return SE;
            if (x < w/2 && y >= h/2)
                return SW;

            return -1;
        }

        int quadrantOf(Point p)
        {
            return quadrantOf(p.x, p.y);
        }

        Point subCoord(Point p)
        {
            return subCoord(quadrantOf(p), p);
        }

        Point subCoord(int q,
                       Point p)
        {
            switch (q) {
            case NW:
                return new Point(p.x*2, p.y*2);
            case NE:
                return new Point((p.x-w/2)*2, p.y*2);
            case SE:
                return new Point((p.x-w/2)*2, (p.y-h/2)*2);
            case SW:
                return new Point(p.x*2, (p.y-h/2)*2);
            }
            return null;
        }

        // The insert() stuff has not been tested and is just a draft
        void insert(Bitmap bm,
                    int level,
                    int x,
                    int y)
        {
            insert(bm, level, 1, new Point(x, y));
        }

        void insert(Bitmap bm,
                    int level,
                    int recursionDepth,
                    Point p)
        {
            int q = quadrantOf(p);

            if (q == -1)
                return;

            if (recursionDepth == level) {
                if (sub[q] == null)
                    sub[q] = new QuadTree(bm, level);
                else
                    sub[q] = new QuadTree(sub[q], bm);
            } else {
                if (sub[q] == null)
                    sub[q] = new QuadTree(this.level+1, pageNumber, location);
                sub[q].insert(bm, level, recursionDepth+1, subCoord(q, p));
            }
        }

        // Ditto for find()

        QuadTree find(int levelCountdown,
                      int x,
                      int y)
        {
            return find(levelCountdown, new Point(x, y));
        }

        QuadTree find(int levelCountdown,
                      Point p)
        {
            final int x = p.x, y = p.y;

            Log.i(TAG, "find(" + levelCountdown + ", (" + p.x + ", " + p.y + "))");

            if (x < 0 || y < 0 || x >= w || y >= h)
                return null;

            if (levelCountdown == 0) {
                Log.i(TAG, "Returning this at level " + this.level);
                return this;
            }

            int q = quadrantOf(p);

            if (sub[q] != null)
                return sub[q].find(levelCountdown-1, subCoord(q, p));

            return null;
        }

        void subDraw(Canvas canvas,
                     int q)
        {
            if (q == -1 || sub[q] == null)
                return;

            Log.i(TAG, "subDraw 1: q=" + q + ", matrix=" + canvas.getMatrix() + ", clip=" + canvas.getClipBounds());

            canvas.save();
            canvas.scale(0.5f, 0.5f);

            float[] values = new float[9];
            canvas.getMatrix().getValues(values);

            Log.i(TAG, "subDraw 2: q=" + q + ", matrix=" + canvas.getMatrix() + ", clip=" + canvas.getClipBounds() + ", translate(" +
                  (((q == NW || q == SW) ? -w : w) /* * values[Matrix.MSCALE_X]*/) + "," +
                  (((q == NW || q == NE) ? -h : h) /* * values[Matrix.MSCALE_X]*/) + ")");

            canvas.translate(((q == NW || q == SW) ? -w : 0) /* * values[Matrix.MSCALE_X]*/,
                             ((q == NW || q == NE) ? -h : 0) /* * values[Matrix.MSCALE_X]*/);

            Log.i(TAG, "subDraw 3: q=" + q + ", matrix=" + canvas.getMatrix() + ", clip=" + canvas.getClipBounds());

            sub[q].draw(canvas);

            canvas.restore();
        }

        @Override
            public void draw(Canvas canvas)
        {
            float[] values = new float[9];
            canvas.getMatrix().getValues(values);

            float scale = values[Matrix.MSCALE_X];
            int zoom = zoomLevel(scale);

            Log.i(TAG, "draw: level=" + level + ", scale=" + scale + ", zoom=" + zoom + ", matrix=" + canvas.getMatrix());
            Rect bounds = new Rect();
            if (canvas.getClipBounds(bounds))
                Log.i(TAG, "  clip=" + bounds + ", bounds=" + getBounds());
            else
                Log.i(TAG, "  no clip");

            int l = (int)(w/2 - values[Matrix.MTRANS_X]/scale);
            int t = (int)(h/2 - values[Matrix.MTRANS_Y]/scale);
            Log.i(TAG, "Unzoomed rect: " + l + ", " + t + ", " + (int)(l+w/scale) + ", " + (int)(t+h/scale));

            Log.i(TAG, "Scales: " + values[Matrix.MSCALE_X] + ", " + values[Matrix.MSCALE_Y]);

            // Assert.assertTrue(values[Matrix.MSCALE_X] == values[Matrix.MSCALE_Y]);

            super.draw(canvas);

            if (/*zoom > 0 */ scale >= 1) {
                subDraw(canvas, quadrantOf(l, t));
                subDraw(canvas, quadrantOf((int)(l+w/scale)-1, t));
                subDraw(canvas, quadrantOf((int)(l+w/scale)-1, (int)(t+h/scale)-1));
                subDraw(canvas, quadrantOf(l, (int)(t+h/scale)-1));
            }
        }
    }

    ByteBuffer renderPage(int number)
    {
        return renderPage(number, 0, 0);
    }

    ByteBuffer renderPage(final int number,
                          final int level,
                          final int location)
    {
        Log.i(TAG, "renderPage(" + number + ", " + level + ", " + location + ")");
        try {
            // Use dummySmallDevice with no scale or offset just to find out
            // the paper size of this page.

            PropertyValue renderProps[] = new PropertyValue[3];
            renderProps[0] = new PropertyValue();
            renderProps[0].Name = "IsPrinter";
            renderProps[0].Value = new Boolean(true);
            renderProps[1] = new PropertyValue();
            renderProps[1].Name = "RenderDevice";
            renderProps[1].Value = dummySmallDevice;
            renderProps[2] = new PropertyValue();
            renderProps[2].Name = "View";
            renderProps[2].Value = new MyXController();

            // getRenderer returns a set of properties that include the PageSize
            long t0 = System.currentTimeMillis();
            PropertyValue rendererProps[] = renderable.getRenderer(number, doc, renderProps);
            long t1 = System.currentTimeMillis();
            Log.i(TAG, "getRenderer took " + ((t1-t0)-timingOverhead) + " ms");

            int pageWidth = 0, pageHeight = 0;
            for (int i = 0; i < rendererProps.length; i++) {
                if (rendererProps[i].Name.equals("PageSize")) {
                    pageWidth = ((Size) rendererProps[i].Value).Width;
                    pageHeight = ((Size) rendererProps[i].Value).Height;
                    Log.i(TAG, "PageSize: " + pageWidth + "x" + pageHeight);
                }
            }

            // Create a new device with the correct scale and offset
            ByteBuffer bb = ByteBuffer.allocateDirect(flipper.getWidth()*flipper.getHeight()*4);
            long wrapped_bb = Bootstrap.new_byte_buffer_wrapper(bb);

            XDevice device;
            if (pageWidth == 0) {
                // Huh?
                device = toolkit.createScreenCompatibleDeviceUsingBuffer(flipper.getWidth(), flipper.getHeight(), 1, 1, 0, 0, wrapped_bb);
            } else {

                // Scale so that it fits our device which has a resolution of
                // 96/inch (see SvpSalGraphics::GetResolution()). The page
                // size returned from getRenderer() is in 1/mm * 100. 2540 is
                // one inch in mm/100.

                int scaleNumerator, scaleDenominator;

                // If the view has a wider aspect ratio than the page, fit
                // height; otherwise, fit width
                if ((double) flipper.getWidth() / flipper.getHeight() > (double) pageWidth / pageHeight) {
                    scaleNumerator = flipper.getHeight();
                    scaleDenominator = pageHeight / 2540 * 96;
                } else {
                    scaleNumerator = flipper.getWidth();
                    scaleDenominator = pageWidth / 2540 * 96;
                }
                scaleNumerator *= scaleOfZoom(level);

                int xOffset = 0, yOffset = 0;
                int hiX = pageWidth, hiY = pageHeight;
                int lvl = level;
                int loc = location;
                while (lvl > 0) {
                    int q = (loc & 0x03);

                    if (q == QuadTree.NE || q == QuadTree.SE) {
                        xOffset += (hiX - xOffset)/2;
                    } else {
                        hiX -= (hiX - xOffset)/2;
                    }
                    if (q == QuadTree.SW || q == QuadTree.SE) {
                        yOffset += (hiY - yOffset)/2;
                    } else {
                        hiY -= (hiY - yOffset)/2;
                    }
                    lvl--;
                    loc >>= 2;
                }

                // Seems that the offsets passed in (which get passed to
                // MapMode::SetOrigin() in
                // VirtualDevice::SetOutputSizePixelScaleOffsetAndBuffer()
                // are ignored... try a random value, no effect ;(
                xOffset = 12345; xOffset = 789;

                Log.i(TAG, "Rendering page " + number + " level=" + level + " scale=" + scaleNumerator + "/" + scaleDenominator + ", offset=(" + xOffset + ", " + yOffset + ")");

                device = toolkit.createScreenCompatibleDeviceUsingBuffer(flipper.getWidth(), flipper.getHeight(),
                                                                         scaleNumerator, scaleDenominator,
                                                                         -xOffset, -yOffset,
                                                                         wrapped_bb);
            }

            // Update the property that points to the device
            renderProps[1].Value = device;

            t0 = System.currentTimeMillis();
            renderable.render(number, doc, renderProps);
            t1 = System.currentTimeMillis();
            Log.i(TAG, "Rendering page " + number + " took " + ((t1-t0)-timingOverhead) + " ms");

            Bootstrap.force_full_alpha_bb(bb, 0, flipper.getWidth() * flipper.getHeight() * 4);

            return bb;
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            finish();
        }

        return null;
    }

    enum PageState { NONEXISTENT, LOADING, READY };

    class PageViewer
        extends ViewSwitcher
    {
        int currentPageNumber = -1;
        TextView waitView;
        PageState state = PageState.NONEXISTENT;
        Bitmap bm;

        class PageLoadTask
            extends AsyncTask<Integer, Void, Integer>
        {
            protected Integer doInBackground(Integer... params)
            {
                int number = params[0];

                Log.i(TAG, "doInBackground(" + number + ")");

                if (number >= pageCount)
                    return -1;

                state = PageState.LOADING;
                currentPageNumber = number;
                ByteBuffer bb = renderPage(currentPageNumber);
                bm = Bitmap.createBitmap(flipper.getWidth(), flipper.getHeight(), Bitmap.Config.ARGB_8888);
                bm.copyPixelsFromBuffer(bb);

                return currentPageNumber;
            }

            protected void onPostExecute(Integer result)
            {
                Log.i(TAG, "onPostExecute: " + result);
                if (result == -1)
                    return;

                GestureImageView imageView = new GestureImageView(DocumentLoader.this, gestureListener);
                imageView.setImageDrawable(new QuadTree(bm, result));

                imageView.setScaleY(-1);

                if (getChildCount() == 2)
                    removeViewAt(1);
                addView(imageView, 1, matchParent);
                showNext();
                state = PageState.READY;
            }
        }

        void display(int number)
        {
            Log.i(TAG, "PageViewer display(" + number + ")");
            if (number >= 0)
                waitView.setText("Page " + (number+1) + ", wait...");
            state = PageState.NONEXISTENT;

            if (getDisplayedChild() == 1) {
                showPrevious();
                removeViewAt(1);
            }

            if (number >= 0) {
                new PageLoadTask().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, number);
            }
        }

        PageViewer(int number)
        {
            super(DocumentLoader.this);

            waitView = new TextView(DocumentLoader.this);
            waitView.setTextSize(24);
            waitView.setGravity(Gravity.CENTER);
            waitView.setBackgroundColor(Color.WHITE);
            waitView.setTextColor(Color.BLACK);
            addView(waitView, 0, matchParent);

            display(number);
        }
    }

    class DocumentLoadTask
        extends AsyncTask<String, Void, Void>
    {
        protected Void doInBackground(String... params)
        {
            try {
                String url = params[0];
                Log.i(TAG, "Attempting to load " + url);

                PropertyValue loadProps[] = new PropertyValue[3];
                loadProps[0] = new PropertyValue();
                loadProps[0].Name = "Hidden";
                loadProps[0].Value = new Boolean(true);
                loadProps[1] = new PropertyValue();
                loadProps[1].Name = "ReadOnly";
                loadProps[1].Value = new Boolean(true);
                loadProps[2] = new PropertyValue();
                loadProps[2].Name = "Preview";
                loadProps[2].Value = new Boolean(true);

                long t0 = System.currentTimeMillis();
                doc = componentLoader.loadComponentFromURL(url, "_blank", 0, loadProps);
                long t1 = System.currentTimeMillis();
                Log.i(TAG, "Loading took " + ((t1-t0)-timingOverhead) + " ms");

                Object toolkitService = mcf.createInstanceWithContext
                    ("com.sun.star.awt.Toolkit", context);
                toolkit = (XToolkit2) UnoRuntime.queryInterface(XToolkit2.class, toolkitService);

                renderable = (XRenderable) UnoRuntime.queryInterface(XRenderable.class, doc);

                // Set up dummySmallDevice and use it to find out the number
                // of pages ("renderers").
                ByteBuffer smallbb = ByteBuffer.allocateDirect(SMALLSIZE*SMALLSIZE*4);
                long wrapped_smallbb = Bootstrap.new_byte_buffer_wrapper(smallbb);
                dummySmallDevice = toolkit.createScreenCompatibleDeviceUsingBuffer(SMALLSIZE, SMALLSIZE, 1, 1, 0, 0, wrapped_smallbb);

                PropertyValue renderProps[] = new PropertyValue[3];
                renderProps[0] = new PropertyValue();
                renderProps[0].Name = "IsPrinter";
                renderProps[0].Value = new Boolean(true);
                renderProps[1] = new PropertyValue();
                renderProps[1].Name = "RenderDevice";
                renderProps[1].Value = dummySmallDevice;
                renderProps[2] = new PropertyValue();
                renderProps[2].Name = "View";
                renderProps[2].Value = new MyXController();

                t0 = System.currentTimeMillis();
                pageCount = renderable.getRendererCount(doc, renderProps);
                t1 = System.currentTimeMillis();
                Log.i(TAG, "getRendererCount: " + pageCount + ", took " + ((t1-t0)-timingOverhead) + " ms");
            }
            catch (Exception e) {
                e.printStackTrace(System.err);
                finish();
            }
            return null;
        }
    }

    static void dumpUNOObject(String objectName, Object object)
    {
        Log.i(TAG, objectName + " is " + (object != null ? object.toString() : "null"));

        if (object == null)
            return;

        XTypeProvider typeProvider = (XTypeProvider)
            UnoRuntime.queryInterface(XTypeProvider.class, object);
        if (typeProvider == null)
            return;

        Type[] types = typeProvider.getTypes();
        if (types == null)
            return;

        for (Type t : types)
            Log.i(TAG, "  " + t.getTypeName());
    }

    static void dumpBytes(String name, byte[] bytes, int offset)
    {
        if (bytes == null) {
            Log.i(TAG, name + " is null");
            return;
        }
        Log.i(TAG, name + ":");

        if (offset != 0)
            Log.i(TAG, "  (offset " + offset + ")");

        for (int i = offset; i < Math.min(bytes.length, offset+160); i += 16) {
            String s = "";
            for (int j = i; j < Math.min(bytes.length, i+16); j++)
                s = s + String.format(" %02x", bytes[j]);

            Log.i(TAG, s);
        }
    }

    static void dumpBytes(String name, ByteBuffer bytes, int offset)
    {
        if (bytes == null) {
            Log.i(TAG, name + " is null");
            return;
        }
        Log.i(TAG, name + ":");

        if (offset != 0)
            Log.i(TAG, "  (offset " + offset + ")");

        for (int i = offset; i < Math.min(bytes.limit(), offset+160); i += 16) {
            String s = "";
            for (int j = i; j < Math.min(bytes.limit(), i+16); j++)
                s = s + String.format(" %02x", bytes.get(j));

            Log.i(TAG, s);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        extras = getIntent().getExtras();

        gestureListener = new GestureListener();
        gestureDetector = new GestureDetector(this, gestureListener);

        try {
            long t0 = System.currentTimeMillis();
            long t1 = System.currentTimeMillis();
            timingOverhead = t1 - t0;

            Bootstrap.setup(this);

            // Avoid all the old style OSL_TRACE calls especially in vcl
            Bootstrap.putenv("SAL_LOG=+WARN+INFO");

            // Log.i(TAG, "Sleeping NOW");
            // Thread.sleep(20000);

            context = com.sun.star.comp.helper.Bootstrap.defaultBootstrap_InitialComponentContext();

            Log.i(TAG, "context is" + (context!=null ? " not" : "") + " null");

            mcf = context.getServiceManager();

            Log.i(TAG, "mcf is" + (mcf!=null ? " not" : "") + " null");

            String input = getIntent().getStringExtra("input");
            if (input == null)
                input = "/assets/test1.odt";

            // We need to fake up an argv, and the argv[0] even needs to
            // point to some file name that we can pretend is the "program".
            // setCommandArgs() will prefix argv[0] with the app's data
            // directory.

            String[] argv = { "lo-document-loader", input };

            Bootstrap.setCommandArgs(argv);

            Bootstrap.initVCL();

            Object desktop = mcf.createInstanceWithContext
                ("com.sun.star.frame.Desktop", context);

            Log.i(TAG, "desktop is" + (desktop!=null ? " not" : "") + " null");

            componentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, desktop);

            Log.i(TAG, "componentLoader is" + (componentLoader!=null ? " not" : "") + " null");

            // Load the wanted document
            new DocumentLoadTask().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, "file://" + input);

            flipper = new ViewFlipper(this);

            matchParent = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

            flipper.addView(new PageViewer(0), 0, matchParent);
            for (int i = 0; i < PAGECACHE_PLUSMINUS; i++)
                flipper.addView(new PageViewer(i+1), i+1, matchParent);
            for (int i = 0; i < PAGECACHE_PLUSMINUS; i++)
                flipper.addView(new PageViewer(-1), PAGECACHE_PLUSMINUS + i+1, matchParent);

            setContentView(flipper);
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            finish();
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        return gestureDetector.onTouchEvent(event);
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
