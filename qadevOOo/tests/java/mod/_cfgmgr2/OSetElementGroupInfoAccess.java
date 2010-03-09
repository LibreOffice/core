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
package mod._cfgmgr2;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNameReplace;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XChangesBatch;


public class OSetElementGroupInfoAccess extends TestCase {
    /** Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects. <br>
     *
     * An instance of com.sun.star.comp.configuration.ConfigurationProvider
     * is created at the MultiServiceFactory of the Office<br>
     * At this an instance of com.sun.star.configuration.ConfigurationAccess is created
     * and from the resulting XNameAccess the first element is taken.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment()
     *
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XInterface oObj = null;
        log.println("creating the Environment");

        PropertyValue[] nodeArgs = new PropertyValue[1];
        PropertyValue nodepath = new PropertyValue();
        nodepath.Name = "nodepath";
        nodepath.Value = "org.openoffice.Office.Common/Menus";
        nodepath.Handle = -1;
        nodepath.State = PropertyState.DEFAULT_VALUE;
        nodeArgs[0] = nodepath;

        XNameAccess xHierachNameAccess = null;
        XNameReplace xChangeView = null;
        XComponent xDisposeComponent = null;

        try {
            XInterface Provider = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                                     .createInstance("com.sun.star.comp.configuration.ConfigurationProvider");
            XMultiServiceFactory pMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                                XMultiServiceFactory.class,
                                                Provider);

            xHierachNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                        pMSF.createInstanceWithArguments(
                                        "com.sun.star.configuration.ConfigurationAccess", nodeArgs));

            XInterface oInnerSetInfoAccess = (XInterface) xHierachNameAccess.getByName("New");

            XNameAccess names = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                            oInnerSetInfoAccess);

            String[] theNames = names.getElementNames();

            log.println("Contains " + theNames.length + " elements");

            xDisposeComponent = (XComponent)
                                UnoRuntime.queryInterface(XComponent.class, xHierachNameAccess);

            String[] elnames = names.getElementNames();

            log.println("use node 'm0'");
            oObj = (XInterface) names.getByName("m0");



            log.println("create ConfigurationUpdateAccess to modify the object...");

            xHierachNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                        pMSF.createInstanceWithArguments(
                                                 "com.sun.star.configuration.ConfigurationUpdateAccess",
                                                 nodeArgs));

            oInnerSetInfoAccess = (XInterface) xHierachNameAccess.getByName("New");

            names = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                            oInnerSetInfoAccess);

            log.println("use node 'm0'");

            XInterface xInt = (XInterface) names.getByName("m0");
            xChangeView = (XNameReplace) UnoRuntime.queryInterface(XNameReplace.class,
                                                            xInt);

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        log.println("ImplementationName: " + utils.getImplName(oObj));

        String[] pNames = new String[] { "ImageIdentifier", "Title", "URL", "TargetName" };

        String[] pTypes = new String[] { "String", "String", "String", "String" };

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("cannotSwitchParent",
                            "configmgr: BasicElement::setParent: cannot move Entry");

        tEnv.addObjRelation("HierachicalName", "/org.openoffice.Office");
        tEnv.addObjRelation("ElementName", "Title");
        tEnv.addObjRelation("NoSetName", "OInnerValueSetInfoAccess");
        tEnv.addObjRelation("TemplateName", "cfg:value/cfg:any");
        tEnv.addObjRelation("expectedName", "ImageIdentifier");

        tEnv.addObjRelation("XContainer.NewValue", "aValue");
        tEnv.addObjRelation("XContainer.ElementName", "TargetName");
        tEnv.addObjRelation("XContainer.Container", xChangeView);

        // dispose the owner of the test object
        tEnv.addObjRelation("XComponent.DisposeThis", xDisposeComponent);

        tEnv.addObjRelation("PropertyNames", pNames);
        tEnv.addObjRelation("PropertyTypes", pTypes);
        tEnv.addObjRelation("allReadOnly",
                            "all Properties of OSetElementGroupInfoAccess are read Only");

        tEnv.addObjRelation("TemplateInstance", "org.openoffice.Office.Common/MenuType");

        return tEnv;
    }
    /**
     * Create entries in the ExternalApps layer, so there is something to test
     * @param oObj The test object: used to create some entries.
     */
    private void createSomeEntries(XInterface xObj) {
        XNameContainer xCont = (XNameContainer)UnoRuntime.queryInterface(XNameContainer.class, xObj);
        insertOrUpdate(xCont, "file", xObj);
//        insertOrUpdate(xCont, "ftp", "some");
//        insertOrUpdate(xCont, "dummy", "arbitrary");
//        insertOrUpdate(xCont, "http", "value");
        // write the changes into the user layer.
        XChangesBatch xBatch = (XChangesBatch)UnoRuntime.queryInterface(XChangesBatch.class, xObj);
        try {
            xBatch.commitChanges();
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            // ignore: bug will be found with the interface test
        }
    }

    /**
     * Insert a value in a name container or else update it
     * @param xCont The name conationer to insert or update.
     * @param name The name of the value.
     * @param value The value itself.
     */
    private void insertOrUpdate(XNameContainer xCont, String name, XInterface value) {
        boolean update = false;
        XSingleServiceFactory xFac = (XSingleServiceFactory) UnoRuntime.queryInterface(
                                                XSingleServiceFactory.class,
                                                value);

        try {
            xCont.insertByName(name, xFac.createInstance());
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("ERROR: " + e.toString());
            // ignore: bug will be found with the interface test
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("ERROR: " + e.toString());
            // ignore: bug will be found with the interface test
        }
        catch(com.sun.star.container.ElementExistException e) {
            update = true;
        }
        catch(com.sun.star.uno.Exception e) {
            log.println("could not create Instance: " + e.toString());
        }

        try {
            if (update)
                xCont.replaceByName(name, xFac.createInstance());
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            // ignore: bug will be found with the interface test
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            // ignore: bug will be found with the interface test
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            // ignore: bug will be found with the interface test
        }
        catch(com.sun.star.uno.Exception e) {
            log.println("could not create Instance: " + e.toString());
        }
    }
}