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



import java.util.Vector;
import java.util.StringTokenizer;

import com.sun.star.beans.Property;
import com.sun.star.ucb.XContent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.sdbc.XRow;


/**
 * Obtaining Property Values from a UCB Content
 */
public class PropertiesRetriever {

    /**
     * Member properties
     */
    private  Helper   m_helper;
    private  XContent m_content;
    private  String   m_contenturl    = "";
    private  Vector   m_propNames     = new Vector();

    /**
     * Constructor.
     *
     *@param      String[]   This construtor requires the arguments:
     *                          -url=...       (optional)
     *                          -propNames=... (optional)
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     *@exception  java.lang.Exception
     */
    public PropertiesRetriever( String args[] ) throws java.lang.Exception {

        // Parse arguments
        parseArguments( args );

        // Init
        m_helper       = new Helper( getContentURL() );

        // Create UCB content
        m_content      = m_helper.createUCBContent();
    }

    /**
     * Get values of the properties.
     * This method requires the main and the optional arguments to be set in order to work.
     * See Constructor.
     *
     *@param  Vector   Properties
     *@return Vector   Returns Properties values if values successfully retrieved, null otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public Vector getPropertyValues()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {
        Vector properties = getProperties();
        return getPropertyValues ( properties );
    }

    /**
     *  Get values of the properties.
     *
     *@param  Vector   Properties
     *@return Vector   Returns Properties values if values successfully retrieved, null otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public Vector getPropertyValues( Vector properties )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {
        Vector m_propValues = null;
        if ( m_content != null && properties != null && !properties.isEmpty() ) {

            int size = properties.size();

            // Fill info for the properties wanted.
            Property[] props = new Property[ size ];
            for ( int index = 0 ; index < size; index++ ) {

                // Define property sequence.
                Property prop = new Property();
                prop.Name = ( String )properties.get( index );
                prop.Handle = -1; // n/a
                props[ index ] = prop;
            }

            // Execute command "getPropertyValues".
            XRow values =
                ( XRow )UnoRuntime.queryInterface(
                    XRow.class, m_helper.executeCommand( m_content,"getPropertyValues", props ));

            m_propValues = new Vector();

            /*
              Extract values from row object. Note that the
              first column is 1, not 0.
              Title: Obtain value of column 1 as string.*/
            for ( int index = 1 ; index <= size; index++ ) {
                Object propertyValue = values.getObject( index, null );
                if ( !values.wasNull() && !(propertyValue instanceof com.sun.star.uno.Any ))
                    m_propValues.add( propertyValue );
                else
                    m_propValues.add( "[ Property not found ]" );
            }
        }
        return m_propValues;
    }

    /**
     *  Get connect URL.
     *
     *@return   String  That contains the connect URL
     */
    public String getContentURL() {
        return m_contenturl;
    }

    /**
     * Get the properties.
     *
     *@return Vector  That contains the properties
     */
    public Vector getProperties() {
        return m_propNames;
    }

    /**
     * Parse arguments
     *
     *@param      String[]   Arguments
     *@exception  java.lang.Exception
     */
    public void parseArguments( String[] args ) throws java.lang.Exception {

        for ( int i = 0; i < args.length; i++ ) {
            if ( args[i].startsWith( "-url=" )) {
                m_contenturl    = args[i].substring( 5 );
            } else if ( args[i].startsWith( "-propNames=" )) {
                StringTokenizer tok
                    = new StringTokenizer( args[i].substring( 11 ), ";" );

                while ( tok.hasMoreTokens() )
                    m_propNames.add( tok.nextToken() );

            } else if ( args[i].startsWith( "-help" ) ||
                        args[i].startsWith( "-?" )) {
                printCmdLineUsage();
                System.exit( 0 );
            }
        }

        if ( m_contenturl == null || m_contenturl.equals( "" )) {
            m_contenturl = Helper.prependCurrentDirAsAbsoluteFileURL( "data/data.txt" );
        }

        if ( m_propNames.size() == 0 ) {
            m_propNames.add( "Title" );
            m_propNames.add( "IsDocument" );
        }
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage   : PropertiesRetriever -url=... -propNames=..." );
        System.out.println(
            "Defaults: -url=<currentdir>/data/data.txt -propNames=Title;IsDocument" );
        System.out.println(
            "\nExample : -propNames=Title;IsFolder" );
    }

    /**
     *  Create a new connection with the specific args to a running office and
     *  get the properties values from a resource.
     *
     *@param  String[]   Arguments
     */
    public static void main ( String args[] ) {
        System.out.println( "\n" );
        System.out.println(
            "--------------------------------------------------------------" );
        System.out.println(
            "PropertiesRetriever - obtains property values from a resource." );
        System.out.println(
            "--------------------------------------------------------------" );
        try {
            PropertiesRetriever obtProperty = new PropertiesRetriever( args );
            Vector properties  = obtProperty.getProperties();
            Vector propertiesValues = obtProperty.getPropertyValues( properties );

            String tempPrint = "\nProperties of resource " + obtProperty.getContentURL();
            int size = tempPrint.length();
            System.out.println( tempPrint );
            tempPrint = "";
            for( int i = 0; i < size; i++ ) {
                tempPrint += "-";
            }
            System.out.println( tempPrint );

            if ( properties != null && propertiesValues != null )  {
                size = properties.size();
                for (int index = 0; index < size ; index++ ) {
                    String property  = ( String )properties.get( index );
                    Object propValue = propertiesValues.get( index );
                    System.out.println( property + " : " + propValue );
                }
            }
        } catch ( com.sun.star.ucb.CommandAbortedException e ) {
            System.out.println( "Error: " + e );
        } catch ( com.sun.star.uno.Exception e ) {
            System.out.println( "Error: " + e );
        } catch ( java.lang.Exception e ) {
            System.out.println( "Error: " + e );
        }
        System.exit( 0 );
    }
}
