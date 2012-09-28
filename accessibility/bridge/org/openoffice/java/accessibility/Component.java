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

import java.awt.EventQueue;

import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleState;

import com.sun.star.uno.*;
import com.sun.star.accessibility.*;

public abstract class Component extends java.awt.Component {
    public static final Type RectangleType = new Type(com.sun.star.awt.Rectangle.class);
    public static final Type SelectionType = new Type(com.sun.star.awt.Selection.class);

    protected XAccessible unoAccessible;
    protected XAccessibleContext unoAccessibleContext;
    protected XAccessibleComponent unoAccessibleComponent;

    protected boolean disposed = false;

    protected Component(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super();
        unoAccessible = xAccessible;
        unoAccessibleContext = xAccessibleContext;
        unoAccessibleComponent = UnoRuntime.queryInterface(XAccessibleComponent.class, xAccessibleContext);
        // Add the event listener right away, because the global focus notification doesn't
        // work yet ..
        XAccessibleEventBroadcaster broadcaster = UnoRuntime.queryInterface(XAccessibleEventBroadcaster.class,
        unoAccessibleComponent);
        if (broadcaster != null) {
            broadcaster.addEventListener(createEventListener());
        }
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

    /*
     * Fake the java focus handling. This is necessary to keep OOo focus
     * in sync with the java focus. See java.awt.DefaultKeyboardFocusManager
     * for implementation details.
     **/

    /** Requests focus for this object */
    public void requestFocus() {
    }

    /** Requests focus for this object */
    public boolean requestFocus(boolean temporary) {
        // Must be a no-op to make focus handling work
        return true;
    }

    /** Requests the focus for this object in the containing window */
    public boolean requestFocusInWindow() {
        return requestFocusInWindow(false);
    }

    /** Requests the focus for this object in the containing window */
    protected boolean requestFocusInWindow(boolean temporary) {
        if (isFocusable() && isVisible()) {
            getEventQueue().postEvent(new java.awt.event.FocusEvent(this, java.awt.event.FocusEvent.FOCUS_GAINED, temporary));
            return true;
        }
        return false;
    }

    public Object[] getAccessibleComponents(Object[] targetSet) {
        try {
            java.util.ArrayList<java.awt.Component> list = new java.util.ArrayList<java.awt.Component>(targetSet.length);
            for (int i=0; i < targetSet.length; i++) {
                java.awt.Component c = AccessibleObjectFactory.getAccessibleComponent(
                    UnoRuntime.queryInterface(XAccessible.class, targetSet[i]));
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
            // Because this code is executed in the DispatchThread, it is better to catch every
            // exception that might occur
            try {
                AccessibleContext ac = accessibleContext;
                if (ac != null) {
                    ac.firePropertyChange(propertyName, oldValue, newValue);
                }
            } catch (java.lang.Exception e) {
                if (Build.DEBUG) {
                    System.err.println(e.getClass().getName() + " caught propagating " + propertyName + " event: " + e.getMessage());
                    e.printStackTrace();
                }
            }
        }
    }

    protected void firePropertyChange(String property, Object oldValue, Object newValue) {
        EventQueue.invokeLater(new PropertyChangeBroadcaster(property, oldValue, newValue));
    }

    protected void fireStatePropertyChange(AccessibleState state, boolean set) {
        PropertyChangeBroadcaster broadcaster;

//      if (Build.DEBUG) {
//          System.err.println("[" + AccessibleRoleAdapter.getAccessibleRole(unoAccessibleContext.getAccessibleRole()) + "] " +
//                  unoAccessibleContext.getAccessibleName() + " is " + (set ? "now " : "no longer ") + state);
//      }

        if (set) {
            broadcaster = new PropertyChangeBroadcaster(
                AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                null, state);
        } else {
            broadcaster = new PropertyChangeBroadcaster(
                AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                state, null);
        }
        EventQueue.invokeLater(broadcaster);
    }

    /**
    * Update the proxy objects appropriatly on property change events
    */
    protected class AccessibleUNOComponentListener implements XAccessibleEventListener {

        protected AccessibleUNOComponentListener() {
        }

        protected void setComponentState(short state, boolean enable) {
            switch (state) {
                case AccessibleStateType.ACTIVE:
                    // Only frames should be active
                    break;
                case AccessibleStateType.ARMED:
                    fireStatePropertyChange(AccessibleState.ARMED, enable);
                    break;
                case AccessibleStateType.CHECKED:
                    fireStatePropertyChange(AccessibleState.CHECKED, enable);
                    break;
                case AccessibleStateType.ENABLED:
                    setEnabled(enable);
                    // Since we can't access awt.Componet.accessibleContext, we need to fire
                    // this event manually ..
                    fireStatePropertyChange(AccessibleState.ENABLED, enable);
                    break;
                case AccessibleStateType.FOCUSED:
                    getEventQueue().postEvent(new java.awt.event.FocusEvent(
                        Component.this, enable ?
                        java.awt.event.FocusEvent.FOCUS_GAINED :
                        java.awt.event.FocusEvent.FOCUS_LOST));
                    break;
                case AccessibleStateType.PRESSED:
                    fireStatePropertyChange(AccessibleState.PRESSED, enable);
                    break;
                case AccessibleStateType.SELECTED:
                    fireStatePropertyChange(AccessibleState.SELECTED, enable);
                    break;
                case AccessibleStateType.SENSITIVE:
                    // This state equals ENABLED in OOo (but not in Gtk+) and does not exist in Java 1.5
                    break;
                case AccessibleStateType.SHOWING:
//                  fireStatePropertyChange(AccessibleState.SHOWING, enable);
                    break;
                case AccessibleStateType.VISIBLE:
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

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {

            if ( !disposed ) {

                switch (event.EventId) {
                    case AccessibleEventId.ACTION_CHANGED:
                        firePropertyChange(AccessibleContext.ACCESSIBLE_ACTION_PROPERTY,
                            toNumber(event.OldValue), toNumber(event.NewValue));
                        break;
                    case AccessibleEventId.NAME_CHANGED:
                        // Set the accessible name for the corresponding context, which will fire a property
                        // change event itself
                        handleNameChangedEvent(event.NewValue);
                        break;
                    case AccessibleEventId.DESCRIPTION_CHANGED:
                        // Set the accessible description for the corresponding context, which will fire a property
                        // change event itself - so do not set propertyName !
                        handleDescriptionChangedEvent(event.NewValue);
                        break;
                    case AccessibleEventId.CHILD:
                        if (Build.DEBUG) {
                            System.out.println("Unexpected child event for object of role " + getAccessibleContext().getAccessibleRole());
                        }
                        break;
                    case AccessibleEventId.STATE_CHANGED:
                        // Update the internal state set and fire the appropriate PropertyChangedEvent
                        handleStateChangedEvent(event.OldValue, event.NewValue);
                        break;
                    case AccessibleEventId.VISIBLE_DATA_CHANGED:
                    case AccessibleEventId.BOUNDRECT_CHANGED:
                        firePropertyChange(AccessibleContext.ACCESSIBLE_VISIBLE_DATA_PROPERTY, null, null);
                        break;
                    case AccessibleEventId.TEXT_CHANGED:
                        firePropertyChange(AccessibleContext.ACCESSIBLE_TEXT_PROPERTY,
                                                AccessibleTextImpl.convertTextSegment(event.OldValue),
                                                AccessibleTextImpl.convertTextSegment(event.NewValue));
                        break;
                    /*
                     * the Java AccessBridge for GNOME maps SELECTION_PROPERTY change events
                     * for objects of role TEXT to object:text-selection-changed
                     */
                    case AccessibleEventId.TEXT_SELECTION_CHANGED:
                        firePropertyChange(AccessibleContext.ACCESSIBLE_SELECTION_PROPERTY, null, null);
                        break;
                    case AccessibleEventId.CARET_CHANGED:
                        firePropertyChange(AccessibleContext.ACCESSIBLE_CARET_PROPERTY, toNumber(event.OldValue), toNumber(event.NewValue));
                        break;
                    case AccessibleEventId.VALUE_CHANGED:
                        firePropertyChange(AccessibleContext.ACCESSIBLE_VALUE_PROPERTY, toNumber(event.OldValue), toNumber(event.NewValue));
                    default:
                        // Warn about unhandled events
                        if(Build.DEBUG) {
                            System.out.println(this + ": unhandled accessibility event id=" + event.EventId);
                        }
                }
            }
        }

        /** Called by OpenOffice process to notify that the UNO component is disposing */
        public void disposing(com.sun.star.lang.EventObject eventObject) {
            disposed = true;
            AccessibleObjectFactory.disposing(Component.this);
        }
    }

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleUNOComponentListener();
    }

    protected javax.accessibility.AccessibleContext accessibleContext = null;

    /** This method actually creates the AccessibleContext object returned by
     * getAccessibleContext().
     */
    protected javax.accessibility.AccessibleContext createAccessibleContext() {
        return null;
    }

    /** Returns the AccessibleContext associated with this object */
    public final javax.accessibility.AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            try {
                AccessibleContext ac = createAccessibleContext();
                if (ac != null) {
                    // Set accessible name and description here to avoid
                    // unnecessary property change events later ..
                    ac.setAccessibleName(unoAccessibleContext.getAccessibleName());
                    ac.setAccessibleDescription(unoAccessibleContext.getAccessibleDescription());
                    accessibleContext = ac;
                }
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }
        return accessibleContext;
    }

    protected abstract class AccessibleUNOComponent extends java.awt.Component.AccessibleAWTComponent
        implements javax.accessibility.AccessibleExtendedComponent {

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

        /**
        * Gets the current state set of this object.
        *
        * @return an instance of <code>AccessibleStateSet</code>
        *    containing the current state set of the object
        * @see AccessibleState
        */
        public javax.accessibility.AccessibleStateSet getAccessibleStateSet() {
            if (disposed)
                return AccessibleStateAdapter.getDefunctStateSet();

            try {
                return AccessibleStateAdapter.getAccessibleStateSet(Component.this,
                    unoAccessibleContext.getAccessibleStateSet());
            } catch (com.sun.star.uno.RuntimeException e) {
                return AccessibleStateAdapter.getDefunctStateSet();
            }
        }

        /** Gets the locale of the component */
        public java.util.Locale getLocale() throws java.awt.IllegalComponentStateException {
            try {
                com.sun.star.lang.Locale unoLocale = unoAccessible.getAccessibleContext().getLocale();
                return new java.util.Locale(unoLocale.Language, unoLocale.Country);
            } catch (IllegalAccessibleComponentStateException e) {
                throw new java.awt.IllegalComponentStateException(e.getMessage());
            } catch (com.sun.star.uno.RuntimeException e) {
                return java.util.Locale.getDefault();
            }
        }

        /*
        * AccessibleExtendedComponent
        */

        /** Returns the background color of the object */
        public java.awt.Color getBackground() {
            try {
                return new java.awt.Color(unoAccessibleComponent.getBackground());
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
                return new java.awt.Color(unoAccessibleComponent.getForeground());
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
            return Component.this.isEnabled();
        }

        public void setEnabled(boolean b) {
            // Not supported by UNO accessibility API
        }

        public boolean isVisible() {
            return Component.this.isVisible();
        }

        public void setVisible(boolean b) {
            // Not supported by UNO accessibility API
        }

        public boolean isShowing() {
            return Component.this.isShowing();
        }

        public boolean contains(java.awt.Point p) {
            try {
                return unoAccessibleComponent.containsPoint(new com.sun.star.awt.Point(p.x, p.y));
            } catch (com.sun.star.uno.RuntimeException e) {
                return false;
            }
        }

        /** Returns the location of the object on the screen. */
        public java.awt.Point getLocationOnScreen() {
            try {
                com.sun.star.awt.Point unoPoint = unoAccessibleComponent.getLocationOnScreen();
//              if (Build.DEBUG) {
//                  System.err.println("Returning location on screen( " + unoPoint.X + ", " + unoPoint.Y + " )" );
//              }
                return new java.awt.Point(unoPoint.X, unoPoint.Y);
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /** Gets the location of this component in the form of a point specifying the component's top-left corner */
        public java.awt.Point getLocation() {
            try {
                com.sun.star.awt.Point unoPoint = unoAccessibleComponent.getLocation();
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
                com.sun.star.awt.Rectangle unoRect = unoAccessibleComponent.getBounds();
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
                com.sun.star.awt.Size unoSize = unoAccessibleComponent.getSize();
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
            return Component.this.isFocusable();
        }

        public void requestFocus() {
            unoAccessibleComponent.grabFocus();
        }

        public String getToolTipText() {
            try {
                XAccessibleExtendedComponent unoAccessibleExtendedComponent = UnoRuntime.queryInterface(XAccessibleExtendedComponent.class, unoAccessibleComponent);
                if (unoAccessibleExtendedComponent != null) {
                    return unoAccessibleExtendedComponent.getToolTipText();
                }
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
            return null;
        }

        public String getTitledBorderText() {
            try {
                XAccessibleExtendedComponent unoAccessibleExtendedComponent = UnoRuntime.queryInterface(XAccessibleExtendedComponent.class, unoAccessibleComponent);
                if (unoAccessibleExtendedComponent != null) {
                    return unoAccessibleExtendedComponent.getTitledBorderText();
                }
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
            return null;
        }

        public javax.accessibility.AccessibleKeyBinding getAccessibleKeyBinding() {
            try {
                XAccessibleAction unoAccessibleAction = UnoRuntime.queryInterface(XAccessibleAction.class, unoAccessibleComponent);
                if (unoAccessibleAction != null) {
                    XAccessibleKeyBinding unoAccessibleKeyBinding = unoAccessibleAction.getAccessibleActionKeyBinding(0);
                    if (unoAccessibleKeyBinding != null) {
                        return new AccessibleKeyBinding(unoAccessibleKeyBinding);
                    }
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return null;
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
            return null;
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

