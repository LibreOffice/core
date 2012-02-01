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



/**
 * Deleting a resource
 */
public class ResourceRemover {

    /**
     * Member properties
     */
    private  Helper   m_helper;
    private  String   m_contenturl    = "";
    private  com.sun.star.ucb.XContent m_content;

    /**
     * Constructor.
     *
     *@param      String[]   This construtor requires the arguments:
     *                          -url=...     (optional)
     *                          -workdir=... (optional)
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     *@exception  java.lang.Exception
     */
    public ResourceRemover( String args[] ) throws java.lang.Exception {

        // Parse arguments
        parseArguments( args );

        // Init
        m_helper       = new Helper( getContentURL() );

        // Create UCB content
        m_content      = m_helper.createUCBContent();
    }

    /**
     *  Delete resource.
     *
     *@return     boolean   Returns true if resource successfully deleted, false otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public boolean deleteResource()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        boolean result = false;
        if ( m_content != null ) {

            /////////////////////////////////////////////////////////////////////
            // Destroy a resource physically...
            /////////////////////////////////////////////////////////////////////

            Boolean deletePhysically = new Boolean( true );

            // Execute command "delete".
            m_helper.executeCommand( m_content, "delete", deletePhysically );
            result = true;
        }
        return result;
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
     *
     *@param      String[]   Arguments
     *@exception  java.lang.Exception
     */
    public void parseArguments( String[] args ) throws java.lang.Exception {

        String workdir = "";

        for ( int i = 0; i < args.length; i++ ) {
            if ( args[i].startsWith( "-url=" )) {
                m_contenturl    = args[i].substring( 5 );
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
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage   : ResourceRemover -url=... -workdir=..." );
        System.out.println(
            "Defaults: -url=<workdir>/resource-<uniquepostfix> -workdir=<currentdir>" );
        System.out.println(
            "\nExample  : -url=file:///temp/MyFile.txt \n" );
    }

    /**
     *  Create a new connection with the specific args to a running office and
     *  delete a resource.
     *
     *@param  String[]   Arguments
     */
    public static void main ( String args[] ) {

        System.out.println( "\n" );
        System.out.println(
            "-----------------------------------------------------------------" );
        System.out.println(
            "ResourceRemover - destroys a resource." );
        System.out.println(
            "-----------------------------------------------------------------" );

        try {
            ResourceRemover delete = new ResourceRemover( args );
            boolean result = delete.deleteResource();
            String url = delete.getContentURL();
            if ( result )  {
                System.out.println(
                        "Delete of resource " + url + " succeeded." );
            } else  {
                System.out.println(
                        "Delete of resource " + url + " failed." );
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
