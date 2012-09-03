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

import drafts.com.sun.star.script.framework.storage.XScriptStorageManager;
import drafts.com.sun.star.script.framework.storage.XScriptInfoAccess;

import java.util.Iterator;
import java.util.Collection;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Exception;

import java.io.PrintWriter;
import lib.MultiMethodTest;
import lib.StatusException;
import lib.Parameters;

public class _XScriptStorageManager extends MultiMethodTest {

    public XScriptStorageManager oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _createScriptStorage() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_createScriptStorage");

        if (c == null) {
            tRes.tested("createScriptStorage()", false);
            return;
        }

        Iterator tests = c.iterator();

        while (tests.hasNext()) {
            Parameters testdata = (Parameters)tests.next();
            String expected = testdata.get("expected");
            String output = "";

            log.println(testdata.get("description"));

            XSimpleFileAccess access = getXSimpleFileAccess();

            if (access == null) {
                output = "Couldn't create XSimpleFileAccess";
            }
            else {
                try {
                    int id = oObj.createScriptStorage(access);
                    output = "success";
                }
                catch (com.sun.star.uno.RuntimeException re) {
                    log.println("Exception from createScriptStorage: " + re);
                    output = "com.sun.star.uno.RuntimeException";
                }
            }
            log.println("expected: " + expected + ", output: " + output);
            result &= output.equals(expected);
        }
        tRes.tested("createScriptStorage()", result);
    }

    public void _createScriptStorageWithURI() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_createScriptStorageWithURI");

        if (c == null) {
            tRes.tested("createScriptStorageWithURI()", false);
            return;
        }

        Iterator tests = c.iterator();

        while (tests.hasNext()) {
            Parameters testdata = (Parameters)tests.next();
            String expected = testdata.get("expected");
            String location = testdata.get("location");
            String output = "";

            log.println(testdata.get("description"));

            String uri = util.utils.getFullTestURL(location);
            XSimpleFileAccess access = getXSimpleFileAccess();

            try {
                int id = oObj.createScriptStorageWithURI(access, uri);

                XInterface ifc = (XInterface)oObj.getScriptStorage(id);

                if (ifc == null)
                    output = "null";
                else {
                    Object info = UnoRuntime.queryInterface(
                        XScriptInfoAccess.class, ifc);

                    if (info == null)
                        output = "null";
                    else
                        output = "XScriptInfoAccess.class";
                }
            }
            catch (com.sun.star.uno.RuntimeException re) {
                log.println("Caught RuntimeException: " + re);
                output = "com.sun.star.uno.RuntimeException";
            }
            log.println("expected: " + expected + ", output: " + output);
            result &= output.equals(expected);
        }

        tRes.tested("createScriptStorageWithURI()", result);
    }

    public void _getScriptStorage() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getScriptStorage");

        if (c == null) {
            tRes.tested("getScriptStorage()", false);
            return;
        }

        Iterator tests = c.iterator();

        while (tests.hasNext()) {
            Parameters testdata = (Parameters)tests.next();
            String expected = testdata.get("expected");
            String location = testdata.get("location");
            String output = "";

            log.println(testdata.get("description"));

            try {
                int storageid = getStorageId(location);

                XInterface ifc = (XInterface)oObj.getScriptStorage(storageid);

                if (ifc == null)
                    output = "null";
                else {
                    Object info = UnoRuntime.queryInterface(
                        XScriptInfoAccess.class, ifc);

                    if (info == null)
                        output = "null";
                    else
                        output = "XScriptInfoAccess.class";
                }
            }
            catch (com.sun.star.uno.RuntimeException re) {
                log.println("Caught RuntimeException: " + re);
                output = "com.sun.star.uno.RuntimeException";
            }
            log.println("expected: " + expected + ", output: " + output);
            result &= output.equals(expected);
        }
        tRes.tested("getScriptStorage()", result);
    }

    public void _refreshScriptStorage() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_refreshScriptStorage");

        if (c == null) {
            tRes.tested("refreshScriptStorage()", false);
            return;
        }

        Iterator tests = c.iterator();

        while (tests.hasNext()) {
            Parameters testdata = (Parameters)tests.next();
            String expected = testdata.get("expected");
            String location = testdata.get("location");
            String output = "";

            log.println(testdata.get("description"));

            try {
                String uri = util.utils.getFullTestURL(location);
                log.println("calling refreshScriptStorage with URI: " + uri);
                oObj.refreshScriptStorage(uri);
                output = "success";
            }
            catch (com.sun.star.uno.RuntimeException re) {
                log.println("Caught RuntimeException: " + re);
                output = "com.sun.star.uno.RuntimeException";
            }
            log.println("expected: " + expected + ", output: " + output);
            result &= output.equals(expected);
        }
        tRes.tested("refreshScriptStorage()", result);
    }

    private int getStorageId(String location) {

        if (location.equals("share"))
            return 0;

        if (location.equals("user"))
            return 1;

        String uri = util.utils.getFullTestURL(location);

        XSimpleFileAccess access = getXSimpleFileAccess();
        if (access == null)
            return -1;

        return oObj.createScriptStorageWithURI(access, uri);
    }

    private XSimpleFileAccess getXSimpleFileAccess() {
        XSimpleFileAccess access = null;

        try {
            Object fa = ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                "com.sun.star.ucb.SimpleFileAccess");

            access = (XSimpleFileAccess)
                UnoRuntime.queryInterface(XSimpleFileAccess.class, fa);
        }
        catch (com.sun.star.uno.Exception e) {
            return null;
        }
        return access;
    }
}
