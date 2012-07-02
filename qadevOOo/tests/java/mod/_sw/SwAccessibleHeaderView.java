/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.WriterTools;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test of accessible object for a header of a text document.<p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>::com::sun::star::accessibility::XAccessible</code></li>
* </ul>
* @see com.sun.star.accessibility.XAccessible
*/
public class SwAccessibleHeaderView extends TestCase {

    XTextDocument xTextDoc = null;

    /**
    * Called to create an instance of <code>TestEnvironment</code>
    * with an object to test and related objects. Obtains style
    * <code>Standard</code> from style family <code>PageStyles</code>.
    * Changes values of properties <code>HeaderIsOn</code> and
    * <code>FooterIsOn</code> by <code>true</code>. Obtains accessible component
    * for header.
    *
    * @param Param test parameters
    * @param log writer to log information while testing
    *
    * @see TestEnvironment
    * @see #getTestEnvironment()
    */
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        XNameAccess PageStyles = null;
        XStyle StdStyle = null;

        XStyleFamiliesSupplier StyleFam = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDoc);
        XNameAccess StyleFamNames = StyleFam.getStyleFamilies();

        // obtains style 'Standatd' from style family 'PageStyles'
        try {
            PageStyles = (XNameAccess) AnyConverter.toObject(
                new Type(XNameAccess.class),StyleFamNames.getByName("PageStyles"));
            StdStyle = (XStyle) AnyConverter.toObject(
                    new Type(XStyle.class),PageStyles.getByName("Standard"));
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
            throw new StatusException("Error getting style by name!", e);
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            e.printStackTrace(log);
            throw new StatusException("Error, no such style name! ", e);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            throw new StatusException("Error getting style by name!", e);
        }

        final XPropertySet PropSet = UnoRuntime.queryInterface( XPropertySet.class, StdStyle);

        // changing/getting some properties
        try {
            log.println( "Switching on header" );
            PropSet.setPropertyValue("HeaderIsOn", new Boolean(true));
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set propertyValue...", e);
        }  catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set propertyValue...", e);
        } catch ( com.sun.star.beans.PropertyVetoException e ) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set propertyValue...", e);
        } catch ( com.sun.star.beans.UnknownPropertyException e ) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set propertyValue...", e);
        }

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xTextDoc);

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow((XMultiServiceFactory)Param.getMSF(), aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.getAccessibleObjectForRole(xRoot, AccessibleRole.HEADER);

        oObj = AccessibilityTools.SearchedContext;

        log.println("ImplementationName " + utils.getImplName(oObj));
        at.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    try {
                        PropSet.setPropertyValue("HeaderLeftMargin",
                            new Integer(1000));
                    } catch (com.sun.star.uno.Exception e) {
                        e.printStackTrace();
                        throw new StatusException("Cann't change footer.", e);
                    }
                }
            });

        return tEnv;

    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes text document.
    * @param Param test parameters
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("dispose text document");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     * Called while the <code>TestCase</code> initialization.
     * Creates a text document.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase()
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        log.println( "creating a text document" );
        xTextDoc = WriterTools.createTextDoc((XMultiServiceFactory)Param.getMSF());
    }
}
