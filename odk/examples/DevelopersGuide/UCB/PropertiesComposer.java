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

import com.sun.star.beans.PropertyValue;
import com.sun.star.ucb.XContent;

/**
 * Setting Property Values of a UCB Content
 */
public class PropertiesComposer {

    /**
     * Member properties
     */
    private  Helper    m_helper;
    private  XContent  m_content;
    private  String    m_contenturl = "";
    private  ArrayList<String>    m_propNames          = new ArrayList<String>();
    private  ArrayList<String>    m_propValues         = new ArrayList<String>();

    /**
     * Constructor.
     *
     *@param      args   This constructor requires the arguments:
     *                          -url=...        (optional)
     *                          -propNames=...  (optional)
     *                          -propValues=... (optional)
     *                          -workdir=...    (optional)
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     */
    public PropertiesComposer( String args[] ) throws java.lang.Exception {

        // Parse arguments
        parseArguments( args );

        // Init
        m_helper       = new Helper( getContentURL() );

        // Create UCB content
        m_content      = m_helper.createUCBContent();
    }

    /**
     *  Set values of the properties.
     * This method requires the main and the optional arguments to be set in order to work.
     * See Constructor.
     *
     *@return Object[]  Returns null or instance object of com.sun.star.uno.Any
     *                  if values successfully seted, properties otherwise
     */
    public Object[] setProperties()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {
        ArrayList<String> properties      = getProperties();
        ArrayList<String> propertyValues  = getPropertyValues();
        return setProperties( properties, propertyValues );
    }

    /**
     *  Set values of the properties.
     *
     *@return Object[]  Returns null or instance object of com.sun.star.uno.Any
     *                  if values successfully seted, properties otherwise
     */
    public Object[] setProperties( ArrayList<String> properties, ArrayList<String> propertiesValues )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        Object[] result = null;
        if ( m_content != null && !properties.isEmpty() &&
             !propertiesValues.isEmpty() &&
             properties.size() == propertiesValues.size() ) {

            /*
            ****     This code is for unregistered properties.     ****

            XPropertyContainer xPropContainer
                    = (XPropertyContainer)UnoRuntime.queryInterface(
                        XPropertyContainer.class, m_content );

            XPropertySetInfo xPropSetInfo = ( XPropertySetInfo )UnoRuntime.queryInterface(
                    XPropertySetInfo.class,
                    m_helper.executeCommand( m_content, "getPropertySetInfo", null ));
            */

            int size = properties.size();
            PropertyValue[] props = new PropertyValue[ size ];
            for ( int index = 0 ; index < size; index++ ) {
                String propName  = properties.get( index );
                Object propValue = propertiesValues.get( index );

                /*
                ****     This code is for unregistered properties.     ****

                if ( !xPropSetInfo.hasPropertyByName( propName )) {
                    xPropContainer.addProperty(
                        propName, PropertyAttribute.MAYBEVOID, propValue );
                }
                */

                // Define property sequence.
                PropertyValue prop = new PropertyValue();
                prop.Name = propName;
                prop.Handle = -1; // n/a
                prop.Value  = propValue;
                props[ index ] = prop;
            }

            // Execute command "setPropertiesValues".
            Object[] obj =
                ( Object[] )m_helper.executeCommand( m_content, "setPropertyValues", props );
            if ( obj.length == size )
                 result = obj;
        }
        return result;
    }

    /**
     *  Get properties names.
     *
     *@return   Vector    That contains the properties names
     */
    public ArrayList<String> getProperties() {
        return m_propNames;
    }

    /**
     *  Get properties values.
     *
     *@return   Vector    That contains the properties values
     */
    public ArrayList<String> getPropertyValues() {
        return m_propValues;
    }

    /**
     *  Get connect URL.
     *
     *@return   String    That contains the connect URL
     */
    public String getContentURL() {
        return m_contenturl;
    }

    /**
     * Parse arguments
     */
    public void parseArguments( String[] args ) throws java.lang.Exception {

        String workdir = "";

        for ( int i = 0; i < args.length; i++ ) {
            if ( args[i].startsWith( "-url=" )) {
                m_contenturl    = args[i].substring( 5 );
            } else if ( args[i].startsWith( "-propNames=" )) {
                StringTokenizer tok
                    = new StringTokenizer( args[i].substring( 11 ), ";" );

                while ( tok.hasMoreTokens() )
                    m_propNames.add( tok.nextToken() );

            } else if ( args[i].startsWith( "-propValues=" )) {
                StringTokenizer tok
                    = new StringTokenizer( args[i].substring( 12 ), ";" );

                while ( tok.hasMoreTokens() )
                    m_propValues.add( tok.nextToken() );
            } else if ( args[i].startsWith( "-workdir=" )) {
                workdir = args[i].substring( 9 );
            } else if ( args[i].startsWith( "-help" ) ||
                        args[i].startsWith( "-?" )) {
                printCmdLineUsage();
                System.exit( 0 );
            }
        }

        if ( m_contenturl == null || m_contenturl.equals( "" )) {
            m_contenturl = Helper.createTargetDataFile( workdir );
        }

        if ( m_propNames.size() == 0 ) {
            m_propNames.add( "Title" );
        }

        if ( m_propValues.size() == 0 ) {
            m_propValues.add(
                "changed-" + m_contenturl.substring(
                    m_contenturl.lastIndexOf( "/" ) + 1 ) );
        }
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage   : PropertiesComposer -url=... -propNames=... -propValues=... -workdir=..." );
        System.out.println(
            "Defaults: -url=<workdir>/resource-<uniquepostfix> -propNames=Title -propValues=changed-<uniquepostfix> -workdir=<currentdir>" );
        System.out.println(
            "\nExample : -propNames=Title;Foo -propValues=MyRenamedFile.txt;bar" );
    }

    /**
     *  Create a new connection with the specific args to a running office and
     *  set properties of a resource.
     */
    public static void main ( String args[] ) {
        System.out.println( "\n" );
        System.out.println(
            "--------------------------------------------------------" );
        System.out.println(
            "PropertiesComposer - sets property values of a resource." );
        System.out.println(
            "--------------------------------------------------------" );

        try {

            PropertiesComposer setProp = new PropertiesComposer( args );
            ArrayList<String> properties       = setProp.getProperties();
            ArrayList<String> propertiesValues = setProp.getPropertyValues();
            Object[] result = setProp.setProperties( properties, propertiesValues );

            String tempPrint = "\nSetting properties of resource " + setProp.getContentURL();
            int size = tempPrint.length();
            System.out.println( tempPrint );
            tempPrint = "";
            for( int i = 0; i < size; i++ ) {
                tempPrint += "-";
            }
            System.out.println( tempPrint );
            if ( result != null )  {
                for ( int index = 0; index < result.length; index++  ) {
                    Object obj = result[ index ];
                    if( obj == null || obj instanceof com.sun.star.uno.Any )
                        System.out.println(
                            "Setting property " + properties.get( index ) + " succeeded." );
                    else
                        System.out.println(
                            "Setting property " + properties.get( index ) + " failed." );
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
