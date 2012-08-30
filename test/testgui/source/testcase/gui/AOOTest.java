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

package testcase.gui;

import static org.junit.Assert.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.Logger;

import testlib.gui.CalcUtil;

/**
 * If AOO is not installed in the default directory, please specify the system
 * property in your command line. <br/>
 * -Dopenoffice.home=
 * "Your OpenOffice installation directory which contains soffice.bin"
 *
 */
public class AOOTest {

    /**
     * Add Log to enable the following capabilities. 1. Take a screenshot when
     * failure occurs. 2. Log any detail information.
     */
    @Rule
    public Logger log = Logger.getLogger(this);

    /**
     * Do some setup task before running test
     *
     * @throws Exception
     */
    @Before
    public void setUp() throws Exception {
        // Start OpenOffice with a clean user profile
        app.start(true);
    }

    /**
     * Clean task after testing
     *
     * @throws Exception
     */
    @After
    public void tearDown() throws Exception {

    }

    /**
     * Implement test steps
     */
    @Test
    public void testHello() {
        startcenter.menuItem("File->New->Spreadsheet").select();
        calc.waitForExistence(10, 3);
        typeKeys("Hello");
        assertEquals("Assert", "Hello", CalcUtil.getCellInput("A1"));
    }

}
