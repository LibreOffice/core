/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import java.util.ArrayList;
import java.util.StringTokenizer;

import com.sun.star.beans.Property;
import com.sun.star.sdbc.XRow;
import com.sun.star.ucb.XContent;
import com.sun.star.uno.UnoRuntime;


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
    private  ArrayList<String>   m_propNames     = new ArrayList<String>();

    /**
     * Constructor.
     *
     *@param      args   This constructor requires the arguments:
     *                          -url=...       (optional)
     *                          -propNames=... (optional)
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
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
     *@return Properties values if values successfully retrieved, null otherwise
     */
    public ArrayList<Object> getPropertyValues()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {
        ArrayList<String> properties = getProperties();
        return getPropertyValues ( properties );
    }

    /**
     *  Get values of the properties.
     *
     *@return Properties values if values successfully retrieved, null otherwise
     */
    public ArrayList<Object> getPropertyValues( ArrayList<String> properties )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {
        ArrayList<Object> m_propValues = null;
        if ( m_content != null && properties != null && !properties.isEmpty() ) {

            int size = properties.size();

            // Fill info for the properties wanted.
            Property[] props = new Property[ size ];
            for ( int index = 0 ; index < size; index++ ) {

                // Define property sequence.
                Property prop = new Property();
                prop.Name = properties.get( index );
                prop.Handle = -1; // n/a
                props[ index ] = prop;
            }

            // Execute command "getPropertyValues".
            XRow values =
                UnoRuntime.queryInterface(
                XRow.class, m_helper.executeCommand( m_content,"getPropertyValues", props ));

            m_propValues = new ArrayList<Object>();

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
    public ArrayList<String> getProperties() {
        return m_propNames;
    }

    /**
     * Parse arguments
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
            ArrayList<String> properties  = obtProperty.getProperties();
            ArrayList<Object> propertiesValues = obtProperty.getPropertyValues( properties );

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
                    String property  = properties.get( index );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
