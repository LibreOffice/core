/*************************************************************************
 *
 *  $RCSfile: Component.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2002-12-06 11:25:34 $
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

import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleState;

import com.sun.star.uno.*;
import drafts.com.sun.star.accessibility.AccessibleEventId;
import drafts.com.sun.star.accessibility.AccessibleEventObject;
import drafts.com.sun.star.accessibility.AccessibleStateType;
import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;
import drafts.com.sun.star.accessibility.XAccessibleEventListener;
import drafts.com.sun.star.accessibility.XAccessibleEventBroadcaster;

public abstract class Component extends java.awt.Component {
    final static String monitorClassName =
        "com.sun.java.accessibility.util.AccessibilityEventMonitor$AccessibilityEventListener";

    public static final Type RectangleType = new Type(com.sun.star.awt.Rectangle.class);

    protected XAccessible unoAccessible = null;
    protected XAccessibleComponent unoAccessibleComponent = null;

    protected Component() {
        super();
//      enableEvents(java.awt.AWTEvent.FOCUS_EVENT_MASK);
    }

    /**
    * Determines whether this <code>Component</code> is showing on screen.
    * This means that the component must be visible, and it must be in a
    * <code>container</code> that is visible and showing.
    * @see       #addNotify
    * @see       #removeNotify
    * @since JDK1.0
    */
    public boolean isShowing() {
        if (isVisible()) {
            java.awt.Container parent = getParent();
            return (parent == null) || parent.isShowing();
        }
        return false;
    }

    /**
    * Makes this <code>Component</code> displayable by connecting it to a
    * native screen resource.
    * This method is called internally by the toolkit and should
    * not be called directly by programs.
    * @see       #isDisplayable
    * @see       #removeNotify
    * @since JDK1.0
    */
    public void addNotify() {
    }

    /**
    * Makes this <code>Component</code> undisplayable by destroying it native
    * screen resource.
    * This method is called by the toolkit internally and should
    * not be called directly by programs.
    * @see       #isDisplayable
    * @see       #addNotify
    * @since JDK1.0
    */
    public void removeNotify() {
    }

    /** Requests focus for this object */
    public void requestFocus() {
        if (isShowing()) {
            try {
                unoAccessibleComponent.grabFocus();
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }
    }
    /** Requests focus for this object */
    protected boolean requestFocus(boolean temporary) {
        // Must be a no-op to make focus handling work
        return true;
    }

    /** Returns the location of the object on the screen. */
    public java.awt.Point getLocationOnScreen() {
        java.awt.Container parent = getParent();
        if (parent != null) {
            // Retrieve location of the parent object
            java.awt.Point p = parent.getLocationOnScreen();

            // Add relative coordinates of the object
            if (p != null) {
                p.translate(getX(), getY());
                return p;
            }
        }

        return null;
    }

    /** Returns the foreground color of the object */
    public java.awt.Color getForeground() {
        if (isShowing()) {
            try {
                return new java.awt.Color(unoAccessibleComponent.getForeground());
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        } else {
            return null;
        }
    }

    /** Returns the background color of the object */
    public java.awt.Color getBackground() {
        if (isShowing()) {
            try {
                return new java.awt.Color(unoAccessibleComponent.getBackground());
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        } else {
            return null;
        }
    }

    public Object[] getAccessibleComponents(Object[] targetSet) {
        try {
            AccessibleObjectFactory factory = AccessibleObjectFactory.getDefault();
            java.util.ArrayList list = new java.util.ArrayList(targetSet.length);
            for (int i=0; i < targetSet.length; i++) {
                java.awt.Component c = factory.getAccessibleComponent(
                    (XAccessible) UnoRuntime.queryInterface(XAccessible.class, targetSet[i]));
                if (c != null) {
                    list.add(c);
                }
            }
            list.trimToSize();
            return list.toArray();
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    protected void addAccessibleEventListener(XAccessibleEventListener listener) {
        XAccessibleEventBroadcaster broadcaster = (XAccessibleEventBroadcaster)
            UnoRuntime.queryInterface(XAccessibleEventBroadcaster.class,
            unoAccessibleComponent);
        if (broadcaster != null) {
            broadcaster.addEventListener(listener);
        }
    }

    protected java.awt.EventQueue getEventQueue() {
        return java.awt.Toolkit.getDefaultToolkit().getSystemEventQueue();
    }

    protected class PropertyChangeBroadcaster implements Runnable {
        String propertyName;
        Object oldValue;
        Object newValue;

        public PropertyChangeBroadcaster(String name, Object param1, Object param2) {
            propertyName = name;
            oldValue = param1;
            newValue = param2;
        }

        public void run() {
            AccessibleContext ac = accessibleContext;
            if (ac != null) {
                ac.firePropertyChange(propertyName, oldValue, newValue);
            } else if (Build.DEBUG) {
                XAccessibleContext xac = unoAccessible.getAccessibleContext();
                System.err.println("Ignoring event: " + propertyName + " for " +
                    AccessibleRoleAdapter.getAccessibleRole(xac.getAccessibleRole()) +
                    " " + xac.getAccessibleName());
            }
        }
    }

    protected void firePropertyChange(String property, Object oldValue, Object newValue) {
        getEventQueue().invokeLater(new PropertyChangeBroadcaster(property, oldValue, newValue));
    }

    protected void fireStatePropertyChange(AccessibleState state, boolean set) {
        PropertyChangeBroadcaster broadcaster;
        if (set) {
            broadcaster = new PropertyChangeBroadcaster(
                accessibleContext.ACCESSIBLE_STATE_PROPERTY,
                null, state);
        } else {
            broadcaster = new PropertyChangeBroadcaster(
                accessibleContext.ACCESSIBLE_STATE_PROPERTY,
                state, null);
        }
        getEventQueue().invokeLater(broadcaster);
    }

    /**
    * Update the proxy objects appropriatly on property change events
    */
    protected abstract class AccessibleUNOComponentListener implements XAccessibleEventListener {

        protected AccessibleUNOComponentListener() {
        }

        protected void setComponentState(short state, boolean enable) {
            switch (state) {
                case AccessibleStateType.ACTIVE:
                    // Only frames should be active
                    break;
                case AccessibleStateType.ENABLED:
                    setEnabled(enable);
                    break;
                case AccessibleStateType.FOCUSED:
                    getEventQueue().postEvent(new java.awt.event.FocusEvent(
                        Component.this, enable ?
                        java.awt.event.FocusEvent.FOCUS_GAINED :
                        java.awt.event.FocusEvent.FOCUS_LOST));
                    break;
                case AccessibleStateType.SELECTED:
                    fireStatePropertyChange(AccessibleState.SELECTED, enable);
                    break;
                case AccessibleStateType.SHOWING:
                    if (enable) {
                        // Query bounding boxes if component becomes visible
                        com.sun.star.awt.Rectangle r = unoAccessibleComponent.getBounds();
                        Component.this.setBounds(r.X, r.Y, r.Width, r.Height);
                    }
//                  fireStatePropertyChange(AccessibleState.SHOWING, enable);
                    break;
                case AccessibleStateType.VISIBLE:
                    if (enable) {
                        // Query bounding boxes if component becomes visible
                        com.sun.star.awt.Rectangle r = unoAccessibleComponent.getBounds();
                        Component.this.setBounds(r.X, r.Y, r.Width, r.Height);
                    }
                    Component.this.setVisible(enable);
                    break;
                default:
                    if (Build.DEBUG) {
                        System.err.println("[component]: " + getName() + "unexpected state change " + state);
                    }
                    break;
            }
        }

        /** Updates the accessible name and fires the appropriate PropertyChangedEvent */
        protected void handleNameChangedEvent(Object any) {
            try {
                // This causes the property change event to be fired in the VCL thread
                // context. If this causes problems, it has to be deligated to the java
                // dispatch thread ..
                if (accessibleContext != null) {
                    accessibleContext.setAccessibleName(AnyConverter.toString(any));
                }
            } catch (com.sun.star.lang.IllegalArgumentException e) {
            }
        }

        /** Updates the accessible description and fires the appropriate PropertyChangedEvent */
        protected void handleDescriptionChangedEvent(Object any) {
            try {
                // This causes the property change event to be fired in the VCL thread
                // context. If this causes problems, it has to be deligated to the java
                // dispatch thread ..
                if (accessibleContext != null) {
                    accessibleContext.setAccessibleDescription(AnyConverter.toString(any));
                }
            } catch (com.sun.star.lang.IllegalArgumentException e) {
            }
        }

        /** Updates the internal states and fires the appropriate PropertyChangedEvent */
        protected void handleStateChangedEvent(Object any1, Object any2) {
            try {
                if (AnyConverter.isShort(any1)) {
                    setComponentState(AnyConverter.toShort(any1), false);
                }

                if (AnyConverter.isShort(any2)) {
                    setComponentState(AnyConverter.toShort(any2), true);
                }
            } catch (com.sun.star.lang.IllegalArgumentException e) {
            }
        }

        /** Fires a visible data property change event */
        protected void handleVisibleDataEvent() {
            try {
                com.sun.star.awt.Rectangle r = unoAccessibleComponent.getBounds();
                Component.this.setBounds(r.X, r.Y, r.Width, r.Height);
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }

        /** Updates internal bounding box cache */
        protected void handleBoundRectEvent() {
            try {
                com.sun.star.awt.Rectangle r = unoAccessibleComponent.getBounds();
                Component.this.setBounds(r.X, r.Y, r.Width, r.Height);
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch (event.EventId) {
                case AccessibleEventId.ACCESSIBLE_NAME_EVENT:
                    // Set the accessible name for the corresponding context, which will fire a property
                    // change event itself
                    handleNameChangedEvent(event.NewValue);
                    break;
                case AccessibleEventId.ACCESSIBLE_DESCRIPTION_EVENT:
                    // Set the accessible description for the corresponding context, which will fire a property
                    // change event itself - so do not set propertyName !
                    handleDescriptionChangedEvent(event.NewValue);
                    break;
                case AccessibleEventId.ACCESSIBLE_STATE_EVENT:
                    // Update the internal state set and fire the appropriate PropertyChangedEvent
                    handleStateChangedEvent(event.OldValue, event.NewValue);
                    break;
                case AccessibleEventId.ACCESSIBLE_VISIBLE_DATA_EVENT:
                    handleVisibleDataEvent();
                    firePropertyChange(AccessibleContext.ACCESSIBLE_VISIBLE_DATA_PROPERTY, null, null);
                    break;
                case AccessibleEventId.ACCESSIBLE_BOUNDRECT_EVENT:
                    handleBoundRectEvent();
                    break;
                case AccessibleEventId.ACCESSIBLE_TEXT_EVENT:
                    // FIXME: enhance text information ..
                    firePropertyChange(AccessibleContext.ACCESSIBLE_TEXT_PROPERTY, null, new Integer(0));
                    break;
                default:
                    // Warn about unhandled events
                    if(Build.DEBUG) {
                        System.out.println(this + ": unhandled accessibility event id=" + event.EventId);
                    }
            }
        }

        /** Called by OpenOffice process to notify that the UNO component is disposing */
        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }
    }

    protected AccessibleContext accessibleContext = null;

    protected abstract class AccessibleUNOComponent extends java.awt.Component.AccessibleAWTComponent {

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleUNOComponent() {
            super();
            // Set accessible name and description here to avoid unnecessary property change
            // events later ..
            XAccessibleContext unoAccessibleContext = unoAccessible.getAccessibleContext();
            String s = unoAccessibleContext.getAccessibleName();
            if (s != null && s.length() > 0) {
                setAccessibleName(s);
            }
            s = unoAccessibleContext.getAccessibleDescription();
            if (s != null && s.length() > 0) {
                setAccessibleDescription(s);
            }
        }

        protected java.awt.event.ComponentListener accessibleComponentHandler = null;

        /**
        * Fire PropertyChange listener, if one is registered,
        * when shown/hidden..
        */
        protected class AccessibleComponentHandler implements java.awt.event.ComponentListener {
            public void componentHidden(java.awt.event.ComponentEvent e)  {
                AccessibleUNOComponent.this.firePropertyChange(
                    AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    AccessibleState.VISIBLE, null);
            }

            public void componentShown(java.awt.event.ComponentEvent e)  {
                AccessibleUNOComponent.this.firePropertyChange(
                    AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    null, AccessibleState.VISIBLE);
            }

            public void componentMoved(java.awt.event.ComponentEvent e)  {
            }

            public void componentResized(java.awt.event.ComponentEvent e)  {
            }
        } // inner class AccessibleComponentHandler

        protected java.awt.event.FocusListener accessibleFocusHandler = null;

        /**
        * Fire PropertyChange listener, if one is registered,
        * when focus events happen
        */
        protected class AccessibleFocusHandler implements java.awt.event.FocusListener {
            public void focusGained(java.awt.event.FocusEvent event) {
                AccessibleUNOComponent.this.firePropertyChange(
                    AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    null, AccessibleState.FOCUSED);
                if (Build.DEBUG) {
                    System.err.println("[" + getAccessibleRole() + "] " + getAccessibleName() + " is now focused");
                }
            }
            public void focusLost(java.awt.event.FocusEvent event) {
                AccessibleUNOComponent.this.firePropertyChange(
                    AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    AccessibleState.FOCUSED, null);
                if (Build.DEBUG) {
                    System.err.println("[" + getAccessibleRole() + "] " + getAccessibleName() + " is no longer focused");
                }
            }
        } // inner class AccessibleFocusHandler

        protected int propertyChangeListenerCount = 0;

        /**
        * Add a PropertyChangeListener to the listener list.
        *
        * @param listener  The PropertyChangeListener to be added
        */
        public void addPropertyChangeListener(java.beans.PropertyChangeListener listener) {
            if (propertyChangeListenerCount++ == 0) {
                accessibleComponentHandler = new AccessibleComponentHandler();
                Component.this.addComponentListener(accessibleComponentHandler);

                accessibleFocusHandler = new AccessibleFocusHandler();
                Component.this.addFocusListener(accessibleFocusHandler);
            }
            super.addPropertyChangeListener(listener);
        }

        /**
        * Remove a PropertyChangeListener from the listener list.
        * This removes a PropertyChangeListener that was registered
        * for all properties.
        *
        * @param listener  The PropertyChangeListener to be removed
        */
        public void removePropertyChangeListener(java.beans.PropertyChangeListener listener) {
            if (--propertyChangeListenerCount == 0) {
                Component.this.removeComponentListener(accessibleComponentHandler);
                accessibleComponentHandler = null;

                Component.this.removeFocusListener(accessibleFocusHandler);
                accessibleFocusHandler = null;
            }
            super.removePropertyChangeListener(listener);
        }
    }

    // Extract a number from a UNO any
    public static java.lang.Number toNumber(java.lang.Object any) {
        try {
            if (AnyConverter.isByte(any)) {
                return new Byte(AnyConverter.toByte(any));
            } else if (AnyConverter.isShort(any)) {
                return new Short(AnyConverter.toShort(any));
            } else if (AnyConverter.isInt(any)) {
                return new Integer(AnyConverter.toInt(any));
            } else if (AnyConverter.isLong(any)) {
                return new Long(AnyConverter.toLong(any));
            } else if (AnyConverter.isFloat(any)) {
                return new Float(AnyConverter.toFloat(any));
            } else if (AnyConverter.isDouble(any)) {
                return new Double(AnyConverter.toDouble(any));
            }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            throw new IllegalArgumentException(e.getMessage());
        }
            return null;
    }

    public String toString() {
        return UnoRuntime.generateOid(unoAccessible);
    }
}

