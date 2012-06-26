/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

/**
 *
 */
package testcase.sw.table;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.awt.Rectangle;
import java.io.File;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;

import testlib.CalcUtil;
import testlib.Log;

/**
 *
 */
public class ConvertTableToText {

    /**
     * TestCapture helps us to do
     * 1. Take a screenshot when failure occurs.
     * 2. Collect extra data when OpenOffice crashes.
     */
    @Rule
    public Log LOG = new Log();

    /**
     * initApp helps us to do
     * 1. Patch the OpenOffice to enable automation if necessary.
     * 2. Start OpenOffice with automation enabled if necessary.
     * 3. Reset OpenOffice to startcenter.
     *
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        initApp();
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
        writer.click(1, 1);
        typeKeys("1<right>2<right>3<right>4");
        sleep(1);

        // Convert table to text
        writer.menuItem("Table->Convert->Table to Text...").select();
        assertTrue("Convert Table to Text dialog pop up", writer_ConvertTableToTextDlg.exists());
        typeKeys("<enter>");
        sleep(1);

        // Verify if text is converted successfully
        writer.menuItem("Edit->Select All").select();
        typeKeys("<$copy>");
        assertEquals("Converted text", "1   2\n3    4\n", app.getClipboard());
    }
}
