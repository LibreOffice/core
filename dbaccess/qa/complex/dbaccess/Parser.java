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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.sdb.XParametersSupplier;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;


// ---------- junit imports -----------------
import org.junit.Test;
import static org.junit.Assert.*;
// ------------------------------------------

public class Parser extends CRMBasedTestCase
{
    // --------------------------------------------------------------------------------------------------------
    @Override
    protected void createTestCase()
    {
        try
        {
            super.createTestCase();
            m_database.getDatabase().getDataSource().createQuery( "query products", "SELECT * FROM \"products\"" );
        }
        catch ( Exception e )
        {
            e.printStackTrace( System.err );
            fail( "caught an exception (" + e.getMessage() + ") while creating the test case");
        }
    }

    // --------------------------------------------------------------------------------------------------------
    @Test
    public void checkWhere() throws Exception
    {
        final XSingleSelectQueryComposer composer = createQueryComposer();
        final String SELECT = "SELECT \"products\".\"Name\" FROM \"products\" WHERE ";
        final String[] queries = new String[]
        {
             "\"ID\" in ( 1,2,3,4)"
            ,"not ( \"ID\" in ( 1,2,3,4))"
            ,"(1 = 1) is true"
            ,"(1 = 1) is not false"
            ,"(1 = 1) is not null"
            ,"not ( (1 = 1) is not null)"
            ,"'a' like 'a%'"
            ,"not ( 'a' like 'a%')"
            ,"'a' not like 'a%'"
            ,"1 between 0 and 2"
            ,"not ( 1 between 0 and 2 )"
            ,"1 not between 3 and 4"
            ,"1 not between ( select \"ID\" from \"categories\") and ( select \"ID\" from \"categories\")"
            ,"1 = 1"
            ,"0 < 1"
            ,"not(0 < 1)"
            ,"1 > 0"
            ,"not(1 > 0)"
            ,"1 <> 0"
            ,"(1 <> 0 and 'a' = 'a' and 'c' = 'd') or (1 = 1 and 2 = 2 and 3 = 4)"
            ,"not ( 1 <> 0 )"
            ,"\"CategoryID\" in ( select \"ID\" from \"categories\")"
            ,"not (\"CategoryID\" in ( select \"ID\" from \"categories\"))"
            ,"\"CategoryID\" not in ( select \"ID\" from \"categories\")"
        };
        for (int i = 0; i < queries.length; i++)
        {
            composer.setQuery( SELECT + queries[i]);
        }
    }
    // --------------------------------------------------------------------------------------------------------
    /** verifies that aliases for inner queries work as expected
     */
    @Test
    public void checkJoinSyntax() throws Exception
    {
        final XSingleSelectQueryComposer composer = createQueryComposer();

        // feed the composer with some statements. If any of those cannot be parsed, the composer
        // will throw an exception - which is a regression then
        composer.setQuery(
            "SELECT \"categories\".\"Name\", " +
                    "\"products\".\"Name\" " +
            "FROM \"products\" RIGHT OUTER JOIN \"categories\" AS \"categories\" ON \"products\".\"CategoryID\" = \"categories\".\"ID\"" );

        composer.setQuery(
            "SELECT \"categories\".\"Name\", " +
                    "\"products\".\"Name\" " +
            "FROM \"products\" LEFT OUTER JOIN \"categories\" AS \"categories\" ON \"products\".\"CategoryID\" = \"categories\".\"ID\"" );

        composer.setQuery(
            "SELECT \"categories\".\"Name\", " +
                    "\"products\".\"Name\" " +
            "FROM \"products\" CROSS JOIN \"categories\" AS \"categories\"" );

        composer.setQuery(
            "SELECT \"categories\".\"Name\", " +
                    "\"products\".\"Name\" " +
            "FROM \"products\" INNER JOIN \"categories\" AS \"categories\" ON \"products\".\"CategoryID\" = \"categories\".\"ID\"" );

        // just to be sure the composer *really* parses upon setting the query: feed it with
        // an unparseable statement
        boolean caughtExpected = false;
        try
        {
            composer.setQuery( "NONSENSE" );
        }
        catch( SQLException e )
        {
            caughtExpected = true;
        }
        assertTrue( "pre-condition not met: parser should except on unparseable statements, else the complete" +
            "test is bogus!", caughtExpected );
    }

    // --------------------------------------------------------------------------------------------------------
    private void impl_checkParameters( final String _statement, final String[] _expectedParameterNames, final int[] _expectedParameterTypes,final String _context ) throws Exception
    {
        final XSingleSelectQueryComposer composer = createQueryComposer();
        composer.setQuery( _statement );

        assertEquals( "checkParameterTypes: internal error", _expectedParameterNames.length, _expectedParameterTypes.length );

        final XParametersSupplier paramSupp = UnoRuntime.queryInterface(XParametersSupplier.class, composer);
        final XIndexAccess parameters = paramSupp.getParameters();

        assertEquals( "(ctx: " + _context + ") unexpected parameter count", _expectedParameterNames.length, parameters.getCount() );
        for ( int i=0; i<parameters.getCount(); ++i )
        {
            final XPropertySet parameter = UnoRuntime.queryInterface(XPropertySet.class, parameters.getByIndex(i));

            final String name = (String)parameter.getPropertyValue( "Name" );
            assertEquals( "(ctx: " + _context + ") unexpected parameter name for parameter number " + ( i + 1 ), _expectedParameterNames[i], name );

            final int type = ((Integer)parameter.getPropertyValue( "Type" )).intValue();
            assertEquals( "(ctx: " + _context + ") unexpected data type for parameter number " + ( i + 1 ), _expectedParameterTypes[i], type );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    /** verifies that the parser properly recognizes the types of parameters
     */
    @Test
    public void checkParameterTypes() throws Exception
    {
        impl_checkParameters(
            "SELECT * FROM \"all orders\" " +
            "WHERE ( \"Order Date\" >= :order_date ) " +
            "  AND (    ( \"Customer Name\" LIKE :customer ) " +
            "      OR   ( \"Product Name\" LIKE ? ) " +
            "      )",
            new String[] { "order_date", "customer", "Product Name" },
            new int[] { DataType.DATE, DataType.VARCHAR, DataType.VARCHAR },
            ">= && LIKE"
        );

        impl_checkParameters(
            "SELECT * FROM \"categories\" " +
            "WHERE \"ID\" BETWEEN :id_lo AND :id_hi",
            new String[] { "id_lo", "id_hi" },
            new int[] { DataType.INTEGER, DataType.INTEGER },
            "BETWEEN"
        );

        impl_checkParameters(
            "SELECT CONCAT( :prefix, CONCAT( \"Name\", :suffix ) ) FROM \"customers\"",
            new String[] { "prefix", "suffix" },
            new int[] { DataType.VARCHAR, DataType.VARCHAR },
            "CONCAT"
        );
    }
}
