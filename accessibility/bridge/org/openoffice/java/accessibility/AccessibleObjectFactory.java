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

import java.awt.Component;
import java.awt.EventQueue;
import java.lang.ref.WeakReference;
import com.sun.star.uno.*;
import com.sun.star.accessibility.*;
import org.openoffice.java.accessibility.logging.XAccessibleEventLog;

/**
*/
public class AccessibleObjectFactory {
    // This type is needed for conversions from/to uno Any
    public static final Type XAccessibleType = new Type(XAccessible.class);

    private static java.util.Hashtable<String, WeakReference<Component>> objectList = new java.util.Hashtable<String, WeakReference<Component>>();
    private static java.awt.FocusTraversalPolicy focusTraversalPolicy = new FocusTraversalPolicy();

    private static java.awt.EventQueue theEventQueue = java.awt.Toolkit.getDefaultToolkit().
                                                                        getSystemEventQueue();

    public static java.awt.EventQueue getEventQueue() {
        return theEventQueue;
    }

    public static void postFocusGained(java.awt.Component c) {
        getEventQueue().postEvent(new java.awt.event.FocusEvent(c, java.awt.event.FocusEvent.FOCUS_GAINED));
    }

    public static void postWindowGainedFocus(java.awt.Window w) {
        postWindowEvent(w, java.awt.event.WindowEvent.WINDOW_GAINED_FOCUS);
    }

    public static void postWindowLostFocus(java.awt.Window w) {
        postWindowEvent(w, java.awt.event.WindowEvent.WINDOW_LOST_FOCUS);
    }

    public static void postWindowActivated(java.awt.Window w) {
        postWindowEvent(w, java.awt.event.WindowEvent.WINDOW_ACTIVATED);
    }

    public static void postWindowDeactivated(java.awt.Window w) {
        postWindowEvent(w, java.awt.event.WindowEvent.WINDOW_DEACTIVATED);
    }

    public static void postWindowOpened(java.awt.Window w) {
        postWindowEvent(w, java.awt.event.WindowEvent.WINDOW_OPENED);
    }

    public static void postWindowClosed(java.awt.Window w) {
        postWindowEvent(w, java.awt.event.WindowEvent.WINDOW_CLOSED);
    }

    public static void invokeAndWait() {
        try {
            EventQueue.invokeAndWait( new java.lang.Runnable () {
                                                public void run() {
                                                }
                                             });
        } catch (java.lang.reflect.InvocationTargetException e) {
        } catch (java.lang.InterruptedException e) {
        }
    }

    private static void postWindowEvent(java.awt.Window w, int i) {
        theEventQueue.postEvent(new java.awt.event.WindowEvent(w, i));
    }

    public static java.awt.Component getAccessibleComponent(XAccessible xAccessible) {
        java.awt.Component c = null;
        if (xAccessible != null) {
            // Retrieve unique id for the original UNO object to be used as a hash key
            String oid = UnoRuntime.generateOid(xAccessible);

            // Check if we already have a wrapper object for this context
            synchronized (objectList) {
                WeakReference<java.awt.Component> r = objectList.get(oid);
                if(r != null) {
                    c = r.get();
                }
            }
        }
        return c;
    }

    public static void addChild(java.awt.Container parent, Object any) {
        try {
            addChild(parent, (XAccessible) AnyConverter.toObject(XAccessibleType, any));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.err.println(e.getClass().getName() + " caught: " + e.getMessage());
        }
    }

    public static void addChild(java.awt.Container parent, XAccessible child) {
        try {
            if (child != null) {
                XAccessibleContext childAC = child.getAccessibleContext();
                if (childAC != null) {
                    XAccessibleStateSet stateSet = childAC.getAccessibleStateSet();
                    if (stateSet != null) {
                        java.awt.Component c = getAccessibleComponent(child);

                        // Re-use existing wrapper if possible, create a new one otherwise
                        if (c != null) {
                            // Seems to be already in child list
                            if (parent.equals(c.getParent()))
                                return;
                            // Update general component states
                            c.setEnabled(stateSet.contains(AccessibleStateType.ENABLED));
                            c.setVisible(stateSet.contains(AccessibleStateType.VISIBLE));
                        } else {
                            c = createAccessibleComponentImpl(child, childAC, stateSet);
                        }

                        if (c != null) {
                            if (c instanceof java.awt.Container) {
                                populateContainer((java.awt.Container) c, childAC);
                            }
                            parent.add(c);
                            // Simulate focus gained event for new child
                            if (stateSet.contains(AccessibleStateType.FOCUSED)) {
                                postFocusGained(c);
                            }
                        }
                    }
                }
            }
        } catch (com.sun.star.uno.RuntimeException e) {
                System.err.println(e.getClass().getName() + " caught: " + e.getMessage());
                e.printStackTrace();
        }
    }

    protected static void removeChild(java.awt.Container parent, Object any) {
        try {
            XAccessible xAccessible = (XAccessible) AnyConverter.toObject(XAccessibleType, any);
            java.awt.Component c = getAccessibleComponent(xAccessible);

            if (c != null) {
                    parent.remove(c);

                    if (c instanceof java.awt.Container) {
                        clearContainer((java.awt.Container) c);
                    }
                }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.err.println(e.getClass().getName() + " caught: " + e.getMessage());
        }
    }


    /**
    * Removes all children from the container parent
    */

    protected static void clearContainer(java.awt.Container parent) {
        // Purge all children from this container
        int count = parent.getComponentCount();
        for (int i = 0; i < count; i++) {
            java.awt.Component c = parent.getComponent(i);
            if (c instanceof java.awt.Container) {
                clearContainer((java.awt.Container) c);
            }
        }
        parent.removeAll();
    }


    /**
    * Populates the given Container parent with wrapper objects for all children of parentAC. This method is
    * intended to be called when a container is added using a CHILDREN_CHANGED event.
    */

    protected static void populateContainer(java.awt.Container parent, XAccessibleContext parentAC) {
        if (parentAC != null) {
            try {
                int childCount = parentAC.getAccessibleChildCount();
                for (int i=0; i<childCount; i++) {
                    addChild(parent, parentAC.getAccessibleChild(i));
                }
            } catch (java.lang.Exception e) {
                System.err.println(e.getClass().getName() + " caught: " + e.getMessage());
                e.printStackTrace();
            }
        }
    }

    /**
    * Populates the given Container parent with wrapper objects for all children of parentAC. This method is
    * intended to be called when a new window has been opened.
    */
    protected static void populateContainer(java.awt.Container parent, XAccessibleContext parentAC, java.awt.Window frame) {
        if (parentAC != null) {
            try {
                int childCount = parentAC.getAccessibleChildCount();
                for (int i=0; i<childCount; i++) {
                    XAccessible child = parentAC.getAccessibleChild(i);
                    if (child != null) {
                        XAccessibleContext childAC = child.getAccessibleContext();
                        java.awt.Component c = createAccessibleComponent(child, childAC, frame);
                        if (c != null) {
                            if (c instanceof java.awt.Container) {
                                populateContainer((java.awt.Container) c, childAC, frame);
                            }
                            parent.add(c);
                        }
                    } else if (Build.DEBUG) {
                        System.err.println("ignoring not accessible child " + i);
                    }
                }
            }

            catch (java.lang.Exception e) {
                System.err.println(e.getClass().getName() + " caught: " + e.getMessage());
                e.printStackTrace();
            }
        }
    }

    protected static java.awt.Component createAccessibleComponent(XAccessible xAccessible) {
        try {
            XAccessibleContext xAccessibleContext = xAccessible.getAccessibleContext();
            if (xAccessibleContext != null) {
                return createAccessibleComponentImpl(xAccessible, xAccessibleContext, xAccessibleContext.getAccessibleStateSet());
            }
        } catch (com.sun.star.uno.RuntimeException e) {
                System.err.println(e.getClass().getName() + " caught: " + e.getMessage());
                e.printStackTrace();
        }
        return null;
    }

    protected static java.awt.Component createAccessibleComponent(XAccessible xAccessible, XAccessibleContext xAccessibleContext,
        java.awt.Window frame) {
        if (xAccessibleContext != null) {
            try {
                XAccessibleStateSet xAccessibleStateSet = xAccessibleContext.getAccessibleStateSet();
                java.awt.Component c = createAccessibleComponentImpl(xAccessible, xAccessibleContext, xAccessibleStateSet);
                if (c != null) {
                    // Set this component as initial component
                    if (xAccessibleStateSet.contains(AccessibleStateType.FOCUSED)) {
                        if (frame instanceof NativeFrame) {
                            ((NativeFrame) frame).setInitialComponent(c);
                        }
                    }
                    return c;
                }
            } catch (com.sun.star.uno.RuntimeException e) {
                System.err.println(e.getClass().getName() + " caught: " + e.getMessage());
                e.printStackTrace();
            }
        }
        return null;
    }

    protected static java.awt.Component createAccessibleComponentImpl(XAccessible xAccessible, XAccessibleContext xAccessibleContext,
        XAccessibleStateSet xAccessibleStateSet) {
        java.awt.Component c = null;
        short role = xAccessibleContext.getAccessibleRole();
        switch (role) {
            case AccessibleRole.CANVAS:
                c = new Container(javax.accessibility.AccessibleRole.CANVAS,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.CHECK_BOX:
                c = new CheckBox(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.COMBO_BOX:
                c = new ComboBox(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.DOCUMENT:
                c = new Container(javax.accessibility.AccessibleRole.CANVAS,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.EMBEDDED_OBJECT:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.END_NOTE:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.FILLER:
                c = new Container(javax.accessibility.AccessibleRole.FILLER,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.FOOTNOTE:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.FOOTER:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.GRAPHIC:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.HEADER:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.ICON:
                c = new Icon(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.LABEL:
                c = new Label(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.LAYERED_PANE:
                c = new Container(javax.accessibility.AccessibleRole.LAYERED_PANE,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.LIST:
                if (xAccessibleStateSet.contains(AccessibleStateType.MANAGES_DESCENDANTS)) {
                    c = new List(xAccessible, xAccessibleContext);
                } else {
                    c = new Container(javax.accessibility.AccessibleRole.LIST,
                        xAccessible, xAccessibleContext);
                }
                break;
            case AccessibleRole.MENU:
                c = new Menu(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.MENU_BAR:
                c = new MenuContainer(javax.accessibility.AccessibleRole.MENU_BAR, xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.MENU_ITEM:
                c = new MenuItem(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.POPUP_MENU:
                c = new MenuContainer(javax.accessibility.AccessibleRole.POPUP_MENU, xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.OPTION_PANE:
                c = new Container(javax.accessibility.AccessibleRole.OPTION_PANE,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.PAGE_TAB:
                c = new Container(javax.accessibility.AccessibleRole.PAGE_TAB, xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.PAGE_TAB_LIST:
                c = new Container(javax.accessibility.AccessibleRole.PAGE_TAB_LIST, xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.PARAGRAPH:
            case AccessibleRole.HEADING:
                c = new Paragraph(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.PANEL:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.PUSH_BUTTON:
                c = new Button(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.RADIO_BUTTON:
                c = new RadioButton(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.ROOT_PANE:
                c = new Container(javax.accessibility.AccessibleRole.ROOT_PANE,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.SCROLL_BAR:
                c = new ScrollBar(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.SCROLL_PANE:
                c = new Container(javax.accessibility.AccessibleRole.SCROLL_PANE,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.SEPARATOR:
                c = new Separator(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.SHAPE:
                c = new Container(javax.accessibility.AccessibleRole.CANVAS,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.SPLIT_PANE:
                c = new Container(javax.accessibility.AccessibleRole.SPLIT_PANE,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.STATUS_BAR:
                c = new Container(javax.accessibility.AccessibleRole.STATUS_BAR,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.COLUMN_HEADER:
            case AccessibleRole.TABLE:
                if (xAccessibleStateSet.contains(AccessibleStateType.MANAGES_DESCENDANTS)) {
                    c = new Table(xAccessible, xAccessibleContext,
                        xAccessibleStateSet.contains(AccessibleStateType.MULTI_SELECTABLE));
                } else {
                    c = new Container(javax.accessibility.AccessibleRole.TABLE,
                        xAccessible, xAccessibleContext);
                }
                break;
            case AccessibleRole.TABLE_CELL:
                if( xAccessibleContext.getAccessibleChildCount() > 0 )
                    c = new Container(javax.accessibility.AccessibleRole.PANEL,
                        xAccessible, xAccessibleContext);
                else
                    c = new Label(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TEXT:
                c = new TextComponent(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TEXT_FRAME:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TOGGLE_BUTTON:
                c = new ToggleButton(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TOOL_BAR:
                c = new Container(javax.accessibility.AccessibleRole.TOOL_BAR,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TOOL_TIP:
                c = new ToolTip(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TREE:
                c = new Tree(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.VIEW_PORT:
                c = new Container(javax.accessibility.AccessibleRole.VIEWPORT,
                    xAccessible, xAccessibleContext);
                break;
            default:
                System.err.println("Unmapped accessible object " + role);
                System.err.println("usually mapped to " + AccessibleRoleAdapter.getAccessibleRole(role));
                c = new Container(AccessibleRoleAdapter.getAccessibleRole(role),
                    xAccessible, xAccessibleContext);
                break;
        }
        if (c != null) {
            // Add the newly created object to the cache list
            synchronized (objectList) {
                objectList.put(c.toString(), new WeakReference<Component>(c));
                if (Build.DEBUG) {
//                  System.out.println("Object cache now contains " + objectList.size() + " objects.");
                }
            }

            AccessibleStateAdapter.setComponentState(c, xAccessibleStateSet);

            if (! Build.PRODUCT) {
                String property = System.getProperty("AccessBridgeLogging");
                if ((property != null) && (property.indexOf("event") != -1)) {
                    XAccessibleEventLog.addEventListener(xAccessibleContext, c);
                }
            }
        }

        return c;
    }

    protected static void disposing(java.awt.Component c) {
        if (c != null) {
            synchronized (objectList) {
                objectList.remove(c.toString());
            }
        }
    }

    public static java.awt.Window getTopWindow(XAccessible xAccessible) {
        XAccessibleContext xAccessibleContext = xAccessible.getAccessibleContext();

        if (xAccessibleContext != null) {
            short role = xAccessibleContext.getAccessibleRole();
            XAccessibleStateSet xAccessibleStateSet = xAccessibleContext.getAccessibleStateSet();
            XAccessibleComponent xAccessibleComponent = UnoRuntime.queryInterface(XAccessibleComponent.class, xAccessibleContext);

            java.awt.Window w;
            if (role == AccessibleRole.DIALOG) {
                w = new Dialog(new Application(),
                    xAccessibleContext.getAccessibleName(),
                    xAccessibleStateSet.contains(AccessibleStateType.MODAL),
                    xAccessibleComponent);
            } else if (role == AccessibleRole.ALERT) {
                w = new Alert(new Application(),
                    xAccessibleContext.getAccessibleName(),
                    xAccessibleStateSet.contains(AccessibleStateType.MODAL),
                    xAccessibleComponent);
            } else if (role == AccessibleRole.FRAME) {
                w = new Frame(xAccessibleContext.getAccessibleName(),
                    xAccessibleComponent);
            } else if (role == AccessibleRole.WINDOW) {
                java.awt.Window activeWindow =
                    java.awt.KeyboardFocusManager.getCurrentKeyboardFocusManager().getActiveWindow();
                if (activeWindow != null) {
                    w = new Window(activeWindow, xAccessibleComponent);
                } else {
                    if (Build.DEBUG) {
                        System.err.println("no active frame found for Window: " + role);
                    }
                    return null;
                }
            } else {
                if (Build.DEBUG) {
                    System.err.println("invalid role for toplevel window: " + role);
                }
                return null;
            }
            populateContainer(w, xAccessibleContext, w);
            w.setFocusTraversalPolicy(focusTraversalPolicy);
            w.setVisible(true);

            // Make the new window the focused one if it has an initialy focused object set.
            java.awt.Component c = ((NativeFrame) w).getInitialComponent();
            if (c != null) {
                postWindowGainedFocus(w);
            }
            return w;
        }

        return null;
    }
}


