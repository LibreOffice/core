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

package ifc.script.framework.runtime;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Collection;

import drafts.com.sun.star.script.framework.runtime.XScriptNameResolver;
import drafts.com.sun.star.script.framework.storage.XScriptInfo;
import drafts.com.sun.star.script.framework.storage.XScriptStorageManager;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import lib.MultiMethodTest;
import lib.StatusException;
import lib.Parameters;

public class _XScriptNameResolver extends MultiMethodTest {

    public XScriptNameResolver oObj = null;
    private XScriptStorageManager storageManager = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _resolve() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_resolve");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                result &= runResolveTest((Parameters)tests.next());
            }
        }
        else {
            result = false;
        }

        tRes.tested("resolve()", result);
    }

    private boolean runResolveTest(Parameters data) {
        String description = data.get("description");
        String location = data.get("location");
        String logicalname = data.get("logicalname");
        String expected = data.get("expected");
        String output = "";

        int storageId = getStorageId(location);

        log.println(description + ": " + logicalname);

        HashMap map = new HashMap();
        map.put("SCRIPTING_DOC_STORAGE_ID", new Integer(storageId));
        map.put("SCRIPTING_DOC_URI", util.utils.getFullTestURL(location));

        Parameters params = new Parameters(map);
        Object[] args = new Object[] {params};

        try {
            XInterface ifc = (XInterface) oObj.resolve(logicalname, args);

            if (ifc == null)
                output = "null";
            else if (UnoRuntime.queryInterface(XScriptInfo.class, ifc) == null)
                output = "null";
            else
                output = "XScriptInfo.class";
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("caught IllegalArgumentException: " + iae);
            output = "com.sun.star.lang.IllegalArgumentException";
        }
        catch (com.sun.star.script.CannotConvertException cce) {
            log.println("caught CannotConvertException: " + cce);
            output = "com.sun.star.script.CannotConvertException";
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("caught RuntimeException: " + re);
            output = "com.sun.star.uno.RuntimeException";
        }

        log.println("expected: " + expected + ", output: " + output);
        if (output.equals(expected))
            return true;
        else
            return false;
    }

    private int getStorageId(String location) {

        if (location.equals("share"))
            return 0;

        if (location.equals("user"))
            return 1;

        XSimpleFileAccess access = null;
        String uri = util.utils.getFullTestURL(location);

        if (storageManager == null) {
            try {
                XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, tParam.getMSF());

                XComponentContext xContext = (XComponentContext)
                    UnoRuntime.queryInterface(XComponentContext.class,
                    xProp.getPropertyValue("DefaultContext"));

                XInterface ifc = (XInterface)
                    xContext.getValueByName("/singletons/drafts.com.sun.star." +
                    "script.framework.storage.theScriptStorageManager");

                storageManager = (XScriptStorageManager)
                    UnoRuntime.queryInterface(XScriptStorageManager.class, ifc);
            }
            catch( Exception e ) {
                return -1;
            }
        }

        access = getXSimpleFileAccess();
        if (access == null)
            return -1;

        int id = storageManager.createScriptStorageWithURI(access, uri);

        return id;
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
