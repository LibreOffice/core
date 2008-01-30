/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Storable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:27:29 $
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
        catch ( Exception e )
        {
            assure( "caught an unexpected exception: " + e.getMessage(), false );
        }
    }
}
