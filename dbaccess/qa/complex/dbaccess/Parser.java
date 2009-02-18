/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Parser.java,v $
 * $Revision: 1.1.6.2 $
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
import com.sun.star.container.XNameAccess;
import com.sun.star.sdb.XParametersSupplier;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;

public class Parser extends CRMBasedTestCase
{
    // --------------------------------------------------------------------------------------------------------
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkJoinSyntax",
            "checkParameterTypes"
        };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return "Parser";
    }

    // --------------------------------------------------------------------------------------------------------
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
            assure( "caught an exception (" + e.getMessage() + ") while creating the test case", false );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    /** verifies that aliases for inner queries work as expected
     */
    public void checkJoinSyntax() throws Exception
    {
        XSingleSelectQueryComposer composer = createQueryComposer();

        // feed the composer with some statements. If any of those cannot be parsed, the composer
        // will throw an exception - which is a regression then
        composer.setQuery(
            "SELECT \"categories\".\"Name\", " +
                    "\"products\".\"Name\" " +
            "FROM \"products\" RIGHT OUTER JOIN \"categories\" AS \"categories\" ON \"products\".\"CategoryID\" = \"categories\".\"ID\"" );

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
        assure( "pre-condition not met: parser should except on unparseable statements, else the complete" +
            "test is bogus!", caughtExpected );
    }

    // --------------------------------------------------------------------------------------------------------
    private void impl_checkParameters( final String _statement, final String[] _expectedParameterNames, final int[] _expectedParameterTypes, String _context ) throws Exception
    {
        XSingleSelectQueryComposer composer = createQueryComposer();
        composer.setQuery( _statement );

        assureEquals( "checkParameterTypes: internal error", _expectedParameterNames.length, _expectedParameterTypes.length );

        XParametersSupplier paramSupp = (XParametersSupplier)UnoRuntime.queryInterface(
            XParametersSupplier.class, composer );
        XIndexAccess parameters = paramSupp.getParameters();

        assureEquals( "(ctx: " + _context + ") unexpected parameter count", _expectedParameterNames.length, parameters.getCount() );
        for ( int i=0; i<parameters.getCount(); ++i )
        {
            XPropertySet parameter = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class,
                parameters.getByIndex(i) );

            String name = (String)parameter.getPropertyValue( "Name" );
            assureEquals( "(ctx: " + _context + ") unexpected parameter name for parameter number " + ( i + 1 ), _expectedParameterNames[i], name );

            int type = ((Integer)parameter.getPropertyValue( "Type" )).intValue();
            assureEquals( "(ctx: " + _context + ") unexpected data type for parameter number " + ( i + 1 ), _expectedParameterTypes[i], type );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    /** verifies that the parser properly recognizes the types of parameters
     */
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
