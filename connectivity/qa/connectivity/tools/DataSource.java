/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import java.util.logging.Level;
import java.util.logging.Logger;

public class DataSource
{
    // the service factory

    private final XMultiServiceFactory m_orb;
    private XDataSource m_dataSource;

    public DataSource(final XMultiServiceFactory _orb, final String _registeredName) throws Exception
    {
        m_orb = _orb;

        final XNameAccess dbContext = UnoRuntime.queryInterface(
            XNameAccess.class, _orb.createInstance( "com.sun.star.sdb.DatabaseContext" ) );

        m_dataSource = UnoRuntime.queryInterface( XDataSource.class, dbContext.getByName( _registeredName ) );
    }

    public DataSource(final XMultiServiceFactory _orb,final XDataSource _dataSource)
    {
        m_orb = _orb;
        m_dataSource = _dataSource;
    }

    final public XDataSource getXDataSource()
    {
        return m_dataSource;
    }

    /**
     * retrieves the data source's settings
     */
    public XPropertySet geSettings()
    {
        return UnoRuntime.queryInterface( XPropertySet.class, impl_getPropertyValue( "Settings" ) );
    }

    /** creates a query with a given name and SQL command
     */
    public void createQuery(final String _name, final String _sqlCommand) throws ElementExistException, WrappedTargetException, com.sun.star.lang.IllegalArgumentException
    {
        createQuery(_name, _sqlCommand, true);
    }

    /** creates a query with a given name, SQL command, and EscapeProcessing flag
     */
    public void createQuery(final String _name, final String _sqlCommand, final boolean _escapeProcessing) throws ElementExistException, WrappedTargetException, com.sun.star.lang.IllegalArgumentException
    {
        final XSingleServiceFactory queryDefsFac = UnoRuntime.queryInterface( XSingleServiceFactory.class, getQueryDefinitions() );
        XPropertySet queryDef = null;
        try
        {
            queryDef = UnoRuntime.queryInterface( XPropertySet.class, queryDefsFac.createInstance() );
            queryDef.setPropertyValue("Command", _sqlCommand);
            queryDef.setPropertyValue("EscapeProcessing", Boolean.valueOf(_escapeProcessing));
        }
        catch (com.sun.star.uno.Exception e)
        {
            e.printStackTrace(System.err);
        }

        final XNameContainer queryDefsContainer = UnoRuntime.queryInterface( XNameContainer.class, getQueryDefinitions() );
        queryDefsContainer.insertByName(_name, queryDef);
    }

    /** provides the query definition with the given name
     */
    public QueryDefinition getQueryDefinition(final String _name) throws NoSuchElementException
    {
        final XNameAccess allDefs = getQueryDefinitions();
        try
        {
            return new QueryDefinition( UnoRuntime.queryInterface( XPropertySet.class, allDefs.getByName( _name) ) );
        }
        catch (WrappedTargetException e)
        {
        }
        throw new NoSuchElementException();
    }

    /** provides the container of query definitions of the data source
     */
    public XNameAccess getQueryDefinitions()
    {
        final XQueryDefinitionsSupplier suppQueries = UnoRuntime.queryInterface(
                XQueryDefinitionsSupplier.class, m_dataSource);
        return suppQueries.getQueryDefinitions();
    }

    /**
     * retrieves a property value from the data source
     * @param i_propertyName
     *      the name of the property whose value is to be returned.
     */
    private Object impl_getPropertyValue( final String i_propertyName )
    {
        Object propertyValue = null;
        try
        {
            final XPropertySet dataSourceProps = UnoRuntime.queryInterface( XPropertySet.class, m_dataSource );
            propertyValue = dataSourceProps.getPropertyValue( i_propertyName );
        }
        catch (Exception ex)
        {
            Logger.getLogger(DataSource.class.getName()).log(Level.SEVERE, null, ex);
        }
        return propertyValue;
    }

    /** returns the name of the data source
     *
     * If a data source is registered at the database context, the name is the registration
     * name. Otherwise, its the URL which the respective database document is based on.
     *
     * Note that the above definition is from the UNO API, not from this wrapper here.
     */
    public String getName()
    {
        return (String)impl_getPropertyValue( "Name" );
    }
};
