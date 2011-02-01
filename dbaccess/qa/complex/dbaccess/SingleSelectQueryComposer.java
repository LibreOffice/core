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
package complex.dbaccess;

import com.sun.star.beans.PropertyState;
import com.sun.star.sdb.SQLFilterOperator;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertyContainer;
import com.sun.star.beans.NamedValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.sdb.XParametersSupplier;
import com.sun.star.beans.PropertyValue;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.container.XIndexAccess;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.sdbc.DataType;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

// ---------- junit imports -----------------
import org.junit.Test;
import static org.junit.Assert.*;
// ------------------------------------------

public class SingleSelectQueryComposer extends CRMBasedTestCase
{

    private XSingleSelectQueryComposer m_composer = null;
    private final static String COMPLEXFILTER = "( \"ID\" = 1 AND \"Postal\" = '4' )" +
            " OR ( \"ID\" = 2 AND \"Postal\" = '5' )" +
            " OR ( \"ID\" = '3' AND \"Postal\" = '6' AND \"Address\" = '7' )" +
            " OR ( \"Address\" = '8' )" +
            " OR ( \"Postal\" = '9' )";
    private final static String INNERPRODUCTSQUERY = "products (inner)";

    // --------------------------------------------------------------------------------------------------------
    private void createQueries() throws Exception
    {
        m_database.getDatabase().getDataSource().createQuery(INNERPRODUCTSQUERY, "SELECT * FROM \"products\"");
    }

    // --------------------------------------------------------------------------------------------------------
    @Override
    protected void createTestCase()
    {
        try
        {
            super.createTestCase();

            createQueries();

            m_composer = createQueryComposer();

        }
        catch (Exception e)
        {
            fail("caught an exception (" + e.getMessage() + ") while creating the test case");
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private void checkAttributeAccess(String _attributeName, String _attributeValue)
    {
        System.out.println("setting " + _attributeName + " to " + _attributeValue);
        String realValue = null;
        try
        {
            final Class composerClass = m_composer.getClass();
            final Method attributeGetter = composerClass.getMethod("get" + _attributeName, new Class[]
                    {
                    });
            final Method attributeSetter = composerClass.getMethod("set" + _attributeName, new Class[]
                    {
                        String.class
                    });

            attributeSetter.invoke(m_composer, new Object[]
                    {
                        _attributeValue
                    });
            realValue = (String) attributeGetter.invoke(m_composer, new Object[]
                    {
                    });
        }
        catch (NoSuchMethodException e)
        {
        }
        catch (IllegalAccessException e)
        {
        }
        catch (InvocationTargetException e)
        {
        }
        assertTrue("set/get" + _attributeName + " not working as expected (set: " + _attributeValue + ", get: " + (realValue != null ? realValue : "null") + ")",
                realValue.equals(_attributeValue));
        System.out.println("  (results in " + m_composer.getQuery() + ")");
    }

    /** tests setCommand of the composer
     */
    @Test
    public void testSetCommand()
    {
        System.out.println("testing SingleSelectQueryComposer's setCommand");

        try
        {
            final String table = "SELECT * FROM \"customers\"";
            m_composer.setCommand("customers",CommandType.TABLE);
            assertTrue("setCommand/getQuery TABLE inconsistent", m_composer.getQuery().equals(table));

            m_database.getDatabase().getDataSource().createQuery("set command test", "SELECT * FROM \"orders for customer\" \"a\", \"customers\" \"b\" WHERE \"a\".\"Product Name\" = \"b\".\"Name\"");
            m_composer.setCommand("set command test",CommandType.QUERY);
            assertTrue("setCommand/getQuery QUERY inconsistent", m_composer.getQuery().equals(m_database.getDatabase().getDataSource().getQueryDefinition("set command test").getCommand()));

            final String sql = "SELECT * FROM \"orders for customer\" WHERE \"Product Name\" = 'test'";
            m_composer.setCommand(sql,CommandType.COMMAND);
            assertTrue("setCommand/getQuery COMMAND inconsistent", m_composer.getQuery().equals(sql));
        }
        catch (Exception e)
        {
            fail("Exception caught: " + e);
        }
    }
    /** tests accessing attributes of the composer (order, filter, group by, having)
     */
    @Test
    public void testAttributes()
    {
        System.out.println("testing SingleSelectQueryComposer's attributes (order, filter, group by, having)");

        try
        {
            System.out.println("check setElementaryQuery");
            final String simpleQuery2 = "SELECT * FROM \"customers\" WHERE \"Name\" = 'oranges'";
            m_composer.setElementaryQuery(simpleQuery2);
            assertTrue("setElementaryQuery/getQuery inconsistent", m_composer.getQuery().equals(simpleQuery2));

            System.out.println("check setQuery");
            final String simpleQuery = "SELECT * FROM \"customers\"";
            m_composer.setQuery(simpleQuery);
            assertTrue("set/getQuery inconsistent", m_composer.getQuery().equals(simpleQuery));

            checkAttributeAccess("Filter", "\"Name\" = 'oranges'");
            checkAttributeAccess("Group", "\"City\"");
            checkAttributeAccess("Order", "\"Address\"");
            checkAttributeAccess("HavingClause", "\"ID\" <> 4");

            final XIndexAccess orderColumns = m_composer.getOrderColumns();
            assertTrue("Order columns doesn't exist: \"Address\"",
                    orderColumns != null && orderColumns.getCount() == 1 && orderColumns.getByIndex(0) != null);

            final XIndexAccess groupColumns = m_composer.getGroupColumns();
            assertTrue("Group columns doesn't exist: \"City\"",
                    groupColumns != null && groupColumns.getCount() == 1 && groupColumns.getByIndex(0) != null);

            // XColumnsSupplier
            final XColumnsSupplier xSelectColumns = UnoRuntime.queryInterface(XColumnsSupplier.class, m_composer);
            assertTrue("no select columns, or wrong number of select columns",
                    xSelectColumns != null && xSelectColumns.getColumns() != null && xSelectColumns.getColumns().getElementNames().length == 6);

            // structured filter
            m_composer.setQuery("SELECT \"ID\", \"Postal\", \"Address\" FROM \"customers\"");
            m_composer.setFilter(COMPLEXFILTER);
            final PropertyValue[][] aStructuredFilter = m_composer.getStructuredFilter();
            m_composer.setFilter("");
            m_composer.setStructuredFilter(aStructuredFilter);
            assertTrue("Structured Filter not identical", m_composer.getFilter().equals(COMPLEXFILTER));

            // structured having clause
            m_composer.setHavingClause(COMPLEXFILTER);
            final PropertyValue[][] aStructuredHaving = m_composer.getStructuredHavingClause();
            m_composer.setHavingClause("");
            m_composer.setStructuredHavingClause(aStructuredHaving);
            assertTrue("Structured Having Clause not identical", m_composer.getHavingClause().equals(COMPLEXFILTER));
        }
        catch (Exception e)
        {
            fail("Exception caught: " + e);
        }
    }

    /** test various sub query related features ("queries in queries")
     */
    @Test
    public void testSubQueries() throws Exception
    {
        m_composer.setQuery("SELECT * from \"" + INNERPRODUCTSQUERY + "\"");
        final XTablesSupplier suppTables = UnoRuntime.queryInterface(XTablesSupplier.class, m_composer);
        final XNameAccess tables = suppTables.getTables();
        assertTrue("a simple SELECT * FROM <query> could not be parsed",
                tables != null && tables.hasByName(INNERPRODUCTSQUERY));

        final String sInnerCommand = m_database.getDatabase().getDataSource().getQueryDefinition(INNERPRODUCTSQUERY).getCommand();
        final String sExecutableQuery = m_composer.getQueryWithSubstitution();
        assertTrue("simple query containing a sub query improperly parsed to SDBC level statement: \n1. " + sExecutableQuery + "\n2. " + "SELECT * FROM ( " + sInnerCommand + " ) AS \"" + INNERPRODUCTSQUERY + "\"",
                sExecutableQuery.equals("SELECT * FROM ( " + sInnerCommand + " ) AS \"" + INNERPRODUCTSQUERY + "\""));
    }

    /** tests the XParametersSupplier functionality
     */
    @Test
    public void testParameters()
    {
        try
        {
            // "orders for customers" is a query with a named parameter (based on another query)
            m_database.getDatabase().getDataSource().createQuery("orders for customer", "SELECT * FROM \"all orders\" WHERE \"Customer Name\" LIKE :cname");
            // "orders for customer and product" is query based on "orders for customers", adding an additional,
            // anonymous parameter
            m_database.getDatabase().getDataSource().createQuery("orders for customer and product", "SELECT * FROM \"orders for customer\" WHERE \"Product Name\" LIKE ?");

            m_composer.setQuery(m_database.getDatabase().getDataSource().getQueryDefinition("orders for customer and product").getCommand());
            final XParametersSupplier suppParams = UnoRuntime.queryInterface(XParametersSupplier.class, m_composer);
            final XIndexAccess parameters = suppParams.getParameters();

            final String expectedParamNames[] =
            {
                "cname",
                "Product Name"
            };

            final int paramCount = parameters.getCount();
            assertTrue("composer did find wrong number of parameters in the nested queries.",
                    paramCount == expectedParamNames.length);

            for (int i = 0; i < paramCount; ++i)
            {
                final XPropertySet parameter = UnoRuntime.queryInterface(XPropertySet.class, parameters.getByIndex(i));
                final String paramName = (String) parameter.getPropertyValue("Name");
                assertTrue("wrong parameter name at position " + (i + 1) + " (expected: " + expectedParamNames[i] + ", found: " + paramName + ")",
                        paramName.equals(expectedParamNames[i]));

            }
        }
        catch (Exception e)
        {
            fail("caught an exception: " + e);
        }
    }

    @Test
    public void testConditionByColumn()
    {
        try
        {
            m_composer.setQuery("SELECT * FROM \"customers\"");

            final Object initArgs[] =
            {
                new NamedValue("AutomaticAddition", Boolean.valueOf(true))
            };
            final String serviceName = "com.sun.star.beans.PropertyBag";
            final XPropertyContainer filter = UnoRuntime.queryInterface(XPropertyContainer.class, getMSF().createInstanceWithArguments(serviceName, initArgs));
            filter.addProperty("Name", PropertyAttribute.MAYBEVOID, "Comment");
            filter.addProperty("RealName", PropertyAttribute.MAYBEVOID, "Comment");
            filter.addProperty("TableName", PropertyAttribute.MAYBEVOID, "customers");
            filter.addProperty("Value", PropertyAttribute.MAYBEVOID, "Good one.");
            filter.addProperty("Type", PropertyAttribute.MAYBEVOID, Integer.valueOf(DataType.LONGVARCHAR));
            final XPropertySet column = UnoRuntime.queryInterface(XPropertySet.class, filter);

            m_composer.appendFilterByColumn(column, true,SQLFilterOperator.LIKE);
            assertTrue("At least one row should exist",m_database.getConnection().createStatement().executeQuery(m_composer.getQuery()).next());

        }
        catch (Exception e)
        {
            // this is an error: the query is expected to be parseable
            fail("caught an exception: " + e);
        }
    }

    private void impl_testDisjunctiveNormalForm(String _query, PropertyValue[][] _expectedDNF)
    {
        try
        {
            m_composer.setQuery(_query);
        }
        catch (Exception e)
        {
            // this is an error: the query is expected to be parseable
            fail("caught an exception: " + e);
        }

        final PropertyValue[][] disjunctiveNormalForm = m_composer.getStructuredFilter();

        assertEquals("DNF: wrong number of rows", _expectedDNF.length, disjunctiveNormalForm.length);
        for (int i = 0; i < _expectedDNF.length; ++i)
        {
            assertEquals("DNF: wrong number of columns in row " + i, _expectedDNF[i].length, disjunctiveNormalForm[i].length);
            for (int j = 0; j < _expectedDNF[i].length; ++j)
            {
                assertEquals("DNF: wrong content in column " + j + ", row " + i,
                        _expectedDNF[i][j].Name, disjunctiveNormalForm[i][j].Name);
            }
        }
    }

    /** tests the disjunctive normal form functionality, aka the structured filter,
     *  of the composer
     */
    @Test
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

        impl_testDisjunctiveNormalForm(query, new PropertyValue[][]
                {
                    new PropertyValue[]
                    {
                        new PropertyValue("CustomerID", SQLFilterOperator.EQUAL, "\"customers\".\"ID\"", PropertyState.DIRECT_VALUE),
                        new PropertyValue("OrderID", SQLFilterOperator.EQUAL, "\"orders\".\"ID\"", PropertyState.DIRECT_VALUE),
                        new PropertyValue("ProductID", SQLFilterOperator.EQUAL, "\"products\".\"ID\"", PropertyState.DIRECT_VALUE)
                    }
                });

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

        impl_testDisjunctiveNormalForm(query, new PropertyValue[][]
                {
                    new PropertyValue[]
                    {
                        new PropertyValue("CustomerID", SQLFilterOperator.EQUAL, "\"customers\".\"ID\"", PropertyState.DIRECT_VALUE),
                        new PropertyValue("OrderID", SQLFilterOperator.EQUAL, "\"orders\".\"ID\"", PropertyState.DIRECT_VALUE),
                        new PropertyValue("ProductID", SQLFilterOperator.EQUAL, "\"products\".\"ID\"", PropertyState.DIRECT_VALUE),
                        new PropertyValue("Name", SQLFilterOperator.EQUAL, "Apples", PropertyState.DIRECT_VALUE)
                    },
                    new PropertyValue[]
                    {
                        new PropertyValue("CustomerID", SQLFilterOperator.EQUAL, "\"customers\".\"ID\"", PropertyState.DIRECT_VALUE),
                        new PropertyValue("OrderID", SQLFilterOperator.EQUAL, "\"orders\".\"ID\"", PropertyState.DIRECT_VALUE),
                        new PropertyValue("ProductID", SQLFilterOperator.EQUAL, "\"products\".\"ID\"", PropertyState.DIRECT_VALUE),
                        new PropertyValue("ID", SQLFilterOperator.EQUAL, Integer.valueOf(2), PropertyState.DIRECT_VALUE)
                    }
                });

    }
}
