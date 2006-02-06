/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ShowTargets.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-02-06 16:48:29 $
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
