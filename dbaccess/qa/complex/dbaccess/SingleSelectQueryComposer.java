/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SingleSelectQueryComposer.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:27:15 $
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
package complex.dbaccess;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.*;
import com.sun.star.sdbcx.*;
import com.sun.star.sdbc.*;
import com.sun.star.sdb.*;
import com.sun.star.container.*;
import com.sun.star.lang.XMultiServiceFactory;

import complexlib.ComplexTestCase;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class SingleSelectQueryComposer extends ComplexTestCase
{
    private CRMDatabase                 m_database;
    private XSingleSelectQueryComposer  m_composer;

    private final String complexFilter = "( \"ID\" = 1 AND \"Postal\" = '4' )" +
                                         " OR ( \"ID\" = 2 AND \"Postal\" = '5' )" +
                                         " OR ( \"ID\" = '3' AND \"Postal\" = '6' AND \"Address\" = '7' )" +
                                         " OR ( \"Address\" = '8' )" +
                                         " OR ( \"Postal\" = '9' )";
    private final String innerProductsQuery = "products (inner)";

    // --------------------------------------------------------------------------------------------------------
    public String[] getTestMethodNames()
    {
        return new String[] {
            "testAttributes",
            "testSubQueries",
            "testParameters",
            "testDisjunctiveNormalForm"
        };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return "SingleSelectQueryComposer";
    }

    // --------------------------------------------------------------------------------------------------------
    private final XMultiServiceFactory getORB()
    {
        return (XMultiServiceFactory)param.getMSF();
    }

    // --------------------------------------------------------------------------------------------------------
    private void createQueries() throws Exception
    {
        m_database.getDatabase().getDataSource().createQuery( innerProductsQuery, "SELECT * FROM \"products\"" );
    }

    // --------------------------------------------------------------------------------------------------------
    public void before()
    {
        createTestCase();
    }

    // --------------------------------------------------------------------------------------------------------
    public void after()
    {
        if ( m_database != null )
            m_database.close();
    }

    // --------------------------------------------------------------------------------------------------------
    private void createTestCase()
    {
        try
        {
            m_database = new CRMDatabase( getORB() );

            createQueries();

            XMultiServiceFactory connectionFactory = (XMultiServiceFactory)UnoRuntime.queryInterface(
                XMultiServiceFactory.class, m_database.getConnection() );
            m_composer = (XSingleSelectQueryComposer)UnoRuntime.queryInterface(
                XSingleSelectQueryComposer.class, connectionFactory.createInstance( "com.sun.star.sdb.SingleSelectQueryComposer" ) );

        }
        catch ( Exception e )
        {
            assure( "caught an exception (" + e.getMessage() + ") while creating the test case", false );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private void checkAttributeAccess( String _attributeName, String _attributeValue )
    {
        log.println( "setting " + _attributeName + " to " + _attributeValue );
        String realValue = null;
        try
        {
            Class composerClass = m_composer.getClass();
            Method attributeGetter = composerClass.getMethod( "get" + _attributeName, new Class[] {} );
            Method attributeSetter = composerClass.getMethod( "set" + _attributeName, new Class[] { String.class } );

            attributeSetter.invoke( m_composer, new Object[] { _attributeValue } );
            realValue = (String)attributeGetter.invoke( m_composer, new Object[] {} );
        }
        catch ( NoSuchMethodException e ) { }
        catch ( IllegalAccessException e ) { }
        catch ( InvocationTargetException e ) { }
        assure( "set/get" + _attributeName + " not working as expected (set: " + _attributeValue + ", get: " + ( realValue != null ? realValue : "null" ) + ")",
            realValue.equals( _attributeValue ) );
        log.println( "  (results in " + (String)m_composer.getQuery() + ")" );
    }

    /** tests accessing attributes of the composer (order, filter, group by, having)
     */
    public void testAttributes()
    {
        log.println("testing SingleSelectQueryComposer's attributes (order, filter, group by, having)");

    try
    {
            log.println("check setQuery");
            final String simpleQuery = "SELECT * FROM \"customers\"";
            m_composer.setQuery( simpleQuery );
            assure( "set/getQuery inconsistent", m_composer.getQuery().equals( simpleQuery ) );

            checkAttributeAccess( "Filter", "\"Name\" = 'oranges'" );
            checkAttributeAccess( "Group", "\"City\"" );
            checkAttributeAccess( "Order", "\"Address\"" );
            checkAttributeAccess( "HavingClause", "\"ID\" <> 4" );

            XIndexAccess orderColumns = m_composer.getOrderColumns();
            assure( "Order columns doesn't exist: \"Address\"",
                orderColumns != null && orderColumns.getCount() == 1 && orderColumns.getByIndex(0) != null );

            XIndexAccess groupColumns = m_composer.getGroupColumns();
            assure( "Group columns doesn't exist: \"City\"",
                groupColumns != null && groupColumns.getCount() == 1 && groupColumns.getByIndex(0) != null );

            // XColumnsSupplier
            XColumnsSupplier xSelectColumns = (XColumnsSupplier)
                    UnoRuntime.queryInterface(XColumnsSupplier.class,m_composer);
            assure( "no select columns, or wrong number of select columns",
                xSelectColumns != null && xSelectColumns.getColumns() != null && xSelectColumns.getColumns().getElementNames().length == 5 );

            // structured filter
            m_composer.setQuery("SELECT \"ID\", \"Postal\", \"Address\" FROM \"customers\"");
            m_composer.setFilter(complexFilter);
            PropertyValue[][] aStructuredFilter = m_composer.getStructuredFilter();
            m_composer.setFilter("");
            m_composer.setStructuredFilter(aStructuredFilter);
            assure("Structured Filter not identical" , m_composer.getFilter().equals(complexFilter));

            // structured having clause
            m_composer.setHavingClause(complexFilter);
            PropertyValue[][] aStructuredHaving = m_composer.getStructuredHavingClause();
            m_composer.setHavingClause("");
            m_composer.setStructuredHavingClause(aStructuredHaving);
            assure("Structured Having Clause not identical" , m_composer.getHavingClause().equals(complexFilter));
     }
    catch(Exception e)
    {
            assure("Exception caught: " + e,false);
    }
    }

    /** test various sub query related features ("queries in queries")
     */
    public void testSubQueries() throws Exception
    {
        m_composer.setQuery( "SELECT * from \"" + innerProductsQuery + "\"" );
        XTablesSupplier suppTables = (XTablesSupplier)UnoRuntime.queryInterface(
            XTablesSupplier.class, m_composer );
        XNameAccess tables = suppTables.getTables();
        assure( "a simple SELECT * FROM <query> could not be parsed",
            tables != null && tables.hasByName( innerProductsQuery ) );

        String sInnerCommand = m_database.getDatabase().getDataSource().getQueryDefinition( innerProductsQuery ).getCommand();
        String sExecutableQuery = m_composer.getQueryWithSubstitution();
        assure( "simple query containing a sub query improperly parsed to SDBC level statement",
            sExecutableQuery.equals( "SELECT * FROM ( " + sInnerCommand  + " ) AS \"" + innerProductsQuery + "\"") );
    }

    /** tests the XParametersSupplier functionality
     */
    public void testParameters()
    {
        try
        {
            // "orders for customers" is a query with a named parameter (based on another query)
            m_database.getDatabase().getDataSource().createQuery( "orders for customer", "SELECT * FROM \"all orders\" WHERE \"Customer Name\" LIKE :cname" );
            // "orders for customer and product" is query based on "orders for customers", adding an additional,
            // anonymous parameter
            m_database.getDatabase().getDataSource().createQuery( "orders for customer and product", "SELECT * FROM \"orders for customer\" WHERE \"Product Name\" LIKE ?" );

            m_composer.setQuery( m_database.getDatabase().getDataSource().getQueryDefinition( "orders for customer and product" ).getCommand() );
            XParametersSupplier suppParams = (XParametersSupplier)UnoRuntime.queryInterface(
                XParametersSupplier.class, m_composer );
            XIndexAccess parameters = suppParams.getParameters();

            String expectedParamNames[] = {
                "cname",
                "Product Name"
            };

            int paramCount = parameters.getCount();
            assure( "composer did find wrong number of parameters in the nested queries.",
                paramCount == expectedParamNames.length );

            for ( int i = 0; i < paramCount; ++i )
            {
                XPropertySet parameter = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, parameters.getByIndex(i) );
                String paramName = (String)parameter.getPropertyValue( "Name" );
                assure( "wrong parameter name at position " + ( i + 1 ) + " (expected: " + expectedParamNames[i] + ", found: " + paramName + ")",
                    paramName.equals( expectedParamNames[i] ) );

            }
        }
        catch( Exception e )
        {
            assure( "caught an exception: " + e, false );
        }
    }

    private void impl_testDisjunctiveNormalForm( String _query, PropertyValue[][] _expectedDNF )
    {
        try { m_composer.setQuery( _query ); }
        catch ( Exception e )
        {
            // this is an error: the query is expected to be parseable
            assure( "caught an exception: " + e, false );
        }

        PropertyValue[][] disjunctiveNormalForm = m_composer.getStructuredFilter();

        assureEquals( "DNF: wrong number of rows", _expectedDNF.length, disjunctiveNormalForm.length );
        for ( int i=0; i<_expectedDNF.length; ++i )
        {
            assureEquals( "DNF: wrong number of columns in row " + i, _expectedDNF[i].length, disjunctiveNormalForm[i].length );
            for ( int j=0; j<_expectedDNF[i].length; ++j )
            {
                assureEquals( "DNF: wrong content in column " + j + ", row " + i,
                    _expectedDNF[i][j].Name, disjunctiveNormalForm[i][j].Name );
            }
        }
    }

    /** tests the disjunctive normal form functionality, aka the structured filter,
     *  of the composer
     */
    public void testDisjunctiveNormalForm()
    {
        // a simple case: WHERE clause simply is a combination of predicates knitted with AND
        String query =
            "SELECT \"customers\".\"Name\", " +
                   "\"customers\".\"Address\", " +
                   "\"customers\".\"City\", " +
                   "\"customers\".\"Postal\", " +
                   "\"products\".\"Name\" " +
            "FROM \"orders\", \"customers\", \"orders_details\", \"products\" " +
            "WHERE (   \"orders\".\"CustomerID\" = \"customers\".\"ID\" " +
                  "AND \"orders_details\".\"OrderID\" = \"orders\".\"ID\" " +
                  "AND \"orders_details\".\"ProductID\" = \"products\".\"ID\" " +
                  ") ";

        impl_testDisjunctiveNormalForm( query, new PropertyValue[][] {
            new PropertyValue[] {
                new PropertyValue( "CustomerID", SQLFilterOperator.EQUAL, "\"customers\".\"ID\"", PropertyState.DIRECT_VALUE ),
                new PropertyValue( "OrderID", SQLFilterOperator.EQUAL, "\"orders\".\"ID\"", PropertyState.DIRECT_VALUE ),
                new PropertyValue( "ProductID", SQLFilterOperator.EQUAL, "\"products\".\"ID\"", PropertyState.DIRECT_VALUE )
            } }
        );

        // somewhat more challenging: One of the conjunction terms is a disjunction itself
        query =
            "SELECT \"customers\".\"Name\", " +
                   "\"customers\".\"Address\", " +
                   "\"customers\".\"City\", " +
                   "\"customers\".\"Postal\", " +
                   "\"products\".\"Name\" " +
            "FROM \"orders\", \"customers\", \"orders_details\", \"products\" " +
            "WHERE (   \"orders\".\"CustomerID\" = \"customers\".\"ID\" " +
                  "AND \"orders_details\".\"OrderID\" = \"orders\".\"ID\" " +
                  "AND \"orders_details\".\"ProductID\" = \"products\".\"ID\" " +
                  ") " +
                  "AND " +
                  "(  \"products\".\"Name\" = 'Apples' " +
                  "OR \"products\".\"ID\" = 2 " +
                  ")";

        impl_testDisjunctiveNormalForm( query, new PropertyValue[][] {
            new PropertyValue[] {
                new PropertyValue( "CustomerID", SQLFilterOperator.EQUAL, "\"customers\".\"ID\"", PropertyState.DIRECT_VALUE ),
                new PropertyValue( "OrderID", SQLFilterOperator.EQUAL, "\"orders\".\"ID\"", PropertyState.DIRECT_VALUE ),
                new PropertyValue( "ProductID", SQLFilterOperator.EQUAL, "\"products\".\"ID\"", PropertyState.DIRECT_VALUE ),
                new PropertyValue( "Name", SQLFilterOperator.EQUAL, "Apples", PropertyState.DIRECT_VALUE )
            },
            new PropertyValue[] {
                new PropertyValue( "CustomerID", SQLFilterOperator.EQUAL, "\"customers\".\"ID\"", PropertyState.DIRECT_VALUE ),
                new PropertyValue( "OrderID", SQLFilterOperator.EQUAL, "\"orders\".\"ID\"", PropertyState.DIRECT_VALUE ),
                new PropertyValue( "ProductID", SQLFilterOperator.EQUAL, "\"products\".\"ID\"", PropertyState.DIRECT_VALUE ),
                new PropertyValue( "ID", SQLFilterOperator.EQUAL, new Integer(2), PropertyState.DIRECT_VALUE )
            } }
        );

    }
}
