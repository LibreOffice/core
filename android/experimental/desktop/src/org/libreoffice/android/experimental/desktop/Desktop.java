// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This is just a testbed for ideas and implementations. (Still, it might turn
// out to be somewhat useful as such while waiting for "real" apps.)

// ================ NOTE ================
// Note that these comments are copy-pasted from another test app,
// DocumentLoader, and might or might not be much relevant for this
// source file and how it will evolve.

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

package org.libreoffice.experimental.desktop;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
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
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.TranslateAnimation;
import android.widget.ImageView;
import android.widget.NumberPicker;
import android.widget.TextView;
import android.widget.ViewFlipper;
import android.widget.ViewSwitcher;

import junit.framework.Assert;

import com.sun.star.awt.Size;
import com.sun.star.awt.XBitmap;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XToolkitExperimental;
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
import java.util.ArrayList;

import org.libreoffice.android.Bootstrap;

public class Desktop
    extends Activity
{
    private static final String TAG = "LODesktop";

    /* implementend by desktop */
    private static native void spawnMain();

    /* implementend by vcl */
    public static native void renderVCL(Bitmap bitmap);
    public static native void setViewSize(int width, int height);

    /**
     * This class contains the state that is initialized once and never changes
     * (not specific to a document or a view).
     */
    class BootstrapContext
    {
        public long timingOverhead;
        public XComponentContext componentContext;
        public XMultiComponentFactory mcf;
        public XComponentLoader componentLoader;
        public XToolkitExperimental toolkit;
        public XDevice dummySmallDevice;
    }

    BootstrapContext bootstrapContext;
    Bundle extras;

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

    // FIXME: we should prolly manage a bitmap buffer here and give it to
    // VCL to render into ... and pull the WM/stacking pieces up into the Java ...
    // [ perhaps ;-]
    // how can we get an event to "create a window" - need a JNI callback I guess ...

    private void initBootstrapContext()
    {
        try
        {
            bootstrapContext = new BootstrapContext();

            long t0 = System.currentTimeMillis();
            long t1 = System.currentTimeMillis();
            bootstrapContext.timingOverhead = t1 - t0;

            Bootstrap.setup(this);

            // Avoid all the old style OSL_TRACE calls especially in vcl
            Bootstrap.putenv("SAL_LOG=+WARN+INFO");

            // Log.i(TAG, "Sleeping NOW");
            // Thread.sleep(20000);

            bootstrapContext.componentContext = com.sun.star.comp.helper.Bootstrap.defaultBootstrap_InitialComponentContext();

            Log.i(TAG, "context is" + (bootstrapContext.componentContext!=null ? " not" : "") + " null");

            bootstrapContext.mcf = bootstrapContext.componentContext.getServiceManager();

            Log.i(TAG, "mcf is" + (bootstrapContext.mcf!=null ? " not" : "") + " null");
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            finish();
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Log.i(TAG, "onCreate - added here\n");

        try {
            String input;
//            input = getIntent().getStringExtra("input");
//            if (input == null)
//            input = "/assets/test1.odt";
            input = "-writer";

            // We need to fake up an argv, and the argv[0] even needs to
            // point to some file name that we can pretend is the "program".
            // setCommandArgs() will prefix argv[0] with the app's data
            // directory.

            String[] argv = { "lo-document-loader", input };

            Bootstrap.setCommandArgs(argv);

            if (bootstrapContext == null)
                initBootstrapContext();

            Log.i(TAG, "onCreate - set content view\n");
            setContentView(new BitmapView());

            spawnMain();
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            finish();
        }
    }

    class BitmapView extends android.view.View
    {
        Bitmap mBitmap;

        public BitmapView()
        {
            super(Desktop.this);
        }

        @Override protected void onDraw(Canvas canvas)
        {
//            canvas.drawColor(0xFF1ABCDD);

            if (mBitmap == null) {
                Log.i(TAG, "calling Bitmap.createBitmap(" + getWidth() + ", " + getHeight() + ", Bitmap.Config.ARGB_8888)");
                mBitmap = Bitmap.createBitmap(getWidth(), getHeight(), Bitmap.Config.ARGB_8888);
                setViewSize(getWidth(), getHeight());
            }
            renderVCL(mBitmap);
            canvas.drawBitmap(mBitmap, 0, 0, null);

            // re-call ourselves a bit later ...
            invalidate();
        }
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
