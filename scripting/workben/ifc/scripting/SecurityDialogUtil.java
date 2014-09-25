/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package ifc.scripting;

import com.sun.star.awt.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import com.sun.star.accessibility.*;
import com.sun.star.awt.XExtendedToolkit;

// Jsuite classes
import util.AccessibilityTools;
/**
* Thread that pushes the buttons or checkbox
* on the message box that is on top.
*/
public class SecurityDialogUtil extends Thread {

    private XMultiServiceFactory xMSF = null;
    private String errorMsg;
    private String btnName;
    private boolean checkBox;

    /**
     * Constructor.
     */
    public SecurityDialogUtil(XMultiServiceFactory xMSF, String btnName,
                              boolean checkBox) {
        this.xMSF = xMSF;
        this.btnName = btnName;
        this.checkBox = checkBox;
        this.errorMsg = "";
    }

    /**
     * Returns the error message that occurred while
     * accessing and pressing the button.
     * @return Error message.
     */
    public String getErrorMessage() {
        return errorMsg;
    }



    /**
     * Press the named button in the currently visible dialog box.
     */
    @Override
    public void run() {
        // wait for the message box to appear
        try {
            Thread.sleep(4000) ;
        } catch (InterruptedException e) {
            System.err.println("While waiting :" + e.getMessage()) ;
        }

        // access the message box

        XAccessibleContext xCon = null;

        try {
            XInterface x = (XInterface) xMSF.createInstance(
                               "com.sun.star.awt.Toolkit") ;
            XExtendedToolkit tk =
                UnoRuntime.queryInterface(
                    XExtendedToolkit.class, x);
            new AccessibilityTools();
            XWindow xWindow = UnoRuntime.queryInterface(
                                  XWindow.class, tk.getActiveTopWindow());
            XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);
            xCon = xRoot.getAccessibleContext();
        } catch (Exception e) {
            errorMsg = "Exception while using Accessibility\n" +
                       e.getMessage();
            return;
        }

        // get the button
        XInterface oObj = null;

        try {
            int count = xCon.getAccessibleChildCount();

            for (int i = 0; i < count; i++) {
                XAccessible xAcc = xCon.getAccessibleChild(i);
                String name =
                    xAcc.getAccessibleContext().getAccessibleName();

                // check for button
                if (name.equals(btnName) && (UnoRuntime.queryInterface(
                                                 XButton.class, xAcc) != null)) {
                    oObj = xAcc.getAccessibleContext();
                }

                // check for checkbox
                if (checkBox && (UnoRuntime.queryInterface(XCheckBox.class, xAcc) != null)) {
                    // want to do this action now
                    // probably equates to toggle cb
                    XAccessibleAction xAction =
                        UnoRuntime.queryInterface(
                            XAccessibleAction.class, xAcc.getAccessibleContext());
                    xAction.doAccessibleAction(0);

                    // might be worth using oObj2 to double check the new state??
                }
            }

            if (oObj == null) {
                errorMsg = "No button has been found:\n" +
                           "No action is triggered.";
                return;
            }

            // press button
            XAccessibleAction xAction =
                UnoRuntime.queryInterface(
                    XAccessibleAction.class, oObj);
            xAction.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            errorMsg = "Exception\n" +
                       e.getMessage();
        }
    }

}




