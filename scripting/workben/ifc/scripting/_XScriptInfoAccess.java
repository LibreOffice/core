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

package ifc.scripting;

import drafts.com.sun.star.script.framework.storage.XScriptInfoAccess;
import drafts.com.sun.star.script.framework.storage.XScriptInfo;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Exception;
import com.sun.star.beans.XPropertySet;

import java.io.PrintWriter;
import lib.MultiMethodTest;
import lib.StatusException;
import lib.Parameters;

import java.util.Collection;
import java.util.Iterator;

public class _XScriptInfoAccess extends MultiMethodTest {

    public XScriptInfoAccess oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _getScriptLogicalNames() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getScriptLogicalNames");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                result &= runGetScriptLogicalNamesTest((Parameters)tests.next());
            }
        }
        else {
            result = false;
        }

        tRes.tested("getScriptLogicalNames()", result);
    }

    private boolean runGetScriptLogicalNamesTest(Parameters testdata) {
        String description = testdata.get("description");
        String expected = testdata.get("expected");
        String output = "";

        log.println(testdata.get("description"));

        // try {
            log.println("In _XScriptInfoAccess.getScriptLogicalNames()");
            String[] logicalNames = oObj.getScriptLogicalNames();

            if (logicalNames == null)
                output = "null";
            else if (logicalNames.length == 0)
                output = "empty";
            else {
                for (int i = 0; i < logicalNames.length; i++) {
                    if (logicalNames[i].equals(expected)) {
                        output = logicalNames[i];
                        break;
                    }
                }
            }
        // }
        // catch (com.sun.star.uno.Exception e) {
            // log.println("Caught UNO Exception :" + e);
            // output = "com.sun.star.uno.Exception";
        // }

        log.println("expected: " + expected + ", output: " + output);
        if (output.equals(expected))
            return true;
        else
            return false;
    }

    public void _getImplementations() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getImplementations");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                result &= runGetImplementationsTest((Parameters)tests.next());
            }
        }
        else {
            result = false;
        }

        tRes.tested("getImplementations()", result);
    }

    private boolean runGetImplementationsTest(Parameters testdata) {
        String description = testdata.get("description");
        String logicalname = testdata.get("logicalname");
        String expected = testdata.get("expected");
        String output = "";

        log.println(testdata.get("description"));

    // performs a basic check to see if 1 match (XScriptInfo) is returned
    // the XScriptInfo object is tested more completely in _XScriptInfo
    // which is drive from ScriptInfo

        try {
            XScriptInfo[] impls = oObj.getImplementations(logicalname);

            // should only be one match
            if (impls == null)
                output = "null";
            else if (impls.length == 0)
                output = "empty";
            else
                output = impls[0].getLogicalName();
        }
        catch (com.sun.star.uno.Exception e) {
            log.println("Caught UNO Exception:" + e);
            output = "com.sun.star.uno.Exception";
        }

        log.println("expected: " + expected + ", output: " + output);
        if (output.equals(expected))
            return true;
        else
            return false;
    }

    public void _getAllImplementations() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getAllImplementations");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                result &= runGetAllImplementationsTest((Parameters)tests.next());
            }
        }
        else {
            result = false;
        }

        tRes.tested("getAllImplementations()", result);
    }

    private boolean runGetAllImplementationsTest(Parameters testdata) {
        String description = testdata.get("description");
        String location = testdata.get("location");
        String expected = testdata.get("expected");
        String output = "";

        log.println(testdata.get("description"));

        Object obj = ScriptingUtils.getDefault().getScriptStorage(
            tParam.getMSF(), location);

        XScriptInfoAccess access = (XScriptInfoAccess)
            UnoRuntime.queryInterface(XScriptInfoAccess.class, obj);

        XScriptInfo[] impls = access.getAllImplementations();

        if (impls == null || impls.length == 0) {
            output = "empty";
        }
        else {
            for (int i = 0; i < impls.length - 1; i++)
                output += impls[i].getLogicalName() + ",";
            output += impls[impls.length - 1].getLogicalName();
        }

        log.println("expected: " + expected + ", output: " + output);
        if (output.equals(expected))
            return true;
        else
            return false;
    }
}
