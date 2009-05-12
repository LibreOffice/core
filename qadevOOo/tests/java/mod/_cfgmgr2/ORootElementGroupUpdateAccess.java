/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ORootElementGroupUpdateAccess.java,v $
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
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameReplace;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XChangesBatch;


public class ORootElementGroupUpdateAccess extends TestCase {
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
        nodepath.Value = "org.openoffice.Office.Common/Internal";

        nodepath.Handle = -1;
        nodepath.State = PropertyState.DEFAULT_VALUE;
        nodeArgs[0] = nodepath;

        try {
            XInterface Provider = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                                     .createInstance("com.sun.star.comp.configuration.ConfigurationProvider");
            XMultiServiceFactory pMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                                XMultiServiceFactory.class,
                                                Provider);
            oObj = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                           pMSF.createInstanceWithArguments(
                                                                   "com.sun.star.configuration.ConfigurationUpdateAccess",
                                                                   nodeArgs));

/*            helper.ConfigurationRead read = new helper.ConfigurationRead(pMSF, "org.openoffice.Office.Common");
            String[] subs = read.getRootNodeNames();
            for (int i=0; i<subs.length; i++)
                System.out.println("Rootnodes " + subs[i]);
            subs = read.getSubNodeNames("Internal");
            for (int i=0; i<subs.length; i++)
                System.out.println("Rootnodes " + subs[i]);
            Object subNode = read.getByHierarchicalName("Internal/RecoveryList");
            util.dbg.getSuppServices(subNode); */

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        log.println("ImplementationName: " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XPropertySet prop = (XPropertySet) UnoRuntime.queryInterface(
                                    XPropertySet.class, oObj);

        Object instance = null;

        String[] pNames = new String[] {
            "CurrentTempURL",
            "DevelopmentChart",
            "SendCrashMail",
            "Slot",
            "UseMailUI"//, "RecoveryList"
        };

        String[] pTypes = new String[]{
            "String",
            "Boolean",
            "Boolean",
            "Boolean",
            "Boolean"
        };

        tEnv.addObjRelation("PropertyNames", pNames);
        tEnv.addObjRelation("PropertyTypes", pTypes);

        // for XHierarchicalNameAccess
        tEnv.addObjRelation("ElementName", "Slot");

        tEnv.addObjRelation("XLocalizable.ReadOnly", "Localization is final and can't be changed");

        tEnv.addObjRelation("XChangesNotifier.ChangesBatch", (XChangesBatch)UnoRuntime.queryInterface(XChangesBatch.class, oObj));

        // set a new temp directory: use java.io.tmpdir as substitute
        String newTempURL = util.utils.getFullURL(util.utils.getUsersTempDir());
        String curTempURL = "";
        try {
            curTempURL = (String)prop.getPropertyValue("CurrentTempURL");
        }
        catch(Exception e) {
            log.println("Cannot get property for XChangesBatch test: this test is bound to fail.");
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

        tEnv.addObjRelation("XChangesBatch.ChangeElement", newTempURL);
        tEnv.addObjRelation("XChangesBatch.OriginalElement", curTempURL);
        tEnv.addObjRelation("XChangesBatch.PropertyName", "CurrentTempURL");
        tEnv.addObjRelation("XChangesBatch.PropertySet", prop);
        tEnv.addObjRelation("XChangesNotifier.ChangeElement", newTempURL);
        tEnv.addObjRelation("XChangesNotifier.OriginalElement", curTempURL);
        tEnv.addObjRelation("XChangesNotifier.PropertyName", "CurrentTempURL");
        tEnv.addObjRelation("XChangesNotifier.PropertySet", prop);

        tEnv.addObjRelation("expectedName", "RecoveryList");
        tEnv.addObjRelation("HierachicalName", "/org.openoffice");

        String overwriteString = new String("ThisIsSenselessForSure");
        tEnv.addObjRelation("INSTANCE1", overwriteString);
        tEnv.addObjRelation("NAMEREPLACE", pNames[0]);

        tEnv.addObjRelation("XContainer.NewValue", overwriteString);
        tEnv.addObjRelation("XContainer.ElementName", pNames[0]);
        tEnv.addObjRelation("XContainer.Container", (XNameReplace)UnoRuntime.queryInterface(XNameReplace.class, oObj));

        tEnv.addObjRelation("NoSetName", "ORootElementGroupInfoAccess");

        return tEnv;
    }
}
