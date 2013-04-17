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

package integration.forms;

import com.sun.star.beans.NamedValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexContainer;
import java.lang.reflect.Method;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.container.XNameContainer;
import com.sun.star.embed.XComponentSupplier;
import com.sun.star.form.XGridColumnFactory;
import com.sun.star.form.XGridFieldDataSupplier;
import com.sun.star.form.XLoadable;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XFormDocumentsSupplier;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XColumnLocate;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.uno.Type;
import com.sun.star.util.XModifiable;
import connectivity.tools.CRMDatabase;
import connectivity.tools.HsqlColumnDescriptor;
import connectivity.tools.HsqlDatabase;
import connectivity.tools.HsqlTableDescriptor;
import org.openoffice.complex.forms.tools.ResultSet;


public class MasterDetailForms extends complexlib.ComplexTestCase implements com.sun.star.form.XLoadListener
{
    private XMultiServiceFactory    m_orb;

    private XPropertySet            m_masterForm;
    private XPropertySet            m_detailForm;
    private ResultSet               m_masterResult;
    private ResultSet               m_detailResult;

    final private Object            m_waitForLoad = new Object();
    private boolean                 m_loaded = false;

    /** Creates a new instance of MasterDetailForms */
    public MasterDetailForms()
    {
    }

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkMultipleKeys",
            "checkDetailFormDefaults"
        };
    }

    /* ------------------------------------------------------------------ */
    public void before()
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Form Control Spreadsheet Cell Binding Test";
    }

    /* ------------------------------------------------------------------ */
    /** creates the table structure needed for the test
     */
    private void impl_createTableStructure( final HsqlDatabase _databaseDocument ) throws SQLException
    {
        HsqlColumnDescriptor[] masterColumns = {
            new HsqlColumnDescriptor( "ID1", "INTEGER", HsqlColumnDescriptor.PRIMARY ),
            new HsqlColumnDescriptor( "ID2", "INTEGER", HsqlColumnDescriptor.PRIMARY ),
            new HsqlColumnDescriptor( "value", "VARCHAR(50)" ),
        };
        HsqlColumnDescriptor[] detailColumns = {
            new HsqlColumnDescriptor( "ID", "INTEGER", HsqlColumnDescriptor.PRIMARY ),
            new HsqlColumnDescriptor( "FK_ID1", "INTEGER", HsqlColumnDescriptor.REQUIRED, "master", "ID1" ),
            new HsqlColumnDescriptor( "FK_ID2", "INTEGER", HsqlColumnDescriptor.REQUIRED, "master", "ID2" ),
            new HsqlColumnDescriptor( "name", "VARCHAR(50)" ),
        };
        _databaseDocument.createTable( new HsqlTableDescriptor( "master", masterColumns ) );
        _databaseDocument.createTable( new HsqlTableDescriptor( "detail", detailColumns ) );

        _databaseDocument.executeSQL( "INSERT INTO \"master\" VALUES ( 1, 1, 'First Record' )" );
        _databaseDocument.executeSQL( "INSERT INTO \"master\" VALUES ( 1, 2, 'Second Record' )" );
        _databaseDocument.executeSQL( "INSERT INTO \"detail\" VALUES ( 1, 1, 1, 'record 1.1 (1)')");
        _databaseDocument.executeSQL( "INSERT INTO \"detail\" VALUES ( 2, 1, 1, 'record 1.1 (2)')");
        _databaseDocument.executeSQL( "INSERT INTO \"detail\" VALUES ( 3, 1, 2, 'record 1.2 (1)')");

        _databaseDocument.defaultConnection().refreshTables();
    }

    /* ------------------------------------------------------------------ */
    private void impl_createForms( final HsqlDatabase _databaseDocument ) throws com.sun.star.uno.Exception
    {
        m_masterForm = dbfTools.queryPropertySet( m_orb.createInstance( "com.sun.star.form.component.DataForm" ) );
        m_masterForm.setPropertyValue( "ActiveConnection", _databaseDocument.defaultConnection().getXConnection() );
        m_masterForm.setPropertyValue( "CommandType", new Integer( com.sun.star.sdb.CommandType.TABLE ) );
        m_masterForm.setPropertyValue( "Command", "master" );

        m_masterResult = new ResultSet( m_masterForm );

        m_detailForm = dbfTools.queryPropertySet( m_orb.createInstance( "com.sun.star.form.component.DataForm" ) );
        m_detailForm.setPropertyValue( "ActiveConnection", _databaseDocument.defaultConnection().getXConnection() );
        m_detailForm.setPropertyValue( "CommandType", new Integer( com.sun.star.sdb.CommandType.TABLE ) );
        m_detailForm.setPropertyValue( "Command", "detail" );

        m_detailResult = new ResultSet( m_detailForm );

        XNameContainer masterContainer = UnoRuntime.queryInterface( XNameContainer.class, m_masterForm );
        masterContainer.insertByName( "slave", m_detailForm );
    }

    /* ------------------------------------------------------------------ */
    /** checks if master-detail relationships including multiple keys work
     */
    public void checkMultipleKeys() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        HsqlDatabase databaseDocument = null;
        try
        {
            databaseDocument = new HsqlDatabase( m_orb );
            impl_createTableStructure( databaseDocument );
            impl_createForms( databaseDocument );

            m_detailForm.setPropertyValue( "MasterFields", new String[] { "ID1", "ID2" } );
            m_detailForm.setPropertyValue( "DetailFields", new String[] { "FK_ID1", "FK_ID2" } );

            XLoadable loadMaster = UnoRuntime.queryInterface( XLoadable.class, m_masterForm );
            XLoadable loadDetail = UnoRuntime.queryInterface( XLoadable.class, m_detailForm );
            loadDetail.addLoadListener( this );

            // wait until the detail form is loaded
            operateMasterAndWaitForDetailForm( loadMaster.getClass().getMethod( "load", new Class[] {} ), loadMaster, new Object[] { } );

            // okay, now the master form should be on the first record
            assure( "wrong form state after loading (ID1)", m_masterResult.getInt(1) == 1 );
            assure( "wrong form state after loading (ID2)", m_masterResult.getInt(2) == 1 );
            assure( "wrong form state after loading (value)", m_masterResult.getString(3).equals( "First Record" ) );

            // the "XResultSet.next" method
            Method methodNext = m_masterResult.getClass().getMethod( "next" , new Class[] {} );

            // the values in the linked fields should be identical
            int expectedDetailRowCounts[] = { 2, 1 };
            do
            {
                verifyColumnValueIdentity( "ID1", "FK_ID1" );
                verifyColumnValueIdentity( "ID2", "FK_ID2" );

                m_detailResult.last();
                int masterPos = m_masterResult.getRow();
                assure( "wrong number of records in detail form, for master form at pos " + masterPos,
                        ((Integer)m_detailForm.getPropertyValue( "RowCount" )).intValue() == expectedDetailRowCounts[ masterPos - 1 ] );

                operateMasterAndWaitForDetailForm( methodNext, m_masterResult, new Object[] {} );
            }
            while ( !m_masterResult.isAfterLast() );
            assure( "wrong number of records in master form", 2 == ((Integer)m_masterForm.getPropertyValue( "RowCount" )).intValue() );
        }
        finally
        {
            if ( databaseDocument != null )
                databaseDocument.closeAndDelete();
            impl_cleanUpStep();
        }
    }

    /* ------------------------------------------------------------------ */
    private final void impl_cleanUpStep()
    {
        if ( m_masterForm != null )
            dbfTools.disposeComponent( m_masterForm );
        if ( m_detailForm != null )
            dbfTools.disposeComponent( m_detailForm );
        m_masterForm = m_detailForm = null;
    }

    /* ------------------------------------------------------------------ */
    /** checks whether default values in detail forms work as expected.
     *
     * Effectively, this test case verifies the issues #i106574# and #i105235# did not creep back in.
     */
    public void checkDetailFormDefaults() throws Exception
    {
        CRMDatabase database = null;
        XCommandProcessor subComponentCommands = null;
        try
        {
            // create our standard CRM database document
            database = new CRMDatabase( m_orb, true );

            // create a form document therein
            XFormDocumentsSupplier formDocSupp = UnoRuntime.queryInterface( XFormDocumentsSupplier.class, database.getDatabase().getModel() );
            XMultiServiceFactory formFactory = UnoRuntime.queryInterface( XMultiServiceFactory.class, formDocSupp.getFormDocuments() );
            NamedValue[] loadArgs = new NamedValue[] {
                new NamedValue( "ActiveConnection", database.getConnection().getXConnection() ),
                new NamedValue( "MediaType", "application/vnd.oasis.opendocument.text" )
            };

            subComponentCommands = UnoRuntime.queryInterface(
                XCommandProcessor.class,
                formFactory.createInstanceWithArguments( "com.sun.star.sdb.DocumentDefinition", loadArgs ) );
            Command command = new Command();
            command.Name = "openDesign";
            command.Argument = new Short( OpenMode.DOCUMENT );

            DocumentHelper subDocument = new DocumentHelper( m_orb,
                UnoRuntime.queryInterface( XComponent.class,
                    subComponentCommands.execute( command, subComponentCommands.createCommandIdentifier(), null )
                )
            );
            FormLayer formLayer = new FormLayer( subDocument );
            XPropertySet controlModel = formLayer.insertControlLine( "DatabaseNumericField", "ID",          "", 10 );
                                        formLayer.insertControlLine( "DatabaseTextField",    "Name",        "", 20 );
                                        formLayer.insertControlLine( "DatabaseTextField",    "Description", "", 30 );

            m_masterForm = (XPropertySet)dbfTools.getParent( controlModel, XPropertySet.class );
            m_masterForm.setPropertyValue( "Command", "categories" );
            m_masterForm.setPropertyValue( "CommandType", new Integer( CommandType.TABLE ) );

            // create a detail form
            m_detailForm = UnoRuntime.queryInterface( XPropertySet.class, subDocument.createSubForm( m_masterForm, "products" ) );
            m_detailForm.setPropertyValue( "Command", "SELECT \"ID\", \"Name\", \"CategoryID\" FROM \"products\"" );
            m_detailForm.setPropertyValue( "CommandType", new Integer( CommandType.COMMAND ) );
            m_detailForm.setPropertyValue( "MasterFields", new String[] { "ID" } );
            m_detailForm.setPropertyValue( "DetailFields", new String[] { "CategoryID" } );

            // create a grid control in the detail form, with some columns
            XPropertySet gridControlModel = formLayer.createControlAndShape( "GridControl", 20, 40, 130, 50, m_detailForm );
            gridControlModel.setPropertyValue( "Name", "product list" );
            XIndexContainer gridColumns = UnoRuntime.queryInterface( XIndexContainer.class, gridControlModel );
                                      impl_createGridColumn( gridColumns, "TextField", "ID" );
            XPropertySet nameColumn = impl_createGridColumn( gridColumns, "TextField", "Name" );
            nameColumn.setPropertyValue( "Width", new Integer( 600 ) ); // 6 cm
            nameColumn.setPropertyValue( "DefaultText", "default text" );

            // go live
            m_masterResult = new ResultSet( m_masterForm );
            m_detailResult = new ResultSet( m_detailForm );

            XLoadable loadDetail = UnoRuntime.queryInterface( XLoadable.class, m_detailForm );
            loadDetail.addLoadListener( this );

            subDocument.getCurrentView().toggleFormDesignMode();
            impl_waitForLoadedEvent();

            // now that we set up this, do the actual tests
            // First, http://www.openoffice.org/issues/show_bug.cgi?id=105235 described the problem
            // that default values in the sub form didn't work when the master form was navigated to a row
            // for which no detail records were present, and the default of the column/control is the same
            // as the last known value.

            // so, take the current value of the "Name" column, and set it as default value ...
            String defaultValue = m_detailResult.getString( 2 );
            nameColumn.setPropertyValue( "DefaultText", defaultValue );
            // ... then move to the second main form row ...
            m_masterResult.absolute( 2 );
            impl_waitForLoadedEvent();
            // ... which should result in an empty sub form ...
            assure( "test precondition not met: The second master form record is expected to have no detail records, " +
                "else the test becomes meaningless", impl_isNewRecord( m_detailForm ) );
            // ... and in the "Name" column having the proper text
            String actualValue = (String)nameColumn.getPropertyValue( "Text" );
            assureEquals( "#i105235#: default value in sub form not working (not propagated to column model)", defaultValue, actualValue );
            // However, checking the column model's value alone is not enough - we need to ensure it is properly
            // propagated to the control.
            XGridFieldDataSupplier gridData = subDocument.getCurrentView().getControl(
                gridControlModel, XGridFieldDataSupplier.class );
            actualValue = (String)(gridData.queryFieldData( 0, Type.STRING )[1]);
            assureEquals( "#i105235#: default value in sub form not working (not propagated to column)", defaultValue, actualValue );
        }
        finally
        {
            if ( subComponentCommands != null )
            {
                XComponentSupplier componentSupplier = UnoRuntime.queryInterface( XComponentSupplier.class, subComponentCommands );
                XModifiable modifySubComponent = UnoRuntime.queryInterface( XModifiable.class, componentSupplier.getComponent() );
                modifySubComponent.setModified( false );
                Command command = new Command();
                command.Name = "close";
                subComponentCommands.execute( command, subComponentCommands.createCommandIdentifier(), null );
            }

            if ( database != null )
                database.saveAndClose();

            impl_cleanUpStep();
        }
    }

    /* ------------------------------------------------------------------ */
    private boolean impl_isNewRecord( final XPropertySet _rowSet )
    {
        boolean isNew = false;
        try
        {
            isNew = ((Boolean)_rowSet.getPropertyValue( "IsNew" )).booleanValue();
        }
        catch ( Exception ex )
        {
            failed( "obtaining the IsNew property failed" );
        }
        return isNew;
    }

    /* ------------------------------------------------------------------ */
    private XPropertySet impl_createGridColumn( final XIndexContainer _gridModel, final String _columnType, final String _boundField ) throws Exception
    {
        final XGridColumnFactory columnFactory = UnoRuntime.queryInterface( XGridColumnFactory.class, _gridModel );
        XPropertySet column = columnFactory.createColumn( _columnType );
        column.setPropertyValue( "DataField", _boundField );
        column.setPropertyValue( "Name", _boundField );
        column.setPropertyValue( "Label", _boundField );
        _gridModel.insertByIndex( _gridModel.getCount(), column );
        return column;
    }

    /* ------------------------------------------------------------------ */
    /** executes an operation on the master, and waits until the detail form has been (re)loaded aferwards
     */
    private void operateMasterAndWaitForDetailForm( Method _masterMethod, Object _masterInterface, Object[] _methodParameters ) throws SQLException
    {
        Object result;
        try
        {
            result = _masterMethod.invoke( _masterInterface, _methodParameters );
        }
        catch( java.lang.Exception e )
        {
            throw new SQLException( "invoking " + _masterMethod.getName() + " failed",new Object(), "", 0, new Object() );
        }

        if ( _masterMethod.getReturnType().getName().equals( "boolean" ) )
            if ( !((Boolean)result).booleanValue() )
                return;

        impl_waitForLoadedEvent();
    }

    private void impl_waitForLoadedEvent()
    {
        synchronized( m_waitForLoad )
        {
            while ( !m_loaded )
            {
                try { m_waitForLoad.wait(); }
                catch( java.lang.InterruptedException e ) { }
            }
            // reset the flag for the next time
            m_loaded = false;
        }
    }

    /** assures that the (integer) values in the given columns of our master and detail forms are identical
     */
    private void verifyColumnValueIdentity( final String masterColName, final String detailColName ) throws SQLException
    {
        XColumnLocate locateMasterCols = UnoRuntime.queryInterface( XColumnLocate.class, m_masterForm );
        XColumnLocate locateDetailCols = UnoRuntime.queryInterface( XColumnLocate.class, m_detailForm );

        int masterValue = m_masterResult.getInt( locateMasterCols.findColumn( masterColName ) );
        int detailValue = m_detailResult.getInt( locateDetailCols.findColumn( detailColName ) );

        assure( "values in linked column pair " + detailColName + "->" + masterColName + " (" +
            detailValue + "->" + masterValue + ") do not match (master position: " + m_masterResult.getRow()  + ")!",
            masterValue == detailValue );
    }

    public void disposing(com.sun.star.lang.EventObject eventObject)
    {
    }

    public void loaded(com.sun.star.lang.EventObject eventObject)
    {
        synchronized( m_waitForLoad )
        {
            m_loaded = true;
            m_waitForLoad.notify();
        }
    }

    public void reloaded(com.sun.star.lang.EventObject eventObject)
    {
        synchronized( m_waitForLoad )
        {
            m_loaded = true;
            m_waitForLoad.notify();
        }
    }

    public void reloading(com.sun.star.lang.EventObject eventObject)
    {
    }

    public void unloaded(com.sun.star.lang.EventObject eventObject)
    {
    }

    public void unloading(com.sun.star.lang.EventObject eventObject)
    {
    }
}
