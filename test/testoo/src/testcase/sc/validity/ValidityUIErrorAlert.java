/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testcase.sc.validity;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;


import testlib.CalcUtil;
import testlib.Log;


public class ValidityUIErrorAlert {
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
     * test Data -> Validity - UI(Error Alert: Default status)
     */
    @Test
    public void testValidityUIErrorAlert() {
        startcenter.menuItem("File->New->Spreadsheet").select();
        sleep(1);
        calc.menuItem("Data->Validity...").select();
        sleep(1);
        SC_ValidityErrorAlertTabPage.select();
        assertEquals(true,SC_ValidityErrorAlertActionList.isEnabled());
        assertEquals("Stop",SC_ValidityErrorAlertActionList.getSelText());
        assertEquals(true,SC_ValidityErrorMessageTitle.isEnabled());
        assertEquals("",SC_ValidityErrorMessageTitle.getText());
        assertEquals(true,SC_ValidityErrorMessage.isEnabled());
        assertEquals("",SC_ValidityErrorMessage.getText());
        SC_ValidityErrorAlertActionList.select("Macro");
        assertEquals(true,SC_ValidityErrorBrowseButton.isEnabled());

    }

}

