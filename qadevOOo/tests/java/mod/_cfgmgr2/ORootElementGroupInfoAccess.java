/*************************************************************************
 *
 *  $RCSfile: ORootElementGroupInfoAccess.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-12-11 11:54:14 $
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
        String newTempURL = util.utils.getFullURL(System.getProperty("java.io.tmpdir"));
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