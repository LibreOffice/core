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

package ifc.configuration.backend;

import com.sun.star.configuration.backend.XSchema;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;

import util.XSchemaHandlerImpl;

public class _XSchema extends MultiMethodTest {
    public XSchema oObj;
    XSchemaHandlerImpl xSchemaHandlerImpl = new XSchemaHandlerImpl();
    String filename = null;

    @Override
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
            Object fileacc = tParam.getMSF().createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
            simpleAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class,fileacc);
            log.println("Going to parse: "+filename);
            xStream = simpleAccess.openFileRead(filename);
        } catch (com.sun.star.uno.Exception e) {
        }

        XActiveDataSink xSink = UnoRuntime.queryInterface(XActiveDataSink.class, oObj);
        xSink.setInputStream(xStream);
    }
}