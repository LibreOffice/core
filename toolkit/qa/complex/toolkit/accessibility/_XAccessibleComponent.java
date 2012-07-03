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

package complex.toolkit.accessibility;

import java.util.ArrayList;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Size;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

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
public class _XAccessibleComponent {

    // private LogWriter log;

    public XAccessibleComponent oObj = null;

    private Rectangle bounds = null ;

    /**
     * Constructor
     * @param object
     */
    public _XAccessibleComponent(XInterface object/*, LogWriter log*/) {
        oObj = UnoRuntime.queryInterface(
                                    XAccessibleComponent.class, object);
        // this.log = log;
    }

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
      * @return
      */
    public boolean _containsPoint() {

        boolean result = true ;

        int curX = 0;
        //while (!oObj.containsPoint(new Point(curX, bounds.Y)) && curX < bounds.Width+bounds.X) {
        while (!oObj.containsPoint(new Point(curX, 0)) && curX < bounds.Width) {
            curX++;
        }
        //if ((bounds.X <= curX) && (curX < bounds.Width+bounds.X)) {
        if (curX < bounds.Width) {
            System.out.println("Upper bound of box contains point ("
                + curX + ",0) - OK");
        } else {
            System.out.println
                ("Upper bound of box contains no component points - FAILED");
            result = false;
        }

        curX = 0;
        //while (!oObj.containsPoint(new Point(curX, bounds.Y+bounds.Height - 1))
        while (!oObj.containsPoint(new Point(curX, bounds.Height - 1))
               && curX < bounds.Width) {

               System.out.println("Contains returns false for ("+curX+","+bounds.Height+")");
            curX++;
        }
        //if ((bounds.X <= curX) && (curX < bounds.Width+bounds.X)) {
        if (curX < bounds.Width) {
            System.out.println("Lower bound of box contains point ("
                + curX + "," + (bounds.Height - 1) + ") - OK");
        } else {
            System.out.println
                ("Lower bound of box contains no component points - FAILED");
            result = false;
        }

        int curY = 0;
        //while (!oObj.containsPoint(new Point(bounds.X, curY)) && curY < bounds.Height+bounds.Y) {
        while (!oObj.containsPoint(new Point(0, curY)) && curY < bounds.Height) {
            curY++;
        }
        //if ((bounds.Y <= curY) && (curY < bounds.Height+bounds.Y)) {
        if (curY < bounds.Height) {
            System.out.println("Left bound of box contains point (0,"
                + curY + ") - OK");
        } else {
            System.out.println
                ("Left bound of box contains no component points - FAILED");
            result = false;
        }

        curY = 0;
        //while (!oObj.containsPoint(new Point(bounds.X+bounds.Width - 1, curY))
        //       && curY < bounds.Height+bounds.Y) {
        while (!oObj.containsPoint(new Point(bounds.Width - 1, curY)) && curY < bounds.Height) {
            curY++;
        }
        //if ((bounds.Y <= curY) && (curY < bounds.Height + bounds.Y)) {
        if (curY < bounds.Height) {
            System.out.println("Right bound of box contains point ("
                + (bounds.Width - 1) + "," + curY + ") - OK");
        } else {
            System.out.println
                ("Right bound of box contains no component points - FAILED");
            result = false;
        }

        boolean locRes = true;
        for (int x = -1; x <= bounds.Width; x++) {
            locRes &= !oObj.containsPoint(new Point(x, -1));
            locRes &= !oObj.containsPoint(new Point(x, bounds.Height+bounds.Y));
        }
        if (locRes) {
            System.out.println("Outer upper and lower bounds contain no component "
                + "points - OK");
        } else {
            System.out.println("Outer upper and lower bounds CONTAIN some component "
                + "points - FAILED");
            result = false;
        }

        locRes = true;
        for (int y = -1; y <= bounds.Height; y++) {
            locRes &= !oObj.containsPoint(new Point(-1, y));
            locRes &= !oObj.containsPoint(new Point(bounds.X+bounds.Width, y));
        }
        if (locRes) {
            System.out.println("Outer left and right bounds contain no component "
                + "points - OK");
        } else {
            System.out.println("Outer left and right bounds CONTAIN some component "
                + "points - FAILED");
            result = false;
        }

        return result;
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
     * @return
     */
    public boolean _getAccessibleAtPoint() {

        boolean result = true ;
        XAccessibleComponent[] children = getChildrenComponents();

        if (children.length > 0) {
            for (int i = 0; i < children.length; i++) {
                Rectangle chBnd = children[i].getBounds();
                if (chBnd.X == -1)
                {
                    continue;
                }
                System.out.println("Checking child with bounds " +
                    "(" + chBnd.X + "," + chBnd.Y + "),("
                    + chBnd.Width + "," + chBnd.Height + "): "
                    +  util.AccessibilityTools.accessibleToString(children[i]));

                System.out.println("finding the point which lies on the component");
                int curX = 0;
                int curY = 0;
                while (!children[i].containsPoint(new Point(curX, curY))
                       && curX < chBnd.Width) {
                    curX++;
                    curY++;
                }

                if (curX==chBnd.Width) {
                    System.out.println("Couldn't find a point with contains");
                    continue;
                }

                // trying the point laying on child
                XAccessible xAcc = oObj.getAccessibleAtPoint
                    (new Point(chBnd.X , chBnd.Y));
                if (xAcc == null) {
                    System.out.println("The child not found at point ("
                        + (chBnd.X ) + "," + chBnd.Y + ") - FAILED");
                    result = false;
                } else {
                    XAccessible xAccCh = UnoRuntime.queryInterface
                        (XAccessible.class, children[i]);
                    System.out.println("Child found at point ("
                        + (chBnd.X ) + "," + chBnd.Y + ") - OK");
                    boolean res = util.AccessibilityTools.equals(xAccCh, xAcc);
                    if (!res) {
                        int expIndex = xAccCh.getAccessibleContext().getAccessibleIndexInParent();
                        int gotIndex = xAcc.getAccessibleContext().getAccessibleIndexInParent();
                        if (expIndex < gotIndex) {
                            System.out.println("The children found is not the same");
                            System.out.println("The expected child " +
                                xAccCh.getAccessibleContext().getAccessibleName());
                            System.out.println("is hidden behind the found Child ");
                            System.out.println(xAcc.getAccessibleContext().getAccessibleName()+" - OK");
                        } else {
                            System.out.println("The children found is not the same - FAILED");
                            System.out.println("Expected: "
                                +xAccCh.getAccessibleContext().getAccessibleName());
                            System.out.println("Found: "
                                +xAcc.getAccessibleContext().getAccessibleName());
                            result = false ;
                        }
                    }
                }

                // trying the point NOT laying on child
                xAcc = oObj.getAccessibleAtPoint
                    (new Point(chBnd.X - 1, chBnd.Y - 1));
                if (xAcc == null) {
                    System.out.println("No children found at point ("
                        + (chBnd.X - 1) + "," + (chBnd.Y - 1) + ") - OK");
                    result &= true;
                } else {
                    XAccessible xAccCh = UnoRuntime.queryInterface(XAccessible.class, children[i]);
                    boolean res = util.AccessibilityTools.equals(xAccCh, xAcc);
                    if (res) {
                        System.out.println("The same child found outside "
                            + "its bounds - FAILED");
                        result = false ;
                    }
                }
            }
        } else {
            System.out.println("There are no children supporting "
                + "XAccessibleComponent");
        }

        return result;
    }

    /**
     * Retrieves the component bounds and stores it. <p>
     *
     * Has <b> OK </b> status if boundary position (x,y) is not negative
     * and size (Width, Height) is greater than 0.
     * @return
     */
    public boolean _getBounds() {
        boolean result = true ;

        bounds = oObj.getBounds() ;
        result &= bounds != null
            && bounds.X >=0 && bounds.Y >=0
            && bounds.Width >0 && bounds.Height >0;

        System.out.println("Bounds = " + (bounds != null
             ? "(" + bounds.X + "," + bounds.Y + "),("
             + bounds.Width + "," + bounds.Height + ")" : "null"));

        return result;
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
     * @return
     */
    public boolean _getLocation() {

        boolean result = true ;
        Point loc = oObj.getLocation() ;

        result &= loc.X == bounds.X && loc.Y == bounds.Y ;

        return result;
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
     * @return
     */
    public boolean _getLocationOnScreen() {

        XAccessibleComponent parent = getParentComponent();

        boolean result = true ;
        Point loc = oObj.getLocationOnScreen();
        System.out.println("Location is (" + loc.X + "," + loc.Y + ")");

        if (parent != null) {
            Point parLoc = parent.getLocationOnScreen();
            System.out.println("Parent location is ("
                + parLoc.X + "," + parLoc.Y + ")");

            result &= parLoc.X + bounds.X == loc.X;
            result &= parLoc.Y + bounds.Y == loc.Y;
        }

        return result;
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
     * @return
     */
    public boolean _getSize() {

        boolean result = true ;
        Size size = oObj.getSize() ;

        result &= size.Width == bounds.Width;
        result &= size.Height == bounds.Height;

        return result;
    }

    /**
     * Just calls the method. <p>
     *
     * Has <b> OK </b> status if no runtime exceptions occurred.
     * @return
     */
    public boolean _grabFocus() {

        boolean result = true ;
        oObj.grabFocus() ;

        return result ;
    }

    /**
     * Retrieves all children (not more than 50) of the current
     * component which support <code>XAccessibleComponent</code>.
     *
     * @return The array of children. Empty array returned if
     * such children were not found or some error occurred.
     */
    private XAccessibleComponent[] getChildrenComponents() {
        XAccessible xAcc = UnoRuntime.queryInterface(XAccessible.class, oObj) ;
        if (xAcc == null) {
            System.out.println("Component doesn't support XAccessible.");
            return new XAccessibleComponent[0];
        }
        XAccessibleContext xAccCon = xAcc.getAccessibleContext();
        int cnt = xAccCon.getAccessibleChildCount();

        // for cases when too many children exist checking only first 50
        if (cnt > 50)
        {
            cnt = 50;
        }

        ArrayList<XAccessibleComponent> childComp = new ArrayList<XAccessibleComponent>();
        for (int i = 0; i < cnt; i++) {
            try {
                XAccessible child = xAccCon.getAccessibleChild(i);
                XAccessibleContext xAccConCh = child.getAccessibleContext();
                XAccessibleComponent xChAccComp = UnoRuntime.queryInterface(XAccessibleComponent.class, xAccConCh);
                if (xChAccComp != null) {
                    childComp.add(xChAccComp) ;
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {}
        }

        return childComp.toArray(new XAccessibleComponent[childComp.size()]);
    }

    /**
     * Gets the parent of the current component which support
     * <code>XAccessibleComponent</code>.
     *
     * @return The parent or <code>null</code> if the component
     * has no parent or some errors occurred.
     */
    private XAccessibleComponent getParentComponent() {
        XAccessible xAcc = UnoRuntime.queryInterface(XAccessible.class, oObj) ;
        if (xAcc == null) {
            System.out.println("Component doesn't support XAccessible.");
            return null;
        }

        XAccessibleContext xAccCon = xAcc.getAccessibleContext();
        XAccessible xAccPar = xAccCon.getAccessibleParent();

        if (xAccPar == null) {
            System.out.println("Component has no accessible parent.");
            return null;
        }
        XAccessibleContext xAccConPar = xAccPar.getAccessibleContext();
        XAccessibleComponent parent = UnoRuntime.queryInterface(XAccessibleComponent.class, xAccConPar);
        if (parent == null) {
            System.out.println
                ("Accessible parent doesn't support XAccessibleComponent");
            return null;
        }

        return parent;
    }

    /**
     * Just calls the method.
     * @return
     */
    public boolean _getForeground() {
        int forColor = oObj.getForeground();
        System.out.println("getForeground(): " + forColor);
        return true;
    }

    /**
     * Just calls the method.
     * @return
     */
    public boolean _getBackground() {
        int backColor = oObj.getBackground();
        System.out.println("getBackground(): " + backColor);
        return true;
    }

}
