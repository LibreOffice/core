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



package mod._ucpdav;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Creates a <code>com.sun.star.ucb.WebDAVContentProvider</code> instance.
 */
public class WebDAVContentProvider extends TestCase {
    /**
     * Creates a <code>com.sun.star.ucb.WebDAVContentProvider</code> instance
     * and adds a <code>XContentIdentifierFactory</code> instance as a "FACTORY"
     * relation for <code>XContentProviderTest</code>.
     */
    public TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface xInt;

        log.println("creating WebDAVContentProvider instance");
        try {
            xInt = (XInterface)xMSF.createInstance(
                    "com.sun.star.ucb.WebDAVContentProvider");
        } catch (Exception e) {
            log.println("Unexpected exception " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        TestEnvironment tEnv = new TestEnvironment(xInt);

        log.println("creating XContentIdentifierFactory");
        XContentIdentifierFactory xCIF;

        try {
            xCIF = (XContentIdentifierFactory)UnoRuntime.queryInterface(
                    XContentIdentifierFactory.class,
                    xMSF.createInstance(
                            "com.sun.star.comp.ucb.UniversalContentBroker"));
        } catch (Exception e) {
            log.println("Unexpected exception " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        tEnv.addObjRelation("FACTORY", xCIF);

        tEnv.addObjRelation("CONTENT1",
            "vnd.sun.star.webdav://localhost/davhome/");

        return tEnv;
    }
}
