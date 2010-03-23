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
            Object fa = tParam.getMSF().createInstance(
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
