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

    private XDataSource m_dataSource;

    public DataSource(final XMultiServiceFactory _orb, final String _registeredName) throws Exception
    {
        final XNameAccess dbContext = UnoRuntime.queryInterface(
            XNameAccess.class, _orb.createInstance( "com.sun.star.sdb.DatabaseContext" ) );

        m_dataSource = UnoRuntime.queryInterface( XDataSource.class, dbContext.getByName( _registeredName ) );
    }

    public DataSource(final XDataSource _dataSource)
    {
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
}
