/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.comp.test.deployment.passive_java;

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
                    "passive", "java");
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
        "com.sun.star.comp.test.deployment.passive_java_singleton";

    static final String[] serviceNames = new String[0];
}
