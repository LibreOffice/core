/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package complex.dbaccess;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import com.sun.star.beans.PropertyValue;
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
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;



public abstract class TestCase
{

    protected final XComponentContext getComponentContext()
    {
        return connection.getComponentContext();
    }


    public void before() throws java.lang.Exception
    {
    }


    public void after() throws java.lang.Exception
    {
    }


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


    protected final XModel loadDocument( final String _docURL ) throws Exception
    {
        final XComponentLoader loader = UnoRuntime.queryInterface( XComponentLoader.class,
            getMSF().createInstance( "com.sun.star.frame.Desktop" ) );
        return UnoRuntime.queryInterface( XModel.class,
            loader.loadComponentFromURL( _docURL, "_blank", 0, new PropertyValue[] {} ) );
    }


    /** invokes a given method on a given object, and assures a certain exception is caught
     * @param _message
     *          is the message to print when the check fails
     * @param _object
     *          is the object to invoke the method on
     * @param _methodName
     *          is the name of the method to invoke
     * @param _methodArgs
     *          are the arguments to pass to the method.
     * @param _argClasses
     *          are the classes to assume for the arguments of the methods
     * @param _expectedExceptionClass
     *          is the class of the exception to be caught. If this is null,
     *          it means that <em>no</em> exception must be throw by invoking the method.
    */
    protected void assureException( final String _message, final Object _object, final String _methodName,
        final Class[] _argClasses, final Object[] _methodArgs, final Class _expectedExceptionClass )
    {
        Class objectClass = _object.getClass();

        boolean noExceptionAllowed = ( _expectedExceptionClass == null );

        boolean caughtExpected = noExceptionAllowed ? true : false;
        try
        {
            Method method = objectClass.getMethod( _methodName, _argClasses );
            method.invoke(_object, _methodArgs );
        }
        catch ( InvocationTargetException e )
        {
            caughtExpected =    noExceptionAllowed
                            ?   false
                            :   ( e.getTargetException().getClass().equals( _expectedExceptionClass ) );
        }
        catch( Exception e )
        {
            caughtExpected = false;
        }

        assertTrue( _message, caughtExpected );
    }

    /** invokes a given method on a given object, and assures a certain exception is caught
     * @param _message is the message to print when the check fails
     * @param _object is the object to invoke the method on
     * @param _methodName is the name of the method to invoke
     * @param _methodArgs are the arguments to pass to the method. Those implicitly define
     *      the classes of the arguments of the method which is called.
     * @param _expectedExceptionClass is the class of the exception to be caught. If this is null,
     *          it means that <em>no</em> exception must be throw by invoking the method.
    */
    protected void assureException( final String _message, final Object _object, final String _methodName,
        final Object[] _methodArgs, final Class _expectedExceptionClass )
    {
        Class[] argClasses = new Class[ _methodArgs.length ];
        for ( int i=0; i<_methodArgs.length; ++i )
            argClasses[i] = _methodArgs[i].getClass();
        assureException( _message, _object, _methodName, argClasses, _methodArgs, _expectedExceptionClass );
    }

    /** invokes a given method on a given object, and assures a certain exception is caught
     * @param _object is the object to invoke the method on
     * @param _methodName is the name of the method to invoke
     * @param _methodArgs are the arguments to pass to the method. Those implicitly define
     *      the classes of the arguments of the method which is called.
     * @param _expectedExceptionClass is the class of the exception to be caught. If this is null,
     *          it means that <em>no</em> exception must be throw by invoking the method.
    */
    protected void assureException( final Object _object, final String _methodName, final Object[] _methodArgs,
        final Class _expectedExceptionClass )
    {
        assureException(
            "did not catch the expected exception (" +
                ( ( _expectedExceptionClass == null ) ? "none" : _expectedExceptionClass.getName() ) +
                ") while calling " + _object.getClass().getName() + "." + _methodName,
            _object, _methodName, _methodArgs, _expectedExceptionClass );
    }

    /** invokes a given method on a given object, and assures a certain exception is caught
     * @param _object is the object to invoke the method on
     * @param _methodName is the name of the method to invoke
     * @param _methodArgs are the arguments to pass to the method
     * @param _argClasses are the classes to assume for the arguments of the methods
     * @param _expectedExceptionClass is the class of the exception to be caught. If this is null,
     *          it means that <em>no</em> exception must be throw by invoking the method.
    */
    protected void assureException( final Object _object, final String _methodName, final Class[] _argClasses,
        final Object[] _methodArgs, final Class _expectedExceptionClass )
    {
        assureException(
            "did not catch the expected exception (" +
                ( ( _expectedExceptionClass == null ) ? "none" : _expectedExceptionClass.getName() ) +
                ") while calling " + _object.getClass().getName() + "." + _methodName,
            _object, _methodName, _argClasses, _methodArgs, _expectedExceptionClass );
    }


    @SuppressWarnings("unchecked")
    protected void assureException( Object _object, Class _unoInterfaceClass, String _methodName, Object[] _methodArgs,
        Class _expectedExceptionClass )
    {
        assureException( UnoRuntime.queryInterface( _unoInterfaceClass, _object ), _methodName,
            _methodArgs, _expectedExceptionClass );
    }


    protected XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }


    // setup and close connections
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        connection.setUp();
    }


    @AfterClass
    public static void tearDownConnection() throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
