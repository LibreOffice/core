/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ORootElementSetInfoAccess.java,v $
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
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameReplace;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XChangesBatch;


public class ORootElementSetInfoAccess extends TestCase {
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
        nodepath.Value = "org.openoffice.Office.Jobs/Jobs";
        nodepath.Handle = -1;
        nodepath.State = PropertyState.DEFAULT_VALUE;
        nodeArgs[0] = nodepath;

        XInterface changeView = null;
        Object instance = null;
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
            changeView =  (XNameReplace) UnoRuntime.queryInterface(XNameReplace.class,
                                                           pMSF.createInstanceWithArguments(
                                                                   "com.sun.star.configuration.ConfigurationUpdateAccess",
                                                                   nodeArgs));
            XSingleServiceFactory jobsFac = (XSingleServiceFactory) UnoRuntime.queryInterface(
                                        XSingleServiceFactory.class,
                                        changeView);
            instance = jobsFac.createInstance();

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        log.println("ImplementationName: " + utils.getImplName(oObj));
        log.println("ChangeView: " + utils.getImplName(changeView));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XNameReplace container = (XNameReplace)UnoRuntime.queryInterface(XNameReplace.class, changeView);
        tEnv.addObjRelation("XContainer.NewValue", instance);
        tEnv.addObjRelation("XContainer.ElementName", "RegistrationRequest");
        tEnv.addObjRelation("XContainer.Container", container);

        tEnv.addObjRelation("XChangesNotifier.ChangesBatch", (XChangesBatch)UnoRuntime.queryInterface(XChangesBatch.class, changeView));
        tEnv.addObjRelation("XChangesNotifier.ChangeElement", instance);
        tEnv.addObjRelation("XChangesNotifier.PropertyName", "RegistrationRequest");
        tEnv.addObjRelation("XChangesNotifier.NameReplace", container);

        tEnv.addObjRelation("XLocalizable.ReadOnly", "Locale of ORootElementSetInfoAccess is read Only");
        tEnv.addObjRelation("allReadOnly",
                            "all Properties of ORootElementSetInfoAccess are read Only");
        tEnv.addObjRelation("NoSetName", "ORootElementSetInfoAccess");
        tEnv.addObjRelation("TemplateName", "org.openoffice.Office.Jobs/Job");
        tEnv.addObjRelation("HierachicalName", "/org.openoffice");
        tEnv.addObjRelation("ElementName", "RegistrationRequest");
        tEnv.addObjRelation("expectedName", "RegistrationRequest");

        return tEnv;
    }
}
