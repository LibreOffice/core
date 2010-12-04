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
package ifc.script.framework;

import com.sun.star.awt.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import drafts.com.sun.star.accessibility.*;
import drafts.com.sun.star.awt.XExtendedToolkit;

// Jsuite classes
import util.AccessibilityTools;
import util.dbg;
/**
* Thread that pushes the buttons or checkbox
* on the message box that is on top.
*/
public class SecurityDialogUtil extends Thread {

private XMultiServiceFactory xMSF = null;
private String errorMsg;
private boolean errorHappened;
private String btnName;
private boolean checkBox;

/**
 * Constructor.
 * @param xMSF A MultiServiceFactory.
 * @param log The log writer.
 */
public SecurityDialogUtil(XMultiServiceFactory xMSF, String btnName, boolean checkBox )
{
    this.xMSF = xMSF;
    this.btnName = btnName;
    this.checkBox = checkBox;
    errorMsg = "";
    errorHappened=false;
}

/**
 * Returns the error message that occurred while
 * accessing and pressing the button.
 * @return Error message.
 */
public String getErrorMessage()
{
    return errorMsg;
}

/**
 * Is there an error message available?
 * @return true, if an error happened
 */
public boolean hasErrorMessage()
{
    return !errorMsg.equals("");
}

/**
 * Press the named button in the currently visible dialog box.
 */
public void run()
{
    // wait for the message box to appear
    try
    {
        Thread.currentThread().sleep(4000) ;
    }
    catch (InterruptedException e)
    {
        System.err.println("While waiting :" + e.getMessage()) ;
    }

    // access the message box

     XAccessibleContext xCon = null;
    try
    {
        XInterface x = (XInterface) xMSF.createInstance(
                                    "com.sun.star.awt.Toolkit") ;
        XExtendedToolkit tk =
                (XExtendedToolkit)UnoRuntime.queryInterface(
                                        XExtendedToolkit.class,x);
        AccessibilityTools at = new AccessibilityTools();
        XWindow xWindow = (XWindow)UnoRuntime.queryInterface(
                                XWindow.class,tk.getActiveTopWindow());
        XAccessible xRoot = at.getAccessibleObject(xWindow);
        xCon = xRoot.getAccessibleContext();
    }
    catch (Exception e)
    {
        errorMsg="Exception while using Accessibility\n"+
                                                    e.getMessage();
        return;
    }
    // get the button
    XInterface oObj = null;
    try
    {
        /* System.err.println("Name of the AccessibleContext:\n\t"+
                                        xCon.getAccessibleName()); */
        int count = xCon.getAccessibleChildCount();
        // System.err.println("Number of children: "+count);
        for (int i=0; i<count; i++) {
            XAccessible xAcc = xCon.getAccessibleChild(i);
            String name =
                    xAcc.getAccessibleContext().getAccessibleName();
            // System.out.println("Child "+i+": "+ name);
            // check for button
            if ( name.equals( btnName ) && ( UnoRuntime.queryInterface(
                                    XButton.class, xAcc ) != null ) )
            {
                // System.out.println("Child "+i+": "+ name);
                oObj = xAcc.getAccessibleContext();
            }
            // check for checkbox
            if ( checkBox &&  ( UnoRuntime.queryInterface( XCheckBox.class, xAcc ) != null ) )
            {
                // want to do this action now
                // probably equates to toggle cb
                XAccessibleAction xAction =
                        (XAccessibleAction)UnoRuntime.queryInterface(
                        XAccessibleAction.class, xAcc.getAccessibleContext());
                xAction.doAccessibleAction(0);

                // might be worth using oObj2 to double check the new state??
            }
        }
        if (oObj == null) {
            errorMsg="No button has been found:\n"+
                     "No action is triggered.";
            return;
        }
        // press button
        XAccessibleAction xAction =
                (XAccessibleAction)UnoRuntime.queryInterface(
                XAccessibleAction.class, oObj);
        xAction.doAccessibleAction(0);
    }
    catch(com.sun.star.lang.IndexOutOfBoundsException e) {
        errorMsg="Exception\n"+
                        e.getMessage();
    }
}

}




