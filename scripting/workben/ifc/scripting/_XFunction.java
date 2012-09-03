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

import drafts.com.sun.star.script.framework.provider.XFunction;
import drafts.com.sun.star.script.framework.provider.XFunctionProvider;

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

public class _XFunction extends MultiMethodTest {

    public XFunction oObj = null;
    public XFunctionProvider oProvider = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
        log.println("getting provider");
        oProvider = (XFunctionProvider) tEnv.getObjRelation("provider");
        if (oProvider == null)
            log.println("it's null");
        else
            log.println("it's not null");
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

        String expreturntype = testdata.get("returntype");
        String expreturnvalue = testdata.get("returnvalue");
        String gotreturntype = "null";
        String gotreturnvalue = "null";

        String location = testdata.get("location");

        String expected = testdata.get("expected");
        String output = "";
        boolean result = true;

        log.println(testdata.get("description"));

        try{
            Object[] aParams = new Object[0];
            short[][] aOutParamIndex = new short[1][];
            aOutParamIndex[0] = new short[0];
            Object[][] aOutParam = new Object[1][];
            aOutParam[0] = new Object[0];

            XFunction func = oProvider.getFunction(logicalname);
            if (func == null) {
                log.println("Couldn't get XFunction for:" + logicalname);
                return false;
            }

            Object ret = func.invoke( aParams, aOutParamIndex, aOutParam );

            if (ret != null) {
                gotreturntype = ret.getClass().getName();
                gotreturnvalue = ret.toString();
            }

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
        catch(java.lang.Exception e){
            log.println("Couldn't invoke script:" + e);
            output = "java.lang.Exception";
        }

        if (expreturntype != null) {
            log.println("expected return type: " + expreturntype +
                ", got return type: " + gotreturntype);

            if (!gotreturntype.equals(expreturntype))
                result = false;
        }

        if (expreturnvalue != null) {
            log.println("expected return value: " + expreturnvalue +
                ", got return value: " + gotreturnvalue);

            if (!gotreturnvalue.equals(expreturnvalue))
                result = false;
        }

        log.println("expected: " + expected + ", output: " + output);
        if (!output.equals(expected))
            result = false;

        return result;
    }
}
