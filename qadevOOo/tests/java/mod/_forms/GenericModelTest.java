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
package mod._forms;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
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
import com.sun.star.form.XBoundComponent;
import com.sun.star.form.XForm;
import com.sun.star.form.XLoadable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import java.util.ArrayList;
import lib.Status;
import util.utils;


/**
* Test for object which is represented by service
* <code>com.sun.star.form.component.DateField</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XPersistObject</code></li>
*  <li> <code>com::sun::star::awt::UnoControlDateFieldModel</code></li>
*  <li> <code>com::sun::star::form::XReset</code></li>
*  <li> <code>com::sun::star::form::XBoundComponent</code></li>
*  <li> <code>com::sun::star::form::FormComponent</code></li>
*  <li> <code>com::sun::star::form::component::DateField</code></li>
*  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
*  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
*  <li> <code>com::sun::star::form::XUpdateBroadcaster</code></li>
*  <li> <code>com::sun::star::form::DataAwareControlModel</code></li>
*  <li> <code>com::sun::star::beans::XPropertyState</code></li>
*  <li> <code>com::sun::star::form::FormControlModel</code></li>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::lang::XEventListener</code></li>
*  <li> <code>com::sun::star::beans::XPropertyAccess</code></li>
*  <li> <code>com::sun::star::beans::XPropertyContainer</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::form::XLoadListener</code></li>
*  <li> <code>com::sun::star::container::XChild</code></li>
* </ul>
* The following files used by this test :
* <ul>
*  <li><b> TestDB </b> (directory) : directory with test database </li>
*  <li><b> TestDB/TestDB.dbf </b> : table file. See
*    {@link util.DBTools DBTools} class for more information.</li>
* </ul> <p>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.io.XPersistObject
* @see com.sun.star.awt.UnoControlDateFieldModel
* @see com.sun.star.form.XReset
* @see com.sun.star.form.XBoundComponent
* @see com.sun.star.form.FormComponent
* @see com.sun.star.form.component.DateField
* @see com.sun.star.beans.XFastPropertySet
* @see com.sun.star.beans.XMultiPropertySet
* @see com.sun.star.form.XUpdateBroadcaster
* @see com.sun.star.form.DataAwareControlModel
* @see com.sun.star.beans.XPropertyState
* @see com.sun.star.form
* @see com.sun.star.container.XNamed
* @see com.sun.star.lang.XComponent
* @see com.sun.star.lang.XEventListener
* @see com.sun.star.beans.XPropertyAccess
* @see com.sun.star.beans.XPropertyContainer
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.form.XLoadListener
* @see com.sun.star.container.XChild
* @see ifc.io._XPersistObject
* @see ifc.awt._UnoControlDateFieldModel
* @see ifc.form._XReset
* @see ifc.form._XBoundComponent
* @see ifc.form._FormComponent
* @see ifc.form.component._DateField
* @see ifc.beans._XFastPropertySet
* @see ifc.beans._XMultiPropertySet
* @see ifc.form._XUpdateBroadcaster
* @see ifc.form._DataAwareControlModel
* @see ifc.beans._XPropertyState
* @see ifc.form._FormControlModel
* @see ifc.container._XNamed
* @see ifc.lang._XComponent
* @see ifc.lang._XEventListener
* @see ifc.beans._XPropertySet
* @see ifc.form._XLoadListener
* @see ifc.container._XChild
*/
public class GenericModelTest extends TestCase {
    private static XTextDocument m_xTextDoc;
    private static Object m_dbSrc = null;
    private static DBTools.DataSourceInfo m_srcInf = null;
    /**
     * This is the name of the Data Base which the test uses: "APITestDatabase"
     */
    protected final static String m_dbSourceName = "APITestDatabase";
    protected final static String m_TestDB = "TestDB";
    private DBTools m_dbTools = null;

    private static boolean m_ConnectionColsed = false;

    /**
     * descibes the kind of the shape which should be created.
     * Example: m_kindOfshape=DateFiled
     */
    public static String m_kindOfControl = null;

    /**
     * If your object needs some special propery values you can specify them with this
     * <CODE>ArrayList</CODE>. You have to add a <CODE>NamedValue</CODE> to this list.
     * Example:
     * NamedValue myProp = new NamedValue();
     * myProp.Name = "Test";
     * myProp.Value = "My special Value";
     * m_propertiesToSet.add(myProp);
     */
    public static ArrayList<NamedValue> m_propertiesToSet = new ArrayList<NamedValue>();

    /**
     * This variable contains the name of the property which should be changed while
     * interface <CODE>com::sun::star::form::XUpdateBroadcaster</CODE> is tested. The
     * interface test needs the <CODE>ObjectRelation</CODE>
     * "XUpdateBroadcaster.Checker" which is a <CODE>ifc.form._XUpdateBroadcaster.UpdateChecker</CODE>.
     * @see ifc.form._XUpdateBroadcaster.UpdateChecker
     * @see ifc.form._XUpdateBroadcaster
     */
    public static String m_ChangePropertyName = null;
    /**
     * This variable contains the value the property should be set while
     * interface <CODE>com::sun::star::form::XUpdateBroadcaster</CODE> is tested.
     * The interface test needs the <CODE>ObjectRelation</CODE>
     * "XUpdateBroadcaster.Checker" which is a <CODE>ifc.form._XUpdateBroadcaster.UpdateChecker</CODE>.
     * Normaly the <CODE>Checker</CODE> uses <CODE>util.ValueChanger</CODE> to change
     * the value of the property. If the current of this property is NULL the
     * <CODE>ValueChanger</CODE> is unable to change the value. In this case the value
     * of this variable was used.
     */
    public static Object m_ChangePropertyValue = null;

    /**
     * This variable contains the implelemtation name of the object.
     */
    public static String m_ObjectName = null;

    /**
     * For local implementaions of <CODE>Checker</CODE> this variable contains the
     * <CODE>FormLoader</CODE>
     */
    protected static XLoadable m_XFormLoader = null;
    /**
     * For local implementaions of <CODE>Checker</CODE> this variable contains the
     * <CODE>XPropertySet</CODE>
     */
    protected static XPropertySet m_XPS = null;
    /**
     * For local implementaions of <CODE>Checker</CODE> this variable contains the
     * <CODE>Control</CODE>
     */
    protected static XInterface m_XCtrl = null;
    /**
     * The insterface test of <CODE>ifc.form._DataWareControlModel</CODE> expects an
     * object relation <CODE>'LC'</CODE>. This is a <CODE>XControlModel</CODE> of a shape.
     * This variable contains the kind of shape to create for the interface test,
     * f.e. "FixedText"
     * @see ifc.form._DataAwareControlModel
     */
    protected static String m_LCShape_Type = null;

    protected static String m_XPropertyAccess_propertyToChange = "HelpText";

    protected static String m_XPropertyContainer_propertyNotRemovable = "HelpText";
    /**
     * If this variable is true some more debug info was logged. It was setted by the parameter variable
     * <code>debug_is_active</code>
     */
    protected static boolean debug = false;

    /**
     * Creates Writer document where controls are placed.
     * @param tParam the test paremter
     * @param log the log writer
     */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        log.println("creating a textdocument");
        m_xTextDoc = WriterTools.createTextDoc(((XMultiServiceFactory) tParam.getMSF()));
        m_ConnectionColsed = false;
        debug = tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE);
        m_propertiesToSet.clear();
    }

    /**
     * close the connection
     * close the data source
     * close the document
     * revoke the data source
     * @param tParam the test parameter
     * @param log the log writer
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("closing connection...");

        // some interface tests call cleanup to reset the environment. If such
        // a test is the last one cleanup was called twice. The second call
        // causes then nasty exceptions...
        if (m_ConnectionColsed) return;

        try {
            XIndexAccess forms = UnoRuntime.queryInterface( XIndexAccess.class,
                FormTools.getForms( WriterTools.getDrawPage( m_xTextDoc ) ) );
            XForm myForm = (XForm) AnyConverter.toObject(new Type(XForm.class),
                                                 forms.getByIndex(0));

            if (debug){
                if (myForm == null){
                    log.println("ERROR: could not get 'Standard' from drawpage!");
                }
                log.println("the draw page contains folowing elemtens:");
                String[] elements = FormTools.getForms(WriterTools.getDrawPage(m_xTextDoc)).getElementNames();
                for (int i = 0; i< elements.length; i++){
                    log.println("Element[" + i + "] :" + elements[i]);
                }

            }

            XPropertySet xSetProp = UnoRuntime.queryInterface( XPropertySet.class, myForm );
            XConnection connection = UnoRuntime.queryInterface( XConnection.class, xSetProp.getPropertyValue( "ActiveConnection" ) );
            if ( connection == null )
            {
                if ( debug )
                    log.println("ERROR: could not get property 'ActiveConnection' from the XForm");
            }
            else
            {
                connection.close();
            }
        } catch (Exception e) {
            log.println("ERROR: Can't close the connection: " + e.toString());
            e.printStackTrace( log );
        }

        log.println("closing data source...");
        try {
            XCloseable closer = UnoRuntime.queryInterface(
                                        XCloseable.class, m_dbSrc);
            if ( closer == null )
            {
                XDocumentDataSource dataSource = UnoRuntime.queryInterface(
                    XDocumentDataSource.class, m_dbSrc);
                if ( dataSource != null )
                    closer = UnoRuntime.queryInterface(
                        XCloseable.class, dataSource.getDatabaseDocument() );
            }
            if (debug && closer==null){
                log.println("ERROR: couldn't get 'XCloseable' from DataSource");
            }
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("ERROR: couldn't close data source: " + e.toString());
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("ERROR: couldn't close data source: " + e.toString());
        } catch (Exception e) {
            log.println("ERROR: couldn't close data source: " + e.toString());
        }

        log.println("disposing data source...");
        try {
            XComponent dataSourceComp = UnoRuntime.queryInterface(
                XComponent.class, m_dbSrc);
            dataSourceComp.dispose();
        }
        catch (Exception e) {
            log.println("couldn't dispose the data source");
        }

        log.println("closing document...");

        try {
            XCloseable closer = UnoRuntime.queryInterface(
                                        XCloseable.class, m_xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("ERROR: couldn't close document: " + e.toString());
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("ERROR: couldn't close document: " + e.toString());
        } catch (Exception e) {
            log.println("ERROR: couldn't close document: " + e.toString());
        }

        log.println("revoking data source...");
        try {
            m_dbTools.revokeDB(m_dbSourceName);
        } catch (com.sun.star.container.NoSuchElementException e){
        } catch (com.sun.star.uno.Exception e) {
            log.println("ERROR: Error while object test cleaning up: " + e.toString());
        }

        m_ConnectionColsed = true;
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * First <code>TestDB</code> database is registered.
     * Creates DateField in the Form, then binds it to TestDB
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
     *    which contains new name of the field ('_DATE') to bind control to.
     *  </li>
     *  <li> <code>'XFastPropertySet.ExcludeProps'</code> : for
     *    <code>com.sun.star.beans.XFastPropertySet</code> interface
     *    the property FormatKey can have only restricted set of values.
     *  </li>
     * </ul>
     * @see ifc.form._XUpdateBroadcaster
     * @param Param the test parameter
     * @param log the log writer
     * @return a test environment
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;
        XControlShape aShape = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory) Param.getMSF();

        try{
            log.println("adding contol shape '" + m_kindOfControl + "'");
            aShape = FormTools.createControlShape(m_xTextDoc, 3000,
                                                            4500, 15000, 10000,
                                                            m_kindOfControl);
        } catch (Exception e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't create following control shape (m_kindOfControl): '" +
                                        m_kindOfControl + "': ", e);

        }

        WriterTools.getDrawPage(m_xTextDoc).add(aShape);
        oObj = aShape.getControl();

        log.println("Implementation name: " + util.utils.getImplName(oObj));

        try {
            String sourceTestDB = utils.getFullURL(utils.getFullTestDocName("TestDB/testDB.dbf"));
            String destTestDB = utils.getOfficeTemp(xMSF);
            destTestDB = utils.getFullURL(destTestDB + "testDB.dbf");

            log.println("copy '"+sourceTestDB + "' -> '" + destTestDB + "'");
            utils.copyFile(xMSF, sourceTestDB, destTestDB);

            m_dbTools = new DBTools( xMSF, log );
            String tmpDir = utils.getOfficeTemp((xMSF));

            m_srcInf = m_dbTools.newDataSourceInfo();
            m_srcInf.URL = "sdbc:dbase:" + DBTools.dirToUrl(tmpDir);
            log.println("data source: " + m_srcInf.URL);

            m_dbSrc = m_srcInf.getDataSourceService();
            m_dbTools.reRegisterDB(m_dbSourceName, m_dbSrc);

            m_XFormLoader = FormTools.bindForm(m_xTextDoc, m_dbSourceName,
                                            m_TestDB);
        } catch (com.sun.star.uno.Exception e) {
            log.println("!!! Can't access TestDB !!!");
            e.printStackTrace(log);
            throw new StatusException("Can't access TestDB", e);
        }

        log.println("creating a new environment for object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("OBJNAME", m_ObjectName);

        log.println("adding shape '" + m_LCShape_Type +"' for DataAwareControlModel test");
        aShape = FormTools.createControlShape(m_xTextDoc, 6000, 4500, 15000,
                                              10000, m_LCShape_Type);
        WriterTools.getDrawPage(m_xTextDoc).add(aShape);

        m_XPS = UnoRuntime.queryInterface(
                                        XPropertySet.class, oObj);

        int i = 0;
        NamedValue prop = null;
        try {
            for (i = 0; i < m_propertiesToSet.size(); i++){
                prop = m_propertiesToSet.get(i);

                log.println("setting property: '"+prop.Name+"' to value '"+prop.Value.toString()+"'");

                m_XPS.setPropertyValue(prop.Name, prop.Value);
            }
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property '" + prop.Name + "': ", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property '" + prop.Name + "': ", e);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property '" + prop.Name + "': ", e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property '" + prop.Name + "': ", e);
        } catch (java.lang.ClassCastException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get property on index '" + i + "': ", e);
        }

        // added LabelControl for 'DataAwareControlModel'
        tEnv.addObjRelation("LC", aShape.getControl());

        // added FormLoader for 'DataAwareControlModel'
        tEnv.addObjRelation("FL", m_XFormLoader);

        // adding relation for XUpdateBroadcaster
        m_XCtrl = oObj;

    tEnv.addObjRelation("XUpdateBroadcaster.Checker",
                            new Checker(m_XFormLoader, m_XPS, m_XCtrl, m_ChangePropertyName, m_ChangePropertyValue));

        // adding relation for DataAwareControlModel service
        tEnv.addObjRelation("DataAwareControlModel.NewFieldName",
                            DBTools.TST_DATE_F);

        //adding ObjRelation for XPersistObject
        tEnv.addObjRelation("PSEUDOPERSISTENT", new Boolean(true));

        // adding relation for XFastPropertySet
        java.util.HashSet<String> exclude = new java.util.HashSet<String>();
        exclude.add("FormatKey");
        tEnv.addObjRelation("XFastPropertySet.ExcludeProps", exclude);

        PropertyValue propVal = new PropertyValue();
        propVal.Name = m_XPropertyAccess_propertyToChange;
        propVal.Value = "Text since XPropertyAccess";
        tEnv.addObjRelation("XPropertyAccess.propertyToChange", propVal);
        tEnv.addObjRelation("XPropertyContainer.propertyNotRemovable", m_XPropertyContainer_propertyNotRemovable);


        return tEnv;
    } // finish method getTestEnvironment


    static class Checker implements ifc.form._XUpdateBroadcaster.UpdateChecker {
            private Object lastValue = null;
            XLoadable formLoaderF = null;
            XPropertySet ps = null;
            XInterface ctrl = null;
            String ChangePropertyName = null;
            Object ChangePropertyValue = null;

            public Checker(XLoadable xl, XPropertySet ps, XInterface ctrl, String ChangePropertyName, Object ChangePropertyValue) {
                formLoaderF = xl;
                this.ps = ps;
                this.ctrl = ctrl;
                this.ChangePropertyName=ChangePropertyName;
                this.ChangePropertyValue=ChangePropertyValue;
            }

            public void update() throws com.sun.star.uno.Exception {
                if (!formLoaderF.isLoaded()) {
                    formLoaderF.load();
                }

                lastValue = util.ValueChanger.changePValue(ps.getPropertyValue(ChangePropertyName));

                if (lastValue == null){

                    if (ChangePropertyValue != null){

                        lastValue = ChangePropertyValue;

                    } else {

                        String msg = "The initial value of the property '" + ChangePropertyName + "' is NULL\n";
                        msg += "The member variable 'm_ChangePropertyValue' is NULL\n";
                        msg += "Could not change Property.";

                        throw new StatusException(Status.failed(msg));
                    }
                }
                ps.setPropertyValue(ChangePropertyName, lastValue);
            }

            public void commit() throws com.sun.star.sdbc.SQLException {
                XBoundComponent bound = UnoRuntime.queryInterface(
                                                XBoundComponent.class, ctrl);
                XResultSetUpdate update = UnoRuntime.queryInterface(
                                                  XResultSetUpdate.class,
                                                  formLoaderF);

                bound.commit();
                update.updateRow();
            }

            public boolean wasCommited() throws com.sun.star.uno.Exception {
                formLoaderF.reload();

                Object newValue = ps.getPropertyValue(ChangePropertyName);

                return (newValue != null) && (util.ValueComparer.equalValue(lastValue, newValue));
            }
        }
} // finish class GenericModelTest
