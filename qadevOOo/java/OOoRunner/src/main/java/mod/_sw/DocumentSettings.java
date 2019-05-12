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


package mod._sw;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.XInterface;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;
import util.WriterTools;

import java.io.PrintWriter;


public class DocumentSettings extends TestCase
{
    XTextDocument xTextDoc = null;

    /**
    * Creates text document.
    */
    protected void initialize(TestParameters tParam, PrintWriter log)
    {
        log.println("    opening xTextDoc ");
        xTextDoc =
            WriterTools.createTextDoc((XMultiServiceFactory) tParam.getMSF());
    }

    /**
    * Disposes text document.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log)
    {
        log.println("    disposing xTextDoc ");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    protected TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log
    )
    {
        SOfficeFactory SOF =
            SOfficeFactory.getFactory((XMultiServiceFactory) tParam.getMSF());
        XInterface oObj = null;

        log.println("creating a test environment");

        oObj =
            (XInterface) SOF.createInstance(
                xTextDoc, "com.sun.star.text.DocumentSettings"
            );

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    }
}
