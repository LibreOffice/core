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

package org.openoffice.java.accessibility;

import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleComponent;
// import com.sun.star.accessibility.XAccessibleExtendedComponent;
import com.sun.star.uno.UnoRuntime;

public class AccessibleComponentImpl implements javax.accessibility.AccessibleComponent {

    protected XAccessibleComponent unoObject;
//  protected XAccessibleExtendedComponent unoAccessibleExtendedComponent = null;

    /** Creates new AccessibleComponentImpl */
    public AccessibleComponentImpl(XAccessibleComponent xAccessibleComponent) {
        unoObject = xAccessibleComponent;
    }

    protected boolean hasState(short state) {
        try {
            XAccessibleContext unoAccessibleContext = UnoRuntime.queryInterface(XAccessibleContext.class, unoObject);
            // All UNO accessibility implementations must support XAccessibleContext
            // and return a valid XAccessibleStateSet ..
            return unoAccessibleContext.getAccessibleStateSet().contains(state);
        } catch (com.sun.star.uno.RuntimeException e) {
            return false;
        } catch (java.lang.NullPointerException e) {
            System.err.println("XAccessibleContext unsupported or no XAccessibleStateSet returned.");
            return false;
        }
    }

    /*
     * XAccessibleComponent
     */

    /** Returns the background color of the object */
    public java.awt.Color getBackground() {
        try {
            return new java.awt.Color(unoObject.getBackground());
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    public void setBackground(java.awt.Color c) {
        // Not supported by UNO accessibility API
    }

    /** Returns the foreground color of the object */
    public java.awt.Color getForeground() {
        try {
            return new java.awt.Color(unoObject.getForeground());
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
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
        return hasState(AccessibleStateType.ENABLED);
        }

    public void setEnabled(boolean b) {
        // Not supported by UNO accessibility API
    }

    public boolean isVisible() {
        return hasState(AccessibleStateType.VISIBLE);
    }

    public void setVisible(boolean b) {
        // Not supported by UNO accessibility API
    }

    public boolean isShowing() {
        return hasState(AccessibleStateType.SHOWING);
    }

    public boolean contains(java.awt.Point p) {
        try {
            return unoObject.containsPoint(new com.sun.star.awt.Point(p.x, p.y));
        } catch (com.sun.star.uno.RuntimeException e) {
            return false;
        }
    }

    /** Returns the location of the object on the screen. */
    public java.awt.Point getLocationOnScreen() {
        try {
            com.sun.star.awt.Point unoPoint = unoObject.getLocationOnScreen();
            return new java.awt.Point(unoPoint.X, unoPoint.Y);
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    /** Gets the location of this component in the form of a point specifying the component's top-left corner */
    public java.awt.Point getLocation() {
        try {
            com.sun.star.awt.Point unoPoint = unoObject.getLocation();
            return new java.awt.Point( unoPoint.X, unoPoint.Y );
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    /** Moves this component to a new location */
    public void setLocation(java.awt.Point p) {
        // Not supported by UNO accessibility API
    }

    /** Gets the bounds of this component in the form of a Rectangle object */
    public java.awt.Rectangle getBounds() {
        try {
            com.sun.star.awt.Rectangle unoRect = unoObject.getBounds();
            return new java.awt.Rectangle(unoRect.X, unoRect.Y, unoRect.Width, unoRect.Height);
            } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    /** Moves and resizes this component to conform to the new bounding rectangle r */
    public void setBounds(java.awt.Rectangle r) {
        // Not supported by UNO accessibility API
    }

    /** Returns the size of this component in the form of a Dimension object */
    public java.awt.Dimension getSize() {
        try {
            com.sun.star.awt.Size unoSize = unoObject.getSize();
            return new java.awt.Dimension(unoSize.Width, unoSize.Height);
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    /** Resizes this component so that it has width d.width and height d.height */
    public void setSize(java.awt.Dimension d) {
        // Not supported by UNO accessibility API
    }

    public javax.accessibility.Accessible getAccessibleAt(java.awt.Point p) {
        // Not supported by this implementation
        return null;
    }

    public boolean isFocusTraversable() {
        return hasState(AccessibleStateType.FOCUSABLE);
        }

    public void requestFocus() {
        unoObject.grabFocus();
    }

    /**
    * Adds the specified focus listener to receive focus events from
    * this component when this component gains input focus.
    * If listener <code>l</code> is <code>null</code>,
    * no exception is thrown and no action is performed.
    */

    public void addFocusListener(java.awt.event.FocusListener l) {
        // Not supported by this implementation
    }

    /**
    * Removes the specified focus listener so that it no longer
    * receives focus events from this component. This method performs
    * no function, nor does it throw an exception, if the listener
    * specified by the argument was not previously added to this component.
    * If listener <code>l</code> is <code>null</code>,
    * no exception is thrown and no action is performed.
    */

    public void removeFocusListener(java.awt.event.FocusListener l) {
        // Not supported by this implementation
    }
}
