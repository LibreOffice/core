/*************************************************************************
 *
 *  $RCSfile: DocumentPreview.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:04:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */

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

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class DocumentPreview  {
    /**
     * The window in which the preview is showed.
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
    public DocumentPreview(XMultiServiceFactory xmsf, Object control) throws Exception {

        //((XWindow)UnoRuntime.queryInterface(XWindow.class,control)).addPaintListener(this);
        xControl = (XControl) UnoRuntime.queryInterface(XControl.class, control);
        //register this object as a listener, to close the frame when disposing.
        //((XComponent) UnoRuntime.queryInterface(XComponent.class, control)).addEventListener(this);

        createPreviewFrame(xmsf, xControl);
    }

    protected XComponent setDocument(String url_, String[] propNames, Object[] propValues) throws com.sun.star.lang.IllegalArgumentException, IOException, CloseVetoException {
        url = url_;

        Properties ps = new Properties();

        for (int i = 0; i < propNames.length; i++)
            ps.put(propNames[i], propValues[i]);

        return setDocument(url, ps.getProperties());
    }

    protected XComponent setDocument(String url, PropertyValue[] lArgs) throws com.sun.star.lang.IllegalArgumentException, IOException, CloseVetoException {

        loadArgs = lArgs;
        XComponentLoader xCompLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, xFrame);
        return xComponent = xCompLoader.loadComponentFromURL(url, "_self", 0, loadArgs);
    }

    public void reload(XMultiServiceFactory xmsf) throws com.sun.star.lang.IllegalArgumentException, IOException, CloseVetoException, com.sun.star.uno.Exception {
        closeFrame();
        createPreviewFrame(xmsf, xControl);
        //System.out.println(xControl);
        setDocument(url, loadArgs);
    }

    private void closeFrame() throws CloseVetoException {
        if (xFrame != null)
             ((XCloseable) UnoRuntime.queryInterface(XCloseable.class, xFrame)).close(false);
    }

    public XComponent setDocument(String url, int mode) throws com.sun.star.lang.IllegalArgumentException, IOException, CloseVetoException {
        switch (mode) {
            case PREVIEW_MODE :
                return setDocument(url, new String[] { "Preview", "ReadOnly" }, new Object[] { Boolean.TRUE, Boolean.TRUE });
        }
        return null;
    }

    /*********************************************************************
        create a new frame with a new container window inside,
        which isnt part of the global frame tree.

        Attention:
            a) This frame wont be destroyed by the office. It must be closed by you!
               Do so - please call XCloseable::close().
            b) The container window is part of the frame. Dont hold it alive - nor try to kill it.
               It will be destroyed inside close().
     */
    public void createPreviewFrame(XMultiServiceFactory xmsf, XControl xControl) throws com.sun.star.uno.Exception, com.sun.star.lang.IllegalArgumentException {
        XWindowPeer controlPeer = xControl.getPeer();
        XWindow controlWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xControl);
        Rectangle r = controlWindow.getPosSize();

        Object toolkit = xmsf.createInstance("com.sun.star.awt.Toolkit");
        XToolkit xToolkit = (XToolkit) UnoRuntime.queryInterface(XToolkit.class, toolkit);

        WindowDescriptor aDescriptor = new WindowDescriptor();
        aDescriptor.Type = WindowClass.SIMPLE;
        aDescriptor.WindowServiceName = "window";
        aDescriptor.ParentIndex = -1;
        aDescriptor.Parent = controlPeer; //xWindowPeer; //argument !
        aDescriptor.Bounds = new Rectangle(0, 0, r.Width, r.Height);
        aDescriptor.WindowAttributes = VclWindowPeerAttribute.CLIPCHILDREN | WindowAttribute.SHOW;

        XWindowPeer xPeer = xToolkit.createWindow(aDescriptor);
        xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xPeer);

        Object frame = xmsf.createInstance("com.sun.star.frame.Frame");
        xFrame = (XFrame) UnoRuntime.queryInterface(XFrame.class, frame);
        xFrame.initialize(xWindow);
        xWindow.setVisible(true);
    }


    public void dispose() {
        try {
            closeFrame();
        } catch (CloseVetoException ex) {
            ex.printStackTrace();
        }
    }


}