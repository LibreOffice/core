/*************************************************************************
 *
 *  $RCSfile: OFormsCollection.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:48:34 $
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
package mod._forms;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.FormTools;

import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


/**
* Test for object which is represented by service
* <code>com.sun.star.form.Forms</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameReplace</code></li>
*  <li> <code>com::sun::star::container::XIndexReplace</code></li>
*  <li> <code>com::sun::star::container::XNameContainer</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexContainer</code></li>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
* </ul>
*
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
*
* @see com.sun.star.container.XNameReplace
* @see com.sun.star.container.XIndexReplace
* @see com.sun.star.container.XNameContainer
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XEnumerationAccess
* @see com.sun.star.container.XIndexContainer
* @see com.sun.star.container.XNameAccess
* @see ifc.container._XNameReplace
* @see ifc.container._XIndexReplace
* @see ifc.container._XNameContainer
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XEnumerationAccess
* @see ifc.container._XIndexContainer
* @see ifc.container._XNameAccess
*/
public class OFormsCollection extends TestCase {
    XComponent xDrawDoc;

    /**
    * Creates Drawing document.
    */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        log.println("creating a draw document");
        xDrawDoc = DrawTools.createDrawDoc(((XMultiServiceFactory) tParam.getMSF()));
    }

    /**
    * Disposes drawing document.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xDrawDoc ");

        try {
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                                        XCloseable.class, xDrawDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }

    /**
    * Adds some controls to the 'Standard' form of a draw page,
    * then adds an empty form to a collection of forms. Then
    * returns the collection as a test object. <p>
    *     Object relations created :
    * <ul>
    *  <li> <code>'INSTANCE1' ... 'INSTANCEN'</code> for
    *    <code>XNameReplace, XNameContainer, XIndexReplace,
    *     XIndexContainer </code> : objects to be inserted
    *     or replaced with in interface tests. Number of relations
    *     depends on number of interface test threads. For each
    *     thread there must be an individual element. </li>
    *  <li> <code>'XNameContainer.AllowDuplicateNames'</code> :
    *   if this relation exists then container elements can have duplicate
    *   names. In case of forms' collection forms can have equal names.</li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;
        XDrawPage oDP = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        oDP = DrawTools.getDrawPage(xDrawDoc, 0);

        (DrawTools.getShapes(oDP))
            .add(FormTools.createControlShape(xDrawDoc, 2000, 1500, 1000, 1000,
                                              "CheckBox"));
        (DrawTools.getShapes(oDP))
            .add(FormTools.createControlShape(xDrawDoc, 3000, 4500, 15000,
                                              1000, "CommandButton"));
        (DrawTools.getShapes(oDP))
            .add(FormTools.createControlShape(xDrawDoc, 5000, 3500, 7500, 5000,
                                              "TextField"));

        oObj = FormTools.getForms(oDP);
        FormTools.insertForm(xDrawDoc, (XNameContainer) oObj, "SecondForm");

        log.println("creating a new environment for drawpage object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        // INSTANCEn : _XNameContainer; _XNameReplace
        log.println("adding INSTANCEn as obj relation to environment");

        XComponent xComp = (XComponent) UnoRuntime.queryInterface(
                                   XComponent.class, xDrawDoc);
        int THRCNT = Integer.parseInt((String) Param.get("THRCNT"));

        for (int n = 1; n < (THRCNT + 1); n++) {
            log.println("adding INSTANCE" + n +
                        " as obj relation to environment");
            tEnv.addObjRelation("INSTANCE" + n,
                                FormTools.createControl(xComp, "Form"));
        }


        // adding indicator that this collection can have duplicate
        // elements with the same names for XNameContainer test.
        tEnv.addObjRelation("XNameContainer.AllowDuplicateNames", new Object());

        tEnv.addObjRelation("INSTANCE", FormTools.createControl(xComp, "Form"));
        tEnv.addObjRelation("INSTANCE2",
                            FormTools.createControl(xComp, "Form"));

        return tEnv;
    } // finish method getTestEnvironment
} // finish class OFormsCollection
