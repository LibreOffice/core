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



import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.ucb.ContentInfo;
import com.sun.star.ucb.InsertCommandArgument;
import com.sun.star.ucb.XContent;
import com.sun.star.io.XInputStream;


/**
 * Creating a New Resource
 */
public class ResourceCreator {

    /**
     * Member properties
     */
    private  Helper    m_helper;
    private  XContent  m_content;
    private  String    m_contenturl    = "";
    private  String    m_name          = "";
    private  String    m_srcURL        = "";

    /**
     * Constructor.
     *
     *@param      String[]   This construtor requires the arguments:
     *                          -url=...     (optional)
     *                          -name=...    (optional)
     *                          -srcURL=...  (optional)
     *                          -workdir=... (optional)
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     *@exception  java.lang.Exception
     */
    public ResourceCreator( String args[] ) throws java.lang.Exception {

        // Parse arguments
        parseArguments( args );
        String url     = getContentURL();

        // Init
        m_helper = new Helper( url );
        if ( url.startsWith( "file:///" )) {

            // Create UCB content
            m_content  = m_helper.createUCBContent();
        } else  {
            throw new Exception(
                "Create new resource : parameter 'url' must contain a File URL " +
                "pointing to the file system folder in which the new resource " +
                "shall be created. (Example: file:///tmp/)" );
        }
    }

    /**
     *  Create a new resource.
     *  This method requires the main and the optional arguments to be set in order to work.
     *  See Constructor.
     *
     *@return boolean  Returns true if resource successfully created, false otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public boolean createNewResource()
        throws com.sun.star.ucb.CommandAbortedException,
               com.sun.star.uno.Exception,
               java.lang.Exception {

        String sourceURL         = getSourceURL();
        String name              = getName();
        return createNewResource( sourceURL, name );
    }

    /**
     *  Create a new resource.
     *
     *@param  String   Source resource URL
     *@param  String   New resource name
     *@return boolean  Returns true if resource successfully created, false otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public boolean createNewResource( String sourceURL, String name )
        throws com.sun.star.ucb.CommandAbortedException,
               com.sun.star.uno.Exception,
               java.lang.Exception {

        XInputStream stream = null;
        if ( sourceURL == null || sourceURL.equals( "" )) {
            stream = new MyInputStream();
        } else  {
            String[] args =  new String[ 1 ];
            args[ 0 ] = "-url=" + sourceURL;
            DataStreamRetriever access = new DataStreamRetriever( args );
            stream = access.getDataStream();
        }
        return createNewResource( stream, name );
    }

    /**
     *  Create a new resource.
     *
     *@param  XInputStream   Source resource stream
     *@param  String         New resource name
     *@return boolean        Returns true if resource successfully created, false otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public boolean createNewResource( XInputStream stream, String name )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        boolean result = false;
        if ( stream != null && name != null && !name.equals( "" )) {

            // Note: The data for info may have been obtained from
            //       property CreatableContentsInfo.
            ContentInfo info = new ContentInfo();
            info.Type = "application/vnd.sun.staroffice.fsys-file";
            info.Attributes = 0;

            // Create new, empty content (execute command "createNewContent").
            XContent newContent = ( XContent )UnoRuntime.queryInterface(
                XContent.class,
                m_helper.executeCommand( m_content, "createNewContent", info ) );

            if ( newContent != null ) {

                /////////////////////////////////////////////////////////////////////
                // Set mandatory properties...
                /////////////////////////////////////////////////////////////////////

                // Define property value sequence.
                PropertyValue[] props = new PropertyValue[ 1 ];
                PropertyValue prop = new PropertyValue();
                prop.Name   = "Title";
                prop.Handle = -1; // n/a
                prop.Value  = name;
                props[ 0 ] = prop;

                // Execute command "setPropertyValues".
                m_helper.executeCommand( newContent, "setPropertyValues", props );

                /////////////////////////////////////////////////////////////////////
                // Write the new file to disk...
                /////////////////////////////////////////////////////////////////////

                // Obtain document data for the new file.
                XInputStream data = stream;

                // Fill argument structure...
                InsertCommandArgument arg = new InsertCommandArgument();
                arg.Data = data;
                arg.ReplaceExisting = false;

                // Execute command "insert".
                m_helper.executeCommand( newContent, "insert", arg );
                result = true;
            }
        }
        return result;
    }

    /**
     * Get new resource name.
     *
     *@return String    That contains the name
     */
    public String getName() {
        return m_name;
    }

    /**
     * Get source URL.
     *
     *@return String    That contains the source URL
     */
    public String getSourceURL() {
        return m_srcURL;
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
                m_contenturl = args[i].substring( 5 );
            } else if ( args[i].startsWith( "-name=" )) {
                m_name = args[i].substring( 6 );
            } else if ( args[i].startsWith( "-srcURL=" )) {
                m_srcURL = args[i].substring( 8 );
            } else if ( args[i].startsWith( "-workdir=" )) {
                workdir = args[i].substring( 9 );
            } else if ( args[i].startsWith( "-help" ) ||
                        args[i].startsWith( "-?" )) {
                printCmdLineUsage();
                System.exit( 0 );
            }
        }

        if ( m_contenturl == null || m_contenturl.equals( "" )) {
            m_contenturl = Helper.getAbsoluteFileURLFromSystemPath( workdir );
        }

        if ( m_name == null || m_name.equals( "" )) {
            m_name = "created-resource-" + System.currentTimeMillis();
        }

        if ( m_srcURL == null || m_srcURL.equals( "" )) {
            m_srcURL = Helper.prependCurrentDirAsAbsoluteFileURL( "data/data.txt" );
        }
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage   : ResourceCreator -url=... -name=... -srcURL=... -workdir=..." );
        System.out.println(
            "Defaults: -url=<workdir> -name=created-resource-<uniquepostfix> -srcURL=<currentdir>/data/data.txt> -workdir=<currentdir>" );
        System.out.println(
            "\nExample : -url=file:///home/kai/ -name=newfile.txt -srcURL=file:///home/kai/sourcefile.txt" );
    }

    /**
     *  Create a new connection with the specific args to a running office and
     *  create a new resource.
     *
     *@param  String[]   Arguments
     */
    public static void main ( String args[] ) {
        System.out.println( "\n" );
        System.out.println(
            "-----------------------------------------------------------------------" );
        System.out.println(
            "ResourceCreator - creates a new file in an existing file system folder." );
        System.out.println(
            " (Content for the new file can be retrieved from another file)." );
        System.out.println(
            "-----------------------------------------------------------------------" );
        try {
            ResourceCreator create = new ResourceCreator( args );
            boolean result = create.createNewResource();
            if ( result )  {
                System.out.println(
                    "Creation of new resource " + create.getName() + " in folder: " +
                    create.getContentURL() + " succeeded." );
            } else  {
                System.out.println(
                    "Creation of new resource " + create.getName() + " in folder: " +
                    create.getContentURL() + " failed." );
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
