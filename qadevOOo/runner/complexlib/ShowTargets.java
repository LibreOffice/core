/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ShowTargets.java,v $
 * $Revision: 1.4 $
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

package complexlib;

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
        java.util.Vector targets = new java.util.Vector();
        java.util.Vector descs = new java.util.Vector();

        targets.add( "run" );
        descs.add( "runs all complex tests in this module" );

        int maxTargetLength = 3;

        for ( int i = 0; i < args.length; ++i )
        {
            String completePotentialClassName = args[i].replace( '/', '.' );

            // filter
            if ( completePotentialClassName.endsWith( ".TestCase" ) )
                continue;
            if ( completePotentialClassName.endsWith( ".TestSkeleton" ) )
                continue;

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
                    String bareClassName = completePotentialClassName.substring( completePotentialClassName.lastIndexOf( '.' ) + 1 );
                    String target = "run_" + bareClassName;
                    targets.add( target );
                    descs.add( getShortTestDescription( potentialTestClass ) );

                    if ( maxTargetLength < target.length() )
                        maxTargetLength = target.length();
                    break;
                }
                superClass = superClass.getSuperclass();
            }
        }

        System.out.println( "possible targets:" );
        for ( int i=0; i<targets.size(); ++i )
        {
            // target
            String target = (String)targets.get(i);
            // 'tab'
            System.out.print( "  " + target );
            for ( int s = maxTargetLength - target.length(); s>0; --s )
                System.out.print( " " );
            // description
            System.out.println( " (" + (String)descs.get(i) + ")" );
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

    static private String getShortTestDescription( Class _testClass )
    {
        java.lang.reflect.Method getShortDescriptionMethod = null;
        try { getShortDescriptionMethod = _testClass.getMethod( "getShortTestDescription", new Class[]{} ); }
        catch( Exception e ) { }

        if ( getShortDescriptionMethod != null )
        {
            try
            {
                return (String)getShortDescriptionMethod.invoke( null, new Object[]{} );
            }
            catch( Exception e ) { }
        }
        return "no description provided by the test";
    }
}
