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



package complex.indeterminateState;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.awt.FontWeight;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextViewCursorSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import util.AccessibilityTools;
import util.DesktopTools;
import util.SOfficeFactory;
import static org.junit.Assert.*;

/**
 */
public class CheckIndeterminateState {
    /*
     * Test the indeterminate state of AccessibleToolBarItem
     * The used tools are in project qadevOOo/runner
     */
    @Test public void checkToolBoxItem() throws Exception {
        XModel aModel = (XModel)
                    UnoRuntime.queryInterface(XModel.class, document);

        XController xController = aModel.getCurrentController();

        XText text = document.getText();
        text.setString("normal");
        XTextRange end = text.getEnd();
        end.setString("bold");
        UnoRuntime.queryInterface(XPropertySet.class, end).setPropertyValue(
            "CharWeight", FontWeight.BOLD);
        UnoRuntime.queryInterface(XTextViewCursorSupplier.class, xController).
            getViewCursor().gotoRange(text, false);

        XInterface oObj = null;

        AccessibilityTools at = new AccessibilityTools();
        XWindow xWindow = at.getCurrentContainerWindow(getFactory(), aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        oObj = at.getAccessibleObjectForRole(xRoot,
            AccessibleRole.TOGGLE_BUTTON, "Bold");
        assertNotNull("Found a TOGGLE_BUTTON", oObj);

        XAccessibleContext oContext = (XAccessibleContext)
            UnoRuntime.queryInterface(XAccessibleContext.class, oObj);

        XAccessibleStateSet oSet = oContext.getAccessibleStateSet();

        assertTrue("The 'INDETERMINATE' state is not set.",oSet.contains(AccessibleStateType.INDETERMINATE));
    }

    @Before public void setUpDocument() throws com.sun.star.uno.Exception {
        document = SOfficeFactory.getFactory(getFactory()).createTextDoc(null);
    }

    @After public void tearDownDocument() {
        DesktopTools.closeDoc(document);
    }

    private XTextDocument document = null;

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

    private static final XMultiServiceFactory getFactory() {
        return UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            connection.getComponentContext().getServiceManager());
    }
}
