/*************************************************************************
 *
 *  $RCSfile: SecurityDialogUtil.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-03-25 11:26:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
 * Returns the error message that occured while
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




