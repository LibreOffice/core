/*************************************************************************
 *
 *  $RCSfile: Window.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2002-12-06 11:25:41 $
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

import com.sun.star.uno.*;
import drafts.com.sun.star.accessibility.*;

public class Window extends java.awt.Window implements javax.accessibility.Accessible, NativeFrame {
    protected XAccessibleComponent unoAccessibleComponent;

    // The coordinates of this frame on screen
    java.awt.Point location = null;

    boolean opened = false;
    boolean visible = false;

    java.awt.EventQueue eventQueue = null;

    public Window(java.awt.Frame owner, XAccessibleComponent xAccessibleComponent) {
        super(owner);
        initialize(xAccessibleComponent);
    }

    private void initialize(XAccessibleComponent xAccessibleComponent) {
        unoAccessibleComponent = xAccessibleComponent;
        eventQueue = java.awt.Toolkit.getDefaultToolkit().getSystemEventQueue();
        XAccessibleEventBroadcaster broadcaster = (XAccessibleEventBroadcaster)
            UnoRuntime.queryInterface(XAccessibleEventBroadcaster.class,
            unoAccessibleComponent);
        if (broadcaster != null) {
            broadcaster.addEventListener(new AccessibleWindowListener());
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
                com.sun.star.awt.Point p = unoAccessibleComponent.getLocationOnScreen();
                location = new java.awt.Point(p.X, p.Y);

                // If it is the first show, fire WINDOW_OPENED event
                if (!opened) {
                    postWindowEvent(java.awt.event.WindowEvent.WINDOW_OPENED);
                    opened = true;
                }
            } else {
                location = null;
            }
        }
    }

    protected void postWindowEvent(int i) {
        eventQueue.postEvent(new java.awt.event.WindowEvent(this, i));
    }

    /** Returns the location of the object on the screen. */
    public java.awt.Point getLocationOnScreen() {
        if (visible) {
            return new java.awt.Point(location);
        }
        return null;
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
                case AccessibleStateType.ACTIVE:
                    postWindowEvent(enable ?
                        java.awt.event.WindowEvent.WINDOW_GAINED_FOCUS :
                        java.awt.event.WindowEvent.WINDOW_LOST_FOCUS);
                    break;
                case AccessibleStateType.ICONIFIED:
                    Window.this.postWindowEvent(enable ?
                        java.awt.event.WindowEvent.WINDOW_ICONIFIED :
                        java.awt.event.WindowEvent.WINDOW_DEICONIFIED);
                    break;
                case AccessibleStateType.SHOWING:
                case AccessibleStateType.VISIBLE:
                    Window.this.setVisible(enable);
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
//              Window.this.setTitle(AnyConverter.toString(any));
                AnyConverter.toString(any);
            }
            catch (com.sun.star.lang.IllegalArgumentException e) {
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
            }
            catch (com.sun.star.lang.IllegalArgumentException e) {
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
            }

            catch (com.sun.star.lang.IllegalArgumentException e) {
            }
        }

        /** Updates the internal child list and fires the appropriate PropertyChangeEvent */
        protected void handleChildRemovedEvent(Object any) {
            try {
                java.awt.Component c = AccessibleObjectFactory.getDefault().getAccessibleComponent(
                    (XAccessible) AnyConverter.toObject(Container.XAccessibleType, any));
                if (c != null) {
                    Window.this.remove(c);
                }
            }

            catch (com.sun.star.uno.Exception e) {
                // FIXME: output
            }
        }

        /** Updates the internal child list and fires the appropriate PropertyChangeEvent */
        protected void handleChildAddedEvent(Object any) {
            try {
                XAccessible xAccessible = (XAccessible) AnyConverter.toObject(AbstractContainer.XAccessibleType, any);
                AccessibleObjectFactory factory = AccessibleObjectFactory.getDefault();
                java.awt.Component c = factory.getAccessibleComponent(xAccessible);
                if (c != null) {
                    // Seems to be already in child list
                    if (c.getParent().equals(this))
                        return;
                } else {
                    c = factory.createAccessibleComponent(xAccessible);
                }
                if (c != null) {
                    Window.this.add(c, xAccessible.getAccessibleContext().
                        getAccessibleIndexInParent());
                }
            }

            catch (com.sun.star.uno.Exception e) {
                // FIXME: output
            }
        }

        /** Fires a visible data property change event */
        protected void handleVisibleDataEvent() {
            try {
                com.sun.star.awt.Rectangle r = unoAccessibleComponent.getBounds();
                Window.this.setBounds(r.X, r.Y, r.Width, r.Height);
                com.sun.star.awt.Point p = unoAccessibleComponent.getLocationOnScreen();
                location = new java.awt.Point(p.X, p.Y);
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }

        /** Updates internal bounding box cache */
        protected void handleBoundRectEvent() {
            try {
                com.sun.star.awt.Rectangle r = unoAccessibleComponent.getBounds();
                Window.this.setBounds(r.X, r.Y, r.Width, r.Height);
                com.sun.star.awt.Point p = unoAccessibleComponent.getLocationOnScreen();
                location = new java.awt.Point(p.X, p.Y);
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
                case AccessibleEventId.ACCESSIBLE_CHILD_EVENT:
                    if (AnyConverter.isObject(event.OldValue)) {
                        handleChildRemovedEvent(event.OldValue);
                    } else if (AnyConverter.isObject(event.NewValue)) {
                        handleChildAddedEvent(event.NewValue);
                    }
                    break;
                case AccessibleEventId.ACCESSIBLE_VISIBLE_DATA_EVENT:
                    handleVisibleDataEvent();
                    break;
                case AccessibleEventId.ACCESSIBLE_BOUNDRECT_EVENT:
                    handleBoundRectEvent();
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

        protected java.awt.event.WindowFocusListener accessibleWindowFocusHandler = null;

        /**
        * Fire PropertyChange listener, if one is registered,
        * when focus events happen
        */
        protected class AccessibleWindowFocusHandler implements java.awt.event.WindowFocusListener {
            public void windowGainedFocus(java.awt.event.WindowEvent e) {
                AccessibleWindow.this.firePropertyChange(
                    javax.accessibility.AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    null, javax.accessibility.AccessibleState.ACTIVE);
            }
            public void windowLostFocus(java.awt.event.WindowEvent e) {
                AccessibleWindow.this.firePropertyChange(
                    javax.accessibility.AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                    javax.accessibility.AccessibleState.ACTIVE, null);
            }
        } // inner class AccessibleFocusHandler

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
                accessibleWindowFocusHandler = new AccessibleWindowFocusHandler();
                Window.this.addWindowFocusListener(accessibleWindowFocusHandler);

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

                Window.this.removeWindowFocusListener(accessibleWindowFocusHandler);
                accessibleWindowFocusHandler = null;
            }
            super.removePropertyChangeListener(listener);
        }
    }
}

