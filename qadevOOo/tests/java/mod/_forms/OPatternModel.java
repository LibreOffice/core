/*************************************************************************
 *
 *  $RCSfile: OPatternModel.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:51:01 $
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
import util.DBTools;
import util.FormTools;
import util.WriterTools;

import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.form.XBoundComponent;
import com.sun.star.form.XLoadable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.form.component.PatternField</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::io::XPersistObject</code></li>
 *  <li> <code>com::sun::star::form::XReset</code></li>
 *  <li> <code>com::sun::star::form::XBoundComponent</code></li>
 *  <li> <code>com::sun::star::form::FormComponent</code></li>
 *  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
 *  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
 *  <li> <code>com::sun::star::form::component::PatternField</code></li>
 *  <li> <code>com::sun::star::form::DataAwareControlModel</code></li>
 *  <li> <code>com::sun::star::form::XUpdateBroadcaster</code></li>
 *  <li> <code>com::sun::star::awt::UnoControlPatternFieldModel</code></li>
 *  <li> <code>com::sun::star::form::component::DatabasePatternField</code></li>
 *  <li> <code>com::sun::star::form::FormControlModel</code></li>
 *  <li> <code>com::sun::star::beans::XPropertyState</code></li>
 *  <li> <code>com::sun::star::container::XNamed</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::lang::XEventListener</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::form::XLoadListener</code></li>
 *  <li> <code>com::sun::star::container::XChild</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.io.XPersistObject
 * @see com.sun.star.form.XReset
 * @see com.sun.star.form.XBoundComponent
 * @see com.sun.star.form.FormComponent
 * @see com.sun.star.beans.XFastPropertySet
 * @see com.sun.star.beans.XMultiPropertySet
 * @see com.sun.star.form.component.PatternField
 * @see com.sun.star.form.DataAwareControlModel
 * @see com.sun.star.form.XUpdateBroadcaster
 * @see com.sun.star.awt.UnoControlPatternFieldModel
 * @see com.sun.star.form.component.DatabasePatternField
 * @see com.sun.star.form.FormControlModel
 * @see com.sun.star.beans.XPropertyState
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.lang.XEventListener
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.form.XLoadListener
 * @see com.sun.star.container.XChild
 * @see ifc.io._XPersistObject
 * @see ifc.form._XReset
 * @see ifc.form._XBoundComponent
 * @see ifc.form._FormComponent
 * @see ifc.beans._XFastPropertySet
 * @see ifc.beans._XMultiPropertySet
 * @see ifc.form.component._PatternField
 * @see ifc.form._DataAwareControlModel
 * @see ifc.form._XUpdateBroadcaster
 * @see ifc.awt._UnoControlPatternFieldModel
 * @see ifc.form.component._DatabasePatternField
 * @see ifc.form._FormControlModel
 * @see ifc.beans._XPropertyState
 * @see ifc.container._XNamed
 * @see ifc.lang._XComponent
 * @see ifc.lang._XEventListener
 * @see ifc.beans._XPropertySet
 * @see ifc.form._XLoadListener
 * @see ifc.container._XChild
 */
public class OPatternModel extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates Writer document where controls are placed.
    */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        log.println("creating a text document");
        xTextDoc = WriterTools.createTextDoc(((XMultiServiceFactory) tParam.getMSF()));
    }

    /**
    * Disposes Writer document.
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
    * First <code>TestDB</code> database is registered.
    * Creates Pattern field in the Form, then binds it to TestDB
    * database and returns Field's control. <p>
    *     Object relations created :
    * <ul>
    *  <li> <code>'OBJNAME'</code> for
    *      {@link ifc.io._XPersistObject} : name of service which is
    *    represented by this object. </li>
    *  <li> <code>'LC'</code> for {@link ifc.form._DataAwareControlModel}.
    *    Specifies the value for LabelControl property. It is
    *    <code>FixedText</code> component added to the document.</li>
    *  <li> <code>'FL'</code> for
    *      {@link ifc.form._DataAwareControlModel} interface.
    *    Specifies XLoadable implementation which connects form to
    *    the data source.</li>
    *  <li> <code>'XUpdateBroadcaster.Checker'</code> : <code>
    *    _XUpdateBroadcaster.UpdateChecker</code> interface implementation
    *    which can update, commit data and check if the data was successfully
    *    commited.</li>
    *  <li> <code>'DataAwareControlModel.NewFieldName'</code> : for
    *    <code>com.sun.star.form.DataAwareControlModel</code> service
    *    which contains new name of the field to bind control to.
    *  </li>
    * </ul>
    * @see ifc.form._XUpdateBroadcaster
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        String objName = "PatternField";

        //get PatternModel
        XControlShape aShape = FormTools.createControlShape(xTextDoc, 3000,
                                                            4500, 15000, 10000,
                                                            objName);

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);
        oObj = aShape.getControl();

        XLoadable formLoader = null;

        try {
            DBTools dbTools = new DBTools(((XMultiServiceFactory) Param.getMSF()));
            dbTools.registerTestDB((String) System.getProperty("DOCPTH"));

            formLoader = FormTools.bindForm(xTextDoc, "APITestDatabase",
                                            "TestDB");
        } catch (com.sun.star.uno.Exception e) {
            log.println("!!! Can't access TestDB !!!");
            e.printStackTrace(log);
            throw new StatusException("Can't access TestDB", e);
        }

        final XPropertySet ps = (XPropertySet) UnoRuntime.queryInterface(
                                        XPropertySet.class, oObj);

        try {
            ps.setPropertyValue("DataField", DBTools.TST_STRING_F);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set Default Date", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set Default Date", e);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set Default Date", e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set Default Date", e);
        }

        aShape = FormTools.createControlShape(xTextDoc, 6000, 4500, 15000,
                                              10000, "FixedText");
        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);

        log.println("creating a new environment for drawpage object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("OBJNAME", "stardiv.one.form.component." +
                            objName);


        //adding ObjRelation for XPersistObject
        tEnv.addObjRelation("PSEUDOPERSISTENT", new Boolean(true));


        // added LabelControl for 'DataAwareControlModel'
        tEnv.addObjRelation("LC", aShape.getControl());


        // added FormLoader for 'DataAwareControlModel'
        tEnv.addObjRelation("FL", formLoader);
        tEnv.addObjRelation("DataAwareControlModel.NewFieldName",
                            DBTools.TST_STRING_F);

        // adding relation for XUpdateBroadcaster
        final XInterface ctrl = oObj;
        final XLoadable formLoaderF = formLoader;

        tEnv.addObjRelation("XUpdateBroadcaster.Checker",
                            new ifc.form._XUpdateBroadcaster.UpdateChecker() {
            private String lastText = "";

            public void update() throws com.sun.star.uno.Exception {
                if (!formLoaderF.isLoaded()) {
                    formLoaderF.load();
                }

                lastText = "_" + ps.getPropertyValue("Text");
                ps.setPropertyValue("Text", lastText);
            }

            public void commit() throws com.sun.star.sdbc.SQLException {
                XBoundComponent bound = (XBoundComponent) UnoRuntime.queryInterface(
                                                XBoundComponent.class, ctrl);
                XResultSetUpdate update = (XResultSetUpdate) UnoRuntime.queryInterface(
                                                  XResultSetUpdate.class,
                                                  formLoaderF);

                bound.commit();
                update.updateRow();
            }

            public boolean wasCommited() throws com.sun.star.uno.Exception {
                formLoaderF.reload();

                String getS = (String) ps.getPropertyValue("Text");

                return lastText.equals(getS);
            }
        });

        return tEnv;
    } // finish method getTestEnvironment
} // finish class OPatternModel
