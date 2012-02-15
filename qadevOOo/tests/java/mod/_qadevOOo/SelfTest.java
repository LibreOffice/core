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


package mod._qadevOOo;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;

import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;

public class SelfTest extends TestCase
{
    private XTextDocument xTextDoc = null;

    protected void initialize(TestParameters tParam, PrintWriter log)
    {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory) tParam.getMSF());

        log.println("initialize the selftest");
        try
        {
            log.println("creating a textdocument");
            xTextDoc = SOF.createTextDoc(null);
        }
        catch (com.sun.star.uno.Exception e)
        {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
     * Disposes text document.
     */
    protected void cleanup(TestParameters tParam, PrintWriter log)
    {
        log.println("    cleanup selftest");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log)
    {
        XInterface oObj = null;
        log.println("creating a test environment");

        // get the bodytext of textdocument here
        log.println("getting the TextCursor");

        final XSimpleText aText = xTextDoc.getText();
        final XTextCursor textCursor = aText.createTextCursor();
        oObj = textCursor;

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    }
}
