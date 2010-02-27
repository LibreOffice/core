/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package complex.indeterminateState;

import complexlib.ComplexTestCase;
import helper.OfficeProvider;
import util.SOfficeFactory;
import util.AccessibilityTools;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XModel;
import com.sun.star.text.XTextDocument;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleValue;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.awt.XTopWindow;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

import java.io.PrintWriter;

/**
 */
public class CheckIndeterminateState extends ComplexTestCase {

    /**
     * Return all test methods.
     * @return The test methods.
     */
     public String[] getTestMethodNames() {
        return new String[]{"checkToolBoxItem"};
    }

    /*
     * Test the indeterminate state of AccessibleToolBarItem
     * The used tools are in project qadevOOo/runner
     */
    public void checkToolBoxItem() {
        log.println( "creating a test environment" );
        XTextDocument xTextDoc = null;
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory) param.getMSF());

        try {
            log.println( "creating a text document" );
            xTextDoc = SOF.createTextDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( (java.io.PrintWriter)log );
            failed (e.getMessage());
        }

        XModel aModel = (XModel)
                    UnoRuntime.queryInterface(XModel.class, xTextDoc);

        XController xController = aModel.getCurrentController();

        XInterface oObj = null;

        System.out.println("Press any key after making 'Bold' indeterminate.");
        try{
            byte[]b = new byte[16];
            System.in.read(b);
        } catch (Exception e) {
            e.printStackTrace();
        }

        AccessibilityTools at = new AccessibilityTools();
        XWindow xWindow = at.getCurrentContainerWindow((XMultiServiceFactory)param.getMSF(), aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        // uncomment to print the whole accessible tree.
//        at.printAccessibleTree((java.io.PrintWriter)log,xRoot);

        oObj = at.getAccessibleObjectForRole(xRoot,
            AccessibleRole.PUSH_BUTTON, "Bold");
        System.out.println("Found a PUSH_BUTTON: " + (oObj != null));

        oObj = at.getAccessibleObjectForRole(xRoot,
            AccessibleRole.TOGGLE_BUTTON, "Bold");
        System.out.println("Found a TOGGLE_BUTTON: " + (oObj != null));

        log.println("ImplementationName: "+ util.utils.getImplName(oObj));

        XAccessibleContext oContext = (XAccessibleContext)
            UnoRuntime.queryInterface(XAccessibleContext.class, oObj);

        XAccessibleStateSet oSet = oContext.getAccessibleStateSet();

        short[]states = oSet.getStates();
        for(int i=0; i<states.length; i++)
            System.out.println("State "+i+": "+states[i]);

        assure("The 'INDETERMINATE' state is not set.",oSet.contains(AccessibleStateType.INDETERMINATE));
    }

}


