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
package org.openoffice.accessibility;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.java.XJavaVM;
import com.sun.star.uno.*;

import org.openoffice.java.accessibility.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import javax.accessibility.*;


public class WindowsAccessBridgeAdapter {
    private static Method registerVirtualFrame;
    private static Method revokeVirtualFrame;
    private static java.util.Hashtable<Integer, Accessible> frameMap;

    protected static native byte[] getProcessID();

    protected static native boolean createMapping(long jvmaccess);

    // On Windows all native frames must be registered to the access bridge.
    // Therefor the bridge exports two methods that we try to find here.
    protected static void attach(XComponentContext xComponentContext) {
        try {
            Class<?> bridge = Class.forName(
                    "com.sun.java.accessibility.AccessBridge");
            Class<?>[] parameterTypes = {
                javax.accessibility.Accessible.class, Integer.class
            };

            if (bridge != null) {
                registerVirtualFrame = bridge.getMethod("registerVirtualFrame",
                        parameterTypes);
                revokeVirtualFrame = bridge.getMethod("revokeVirtualFrame",
                        parameterTypes);

                // load the native dll
                System.loadLibrary("java_uno_accessbridge");

                Object any = xComponentContext.getValueByName(
                        "/singletons/com.sun.star.java.theJavaVirtualMachine");

                if (AnyConverter.isObject(any)) {
                    XJavaVM xJavaVM = UnoRuntime.queryInterface(XJavaVM.class,
                            AnyConverter.toObject(new Type(XJavaVM.class), any));

                    if (xJavaVM != null) {
                        any = xJavaVM.getJavaVM(getProcessID());

                        if (AnyConverter.isLong(any)) {
                            createMapping(AnyConverter.toLong(any));
                            frameMap = new java.util.Hashtable<Integer, Accessible>();
                        }
                    }
                }
            }
        } catch (NoSuchMethodException e) {
            System.err.println("ERROR: incompatible AccessBridge found: " +
                e.getMessage());

            // Forward this exception to UNO to indicate that the service will
            // not work correctly.
            throw new com.sun.star.uno.RuntimeException(
                "incompatible AccessBridge class: " + e.getMessage());
        } catch (java.lang.SecurityException e) {
            System.err.println("ERROR: no access to AccessBridge: " +
                e.getMessage());

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException(
                "Security exception caught: " + e.getMessage());
        } catch (ClassNotFoundException e) {
            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException(
                "ClassNotFound exception caught: " + e.getMessage());
        } catch (IllegalArgumentException e) {
            System.err.println("IllegalArgumentException caught: " +
                e.getMessage());

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException(
                "IllegalArgumentException caught: " + e.getMessage());
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.err.println("UNO IllegalArgumentException caught: " +
                e.getMessage());

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException(
                "UNO IllegalArgumentException caught: " + e.getMessage());
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
                    case AccessibleRole.LIST:
                        a = (Accessible) AccessibleObjectFactory.getAccessibleComponent(xAccessible);
                        if (a != null) {
                            a = new ListProxy(a.getAccessibleContext());
                        }
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

                    case AccessibleRole.TOOL_TIP:
                        a = PopupWindow.create(xAccessible);
                        break;

                    default:
                        a = AccessBridge.getTopWindow(xAccessible);
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

        if (!frameMap.contains(hwnd)) {
            if (Build.DEBUG) {
                System.err.println("Native frame " + hwnd + " of role " +
                    AccessibleRoleAdapter.getAccessibleRole(xAccessible) +
                    " has been opened");
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
                } catch (IllegalAccessException e) {
                    System.err.println("IllegalAccessException caught: " +
                        e.getMessage());
                } catch (IllegalArgumentException e) {
                    System.err.println("IllegalArgumentException caught: " +
                        e.getMessage());
                } catch (InvocationTargetException e) {
                    System.err.println("InvokationTargetException caught: " +
                        e.getMessage());
                }
            }
        }
    }

    /** Revokes a native frame at the Java AccessBridge for Windows */
    public static void revokeTopWindow(int handle, XAccessible xAccessible) {
        Integer hwnd = new Integer(handle);

        Accessible a = frameMap.remove(hwnd);

        if (a != null) {
            Object[] args = { a, hwnd };

            if (Build.DEBUG) {
                System.err.println("revoking native frame " + hwnd);
            }

            try {
                revokeVirtualFrame.invoke(null, args);
            } catch (IllegalAccessException e) {
                System.err.println("IllegalAccessException caught: " +
                    e.getMessage());
            } catch (IllegalArgumentException e) {
                System.err.println("IllegalArgumentException caught: " +
                    e.getMessage());
            } catch (InvocationTargetException e) {
                System.err.println("InvokationTargetException caught: " +
                    e.getMessage());
            }
        }

        if (a instanceof PopupWindow) {
            PopupWindow toolTipWindow = (PopupWindow) a;
            toolTipWindow.removeAll();
            toolTipWindow.dispose();
        }
    }

    protected static class PopupMenuProxy extends AccessibleContext
        implements Accessible, AccessibleComponent {
        AccessibleContext menu;
        AccessibleComponent menuComponent;
        int x = 0; int y = 0; int width = 0; int height = 0;

        PopupMenuProxy(AccessibleContext ac) {
            menu = ac;
            menuComponent = menu.getAccessibleComponent();

            /** calculate the bounding rectangle by iterating over the
             *  the children.
             */
            int x2 = 0; int y2 = 0;
            int count = ac.getAccessibleChildrenCount();
            for (int i = 0; i < count; i++) {
                Accessible a = menu.getAccessibleChild(i);

                if (a != null) {
                    AccessibleContext childAC = a.getAccessibleContext();

                    if (childAC != null) {
                        AccessibleComponent comp = ac.getAccessibleComponent();

                        if (comp != null) {
                            java.awt.Point p = comp.getLocationOnScreen();
                            java.awt.Dimension d = comp.getSize();

                            if (p != null && d != null) {
                                if (p.x < x) {
                                    x = p.x;
                                }
                                if (p.y < y) {
                                    y = p.y;
                                }
                                if (p.x + d.width > x2) {
                                    x2 = p.x + d.width;
                                }
                                if (p.y + d.height > y2) {
                                    y2 = p.y + d.height;
                                }
                            }
                        }
                    }
                }
            }

            width = x2 - x;
            height = y2 - y;
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
                System.err.println("PopupMenuProxy.containsPoint(" + p.x + "," +
                    p.y + ") returns " +
                    (((d.width >= 0) && (p.x < d.width) && (d.height >= 0) &&
                    (p.y < d.height)) ? "true" : "false"));
            }

            if ((d.width >= 0) && (p.x < d.width) && (d.height >= 0) &&
                    (p.y < d.height)) {
                return true;
            }

            return false;
        }

        /** Returns the location of the object on the screen. */
        public java.awt.Point getLocationOnScreen() {
            return new java.awt.Point(x,y);
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
            return new java.awt.Rectangle(x, y, width, height);
        }

        /** Moves and resizes this component to conform to the new bounding rectangle r */
        public void setBounds(java.awt.Rectangle r) {
            // Not supported by UNO accessibility API
        }

        /** Returns the size of this component in the form of a Dimension object */
        public java.awt.Dimension getSize() {
            return new java.awt.Dimension(width, height);
        }

        /** Resizes this component so that it has width d.width and height d.height */
        public void setSize(java.awt.Dimension d) {
            // Not supported by UNO accessibility API
        }

        /** Returns the Accessible child, if one exists, contained at the local
         * coordinate Point
         */
        public javax.accessibility.Accessible getAccessibleAt(java.awt.Point p) {
            java.awt.Point p2 = menuComponent.getLocationOnScreen();
            return menuComponent.getAccessibleAt(
                new java.awt.Point(p.x + x - p2.x, p.y + y - p2.y));
        }

        public boolean isFocusTraversable() {
            return menuComponent.isFocusTraversable();
        }

        public void requestFocus() {
            menuComponent.requestFocus();
        }
    }

    protected static class ListProxy extends AccessibleContext
        implements Accessible, AccessibleComponent {
        AccessibleContext list;
        AccessibleComponent listComponent;

        ListProxy(AccessibleContext ac) {
            list = ac;
            listComponent = list.getAccessibleComponent();
        }

        /** Returns the AccessibleContext associated with this object */
        public javax.accessibility.AccessibleContext getAccessibleContext() {
            return this;
        }

        /** Returns the AccessibleContext associated with this object */
        public javax.accessibility.AccessibleComponent getAccessibleComponent() {
            return this;
        }

        /** Returns the AccessibleSelection associated with this object */
        public javax.accessibility.AccessibleSelection getAccessibleSelection() {
            return list.getAccessibleSelection();
        }

        /** Returns the AccessibleContext associated with this object */
        public javax.accessibility.AccessibleStateSet getAccessibleStateSet() {
            return list.getAccessibleStateSet();
        }

        public java.util.Locale getLocale() {
            return list.getLocale();
        }

        public int getAccessibleIndexInParent() {
            return -1;
        }

        public int getAccessibleChildrenCount() {
            return list.getAccessibleChildrenCount();
        }

        public javax.accessibility.Accessible getAccessibleChild(int i) {
            return list.getAccessibleChild(i);
        }

        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.LIST;
        }

        /*
        * AccessibleComponent
        */
        public void addFocusListener(java.awt.event.FocusListener fl) {
            listComponent.addFocusListener(fl);
        }

        public void removeFocusListener(java.awt.event.FocusListener fl) {
            listComponent.removeFocusListener(fl);
        }

        /** Returns the background color of the object */
        public java.awt.Color getBackground() {
            return listComponent.getBackground();
        }

        public void setBackground(java.awt.Color c) {
            // Not supported by UNO accessibility API
        }

        /** Returns the foreground color of the object */
        public java.awt.Color getForeground() {
            return listComponent.getForeground();
        }

        public void setForeground(java.awt.Color c) {
            listComponent.setForeground(c);
        }

        public java.awt.Cursor getCursor() {
            return listComponent.getCursor();
        }

        public void setCursor(java.awt.Cursor cursor) {
            listComponent.setCursor(cursor);
        }

        public java.awt.Font getFont() {
            return listComponent.getFont();
        }

        public void setFont(java.awt.Font f) {
            listComponent.setFont(f);
        }

        public java.awt.FontMetrics getFontMetrics(java.awt.Font f) {
            return listComponent.getFontMetrics(f);
        }

        public boolean isEnabled() {
            return listComponent.isEnabled();
        }

        public void setEnabled(boolean b) {
            listComponent.setEnabled(b);
        }

        public boolean isVisible() {
            return listComponent.isVisible();
        }

        public void setVisible(boolean b) {
           listComponent.setVisible(b);
        }

        public boolean isShowing() {
            return listComponent.isShowing();
        }

        public boolean contains(java.awt.Point p) {
            return listComponent.contains(p);
        }

        /** Returns the location of the object on the screen. */
        public java.awt.Point getLocationOnScreen() {
            return listComponent.getLocationOnScreen();
        }

        /** Gets the location of this component in the form of a point specifying
         * the component's top-left corner
         */
        public java.awt.Point getLocation() {
            // This object represents a toplevel object, so getLocation() should
            // return the same as getLocationOnScreen().
            return getLocationOnScreen();
        }

        /** Moves this component to a new location */
        public void setLocation(java.awt.Point p) {
            // Not supported by UNO accessibility API
        }

        /** Gets the bounds of this component in the form of a Rectangle object */
        public java.awt.Rectangle getBounds() {
            java.awt.Point p = getLocationOnScreen();
            java.awt.Dimension d = getSize();
            return new java.awt.Rectangle(p.x, p.y, d.width, d.height);
        }

        /** Moves and resizes this component to conform to the new bounding rectangle r */
        public void setBounds(java.awt.Rectangle r) {
            // Not supported by UNO accessibility API
        }

        /** Returns the size of this component in the form of a Dimension object */
        public java.awt.Dimension getSize() {
            return listComponent.getSize();
        }

        /** Resizes this component so that it has width d.width and height d.height */
        public void setSize(java.awt.Dimension d) {
            // Not supported by UNO accessibility API
        }

        /** Returns the Accessible child, if one exists, contained at the local
         * coordinate Point
         */
        public javax.accessibility.Accessible getAccessibleAt(java.awt.Point p) {
            return listComponent.getAccessibleAt(p);
        }

        public boolean isFocusTraversable() {
            return listComponent.isFocusTraversable();
        }

        public void requestFocus() {
            listComponent.requestFocus();
        }
    }
}
