/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testcase;

import static org.junit.Assert.*;
import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import testlib.CalcUtil;
import testlib.Log;

public class SayHelloToOO {

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
