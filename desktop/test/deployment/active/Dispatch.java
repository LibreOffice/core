/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

package com.sun.star.comp.test.deployment.active_java;

import com.sun.star.awt.MessageBoxButtons;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XMessageBox;
import com.sun.star.awt.XMessageBoxFactory;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.DispatchDescriptor;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XStatusListener;
import com.sun.star.lang.WrappedTargetRuntimeException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.URL;

public final class Dispatch extends WeakBase implements XServiceInfo, XDispatch
{
    public Dispatch(XComponentContext context) {
        this.context = context;
    }

    public String getImplementationName() { return implementationName; }

    public boolean supportsService(String ServiceName) {
        return false; //TODO
    }

    public String[] getSupportedServiceNames() {
        return serviceNames;
    }

    public void dispatch(URL URL, PropertyValue[] Arguments) {
        try {
            XMultiComponentFactory smgr = UnoRuntime.queryInterface(
                XMultiComponentFactory.class, context.getServiceManager());
            XMessageBox box = UnoRuntime.queryInterface(
                XMessageBoxFactory.class,
                smgr.createInstanceWithContext(
                    "com.sun.star.awt.Toolkit", context)).
                createMessageBox(
                    UnoRuntime.queryInterface(
                        XWindowPeer.class,
                        (UnoRuntime.queryInterface(
                            XDesktop.class,
                            smgr.createInstanceWithContext(
                                "com.sun.star.frame.Desktop", context)).
                         getCurrentFrame().getComponentWindow())),
                    new Rectangle(), "infobox", MessageBoxButtons.BUTTONS_OK,
                    "active", "java");
            box.execute();
            UnoRuntime.queryInterface(XComponent.class, box).dispose();
        } catch (com.sun.star.uno.RuntimeException e) {
            throw e;
        } catch (com.sun.star.uno.Exception e) {
            throw new WrappedTargetRuntimeException(
                "wrapped: " + e.getMessage(), this, e);
        }
    }

    public void addStatusListener(XStatusListener Control, URL URL) {}

    public void removeStatusListener(XStatusListener Control, URL URL) {}

    private final XComponentContext context;

    static final String implementationName =
        "com.sun.star.comp.test.deployment.active_java_singleton";

    static final String[] serviceNames = new String[0];
}
