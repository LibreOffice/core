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

package ifc.frame;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.utils;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XDispatchRecorder;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;

/**
* Testing <code>com.sun.star.frame.XDispatchRecorder</code>
* interface methods:
* <ul>
*  <li><code> startRecording() </code></li>
*  <li><code> recordDispatch() </code></li>
*  <li><code> recordDispatchAsComment() </code></li>
*  <li><code> endRecording() </code></li>
*  <li><code> getRecordedMacro() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.frame.XDispatchRecorder
*/
public class _XDispatchRecorder extends MultiMethodTest {
    public static XDispatchRecorder oObj = null;

    XFrame xFrame = null;
    URL dispURL = null;
    PropertyValue[] dispArgs = new PropertyValue[0];

    protected void before() {
        xFrame = (XFrame) tEnv.getObjRelation("XDispatchRecorder.Frame");

        if (xFrame == null) {
            throw new StatusException(Status.failed("Relation not found"));
        }
    }

    public void _startRecording() {

        oObj.startRecording(xFrame);
        oObj.endRecording();
        tRes.tested("startRecording()", true);
    }

    public void _getRecordedMacro() {

        clearBuffer();

        oObj.startRecording(xFrame);

        log.println("dispatch calles via API must be recorded");
        dispURL = utils.parseURL((XMultiServiceFactory)tParam.getMSF(), ".uno:InsertText");

        PropertyValue prop = new PropertyValue();
        prop.Name = "Text";
        prop.Value = "XDispatchRecorder.endRecording()";
        dispArgs = new PropertyValue[] {prop};

        log.println("Dispatching event for frame ...");
        XDispatchProvider xDispProv = (XDispatchProvider)
            UnoRuntime.queryInterface(XDispatchProvider.class, xFrame);
        XDispatch xDisp = xDispProv.queryDispatch(dispURL, "", 0);
        xDisp.dispatch(dispURL, dispArgs);

        shortWait();

        log.println("Getting macro ... :");
        String macro = oObj.getRecordedMacro();
        log.println(macro);

        log.println("Ending record ...");
        oObj.endRecording();

        boolean res = macro != null
                   && macro.indexOf(dispURL.Complete) > -1;
        if (!res) log.println("Dispatch URL '" + dispURL.Complete
            + "' was NOT found in macro - FAILED");

        tRes.tested("getRecordedMacro()", res);
    }

    public void _endRecording() {

        oObj.startRecording(xFrame);
        oObj.endRecording();
        String macro = oObj.getRecordedMacro();
        boolean res = true;
        if (macro.length() != 0){
            log.println("'endRecording()' was called but macro buffer is not cleared: FALSE");
            log.println(macro);
            res = false;
        }

        tRes.tested("endRecording()", res);
    }

    public void _recordDispatch() {
        clearBuffer();

        oObj.startRecording(xFrame);

        // positve test
        dispURL = utils.parseURL((XMultiServiceFactory)tParam.getMSF(), ".uno:InsertText");

        PropertyValue prop = new PropertyValue();
        prop.Name = "Text";
        prop.Value = "XDispatchRecorder.recordDispatch()";
        dispArgs = new PropertyValue[] {prop};

        log.println("Recording dispatch ...");
        oObj.recordDispatch(dispURL, dispArgs);

        log.println("Getting macro ... :");
        String macro = oObj.getRecordedMacro();
        log.println(macro);

        oObj.endRecording();

        boolean res = macro != null
                   && macro.indexOf(dispURL.Complete) > -1
                   && macro.indexOf((String)dispArgs[0].Value) > -1;
        if (!res) log.println("Dispatch URL '" + dispURL.Complete
            + "' or its argument '" + dispArgs[0].Value
            + "' was not found in macro returned - FAILED");

        tRes.tested("recordDispatch()", res);
    }

    public void _recordDispatchAsComment() {
        clearBuffer();

        oObj.startRecording(xFrame);

        dispURL = utils.parseURL((XMultiServiceFactory)tParam.getMSF(), ".uno:InsertText");

        PropertyValue prop = new PropertyValue();
        prop.Name = "Text";
        prop.Value = "XDispatchRecorder.recordDispatchAsComment()";
        dispArgs = new PropertyValue[] {prop};

        log.println("Recording dispatch ...");
        oObj.recordDispatchAsComment(dispURL, dispArgs);

        log.println("Getting macro ... :");
        String macro = oObj.getRecordedMacro();
        log.println(macro);

        oObj.endRecording();

        boolean res = macro != null
                   && macro.indexOf(dispURL.Complete) > -1
                   && macro.indexOf((String)dispArgs[0].Value) > -1;
        if (!res) log.println("Dispatch URL '" + dispURL.Complete
            + "' or its argument '" + dispArgs[0].Value
            + "' was not found in macro returned - FAILED");

        log.println("Checking if macro is commented ...");
        int idx = macro.indexOf((String)dispArgs[0].Value);
        int lineStartIdx = macro.lastIndexOf("\n", idx);
        String lineStart = macro.substring(lineStartIdx + 1, idx).toLowerCase();
        log.println("Line start is '" + lineStart + "'");

        if (lineStart.startsWith("'") || lineStart.startsWith("rem")) {
            log.println("Line is commented : OK");
        } else {
            log.println("Line is not commented : FAILED");
            res =false;
        }

        tRes.tested("recordDispatchAsComment()", res);
    }


    private void shortWait() {
        try {
            Thread.sleep(500);
        } catch (InterruptedException ex) {}
    }

    private void clearBuffer() {
        oObj.startRecording(xFrame);
        oObj.endRecording();
        String macro = oObj.getRecordedMacro();
        if (macro.length() != 0){
            log.println("ERROR: 'endRecording()' was called but macro buffer is not cleared!");
            log.println(macro);
        }
    }

}
