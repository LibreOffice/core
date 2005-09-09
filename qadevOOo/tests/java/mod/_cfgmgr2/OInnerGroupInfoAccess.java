/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OInnerGroupInfoAccess.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:53:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


public class OInnerGroupInfoAccess extends TestCase {
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

        XNameReplace updateAccess = null;
        try {
            XInterface Provider = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                                     .createInstance("com.sun.star.comp.configuration.ConfigurationProvider");
            XMultiServiceFactory pMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                                XMultiServiceFactory.class,
                                                Provider);
            XNameAccess names = (XNameAccess) UnoRuntime.queryInterface(
                                        XNameAccess.class,
                                        pMSF.createInstanceWithArguments(
                                                "com.sun.star.configuration.ConfigurationAccess",
                                                nodeArgs));
            oObj = (XInterface) names.getByName(names.getElementNames()[0]);

            names = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class,
                                                            oObj);

            String[] theNames = names.getElementNames();

            log.println("Contains " + theNames.length + " elements");

            // create a changeable view on the element for XContainer interface
            names = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, pMSF.createInstanceWithArguments(
                                "com.sun.star.configuration.ConfigurationUpdateAccess", nodeArgs));
            updateAccess = (XNameReplace) UnoRuntime.queryInterface(XNameReplace.class,
                                names.getByName(names.getElementNames()[0]));

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

        tEnv.addObjRelation("XContainer.Container", updateAccess);
        // create an unlikely value for "HelpTipSeconds"
        tEnv.addObjRelation("XContainer.NewValue", new Short((short)300000));
        tEnv.addObjRelation("XContainer.ElementName", pNames[1]);

        tEnv.addObjRelation("ElementName", "AutoDetectSystemHC");

        tEnv.addObjRelation("cannotSwitchParent",
                            "configmgr: BasicElement::setParent: cannot move Entry");

        tEnv.addObjRelation("allReadOnly",
                            "all Properties of OInnerGroupInfoAccess are read Only");

        tEnv.addObjRelation("expectedName", "IsSelectionInReadonly");
        tEnv.addObjRelation("HierachicalName", "/org.openoffice.Office");
        tEnv.addObjRelation("NoSetName", "OInnerGroupInfoAccess");

        return tEnv;
    }
}