/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OSetElementSetInfoAccess.java,v $
 * $Revision: 1.5 $
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
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNameReplace;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


public class OSetElementSetInfoAccess extends TestCase {
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
        Object instance = null;
        Object instance1 = null;
        log.println("creating the Environment");

        PropertyValue[] nodeArgs = new PropertyValue[1];
        PropertyValue nodepath = new PropertyValue();
        nodepath.Name = "nodepath";
        nodepath.Value = "org.openoffice.Office.Labels/Manufacturer";
        nodepath.Handle = -1;
        nodepath.State = PropertyState.DEFAULT_VALUE;
        nodeArgs[0] = nodepath;

        String hierarchicalElementName = "Labels['Avery A4']";
        String elementName = "Avery A4";

        XNameReplace xChangeView = null;
        XNameContainer xContainer = null;
        XHierarchicalNameAccess xHierachNameAccess = null;
        try {
            XInterface Provider = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                                     .createInstance("com.sun.star.comp.configuration.ConfigurationProvider");
            XMultiServiceFactory pMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                                XMultiServiceFactory.class,
                                                Provider);
            xHierachNameAccess = (XHierarchicalNameAccess) UnoRuntime.queryInterface(
                                                    XHierarchicalNameAccess.class,
                                                    pMSF.createInstanceWithArguments(
                                                            "com.sun.star.configuration.ConfigurationAccess",
                                                            nodeArgs));

            oObj = (XInterface) xHierachNameAccess.getByHierarchicalName(hierarchicalElementName);

            // craete a changeable view.
            XHierarchicalNameAccess changeableNames = (XHierarchicalNameAccess) UnoRuntime.queryInterface(XHierarchicalNameAccess.class,
                        pMSF.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess", nodeArgs));

            XInterface xInt = (XInterface) changeableNames.getByHierarchicalName(hierarchicalElementName);
            xChangeView = (XNameReplace)UnoRuntime.queryInterface(XNameReplace.class, xInt);
            XSingleServiceFactory jobsFac = (XSingleServiceFactory) UnoRuntime.queryInterface(
                                                    XSingleServiceFactory.class, xChangeView);
            instance1 = jobsFac.createInstance();

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        log.println("ImplementationName: " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(
                                    XNameAccess.class, oObj);

        String[] pNames = xNameAccess.getElementNames();

        tEnv.addObjRelation("ElementName", pNames[0]);

        tEnv.addObjRelation("cannotSwitchParent",
                            "configmgr: BasicElement::setParent: cannot move Entry");

        tEnv.addObjRelation("XContainer.NewValue", instance1);
        tEnv.addObjRelation("XContainer.ElementName", "L0");
        tEnv.addObjRelation("XContainer.Container", xChangeView);

        // dispose the owner of the test object
        tEnv.addObjRelation("XComponent.DisposeThis", (XComponent)
                    UnoRuntime.queryInterface(XComponent.class, xHierachNameAccess));

        tEnv.addObjRelation("expectedName", pNames[0]);
        tEnv.addObjRelation("HierachicalName", "/org.openoffice.Office");
        tEnv.addObjRelation("NAMEREPLACE", pNames[0]);
        tEnv.addObjRelation("NoSetName", "OInnerTreeSetInfoAccess");
        tEnv.addObjRelation("TemplateName", "org.openoffice.Office.Labels/LabelType");
        tEnv.addObjRelation("TemplateInstance", "org.openoffice.Office.Labels/Labels");

        return tEnv;
    }
}
