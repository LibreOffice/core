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



package testcase;

import static org.junit.Assert.*;
import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import testlib.CalcUtil;
import testlib.Log;

/**
 * Before running the testing class, you need specify the AOO location firstly with system property openoffice.home.
 *
 * @author test
 *
 */
public class SayHelloToOO {

    /**
     * TestCapture helps us to do
     * 1. Take a screenshot when failure occurs.
     * 2. Collect extra data when OpenOffice crashes.
     */
    @Rule
    public Log LOG = new Log();


    /**
     * Do some setup task before running test
     * @throws Exception
     */
    @Before
    public void setUp() throws Exception {
        //Start OpenOffice
        app.start();
    }

    /**
     * Clean task after testing
     * @throws Exception
     */
    @After
    public void tearDown() throws Exception {
        //Close OpenOffice
        app.close();
    }

    /**
     * Implement the case
     */
    @Test
    public void helloOO() {
        startcenter.menuItem("File->New->Spreadsheet").select();
        calc.waitForExistence(10, 3);
        typeKeys("Hello");
        assertEquals("Assert", "Hello", CalcUtil.getCellInput("A1"));
    }

}
