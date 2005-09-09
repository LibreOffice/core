/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OButtonModel.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:06:55 $
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

package mod._forms;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.WriterTools;

import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


/**
* Test for object which is represented by service
* <code>com.sun.star.com.sun.star.form.component.CommandButton</code>.
* <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XPersistObject</code></li>
*  <li> <code>com::sun::star::container::XChild</code></li>
*  <li> <code>com::sun::star::form::FormControlModel</code></li>
*  <li> <code>com::sun::star::form::XImageProducerSupplier</code></li>
*  <li> <code>com::sun::star::form::FormComponent</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
*  <li> <code>com::sun::star::beans::XPropertyState</code></li>
*  <li> <code>com::sun::star::awt::UnoControlButtonModel</code></li>
*  <li> <code>com::sun::star::form::component::CommandButton</code></li>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
*
* @see com.sun.star.io.XPersistObject
* @see com.sun.star.container.XChild
* @see com.sun.star.form.FormControlModel
* @see com.sun.star.form.XImageProducerSupplier
* @see com.sun.star.form.FormComponent
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.beans.XFastPropertySet
* @see com.sun.star.beans.XPropertyState
* @see com.sun.star.awt.UnoControlButtonModel
* @see com.sun.star.form.component.CommandButton
* @see com.sun.star.container.XNamed
* @see com.sun.star.beans.XMultiPropertySet
* @see com.sun.star.lang.XComponent
* @see ifc.io._XPersistObject
* @see ifc.container._XChild
* @see ifc.form._FormControlModel
* @see ifc.form._XImageProducerSupplier
* @see ifc.form._FormComponent
* @see ifc.beans._XPropertySet
* @see ifc.beans._XFastPropertySet
* @see ifc.beans._XPropertyState
* @see ifc.awt._UnoControlButtonModel
* @see ifc.form.component._CommandButton
* @see ifc.container._XNamed
* @see ifc.beans._XMultiPropertySet
* @see ifc.lang._XComponent
*/
public class OButtonModel extends TestCase {

    XTextDocument xTextDoc;

    /**
    * Creates StarWriter document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        log.println( "creating a textdocument" );
        xTextDoc = WriterTools.createTextDoc(((XMultiServiceFactory) tParam.getMSF()));
    }

    /**
    * Disposes StarWriter document.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                                        XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }


    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Adds button into text and retrieves it's control model.
    *     Object relations created :
    * <ul>
    *  <li> <code>'OBJNAME'</code> for
    *      {@link ifc.io._XPersistObject} : name of service which is
    *    represented by this object. </li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        XControlShape aShape = FormTools.createControlShape(
                                xTextDoc,3000,4500,15000,10000,"CommandButton");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);
        oObj = aShape.getControl();
        log.println( "creating a new environment for OButtonModel object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );
        String objName = "CommandButton";
        tEnv.addObjRelation("OBJNAME", "stardiv.one.form.component." + objName);
        //adding ObjRelation for XPersistObject
        tEnv.addObjRelation("PSEUDOPERSISTENT", new Boolean(true));
        return tEnv;
    } // finish method getTestEnvironment

}    // finish class OButtonModel

