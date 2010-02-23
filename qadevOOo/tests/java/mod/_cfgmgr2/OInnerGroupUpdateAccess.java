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
import com.sun.star.container.XNameReplace;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


public class OInnerGroupUpdateAccess extends TestCase {
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
            oObj = (XInterface) names.getByName(names.getElementNames()[0]);

            names = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                            oObj);

            String[] theNames = names.getElementNames();

            log.println("Contains " + theNames.length + " elements");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        log.println("ImplementationName: " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        String[] pNames = new String[] {
            "AutoDetectSystemHC", "HelpTipSeconds", "IsAllowAnimatedGraphics",
            "IsAllowAnimatedText", "IsAutomaticFontColor", "IsForPagePreviews",
            "IsHelpTipsDisappear", "IsSelectionInReadonly", "IsSystemFont"
        };

        String[] pTypes = new String[] {
            "Boolean", "Short", "Boolean", "Boolean", "Boolean", "Boolean",
            "Boolean", "Boolean", "Boolean"
        };

        tEnv.addObjRelation("PropertyNames", pNames);
        tEnv.addObjRelation("PropertyTypes", pTypes);

        tEnv.addObjRelation("ElementName", "AutoDetectSystemHC");

        tEnv.addObjRelation("cannotSwitchParent",
                            "configmgr: BasicElement::setParent: cannot move Entry");

        tEnv.addObjRelation("expectedName", "IsSelectionInReadonly");
        tEnv.addObjRelation("HierachicalName", "/org.openoffice.Office");

        Short val = new Short((short)3);
        tEnv.addObjRelation("INSTANCE1", val);
        tEnv.addObjRelation("NAMEREPLACE", pNames[1]);
        tEnv.addObjRelation("XContainer.NewValue", val);
        tEnv.addObjRelation("XContainer.ElementName", pNames[1]);
        tEnv.addObjRelation("XContainer.Container", (XNameReplace)
                        UnoRuntime.queryInterface(XNameReplace.class, oObj));
        tEnv.addObjRelation("NoSetName", "OInnerGroupInfoAccess");

        return tEnv;
    }
}