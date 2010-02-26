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
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameReplace;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XChangesBatch;


public class ORootElementGroupInfoAccess extends TestCase {
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
        nodepath.Value = "org.openoffice.Office.Common";
        nodepath.Handle = -1;
        nodepath.State = PropertyState.DEFAULT_VALUE;
        nodeArgs[0] = nodepath;

        XInterface changeView = null;
        XInterface alternateObject = null;
        try {
            XInterface Provider = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                                     .createInstance("com.sun.star.comp.configuration.ConfigurationProvider");
            XMultiServiceFactory pMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                                XMultiServiceFactory.class,
                                                Provider);
            oObj = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                           pMSF.createInstanceWithArguments(
                                                                   "com.sun.star.configuration.ConfigurationAccess",
                                                                   nodeArgs));
            // create a view on a different object that can change something, so listeners can be triggered.
            // this is done for XContainer test
            nodepath.Value = "org.openoffice.Office.Common/Internal";
            changeView =  (XNameReplace) UnoRuntime.queryInterface(XNameReplace.class,
                                                           pMSF.createInstanceWithArguments(
                                                                   "com.sun.star.configuration.ConfigurationUpdateAccess",
                                                                   nodeArgs));
            alternateObject = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                           pMSF.createInstanceWithArguments(
                                                                   "com.sun.star.configuration.ConfigurationAccess",
                                                                   nodeArgs));
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        log.println("ImplementationName: " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XPropertySet prop = (XPropertySet) UnoRuntime.queryInterface(
                                    XPropertySet.class, oObj);
        util.dbg.printPropertiesNames(prop);

        String[] pNames = new String[] {
            "Accessibility", "AddXMLToStorage", "AsianLayout", "AutoCorrect",
            "Cache", "DateFormat", "ExternalApps", "ExternalMailer", "Filter",
            "Font", "Gallery", "Help", "History", "I18N", "Image", "Internal",
            "InternalMSExport", "Java", "Load", "Menus", "Misc",
            "OfficeObjects", "Passwords", "Path", "Print", "Save",
            "SearchOptions", "Security", "Setup", "Startup", "Undo",
            "Vectorize", "View", "WorkingSet", "_3D_Engine"
        };

        String[] pTypes = new String[36];

        for (int k = 0; k < pTypes.length; k++) {
            pTypes[k] = "XInterface";
        }

        tEnv.addObjRelation("PropertyNames", pNames);
        tEnv.addObjRelation("PropertyTypes", pTypes);

        tEnv.addObjRelation("ElementName", "Accessibility");

        tEnv.addObjRelation("XLocalizable.ReadOnly", "Locale of ORootElementGroupInfoAccess is read Only");
        tEnv.addObjRelation("allReadOnly",
                            "all Properties of ORootElementGroupInfoAccess are read Only");

        tEnv.addObjRelation("XContainer.NewValue", "SenselessString");
        tEnv.addObjRelation("XContainer.ElementName", "CurrentTempURL");
        tEnv.addObjRelation("XContainer.Container", changeView);
        tEnv.addObjRelation("XContainer.AlternateObject", alternateObject);

        tEnv.addObjRelation("XChangesNotifier.ChangesBatch", (XChangesBatch)UnoRuntime.queryInterface(
                                                                            XChangesBatch.class, changeView));

        // set a new temp directory: use java.io.tmpdir as substitute, for XChangesNotifier test
        // get a XPropertySet from the sub element
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, changeView);
        String newTempURL = util.utils.getFullURL(util.utils.getUsersTempDir());
        String curTempURL = "";
        try {
            curTempURL = (String)xProp.getPropertyValue("CurrentTempURL");
        }
        catch(Exception e) {
            log.println("Cannot get property for XChangesNotifier test: this test is bound to fail.");
            e.printStackTrace((PrintWriter)log);
        }
        // fallback 1: get user home
        if (newTempURL.equalsIgnoreCase(curTempURL)) {
            newTempURL = util.utils.getFullURL(System.getProperty("user.home"));
            // fallback 2: get user dir
            if (newTempURL.equalsIgnoreCase(curTempURL)) {
                newTempURL = util.utils.getFullURL(System.getProperty("user.dir"));
            }
        }
        tEnv.addObjRelation("XChangesNotifier.ChangeElement", newTempURL);
        tEnv.addObjRelation("XChangesNotifier.OriginalElement", curTempURL);
        tEnv.addObjRelation("XChangesNotifier.PropertyName", "CurrentTempURL");
        tEnv.addObjRelation("XChangesNotifier.PropertySet", xProp);

        tEnv.addObjRelation("expectedName", "OfficeObjects");
        tEnv.addObjRelation("HierachicalName", "/org.openoffice");

        tEnv.addObjRelation("NoSetName", "ORootElementGroupInfoAccess");

        return tEnv;
    }
}