// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-

// Version: MPL 1.1 / GPLv3+ / LGPLv3+
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License or as specified alternatively below. You may obtain a copy of
// the License at http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// Major Contributor(s):
// Copyright (C) 2011 Tor Lillqvist <tml@iki.fi> (initial developer)
// Copyright (C) 2011 SUSE Linux http://suse.com (initial developer's employer)
//
// All Rights Reserved.
//
// For minor contributions see the git repository.
//
// Alternatively, the contents of this file may be used under the terms of
// either the GNU General Public License Version 3 or later (the "GPLv3+"), or
// the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
// in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
// instead of those above.

package org.libreoffice.android.examples;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.TranslateAnimation;
import android.widget.ImageView;
import android.widget.ViewFlipper;

import com.polites.android.GestureImageView;

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
    private static String TAG = "DocumentLoader";

    long timingOverhead;
    Object desktop;
    XComponentLoader componentLoader;
    XToolkit2 toolkit;
    Object doc;
    int pageCount;
    XRenderable renderable;

    PropertyValue[] loadProps;

    GestureDetector gestureDetector;
    ViewFlipper flipper;

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
                AnimationSet leftIn = new AnimationSet(true);
                leftIn.addAnimation(new AlphaAnimation(0.1f, 1.0f));
                leftIn.addAnimation(new TranslateAnimation(Animation.RELATIVE_TO_SELF, 1, Animation.RELATIVE_TO_SELF, 0,
                                                           Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0));
                leftIn.setDuration(500);
                flipper.setInAnimation(leftIn);

                AnimationSet leftOut = new AnimationSet(true);
                leftOut.addAnimation(new AlphaAnimation(1f, 0.1f));
                leftOut.addAnimation(new TranslateAnimation(Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, -1,
                                                            Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0));
                leftOut.setDuration(500);
                flipper.setOutAnimation(leftOut);

                flipper.showNext();
                return true;
            } else if (event2.getX() - event1.getX() > 120) {
                AnimationSet rightIn = new AnimationSet(true);
                rightIn.addAnimation(new AlphaAnimation(0.1f, 1.0f));
                rightIn.addAnimation(new TranslateAnimation(Animation.RELATIVE_TO_SELF, -1, Animation.RELATIVE_TO_SELF, 0,
                                                            Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0));
                rightIn.setDuration(500);
                flipper.setInAnimation(rightIn);

                AnimationSet rightOut = new AnimationSet(true);
                rightOut.addAnimation(new AlphaAnimation(1f, 0.1f));
                rightOut.addAnimation(new TranslateAnimation(Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 1,
                                                             Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0));
                rightOut.setDuration(500);
                flipper.setOutAnimation(rightOut);

                flipper.showPrevious();
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

    enum PageState { LOADING, READY };

    ByteBuffer renderPage(int number)
    {
        ByteBuffer bb;

        bb = ByteBuffer.allocateDirect(1024*1024*4);
        long wrapped_bb = Bootstrap.new_byte_buffer_wrapper(bb);
        Log.i(TAG, "bb is " + bb);
        XDevice device = toolkit.createScreenCompatibleDeviceUsingBuffer(1024, 1024, wrapped_bb);

        dumpUNOObject("device", device);

        PropertyValue renderProps[] = new PropertyValue[3];
        renderProps[0] = new PropertyValue();
        renderProps[0].Name = "IsPrinter";
        renderProps[0].Value = new Boolean(true);
        renderProps[1] = new PropertyValue();
        renderProps[1].Name = "RenderDevice";
        renderProps[1].Value = device;
        renderProps[2] = new PropertyValue();
        renderProps[2].Name = "View";
        renderProps[2].Value = new MyXController();

        try {
            long t0 = System.currentTimeMillis();
            renderable.render(number, doc, renderProps);
            long t1 = System.currentTimeMillis();
            Log.i(TAG, "Rendering page " + number + " took " + ((t1-t0)-timingOverhead) + " ms");
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }

        Bootstrap.force_full_alpha_bb(bb, 0, 1024 * 1024 * 4);

        return bb;
    }

    class Page
    {
        int number;
        PageState state = PageState.LOADING;
        ByteBuffer bb;

        Page(int number)
        {
            this.number = number;

            bb = renderPage(number);

            state = PageState.READY;
        }
    }

    static void dumpUNOObject(String objectName, Object object)
    {
        Log.i(TAG, objectName + " is " + (object != null ? object.toString() : "null"));

        if (object == null)
            return;

        XTypeProvider typeProvider = (XTypeProvider)
            UnoRuntime.queryInterface(XTypeProvider.class, object);

        Log.i(TAG, "typeProvider is " + (typeProvider != null ? typeProvider.toString() : "null"));

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

        gestureDetector = new GestureDetector(this, new GestureListener());

        try {
            long t0 = System.currentTimeMillis();
            long t1 = System.currentTimeMillis();
            timingOverhead = t1 - t0;

            Bootstrap.setup(this);

            Bootstrap.putenv("SAL_LOG=yes");

            // Load a lot of shlibs here explicitly in advance because that
            // makes debugging work better, sigh
            Bootstrap.dlopen("libvcllo.so");
            Bootstrap.dlopen("libmergedlo.so");
            Bootstrap.dlopen("libswdlo.so");
            Bootstrap.dlopen("libswlo.so");
            
            Log.i(TAG, "Sleeping NOW");
            Thread.sleep(20000);

            XComponentContext context = null;

            context = com.sun.star.comp.helper.Bootstrap.defaultBootstrap_InitialComponentContext();

            Log.i(TAG, "context is" + (context!=null ? " not" : "") + " null");

            XMultiComponentFactory mcf = context.getServiceManager();

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

            Object oDesktop = mcf.createInstanceWithContext
                ("com.sun.star.frame.Desktop", context);

            Log.i(TAG, "oDesktop is" + (oDesktop!=null ? " not" : "") + " null");

            Bootstrap.initUCBHelper();

            XComponentLoader xCompLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, oDesktop);

            Log.i(TAG, "xCompLoader is" + (xCompLoader!=null ? " not" : "") + " null");

            // Load the wanted document

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

            String sUrl = "file://" + input;

            Log.i(TAG, "Attempting to load " + sUrl);

            t0 = System.currentTimeMillis();
            doc = xCompLoader.loadComponentFromURL(sUrl, "_blank", 0, loadProps);
            t1 = System.currentTimeMillis();
            Log.i(TAG, "Loading took " + ((t1-t0)-timingOverhead) + " ms");

            dumpUNOObject("doc", doc);

            Object toolkitService = mcf.createInstanceWithContext
                ("com.sun.star.awt.Toolkit", context);

            dumpUNOObject("toolkitService", toolkitService);

            toolkit = (XToolkit2) UnoRuntime.queryInterface(XToolkit2.class, toolkitService);
            dumpUNOObject("toolkit", toolkit);

            renderable = (XRenderable) UnoRuntime.queryInterface(XRenderable.class, doc);

            ByteBuffer smallbb = ByteBuffer.allocateDirect(128*128*4);
            long wrapped_smallbb = Bootstrap.new_byte_buffer_wrapper(smallbb);
            XDevice smalldevice = toolkit.createScreenCompatibleDeviceUsingBuffer(128, 128, wrapped_smallbb);

            PropertyValue renderProps[] = new PropertyValue[3];
            renderProps[0] = new PropertyValue();
            renderProps[0].Name = "IsPrinter";
            renderProps[0].Value = new Boolean(true);
            renderProps[1] = new PropertyValue();
            renderProps[1].Name = "RenderDevice";
            renderProps[1].Value = smalldevice;
            renderProps[2] = new PropertyValue();
            renderProps[2].Name = "View";
            renderProps[2].Value = new MyXController();

            pageCount = renderable.getRendererCount(doc, renderProps);
            Log.i(TAG, "getRendererCount: " + pageCount);

            flipper = new ViewFlipper(this);

            flipper.setScaleY(-1);

            for (int i = 0; i < pageCount; i++) {
                ByteBuffer bb = renderPage(i);
                Bitmap bm = Bitmap.createBitmap(1024, 1024, Bitmap.Config.ARGB_8888);
                bm.copyPixelsFromBuffer(bb);

                ImageView imageView = new ImageView(this);
                imageView.setImageBitmap(bm);

                flipper.addView(imageView, i, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
            }

            setContentView(flipper);
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        return gestureDetector.onTouchEvent(event);
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
