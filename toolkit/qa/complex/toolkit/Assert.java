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

package complex.toolkit;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import com.sun.star.uno.UnoRuntime;
import static org.junit.Assert.*;

/**
 * provides assertion capabilities not found in {@link org.junit.Assert}
 */
public class Assert
{

    /** invokes a given method on a given object, and assures a certain exception is caught
     * @param i_message
     *          is the message to print when the check fails
     * @param i_object
     *          is the object to invoke the method on
     * @param i_methodName
     *          is the name of the method to invoke
     * @param i_methodArgs
     *          are the arguments to pass to the method.
     * @param i_argClasses
     *          are the classes to assume for the arguments of the methods
     * @param i_expectedExceptionClass
     *          is the class of the exception to be caught. If this is null,
     *          it means that <em>no</em> exception must be throw by invoking the method.
    */
    public static void assertException( final String i_message, final Object i_object, final String i_methodName,
        final Class<?>[] i_argClasses, final Object[] i_methodArgs, final Class<?> i_expectedExceptionClass )
    {
        Class<?> objectClass = i_object.getClass();

        boolean noExceptionAllowed = ( i_expectedExceptionClass == null );

        boolean caughtExpected = noExceptionAllowed ? true : false;
        try
        {
            Method method = impl_getMethod( objectClass, i_methodName, i_argClasses );
            method.invoke(i_object, i_methodArgs );
        }
        catch ( NoSuchMethodException e )
        {
            StringBuilder message = new StringBuilder();
            message.append( "no such method: " ).append( objectClass.getName() ).append( "." ).append( i_methodName ).append( "( " );
            for ( int i=0; i<i_argClasses.length; ++i )
            {
                message.append( i_argClasses[i].getName() );
                if ( i<i_argClasses.length - 1 )
                    message.append( ", " );
            }
            message.append( " )" );
            fail( message.toString() );
        }
        catch ( InvocationTargetException e )
        {
            caughtExpected =    noExceptionAllowed
                            ?   false
                            :   ( e.getTargetException().getClass().equals( i_expectedExceptionClass ) );
        }
        catch( Exception e )
        {
            caughtExpected = false;
        }

        assertTrue( i_message, caughtExpected );
    }

    /**
     * retrieves a method, given by name and parameter signature, from the given class
     *
     * The method does somewhat more than simply calling {@link Class#getMethod}. In particular, it recognizes
     * primitiive parameter types, and attempts to find a method taking the given primitive type, instead of the
     * type represented by the parameter class.
     *
     * For instance, if you have a method <code>foo( int )</code>, {@link Class#getMethod} would not return this
     * method when you pass <code>Integer.class</code>. <code>impl_getMethod</code> will recognize this, and
     * properly retrieve the method.
     *
     * Note: <code>impl_getMethod</code> is limited in that it will not try all possible combinations of primitive
     * and non-primitive types. That is, a method like <code>foo( int, Integer, int )</code> is likely to not be
     * found.
     *
     * @param i_objectClass
     * @param i_methodName
     * @param i_argClasses
     * @return
     */
    private static Method impl_getMethod( final Class<?> i_objectClass, final String i_methodName, final Class<?>[] i_argClasses ) throws NoSuchMethodException
    {
        try
        {
            return i_objectClass.getMethod( i_methodName, i_argClasses );
        }
        catch ( NoSuchMethodException ex )
        {
        }

        int substitutedTypes = 0;
        int substitutedTypesLastRound = 0;
        final Class<?>[][] substitutionTable = new Class[][] {
            new Class[] { Long.class, long.class },
            new Class[] { Integer.class, int.class },
            new Class[] { Short.class, short.class },
            new Class[] { Byte.class, byte.class },
            new Class[] { Double.class, double.class },
            new Class[] { Float.class, float.class },
            new Class[] { Character.class, char.class }
        };
        do
        {
            substitutedTypes = 0;
            final Class<?>[] argClasses = new Class[ i_argClasses.length ];
            for ( int i=0; i < argClasses.length; ++i )
            {
                argClasses[i] = i_argClasses[i];
                if ( substitutedTypes > substitutedTypesLastRound )
                    continue;

                for ( int c=0; c<substitutionTable.length; ++c )
                {
                    if ( i_argClasses[i].equals( substitutionTable[c][0] ) )
                    {
                        argClasses[i] = substitutionTable[c][1];
                        ++substitutedTypes;
                        break;
                    }
                }
            }
            if ( substitutedTypes == substitutedTypesLastRound )
                throw new NoSuchMethodException();
            substitutedTypesLastRound = substitutedTypes;

            try
            {
                return i_objectClass.getMethod( i_methodName, argClasses );
            }
            catch ( NoSuchMethodException e )
            {
            }
        }
        while ( substitutedTypes > 0 );
        throw new NoSuchMethodException();
    }

    /** invokes a given method on a given object, and assures a certain exception is caught
     * @param i_message is the message to print when the check fails
     * @param i_object is the object to invoke the method on
     * @param i_methodName is the name of the method to invoke
     * @param i_methodArgs are the arguments to pass to the method. Those implicitly define
     *      the classes of the arguments of the method which is called.
     * @param i_expectedExceptionClass is the class of the exception to be caught. If this is null,
     *          it means that <em>no</em> exception must be throw by invoking the method.
    */
    public static void assertException( final String i_message, final Object i_object, final String i_methodName,
        final Object[] i_methodArgs, final Class<?> i_expectedExceptionClass )
    {
        Class<?>[] argClasses = new Class[ i_methodArgs.length ];
        for ( int i=0; i<i_methodArgs.length; ++i )
            argClasses[i] = i_methodArgs[i].getClass();
        assertException( i_message, i_object, i_methodName, argClasses, i_methodArgs, i_expectedExceptionClass );
    }

    /** invokes a given method on a given object, and assures a certain exception is caught
     * @param i_object is the object to invoke the method on
     * @param i_methodName is the name of the method to invoke
     * @param i_methodArgs are the arguments to pass to the method. Those implicitly define
     *      the classes of the arguments of the method which is called.
     * @param i_expectedExceptionClass is the class of the exception to be caught. If this is null,
     *          it means that <em>no</em> exception must be throw by invoking the method.
    */
    public static void assertException( final Object i_object, final String i_methodName, final Object[] i_methodArgs,
        final Class<?> i_expectedExceptionClass )
    {
        assertException(
            "did not catch the expected exception (" +
                ( ( i_expectedExceptionClass == null ) ? "none" : i_expectedExceptionClass.getName() ) +
                ") while calling " + i_object.getClass().getName() + "." + i_methodName,
            i_object, i_methodName, i_methodArgs, i_expectedExceptionClass );
    }

    /** invokes a given method on a given object, and assures a certain exception is caught
     * @param i_object is the object to invoke the method on
     * @param i_methodName is the name of the method to invoke
     * @param i_methodArgs are the arguments to pass to the method
     * @param i_argClasses are the classes to assume for the arguments of the methods
     * @param i_expectedExceptionClass is the class of the exception to be caught. If this is null,
     *          it means that <em>no</em> exception must be throw by invoking the method.
    */
    public static void assertException( final Object i_object, final String i_methodName, final Class<?>[] i_argClasses,
        final Object[] i_methodArgs, final Class<?> i_expectedExceptionClass )
    {
        assertException(
            "did not catch the expected exception (" +
                ( ( i_expectedExceptionClass == null ) ? "none" : i_expectedExceptionClass.getName() ) +
                ") while calling " + i_object.getClass().getName() + "." + i_methodName,
            i_object, i_methodName, i_argClasses, i_methodArgs, i_expectedExceptionClass );
    }


    public static void assertException( Object i_object, Class<?> _unoInterfaceClass, String i_methodName, Object[] i_methodArgs,
        Class<?> i_expectedExceptionClass )
    {
        assertException( UnoRuntime.queryInterface( _unoInterfaceClass, i_object ), i_methodName,
            i_methodArgs, i_expectedExceptionClass );
    }
}
