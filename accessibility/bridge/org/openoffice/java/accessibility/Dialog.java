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

import com.sun.star.uno.*;
import com.sun.star.accessibility.*;

public class Dialog extends java.awt.Dialog implements javax.accessibility.Accessible, NativeFrame {
    protected XAccessibleComponent unoAccessibleComponent;

    boolean opened = false;
    boolean visible = false;
    boolean active = false;

    java.awt.EventQueue eventQueue = null;

    protected Dialog(java.awt.Frame owner, XAccessibleComponent xAccessibleComponent) {
        super(owner);
        initialize(xAccessibleComponent);
    }

    protected Dialog(java.awt.Frame owner, String name, XAccessibleComponent xAccessibleComponent) {
        super(owner, name);
        initialize(xAccessibleComponent);
    }

    protected Dialog(java.awt.Frame owner, String name, boolean modal, XAccessibleComponent xAccessibleComponent) {
        super(owner, name, modal);
        initialize(xAccessibleComponent);
    }

    private void initialize(XAccessibleComponent xAccessibleComponent) {
        unoAccessibleComponent = xAccessibleComponent;
        eventQueue = java.awt.Toolkit.getDefaultToolkit().getSystemEventQueue();
        XAccessibleEventBroadcaster broadcaster = UnoRuntime.queryInterface(XAccessibleEventBroadcaster.class,
        xAccessibleComponent);
        if (broadcaster != null) {
            broadcaster.addEventListener(new AccessibleDialogListener());
        }
    }

    java.awt.Component initialComponent = null;

    public java.awt.Component getInitialComponent() {
        return initialComponent;
    }

    public void setInitialComponent(java.awt.Component c) {
        initialComponent = c;
    }

    public Integer getHWND() {
        return null;
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
//      createHierarchyEvents(0, null, null, 0, false);
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

        /**
         * Determines if the object is visible.  Note: this means that the
         * object intends to be visible; however, it may not in fact be
         * showing on the screen because one of the objects that this object
         * is contained by is not visible.  To determine if an object is
         * showing on the screen, use <code>isShowing</code>.
         *
         * @return true if object is visible; otherwise, false
         */
    public boolean isVisible(){
        return visible;
    }

    /**
    * Shows or hides this component depending on the value of parameter
    * <code>b</code>.
    * @param b  if <code>true</code>, shows this component;
    * otherwise, hides this component
    * @see #isVisible
    * @since JDK1.1
    */
    public void setVisible(boolean b) {
        if (visible != b){
            visible = b;
            if (b) {
                // If it is the first show, fire WINDOW_OPENED event
                if (!opened) {
                    postWindowEvent(java.awt.event.WindowEvent.WINDOW_OPENED);
                    opened = true;
                }
                postComponentEvent(java.awt.event.ComponentEvent.COMPONENT_SHOWN);
            } else {
                postComponentEvent(java.awt.event.ComponentEvent.COMPONENT_HIDDEN);
            }
        }
    }

    public void dispose() {
        setVisible(false);
        postWindowEvent(java.awt.event.WindowEvent.WINDOW_CLOSED);
    }

    protected void postWindowEvent(int i) {
        eventQueue.postEvent(new java.awt.event.WindowEvent(this, i));
    }

    protected void postComponentEvent(int i) {
        eventQueue.postEvent(new java.awt.event.ComponentEvent(this, i));
    }

    /**
    * Update the proxy objects appropriatly on property change events
    */
    protected class AccessibleDialogListener implements XAccessibleEventListener {

        protected AccessibleDialogListener() {
        }

        protected void setComponentState(short state, boolean enable) {
            switch (state) {
                case AccessibleStateType.ACTIVE:
                    active = enable;
                    if (enable) {
                        AccessibleObjectFactory.postWindowActivated(Dialog.this);
                    } else {
                        AccessibleObjectFactory.postWindowLostFocus(Dialog.this);
                    }
                    break;
                case AccessibleStateType.ICONIFIED:
                    postWindowEvent(enable ?
                        java.awt.event.WindowEvent.WINDOW_ICONIFIED :
                        java.awt.event.WindowEvent.WINDOW_DEICONIFIED);
                    break;
                case AccessibleStateType.VISIBLE:
                    Dialog.this.setVisible(enable);
                    break;
                default:
                    if (Build.DEBUG) {
                        System.err.println("[dialog]: " + getTitle() + "unexpected state change " + state);
                    }
                    break;
            }
        }

        /** Updates the accessible name and fires the appropriate PropertyChangedEvent */
        protected void handleNameChangedEvent(Object any) {
            try {
                String title = AnyConverter.toString(any);
                setTitle(title);
                // This causes the property change event to be fired in the VCL thread
                // context. If this causes problems, it has to be deligated to the java
                // dispatch thread ..
                javax.accessibility.AccessibleContext ac = accessibleContext;
                if (ac!= null) {
                    ac.setAccessibleName(title);
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
                javax.accessibility.AccessibleContext ac = accessibleContext;
                if (ac!= null) {
                    ac.setAccessibleDescription(AnyConverter.toString(any));
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
            javax.accessibility.AccessibleContext ac = accessibleContext;
            if (ac != null) {
                ac.firePropertyChange(javax.accessibility.AccessibleContext.ACCESSIBLE_VISIBLE_DATA_PROPERTY, null, null);
            }
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch (event.EventId) {
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
                case AccessibleEventId.STATE_CHANGED:
                    // Update the internal state set and fire the appropriate PropertyChangedEvent
                    handleStateChangedEvent(event.OldValue, event.NewValue);
                    break;
                case AccessibleEventId.CHILD:
                    if (AnyConverter.isObject(event.OldValue)) {
                        AccessibleObjectFactory.removeChild(Dialog.this, event.OldValue);
                    } else if (AnyConverter.isObject(event.NewValue)) {
                        AccessibleObjectFactory.addChild(Dialog.this, event.NewValue);
                    }
                    break;
                case AccessibleEventId.VISIBLE_DATA_CHANGED:
                case AccessibleEventId.BOUNDRECT_CHANGED:
                    handleVisibleDataEvent();
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

    javax.accessibility.AccessibleContext accessibleContext = null;

    /** Returns the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessibleDialog();
            accessibleContext.setAccessibleName(getTitle());
        }
        return accessibleContext;
    }

    protected class AccessibleDialog extends java.awt.Dialog.AccessibleAWTDialog {
        protected AccessibleDialog() {
            super();
        }

        protected java.awt.event.ComponentListener accessibleComponentHandler = null;

        /**
        * Fire PropertyChange listener, if one is registered,
        * when shown/hidden..
        */
        protected class AccessibleComponentHandler implements java.awt.event.ComponentListener {
            public void componentHidden(java.awt.event.ComponentEvent e)  {
                AccessibleDialog.this.firePropertyChange(
                    javax.accessibility.AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    javax.accessibility.AccessibleState.VISIBLE, null);
            }

            public void componentShown(java.awt.event.ComponentEvent e)  {
                AccessibleDialog.this.firePropertyChange(
                    javax.accessibility.AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    null, javax.accessibility.AccessibleState.VISIBLE);
            }

            public void componentMoved(java.awt.event.ComponentEvent e)  {
            }

            public void componentResized(java.awt.event.ComponentEvent e)  {
            }
        } // inner class AccessibleComponentHandler

        protected java.awt.event.WindowListener accessibleWindowHandler = null;

        /**
        * Fire PropertyChange listener, if one is registered,
        * when window events happen
        */
        protected class AccessibleWindowHandler implements java.awt.event.WindowListener {
            /** Invoked when the Window is set to be the active Window. */
            public void windowActivated(java.awt.event.WindowEvent e) {
                AccessibleDialog.this.firePropertyChange(
                    javax.accessibility.AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    null, javax.accessibility.AccessibleState.ACTIVE);
                if (Build.DEBUG) {
                    System.err.println("[Dialog] " + getTitle() + " is now active");
                }
            }

            /** Invoked when a window has been closed as the result of calling dispose on the window. */
            public void windowClosed(java.awt.event.WindowEvent e) {
                if (Build.DEBUG) {
                    System.err.println("[Dialog] " + getTitle() + " has been closed");
                }
            }

            /** Invoked when the user attempts to close the window from the window's system menu. */
            public void windowClosing(java.awt.event.WindowEvent e) {
                if (Build.DEBUG) {
                    System.err.println("[Dialog] " + getTitle() + " is closing");
                }
            }

            /** Invoked when a Window is no longer the active Window. */
            public void windowDeactivated(java.awt.event.WindowEvent e) {
                AccessibleDialog.this.firePropertyChange(
                    javax.accessibility.AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    javax.accessibility.AccessibleState.ACTIVE, null);
                if (Build.DEBUG) {
                    System.err.println("[Dialog] " + getTitle() + " is no longer active");
                }
            }

            /** Invoked when a window is changed from a minimized to a normal state. */
            public void windowDeiconified(java.awt.event.WindowEvent e) {
                if (Build.DEBUG) {
                    System.err.println("[Dialog] " + getTitle() + " has been deiconified");
                }
            }

            /** Invoked when a window is changed from a normal to a minimized state. */
            public void windowIconified(java.awt.event.WindowEvent e) {
                if (Build.DEBUG) {
                    System.err.println("[Dialog] " + getTitle() + " has been iconified");
                }
            }

            /** Invoked the first time a window is made visible. */
            public void windowOpened(java.awt.event.WindowEvent e) {
                if (Build.DEBUG) {
                    System.err.println("[Dialog] " + getTitle() + " has been opened");
                }
            }

        } // inner class AccessibleWindowHandler

        protected java.awt.event.ContainerListener accessibleContainerHandler = null;

        /**
        * Fire PropertyChange listener, if one is registered,
        * when children added/removed.
        */

        protected class AccessibleContainerHandler implements java.awt.event.ContainerListener {
            public void componentAdded(java.awt.event.ContainerEvent e) {
                java.awt.Component c = e.getChild();
                if (c != null && c instanceof javax.accessibility.Accessible) {
                    AccessibleDialog.this.firePropertyChange(
                        javax.accessibility.AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                        null, ((javax.accessibility.Accessible) c).getAccessibleContext());
                }
            }
            public void componentRemoved(java.awt.event.ContainerEvent e) {
                java.awt.Component c = e.getChild();
                if (c != null && c instanceof javax.accessibility.Accessible) {
                    AccessibleDialog.this.firePropertyChange(
                        javax.accessibility.AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                        ((javax.accessibility.Accessible) c).getAccessibleContext(), null);
                }
            }
        }

        protected int propertyChangeListenerCount = 0;

        /**
        * Add a PropertyChangeListener to the listener list.
        *
        * @param listener  The PropertyChangeListener to be added
        */
        public void addPropertyChangeListener(java.beans.PropertyChangeListener listener) {
            if (propertyChangeListenerCount++ == 0) {
                accessibleWindowHandler = new AccessibleWindowHandler();
                Dialog.this.addWindowListener(accessibleWindowHandler);

                accessibleContainerHandler = new AccessibleContainerHandler();
                Dialog.this.addContainerListener(accessibleContainerHandler);

                accessibleComponentHandler = new AccessibleComponentHandler();
                Dialog.this.addComponentListener(accessibleComponentHandler);
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
                Dialog.this.removeComponentListener(accessibleComponentHandler);
                accessibleComponentHandler = null;

                Dialog.this.removeContainerListener(accessibleContainerHandler);
                accessibleContainerHandler = null;

                Dialog.this.removeWindowListener(accessibleWindowHandler);
                accessibleWindowHandler = null;
            }
            super.removePropertyChangeListener(listener);
        }

        /*
        * AccessibleComponent
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
            return Dialog.this.isEnabled();
        }

        public void setEnabled(boolean b) {
            // Not supported by UNO accessibility API
        }

        public boolean isVisible() {
            return Dialog.this.isVisible();
        }

        public void setVisible(boolean b) {
            // Not supported by UNO accessibility API
        }

        public boolean isShowing() {
            return Dialog.this.isShowing();
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

        /** Returns the Accessible child, if one exists, contained at the local coordinate Point */
        public javax.accessibility.Accessible getAccessibleAt(java.awt.Point p) {
            try {
                java.awt.Component c = AccessibleObjectFactory.getAccessibleComponent(
                    unoAccessibleComponent.getAccessibleAtPoint(new com.sun.star.awt.Point(p.x, p.y)));

                return (javax.accessibility.Accessible) c;
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        public boolean isFocusTraversable() {
            return Dialog.this.isFocusable();
        }

        public void requestFocus() {
            unoAccessibleComponent.grabFocus();
        }
    }
}

