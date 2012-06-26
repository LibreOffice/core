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
package testcase.formula.elements;

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
public class ElementsWindowActive {

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
     * Test elements window active and inactive
     * @throws Exception
     */
    @Test
    public void testElementsWindowActive() throws Exception{

        // New a formula document
        startcenter.menuItem("File->New->Formula").select();
        sleep(3);

        // Check if the "View->Selection" menu is selected (For AOO3.4: View->Elements)
        math_EditWindow.menuItem("View").select();
        boolean viewElements = math_EditWindow.menuItem("View->Selection").isSelected();

        // If the menu is selected, the Elements window should display
        assertEquals("Elements window displays correctly", viewElements, math_ElementsWindow.exists());

        // Active or inactive the Elements window, it should display correctly
        math_EditWindow.menuItem("View->Selection").select();
        sleep(1);
        math_EditWindow.menuItem("View").select();
        viewElements = math_EditWindow.menuItem("View->Selection").isSelected();
        assertEquals("Elements window display correctly", viewElements, math_ElementsWindow.exists());
    }
}
