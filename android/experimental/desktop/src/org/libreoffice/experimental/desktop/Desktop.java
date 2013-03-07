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

    /* In desktop */
    private static native void runMain();

    /* In vcl */
    public static native void renderVCL(Bitmap bitmap);
    public static native void setViewSize(int width, int height);
    public static native void key(char c);
    public static native void touch(int action, int x, int y);
    public static native void zoom(float scale, int x, int y);
    public static native void scroll(int x, int y);

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

    // This sucks, we need to experiment and think, can an app process
    // have several instances of this Activity active?
    static BitmapView theView;

    // This is called back from LO in the LO thread
    static public void callbackDamaged()
    {
        synchronized (theView) {
            if (!invalidatePosted)
                theView.post(new Runnable() {
                        @Override public void run() {
                            synchronized (theView) {
                                theView.invalidate();
                                invalidatePosted = false;
                            }
                        }
                    });
            invalidatePosted = true;
        }
    }
    static boolean invalidatePosted;

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
            theView = new BitmapView();
            setContentView(theView);

            // Start a Java thread to run soffice_main(). We don't
            // want to start the thread from native code becauce
            // native threads apparently have no Java class loaders in
            // Android, or someghin. So for instance FindClass fails.

            // See https://groups.google.com/group/android-ndk/msg/a0793f009e6e71f7?dmode=source
            // .

            new Thread(new Runnable() {
                    @Override public void run() {
                        runMain();
                    }
                }).start();
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

        GestureDetector gestureDetector;
        ScaleGestureDetector scaleDetector;

        boolean scrollInProgress, scalingInProgress;
        float translateX = 0, translateY = 0;
        float accumulatedScale = 1;
        float pivotX = 0, pivotY = 0;

        public BitmapView()
        {
            super(Desktop.this);
            setFocusableInTouchMode(true);

            gestureDetector =
                new GestureDetector(Desktop.this,
                                    new GestureDetector.SimpleOnGestureListener() {
                                        @Override public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
                                        {
                                            Log.i(TAG, "onFling: (" + velocityX + ", " + velocityY + ")");
                                            return false;
                                        }

                                        @Override public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY)
                                        {
                                            Log.i(TAG, "onScroll: (" + distanceX + ", " + distanceY + ")");
                                            translateX += -distanceX;
                                            translateY += -distanceY;
                                            scrollInProgress = true;
                                            invalidate();
                                            return true;
                                        }
                                    });

            scaleDetector =
                new ScaleGestureDetector(Desktop.this,
                                         new ScaleGestureDetector.SimpleOnScaleGestureListener() {

                                             @Override public boolean onScaleBegin(ScaleGestureDetector detector)
                                             {
                                                 scalingInProgress = true;
                                                 return true;
                                             }

                                             @Override public boolean onScale(ScaleGestureDetector detector)
                                             {
                                                 accumulatedScale *= detector.getScaleFactor();
                                                 pivotX = detector.getFocusX();
                                                 pivotY = detector.getFocusY();
                                                 invalidate();
                                                 return true;
                                             }

                                             @Override public void onScaleEnd(ScaleGestureDetector detector)
                                             {
                                                 accumulatedScale *= detector.getScaleFactor();
                                                 Desktop.zoom(accumulatedScale, (int) pivotX, (int) pivotY);
                                                 accumulatedScale = 1;
                                                 pivotX = pivotY = 0;
                                                 scalingInProgress = false;
                                                 invalidate();
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
            if (scrollInProgress) {
                canvas.save();
                canvas.translate(translateX, translateY);
                canvas.drawBitmap(mBitmap, 0, 0, null);
                canvas.restore();
            } else if (scalingInProgress) {
                canvas.save();
                canvas.scale(accumulatedScale, accumulatedScale, pivotX, pivotY);
                canvas.drawBitmap(mBitmap, 0, 0, null);
                canvas.restore();
            } else {
                canvas.drawBitmap(mBitmap, 0, 0, null);
            }
            renderedOnce = true;
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
                Desktop.key((char) ('0' + keyCode - KeyEvent.KEYCODE_0));
                return true;
            case KeyEvent.KEYCODE_DEL:
                Desktop.key((char) Key.BACKSPACE);
                return true;
            case KeyEvent.KEYCODE_ENTER:
                Desktop.key((char) Key.RETURN);
                return true;
            case KeyEvent.KEYCODE_TAB:
                Desktop.key((char) Key.TAB);
                return true;
            default:
                return false;
            }
        }

        @Override public boolean onTouchEvent(MotionEvent event)
        {
            if (event.getPointerCount() == 1 &&
                gestureDetector.onTouchEvent(event)) {
                return true;
            }

            // There is no callback in SimpleOnGestureListener for end
            // of scroll. Is this a good way to detect it? Assume that
            // as long as the scrolling gesture is in progress, the
            // Gesturedetector.onTouchEvent() will keep returning
            // true, so if scrollInProgress is true and we get here,
            // the scroll must have ended.

            if (scrollInProgress) {
                Desktop.scroll((int) translateX, (int) translateY);
                translateX = translateY = 0;
                scrollInProgress = false;
                invalidate();
            } else if (event.getPointerCount() == 2 &&
                       scaleDetector.onTouchEvent(event) &&
                       scalingInProgress) {
                // If a scaling gesture is in progress no other touch
                // processing should be done.
                return true;
            }

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

            if (event.getPointerCount() == 1) {
                switch (event.getActionMasked()) {
                case MotionEvent.ACTION_DOWN:
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_MOVE:
                    Desktop.touch(event.getActionMasked(), (int) event.getX(), (int) event.getY());
                    break;
                }
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
                Desktop.key(text.charAt(i));
            }
            return true;
        }
    }

}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
