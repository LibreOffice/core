/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package ifc.script.framework.storage;

import drafts.com.sun.star.script.framework.storage.XScriptStorageRefresh;

import java.util.Collection;
import java.util.Iterator;

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

public class _XScriptStorageRefresh extends MultiMethodTest {

    public XScriptStorageRefresh oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _refresh() {
        boolean result = true;

        Collection c =
            (Collection) tEnv.getObjRelation("_refresh");

        if (c == null) {
            tRes.tested("refresh()", false);
            return;
        }

        Iterator tests = c.iterator();

        while (tests.hasNext()) {
            Parameters testdata = (Parameters)tests.next();
            String expected = testdata.get("expected");
            String output = "";

            log.println(testdata.get("description"));

            try {
                oObj.refresh();
                output = "success";
            }
            catch (com.sun.star.uno.RuntimeException re) {
                log.println("Caught RuntimeException: " + re);
                output = "com.sun.star.uno.RuntimeException";
            }
            log.println("expected: " + expected + ", output: " + output);
            result &= output.equals(expected);
        }
        tRes.tested("refresh()", result);
    }
}
