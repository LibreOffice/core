/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MasterDetailForms.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package integration.forms;

import java.lang.reflect.Method;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.*;
import com.sun.star.container.XNameContainer;
import com.sun.star.form.XLoadable;
import com.sun.star.sdbc.*;

import connectivity.tools.*;


public class MasterDetailForms extends complexlib.ComplexTestCase implements com.sun.star.form.XLoadListener
{
    private XMultiServiceFactory    m_orb;

    private HsqlDatabase            m_databaseDocument;
    private XPropertySet            m_masterForm;
    private XPropertySet            m_detailForm;
    private XResultSet              m_masterSet;
    private XResultSet              m_detailSet;
    private XRow                    m_masterRow;
    private XRow                    m_detailRow;

    private Object                  m_waitForLoad;
    private boolean                 m_loaded;

    /** Creates a new instance of ValueBinding */
    public MasterDetailForms()
    {
        m_loaded = false;
        m_waitForLoad = new Object();
    }

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkMultipleKeys"
        };
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Form Control Spreadsheet Cell Binding Test";
    }

    /* ------------------------------------------------------------------ */
    public void before() throws java.lang.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
        m_databaseDocument = new HsqlDatabase( m_orb );
        createTableStructure();
        createForms();
    }

    /* ------------------------------------------------------------------ */
    /** creates the table structure needed for the test
     */
    private void createTableStructure() throws SQLException
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
        m_databaseDocument.createTable( new HsqlTableDescriptor( "master", masterColumns ) );
        m_databaseDocument.createTable( new HsqlTableDescriptor( "detail", detailColumns ) );

        m_databaseDocument.executeSQL( "INSERT INTO \"master\" VALUES ( 1, 1, 'First Record' )" );
        m_databaseDocument.executeSQL( "INSERT INTO \"master\" VALUES ( 1, 2, 'Second Record' )" );
        m_databaseDocument.executeSQL( "INSERT INTO \"detail\" VALUES ( 1, 1, 1, 'record 1.1 (1)')");
        m_databaseDocument.executeSQL( "INSERT INTO \"detail\" VALUES ( 2, 1, 1, 'record 1.1 (2)')");
        m_databaseDocument.executeSQL( "INSERT INTO \"detail\" VALUES ( 3, 1, 2, 'record 1.2 (1)')");

        m_databaseDocument.getDataSource().refreshTables( m_databaseDocument.defaultConnection() );
    }

    /* ------------------------------------------------------------------ */
    public void createForms() throws com.sun.star.uno.Exception
    {
        m_masterForm = dbfTools.queryPropertySet( m_orb.createInstance( "com.sun.star.form.component.DataForm" ) );
        m_masterRow = (XRow)UnoRuntime.queryInterface( XRow.class, m_masterForm );
        m_masterSet = (XResultSet)UnoRuntime.queryInterface( XResultSet.class, m_masterForm );
        m_masterForm.setPropertyValue( "ActiveConnection", m_databaseDocument.defaultConnection() );
        m_masterForm.setPropertyValue( "CommandType", new Integer( com.sun.star.sdb.CommandType.TABLE ) );
        m_masterForm.setPropertyValue( "Command", "master" );

        m_detailForm = dbfTools.queryPropertySet( m_orb.createInstance( "com.sun.star.form.component.DataForm" ) );
        m_detailRow = (XRow)UnoRuntime.queryInterface( XRow.class, m_detailForm );
        m_detailSet = (XResultSet)UnoRuntime.queryInterface( XResultSet.class, m_detailForm );
        m_detailForm.setPropertyValue( "ActiveConnection", m_databaseDocument.defaultConnection() );
        m_detailForm.setPropertyValue( "CommandType", new Integer( com.sun.star.sdb.CommandType.TABLE ) );
        m_detailForm.setPropertyValue( "Command", "detail" );

        XNameContainer masterContainer = (XNameContainer)UnoRuntime.queryInterface( XNameContainer.class,
            m_masterForm );
        masterContainer.insertByName( "slave", m_detailForm );
    }

    /* ------------------------------------------------------------------ */
    public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_databaseDocument.closeAndDelete();
        dbfTools.disposeComponent( m_masterForm );
        dbfTools.disposeComponent( m_detailForm );
    }

    /* ------------------------------------------------------------------ */
    /** checks if master-detail relationships including multiple keys work
     */
    public void checkMultipleKeys() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_detailForm.setPropertyValue( "MasterFields", new String[] { "ID1", "ID2" } );
        m_detailForm.setPropertyValue( "DetailFields", new String[] { "FK_ID1", "FK_ID2" } );

        XLoadable loadMaster = (XLoadable)UnoRuntime.queryInterface( XLoadable.class, m_masterForm );
        XLoadable loadDetail = (XLoadable)UnoRuntime.queryInterface( XLoadable.class, m_detailForm );
        loadDetail.addLoadListener( this );

        // wait until the detail form is loaded
        operateMasterAndWaitForDetailForm( loadMaster.getClass().getMethod( "load", new Class[] {} ), loadMaster, new Object[] { } );

        // okay, now the master form should be on the first record
        assure( "wrong form state after loading (ID1)", m_masterRow.getInt(1) == 1 );
        assure( "wrong form state after loading (ID2)", m_masterRow.getInt(2) == 1 );
        assure( "wrong form state after loading (value)", m_masterRow.getString(3).equals( "First Record" ) );

        // the "XResultSet.next" method
        Method methodNext = m_masterSet.getClass().getMethod( "next" , new Class[] {} );

        // the values in the linked fields should be identical
        int expectedDetailRowCounts[] = { 2, 1 };
        do
        {
            verifyColumnValueIdentity( "ID1", "FK_ID1" );
            verifyColumnValueIdentity( "ID2", "FK_ID2" );

            m_detailSet.last();
            int masterPos = m_masterSet.getRow();
            assure( "wrong number of records in detail form, for master form at pos " + masterPos,
                    ((Integer)m_detailForm.getPropertyValue( "RowCount" )).intValue() == expectedDetailRowCounts[ masterPos - 1 ] );

            operateMasterAndWaitForDetailForm( methodNext, m_masterSet, new Object[] {} );
        }
        while ( !m_masterSet.isAfterLast() );
        assure( "wrong number of records in master form", 2 == ((Integer)m_masterForm.getPropertyValue( "RowCount" )).intValue() );
    }

    /** executes an operation on the master, and waits until the detail form has been (re)loaded aferwards
     */
    private void operateMasterAndWaitForDetailForm( Method _masterMethod, Object _masterInterface, Object[] _methodParameters ) throws SQLException
    {
        m_loaded = false;
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

        synchronized( m_waitForLoad )
        {
            if ( !m_loaded )
            {
                try { m_waitForLoad.wait(); }
                catch( java.lang.InterruptedException e ) { }
            }
        }
    }

    /** assures that the (integer) values in the given columns of our master and detail forms are identical
     */
    public void verifyColumnValueIdentity( String masterColName, String detailColName ) throws SQLException
    {
        XColumnLocate locateMasterCols = (XColumnLocate)UnoRuntime.queryInterface( XColumnLocate.class, m_masterForm );
        XColumnLocate locateDetailCols = (XColumnLocate)UnoRuntime.queryInterface( XColumnLocate.class, m_detailForm );

        int masterValue = m_masterRow.getInt( locateMasterCols.findColumn( masterColName ) );
        int detailValue = m_detailRow.getInt( locateDetailCols.findColumn( detailColName ) );

        assure( "values in linked column pair " + detailColName + "->" + masterColName + " (" +
            detailValue + "->" + masterValue + ") do not match (master position: " + m_masterSet.getRow()  + ")!",
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
