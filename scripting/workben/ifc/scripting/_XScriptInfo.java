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

import drafts.com.sun.star.script.framework.storage.XScriptInfo;

import com.sun.star.beans.XPropertySet;

import java.util.Collection;
import java.util.Iterator;

import lib.Parameters;
import lib.MultiMethodTest;
import lib.StatusException;

public class _XScriptInfo extends MultiMethodTest {

    public XScriptInfo oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _getLogicalName() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getLogicalName");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                Parameters testdata = (Parameters)tests.next();
                String expected = testdata.get("expected");
                String output = "";

                log.println(testdata.get("description"));

                output = oObj.getLogicalName();

                log.println("expected: " + expected + ", output: " + output);
                result &= output.equals(expected);
            }
        }
        else {
            result = false;
        }

        tRes.tested("getLogicalName()", result);
    }

    public void _getParcelURI() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getParcelURI");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                Parameters testdata = (Parameters)tests.next();
                String expected = testdata.get("expected");
                String output = "";

                log.println(testdata.get("description"));

                output = oObj.getParcelURI();

                log.println("expected: " + expected + ", output: " + output);
                result &= output.endsWith(expected);
            }
        }
        else {
            result = false;
        }
        tRes.tested("getParcelURI()", result);
    }

    public void _getLanguage() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getLanguage");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                Parameters testdata = (Parameters)tests.next();
                String expected = testdata.get("expected");
                String output = "";

                log.println(testdata.get("description"));

                output = oObj.getLanguage();

                log.println("expected: " + expected + ", output: " + output);
                result &= output.equals(expected);
            }
        }
        else {
            result = false;
        }
        tRes.tested("getLanguage()", result);
    }

    public void _getFunctionName() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getFunctionName");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                Parameters testdata = (Parameters)tests.next();
                String expected = testdata.get("expected");
                String output = "";

                log.println(testdata.get("description"));

                output = oObj.getFunctionName();

                log.println("expected: " + expected + ", output: " + output);
                result &= output.equals(expected);
            }
        }
        else {
            result = false;
        }
        tRes.tested("getFunctionName()", result);
    }

    public void _getLanguageProperties() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getLanguageProperties");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                Parameters testdata = (Parameters)tests.next();
                String expected = testdata.get("expected");
                String output = "";

                log.println(testdata.get("description"));

                try {
                    XPropertySet langProps = oObj.getLanguageProperties();
                    output = (String)langProps.getPropertyValue("classpath");

                    if (output == null)
                        output = "null";
                }
                catch( com.sun.star.uno.Exception e) {
                    log.println("caught UNO Exception:" + e);
                    output = "com.sun.star.uno.Exception";
                }

                log.println("expected: " + expected + ", output: " + output);
                result &= output.equals(expected);
            }
        }
        else {
            result = false;
        }
        tRes.tested("getLanguageProperties()", true);
    }

    public void _getFileSetNames() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getFileSetNames");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                Parameters testdata = (Parameters)tests.next();
                String expected = testdata.get("expected");
                String output = "";

                log.println(testdata.get("description"));

                String[] fileSets = oObj.getFileSetNames();

                if (fileSets == null)
                    output = "null";
                else if (fileSets.length != 1)
                    output = "WrongNumberOfFileSets";
                else
                    output = fileSets[0];

                log.println("expected: " + expected + ", output: " + output);
                result &= output.equals(expected);
            }
        }
        else {
            result = false;
        }
        tRes.tested("getFileSetNames()", result);
    }

    public void _getFilesInFileSet() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getFilesInFileSet");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                Parameters testdata = (Parameters)tests.next();
                String expected = testdata.get("expected");
                String output = "";

                log.println(testdata.get("description"));

                String[] filesInFileSet =
                    oObj.getFilesInFileSet(oObj.getFileSetNames()[0]);

                if (filesInFileSet == null)
                    output = "null";
                else if (filesInFileSet.length != 1)
                    output = "WrongNumberOfFilesInFileSet";
                else
                    output = filesInFileSet[0];

                log.println("expected: " + expected + ", output: " + output);
                result &= output.equals(expected);
            }
        }
        else {
            result = false;
        }
        tRes.tested("getFilesInFileSet()", result);
    }

    public void _getDescription() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_getDescription");

        Iterator tests;

        if (c != null) {
            tests = c.iterator();

            while (tests.hasNext()) {
                Parameters testdata = (Parameters)tests.next();
                String expected = testdata.get("expected");
                String output = "";

                log.println(testdata.get("description"));

                output = oObj.getDescription();

                if (output == null)
                    output = "null";
                else if (output.length() == 0)
                    output = "empty";

                log.println("expected: [" + expected + "], output: [" +
                    output + "]");
                result &= output.equals(expected);
            }
        }
        else {
            result = false;
        }

        tRes.tested("getDescription()", result);
    }
}
