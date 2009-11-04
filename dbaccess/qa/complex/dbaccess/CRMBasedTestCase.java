/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CRMBasedTestCase.java,v $
 * $Revision: 1.1.6.6 $
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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.uno.UnoRuntime;
import java.util.logging.Level;
import java.util.logging.Logger;

public abstract class CRMBasedTestCase extends TestCase
{
    protected   CRMDatabase m_database;

    // --------------------------------------------------------------------------------------------------------
    protected void createTestCase()
    {
        try
        {
            m_database = new CRMDatabase( getORB() );
        }
        catch ( Exception e )
        {
            e.printStackTrace( System.err );
            assure( "caught an exception (" + e.getMessage() + ") while creating the test case", false );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    @Override
    public void before()
    {
        createTestCase();
    }

    // --------------------------------------------------------------------------------------------------------
    @Override
    public void after()
    {
        try
        {
            if ( m_database != null )
                m_database.saveAndClose();
        }
        catch ( Exception ex )
        {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, null, ex );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    /** creates a SingleSelectQueryComposer for our connection
     */
    protected final XSingleSelectQueryComposer createQueryComposer() throws com.sun.star.uno.Exception
    {
        final XMultiServiceFactory connectionFactory = (XMultiServiceFactory)UnoRuntime.queryInterface(
            XMultiServiceFactory.class, m_database.getConnection() );
        return (XSingleSelectQueryComposer)UnoRuntime.queryInterface(
            XSingleSelectQueryComposer.class, connectionFactory.createInstance( "com.sun.star.sdb.SingleSelectQueryComposer" ) );
    }
}
