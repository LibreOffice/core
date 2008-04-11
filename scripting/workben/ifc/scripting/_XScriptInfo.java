/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XScriptInfo.java,v $
 * $Revision: 1.6 $
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
import drafts.com.sun.star.script.framework.storage.XScriptInfo;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Exception;
import com.sun.star.beans.XPropertySet;

import java.util.Collection;
import java.util.Iterator;

import java.io.PrintWriter;
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
