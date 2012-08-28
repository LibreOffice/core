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
package testcase.gui.sw.table;

import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;
import static org.junit.Assert.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.SystemUtil;

import testlib.gui.Log;

/**
 *
 */
public class Table {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start(true);
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     * Test convert table to text in text document
     * @throws Exception
     */
    @Test
    public void testConvertTableToText() throws Exception{

        //Create a new text document
        app.dispatch("private:factory/swriter");

        // Insert a table and input some data
        app.dispatch(".uno:InsertTable");
        writer_InsertTable.ok();
        writer.focus();
        typeKeys("1<right>2<right>3<right>4");
        sleep(1);

        // Convert table to text
        app.dispatch(".uno:ConvertTableToText");
        assertTrue("Convert Table to Text dialog pop up", writer_ConvertTableToTextDlg.exists());
//      typeKeys("<enter>");
        writer_ConvertTableToTextDlg.ok();  // "Enter" does not work on linux

        // Verify if text is converted successfully
        app.dispatch(".uno:SelectAll");
        app.dispatch(".uno:Copy");
        if (SystemUtil.isWindows())
            assertEquals("Converted text", "1\t2\r\n3\t4\r\n", app.getClipboard()); // in windows, \n is \r\n
        else
            assertEquals("Converted text", "1\t2\n3\t4\n", app.getClipboard());
    }
}
