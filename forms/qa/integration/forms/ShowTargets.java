/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ShowTargets.java,v $
 * $Revision: 1.3 $
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

package integration.forms;

/**
 *
 * @author fs93730
 */
public class ShowTargets
{
    /** Creates a new instance of ShowTargets */
    public ShowTargets()
    {
    }

    public static void main( String[] args )
    {
        System.out.println( "possible targets (* denotes a test case which requires user interaction):" );
        System.out.println( "  run" );

        for ( int i = 0; i < args.length; ++i )
        {
            // filter
            if ( args[i].equals( "TestCase" ) )
                continue;
            if ( args[i].equals( "TestSkeleton" ) )
                continue;

            String completePotentialClassName = "integration.forms." + args[i];

            // get the class
            Class potentialTestClass = null;
            try { potentialTestClass = Class.forName( completePotentialClassName ); }
            catch( java.lang.ClassNotFoundException e )
            {
                continue;
            }

            // see if it is derived from complexlib.ComplexTestCase
            Class superClass = potentialTestClass.getSuperclass();
            while ( superClass != null )
            {
                if ( superClass.getName().equals( "complexlib.ComplexTestCase" ) )
                {
                    if ( isInteractiveTest( potentialTestClass ) )
                        System.out.print( "* " );
                    else
                        System.out.print( "  " );
                    System.out.println( "run_" + args[i] );
                }
                superClass = superClass.getSuperclass();
            }
        }
    }

    /** determines if the test denoted by a given Class is an interactive test
     */
    static private boolean isInteractiveTest( Class testClass )
    {
        java.lang.reflect.Method interactiveTestMethod = null;
        try { interactiveTestMethod = testClass.getMethod( "isInteractiveTest", new Class[]{} ); }
        catch( Exception e ) { }

        if ( interactiveTestMethod != null )
        {
            try
            {
                Boolean result = (Boolean)interactiveTestMethod.invoke( null, new Object[]{} );
                return result.booleanValue();
            }
            catch( Exception e ) { }
        }
        return false;
    }
}
