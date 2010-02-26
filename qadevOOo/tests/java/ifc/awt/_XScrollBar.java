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
package ifc.awt;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.awt.Point;
import com.sun.star.awt.ScrollBarOrientation;
import com.sun.star.awt.XScrollBar;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;

import java.awt.Robot;
import java.awt.event.InputEvent;

import lib.MultiMethodTest;


public class _XScrollBar extends MultiMethodTest {
    public XScrollBar oObj;
    public boolean adjusted = false;
    com.sun.star.awt.XAdjustmentListener listener = new AdjustmentListener();

    public void _addAdjustmentListener() {
        util.FormTools.switchDesignOf((XMultiServiceFactory) tParam.getMSF(),
                                 (XTextDocument) tEnv.getObjRelation("Document"));
        shortWait();
        oObj.addAdjustmentListener(listener);
        adjustScrollBar();

        boolean res = adjusted;
        oObj.removeAdjustmentListener(listener);
        adjusted = false;
        adjustScrollBar();
        res &= !adjusted;
        tRes.tested("addAdjustmentListener()", res);
    }

    public void _removeAdjustmentListener() {
        //this method is checked in addAjustmentListener
        //so that method is requiered here and if it works
        //this method is given OK too
        requiredMethod("addAdjustmentListener()");
        tRes.tested("removeAdjustmentListener()", true);
    }

    public void _setBlockIncrement() {
        oObj.setBlockIncrement(15);
        oObj.setBlockIncrement(5);
        int bi = oObj.getBlockIncrement();
        tRes.tested("setBlockIncrement()",bi==5);
    }

    public void _getBlockIncrement() {
        //this method is checked in the corresponding set method
        //so that method is requiered here and if it works
        //this method is given OK too
        requiredMethod("setBlockIncrement()");
        tRes.tested("getBlockIncrement()", true);
    }

    public void _setLineIncrement() {
        oObj.setLineIncrement(12);
        oObj.setLineIncrement(2);
        int li = oObj.getLineIncrement();
        tRes.tested("setLineIncrement()",li==2);
    }

    public void _getLineIncrement() {
        //this method is checked in the corresponding set method
        //so that method is requiered here and if it works
        //this method is given OK too
        requiredMethod("setLineIncrement()");
        tRes.tested("getLineIncrement()", true);
    }

    public void _setMaximum() {
        oObj.setMaximum(490);
        oObj.setMaximum(480);
        int max = oObj.getMaximum();
        tRes.tested("setMaximum()",max==480);
    }

    public void _getMaximum() {
        //this method is checked in the corresponding set method
        //so that method is requiered here and if it works
        //this method is given OK too
        requiredMethod("setMaximum()");
        tRes.tested("getMaximum()", true);
    }

    public void _setOrientation() {
        oObj.setOrientation(ScrollBarOrientation.HORIZONTAL);
        oObj.setOrientation(ScrollBarOrientation.VERTICAL);
        int ori = oObj.getOrientation();
        tRes.tested("setOrientation()",ori==ScrollBarOrientation.VERTICAL);
    }

    public void _getOrientation() {
        //this method is checked in the corresponding set method
        //so that method is requiered here and if it works
        //this method is given OK too
        requiredMethod("setOrientation()");
        tRes.tested("getOrientation()", true);
    }

    public void _setValue() {
        oObj.setMaximum(600);
        oObj.setValue(480);
        oObj.setValue(520);
        int val = oObj.getValue();
        tRes.tested("setValue()",val==520);
    }

    public void _getValue() {
        //this method is checked in the corresponding set method
        //so that method is requiered here and if it works
        //this method is given OK too
        requiredMethod("setValue()");
        tRes.tested("getValue()", true);
    }

    public void _setVisibleSize() {
        oObj.setVisibleSize(700);
        oObj.setVisibleSize(500);
        int vs = oObj.getVisibleSize();
        tRes.tested("setVisibleSize()",vs==500);
    }

    public void _getVisibleSize() {
        //this method is checked in the corresponding set method
        //so that method is requiered here and if it works
        //this method is given OK too
        requiredMethod("setVisibleSize()");
        tRes.tested("getVisibleSize()", true);
    }

    public void _setValues() {
        oObj.setValues(80, 200, 300);
        oObj.setValues(70, 210, 500);
        int val = oObj.getValue();
        int vs = oObj.getVisibleSize();
        int max = oObj.getMaximum();
        tRes.tested("setValues()",((val==70) && (vs==210) && (max==500)));
    }

    private void adjustScrollBar() {


        XScrollBar sc = (XScrollBar) UnoRuntime.queryInterface(
                                XScrollBar.class, tEnv.getTestObject());

        sc.setValue(500);

        shortWait();

        XAccessible acc = (XAccessible) UnoRuntime.queryInterface(
                                  XAccessible.class, tEnv.getTestObject());

        XAccessibleComponent aCom = (XAccessibleComponent) UnoRuntime.queryInterface(
                                            XAccessibleComponent.class,
                                            acc.getAccessibleContext());

        Point location = aCom.getLocationOnScreen();
        //Point location = (Point) tEnv.getObjRelation("Location");
        //XAccessibleComponent aCom = (XAccessibleComponent) tEnv.getObjRelation("Location");
        //Point location = aCom.getLocationOnScreen();
        try {
            Robot rob = new Robot();
            rob.mouseMove(location.X + 50, location.Y + 75);
            rob.mousePress(InputEvent.BUTTON1_MASK);
            rob.mouseRelease(InputEvent.BUTTON1_MASK);
        } catch (java.awt.AWTException e) {
            System.out.println("couldn't adjust scrollbar");
        }

        shortWait();
    }

    /**
     * Sleeps for 0.5 sec. to allow Office to react
     */
    private void shortWait() {
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            log.println("While waiting :" + e);
        }
    }

    public class AdjustmentListener
        implements com.sun.star.awt.XAdjustmentListener {
        public void adjustmentValueChanged(com.sun.star.awt.AdjustmentEvent adjustmentEvent) {
            System.out.println("Adjustment Value changed");
            System.out.println("AdjustmentEvent: " + adjustmentEvent.Value);
            adjusted = true;
        }

        public void disposing(com.sun.star.lang.EventObject eventObject) {
            System.out.println("Listener disposed");
        }
    }
}
