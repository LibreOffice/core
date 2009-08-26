/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestCase.java,v $
 * $Revision: 1.1.2.1 $
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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import helper.FileTools;
import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URI;
import java.net.URISyntaxException;

public abstract class TestCase extends complexlib.ComplexTestCase
{
    // --------------------------------------------------------------------------------------------------------
    protected final XMultiServiceFactory getORB()
    {
        return (XMultiServiceFactory)param.getMSF();
    }

    // --------------------------------------------------------------------------------------------------------
    protected final XComponentContext getComponentContext()
    {
        XComponentContext context = null;
        try
        {
            final XPropertySet orbProps = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, getORB() );
            context = (XComponentContext)UnoRuntime.queryInterface( XComponentContext.class,
                orbProps.getPropertyValue( "DefaultContext" ) );
        }
        catch ( Exception ex )
        {
            failed( "could not retrieve the ComponentContext" );
        }
        return context;
    }

    // --------------------------------------------------------------------------------------------------------
    public void before()
    {
    }

    // --------------------------------------------------------------------------------------------------------
    public void after()
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
        final File documentFile = java.io.File.createTempFile( getTestObjectName(), ".odb" );
        documentFile.deleteOnExit();
        return documentFile.getAbsoluteFile().toURI().toURL().toString();
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
    protected void assureException( Object _object, Class _unoInterfaceClass, String _methodName, Object[] _methodArgs,
        Class _expectedExceptionClass )
    {
        assureException( UnoRuntime.queryInterface( _unoInterfaceClass, _object ), _methodName,
            _methodArgs, _expectedExceptionClass );
    }
}
