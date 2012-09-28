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
package ifc.accessibility;

import java.util.ArrayList;
import lib.MultiMethodTest;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Size;
import com.sun.star.uno.UnoRuntime;


/**
 * Testing <code>com.sun.star.accessibility.XAccessibleComponent</code>
 * interface methods :
 * <ul>
 *  <li><code> containsPoint()</code></li>
 *  <li><code> getAccessibleAtPoint()</code></li>
 *  <li><code> getBounds()</code></li>
 *  <li><code> getLocation()</code></li>
 *  <li><code> getLocationOnScreen()</code></li>
 *  <li><code> getSize()</code></li>
 *  <li><code> grabFocus()</code></li>
 *  <li><code> getAccessibleKeyBinding()</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleComponent
 */
public class _XAccessibleComponent extends MultiMethodTest {

    public XAccessibleComponent oObj = null;
    private Rectangle bounds = null;
    private ArrayList<Rectangle> KnownBounds = new ArrayList<Rectangle>();


    /**
     * First checks 4 inner bounds (upper, lower, left and right)
     * of component bounding box to contain
     * at least one point of the component. Second 4 outer bounds
     * are checked to not contain any component points.<p>
     *
     * Has <b> OK </b> status if inner bounds contain component points
     * and outer bounds don't contain any component points. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getBounds() </code> : to have size of a component.</li>
     * </ul>
     */
    public void _containsPoint() {
        requiredMethod("getBounds()");

        boolean result = true;

        int curX = 0;

        //while (!oObj.containsPoint(new Point(curX, bounds.Y)) && curX < bounds.Width+bounds.X) {
        while (!oObj.containsPoint(new Point(curX, 0)) &&
               (curX < bounds.Width)) {
            curX++;
        }

        //if ((bounds.X <= curX) && (curX < bounds.Width+bounds.X)) {
        if (curX < bounds.Width) {
            log.println("Upper bound of box containsPoint point (" + curX +
                        ",0) - OK");
        } else {
            log.println(
                    "Upper bound of box containsPoint no component points - FAILED");
            result = false;
        }

        curX = 0;

        //while (!oObj.containsPoint(new Point(curX, bounds.Y+bounds.Height - 1))
        while (!oObj.containsPoint(new Point(curX, bounds.Height - 1)) &&
               (curX < bounds.Width)) {
            log.println("containsPoint returns false for (" + curX + "," +
                        bounds.Height + ")");
            curX++;
        }

        //if ((bounds.X <= curX) && (curX < bounds.Width+bounds.X)) {
        if (curX < bounds.Width) {
            log.println("Lower bound of box containsPoint point (" + curX +
                        "," + (bounds.Height - 1) + ") - OK");
        } else {
            log.println(
                    "Lower bound of box containsPoint no component points - FAILED");
            result = false;
        }

        int curY = 0;

        //while (!oObj.containsPoint(new Point(bounds.X, curY)) && curY < bounds.Height+bounds.Y) {
        while (!oObj.containsPoint(new Point(0, curY)) &&
               (curY < bounds.Height)) {
            curY++;
        }

        //if ((bounds.Y <= curY) && (curY < bounds.Height+bounds.Y)) {
        if (curY < bounds.Height) {
            log.println("Left bound of box containsPoint point (0," + curY +
                        ") - OK");
        } else {
            log.println(
                    "Left bound of box containsPoint no component points - FAILED");
            result = false;
        }

        curY = 0;

        //while (!oObj.containsPoint(new Point(bounds.X+bounds.Width - 1, curY))
        //       && curY < bounds.Height+bounds.Y) {
        while (!oObj.containsPoint(new Point(bounds.Width - 1, curY)) &&
               (curY < bounds.Height)) {
            curY++;
        }

        //if ((bounds.Y <= curY) && (curY < bounds.Height + bounds.Y)) {
        if (curY < bounds.Height) {
            log.println("Right bound of box containsPoint point (" +
                        (bounds.Width - 1) + "," + curY + ") - OK");
        } else {
            log.println(
                    "Right bound of box containsPoint no component points - FAILED");
            result = false;
        }

        boolean locRes = true;

        for (int x = -1; x <= bounds.Width; x++) {
            if (oObj.containsPoint(new Point(x, -1))) {
                log.println(
                    "Outer upper and lower bounds CONTAIN some component point"
                    + " (" + x + ", -1) - FAILED");
                locRes = false;
                break;
            }
            if (oObj.containsPoint(new Point(x, bounds.Height + bounds.Y))) {
                log.println(
                    "Outer upper and lower bounds CONTAIN some component point"
                    + " (" + x + ", " + bounds.Height + bounds.Y
                    + ") - FAILED");
                locRes = false;
                break;
            }
        }

        if (locRes) {
            log.println("Outer upper and lower bounds contain no component " +
                        "points - OK");
        } else {
            result = false;
        }

        locRes = true;

        for (int y = -1; y <= bounds.Height; y++) {
            if (oObj.containsPoint(new Point(-1, y))) {
                log.println(
                    "Outer left and right bounds CONTAIN some component point"
                    + " (-1, " + y + ") - FAILED");
                locRes = false;
                break;
            }
            if (oObj.containsPoint(new Point(bounds.X + bounds.Width, y))) {
                log.println(
                    "Outer left and right bounds CONTAIN some component point"
                    + " (" + bounds.X + bounds.Width + ", " + y + ") - FAILED");
                locRes = false;
                break;
            }
        }

        if (locRes) {
            log.println("Outer left and right bounds contain no component " +
                        "points - OK");
        } else {
            result = false;
        }

        tRes.tested("containsPoint()", result);
    }

    /**
     * Iterates through all children which implement
     * <code>XAccessibleComponent</code> (if they exist) determines their
     * boundaries and tries to get each child by <code>getAccessibleAtPoint</code>
     * passing point which belongs to the child.
     * Also the point is checked which doesn't belong to child boundary
     * box. <p>
     *
     * Has <b> OK </b> status if in the first cases the right children
     * are returned, and in the second <code>null</code> or
     * another child is returned.
     */
    public void _getAccessibleAtPoint() {
        boolean result = true;
        XAccessibleComponent[] children = getChildrenComponents();

        if (children.length > 0) {
            for (int i = 0; i < children.length; i++) {
                Rectangle chBnd = children[i].getBounds();

                if (chBnd.X == -1) {
                    continue;
                }

                log.println("Checking child with bounds " + "(" + chBnd.X +
                            "," + chBnd.Y + "),(" + chBnd.Width + "," +
                            chBnd.Height + "): " +
                            util.AccessibilityTools.accessibleToString(
                                    children[i]));

                XAccessibleContext xAc = UnoRuntime.queryInterface(
                                                 XAccessibleContext.class,
                                                 children[i]);

                boolean MightBeCovered = false;
                boolean isShowing = xAc.getAccessibleStateSet()
                                       .contains(com.sun.star.accessibility.AccessibleStateType.SHOWING);
                log.println("\tStateType containsPoint SHOWING: " +
                            isShowing);

                if (!isShowing) {
                    log.println("Child is invisible - OK");

                    continue;
                }

                log.println("finding the point which lies on the component");

                int curX = chBnd.Width / 2;
                int curY = chBnd.Height / 2;

                while (!children[i].containsPoint(new Point(curX, curY)) &&
                       (curX > 0) && (curY > 0)) {
                    curX--;
                    curY--;
                }

                if ((curX == chBnd.Width) && isShowing) {
                    log.println("Couldn't find a point with containsPoint");

                    continue;
                }

                // trying the point laying on child
                XAccessible xAcc = oObj.getAccessibleAtPoint(
                                           new Point(chBnd.X + curX,
                                                     chBnd.Y + curY));


                Point p = new Point(chBnd.X + curX,chBnd.X + curX);

                if (isCovered(p) && isShowing) {
                    log.println(
                            "Child might be covered by another and can't be reached");
                    MightBeCovered = true;
                }

                KnownBounds.add(chBnd);

                if (xAcc == null) {
                    log.println("The child not found at point (" +
                                (chBnd.X + curX) + "," + (chBnd.Y + curY) +
                                ") - FAILED");

                    if (isShowing) {
                        result = false;
                    } else {
                        result &= true;
                    }
                } else {
                    XAccessible xAccCh = UnoRuntime.queryInterface(
                                                 XAccessible.class,
                                                 children[i]);
                    XAccessibleContext xAccC = UnoRuntime.queryInterface(
                                                       XAccessibleContext.class,
                                                       children[i]);
                    log.println("Child found at point (" + (chBnd.X + curX) +
                                "," + (chBnd.Y + curY) + ") - OK");

                    boolean res = false;
                    int expIndex;
                    String expName;
                    String expDesc;

                    if (xAccCh != null) {
                        res = util.AccessibilityTools.equals(xAccCh, xAcc);
                        expIndex = xAccCh.getAccessibleContext()
                                         .getAccessibleIndexInParent();
                        expName = xAccCh.getAccessibleContext()
                                        .getAccessibleName();
                        expDesc = xAccCh.getAccessibleContext()
                                        .getAccessibleDescription();
                    } else {
                        res = xAccC.getAccessibleName()
                                   .equals(xAcc.getAccessibleContext()
                                               .getAccessibleName());
                        expIndex = xAccC.getAccessibleIndexInParent();
                        expName = xAccC.getAccessibleName();
                        expDesc = xAccC.getAccessibleDescription();
                    }

                    if (!res) {
                        int gotIndex = xAcc.getAccessibleContext()
                                           .getAccessibleIndexInParent();

                        if (expIndex < gotIndex) {
                            log.println("The children found is not the same");
                            log.println("The expected child " + expName);
                            log.print("is hidden behind the found Child ");
                            log.println(xAcc.getAccessibleContext()
                                            .getAccessibleName() + " - OK");
                        } else {
                            log.println(
                                    "The children found is not the same");
                            log.println("Expected: " + expName);
                            log.println("Description:  " + expDesc);
                            log.println("Found: " +
                                        xAcc.getAccessibleContext()
                                            .getAccessibleName());
                            log.println("Description:  " +
                                        xAcc.getAccessibleContext()
                                            .getAccessibleDescription());
                            if (MightBeCovered) {
                                log.println("... Child is covered by another - OK");
                            } else {
                                log.println("... FAILED");
                                result = false;
                            }

                        }
                    }
                }


                // trying the point NOT laying on child
                xAcc = oObj.getAccessibleAtPoint(
                               new Point(chBnd.X - 1, chBnd.Y - 1));

                if (xAcc == null) {
                    log.println("No children found at point (" +
                                (chBnd.X - 1) + "," + (chBnd.Y - 1) +
                                ") - OK");
                    result &= true;
                } else {
                    XAccessible xAccCh = UnoRuntime.queryInterface(
                                                 XAccessible.class,
                                                 children[i]);
                    boolean res = util.AccessibilityTools.equals(xAccCh, xAcc);

                    if (res) {
                        log.println("The same child found outside " +
                                    "its bounds at (" + (chBnd.X - 1) + "," +
                                    (chBnd.Y - 1) + ") - FAILED");
                        result = false;
                    }
                }
            }
        } else {
            log.println("There are no children supporting " +
                        "XAccessibleComponent");
        }

        tRes.tested("getAccessibleAtPoint()", result);
    }

    /**
     * Retrieves the component bounds and stores it. <p>
     *
     * Has <b> OK </b> status if boundary position (x,y) is not negative
     * and size (Width, Height) is greater than 0.
     */
    public void _getBounds() {
        boolean result = true;

        bounds = oObj.getBounds();
        result &= ((bounds != null) && (bounds.X >= 0) && (bounds.Y >= 0) && (bounds.Width > 0) && (bounds.Height > 0));

        log.println("Bounds = " +
                    ((bounds != null)
                     ? ("(" + bounds.X + "," + bounds.Y + "),(" +
                               bounds.Width + "," + bounds.Height + ")") : "null"));

        tRes.tested("getBounds()", result);
    }

    /**
     * Gets the location. <p>
     *
     * Has <b> OK </b> status if the location is the same as location
     * of boundary obtained by <code>getBounds()</code> method.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getBounds() </code> : to have bounds </li>
     * </ul>
     */
    public void _getLocation() {
        requiredMethod("getBounds()");

        boolean result = true;
        Point loc = oObj.getLocation();

        result &= ((loc.X == bounds.X) && (loc.Y == bounds.Y));

        tRes.tested("getLocation()", result);
    }

    /**
     * Get the screen location of the component and its parent
     * (if it exists and supports <code>XAccessibleComponent</code>). <p>
     *
     * Has <b> OK </b> status if component screen location equals
     * to screen location of its parent plus location of the component
     * relative to the parent. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getBounds() </code> : to have location of the component
     *      relative to its parent</li>
     * </ul>
     */
    public void _getLocationOnScreen() {
        requiredMethod("getBounds()");

        XAccessibleComponent parent = getParentComponent();

        boolean result = true;
        Point loc = oObj.getLocationOnScreen();
        log.println("Location is (" + loc.X + "," + loc.Y + ")");

        if (parent != null) {
            Point parLoc = parent.getLocationOnScreen();
            log.println("Parent location is (" + parLoc.X + "," + parLoc.Y +
                        ")");

            result &= ((parLoc.X + bounds.X) == loc.X);
            result &= ((parLoc.Y + bounds.Y) == loc.Y);
        }

        tRes.tested("getLocationOnScreen()", result);
    }

    /**
     * Obtains the size of the component. <p>
     *
     * Has <b> OK </b> status if the size is the same as in bounds. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getBounds() </code>  </li>
     * </ul>
     */
    public void _getSize() {
        requiredMethod("getBounds()");

        boolean result = true;
        Size size = oObj.getSize();

        result &= (size.Width == bounds.Width);
        result &= (size.Height == bounds.Height);

        tRes.tested("getSize()", result);
    }

    /**
     * Just calls the method. <p>
     *
     * Has <b> OK </b> status if no runtime exceptions occurred.
     */
    public void _grabFocus() {
        boolean result = true;
        oObj.grabFocus();

        tRes.tested("grabFocus()", result);
    }

    /**
     * Retrieves all children (not more than 50) of the current
     * component which support <code>XAccessibleComponent</code>.
     *
     * @return The array of children. Empty array returned if
     * such children were not found or some error occurred.
     */
    private XAccessibleComponent[] getChildrenComponents() {
        XAccessible xAcc = UnoRuntime.queryInterface(
                                   XAccessible.class, oObj);

        if (xAcc == null) {
            log.println("Component doesn't support XAccessible.");

            return new XAccessibleComponent[0];
        }

        XAccessibleContext xAccCon = xAcc.getAccessibleContext();
        int cnt = xAccCon.getAccessibleChildCount();

        // for cases when too many children exist checking only first 50
        if (cnt > 50) {
            cnt = 50;
        }

        ArrayList<XAccessibleComponent> childComp = new ArrayList<XAccessibleComponent>();

        for (int i = 0; i < cnt; i++) {
            try {
                XAccessible child = xAccCon.getAccessibleChild(i);
                XAccessibleContext xAccConCh = child.getAccessibleContext();
                XAccessibleComponent xChAccComp = UnoRuntime.queryInterface(
                                                          XAccessibleComponent.class,
                                                          xAccConCh);

                if (xChAccComp != null) {
                    childComp.add(xChAccComp);
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            }
        }

        return childComp.toArray(
                         new XAccessibleComponent[childComp.size()]);
    }

    /**
     * Gets the parent of the current component which support
     * <code>XAccessibleComponent</code>.
     *
     * @return The parent or <code>null</code> if the component
     * has no parent or some errors occurred.
     */
    private XAccessibleComponent getParentComponent() {
        XAccessible xAcc = UnoRuntime.queryInterface(
                                   XAccessible.class, oObj);

        if (xAcc == null) {
            log.println("Component doesn't support XAccessible.");

            return null;
        }

        XAccessibleContext xAccCon = xAcc.getAccessibleContext();
        XAccessible xAccPar = xAccCon.getAccessibleParent();

        if (xAccPar == null) {
            log.println("Component has no accessible parent.");

            return null;
        }

        XAccessibleContext xAccConPar = xAccPar.getAccessibleContext();
        XAccessibleComponent parent = UnoRuntime.queryInterface(
                                              XAccessibleComponent.class,
                                              xAccConPar);

        if (parent == null) {
            log.println(
                    "Accessible parent doesn't support XAccessibleComponent");

            return null;
        }

        return parent;
    }

    /**
     * Just calls the method.
     */
    public void _getForeground() {
        int forColor = oObj.getForeground();
        log.println("getForeground(): " + forColor);
        tRes.tested("getForeground()", true);
    }

    /**
     * Just calls the method.
     */
    public void _getBackground() {
        int backColor = oObj.getBackground();
        log.println("getBackground(): " + backColor);
        tRes.tested("getBackground()", true);
    }

    /**
     * Restores initial component text.
     */
    protected void after() {
        if (tEnv.getObjRelation("Destroy") != null) {
            disposeEnvironment();
        }
    }

    private boolean isCovered(Point p) {
        int elements = KnownBounds.size();
        boolean Covered = false;
        for (int k=0;k<elements;k++) {
            Rectangle known = KnownBounds.get(k);
            Covered = (known.X < p.X);
            Covered &= (known.Y < p.Y);
            Covered &= (p.Y < known.Y+known.Height);
            Covered &= (p.X < known.X+known.Width);

            if (Covered) {
                break;
            }
        }
        return Covered;
    }
}