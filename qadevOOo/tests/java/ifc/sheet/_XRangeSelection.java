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
package ifc.sheet;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Point;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XModel;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.RangeSelectionEvent;
import com.sun.star.sheet.XRangeSelection;
import com.sun.star.sheet.XRangeSelectionChangeListener;
import com.sun.star.sheet.XRangeSelectionListener;
import com.sun.star.uno.UnoRuntime;
import java.awt.Robot;
import java.awt.event.InputEvent;
import java.io.PrintWriter;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.AccessibilityTools;

/**
 * Check the XRangeSelection interface.
 */
public class _XRangeSelection extends MultiMethodTest {
    public XRangeSelection oObj = null;
    MyRangeSelectionListener aListener = null;

    public void before() {
        aListener = new _XRangeSelection.MyRangeSelectionListener(log);
        // workaround for i34499
        XModel xModel = (XModel)tEnv.getObjRelation("FirstModel");
        if (xModel == null)
            throw new StatusException(Status.failed("Object relation FirstModel' not set."));
        XWindow xWindow = xModel.getCurrentController().getFrame().getContainerWindow();
        XTopWindow xTopWindow = UnoRuntime.queryInterface(XTopWindow.class, xWindow);
        xTopWindow.toFront();
        util.utils.shortWait(500);
    }


    public void _abortRangeSelection() {
        requiredMethod("removeRangeSelectionChangeListener()");
        requiredMethod("removeRangeSelectionListener()");
        oObj.abortRangeSelection();
        tRes.tested("abortRangeSelection()", true);
    }

    public void _addRangeSelectionChangeListener() {
        oObj.addRangeSelectionChangeListener(aListener);
        tRes.tested("addRangeSelectionChangeListener()", true);
    }

    public void _addRangeSelectionListener() {
        oObj.addRangeSelectionListener(aListener);
        tRes.tested("addRangeSelectionListener()", true);
    }

    public void _removeRangeSelectionChangeListener() {
        oObj.removeRangeSelectionChangeListener(aListener);
        tRes.tested("removeRangeSelectionChangeListener()", true);
    }

    public void _removeRangeSelectionListener() {
        oObj.removeRangeSelectionListener(aListener);
        tRes.tested("removeRangeSelectionListener()", true);
    }

    public void _startRangeSelection() {
        requiredMethod("addRangeSelectionChangeListener()");
        requiredMethod("addRangeSelectionListener()");

        // get the sheet center
        Point center = getSheetCenter();
        if (center == null)
            throw new StatusException(Status.failed("Couldn't get the sheet center."));

        PropertyValue[] props = new PropertyValue[3];
        props[0] = new PropertyValue();
        props[0].Name = "InitialValue";
        props[0].Value = "B3:D5";
        props[1] = new PropertyValue();
        props[1].Name = "Title";
        props[1].Value = "the title";
        props[2] = new PropertyValue();
        props[2].Name = "CloseOnMouseRelease";
        props[2].Value = Boolean.FALSE;
        oObj.startRangeSelection(props);
        // wait for listeners
        util.utils.shortWait(1000);

        // get closer button: move if window cobvers the sheet center
        Point closer = getCloser(center);
        if (closer == null)
            throw new StatusException(Status.failed("Couldn't get the close Button."));

        // do something to trigger the listeners
        clickOnSheet(center);
        util.utils.shortWait(5000);

        // click on closer
        clickOnSheet(closer);
        util.utils.shortWait(5000);

//        System.out.println("X: " + closer.X + "    Y: " + closer.Y);

        // just check that we do not have the page instead of the range descriptor
/*        int childCount = xRoot.getAccessibleContext().getAccessibleChildCount();
        if (childCount > 3) {// too many children: wrong type
            throw new StatusException(Status.failed("Could not get the Range Descriptor"));
        }
        XAccessible xAcc = null;
        try {
            xAcc = xRoot.getAccessibleContext().getAccessibleChild(1);
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {

        }
        accTools.printAccessibleTree(log, xAcc);
*/
        // open a new range selection
        props[0].Value = "C4:E6";
        oObj.startRangeSelection(props);
        util.utils.shortWait(1000);
        props[0].Value = "C2:E3";
        oObj.startRangeSelection(props);
        util.utils.shortWait(1000);

        oObj.startRangeSelection(props);
        util.utils.shortWait(1000);
        oObj.abortRangeSelection();
        aListener.reset();
        System.out.println("Listener called: " + aListener.bAbortCalled);

        tRes.tested("startRangeSelection()", aListener.listenerCalled());
    }

    /**
     * Determine the current top window center and return this as a point.
     * @return a point representing the sheet center.
     */
    protected Point getSheetCenter() {
        log.println("Trying to get AccessibleSpreadsheet");
        AccessibilityTools at = new AccessibilityTools();
        XComponent xSheetDoc = (XComponent) tEnv.getObjRelation("DOCUMENT");

        XModel xModel = UnoRuntime.queryInterface(XModel.class, xSheetDoc);
        System.out.println("Name: " + xModel.getCurrentController().getFrame().getName());

        XWindow xWindow = at.getCurrentWindow((XMultiServiceFactory)tParam.getMSF(), xModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.getAccessibleObjectForRole(xRoot, AccessibleRole.TABLE  );

        XAccessibleComponent AccessibleSpreadsheet = UnoRuntime.queryInterface(XAccessibleComponent.class,AccessibilityTools.SearchedContext);

        log.println("Got " + util.utils.getImplName(AccessibleSpreadsheet));

        Object toolkit = null;

        try {
            toolkit = ((XMultiServiceFactory)tParam.getMSF()).createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        XExtendedToolkit tk = UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, toolkit);

        XTopWindow tw = null;

        int k = tk.getTopWindowCount();
        for (int i=0;i<k;i++) {
            try {
                XTopWindow tw_temp = tk.getTopWindow(i);
                XAccessible xacc = UnoRuntime.queryInterface(XAccessible.class,  tw_temp);
                if (xacc != null) {
                    if (xacc.getAccessibleContext().getAccessibleName().indexOf("d2")>0) {
                        tw=tw_temp;
                    }
                } else {
                    log.println("\t unknown window");
                }

            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            }
        }
        if (tw == null) {
            System.out.println("No TopWindow :-(");
            return null;
        }

        Point point = AccessibleSpreadsheet.getLocationOnScreen();
        Rectangle rect = AccessibleSpreadsheet.getBounds();
        Point retPoint = new Point();
        retPoint.X = point.X + (rect.Width / 2);
        retPoint.Y = point.Y + (rect.Height / 2);
        return retPoint;
    }

    /**
     * Get the closer button on the right top of the current window.
     * @return A point representing the closer button.
     */
    private Point getCloser(Point center) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        Object aToolkit = null;
        try {
            aToolkit = xMSF.createInstance("com.sun.star.awt.Toolkit");
        }
        catch(com.sun.star.uno.Exception e) {
            throw new StatusException("Could not create 'com.sun.star.awt.Toolkit'.", e);
        }
        XExtendedToolkit xExtendedToolkit = UnoRuntime.queryInterface(XExtendedToolkit.class, aToolkit);
        XTopWindow tw = null;

        XAccessibleComponent xAccessibleComponent = null;
        int k = xExtendedToolkit.getTopWindowCount();
        for (int i=0;i<k;i++) {
            try {
                XTopWindow tw_temp = xExtendedToolkit.getTopWindow(i);
                XAccessible xacc = UnoRuntime.queryInterface(XAccessible.class,  tw_temp);
                if (xacc != null) {
                    System.out.println("Name: " + xacc.getAccessibleContext().getAccessibleName());
                    if (xacc.getAccessibleContext().getAccessibleName().startsWith("the title")) {
                        tw = tw_temp;
                        XAccessibleContext xContext = xacc.getAccessibleContext();
                        xAccessibleComponent = UnoRuntime.queryInterface(XAccessibleComponent.class, xContext);
                        if (xAccessibleComponent == null)
                            System.out.println("!!!! MIST !!!!");
                        else
                            System.out.println("########## KLAPPT ########## ");
                    }
                }
                else {
                    log.println("\t unknown window");
                }

            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            }
        }
        if (tw == null) {
            System.out.println("No TopWindow :-(");
            return null;
        }

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, tw);
        Rectangle posSize = xWindow.getPosSize();

        // compare the center point with the dimensions of the current top window
        boolean windowOK = false;
        while(!windowOK) {
            if (posSize.X <= center.X && center.X <= posSize.X + posSize.Width) {
                if (posSize.Y <= center.Y && center.Y <= posSize.Y +posSize.Height) {
                    // move window out of the way
                    posSize.X = posSize.X + 10;
                    posSize.Y = posSize.Y +10;
                    xWindow.setPosSize(posSize.X, posSize.Y, posSize.Width, posSize.Height, PosSize.POS);
                }
                else {
                    windowOK = true;
                }
            }
            else {
                windowOK = true;
            }

        }

        Point p = xAccessibleComponent.getLocationOnScreen();
//        System.out.println("ScreenPoint: " + p.X + "   " + p.Y );
//        System.out.println("WindowPoint: " + posSize.X + "   " + posSize.Y + "   " + posSize.Width + "   " + posSize.Height);
        Point closer = new Point();
        closer.X = p.X + posSize.Width - 2;
        closer.Y = p.Y + 5;
        System.out.println("Closer: " + closer.X + "   " + closer.Y);
        return closer;
    }

    protected boolean clickOnSheet(Point point) {
        log.println("Clicking in the center of the AccessibleSpreadsheet");

        try {
            Robot rob = new Robot();
            rob.mouseMove(point.X, point.Y);
            rob.mousePress(InputEvent.BUTTON1_MASK);
            util.utils.shortWait(1000);
            rob.mouseRelease(InputEvent.BUTTON1_MASK);
            util.utils.shortWait(1000);
        } catch (java.awt.AWTException e) {
            log.println("couldn't press mouse button");
        }

        return true;
    }

    /**
     *
     */
    public static class MyRangeSelectionListener implements XRangeSelectionListener, XRangeSelectionChangeListener {
        boolean bAbortCalled = false;
        boolean bChangeCalled = false;
        boolean bDoneCalled = false;
        PrintWriter log = null;

        public MyRangeSelectionListener(PrintWriter log) {
            this.log = log;
        }

        public void aborted(RangeSelectionEvent rangeSelectionEvent) {
            log.println("Called 'aborted' with: " + rangeSelectionEvent.RangeDescriptor);
            bAbortCalled = true;
        }

        public void descriptorChanged(RangeSelectionEvent rangeSelectionEvent) {
            log.println("Called 'descriptorChanged' with: " + rangeSelectionEvent.RangeDescriptor);
            bChangeCalled = true;
        }

        public void done(RangeSelectionEvent rangeSelectionEvent) {
            log.println("Called 'done' with: " + rangeSelectionEvent.RangeDescriptor);
            bDoneCalled = true;
        }

        public boolean listenerCalled() {
            return bAbortCalled & bChangeCalled & bDoneCalled;
        }

        public void reset() {
            bAbortCalled = false;
            bChangeCalled = false;
            bDoneCalled = false;
        }

        /**
         * ignore disposing
         * @param eventObject The event.
         */
        public void disposing(EventObject eventObject) {
        }
    }
}
