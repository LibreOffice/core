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

public class Window extends java.awt.Window implements javax.accessibility.Accessible, NativeFrame {
    protected XAccessibleComponent unoAccessibleComponent;

    boolean opened = false;
    boolean visible = false;

    java.awt.EventQueue eventQueue = null;

    public Window(java.awt.Window owner, XAccessibleComponent xAccessibleComponent) {
        super(owner);
        initialize(xAccessibleComponent);
    }

    private void initialize(XAccessibleComponent xAccessibleComponent) {
        unoAccessibleComponent = xAccessibleComponent;
        eventQueue = java.awt.Toolkit.getDefaultToolkit().getSystemEventQueue();
        XAccessibleEventBroadcaster broadcaster = UnoRuntime.queryInterface(XAccessibleEventBroadcaster.class,
        unoAccessibleComponent);
        if (broadcaster != null) {
            broadcaster.addEventListener(new AccessibleWindowListener());
        }
    }

    java.awt.Component initialComponent = null;

    public java.awt.Component getInitialComponent() {
        if (Build.DEBUG) {
            System.err.println("returning initial component object of class: " + initialComponent.getClass().getName());
        }
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

        // Transfer window focus back to the owner window if it is still the active frame
        if ((getOwner() instanceof Frame && ((Frame) getOwner()).active) ||
                (getOwner() instanceof Dialog && ((Dialog) getOwner()).active)) {
            eventQueue.postEvent(new java.awt.event.WindowEvent(getOwner(),
                java.awt.event.WindowEvent.WINDOW_GAINED_FOCUS));
        }
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
    protected class AccessibleWindowListener implements XAccessibleEventListener {

        protected AccessibleWindowListener() {
        }

        // The only expected state changes are ACTIVE and VISIBLE
        protected void setComponentState(short state, boolean enable) {
            switch (state) {
                case AccessibleStateType.ICONIFIED:
                    postWindowEvent(enable ?
                        java.awt.event.WindowEvent.WINDOW_ICONIFIED :
                        java.awt.event.WindowEvent.WINDOW_DEICONIFIED);
                    break;
                case AccessibleStateType.SHOWING:
                case AccessibleStateType.VISIBLE:
                    setVisible(enable);
                    break;
                default:
                    if (Build.DEBUG) {
//                      System.err.println("[frame]: " + getTitle() + "unexpected state change " + state);
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
                javax.accessibility.AccessibleContext ac = accessibleContext;
                if (ac!= null) {
                    ac.setAccessibleName(AnyConverter.toString(any));
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
                        AccessibleObjectFactory.removeChild(Window.this, event.OldValue);
                    } else if (AnyConverter.isObject(event.NewValue)) {
                        AccessibleObjectFactory.addChild(Window.this, event.NewValue);
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

    protected javax.accessibility.AccessibleContext accessibleContext = null;

    /** Returns the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessibleWindow();
//          accessibleContext.setAccessibleName(getTitle());
        }
        return accessibleContext;
    }

    protected class AccessibleWindow extends java.awt.Window.AccessibleAWTWindow {
        protected AccessibleWindow() {
            super();
        }

        protected java.awt.event.ComponentListener accessibleComponentHandler = null;

        /**
        * Fire PropertyChange listener, if one is registered,
        * when shown/hidden..
        */
        protected class AccessibleComponentHandler implements java.awt.event.ComponentListener {
            public void componentHidden(java.awt.event.ComponentEvent e)  {
                AccessibleWindow.this.firePropertyChange(
                    javax.accessibility.AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    javax.accessibility.AccessibleState.VISIBLE, null);
            }

            public void componentShown(java.awt.event.ComponentEvent e)  {
                AccessibleWindow.this.firePropertyChange(
                    javax.accessibility.AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    null, javax.accessibility.AccessibleState.VISIBLE);
            }

            public void componentMoved(java.awt.event.ComponentEvent e)  {
            }

            public void componentResized(java.awt.event.ComponentEvent e)  {
            }
        } // inner class AccessibleComponentHandler

        protected java.awt.event.ContainerListener accessibleContainerHandler = null;

        /**
        * Fire PropertyChange listener, if one is registered,
        * when children added/removed.
        */

        protected class AccessibleContainerHandler implements java.awt.event.ContainerListener {
            public void componentAdded(java.awt.event.ContainerEvent e) {
                java.awt.Component c = e.getChild();
                if (c != null && c instanceof javax.accessibility.Accessible) {
                    AccessibleWindow.this.firePropertyChange(
                        javax.accessibility.AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                        null, ((javax.accessibility.Accessible) c).getAccessibleContext());
                }
            }
            public void componentRemoved(java.awt.event.ContainerEvent e) {
                java.awt.Component c = e.getChild();
                if (c != null && c instanceof javax.accessibility.Accessible) {
                    AccessibleWindow.this.firePropertyChange(
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
                accessibleContainerHandler = new AccessibleContainerHandler();
                Window.this.addContainerListener(accessibleContainerHandler);

                accessibleComponentHandler = new AccessibleComponentHandler();
                Window.this.addComponentListener(accessibleComponentHandler);
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
                Window.this.removeComponentListener(accessibleComponentHandler);
                accessibleComponentHandler = null;

                Window.this.removeContainerListener(accessibleContainerHandler);
                accessibleContainerHandler = null;
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
            return Window.this.isEnabled();
        }

        public void setEnabled(boolean b) {
            // Not supported by UNO accessibility API
        }

        public boolean isVisible() {
            return Window.this.isVisible();
        }

        public void setVisible(boolean b) {
            // Not supported by UNO accessibility API
        }

        public boolean isShowing() {
            return Window.this.isShowing();
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
                com.sun.star.awt.Point unoPoint = unoAccessibleComponent.getLocationOnScreen();
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
            return Window.this.isFocusable();
        }

        public void requestFocus() {
            unoAccessibleComponent.grabFocus();
        }
    }
}

