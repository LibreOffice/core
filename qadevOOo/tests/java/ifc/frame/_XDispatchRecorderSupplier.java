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

package ifc.frame;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XDispatchRecorder;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.frame.XDispatchRecorderSupplier;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import lib.MultiMethodTest;
import lib.StatusException;
import util.SOfficeFactory;
import util.utils;

/**
* Testing <code>com.sun.star.frame.XDispatchRecorderSupplier</code>
* interface methods:
* <ul>
*  <li><code> setDispatchRecorder() </code></li>
*  <li><code> getDispatchRecorder() </code></li>
*  <li><code> dispatchAndRecord() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.frame.XDispatchRecorderSupplier
*/
public class _XDispatchRecorderSupplier extends MultiMethodTest {
    public static XDispatchRecorderSupplier oObj = null;

    XComponent xTextDoc = null;
    XDispatchRecorder recorder = null;
    XDesktop desktop = null;

    /**
     * Simple <code>XDispatchRecorder</code> implementation
     * which method <code>getRecordedMacro</code> returns a fixed
     * string.
     */
    private static class MyRecorder implements XDispatchRecorder {
        public void startRecording(XFrame p0) {}
        public void recordDispatch(URL p0, PropertyValue[] p1) {}
        public void recordDispatchAsComment(URL p0, PropertyValue[] p1) {}
        public void endRecording(){}
        public String getRecordedMacro() {
            return "MyRecorder implementation";
        }
    }

    /**
     * Creates a new document which supplies a frame.
     * Also a <code>com.sun.star.frame.Desktop</code>
     * service created for obtaining document's frame.
     */
    protected void before() {
        SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory) tParam.getMSF());

        try {
            log.println( "creating a text document" );
            xTextDoc = SOF.createTextDoc(null);

            Object inst = ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.frame.Desktop");
            desktop = UnoRuntime.queryInterface
                (XDesktop.class, inst);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

    }

    /**
     * Creates an instance of <code>MyRecorder</code> and set if,
     * then get the current recorder. Second case is setting
     * recorder to null. Finally restores the old macro recorder. <p>
     *
     * Has <b>OK</b> status if in the first case custom recorder
     * was successfully returned, and in second case current
     * recorder is null.
     */
    public void _setDispatchRecorder() {
        requiredMethod("getDispatchRecorder()");

        boolean res = true,
                locRes = true;
        log.print("Setting custom macro recorder ...");
        oObj.setDispatchRecorder(new MyRecorder());
        XDispatchRecorder rec = oObj.getDispatchRecorder();

        locRes = rec != null &&
            "MyRecorder implementation".equals(rec.getRecordedMacro());
        if (locRes) log.println("OK");
        else log.println("FAILED");
        res &= locRes;

        log.print("Setting null dispatch recorder ...");
        oObj.setDispatchRecorder(null);
        locRes = oObj.getDispatchRecorder() == null;
        if (locRes) log.println("OK");
        else log.println("FAILED");
        res &= locRes;

        log.println("Setting old macro recorder ...");
        oObj.setDispatchRecorder(recorder);

        tRes.tested("setDispatchRecorder()", res);
    }

    /**
     * Just gets the current recorder and stores it.
     *
     * Has <b>OK</b> status.
     */
    public void _getDispatchRecorder() {
        recorder = oObj.getDispatchRecorder();
        tRes.tested("getDispatchRecorder()", true);
    }

    /**
     * First sets the current dispatch recorder to new
     * <code>DispatchRecorder</code> instance if the current one
     * is null. The a <code>Dispatch</code> instance is created
     * which inserts some text into text document.
     * A number of cases is checked :
     * <ul>
     *  <li> A valid call : here the recorded macro must contain
     *      inserted string and URL </li>
     *  <li> Call with invalid URL : the macro recorded must not
     *      contain this URL </li>
     *  <li> Call with null dispatcher : checking for GPF </li>
     *  <li> Call with the current recorder set to null :
     *      checking for GPF </li>
     * </ul>
     *
     * Has <b>OK</b> status if all cases are OK.
     */
    public void _dispatchAndRecord() {
        requiredMethod("getDispatchRecorder()");

        boolean res = true;
        if (recorder == null) {
            try {
                Object inst = ((XMultiServiceFactory) tParam.getMSF()).createInstance
                    ("com.sun.star.comp.framework.DispatchRecorder");
                recorder = UnoRuntime.queryInterface
                    (XDispatchRecorder.class, inst);
                oObj.setDispatchRecorder(recorder);
            } catch (com.sun.star.uno.Exception e) {
                throw new StatusException("Couldn't create recorder", e);
            }
        }

        try {
            Thread.sleep(500);
        } catch (InterruptedException ex) {}

        XModel model = UnoRuntime.queryInterface(XModel.class, xTextDoc);
        XFrame fr = model.getCurrentController().getFrame();

        XDispatchProvider xDispProv = UnoRuntime.queryInterface(XDispatchProvider.class, fr);

        URL dispURL = utils.parseURL((XMultiServiceFactory) tParam.getMSF(), ".uno:InsertText");

        XDispatch xDisp = xDispProv.queryDispatch(dispURL,"",0);

        PropertyValue[] args = new PropertyValue[1];
        args[0] = new PropertyValue();
        args[0].Name = "Text";
        args[0].Value = "XDispatchRecorderSupplier";

        log.print("Dispatching and recording ...");
        oObj.dispatchAndRecord(dispURL, args, xDisp);

        String macro = recorder.getRecordedMacro();
        boolean locRes = macro != null &&
            macro.indexOf("XDispatchRecorderSupplier")>-1 &&
            macro.indexOf(".uno:InsertText")>-1;
        if (locRes) log.println("OK");
        else log.println("FAILED");
        res &= locRes;
        log.println("Recorder macro :\n" + macro);

        log.print("Trying to set dispatch with null Dispatcher ...");
        try {
            oObj.dispatchAndRecord(dispURL, args, null);
            log.println("OK");
        } catch (java.lang.Exception e){
            log.println("Exception is OK: " + e);
        }

        log.print("Trying to set dispatch recorder to null and record ...");
        oObj.setDispatchRecorder(null);
        try {
            oObj.dispatchAndRecord(dispURL, args, xDisp);
            log.println("OK");
        } catch (java.lang.Exception e){
            log.println("Exception is OK: " + e);
        }

        oObj.setDispatchRecorder(recorder);

        tRes.tested("dispatchAndRecord()", res);
    }

    /**
     * Disposes the document created in <code>before()</code>
     */
    protected void after() {
        xTextDoc.dispose();
    }
}