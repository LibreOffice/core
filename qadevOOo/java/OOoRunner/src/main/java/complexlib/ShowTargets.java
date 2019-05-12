/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
