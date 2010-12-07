/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package complex.toolkit.accessibility;

import com.sun.star.lang.Locale;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.IllegalAccessibleComponentStateException;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleRelationSet;
import com.sun.star.accessibility.XAccessibleStateSet;
// import com.sun.star.accessibility.AccessibleRelationType;
import util.AccessibilityTools;
// import share.LogWriter;

/**
 * Testing <code>com.sun.star.accessibility.XAccessibleContext</code>
 * interface methods :
 * <ul>
 *  <li><code> getAccessibleChildCount()</code></li>
 *  <li><code> getAccessibleChild()</code></li>
 *  <li><code> getAccessibleParent()</code></li>
 *  <li><code> getAccessibleIndexInParent()</code></li>
 *  <li><code> getAccessibleRole()</code></li>
 *  <li><code> getAccessibleDescription()</code></li>
 *  <li><code> getAccessibleName()</code></li>
 *  <li><code> getAccessibleRelationSet()</code></li>
 *  <li><code> getAccessibleStateSet()</code></li>
 *  <li><code> getLocale()</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleContext
 */
public class _XAccessibleContext {

    // private LogWriter log;

    private static final String className =
        "com.sun.star.accessibility.XAccessibleContext" ;

    public XAccessibleContext oObj = null;

    private int childCount = 0;
    private XAccessible parent = null ;

    public _XAccessibleContext(XInterface object) {
        oObj = UnoRuntime.queryInterface(XAccessibleContext.class, object);
        // this.log = log;
    }

    /**
     * Calls the method and stores the number of children. <p>
     * Has <b> OK </b> status if non-negative number rutrned.
     * @return
     */
    public boolean _getAccessibleChildCount() {
        childCount = oObj.getAccessibleChildCount();
        System.out.println("" + childCount + " children found.");
        return childCount > -1;
    }

    /**
     * Tries to get every child and checks its parent. <p>
     *
     * Has <b> OK </b> status if parent of every child
     * and the tested component are the same objects.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getAccessibleChildCount() </code> : to have a number of
     *     children </li>
     * </ul>
     * @return
     */
    public boolean _getAccessibleChild() {
        boolean bOK = true;
        int counter = childCount;
        if (childCount > 500)
        {
            counter = 500;
        }
        for (int i = 0; i < counter; i++) {
            try {
                XAccessible ch = oObj.getAccessibleChild(i) ;
                XAccessibleContext chAC = ch.getAccessibleContext();

                System.out.println("  Child " + i + ": " +
                    chAC.getAccessibleDescription()) ;

                if (!AccessibilityTools.equals
                    (chAC.getAccessibleParent().getAccessibleContext(), oObj)){

                    System.out.println("Role:");
                    System.out.println("Getting: "+chAC.getAccessibleParent().getAccessibleContext().getAccessibleRole());
                    System.out.println("Expected: "+oObj.getAccessibleRole());

                    System.out.println("ImplementationName:");
                    System.out.println("Getting: "+util.utils.getImplName(chAC.getAccessibleParent().getAccessibleContext()));
                    System.out.println("Expected: "+util.utils.getImplName(oObj));

                    System.out.println("The parent of child and component "+
                        "itself differ.");
                    System.out.println("Getting(Description): "
                            +chAC.getAccessibleParent().getAccessibleContext().getAccessibleDescription());
                    System.out.println("Expected(Description): "
                            +oObj.getAccessibleDescription());

                    bOK = false;
                } else {
                    System.out.println("Getting the expected Child -- OK");
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                e.printStackTrace();
                bOK = false;
            }
        }

        return bOK;
    }

    /**
     * Just gets the parent. <p>
     *
     * Has <b> OK </b> status if parent is not null.
     * @return
     */
    public boolean _getAccessibleParent() {
        // assume that the component is not ROOT
        parent = oObj.getAccessibleParent();
        return parent != null;
    }

    /**
     * Retrieves the index of tested component in its parent.
     * Then gets the parent's child by this index and compares
     * it with tested component.<p>
     *
     * Has <b> OK </b> status if the parent's child and the tested
     * component are the same objects.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getAccessibleParent() </code> : to have a parent </li>
     * </ul>
     * @return
     */
    public boolean _getAccessibleIndexInParent() {

        boolean bOK = true;
        int idx = oObj.getAccessibleIndexInParent();

        XAccessibleContext parentAC = parent.getAccessibleContext() ;
        try {
            bOK &= AccessibilityTools.equals(
                parentAC.getAccessibleChild(idx).getAccessibleContext(),oObj);
            if (!bOK) {
                System.out.println("Expected: "+util.utils.getImplName(oObj));
                System.out.println("Getting: "+util.utils.getImplName(
                    parentAC.getAccessibleChild(idx).getAccessibleContext()));
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
            bOK = false;
        }
        return bOK;
    }

    /**
     * Get the accessible role of component. <p>
     *
     * Has <b> OK </b> status if non-negative number rutrned.
     * @return
     */
    public boolean _getAccessibleRole() {
        short role = oObj.getAccessibleRole();
        System.out.println("The role is " + role);
        return role > -1;
    }

    /**
     * Get the accessible name of the component. <p>
     *
     * Has <b> OK </b> status if the name has non-zero length.
     * @return
     */
    public boolean _getAccessibleName() {
        String name = oObj.getAccessibleName();
        System.out.println("The name is '" + name + "'");
        return name != null;
    }

    /**
     * Get the accessible description of the component. <p>
     *
     * Has <b> OK </b> status if the description has non-zero length.
     * @return
     */
    public boolean _getAccessibleDescription() {
        String descr = oObj.getAccessibleDescription();
        System.out.println("The description is '" + descr + "'");
        return descr != null;
    }

    /**
     * Just gets the set. <p>
     *
     * Has <b> OK </b> status if the set is not null.
     * @return
     */
    public boolean _getAccessibleRelationSet() {
        XAccessibleRelationSet set = oObj.getAccessibleRelationSet();
        return set != null;
    }

    /**
     * Just gets the set. <p>
     *
     * Has <b> OK </b> status if the set is not null.
     * @return
     */
    public boolean _getAccessibleStateSet() {
        XAccessibleStateSet set = oObj.getAccessibleStateSet();
        return set != null;
    }

    /**
     * Gets the locale. <p>
     *
     * Has <b> OK </b> status if <code>Country</code> and
     * <code>Language</code> fields of locale structure
     * are not empty.
     * @return
     */
    public boolean _getLocale() {
        Locale loc = null ;
        try {
            loc = oObj.getLocale();
            System.out.println("The locale is " + loc.Language + "," + loc.Country);
        } catch (IllegalAccessibleComponentStateException e) {
            e.printStackTrace();
        }

        return loc != null && loc.Language.length() > 0 &&
                                            loc.Country.length() > 0;
    }
}

