/*************************************************************************
 *
 *  $RCSfile: AccessBridge.java,v $
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
 *
 ************************************************************************/

package org.openoffice.accessibility;

import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XTopWindowListener;
import com.sun.star.awt.XWindow;

import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.*;
import com.sun.star.uno.*;
import com.sun.star.comp.loader.FactoryHelper;

import org.openoffice.java.accessibility.*;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XExtendedToolkit;

import javax.accessibility.Accessible;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;


public class AccessBridge {

    //
    protected static java.util.Hashtable topWindowMap = new java.util.Hashtable();

    protected static java.awt.Window getTopWindow(XAccessible xAccessible) {
        if (xAccessible != null) {
            // Toolkit reports the VCL peer windows as toplevels. These have an accessible parent
            // which represents the native frame window
            XAccessibleContext xAccessibleContext = xAccessible.getAccessibleContext();
            if ((xAccessibleContext != null) && (xAccessibleContext.getAccessibleIndexInParent() != -1)) {
                return getTopWindow(xAccessibleContext.getAccessibleParent());
            }

            // Because it can not garantied that WindowsAccessBridgeAdapter.registerTopWindow()
            // is called before windowOpened(), we have to make this operation atomic.
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
        return null;
    }

    static public class _AccessBridge implements XTopWindowListener, XInitialization {
        static final String _serviceName = "com.sun.star.accessibility.AccessBridge";

        public _AccessBridge(XComponentContext xComponentContext) {

            // Try to initialize the WindowsAccessBridgeAdapter
            String os = (String) System.getProperty("os.name");
            if(os.startsWith("Windows")) {
                WindowsAccessBridgeAdapter.attach(xComponentContext);
            }
        }

        /*
        * XInitialization
        */

        public void initialize(java.lang.Object[] arguments) {
            try {
                // FIXME: Currently there is no way to determine if key event forwarding is needed or not,
                // so we have to do it always ..
                XExtendedToolkit unoToolkit = (XExtendedToolkit)
                    AnyConverter.toObject(new Type(XExtendedToolkit.class), arguments[0]);

                if(unoToolkit != null) {
                    // FIXME this should be done in VCL
                    unoToolkit.addTopWindowListener(this);
                    unoToolkit.addKeyHandler(new KeyHandler());
                } else if( Build.DEBUG) {
                    System.err.println("argument 0 is not of type XExtendedToolkit.");
                }
            } catch(com.sun.star.lang.IllegalArgumentException e) {
                        // FIXME: output
            }
        }

        /*
        * XTopWindowListener
        */

        public void windowOpened(com.sun.star.lang.EventObject event){
            XAccessible xAccessible = (XAccessible) UnoRuntime.queryInterface(XAccessible.class, event.Source);
            java.awt.Window w = getTopWindow(xAccessible);
        }

        public void windowActivated(com.sun.star.lang.EventObject event){
        }

        public void windowDeactivated(com.sun.star.lang.EventObject event){
        }

        public void windowMinimized(com.sun.star.lang.EventObject event){
        }

        public void windowNormalized(com.sun.star.lang.EventObject event){
        }

        public void windowClosing(com.sun.star.lang.EventObject event){
        }

        public void windowClosed(com.sun.star.lang.EventObject event){
            java.awt.Window w = (java.awt.Window)
                topWindowMap.remove(UnoRuntime.generateOid(event.Source));

            if (w != null) {
                w.dispose();
                if (Build.DEBUG) {
                    System.err.println("TopWindow closed");
                }
            }
        }

        public void disposing(com.sun.star.lang.EventObject event) {
        }
    }

    public static XSingleServiceFactory __getServiceFactory(String implName, XMultiServiceFactory multiFactory, XRegistryKey regKey) {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(AccessBridge.class.getName()) ) {
            // Initialize toolkit to register at Java <-> Windows access bridge
            java.awt.Toolkit tk = java.awt.Toolkit.getDefaultToolkit();

            xSingleServiceFactory = FactoryHelper.getServiceFactory(
                _AccessBridge.class,
                _AccessBridge._serviceName,
                multiFactory,
                regKey
            );
        }

        return xSingleServiceFactory;
    }

    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(AccessBridge.class.getName(), _AccessBridge._serviceName, regKey);
    }
}
