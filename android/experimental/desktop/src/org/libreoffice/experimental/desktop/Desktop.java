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
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import com.sun.star.awt.Key;

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
    public static native void touch(int action, int x, int y, short timestamp);
    public static native void zoom(float scale, int x, int y);

    /**
     * This class contains the state that is initialized once and never changes
     * (not specific to a document or a view).
     */
    class BootstrapContext
    {
    }

    BootstrapContext bootstrapContext;

    private void initBootstrapContext()
    {
        bootstrapContext = new BootstrapContext();

        Bootstrap.setup(this);

        Bootstrap.putenv("SAL_LOG=+WARN+INFO");
    }

    @Override public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Log.i(TAG, "onCreate");

        try {
            String input;
//            input = getIntent().getStringExtra("input");
//            if (input == null)
            input = "/assets/test1.odt";
//          input = "--writer";

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

    static short getTimestamp()
    {
        return (short) (System.currentTimeMillis() % Short.MAX_VALUE);
    }

    class BitmapView
        extends View
    {
        Bitmap mBitmap;
        boolean renderedOnce;
        boolean scalingInProgress;
        GestureDetector gestureDetector;
        ScaleGestureDetector scaleDetector;
        long lastGestureEventTime;

        public BitmapView()
        {
            super(Desktop.this);
            setFocusableInTouchMode(true);

            gestureDetector =
                new GestureDetector(Desktop.this,
                                    new GestureDetector.SimpleOnGestureListener() {
                                        @Override public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
                                        {
                                            Log.i(TAG, "onFling: events:" + e1 + ", " + e2 + ", velocity: (" + velocityX + ", " + velocityY + ")");
                                            return false;
                                        }

                                        @Override public boolean onScroll(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
                                        {
                                            Log.i(TAG, "onScroll: events:" + e1 + ", " + e2 + ", velocity: (" + velocityX + ", " + velocityY + ")");
                                            return false;
                                        }
                                    });

            // Is this sane? It is rather slow to ask LO to zoom
            // continuously while the scaling gesture is in progress.

            // What we used to do was while a scale gesture was in
            // progress to just scale the bitmap view (UI elements
            // too, which of course was a bit silly).

            scaleDetector =
                new ScaleGestureDetector(Desktop.this,
                                         new ScaleGestureDetector.SimpleOnScaleGestureListener() {
                                             @Override public boolean onScaleBegin(ScaleGestureDetector detector)
                                             {
                                                 scalingInProgress = true;
                                                 lastGestureEventTime = System.currentTimeMillis();
                                                 return true;
                                             }

                                             @Override public boolean onScale(ScaleGestureDetector detector)
                                             {
                                                 long now = System.currentTimeMillis();
                                                 if (now - lastGestureEventTime < 100)
                                                     return false;
                                                 float scale = detector.getScaleFactor();
                                                 if (scale > 0.95 && scale < 1.05)
                                                     return false;
                                                 Log.i(TAG, "onScale: " + scale);
                                                 lastGestureEventTime = now;
                                                 Desktop.zoom(scale, (int) detector.getFocusX(), (int) detector.getFocusY());
                                                 return true;
                                             }

                                             @Override public void onScaleEnd(ScaleGestureDetector detector)
                                             {
                                                 float scale = detector.getScaleFactor();
                                                 Log.i(TAG, "onScaleEnd: " + scale);
                                                 if (!(scale > 0.95 && scale < 1.05))
                                                     Desktop.zoom(scale, (int) detector.getFocusX(), (int) detector.getFocusY());
                                                 scalingInProgress = false;
                                             }
                                         });
        }

        @Override protected void onDraw(Canvas canvas)
        {
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

        @Override public boolean onKeyDown(int keyCode, KeyEvent event)
        {
            switch (keyCode) {
            case KeyEvent.KEYCODE_0:
            case KeyEvent.KEYCODE_1:
            case KeyEvent.KEYCODE_2:
            case KeyEvent.KEYCODE_3:
            case KeyEvent.KEYCODE_4:
            case KeyEvent.KEYCODE_5:
            case KeyEvent.KEYCODE_6:
            case KeyEvent.KEYCODE_7:
            case KeyEvent.KEYCODE_8:
            case KeyEvent.KEYCODE_9:
                Desktop.key((char) ('0' + keyCode - KeyEvent.KEYCODE_0), Desktop.getTimestamp());
                return true;
            case KeyEvent.KEYCODE_DEL:
                Desktop.key((char) Key.BACKSPACE, Desktop.getTimestamp());
                return true;
            case KeyEvent.KEYCODE_ENTER:
                Desktop.key((char) Key.RETURN, Desktop.getTimestamp());
                return true;
            case KeyEvent.KEYCODE_TAB:
                Desktop.key((char) Key.TAB, Desktop.getTimestamp());
                return true;
            default:
                return false;
            }
        }

        @Override public boolean onTouchEvent(MotionEvent event)
        {
            if (gestureDetector.onTouchEvent(event))
                return true;

            // If a scaling gesture is in progress no other touch
            // processing should be done.
            if (scaleDetector.onTouchEvent(event) && scalingInProgress)
                return true;

            if (!renderedOnce)
                return super.onTouchEvent(event);

            super.onTouchEvent(event);

            // Just temporary hack. We should not show the keyboard
            // unconditionally on a ACTION_UP event here. The LO level
            // should callback to us requesting showing the keyboard
            // if the user taps in a text area. Also, if the device
            // has a hardware keyboard, we probably should not show
            // the soft one unconditionally? But what if the user
            // wants to input in another script than what the hardware
            // keyboard covers?
            if (event.getActionMasked() == MotionEvent.ACTION_UP) {
                // show the keyboard so we can enter text
                InputMethodManager imm = (InputMethodManager) getContext()
                    .getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.showSoftInput(this, InputMethodManager.SHOW_FORCED);
            }

            switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_MOVE:
                Desktop.touch(event.getActionMasked(), (int) event.getX(), (int) event.getY(), Desktop.getTimestamp());
                break;
            }

            return true;
        }

        @Override public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
            BaseInputConnection fic = new LOInputConnection(this, true);
            outAttrs.actionLabel = null;
            outAttrs.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
            outAttrs.imeOptions = EditorInfo.IME_ACTION_NONE;

            return fic;
        }

        @Override public boolean onCheckIsTextEditor() {
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
            for (int i = 0; i < text.length(); i++) {
                Desktop.key(text.charAt(i), Desktop.getTimestamp());
            }
            return true;
        }
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
