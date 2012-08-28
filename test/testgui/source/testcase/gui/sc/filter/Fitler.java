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



/**
 *
 */
package testcase.gui.sc.filter;

import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;

import testlib.gui.CalcUtil;
import testlib.gui.Log;

/**
 * Test cases about Data->Filter in spreadsheet
 */
public class Fitler {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start(true);

        // Create a new spreadsheet document
        app.dispatch("private:factory/scalc");
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     *
     * Verify 2+ won't be treated as 2
     */
    @Ignore("Bug 120076")
    public void testAutoFilterWithPlusSign() {
        String expect ="2+";
        CalcUtil.selectRange("A1");
        typeKeys(expect + "<enter>");
        assertEquals(expect,CalcUtil.getCellText("A1"));
    }
}
