/*************************************************************************
 *
 *  $RCSfile: OGroupBoxModel.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:49:05 $
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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.SOfficeFactory;

import com.sun.star.drawing.XControlShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


/**
* Test for object which is represented by service
* <code>com.sun.star.form.component.GroupBox</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::awt::UnoControlGroupBoxModel</code></li>
*  <li> <code>com::sun::star::io::XPersistObject</code></li>
*  <li> <code>com::sun::star::container::XChild</code></li>
*  <li> <code>com::sun::star::form::FormControlModel</code></li>
*  <li> <code>com::sun::star::form::FormComponent</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
*  <li> <code>com::sun::star::beans::XPropertyState</code></li>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.awt.UnoControlGroupBoxModel
* @see com.sun.star.io.XPersistObject
* @see com.sun.star.container.XChild
* @see com.sun.star.form.FormControlModel
* @see com.sun.star.form.FormComponent
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.beans.XFastPropertySet
* @see com.sun.star.beans.XPropertyState
* @see com.sun.star.container.XNamed
* @see com.sun.star.beans.XMultiPropertySet
* @see com.sun.star.lang.XComponent
* @see ifc.awt._UnoControlGroupBoxModel
* @see ifc.io._XPersistObject
* @see ifc.container._XChild
* @see ifc.form._FormControlModel
* @see ifc.form._FormComponent
* @see ifc.beans._XPropertySet
* @see ifc.beans._XFastPropertySet
* @see ifc.beans._XPropertyState
* @see ifc.container._XNamed
* @see ifc.beans._XMultiPropertySet
* @see ifc.lang._XComponent
*/
public class OGroupBoxModel extends TestCase {
    XComponent xDrawDoc = null;

    /**
    * Creates Drawing document.
    */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory(((XMultiServiceFactory) tParam.getMSF()));

        try {
            log.println("creating a draw document");
            xDrawDoc = SOF.createDrawDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
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
    * <code>GroupBox</code> component created and added to the draw
    * page.
    *     Object relations created :
    * <ul>
    *  <li> <code>'OBJNAME'</code> for
    *      {@link ifc.io._XPersistObject} : name of service which is
    *    represented by this object. </li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        //get GroupBoxModel
        String objName = "GroupBox";

        XControlShape shape = FormTools.insertControlShape(xDrawDoc, 5000,
                                                           7000, 2000, 2000,
                                                           objName);
        oObj = shape.getControl();

        log.println("creating a new environment for drawpage object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("adding DrawDocument as obj relation to environment");
        tEnv.addObjRelation("OBJNAME", "stardiv.one.form.component." +
                            objName);


        //adding ObjRelation for XPersistObject
        tEnv.addObjRelation("PSEUDOPERSISTENT", new Boolean(true));

        return tEnv;
    } // finish method getTestEnvironment
} // finish class OGroupBoxModel
