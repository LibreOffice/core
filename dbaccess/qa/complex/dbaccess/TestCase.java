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

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
// import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import helper.FileTools;
import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;

// ---------- junit imports -----------------
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
// ------------------------------------------


public abstract class TestCase
{
    // --------------------------------------------------------------------------------------------------------
//    protected final XMultiServiceFactory getORB()
//    {
//        return (XMultiServiceFactory)param.getMSF();
//    }

    // --------------------------------------------------------------------------------------------------------
    protected final XComponentContext getComponentContext()
    {
        XComponentContext context = null;
        try
        {
            final XPropertySet orbProps = UnoRuntime.queryInterface( XPropertySet.class, getMSF() );
            context = UnoRuntime.queryInterface( XComponentContext.class,
                orbProps.getPropertyValue( "DefaultContext" ) );
        }
        catch ( Exception ex )
        {
            fail( "could not retrieve the ComponentContext" );
        }
        return context;
    }

    // --------------------------------------------------------------------------------------------------------
    public void before() throws java.lang.Exception
    {
    }

    // --------------------------------------------------------------------------------------------------------
    public void after() throws java.lang.Exception
    {
    }

    // --------------------------------------------------------------------------------------------------------
    /** returns the URL of a temporary file which can be used during the test.
     *
     *  The file will be deleted when the process exits
     *  @return the URL of a temporary file
     */
    protected final String createTempFileURL() throws IOException
    {
        final File documentFile = java.io.File.createTempFile( "dbaccess_test", ".odb" ).getAbsoluteFile();
        if ( documentFile.exists() )
        {
            documentFile.delete();
        }
        return FileHelper.getOOoCompatibleFileURL( documentFile.toURI().toURL().toString() );
    }

    // --------------------------------------------------------------------------------------------------------
    /**
     * copies the file given by URL to a temporary file
     * @return
     *  the URL of the new file
     */
    protected final String copyToTempFile( String _sourceURL ) throws IOException
    {
        final String targetURL = createTempFileURL();
        try
        {
            FileTools.copyFile( new File( new URI( _sourceURL ) ), new File( new URI( targetURL ) ) );
        }
        catch ( URISyntaxException e ) { }

        return FileHelper.getOOoCompatibleFileURL( targetURL );
    }

    // --------------------------------------------------------------------------------------------------------
    protected final XModel loadDocument( final String _docURL ) throws Exception
    {
        final XComponentLoader loader = UnoRuntime.queryInterface( XComponentLoader.class,
            getMSF().createInstance( "com.sun.star.frame.Desktop" ) );
        return UnoRuntime.queryInterface( XModel.class,
            loader.loadComponentFromURL( _docURL, "_blank", 0, new PropertyValue[] {} ) );
    }

    // --------------------------------------------------------------------------------------------------------
//    protected void assureException( Object _object, Class _unoInterfaceClass, String _methodName, Object[] _methodArgs,
//        Class _expectedExceptionClass )
//    {
//        assureException( UnoRuntime.queryInterface( _unoInterfaceClass, _object ), _methodName,
//            _methodArgs, _expectedExceptionClass );
//    }



    protected XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
