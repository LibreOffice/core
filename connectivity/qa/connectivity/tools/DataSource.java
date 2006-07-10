/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSource.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:17:42 $
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
