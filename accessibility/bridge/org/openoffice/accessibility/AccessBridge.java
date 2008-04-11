/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessBridge.java,v $
 * $Revision: 1.21 $
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
 *
 ************************************************************************/
package org.openoffice.accessibility;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XTopWindowListener;
import com.sun.star.awt.XWindow;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.*;
import com.sun.star.uno.*;

import org.openoffice.java.accessibility.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import javax.accessibility.Accessible;


public class AccessBridge {
    //
    protected static java.util.Hashtable topWindowMap = new java.util.Hashtable();

    private static java.awt.Window getTopWindowImpl(XAccessible xAccessible) {
        // Because it can not be garantied that
        // WindowsAccessBridgeAdapter.registerTopWindow() is called
        // before windowOpened(), we have to make this operation
        // atomic.
        synchronized (topWindowMap) {
            String oid = UnoRuntime.generateOid(xAccessible);
            java.awt.Window w = (java.awt.Window) topWindowMap.get(oid);

            if (w == null) {
                w = AccessibleObjectFactory.getTopWindow(xAccessible);

                if (w != null) {
                    topWindowMap.put(oid, w);
                }
            }

            return w;
        }
    }

    protected static java.awt.Window getTopWindow(XAccessible xAccessible) {
        if (xAccessible != null) {
            XAccessibleContext xAccessibleContext = xAccessible.getAccessibleContext();
            if (xAccessibleContext != null) {

                // Toolkit reports the VCL peer windows as toplevels. These have an
                // accessible parent which represents the native frame window
                switch(xAccessibleContext.getAccessibleRole()) {
                    case AccessibleRole.ROOT_PANE:
                    case AccessibleRole.POPUP_MENU:
                        return getTopWindow(xAccessibleContext.getAccessibleParent());

                    case AccessibleRole.WINDOW:
                    case AccessibleRole.FRAME:
                    case AccessibleRole.DIALOG:
                    case AccessibleRole.ALERT:
                        return getTopWindowImpl(xAccessible);

                    default:
                        break;
                }
            }
        }

        return null;
    }

    protected static java.awt.Window removeTopWindow(XAccessible xAccessible) {
        if (xAccessible != null) {
            XAccessibleContext xAccessibleContext = xAccessible.getAccessibleContext();
            if (xAccessibleContext != null) {

                // Toolkit reports the VCL peer windows as toplevels. These have an
                // accessible parent which represents the native frame window
                switch(xAccessibleContext.getAccessibleRole()) {
                    case AccessibleRole.ROOT_PANE:
                    case AccessibleRole.POPUP_MENU:
                        return removeTopWindow(xAccessibleContext.getAccessibleParent());

                    case AccessibleRole.WINDOW:
                    case AccessibleRole.FRAME:
                    case AccessibleRole.DIALOG:
                        return (java.awt.Window) topWindowMap.remove(UnoRuntime.generateOid(xAccessible));

                    default:
                        break;
                }
            }
        }

        return null;
    }

    public static XSingleServiceFactory __getServiceFactory(String implName,
        XMultiServiceFactory multiFactory, XRegistryKey regKey) {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(AccessBridge.class.getName())) {
            // Initialize toolkit to register at Java <-> Windows access bridge
            java.awt.Toolkit tk = java.awt.Toolkit.getDefaultToolkit();

            xSingleServiceFactory = FactoryHelper.getServiceFactory(_AccessBridge.class,
                    _AccessBridge._serviceName, multiFactory, regKey);
        }

        return xSingleServiceFactory;
    }

    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(AccessBridge.class.getName(),
            _AccessBridge._serviceName, regKey);
    }

    static public class _AccessBridge implements XTopWindowListener,
        XInitialization, XComponent {
        static final String _serviceName = "com.sun.star.accessibility.AccessBridge";
        XComponentContext xComponentContext;

        public _AccessBridge(XComponentContext xComponentContext) {
            this.xComponentContext = xComponentContext;
        }

        /*
        * XInitialization
        */
        public void initialize(java.lang.Object[] arguments) {
            try {
                // FIXME: Currently there is no way to determine if key event forwarding is needed or not,
                // so we have to do it always ..
                XExtendedToolkit unoToolkit = (XExtendedToolkit) AnyConverter.toObject(new Type(
                            XExtendedToolkit.class), arguments[0]);

                if (unoToolkit != null) {
                    // FIXME this should be done in VCL
                    unoToolkit.addTopWindowListener(this);

                    String os = (String) System.getProperty("os.name");

                    // Try to initialize the WindowsAccessBridgeAdapter
                    if (os.startsWith("Windows")) {
                        WindowsAccessBridgeAdapter.attach(xComponentContext);
                    } else {
                        unoToolkit.addKeyHandler(new KeyHandler());
                    }
                } else if (Build.DEBUG) {
                    System.err.println(
                        "argument 0 is not of type XExtendedToolkit.");
                }
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                // FIXME: output
            }
        }

        /*
        * XTopWindowListener
        */
        public void windowOpened(com.sun.star.lang.EventObject event) {
            XAccessible xAccessible = (XAccessible) UnoRuntime.queryInterface(XAccessible.class,
                    event.Source);
            java.awt.Window w = getTopWindow(xAccessible);
        }

        public void windowActivated(com.sun.star.lang.EventObject event) {
        }

        public void windowDeactivated(com.sun.star.lang.EventObject event) {
        }

        public void windowMinimized(com.sun.star.lang.EventObject event) {
        }

        public void windowNormalized(com.sun.star.lang.EventObject event) {
        }

        public void windowClosing(com.sun.star.lang.EventObject event) {
        }

        public void windowClosed(com.sun.star.lang.EventObject event) {
            XAccessible xAccessible = (XAccessible) UnoRuntime.queryInterface(XAccessible.class,
                    event.Source);

            java.awt.Window w = removeTopWindow(xAccessible);

            if (w != null) {
                w.dispose();
            }
        }

        public void disposing(com.sun.star.lang.EventObject event) {
        }

        /*
        * XComponent
        */

        public void addEventListener(com.sun.star.lang.XEventListener listener) {
        }

        public void removeEventListener(com.sun.star.lang.XEventListener listener) {
        }

        public void dispose() {
            try {
                java.awt.Toolkit.getDefaultToolkit().getSystemEventQueue().invokeAndWait(
                    new Runnable() {
                        public void run() {
                        }
                    } );
            } catch (java.lang.InterruptedException e) {
            } catch (java.lang.reflect.InvocationTargetException e) {
            }
        }
    }
}
