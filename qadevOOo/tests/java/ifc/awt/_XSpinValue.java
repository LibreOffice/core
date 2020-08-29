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

package ifc.awt;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.awt.Point;
import com.sun.star.awt.ScrollBarOrientation;
import com.sun.star.awt.XSpinValue;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import java.awt.Robot;
import java.awt.event.InputEvent;
import lib.MultiMethodTest;

public class _XSpinValue extends MultiMethodTest {

    public XSpinValue oObj;
    public boolean adjusted = false;
    com.sun.star.awt.XAdjustmentListener listener = new AdjustmentListener();

    public void _addAdjustmentListener() throws Exception {
        util.FormTools.switchDesignOf(tParam.getMSF(),
                                 (XTextDocument) tEnv.getObjRelation("Document"));
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
        //this method is checked in addAdjustmentListener
        //so that method is required here and if it works
        //this method is given OK too
        requiredMethod("addAdjustmentListener()");
        tRes.tested("removeAdjustmentListener()", true);
    }

    public void _setSpinIncrement() {
        oObj.setSpinIncrement(15);
        oObj.setSpinIncrement(5);
        int bi = oObj.getSpinIncrement();
        tRes.tested("setSpinIncrement()",bi==5);
    }

    public void _getSpinIncrement() {
        //this method is checked in the corresponding set method
        //so that method is required here and if it works
        //this method is given OK too
        requiredMethod("setSpinIncrement()");
        tRes.tested("getSpinIncrement()", true);
    }


    public void _setMaximum() {
        oObj.setMaximum(490);
        oObj.setMaximum(480);
        int max = oObj.getMaximum();
        tRes.tested("setMaximum()",max==480);
    }

    public void _getMaximum() {
        //this method is checked in the corresponding set method
        //so that method is required here and if it works
        //this method is given OK too
        requiredMethod("setMaximum()");
        tRes.tested("getMaximum()", true);
    }

    public void _setMinimum() {
        oObj.setMinimum(90);
        oObj.setMinimum(80);
        int max = oObj.getMinimum();
        tRes.tested("setMinimum()",max==80);
    }

    public void _getMinimum() {
        //this method is checked in the corresponding set method
        //so that method is required here and if it works
        //this method is given OK too
        requiredMethod("setMinimum()");
        tRes.tested("getMinimum()", true);
    }

    public void _setOrientation() {
        boolean res = true;
        try {
            oObj.setOrientation(ScrollBarOrientation.HORIZONTAL);
            oObj.setOrientation(ScrollBarOrientation.VERTICAL);
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println("Couldn't set Orientation");
        }
        int ori = oObj.getOrientation();
        res &= (ori==ScrollBarOrientation.VERTICAL);
        tRes.tested("setOrientation()",res );
    }

    public void _getOrientation() {
        //this method is checked in the corresponding set method
        //so that method is required here and if it works
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
        //so that method is required here and if it works
        //this method is given OK too
        requiredMethod("setValue()");
        tRes.tested("getValue()", true);
    }

    public void _setValues() {
        oObj.setValues(80, 200, 180);
        oObj.setValues(70, 210, 200);
        int val = oObj.getValue();
        int min = oObj.getMinimum();
        int max = oObj.getMaximum();
        tRes.tested("setValues()",((min==70) && (max==210) && (val==200)));
    }

    private void adjustScrollBar() {


        XSpinValue sv = UnoRuntime.queryInterface(
                                XSpinValue.class, tEnv.getTestObject());

        sv.setValue(500);

        waitForEventIdle();

        XAccessible acc = UnoRuntime.queryInterface(
                                  XAccessible.class, tEnv.getTestObject());

        XAccessibleComponent aCom = UnoRuntime.queryInterface(
                                            XAccessibleComponent.class,
                                            acc.getAccessibleContext());

        Point location = aCom.getLocationOnScreen();
        try {
            Robot rob = new Robot();
            rob.mouseMove(location.X + 20, location.Y + 10);
            rob.mousePress(InputEvent.BUTTON1_MASK);
            rob.mouseRelease(InputEvent.BUTTON1_MASK);
        } catch (java.awt.AWTException e) {
            System.out.println("couldn't adjust scrollbar");
        }

        waitForEventIdle();
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
