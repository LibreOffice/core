/*************************************************************************
 *
 *  $RCSfile: OSetElementGroupInfoAccess.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2004-11-02 11:59:29 $
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