/*************************************************************************
 *
 *  $RCSfile: AccessibleObjectFactory.java,v $
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

import org.openoffice.accessibility.internal.*;

import java.lang.ref.WeakReference;
import javax.accessibility.Accessible;
import javax.accessibility.AccessibleStateSet;

import com.sun.star.uno.*;
import drafts.com.sun.star.accessibility.*;

/**
*/
public class AccessibleObjectFactory {
    // This type is needed for conversions from/to uno Any
    public static final Type XAccessibleType = new Type(XAccessible.class);

    private static java.util.Hashtable objectList = new java.util.Hashtable();
    private static java.awt.FocusTraversalPolicy focusTraversalPolicy = new FocusTraversalPolicy();

    private static java.awt.EventQueue eventQueue = java.awt.Toolkit.getDefaultToolkit().getSystemEventQueue();

    public static java.awt.EventQueue getEventQueue() {
        return eventQueue;
    }

    public static void postFocusGained(java.awt.Component c) {
        getEventQueue().postEvent(new java.awt.event.FocusEvent(c, java.awt.event.FocusEvent.FOCUS_GAINED));
    }

    public static void postWindowGainedFocus(java.awt.Window w) {
        getEventQueue().postEvent(new java.awt.event.WindowEvent(w, java.awt.event.WindowEvent.WINDOW_GAINED_FOCUS));
    }

    public static void postWindowLostFocus(java.awt.Window w) {
        getEventQueue().postEvent(new java.awt.event.WindowEvent(w, java.awt.event.WindowEvent.WINDOW_LOST_FOCUS));
    }

    public static void postWindowActivated(java.awt.Window w) {
        getEventQueue().postEvent(new java.awt.event.WindowEvent(w, java.awt.event.WindowEvent.WINDOW_ACTIVATED));
    }

    public static void postWindowDeactivated(java.awt.Window w) {
        getEventQueue().postEvent(new java.awt.event.WindowEvent(w, java.awt.event.WindowEvent.WINDOW_DEACTIVATED));
    }

    protected static java.awt.Component getAccessibleComponent(XAccessible xAccessible) {
        java.awt.Component c = null;
        if (xAccessible != null) {
            // Retrieve unique id for the original UNO object to be used as a hash key
            String oid = UnoRuntime.generateOid(xAccessible);

            // Check if we already have a wrapper object for this context
            synchronized (objectList) {
                WeakReference r = (WeakReference) objectList.get(oid);
                if(r != null) {
                    c = (java.awt.Component) r.get();
                }
            }
        }
        return c;
    }

    protected static void addChild(java.awt.Container parent, Object any) {
        try {
            addChild(parent, (XAccessible) AnyConverter.toObject(XAccessibleType, any));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.err.println(e.getClass().getName() + " caught: " + e.getMessage());
        }
    }

    protected static void addChild(java.awt.Container parent, XAccessible child) {
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
            case AccessibleRole.CHECKBOX:
                c = new CheckBox(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.COMBOBOX:
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
            case AccessibleRole.ENDNOTE:
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
            case AccessibleRole.LAYEREDPANE:
                c = new Container(javax.accessibility.AccessibleRole.LAYERED_PANE,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.LIST:
                c = new List(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.MENU:
                c = new Menu(xAccessible, xAccessibleContext);
                // !!! FIXME !!!
                // Menu items are always visible, but change SHOWING state
//              if (!xAccessibleStateSet.contains(AccessibleStateType.SHOWING)) {
//                  c.setVisible(false);
//              }
                break;
            case AccessibleRole.MENUBAR:
                c = new Container(javax.accessibility.AccessibleRole.MENU_BAR, xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.MENUITEM:
                c = new MenuItem(xAccessible, xAccessibleContext);
                c.setFocusable(false);
                // Menu items are always visible, but change SHOWING state
//              if (!xAccessibleStateSet.contains(AccessibleStateType.SHOWING)) {
//                  c.setVisible(false);
//              }
                break;
            case AccessibleRole.POPUPMENU:
                c = new Container(javax.accessibility.AccessibleRole.POPUP_MENU, xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.OPTIONPANE:
                c = new Container(javax.accessibility.AccessibleRole.OPTION_PANE,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.PAGETAB:
                c = new Container(javax.accessibility.AccessibleRole.PAGE_TAB, xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.PAGETABLIST:
                c = new Container(javax.accessibility.AccessibleRole.PAGE_TAB_LIST, xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.PARAGRAPH:
            case AccessibleRole.HEADING:
                c = new Paragraph(xAccessible, xAccessibleContext, xAccessibleStateSet);
                break;
            case AccessibleRole.PANEL:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.PUSHBUTTON:
                c = new Button(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.RADIOBUTTON:
                c = new RadioButton(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.ROOTPANE:
                c = new Container(javax.accessibility.AccessibleRole.ROOT_PANE,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.SCROLLBAR:
                if (xAccessibleStateSet.contains(AccessibleStateType.VERTICAL)) {
                    c = new ScrollBar(xAccessible, xAccessibleContext,
                        javax.swing.SwingConstants.VERTICAL);
                } else {
                    c = new ScrollBar(xAccessible, xAccessibleContext);
                }
                break;
            case AccessibleRole.SCROLLPANE:
                c = new Container(javax.accessibility.AccessibleRole.SCROLL_PANE,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.SEPARATOR:
                if (xAccessibleStateSet.contains(AccessibleStateType.VERTICAL)) {
                    c = new Separator(xAccessible, xAccessibleContext,
                        javax.swing.SwingConstants.VERTICAL);
                } else {
                    c = new Separator(xAccessible, xAccessibleContext);
                }
                break;
            case AccessibleRole.SHAPE:
                c = new Container(javax.accessibility.AccessibleRole.CANVAS,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.SPLITPANE:
                c = new Container(javax.accessibility.AccessibleRole.SPLIT_PANE,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.STATUSBAR:
                c = new Container(javax.accessibility.AccessibleRole.STATUS_BAR,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.COLUMN_HEADER:
            case AccessibleRole.TABLE:
                if (xAccessibleStateSet.contains(AccessibleStateType.MANAGES_DESCENDANT)) {
                    c = new Table(xAccessible, xAccessibleContext,
                        xAccessibleStateSet.contains(AccessibleStateType.MULTISELECTABLE));
                } else {
                    c = new Container(javax.accessibility.AccessibleRole.TABLE,
                        xAccessible, xAccessibleContext);
                }
                break;
            case AccessibleRole.TABLE_CELL:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TEXT:
                c = new TextComponent(xAccessible, xAccessibleContext, xAccessibleStateSet);
                break;
            case AccessibleRole.TEXT_FRAME:
                c = new Container(javax.accessibility.AccessibleRole.PANEL,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TOGGLEBUTTON:
                c = new ToggleButton(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TOOLBAR:
                c = new Container(javax.accessibility.AccessibleRole.TOOL_BAR,
                    xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TOOLTIP:
                c = new ToolTip(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.TREE:
                c = new Tree(xAccessible, xAccessibleContext);
                break;
            case AccessibleRole.VIEWPORT:
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
                objectList.put(c.toString(), new WeakReference(c));
                if (Build.DEBUG) {
//                  System.out.println("Object cache now contains " + objectList.size() + " objects.");
                }
            }
            // Set the boundings of the component if it is visible ..
            if (!xAccessibleStateSet.contains(AccessibleStateType.VISIBLE)) {
                c.setVisible(false);
            }
            // Set the components' enabled state ..
            if (!xAccessibleStateSet.contains(AccessibleStateType.ENABLED)) {
                c.setEnabled(false);
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
            XAccessibleComponent xAccessibleComponent = (XAccessibleComponent)
                UnoRuntime.queryInterface(XAccessibleComponent.class, xAccessibleContext);

            java.awt.Window w;
            if (role == AccessibleRole.DIALOG) {
                w = new Dialog(new Application(),
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


