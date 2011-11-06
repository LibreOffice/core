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