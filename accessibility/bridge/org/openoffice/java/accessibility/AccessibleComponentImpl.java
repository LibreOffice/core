/*************************************************************************
 *
 *  $RCSfile: AccessibleComponentImpl.java,v $
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
            XAccessibleContext unoAccessibleContext = (XAccessibleContext)
                UnoRuntime.queryInterface(XAccessibleContext.class, unoObject);
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
