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
package mod._toolkit;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.utils;


/**
* Test for object which is represented by service
* <code>com.sun.star.awt.UnoControlDialogModel</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::awt::UnoControlDialogModel</code></li>
*  <li> <code>com::sun::star::io::XPersistObject</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.awt.UnoControlDialogModel
* @see com.sun.star.io.XPersistObject
* @see com.sun.star.lang.XComponent
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.beans.XMultiPropertySet
* @see ifc.awt._UnoControlDialogModel
* @see ifc.io._XPersistObject
* @see ifc.lang._XComponent
* @see ifc.beans._XPropertySet
* @see ifc.beans._XMultiPropertySet
*/
public class UnoControlDialogModel extends TestCase {
    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.awt.UnoControlDialogModel</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'OBJNAME'</code> for
    *      {@link ifc.io._XPersistObject} </li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;
        XInterface dialogModel = null;
        String _buttonName = "MyButton";
        String _labelName = "MyLabel";
        String _labelPrefix = "MyLabelPrefix";
        XMultiServiceFactory xMultiServiceFactory = null;

        try {
            dialogModel = (XInterface) ((XMultiServiceFactory) Param.getMSF()).createInstance(
                                  "com.sun.star.awt.UnoControlDialogModel");

            // create the dialog model and set the properties
            XPropertySet xPSetDialog = (XPropertySet) UnoRuntime.queryInterface(
                                               XPropertySet.class, dialogModel);
            xPSetDialog.setPropertyValue("PositionX", new Integer(100));
            xPSetDialog.setPropertyValue("PositionY", new Integer(100));
            xPSetDialog.setPropertyValue("Width", new Integer(150));
            xPSetDialog.setPropertyValue("Height", new Integer(100));
            xPSetDialog.setPropertyValue("Title",
                                         new String("Runtime Dialog Demo"));


            // get the service manager from the dialog model
            xMultiServiceFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                           XMultiServiceFactory.class,
                                           dialogModel);

            // create the button model and set the properties
            Object buttonModel = xMultiServiceFactory.createInstance(
                                         "com.sun.star.awt.UnoControlButtonModel");
            XPropertySet xPSetButton = (XPropertySet) UnoRuntime.queryInterface(
                                               XPropertySet.class, buttonModel);
            xPSetButton.setPropertyValue("PositionX", new Integer(50));
            xPSetButton.setPropertyValue("PositionY", new Integer(30));
            xPSetButton.setPropertyValue("Width", new Integer(50));
            xPSetButton.setPropertyValue("Height", new Integer(14));
            xPSetButton.setPropertyValue("Name", _buttonName);
            xPSetButton.setPropertyValue("TabIndex", new Short((short) 0));
            xPSetButton.setPropertyValue("Label", new String("Click Me"));

            // create the label model and set the properties
            Object labelModel = xMultiServiceFactory.createInstance(
                                        "com.sun.star.awt.UnoControlFixedTextModel");
            XPropertySet xPSetLabel = (XPropertySet) UnoRuntime.queryInterface(
                                              XPropertySet.class, labelModel);
            xPSetLabel.setPropertyValue("PositionX", new Integer(40));
            xPSetLabel.setPropertyValue("PositionY", new Integer(60));
            xPSetLabel.setPropertyValue("Width", new Integer(100));
            xPSetLabel.setPropertyValue("Height", new Integer(14));
            xPSetLabel.setPropertyValue("Name", _labelName);
            xPSetLabel.setPropertyValue("TabIndex", new Short((short) 1));
            xPSetLabel.setPropertyValue("Label", _labelPrefix);

            // insert the control models into the dialog model
            XNameContainer xNameCont = (XNameContainer) UnoRuntime.queryInterface(
                                               XNameContainer.class,
                                               dialogModel);
            xNameCont.insertByName(_buttonName, buttonModel);
            xNameCont.insertByName(_labelName, labelModel);

            // create the dialog control and set the model
            XControl dialog = (XControl) UnoRuntime.queryInterface(
                                      XControl.class,
                                      ((XMultiServiceFactory) Param.getMSF()).createInstance(
                                              "com.sun.star.awt.UnoControlDialog"));
            XControl xControl = (XControl) UnoRuntime.queryInterface(
                                        XControl.class, dialog);
            XControlModel xControlModel = (XControlModel) UnoRuntime.queryInterface(
                                                  XControlModel.class,
                                                  dialogModel);
            xControl.setModel(xControlModel);
        } catch (Exception e) {
            throw new StatusException("Could no create test object", e);
        }

        oObj = dialogModel;

        log.println("creating a new environment for object");

        XMultiServiceFactory oMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                            XMultiServiceFactory.class, oObj);

        TestEnvironment tEnv = new TestEnvironment(oObj);

        try {
            // XNameReplace
            tEnv.addObjRelation("INSTANCE1",
                                xMultiServiceFactory.createInstance(
                                        "com.sun.star.awt.UnoControlFixedTextModel"));


            //XContainer
            tEnv.addObjRelation("INSTANCE",
                                xMultiServiceFactory.createInstance(
                                        "com.sun.star.awt.UnoControlFixedTextModel"));
        } catch (com.sun.star.uno.Exception e) {
            log.println("Could not add object relations 'INSTANCEn'");
            e.printStackTrace(log);
        }

        tEnv.addObjRelation("OBJNAME", "stardiv.vcl.controlmodel.Dialog");
        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment
}