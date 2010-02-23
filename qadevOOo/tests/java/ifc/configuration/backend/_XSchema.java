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

package ifc.configuration.backend;

import com.sun.star.configuration.backend.XSchema;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;

import util.XSchemaHandlerImpl;

public class _XSchema extends MultiMethodTest {
    public XSchema oObj;
    XSchemaHandlerImpl xSchemaHandlerImpl = new XSchemaHandlerImpl();
    String filename = null;

    protected void before() {
        filename = (String)tEnv.getObjRelation("ParsedFileName");
    }

    public void _readComponent() {
        requiredMethod("readTemplates()");
        boolean res = false;

        log.println("Checking for Exception in case of null argument");

        try {
            oObj.readComponent(null);
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Expected Exception -- OK");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        }

        log.println(
                "checking readComponent with own XSchemeHandler implementation");

        try {
            xSchemaHandlerImpl.cleanCalls();
            oObj.readComponent(xSchemaHandlerImpl);

            String implCalled = xSchemaHandlerImpl.getCalls();

            System.out.println(implCalled);

            int sc = implCalled.indexOf("startComponent");

            if (sc < 0) {
                log.println("startComponent wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("startComponent was called -- OK");
                res &= true;
            }

            int ec = implCalled.indexOf("endComponent");

            if (ec < 0) {
                log.println("endComponent wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("endComponent was called -- OK");
                res &= true;
            }
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        }

        tRes.tested("readComponent()", res);
        reopenFile();
    }

    public void _readSchema() {
        requiredMethod("readComponent()");
        boolean res = false;

        log.println("Checking for Exception in case of null argument");

        try {
            xSchemaHandlerImpl.cleanCalls();
            oObj.readSchema(null);
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Expected Exception -- OK");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        }

        log.println(
                "checking read data with own XSchemeHandler implementation");

        try {
            xSchemaHandlerImpl.cleanCalls();
            oObj.readSchema(xSchemaHandlerImpl);

            String implCalled = xSchemaHandlerImpl.getCalls();

            int sc = implCalled.indexOf("startSchema");

            if (sc < 0) {
                log.println("startSchema wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("startSchema was called -- OK");
                res &= true;
            }

            int ec = implCalled.indexOf("endSchema");

            if (ec < 0) {
                log.println("endSchema wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("endSchema was called -- OK");
                res &= true;
            }
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        }

        // check for the wrapped target exception
        try {
            xSchemaHandlerImpl.cleanCalls();
            oObj.readSchema(xSchemaHandlerImpl);
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Expected Exception -- OK");
            res = true;
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        }

        tRes.tested("readSchema()", res);
        reopenFile();
    }

    public void _readTemplates() {
        boolean res = false;

        log.println("Checking for Exception in case of null argument");

        try {
            oObj.readTemplates(null);
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Expected Exception -- OK");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        }

        log.println(
                "checking readComponent with own XSchemeHandler implementation");

        try {
            xSchemaHandlerImpl.cleanCalls();
            oObj.readComponent(xSchemaHandlerImpl);

            String implCalled = xSchemaHandlerImpl.getCalls();

            int sc = implCalled.indexOf("startGroup");

            if (sc < 0) {
                log.println("startGroup wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("startGroup was called -- OK");
                res &= true;
            }

            int ec = implCalled.indexOf("endNode");

            if (ec < 0) {
                log.println("endNode wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("endNode was called -- OK");
                res &= true;
            }
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        }

        tRes.tested("readTemplates()", res);
        reopenFile();
    }

    /**
     * reopen the parsed file again, to avoid the wrapped target exception.
     */
    private void reopenFile() {
        XSimpleFileAccess simpleAccess = null;
        XInputStream xStream = null;
        try {
            Object fileacc = ((XMultiServiceFactory)tParam.getMSF()).createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
            simpleAccess = (XSimpleFileAccess)
                            UnoRuntime.queryInterface(XSimpleFileAccess.class,fileacc);
            log.println("Going to parse: "+filename);
            xStream = simpleAccess.openFileRead(filename);
        } catch (com.sun.star.uno.Exception e) {
        }

        XActiveDataSink xSink = (XActiveDataSink) UnoRuntime.queryInterface(XActiveDataSink.class, oObj);
        xSink.setInputStream(xStream);
    }
}