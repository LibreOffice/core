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

import javax.accessibility.*;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;


public class WindowsAccessBridgeAdapter implements java.awt.event.AWTEventListener, java.beans.PropertyChangeListener {

    Method registerVirtualFrame;
    Method revokeVirtualFrame;
    Method removePropertyChangeListener;

    protected class PopupMenuProxy extends AccessibleContext implements Accessible, AccessibleComponent {
        AccessibleContext menu;

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

        /** Returns the AccessibleContext associated with this object */
        public javax.accessibility.AccessibleStateSet getAccessibleStateSet() {
             return null;
        }

        public java.util.Locale getLocale() {
            return java.util.Locale.getDefault();
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
        }

        public void removeFocusListener(java.awt.event.FocusListener fl) {
        }

        /** Returns the background color of the object */
        public java.awt.Color getBackground() {
            return null;
        }

        public void setBackground(java.awt.Color c) {
            // Not supported by UNO accessibility API
        }

        /** Returns the foreground color of the object */
        public java.awt.Color getForeground() {
            return null;
        }

        public void setForeground(java.awt.Color c) {
            // Not supported by UNO accessibility API
        }

        public java.awt.Cursor getCursor() {
            // Not supported by UNO accessibility API
            return null;
        }

        public void setCursor(java.awt.Cursor cursor) {
            // Not supported by UNO accessibility API
        }

        public java.awt.Font getFont() {
            // FIXME
            return null;
        }

        public void setFont(java.awt.Font f) {
            // Not supported by UNO accessibility API
        }

        public java.awt.FontMetrics getFontMetrics(java.awt.Font f) {
            // FIXME
            return null;
        }

        public boolean isEnabled() {
            return true;
        }

        public void setEnabled(boolean b) {
            // Not supported by UNO accessibility API
        }

        public boolean isVisible() {
            return true;
        }

        public void setVisible(boolean b) {
            // Not supported by UNO accessibility API
        }

        public boolean isShowing() {
            return true;
        }

        public boolean contains(java.awt.Point p) {
            java.awt.Dimension d = getSize();

            if (Build.DEBUG) {
                System.err.println("PopupMenuProxy.contains(" + p.x + "," + p.y + ") returns " +
                (((d.width >= 0) && (p.x < d.width) && (d.height >= 0) && (p.y < d.height)) ? "true" : "false"));
            }

            if ((d.width >= 0) && (p.x < d.width) && (d.height >= 0) && (p.y < d.height)) {
                return true;
            }
            return false;
        }

        /** Returns the location of the object on the screen. */
        public java.awt.Point getLocationOnScreen() {
            if (menu != null) {
                Accessible a = menu.getAccessibleChild(0);
                if (a != null) {
                    AccessibleContext ac = a.getAccessibleContext();
                    if (ac != null) {
                        return ac.getAccessibleComponent().getLocationOnScreen();
                    }
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
            if (menu != null) {
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
            return null;
        }

        /** Resizes this component so that it has width d.width and height d.height */
        public void setSize(java.awt.Dimension d) {
            // Not supported by UNO accessibility API
        }

        /** Returns the Accessible child, if one exists, contained at the local coordinate Point */
        public javax.accessibility.Accessible getAccessibleAt(java.awt.Point p) {
            if (menu != null) {
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
            }
            return null;
        }

        public boolean isFocusTraversable() {
            return true;
        }

        public void requestFocus() {
        }
    }

    // On Windows all native frames must be registered to the access bridge. Therefor
    // the bridge exports two methods that we try to find here.
    public WindowsAccessBridgeAdapter() {
        try {
            Class bridge = Class.forName("com.sun.java.accessibility.AccessBridge");
            Class[] parameterTypes = { javax.accessibility.Accessible.class, Integer.class };

            if (bridge != null) {
                registerVirtualFrame = bridge.getMethod("registerVirtualFrame", parameterTypes);
                revokeVirtualFrame = bridge.getMethod("revokeVirtualFrame", parameterTypes);
            }

            // Register PropertyChangeListener to track popup windows
            Class monitor = Class.forName("com.sun.java.accessibility.util.AccessibilityEventMonitor");
             Class[] monitorParams = { java.beans.PropertyChangeListener.class };

            Method addPropertyChangeListener = monitor.getMethod("addPropertyChangeListener", monitorParams);
            removePropertyChangeListener = monitor.getMethod("removePropertyChangeListener", monitorParams);

            Object[] params = { this };
            addPropertyChangeListener.invoke(null, params);

            System.loadLibrary("java_uno_accessbridge");

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
        } catch(IllegalAccessException e) {
            System.err.println("IllegalAccessException caught: " + e.getMessage());

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException("IllegalAccessException caught: " + e.getMessage());
        } catch(IllegalArgumentException e) {
            System.err.println("IllegalArgumentException caught: " + e.getMessage());

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException("IllegalArgumentException caught: " + e.getMessage());
        } catch(InvocationTargetException e) {
            System.err.println("InvokationTargetException caught: " + e.getMessage());

            // Forward this exception to UNO to indicate that the service will not work correctly.
            throw new com.sun.star.uno.RuntimeException("InvokationTargetException caught: " + e.getMessage());
        }
    }

    /** Registers a native frame at the Java AccessBridge for Windows */
    protected void registerFrame(Integer handle, Accessible a) {
        Object[] args = { a, handle };

        if (Build.DEBUG) {
            System.err.println("Native frame " + handle + " has been opened");
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

    /** Registers a popup window at the Java AccessBridge for Windows */
    protected void registerPopup(Integer handle, Accessible a) {
        Object[] args = { a, handle };

        if (Build.DEBUG) {
            System.err.println("Popup menu " + handle + " has been opened");
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

    protected void handleWindowOpened(Accessible a) {
        if (a != null) {
            AccessibleContext ac = a.getAccessibleContext();
            if (ac != null) {
                int hwnd = getHWNDFromComponent(ac.getAccessibleComponent());
                if (hwnd > 0) {
                    registerFrame(new Integer(hwnd), a);
                }
            }
        }
    }

    public void eventDispatched(java.awt.AWTEvent evt) {
        switch (evt.getID()) {
            case java.awt.event.WindowEvent.WINDOW_OPENED:
                handleWindowOpened((Accessible) evt.getSource());
                break;
            case java.awt.event.WindowEvent.WINDOW_CLOSED:
                if (Build.DEBUG) {
                    System.err.println("retrieved WINDOW_CLOSED");
                }
                break;
            default:
                break;
        }
    }

    protected void handlePopupMenuOpened(AccessibleContext menu) {
        try {
            PopupMenuProxy pmp = new PopupMenuProxy(menu);
            int hwnd = getHWNDFromComponent(pmp.getAccessibleComponent());
            if (hwnd != 0) {
                registerPopup(new Integer(hwnd), pmp);
            }
        } catch (NullPointerException e) {
        } catch (IndexOutOfBoundsException e) {
        }
    }

    /** This method gets called when a bound property is changed. */
    public void propertyChange(java.beans.PropertyChangeEvent evt) {
        if (evt.getPropertyName().equals(AccessibleContext.ACCESSIBLE_STATE_PROPERTY)) {
            AccessibleContext ac = null;

            if (evt.getSource() instanceof AccessibleContext) {
                ac = (AccessibleContext) evt.getSource();
            } else if (evt.getSource() instanceof Accessible) {
                ac = ((Accessible) evt.getSource()).getAccessibleContext();
            } else {
                // This should never happen
                return;
            }

            // Track ComboBox popups by VISIBLE state of their lists ..
            if (ac.getAccessibleRole().equals(AccessibleRole.LIST)) {
                Accessible pa = ac.getAccessibleParent();
                if (pa != null) {
                    AccessibleContext pac = pa.getAccessibleContext();
                    if (pac != null && pac.getAccessibleRole().equals(AccessibleRole.COMBO_BOX)) {
                        if (evt.getOldValue() instanceof AccessibleState) {
                            AccessibleState as = (AccessibleState) evt.getOldValue();
                            if (as.equals(AccessibleState.VISIBLE)) {
                                if (Build.DEBUG) {
                                    System.err.println("ComboBox popup closed");
                                }
                            }
                        }
                        if (evt.getNewValue() instanceof AccessibleState) {
                            AccessibleState as = (AccessibleState) evt.getNewValue();
                            if (as.equals(AccessibleState.VISIBLE)) {
                                if (Build.DEBUG) {
                                    System.err.println("ComboBox popup opened");
                                }
                            }
                        }
                    }
                }
            }

            // Track Menu popups by SELECTED state changes ..
            if (ac.getAccessibleRole().equals(AccessibleRole.MENU)) {
                if (evt.getOldValue() instanceof AccessibleState) {
                    AccessibleState as = (AccessibleState) evt.getOldValue();
                    if (as.equals(AccessibleState.SELECTED)) {
                        if (Build.DEBUG) {
                            System.err.println("Popup menu closed");
                        }
                    }
                }
                if (evt.getNewValue() instanceof AccessibleState) {
                    AccessibleState as = (AccessibleState) evt.getNewValue();
                    if (as.equals(AccessibleState.SELECTED)) {
                        handlePopupMenuOpened(ac);
                    }
                }
            }
        }
    }

    protected static int getHWNDFromComponent(AccessibleComponent ac) {
        if (ac != null) {
            java.awt.Point p = ac.getLocationOnScreen();
            if (p != null) {
                java.awt.Dimension d = ac.getSize();
                if (d != null) {
                    return getHWNDFromPoint(p.x + (d.width % 2), p.y + (d.height % 2));
                }
            }
        }
        return -1;
    }

    protected static native int getHWNDFromPoint(int x, int y);

    protected void finalize() throws Throwable {
        Object[] params = { this };
        removePropertyChangeListener.invoke(null, params);
    }
}
