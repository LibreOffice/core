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

import org.openoffice.accessibility.internal.*;
import org.openoffice.java.accessibility.*;

import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.bridge.XAccessibleTopWindowMap;
import drafts.com.sun.star.awt.XExtendedToolkit;

import javax.accessibility.Accessible;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;


public class AccessBridge {

    //
    static public class _AccessBridge implements XAccessibleTopWindowMap, XInitialization {
        static final String _serviceName = "drafts.com.sun.star.accessibility.bridge.AccessBridge";

        XMultiServiceFactory serviceManager;
        java.util.Hashtable frameMap;


        public _AccessBridge(XMultiServiceFactory xMultiServiceFactory) {
            serviceManager = xMultiServiceFactory;
            frameMap = new java.util.Hashtable();
        }

        /*
        * XInitialization
        */

        public void initialize(java.lang.Object[] arguments) {
            try {
                // Currently there is no way to determine if key event forwarding is needed or not,
                // so we have to do it always ..
                XExtendedToolkit unoToolkit = (XExtendedToolkit)
                    AnyConverter.toObject(new Type(XExtendedToolkit.class), arguments[0]);

                if(unoToolkit != null) {
                    unoToolkit.addKeyHandler(new KeyHandler());
                } else if( Build.DEBUG) {
                    System.err.println("argument 0 is not of type XExtendedToolkit.");
                }
            } catch(com.sun.star.lang.IllegalArgumentException e) {
                        // FIXME: output
            }
        }

        /*
        * XAccessibleNativeFrameMap
        */

        public void registerAccessibleNativeFrame(Object any, XAccessible xAccessible, XTopWindow xTopWindow ){
            try {
                // The office sometimes registers frames more than once, so check here if already done
                Integer handle = new Integer(AnyConverter.toInt(any));
                if (! frameMap.containsKey(handle)) {
                    if( Build.DEBUG ) {
                        System.out.println("register native frame: " + handle);
                    }

                    java.awt.Window w = AccessibleObjectFactory.getTopWindow(xAccessible);
                    if (w != null) {
                        frameMap.put(handle, w);
                    }
                }
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                System.err.println("IllegalArgumentException caught: " + e.getMessage());
            } catch (java.lang.Exception e) {
                System.err.println("Exception caught: " + e.getMessage());
            }
        }

        public void revokeAccessibleNativeFrame(Object any) {
            try {
                Integer handle = new Integer(AnyConverter.toInt(any));

                // Remember the accessible object associated to this frame
                java.awt.Window w = (java.awt.Window) frameMap.remove(handle);

                if (w != null) {
                    if (Build.DEBUG) {
                        System.out.println("revoke native frame: " + handle);
                    }

                    w.dispose();
                }
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                System.err.println("IllegalArgumentException caught: " + e.getMessage());
            }
        }
    }

    static public class _WinAccessBridge extends _AccessBridge {
        Method registerVirtualFrame;
        Method revokeVirtualFrame;

        public _WinAccessBridge(XMultiServiceFactory xMultiServiceFactory) {
            super(xMultiServiceFactory);

//          java.awt.Toolkit tk = java.awt.Toolkit.getDefaultToolkit();
//          tk.addAWTEventListener(new WindowsAccessBridgeAdapter(), java.awt.AWTEvent.WINDOW_EVENT_MASK);

            // On Windows all native frames must be registered to the access bridge. Therefor
            // the bridge exports two methods that we try to find here.
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
                // Forward this exception to UNO to indicate that the service will not work correctly.
                throw new com.sun.star.uno.RuntimeException("ClassNotFound exception caught: " + e.getMessage());
            }
        }

        // Registers the native frame at the Windows access bridge
        protected void registerAccessibleNativeFrameImpl(Integer handle, Accessible a) {
            // register this frame to the access bridge
            Object[] args = { a, handle };

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

        // Revokes the native frame from the Windows access bridge
        protected void revokeAccessibleNativeFrameImpl(Integer handle, Accessible a) {
            Object[] args = { a, handle };

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

        /*
        * XAccessibleNativeFrameMap
        */

        public void registerAccessibleNativeFrame(Object any, XAccessible xAccessible, XTopWindow xTopWindow ){
            try {
                // The office sometimes registers frames more than once, so check here if already done
                Integer handle = new Integer(AnyConverter.toInt(any));
                if (! frameMap.containsKey(handle)) {
                    java.awt.Window w = AccessibleObjectFactory.getTopWindow(xAccessible);

                    if (Build.DEBUG) {
                        System.out.println("register native frame: " + handle);
                    }

                    if (w != null) {
                        // Add the window proxy object to the frame list
                        frameMap.put(handle, w);

                        // Also register the frame with the access bridge object
                        registerAccessibleNativeFrameImpl(handle,w);
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

                // Remember the accessible object associated to this frame
                java.awt.Window w = (java.awt.Window) frameMap.remove(handle);

                if (w != null)  {
                    // Revoke the frame with the access bridge object
                    revokeAccessibleNativeFrameImpl(handle, w);

                    if (Build.DEBUG) {
                        System.out.println("revoke native frame: " + handle);
                    }

                    // It seems that we have to do this synchronously, because otherwise on exit
                    // the main thread of the office terminates before AWT has shut down and the
                    // process will hang ..
                    w.dispose();

/*
                    class DisposeAction implements Runnable {
                        java.awt.Window window;
                        DisposeAction(java.awt.Window w) {
                            window = w;
                        }
                        public void run() {
                            window.dispose();
                        }
                    }

                    java.awt.Toolkit.getDefaultToolkit().getSystemEventQueue().invokeLater(new DisposeAction(w));
*/
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

            Class serviceClass;
            String os = (String) System.getProperty("os.name");
            if(os.startsWith("Windows")) {
                serviceClass = _WinAccessBridge.class;
            } else {
                serviceClass = _AccessBridge.class;
            }

            xSingleServiceFactory = FactoryHelper.getServiceFactory(
                serviceClass,
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
