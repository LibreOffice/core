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
package testcase.sw.table;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;
import static org.junit.Assert.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.SystemUtil;

import testlib.Log;

/**
 *
 */
public class Table {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start();
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }

    /**
     * Test convert table to text in text document
     * @throws Exception
     */
    @Test
    public void testConvertTableToText() throws Exception{

        //Create a new text document
        startcenter.menuItem("File->New->Text Document").select();
        sleep(3);

        // Insert a table and input some data
        writer.menuItem("Insert->Table...").select();
        writer_InsertTable.ok();
        writer.focus();
        typeKeys("1<right>2<right>3<right>4");
        sleep(1);

        // Convert table to text
        writer.menuItem("Table->Convert->Table to Text...").select();
        assertTrue("Convert Table to Text dialog pop up", writer_ConvertTableToTextDlg.exists());
        typeKeys("<enter>");
        sleep(1);

        // Verify if text is converted successfully
        writer.menuItem("Edit->Select All").select();
        app.dispatch(".uno:Copy");
        if (SystemUtil.isWindows())
            assertEquals("Converted text", "1\t2\r\n3\t4\r\n", app.getClipboard()); // windows�лس�������\r\n
        else
            assertEquals("Converted text", "1\t2\n3\t4\n", app.getClipboard());
    }
}
