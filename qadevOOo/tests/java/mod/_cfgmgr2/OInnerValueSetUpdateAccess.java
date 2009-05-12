/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OInnerValueSetUpdateAccess.java,v $
 * $Revision: 1.6 $
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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XChangesBatch;


public class OInnerValueSetUpdateAccess extends TestCase {
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

        // create some entries for testing, use the ORootElementValueSetUpdateAccess
        // service for this: see cfgmgr2.ORootElementValueSetUpdateAccess
        createSomeEntries((XMultiServiceFactory)tParam.getMSF());

        PropertyValue[] nodeArgs = new PropertyValue[1];
        PropertyValue nodepath = new PropertyValue();
        nodepath.Name = "nodepath";
        nodepath.Value = "/org.openoffice.Office.Common";
        nodepath.Handle = -1;
        nodepath.State = PropertyState.DEFAULT_VALUE;
        nodeArgs[0] = nodepath;

        try {
            XInterface Provider = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                                     .createInstance("com.sun.star.comp.configuration.ConfigurationProvider");
            XMultiServiceFactory pMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                                XMultiServiceFactory.class,
                                                Provider);
            XNameAccess names = (XNameAccess) UnoRuntime.queryInterface(
                                        XNameAccess.class,
                                        pMSF.createInstanceWithArguments(
                                                "com.sun.star.configuration.ConfigurationUpdateAccess",
                                                nodeArgs));

            oObj = (XInterface) names.getByName("ExternalApps");

            names = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                            oObj);

            String[] theNames = names.getElementNames();

            log.println("Contains " + theNames.length + " elements");

//            for (int k = 0; k < theNames.length; k++) {
//                System.out.println("child " + theNames[k]);
//            }
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        log.println("ImplementationName: " + utils.getImplName(oObj));

        String[] pNames = new String[] {
            "file", "ftp", "http", "https", "mailto"
        };

        String[] pTypes = new String[] {
            "String", "String", "String", "String", "String"
        };

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("cannotSwitchParent",
                            "configmgr: BasicElement::setParent: cannot move Entry");

        String overwriteString = "NewOne";
        tEnv.addObjRelation("INSTANCE1", overwriteString);
        tEnv.addObjRelation("NAMEREPLACE", pNames[1]);
        // for XContainer
        tEnv.addObjRelation("INSTANCE", overwriteString);

        tEnv.addObjRelation("HierachicalName", "/org.openoffice.Office");
        tEnv.addObjRelation("ElementName", "file");
        tEnv.addObjRelation("NoSetName", "OInnerValueSetInfoAccess");
        tEnv.addObjRelation("TemplateName", "cfg:value/cfg:any");
        tEnv.addObjRelation("expectedName", "file");

        return tEnv;
    }

        /**
     * Create entries in the ExternalApps layer, so there is something to test
     * @param oObj The test object: used to create some entries.
     */
    private void createSomeEntries(XMultiServiceFactory xMSF) {
        PropertyValue[] nodeArgs = new PropertyValue[1];
        PropertyValue nodepath = new PropertyValue();
        nodepath.Name = "nodepath";
        nodepath.Value = "org.openoffice.Office.Common/ExternalApps";
        nodepath.Handle = -1;
        nodepath.State = PropertyState.DEFAULT_VALUE;
        nodeArgs[0] = nodepath;

        Object oObj = null;
        try {
            XInterface Provider = (XInterface) xMSF.createInstance("com.sun.star.comp.configuration.ConfigurationProvider");
            XMultiServiceFactory pMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                                XMultiServiceFactory.class,
                                                Provider);
            oObj = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                           pMSF.createInstanceWithArguments(
                                                                   "com.sun.star.configuration.ConfigurationUpdateAccess",
                                                                   nodeArgs));

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        XNameContainer xCont = (XNameContainer)UnoRuntime.queryInterface(XNameContainer.class, oObj);
        insertOrUpdate(xCont, "file", "just");
        insertOrUpdate(xCont, "ftp", "some");
        insertOrUpdate(xCont, "dummy", "arbitrary");
        insertOrUpdate(xCont, "http", "value");
        // write the changes into the user layer.
        XChangesBatch xBatch = (XChangesBatch)UnoRuntime.queryInterface(XChangesBatch.class, oObj);
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
    private void insertOrUpdate(XNameContainer xCont, String name, String value) {
        boolean update = false;
        try {
            xCont.insertByName(name, value);
            System.out.println("##### No Exception!");
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            // ignore: bug will be found with the interface test
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            // ignore: bug will be found with the interface test
        }
        catch(com.sun.star.container.ElementExistException e) {
            update = true;
        }
        try {
            if (update)
                xCont.replaceByName(name, value);
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
    }

}
