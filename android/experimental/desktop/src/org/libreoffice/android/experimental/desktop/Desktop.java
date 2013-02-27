// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This is just a testbed for ideas and implementations. (Still, it might turn
// out to be somewhat useful as such while waiting for "real" apps.)

package org.libreoffice.experimental.desktop;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Bundle;
import android.text.InputType;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;

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
    public static native void key(char c, short timestamp);

    /**
     * This class contains the state that is initialized once and never changes
     * (not specific to a document or a view).
     */
    class BootstrapContext
    {
        public long timingOverhead;
        public XComponentContext componentContext;
        public XMultiComponentFactory mcf;
    }

    BootstrapContext bootstrapContext;
    Bundle extras;

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

    @Override public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Log.i(TAG, "onCreate");

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

            // To enable the sleep below, do: "adb shell setprop
            // log.tag.LODesktopSleepOnCreate VERBOSE". Yeah, has
            // nothing to do with logging as such.

            // This should be after at least one call to something in
            // the Bootstrap class as it is the static initialiser
            // that loads the lo-native-code library, and presumably
            // in ndk-gdb you want to set a breapoint in some native
            // code...

            if (Log.isLoggable("LODesktopSleepOnCreate", Log.VERBOSE)) {
                Log.i(TAG, "Sleeping, start ndk-gdb NOW if you intend to debug");
                Thread.sleep(20000);
            }

            if (bootstrapContext == null)
                initBootstrapContext();

            Log.i(TAG, "onCreate - set content view");
            setContentView(new BitmapView());

            spawnMain();
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            finish();
        }
    }

    class BitmapView
        extends View
    {
        Bitmap mBitmap;
        boolean renderedOnce;

        public BitmapView()
        {
            super(Desktop.this);
            setFocusableInTouchMode(true);
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
            renderedOnce = true;

            // re-call ourselves a bit later ...
            invalidate();
        }

        @Override public boolean onTouchEvent(MotionEvent event)
        {
            if (!renderedOnce)
                return super.onTouchEvent(event);

            super.onTouchEvent(event);
            Log.d(TAG, "onTOUCH");
            if (event.getAction() == MotionEvent.ACTION_UP) {
                // show the keyboard so we can enter text
                InputMethodManager imm = (InputMethodManager) getContext()
                    .getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.showSoftInput(this, InputMethodManager.SHOW_FORCED);
            }
            return true;
        }

        @Override public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
            Log.d(TAG, "onCreateInputConnection");

            BaseInputConnection fic = new LOInputConnection(this, true);
            outAttrs.actionLabel = null;
            outAttrs.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
            outAttrs.imeOptions = EditorInfo.IME_ACTION_NONE;

            return fic;
        }

        @Override public boolean onCheckIsTextEditor() {
            Log.d(TAG, "onCheckIsTextEditor");
            return renderedOnce;
        }
    }

    class LOInputConnection
        extends BaseInputConnection
    {
        public LOInputConnection(View targetView, boolean fullEditor) {
            super(targetView, fullEditor);
        }

        @Override public boolean commitText(CharSequence text, int newCursorPosition) {
            Log.i(TAG, "commitText(" + text + ", " + newCursorPosition + ")");
            short timestamp = (short) (System.currentTimeMillis() % Short.MAX_VALUE);
            for (int i = 0; i < text.length(); i++) {
                Desktop.key(text.charAt(i), timestamp);
            }
            return true;
        }
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
