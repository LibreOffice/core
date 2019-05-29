/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.wizards.ui;

import com.sun.star.awt.*;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFrame;
import com.sun.star.io.IOException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.PropertyNames;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class DocumentPreview
{

    /**
     * The window in which the preview is shown.
     */
    private XWindow xWindow;
    /**
     * The frame service which is used to show the preview
     */
    private XFrame xFrame;
    /**
     * The content component of the frame.
     */
    private XComponent xComponent;
    private XControl xControl;
    private int step;
    private PropertyValue[] loadArgs;
    private String url;
    public static final int PREVIEW_MODE = 1;

    /*********************************************************************
    main

    - create new frame with window inside
    - load a component as preview into this frame
    */
    public DocumentPreview(XMultiServiceFactory xmsf, Object control) throws Exception
    {

        //((XWindow)UnoRuntime.queryInterface(XWindow.class,control)).addPaintListener(this);
        xControl = UnoRuntime.queryInterface(XControl.class, control);
        //register this object as a listener, to close the frame when disposing.
        //((XComponent) UnoRuntime.queryInterface(XComponent.class, control)).addEventListener(this);

        createPreviewFrame(xmsf, xControl);
    }

    protected XComponent setDocument(String url_, String[] propNames, Object[] propValues) throws com.sun.star.lang.IllegalArgumentException, IOException, CloseVetoException
    {
        url = url_;

        Properties ps = new Properties();

        for (int i = 0; i < propNames.length; i++)
        {
            ps.put(propNames[i], propValues[i]);
        }
        return setDocument(url, ps.getProperties());
    }

    protected XComponent setDocument(String url, PropertyValue[] lArgs) throws com.sun.star.lang.IllegalArgumentException, IOException, CloseVetoException
    {
        loadArgs = lArgs;
        XComponentLoader xCompLoader = UnoRuntime.queryInterface(XComponentLoader.class, xFrame);
        xFrame.activate();
        return xComponent = xCompLoader.loadComponentFromURL(url, "_self", 0, loadArgs);
    }

    public void reload(XMultiServiceFactory xmsf) throws com.sun.star.lang.IllegalArgumentException, IOException, CloseVetoException, com.sun.star.uno.Exception
    {
        closeFrame();
        createPreviewFrame(xmsf, xControl);
        //System.out.println(xControl);
        setDocument(url, loadArgs);
    }

    private void closeFrame() throws CloseVetoException
    {
        if (xFrame != null)
        {
            UnoRuntime.queryInterface(XCloseable.class, xFrame).close(false);
        }
    }

    public XComponent setDocument(String url, int mode) throws com.sun.star.lang.IllegalArgumentException, IOException, CloseVetoException
    {
        switch (mode)
        {
            case PREVIEW_MODE:
                return setDocument(url, new String[]
                        {
                            "Preview", PropertyNames.READ_ONLY
                        }, new Object[]
                        {
                            Boolean.TRUE, Boolean.TRUE
                        });
        }
        return null;
    }

    /*********************************************************************
    create a new frame with a new container window inside,
    which isn't part of the global frame tree.

    Attention:
    a) This frame won't be destroyed by the office. It must be closed by you!
    Do so - please call XCloseable::close().
    b) The container window is part of the frame. Don't hold it alive - nor try to kill it.
    It will be destroyed inside close().
     */
    public void createPreviewFrame(XMultiServiceFactory xmsf, XControl xControl) throws com.sun.star.uno.Exception, com.sun.star.lang.IllegalArgumentException
    {
        XWindowPeer controlPeer = xControl.getPeer();
        XWindow controlWindow = UnoRuntime.queryInterface(XWindow.class, xControl);
        Rectangle r = controlWindow.getPosSize();

        Object toolkit = xmsf.createInstance("com.sun.star.awt.Toolkit");
        XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class, toolkit);

        WindowDescriptor aDescriptor = new WindowDescriptor();
        aDescriptor.Type = WindowClass.SIMPLE;
        aDescriptor.WindowServiceName = "window";
        aDescriptor.ParentIndex = -1;
        aDescriptor.Parent = controlPeer; //xWindowPeer; //argument !
        aDescriptor.Bounds = new Rectangle(0, 0, r.Width, r.Height);
        aDescriptor.WindowAttributes = VclWindowPeerAttribute.CLIPCHILDREN | WindowAttribute.SHOW;

        XWindowPeer xPeer = xToolkit.createWindow(aDescriptor);
        xWindow = UnoRuntime.queryInterface(XWindow.class, xPeer);
        Object frame = xmsf.createInstance("com.sun.star.frame.Frame");
        xFrame = UnoRuntime.queryInterface(XFrame.class, frame);
//      XFrame xF = (XFrame) UnoRuntime.queryInterface(XFrame.class, Desktop.getDesktop(xmsf));
//      xFrame = xF.findFrame("_blank", 0);
        xFrame.initialize(xWindow);
        xWindow.setVisible(true);
    }

    public void dispose()
    {
        try
        {
            closeFrame();
        }
        catch (CloseVetoException ex)
        {
            ex.printStackTrace();
        }
    }
}
