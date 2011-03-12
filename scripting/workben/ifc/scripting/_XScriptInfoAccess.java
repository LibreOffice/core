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

package ifc.script.framework.storage;

import ifc.script.framework.ScriptingUtils;

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
