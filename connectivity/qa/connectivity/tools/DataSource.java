/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataSource.java,v $
 * $Revision: 1.3 $
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

package connectivity.tools;

import com.sun.star.container.ElementExistException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.sdb.XQueryDefinitionsSupplier;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XRefreshable;

public class DataSource
{
    // the service factory
    XMultiServiceFactory    m_orb;
    XDataSource             m_dataSource;

    public DataSource( XMultiServiceFactory _orb, XDataSource _dataSource )
    {
        m_orb = _orb;
        m_dataSource = _dataSource;
    }

    /** creates a query with a given name and SQL command
    */
    public void createQuery( String _name, String _sqlCommand ) throws ElementExistException, WrappedTargetException, com.sun.star.lang.IllegalArgumentException
    {
        createQuery( _name, _sqlCommand, true );
    }

    /** creates a query with a given name, SQL command, and EscapeProcessing flag
    */
    public void createQuery( String _name, String _sqlCommand, boolean _escapeProcessing ) throws ElementExistException, WrappedTargetException, com.sun.star.lang.IllegalArgumentException
    {
        XSingleServiceFactory queryDefsFac = (XSingleServiceFactory)UnoRuntime.queryInterface(
            XSingleServiceFactory.class, getQueryDefinitions() );
        XPropertySet queryDef = null;
        try
        {
            queryDef = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, queryDefsFac.createInstance() );
            queryDef.setPropertyValue( "Command", _sqlCommand );
            queryDef.setPropertyValue( "EscapeProcessing", new Boolean( _escapeProcessing ) );
        }
        catch( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.err );
        }

        XNameContainer queryDefsContainer = (XNameContainer)UnoRuntime.queryInterface(
            XNameContainer.class, getQueryDefinitions() );
        queryDefsContainer.insertByName( _name, queryDef );
    }

    /** provides the query definition with the given name
     */
    public QueryDefinition getQueryDefinition( String _name ) throws NoSuchElementException
    {
        XNameAccess allDefs = getQueryDefinitions();
        try
        {
            return new QueryDefinition(
                (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, allDefs.getByName( _name ) ) );
        }
        catch ( WrappedTargetException e )
        {
        }
        throw new NoSuchElementException();
    }

    /** provides the container of query definitions of the data source
     */
    public XNameAccess getQueryDefinitions()
    {
        XQueryDefinitionsSupplier suppQueries = (XQueryDefinitionsSupplier)UnoRuntime.queryInterface(
            XQueryDefinitionsSupplier.class, m_dataSource );
        return suppQueries.getQueryDefinitions();
    }

    /** refreshs the table container of a given connection
     *
     *  This is usually necessary if you created tables by directly executing SQL statements,
     *  bypassing the SDBCX layer.
     */
    public void refreshTables( com.sun.star.sdbc.XConnection _connection )
    {
        XTablesSupplier suppTables = (XTablesSupplier)UnoRuntime.queryInterface(
            XTablesSupplier.class, _connection );
        XRefreshable refreshTables = (XRefreshable)UnoRuntime.queryInterface(
            XRefreshable.class, suppTables.getTables() );
        refreshTables.refresh();
    }
};
