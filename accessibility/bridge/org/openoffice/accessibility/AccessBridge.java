/*************************************************************************
 *
 *  $RCSfile: AccessBridge.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obr $ $Date: 2002-08-19 11:20:47 $
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
import com.sun.star.awt.XWindow;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.*;
import com.sun.star.uno.*;
import com.sun.star.comp.loader.FactoryHelper;

import org.openoffice.accessibility.internal.*;
import org.openoffice.java.accessibility.*;

import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.bridge.XAccessibleTopWindowMap;

import javax.accessibility.Accessible;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;


public class AccessBridge {

    static public class _AccessBridge implements XAccessibleTopWindowMap {
        static private final String _serviceName = "drafts.com.sun.star.accessibility.bridge.AccessBridge";
        private XMultiServiceFactory _xMultiServiceFactory;

        private java.util.Hashtable frameMap = null;
        private Method registerVirtualFrame = null;
        private Method revokeVirtualFrame = null;

        public _AccessBridge(XMultiServiceFactory xMultiServiceFactory) {
            _xMultiServiceFactory = xMultiServiceFactory;

            frameMap = new java.util.Hashtable();

            // On Windows all native frames must be registered to the access bridge. Therefor
            // the bridge exports two methods that we try to find here.
            String os = (String) System.getProperty("os.name");
            if(os.startsWith("Windows")) {
                try {
                    Class bridge = Class.forName("com.sun.java.accessibility.AccessBridge");
                    Class[] parameterTypes = { javax.accessibility.Accessible.class, Integer.class };

                    if(bridge != null) {
                        registerVirtualFrame = bridge.getMethod("registerVirtualFrame", parameterTypes);
                        revokeVirtualFrame = bridge.getMethod("revokeVirtualFrame", parameterTypes);
                    }
                }

                catch(NoSuchMethodException e) {
                    System.err.println("ERROR: incompatible AccessBridge found: " + e.getMessage());

                    // Forward this exception to UNO to indicate that the service will not work correctly.
                    throw new com.sun.star.uno.RuntimeException("incompatible AccessBridge class: " + e.getMessage());
                }

                catch(java.lang.SecurityException e) {
                    System.err.println("ERROR: no access to AccessBridge: " + e.getMessage());

                    // Forward this exception to UNO to indicate that the service will not work correctly.
                    throw new com.sun.star.uno.RuntimeException("Security exception caught: " + e.getMessage());
                }

                catch(ClassNotFoundException e) {
                    // This is quite normal on systems that aren't used by disabled persons.
                }

                // Redirect output to log file on Windows for stdout / stderr are not visible
                if( Build.DEBUG ) {
                    try {
                        java.io.PrintStream log = new java.io.PrintStream(
                            new java.io.FileOutputStream("AccessBridge.log")
                        );

                        System.setOut(log);
                        System.setErr(log);
                    }

                    catch(java.io.FileNotFoundException e) {
                    }
                }
            }
        }

        /*
        * XAccessibleNativeFrameMap
        */

        public void registerAccessibleNativeFrame(Object any, XAccessible xAccessible, XTopWindow xTopWindow ){
            try {
                Integer handle = new Integer(AnyConverter.toInt(any));

                // Add the window fake object as top window listener to receive activate/deactivate events
                WindowFake w = new WindowFake(xAccessible, registerVirtualFrame == null);
                xTopWindow.addTopWindowListener(w);

                if(Build.DEBUG) {
                    System.out.println("register native frame: " + handle);
                }

                // Remember the accessible object associated to this frame
                synchronized(frameMap) {
                    frameMap.put(handle, w);
                }

                // If running on Windows, register this frame to the access bridge
                if(registerVirtualFrame != null) {
                    Object[] args = { w, handle};

                    try {
                        registerVirtualFrame.invoke(null, args);
                    }

                    catch(IllegalAccessException e) {
                        System.err.println("IllegalAccessException caught: " + e.getMessage());
                    }

                    catch(IllegalArgumentException e) {
                        System.err.println("IllegalArgumentException caught: " + e.getMessage());
                    }

                    catch(InvocationTargetException e) {
                        System.err.println("InvokationTargetException caught: " + e.getMessage());
                    }
                }
            }

            catch(com.sun.star.lang.IllegalArgumentException exception) {
                System.err.println("IllegalArgumentException caught: " + exception.getMessage());
            }
        }

        public void revokeAccessibleNativeFrame(Object any) {
            try {
                Integer handle = new Integer(AnyConverter.toInt(any));
                WindowFake w;

                // Remember the accessible object associated to this frame
                synchronized(frameMap) {
                    w = (WindowFake) frameMap.remove(handle);
                }

                if(w != null) {
                    if(Build.DEBUG) {
                        System.out.println("revoke native frame: " + handle);
                    }

                    // If running on Windows, register this frame to the access bridge
                    if(revokeVirtualFrame != null) {
                        Object[] args = { w, handle };

                        try {
                            revokeVirtualFrame.invoke(null, args);
                        }

                        catch(IllegalAccessException e) {
                            System.err.println("IllegalAccessException caught: " + e.getMessage());
                        }

                        catch(IllegalArgumentException e) {
                            System.err.println("IllegalArgumentException caught: " + e.getMessage());
                        }

                        catch(InvocationTargetException e) {
                            System.err.println("InvokationTargetException caught: " + e.getMessage());
                        }
                    }
                }
            }

            catch(com.sun.star.lang.IllegalArgumentException exception) {
                System.err.println("IllegalArgumentException caught: " + exception.getMessage());
            }
        }
    }

    public static XSingleServiceFactory __getServiceFactory(String implName, XMultiServiceFactory multiFactory, XRegistryKey regKey) {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(AccessBridge.class.getName()) ) {
            // Initialize toolkit to register at Java <-> Windows access bridge
            java.awt.Toolkit tk = java.awt.Toolkit.getDefaultToolkit();

            try {
                XInterface instance = (XInterface) multiFactory.createInstance(
                    "org.openoffice.accessibility.internal.RemoteAccessBridge"
                );

                if(instance != null) {
                    XAccessibilityInformationProvider infoProvider = (XAccessibilityInformationProvider)
                        UnoRuntime.queryInterface(XAccessibilityInformationProvider.class, instance);

                    if(infoProvider != null) {
                        AccessibleObjectFactory.getDefault().setInformationProvider(infoProvider);
                    } else {
                        System.err.println("InfoProvider does not implement XAccessibleInformationProvider.");
                    }
                } else {
                    System.err.println("InfoProvider service not found.");
                    throw new com.sun.star.uno.RuntimeException("RemoteAccessBridge service not found.\n");
                }
            }

            catch (com.sun.star.uno.Exception e) {
                System.err.println(e.getMessage());
                throw new com.sun.star.uno.RuntimeException(e.getMessage());
            }

            xSingleServiceFactory = FactoryHelper.getServiceFactory(_AccessBridge.class,
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
