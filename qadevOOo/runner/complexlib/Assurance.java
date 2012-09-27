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

package complexlib;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * I have removed the assure(...) functions from ComplexTestCase due to the fact now I can
 * use the functions every where and don't need to be a ComplexTestCase any longer.
 */
public class Assurance
{
    public static final boolean CONTINUE = true;

    /** State of the current test method **/
    protected boolean state = true;

    /** The message if the test did fail **/
    protected String message = null;


    /**
     * Assure that s is true.
     * This function generates "Assure failed." as standard message.
     * @param s The condition that should be true.
     */
    protected void assure(boolean s) {
        assure("Assure failed.", s, false);
    }

    /**
     * Assure that s is true.
     * The given message will be only evaluated, if s is false.
     * @param msg The message that is evaluated.
     * @param s The condition that should be true.
     */
    protected void assure(String msg, boolean s) {
        assure(msg, s, false);
    }

    /**
     * Assure that two boolean values are equal
     * @param expected specifies the expected boolean value
     * @param actual specifies the actual boolean value
     */
    protected void assureEquals( boolean expected, boolean actual ) {
        assureEquals( "Equality test failed", expected, new Boolean( actual ), false );
    }

    /**
     * Assure that two boolean values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected boolean value
     * @param actual specifies the actual boolean value
     */
    protected void assureEquals( String message, boolean expected, boolean actual ) {
        assureEquals( message, expected, actual, false );
    }

    /**
     * Assure that two byte values are equal
     * @param expected specifies the expected byte value
     * @param actual specifies the actual byte value
     */
    protected void assureEquals( byte expected, byte actual ) {
        assureEquals( "Equality test failed", new Byte( expected ), new Byte( actual ), false );
    }

    /**
     * Assure that two byte values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected byte value
     * @param actual specifies the actual byte value
     */
    protected void assureEquals( String message, byte expected, byte actual ) {
        assureEquals( message, new Byte( expected ), new Byte( actual ), false );
    }

    /**
     * Assure that two double values are equal
     * @param expected specifies the expected double value
     * @param actual specifies the actual double value
     */
    protected void assureEquals( double expected, double actual ) {
        assureEquals( "Equality test failed", new Double( expected ), new Double( actual ), false );
    }

    /**
     * Assure that two double values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected double value
     * @param actual specifies the actual double value
     */
    protected void assureEquals( String message, double expected, double actual ) {
        assureEquals( message, new Double( expected ), new Double( actual ), false );
    }

    /**
     * Assure that two float values are equal
     * @param expected specifies the expected float value
     * @param actual specifies the actual float value
     */
    protected void assureEquals( float expected, float actual ) {
        assureEquals( "Equality test failed", new Float( expected ), new Float( actual ), false );
    }

    /**
     * Assure that two float values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected float value
     * @param actual specifies the actual float value
     */
    protected void assureEquals( String message, float expected, float actual ) {
        assureEquals( message, new Float( expected ), new Float( actual ), false );
    }

    /**
     * Assure that two short values are equal
     * @param expected specifies the expected short value
     * @param actual specifies the actual short value
     */
    protected void assureEquals( short expected, short actual ) {
        assureEquals( "Equality test failed", new Short( expected ), new Short( actual ), false );
    }

    /**
     * Assure that two short values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected short value
     * @param actual specifies the actual short value
     */
    protected void assureEquals( String message, short expected, short actual ) {
        assureEquals( message, new Short( expected ), new Short( actual ), false );
    }

    /**
     * Assure that two int values are equal
     * @param expected specifies the expected int value
     * @param actual specifies the actual int value
     */
    protected void assureEquals( int expected, int actual ) {
        assureEquals( "Equality test failed", new Integer( expected ), new Integer( actual ), false );
    }

    /**
     * Assure that two int values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected int value
     * @param actual specifies the actual int value
     */
    protected void assureEquals( String message, int expected, int actual ) {
        assureEquals( message, new Integer( expected ), new Integer( actual ), false );
    }

    /**
     * Assure that two long values are equal
     * @param expected specifies the expected long value
     * @param actual specifies the actual long value
     */
    protected void assureEquals( long expected, long actual ) {
        assureEquals( "Equality test failed", new Long( expected ), new Long( actual ), false );
    }

    /**
     * Assure that two long values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected long value
     * @param actual specifies the actual long value
     */
    protected void assureEquals( String message, long expected, long actual ) {
        assureEquals( message, new Long( expected ), new Long( actual ), false );
    }

    /**
     * Assure that two string values are equal
     * @param expected specifies the expected string value
     * @param actual specifies the actual string value
     */
    protected void assureEquals( String expected, String actual ) {
        assureEquals( "Equality test failed", expected, actual, false );
    }

    /**
     * Assure that two string values are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected string value
     * @param actual specifies the actual string value
     */
    protected void assureEquals( String message, String expected, String actual ) {
        assureEquals( message, expected, actual, false );
    }

    /**
     * Assure that two object are equal
     * @param expected specifies the expected object value
     * @param actual specifies the actual object value
     */
    protected void assureEquals( Object expected, Object actual ) {
        assureEquals( "Equality test failed", expected, actual, false );
    }

    /**
     * Assure that two objects are equal
     * @param message the message to print when the equality test fails
     * @param expected specifies the expected object value
     * @param actual specifies the actual object value
     */
    protected void assureEquals( String message, Object expected, Object actual ) {
        assureEquals( message, expected, actual, false );
    }

    /**
     * assures the two given sequences are of equal length, and have equal content
     */
    public <T> void assureEquals( String i_message, T[] i_expected, T[] i_actual, boolean i_continue )
    {
        if ( i_expected.length != i_actual.length )
            failed( i_message + ": expected element count: " + i_expected.length + ", actual element count: " + i_actual.length );
        for ( int i=0; i<i_expected.length; ++i )
        {
            assureEquals( i_message + ": mismatch at element pos " + i, i_expected[i], i_actual[i], i_continue );
        }
    }

    /**
     * assures the two given sequences are of equal length, and have equal content
     */
    public <T> void assureEquals( String i_message, T[] i_expected, T[] i_actual )
    {
        assureEquals( i_message, i_expected, i_actual, false );
    }

    /** invokes a given method on a given object, and assures a certain exception is caught
     * @param _message is the message to print when the check fails
     * @param _object is the object to invoke the method on
     * @param _methodName is the name of the method to invoke
     * @param _methodArgs are the arguments to pass to the method.
     * @param _argClasses are the classes to assume for the arguments of the methods
     * @param _expectedExceptionClass is the class of the exception to be caught. If this is null,
     *          it means that <em>no</em> exception must be throw by invoking the method.
    */
    protected void assureException( final String _message, final Object _object, final String _methodName,
        final Class<?>[] _argClasses, final Object[] _methodArgs, final Class<?> _expectedExceptionClass )
    {
        Class<?> objectClass = _object.getClass();

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

        assure( _message, caughtExpected );
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
        final Object[] _methodArgs, final Class<?> _expectedExceptionClass )
    {
        Class<?>[] argClasses = new Class[ _methodArgs.length ];
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
        final Class<?> _expectedExceptionClass )
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
    protected void assureException( final Object _object, final String _methodName, final Class<?>[] _argClasses,
        final Object[] _methodArgs, final Class<?> _expectedExceptionClass )
    {
        assureException(
            "did not catch the expected exception (" +
                ( ( _expectedExceptionClass == null ) ? "none" : _expectedExceptionClass.getName() ) +
                ") while calling " + _object.getClass().getName() + "." + _methodName,
            _object, _methodName, _argClasses, _methodArgs, _expectedExceptionClass );
    }

    /**
     * Mark the currently executed method as failed.
     * This function generates "Test did fail." as standard message.
     */
    protected void failed() {
        assure("Test did fail.", false, false);
    }

    /**
     * Mark the currently executed method as failed.
     * with the given message.
     * @param msg The message of the failure.
     */
    protected void failed(String msg) {
        assure(msg, false, false);
    }

    /**
     * Assure that s is true.
     * The given message will be only evaluated, if s is false.
     * Normally, assure() leaves the current test method, and the next one
     * is executed. With the parameter 'cont' set to true, the current test
     * method will continue.<br>
     * The current method will of course marked as failed.
     * @param msg The message that is evaluated.
     * @param s The condition that should be true.
     * @param cont Continue with test method, even if s is false.
     */
    protected void assure(String msg, boolean s, boolean cont) {
        state &= s;
        if (!s) {
            message += msg + "\r\n";
            // log.println(msg);
            if (!cont) {
                throw new AssureException(msg);
            }
        }
    }

    protected void assureEquals( String message, Object expected, Object actual, boolean cont ) {
        assure( message + " (expected: " + expected.toString() + ", actual: " + actual.toString() + ")",
            expected.equals( actual ), cont );
    }

    /**
     * Mark the currently executed method as failed.
     * with the given message.
     * The given message will be only evaluated, if s is false.
     * With the parameter 'cont' set to true, the current test
     * method will continue.<br>
     * The current method will of course marked as failed.
     * @param msg The message that is evaluated.
     * @param cont Continue with test method, even if s is false.
     */
    protected void failed(String msg, boolean cont) {
        assure(msg, false, cont);
    }

    public class AssureException extends RuntimeException {

        public AssureException(String msg) {
            super(msg);
        }
    }
}
