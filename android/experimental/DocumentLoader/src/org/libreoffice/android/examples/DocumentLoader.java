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
import android.widget.ImageView;

import com.polites.android.GestureImageView;

import com.sun.star.awt.XBitmap;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XToolkit;
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
    extends Activity {

    private static String TAG = "DocumentLoader";

    class MyXController
        implements XController {

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

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        try {

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

            XComponentContext xContext = null;

            xContext = com.sun.star.comp.helper.Bootstrap.defaultBootstrap_InitialComponentContext();

            Log.i(TAG, "xContext is" + (xContext!=null ? " not" : "") + " null");

            XMultiComponentFactory xMCF = xContext.getServiceManager();

            Log.i(TAG, "xMCF is" + (xMCF!=null ? " not" : "") + " null");

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

            Object oDesktop = xMCF.createInstanceWithContext
                ("com.sun.star.frame.Desktop", xContext);

            Log.i(TAG, "oDesktop is" + (oDesktop!=null ? " not" : "") + " null");

            Bootstrap.initUCBHelper();

            XComponentLoader xCompLoader = (XComponentLoader)
                UnoRuntime.queryInterface(XComponentLoader.class, oDesktop);

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

            Object oDoc =
                xCompLoader.loadComponentFromURL
                (sUrl, "_blank", 0, loadProps);

            dumpUNOObject("oDoc", oDoc);

            Object toolkit = xMCF.createInstanceWithContext
                ("com.sun.star.awt.Toolkit", xContext);

            dumpUNOObject("toolkit", toolkit);

            XToolkit xToolkit = (XToolkit)
                UnoRuntime.queryInterface(XToolkit.class, toolkit);

            XDevice device = xToolkit.createScreenCompatibleDevice(1024, 1024);

            dumpUNOObject("device", device);

            XRenderable renderBabe = (XRenderable)
                UnoRuntime.queryInterface(XRenderable.class, oDoc);

            PropertyValue renderProps[] =
                new PropertyValue[3];
            renderProps[0] = new PropertyValue();
            renderProps[0].Name = "IsPrinter";
            renderProps[0].Value = new Boolean(true);
            renderProps[1] = new PropertyValue();
            renderProps[1].Name = "RenderDevice";
            renderProps[1].Value = device;
            renderProps[2] = new PropertyValue();
            renderProps[2].Name = "View";
            renderProps[2].Value = new MyXController();

            Log.i(TAG, "getRendererCount: " + renderBabe.getRendererCount(oDoc, renderProps));

            renderBabe.render(0, oDoc, renderProps);

            XBitmap bitmap = device.createBitmap(0, 0, 1024, 1024);

            byte[] image = bitmap.getDIB();

            dumpBytes("image", image, 0);

            if (image[0] != 'B' || image[1] != 'M') {
                Log.wtf(TAG, "getDIB() didn't return a BMP file");
                return;
            }

            ByteBuffer imagebb = ByteBuffer.wrap(image);
            imagebb.order(ByteOrder.LITTLE_ENDIAN);

            if (imagebb.getInt(0x0e) != 40) {
                Log.wtf(TAG, "getDIB() didn't return a DIB with BITMAPINFOHEADER");
                return;
            }

            if (imagebb.getShort(0x1c) != 32) {
                Log.wtf(TAG, "getDIB() didn't return a 32 bpp DIB");
                return;
            }

            if (imagebb.getInt(0x1e) != 3) {
                Log.wtf(TAG, "getDIB() didn't return a BI_BITFIELDS DIB");
                return;
            }

            if (imagebb.getInt(0x36) != 0x000000ff |
                imagebb.getInt(0x3a) != 0x0000ff00 ||
                imagebb.getInt(0x3e) != 0x00ff0000) {
                Log.wtf(TAG, "getDIB() didn't return DIB in RGBX format");
                return;
            }

            int offset = imagebb.getInt(0x0a);
            int width = imagebb.getInt(0x12);
            int height = imagebb.getInt(0x16);

            Log.i(TAG, String.format("offset: %d (%x), width: %d, height: %d", offset, offset, width, height));

            Bootstrap.force_full_alpha(image, offset, width * height * 4);

            Log.i(TAG, "after force_full_alpha:");
            dumpBytes("image", image, 0);

            for (int i = offset;  i < offset + width * height * 4; i++) {
                if (image[i] != -1) {
                    int o = offset + (((i-offset) - 4) / 4) * 4;
                    dumpBytes("First non-ones bytes", image, o);
                    o += 160;
                    dumpBytes("...", image, o);
                    o += 160;
                    dumpBytes("...", image, o);
                    o += 160;
                    dumpBytes("...", image, o);
                    break;
                }
            }

            ImageView imageView = new GestureImageView(this);
            imageView.setScaleY(-1);

            Bitmap bm = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            imagebb.position(offset);

            // Thanks to Android bug 32588, the above is not enough to get the
            // below copyPixelsFromBuffer() to start copying at offset, it
            // will (as of Android 4.0.3) copy from position zero anyway. So
            // instead have to shift (compact) the bloody buffer.
            imagebb.compact();

            // I don't understand what the compact() documentation says about
            // the new position; so explicitly put it at zero, in case
            // runnning on an Android where copyPixelsFromBuffer() *does* take
            // the position into account.
            imagebb.position(0);

            bm.copyPixelsFromBuffer(imagebb);

            imageView.setImageBitmap(bm);

            setContentView(imageView);
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
