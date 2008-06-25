/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Storable.java,v $
 * $Revision: 1.5 $
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
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XStorable;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.util.XCloseable;
import connectivity.tools.*;

public class Storable extends complexlib.ComplexTestCase {

    HsqlDatabase    m_database;

    // --------------------------------------------------------------------------------------------------------
    public String[] getTestMethodNames() {
        return new String[]
        {
            "testStorable"
        };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName() {
        return "Storable";
    }

    // --------------------------------------------------------------------------------------------------------
    private void createTestCase()
    {
        try
        {
            if ( m_database == null )
            {
                CRMDatabase database = new CRMDatabase( getFactory() );
                m_database = database.getDatabase();
            }
        }
        catch( Exception e )
        {
            System.err.println( "could not create the test case, error message:\n" + e.getMessage() );
            e.printStackTrace( System.err );
            assure( "failed to created the test case", false );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private XMultiServiceFactory getFactory()
    {
        return (XMultiServiceFactory)param.getMSF();
    }

    // --------------------------------------------------------------------------------------------------------
    public void testStorable()
    {
        createTestCase();

        try
        {
            Object object = getFactory().createInstance("com.sun.star.frame.Desktop");
            XComponentLoader xComponentLoader = (XComponentLoader)UnoRuntime.queryInterface(XComponentLoader.class, object);
            XComponent xComponent = xComponentLoader.loadComponentFromURL(m_database.getDocumentURL(), "_blank",FrameSearchFlag.ALL, new PropertyValue[0]);
            m_database.close();
            XStorable storable = (XStorable)UnoRuntime.queryInterface(XStorable.class,xComponent);
            storable.store();
            XCloseable close = (XCloseable)UnoRuntime.queryInterface(XCloseable.class,xComponent);
            close.close(true);
        }
        catch ( AssureException e ) { throw e; }
        catch ( Exception e )
        {
            assure( "caught an unexpected exception: " + e.getMessage(), false );
        }
    }
}
