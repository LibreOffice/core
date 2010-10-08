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

package ifc.script.framework.provider;

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
