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

import java.util.HashMap;
import java.util.Iterator;
import java.util.Collection;

import drafts.com.sun.star.script.framework.runtime.XScriptInvocation;
import drafts.com.sun.star.script.framework.storage.XScriptStorageManager;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.frame.XModel;

import lib.MultiMethodTest;
import lib.StatusException;
import lib.Parameters;
import util.SOfficeFactory;

public class _XScriptInvocation extends MultiMethodTest {

    public XScriptInvocation oObj = null;
    private XScriptStorageManager storageManager = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void after() throws StatusException {
    }

    public void _invoke() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_invoke");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                result &= runInvokeTest((Parameters)tests.next());
            }
        }
        else {
            result = false;
        }

        tRes.tested("invoke()", result);
    }

    private boolean runInvokeTest(Parameters testdata) {
        String description = testdata.get("description");
        String logicalname = testdata.get("logicalname");
        String context = testdata.get("context");
        String location = testdata.get("location");
        String expected = testdata.get("expected");
        String output = "";

        int storageId = getStorageId(location);

        XModel ctx = null;
        if (!context.equals("null"))
            ctx = loadDocument(context);

        HashMap map = new HashMap();
        map.put("SCRIPTING_DOC_STORAGE_ID", new Integer(storageId));
        map.put("SCRIPTING_DOC_URI", "hahaha");
        if (ctx != null)
            map.put("SCRIPTING_DOC_REF", ctx);

        Parameters params = new Parameters(map);
        Object[] args = new Object[0];

        Object[][] result = new Object[1][0];
        result[0] = new Object[0];

        short[][] num = new short[1][0];
        num[0] = new short[0];

        log.println(description + ": " + logicalname);

        try {
            Object ret = oObj.invoke(logicalname, params, args, num, result);
            log.println("return type is: " + ret.getClass().getName() +
                ", value is: " + ret.toString());
            output = "success";
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Couldn't invoke script:" + iae);
            output = "com.sun.star.lang.IllegalArgumentException";
        }
        catch (com.sun.star.script.CannotConvertException cce) {
            log.println("Couldn't invoke script:" + cce);
            output = "com.sun.star.script.CannotConvertException";
        }
        catch (com.sun.star.reflection.InvocationTargetException ite) {
            log.println("Couldn't invoke script:" + ite);
            output = "com.sun.star.reflection.InvocationTargetException";
        }
        catch (com.sun.star.uno.RuntimeException re) {
            log.println("Couldn't invoke script:" + re);
            output = "com.sun.star.uno.RuntimeException";
        }

        if (ctx != null)
            ctx.dispose();

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

    private XModel loadDocument(String name) {
        XModel model = null;
        SOfficeFactory factory = SOfficeFactory.getFactory(tParam.getMSF());

        String fullname = util.utils.getFullTestURL(name);

        try {
            Object obj = factory.loadDocument(fullname);
            model = (XModel) UnoRuntime.queryInterface(XModel.class, obj);
        }
        catch (com.sun.star.lang.IllegalArgumentException iae) {
            return null;
        }
        catch (Exception e) {
            return null;
        }

        try {
            Thread.sleep(5000);
        }
        catch (InterruptedException ie) {
        }

        return model;
    }
}
