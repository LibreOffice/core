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

import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.Exception;

public class Parser extends CRMBasedTestCase
{
    // --------------------------------------------------------------------------------------------------------
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkJoinSyntax"
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

}
