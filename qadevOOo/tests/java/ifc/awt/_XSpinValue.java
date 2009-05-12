/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XSpinValue.java,v $
 * $Revision: 1.4 $
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
import com.sun.star.awt.XSpinValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import java.awt.Robot;
import java.awt.event.InputEvent;
import lib.MultiMethodTest;

public class _XSpinValue extends MultiMethodTest {

    public XSpinValue oObj;
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

    public void _setSpinIncrement() {
        oObj.setSpinIncrement(15);
        oObj.setSpinIncrement(5);
        int bi = oObj.getSpinIncrement();
        tRes.tested("setSpinIncrement()",bi==5);
    }

    public void _getSpinIncrement() {
        //this method is checked in the corresponding set method
        //so that method is requiered here and if it works
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
        //so that method is requiered here and if it works
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
        //so that method is requiered here and if it works
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

    public void _setValues() {
        oObj.setValues(80, 200, 180);
        oObj.setValues(70, 210, 200);
        int val = oObj.getValue();
        int min = oObj.getMinimum();
        int max = oObj.getMaximum();
        tRes.tested("setValues()",((min==70) && (max==210) && (val==200)));
    }

    private void adjustScrollBar() {


        XSpinValue sv = (XSpinValue) UnoRuntime.queryInterface(
                                XSpinValue.class, tEnv.getTestObject());

        sv.setValue(500);

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
            rob.mouseMove(location.X + 20, location.Y + 10);
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
