/*************************************************************************
 *
 *  $RCSfile: AccessibleObjectFactory.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obr $ $Date: 2002-08-16 13:13:08 $
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
import drafts.com.sun.star.accessibility.AccessibleRole;
import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;

/**
*/
public class AccessibleObjectFactory {

    // This type is needed for conversions from/to uno Any
    public static final Type XAccessibleType = new Type(XAccessible.class);

    java.util.Hashtable objectList = new java.util.Hashtable();
    XAccessibilityInformationProvider infoProvider;

    private static AccessibleObjectFactory defaultFactory = new AccessibleObjectFactory();

    protected AccessibleObjectFactory() {
        // FIXME: provide default implementation of XAccessibilityInformationProvider
//      infoProvider = provider;
    }

    public class GenericAccessibleParent extends Object implements javax.accessibility.Accessible {
        XAccessibleContext unoObject;
        AccessibleObject wrapperObject;

        public GenericAccessibleParent(AccessibleObject o, XAccessibleContext ac) {
            unoObject = ac;
            wrapperObject = o;
        }

        public javax.accessibility.AccessibleContext getAccessibleContext() {
            XAccessible xAccessible = unoObject.getAccessibleParent();
            if( xAccessible != null ) {
                AccessibleObject o = AccessibleObjectFactory.this.getAccessibleObject(xAccessible, true, null);
                wrapperObject.setAccessibleParent(o);
                return o;
            }

            return null;
        }
    }

    /** Returns the default accessible object factory */
    public static AccessibleObjectFactory getDefault() {
        return defaultFactory;
    }

    /** Sets a new AccessibleInformationProvider to be used by this factory object */
    public void setInformationProvider(XAccessibilityInformationProvider provider) {
        infoProvider = provider;
    }

    /** Returns the AccessibleInformationProvider currently used by this factory object */
    public XAccessibilityInformationProvider getInformationProvider() {
        return infoProvider;
    }

    public AccessibleObject getAccessibleObject(XAccessible xAccessible, boolean create, Accessible parent) {
        XAccessibleContext xAccessibleContext = null;

        if(xAccessible != null) {
            // Save the round trip to C++ UNO if possible
            if(xAccessible instanceof XAccessibleContext) {
                xAccessibleContext = (XAccessibleContext) xAccessible;
            } else {
                xAccessibleContext = xAccessible.getAccessibleContext();
            }
        }

        // Ensure that we really got an UNO accessible context
        if(xAccessibleContext == null) {
            if( Build.DEBUG ) {
                System.err.println("No accessible context");
            }
            return null;
        }

        // Retrieve unique id for the original UNO object to be used as a hash key
        String oid = UnoRuntime.generateOid(xAccessibleContext);
        AccessibleObject o = null;

        // Check if we already have a wrapper object for this context
        synchronized (objectList) {
            WeakReference r = (WeakReference) objectList.get(oid);
            if(r != null) {
                o = (AccessibleObject) r.get();
            }
        }

        if( o == null && create ) {
            AccessibleContextInfo info = infoProvider.getAccessibleContextInfo(xAccessibleContext);

            switch(info.Role) {
                case AccessibleRole.CHECKBOX:
                    o = new AccessibleButton(
                        javax.accessibility.AccessibleRole.CHECK_BOX,
                        javax.accessibility.AccessibleState.CHECKED,
                        xAccessibleContext
                        );
                    break;
                case AccessibleRole.COMBOBOX:
                case AccessibleRole.LIST:
                case AccessibleRole.MENUBAR:
                case AccessibleRole.POPUPMENU:
                    o = new AccessibleContainer(
                        AccessibleRoleMap.toAccessibleRole(info.Role),
                        xAccessibleContext
                    );
                    break;
                case AccessibleRole.LABEL:
                case AccessibleRole.TABLE_CELL:
                    if((info.States & AccessibleState.TRANSIENT) == 0) {
                        o = new AccessibleFixedText(info.IndexInParent);
                    } else {
                        o = new AccessibleLabel(xAccessibleContext);
                    }
                    break;
                case AccessibleRole.DIALOG:
                case AccessibleRole.FRAME:
                    o = new AccessibleFrame(
                        AccessibleRoleMap.toAccessibleRole(info.Role),
                        xAccessibleContext
                    );
                    break;
                case AccessibleRole.ICON:
                case AccessibleRole.GRAPHIC:
                case AccessibleRole.SHAPE:
                case AccessibleRole.EMBEDDED_OBJECT:
                    o = new AccessibleImage(xAccessibleContext);
                    break;
                case AccessibleRole.LISTITEM:
                    o = new AccessibleListItem(xAccessibleContext);
                    break;
                case AccessibleRole.MENU:
                    o = new AccessibleMenu(xAccessibleContext);
                    break;
                case AccessibleRole.MENUITEM:
                    o = new AccessibleMenuItem(xAccessibleContext);
                    break;
                case AccessibleRole.PARAGRAPH:
                    o = new AccessibleParagraph(xAccessibleContext);
                    break;
                case AccessibleRole.PUSHBUTTON:
                    o = new AccessibleButton(
                        javax.accessibility.AccessibleRole.PUSH_BUTTON,
                        javax.accessibility.AccessibleState.SELECTED,
                        xAccessibleContext
                        );
                    break;
                case AccessibleRole.RADIOBUTTON:
                    o = new AccessibleButton(
                        javax.accessibility.AccessibleRole.RADIO_BUTTON,
                        javax.accessibility.AccessibleState.CHECKED,
                        xAccessibleContext
                        );
                    break;
                case AccessibleRole.SCROLLBAR:
                    o = new AccessibleScrollBar(xAccessibleContext);
                    break;
                case AccessibleRole.SEPARATOR:
                    o = new AccessibleSeparator(xAccessibleContext);
                    break;
                case AccessibleRole.TABLE:
                    if((info.States & AccessibleState.CHILDREN_TRANSIENT) != 0) {
                        o = new AccessibleSpreadsheet(xAccessibleContext);
                    } else {
                        o = new AccessibleTextTable(xAccessibleContext);
                    }
                    break;
                case AccessibleRole.TEXT:
                    o = new AccessibleEditLine(xAccessibleContext);
                    break;
                case AccessibleRole.TREE:
                    if((info.States & AccessibleState.CHILDREN_TRANSIENT) == 0) {
                        o = new AccessibleTreeList(xAccessibleContext);
                    } else {
                        o = new AccessibleTreeListItem(xAccessibleContext, info.IndexInParent);
                    }
                    break;
                case AccessibleRole.CANVAS:
                case AccessibleRole.DOCUMENT:
                case AccessibleRole.ENDNOTE:
                case AccessibleRole.FILLER:
                case AccessibleRole.FOOTER:
                case AccessibleRole.FOOTNOTE:
                case AccessibleRole.HEADER:
                case AccessibleRole.LAYEREDPANE:
                case AccessibleRole.OPTIONPANE:
                case AccessibleRole.PAGETAB:
                case AccessibleRole.PANEL:
                case AccessibleRole.ROOTPANE:
                case AccessibleRole.SCROLLPANE:
                case AccessibleRole.SPLITPANE:
                case AccessibleRole.STATUSBAR:
                case AccessibleRole.TOOLBAR:
                case AccessibleRole.WINDOW:
                    o = new AccessibleWindow(
                        AccessibleRoleMap.toAccessibleRole(info.Role),
                        xAccessibleContext
                    );
                    break;
                default:
                    if( Build.DEBUG) {
                        System.out.println("Unmapped role: " + AccessibleRoleMap.toAccessibleRole(info.Role)
                         + " (id = " + info.Role + ")");
                    }
                    o = new AccessibleWindow(
                        AccessibleRoleMap.toAccessibleRole(info.Role),
                        xAccessibleContext
                    );
                    break;
            }

            // Add the newly created object to the cache list
            synchronized (objectList) {
                objectList.put(oid, new WeakReference(o));
                if( Build.DEBUG ) {
//                  System.out.println("Object cache now contains " + objectList.size() + " objects.");
                }
            }

            // Register as event listener if possible
            AccessibleEventListener listener = null;
            if(o instanceof AccessibleEventListener && (info.States & AccessibleState.TRANSIENT) == 0) {
                listener = (AccessibleEventListener) o;
            }

            /* The accessible event broadcaster will never be removed by a removeEventListener
             * call. This requires that the UNO accessibility objects get activly destructed
             * using dispose().
             */
            synchronized (o) {
                o.initialize(infoProvider.getAccessibleComponentInfo(xAccessibleContext, listener));
            }

            // Create generic parent if parent is null
            if( parent == null && info.IndexInParent != -1 ) {
                parent = new GenericAccessibleParent(o, xAccessibleContext);
            }

            // Finaly set accessible parent object
            if( parent != null ) {
                o.setAccessibleParent(parent);
            }

            // Add the child to the internal list if parent is AccessibleWindow
            if( ((info.States & AccessibleState.CHILDREN_TRANSIENT) == 0) && (parent instanceof AccessibleWindow) ) {
                AccessibleWindow w = (AccessibleWindow) parent;
                w.addAccessibleChild(o);
            }
        }

        if(Build.DEBUG && o == null) {
            System.out.println("AccessibleObjectFactory: returning null object");
        }
        return o;
    }
/*
    public AccessibleObject removeAccessibleObject(XAccessible xAccessible) {
        XAccessibleContext xAccessibleContext = null;

        if(xAccessible == null) {
            return null;
        }

        // Save the round trip to C++ UNO if possible
        if(xAccessible instanceof XAccessibleContext) {
            xAccessibleContext = (XAccessibleContext) xAccessible;
        } else {
            xAccessibleContext = xAccessible.getAccessibleContext();
        }

        // Ensure that we really got an UNO accessible context
        if(xAccessibleContext == null) {
            return null;
        }

        // Retrieve unique id for the original UNO object to be used as a hash key
        String oid = UnoRuntime.generateOid(xAccessibleContext);
        AccessibleObject o = null;
        // Check if we already have a wrapper object for this context
        synchronized (objectList) {
            // Do not remove the object from the list, may need to recycle
            // them as long as they are do garbage collected
            WeakReference r = (WeakReference) objectList.get(oid);
            if(r != null) {
                o = (AccessibleObject) r.get();
            }
        }

        // FIXME create dummy for objects not found.

        return null;
    }
*/
    public static XAccessible toXAccessible(Object any) {
        if(AnyConverter.isObject(any)) {
            try {
                return (XAccessible) AnyConverter.toObject(XAccessibleType, any);
            }

            catch(com.sun.star.lang.IllegalArgumentException e) {
                // FIXME: output
            }
        }

        return null;
    }

    public java.lang.Object[] getAccessibleObjectSet(java.lang.Object o) {
        java.lang.Object[] list = null;

        if( o instanceof java.lang.Object[] ) {
            list = (java.lang.Object[]) o;
        } else if( o instanceof com.sun.star.uno.Any && AnyConverter.isArray(o) ) {
            try {
                list = (java.lang.Object[]) AnyConverter.toArray(o);
            }

            catch(com.sun.star.lang.IllegalArgumentException e) {
                // this should never happen
            }
        }

        if( list != null ) {
            java.util.ArrayList set = new java.util.ArrayList(list.length);

            for(int i=0; i<list.length; i++) {
                AccessibleObject ao = getAccessibleObject(toXAccessible(list[i]), true,  null);
                if( ao != null ) {
                    set.add(ao);
                }
            }

            set.trimToSize();
            return set.toArray();
        }

        return null;
    }

    public void releaseAccessibleObject(AccessibleObject o) {
        synchronized (objectList) {
            objectList.remove(o.getObjectId());
            if( Build.DEBUG ) {
                System.out.println("Object cache now contains " + objectList.size() + " objects.");
            }
        }
    }

}


