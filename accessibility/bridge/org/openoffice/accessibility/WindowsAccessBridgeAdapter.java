/*************************************************************************
 *
 *  $RCSfile: WindowsAccessBridgeAdapter.java,v $
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

import org.openoffice.java.accessibility.*;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.AccessibleRole;

import com.sun.star.uno.*;
import com.sun.star.java.XJavaVM;

import javax.accessibility.*;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;


public class WindowsAccessBridgeAdapter {

    private static Method registerVirtualFrame;
    private static Method revokeVirtualFrame;

    private static java.util.Hashtable frameMap;

    protected static class PopupMenuProxy extends AccessibleContext implements Accessible, AccessibleComponent {
        AccessibleContext menu;
        AccessibleComponent menuComponent;

        PopupMenuProxy(AccessibleContext ac) {
            menu = ac;
        }

        /** Returns the AccessibleContext associated with this object */
        public javax.accessibility.AccessibleContext getAccessibleContext() {
            return this;
        }

        /** Returns the AccessibleContext associated with this object */
        public javax.accessibility.AccessibleComponent getAccessibleComponent() {
            return this;
        }

        /** Returns the AccessibleText associated with this object */
        public javax.accessibility.AccessibleText getAccessibleText() {
            return menu.getAccessibleText();
        }


        /** Returns the AccessibleContext associated with this object */
        public javax.accessibility.AccessibleStateSet getAccessibleStateSet() {
             return menu.getAccessibleStateSet();
        }

        public java.util.Locale getLocale() {
            return menu.getLocale();
        }

        public int getAccessibleIndexInParent() {
            return -1;
        }

        public int getAccessibleChildrenCount() {
            return menu.getAccessibleChildrenCount();
        }

        public javax.accessibility.Accessible getAccessibleChild(int i) {
            return menu.getAccessibleChild(i);
        }

        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.POPUP_MENU;
        }

        /*
        * AccessibleComponent
        */

        public void addFocusListener(java.awt.event.FocusListener fl) {
            menuComponent.addFocusListener(fl);
        }

        public void removeFocusListener(java.awt.event.FocusListener fl) {
            menuComponent.removeFocusListener(fl);
        }

        /** Returns the background color of the object */
        public java.awt.Color getBackground() {
            return menuComponent.getBackground();
        }

        public void setBackground(java.awt.Color c) {
            // Not supported by UNO accessibility API
        }

        /** Returns the foreground color of the object */
        public java.awt.Color getForeground() {
            return menuComponent.getForeground();
        }

        public void setForeground(java.awt.Color c) {
            menuComponent.setForeground(c);
        }

        public java.awt.Cursor getCursor() {
            return menuComponent.getCursor();
        }

        public void setCursor(java.awt.Cursor cursor) {
            menuComponent.setCursor(cursor);
        }

        public java.awt.Font getFont() {
            return menuComponent.getFont();
        }

        public void setFont(java.awt.Font f) {
            menuComponent.setFont(f);
        }

        public java.awt.FontMetrics getFontMetrics(java.awt.Font f) {
            return menuComponent.getFontMetrics(f);
        }

        public boolean isEnabled() {
            return menuComponent.isEnabled();
        }

        public void setEnabled(boolean b) {
            menuComponent.setEnabled(b);
        }

        public boolean isVisible() {
            return menuComponent.isVisible();
        }

        public void setVisible(boolean b) {
            menuComponent.setVisible(b);
        }

        public boolean isShowing() {
            return menuComponent.isShowing();
        }

        public boolean contains(java.awt.Point p) {
            java.awt.Dimension d = getSize();

            if (Build.DEBUG) {
                System.err.println("PopupMenuProxy.containsPoint(" + p.x + "," + p.y + ") returns " +
                (((d.width >= 0) && (p.x < d.width) && (d.height >= 0) && (p.y < d.height)) ? "true" : "false"));
            }

            if ((d.width >= 0) && (p.x < d.width) && (d.height >= 0) && (p.y < d.height)) {
                return true;
            }
            return false;
        }

        /** Returns the location of the object on the screen. */
        public java.awt.Point getLocationOnScreen() {
            Accessible a = menu.getAccessibleChild(0);
            if (a != null) {
                AccessibleContext ac = a.getAccessibleContext();
                if (ac != null) {
                    return ac.getAccessibleComponent().getLocationOnScreen();
                }
            }
            return null;
        }

        /** Gets the location of this component in the form of a point specifying the component's top-left corner */
        public java.awt.Point getLocation() {
            // This object represents a toplevel, so this is the same as getLocationOnScreen()
            return getLocationOnScreen();
        }

        /** Moves this component to a new location */
        public void setLocation(java.awt.Point p) {
            // Not supported by UNO accessibility API
        }

        /** Gets the bounds of this component in the form of a Rectangle object */
        public java.awt.Rectangle getBounds() {
            java.awt.Dimension d = getSize();
            java.awt.Point p = getLocationOnScreen();
            return new java.awt.Rectangle(p.x, p.y, d.width, d.height);
        }

        /** Moves and resizes this component to conform to the new bounding rectangle r */
        public void setBounds(java.awt.Rectangle r) {
            // Not supported by UNO accessibility API
        }

        /** Returns the size of this component in the form of a Dimension object */
        public java.awt.Dimension getSize() {
            int count = menu.getAccessibleChildrenCount();
            int width = 0; int height = 0;
            for (int i = 0; i < count; i++) {
                Accessible a = menu.getAccessibleChild(i);
                if (a != null) {
                    AccessibleContext ac = a.getAccessibleContext();
                    if (ac != null) {
                        java.awt.Dimension d = ac.getAccessibleComponent().getSize();
                        width += d.width;
                        height += d.height;
                    }
                }
            }
            return new java.awt.Dimension(width, height);
        }

        /** Resizes this component so that it has width d.width and height d.height */
        public void setSize(java.awt.Dimension d) {
            // Not supported by UNO accessibility API
        }

        /** Returns the Accessible child, if one exists, contained at the local coordinate Point */
        public javax.accessibility.Accessible getAccessibleAt(java.awt.Point p) {
            int count = menu.getAccessibleChildrenCount();
            for (int i = 0; i < count; i++) {
                Accessible a = menu.getAccessibleChild(i);
                if (a != null) {
                    AccessibleContext ac = a.getAccessibleContext();
                    AccessibleComponent comp = ac.getAccessibleComponent();
                    java.awt.Point loc = comp.getLocationOnScreen();
                    if ((p.x >= loc.x) && (p.y >= loc.y)) {
                        java.awt.Dimension d = comp.getSize();
                        if ((p.x < loc.x + d.width) && (p.y < loc.y + d.height)) {
                            return a;
                        }
                    }
                }
            }
            return null;
        }

        public boolean isFocusTraversable() {
            return menuComponent.isFocusTraversable();
        }

        public void requestFocus() {
            menuComponent.requestFocus();
        }
    }

    protected static native byte[] getProcessID();
    protected static native boolean createMapping(long jvmaccess);

    // On Windows all native frames must be registered to the access bridge. Therefor
    // the bridge exports two methods that we try to find here.
    protected static void attach(XComponentContext xComponentContext) {
        try {
            Class bridge = Class.forName("com.sun.java.accessibility.AccessBridge");
            Class[] parameterTypes = { javax.accessibility.Accessible.class, Integer.class };

            if (bridge != null) {
                registerVirtualFrame = bridge.getMethod("registerVirtualFrame", parameterTypes);
                revokeVirtualFrame = bridge.getMethod("revokeVirtualFrame", parameterTypes);

                // load the native dll
                System.loadLibrary("java_uno_accessbridge");


                Object any = xComponentContext.getValueByName("/singletons/com.sun.star.java.theJavaVirtualMachine");
                if (AnyConverter.isObject(any)) {
                    XJavaVM xJavaVM = (XJavaVM) UnoRuntime.queryInterface(XJavaVM.class,
                        AnyConverter.toObject(new Type(XJavaVM.class), any));

                    if (xJavaVM != null) {
                        any = xJavaVM.getJavaVM(getProcessID());
                        if (AnyConverter.isLong(any)) {
                            createMapping(AnyConverter.toLong(any));
                            frameMap = new java.util.Hashtable();
                        }
                    }
                }
            }
        } catch (NoSuchMethodException e) {
            System.err.println("ERROR: incompatible AccessBridge found: " + e.getMessage());

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException("incompatible AccessBridge class: " + e.getMessage());
        } catch (java.lang.SecurityException e) {
            System.err.println("ERROR: no access to AccessBridge: " + e.getMessage());

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException("Security exception caught: " + e.getMessage());
        } catch(ClassNotFoundException e) {

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException("ClassNotFound exception caught: " + e.getMessage());
        } catch(IllegalArgumentException e) {
            System.err.println("IllegalArgumentException caught: " + e.getMessage());

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException("IllegalArgumentException caught: " + e.getMessage());
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            System.err.println("UNO IllegalArgumentException caught: " + e.getMessage());

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException("UNO IllegalArgumentException caught: " + e.getMessage());
        }
    }

    protected static boolean isAttached() {
        return frameMap != null;
    }

    protected static Accessible getAccessibleWrapper(XAccessible xAccessible) {
        Accessible a = null;

        try {
            XAccessibleContext xAccessibleContext = xAccessible.getAccessibleContext();
            if (xAccessibleContext != null) {
                switch (xAccessibleContext.getAccessibleRole()) {
                    case AccessibleRole.POPUP_MENU:
                    case AccessibleRole.LIST:
                        a = (Accessible) AccessibleObjectFactory.getAccessibleComponent(xAccessible);
                        break;
                    case AccessibleRole.MENU:
                        Accessible tmp = (Accessible) AccessibleObjectFactory.getAccessibleComponent(xAccessible);
                        if (tmp != null) {
                            AccessibleContext ac = tmp.getAccessibleContext();
                            if (ac != null) {
                                a = new PopupMenuProxy(ac);
                            }
                        }
                        break;
                    default:
                        a = (Accessible) AccessBridge.getTopWindow(xAccessible);
                        break;
                }
            }
        } catch (com.sun.star.uno.RuntimeException e) {
        }

        return a;
    }

    /** Registers a native frame at the Java AccessBridge for Windows */
    public static void registerTopWindow(int handle, XAccessible xAccessible) {
        Integer hwnd = new Integer(handle);

        if (! frameMap.contains(hwnd) ) {
            if (Build.DEBUG) {
                System.err.println("Native frame " + hwnd + " has been opened");
            }

            Accessible a = getAccessibleWrapper(xAccessible);
            if (a != null) {
                Object[] args = { a, hwnd };

                frameMap.put(hwnd, a);

                if (Build.DEBUG) {
                    System.err.println("registering native frame " + hwnd);
                }

                try {
                    registerVirtualFrame.invoke(null, args);
                } catch(IllegalAccessException e) {
                    System.err.println("IllegalAccessException caught: " + e.getMessage());
                } catch(IllegalArgumentException e) {
                    System.err.println("IllegalArgumentException caught: " + e.getMessage());
                } catch(InvocationTargetException e) {
                    System.err.println("InvokationTargetException caught: " + e.getMessage());
                }
            }
        }
    }

    /** Revokes a native frame at the Java AccessBridge for Windows */
    public static void revokeTopWindow(int handle, XAccessible xAccessible) {
        Integer hwnd = new Integer(handle);

        Accessible a = (Accessible) frameMap.remove(hwnd);
        if (a != null) {
            Object[] args = { a, hwnd };

            if (Build.DEBUG) {
                System.err.println("revoking native frame " + hwnd);
            }

            try {
                revokeVirtualFrame.invoke(null, args);
            } catch(IllegalAccessException e) {
                System.err.println("IllegalAccessException caught: " + e.getMessage());
            } catch(IllegalArgumentException e) {
                System.err.println("IllegalArgumentException caught: " + e.getMessage());
            } catch(InvocationTargetException e) {
                System.err.println("InvokationTargetException caught: " + e.getMessage());
            }
        }
    }
}
