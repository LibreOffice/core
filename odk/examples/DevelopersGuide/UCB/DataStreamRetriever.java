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



import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.XContent;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;

/**
 * Accessing (Loading) the Content Data Stream of a UCB Document Content
 */
public class DataStreamRetriever {

    /**
     * Member properties
     */
    private  Helper   m_helper;
    private  XContent m_content;
    private  String   m_contenturl    = "";

    /**
     * Constructor.
     *
     *@param      String[]   This construtor requires the arguments:
     *                          -url=... (optional)
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     *@exception  java.lang.Exception
     */
    public DataStreamRetriever( String args[] ) throws java.lang.Exception {

        // Parse arguments
        parseArguments( args );

        // Init
        m_helper       = new Helper( getContentURL() );

        // Create UCB content
        m_content      = m_helper.createUCBContent();
    }

    /**
     *  Read the document data stream of a document content using a
     *  XActiveDataSink implementation as data sink....
     *
     *@return     XInputStream  Returns input stream if stream successfully retrieved,
     *                          null otherwise
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    public XInputStream getDataStream()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        XInputStream data = null;
        if ( m_content != null ) {

            // Fill argument structure...
            OpenCommandArgument2 arg = new OpenCommandArgument2();
            arg.Mode = OpenMode.DOCUMENT;
            arg.Priority = 32768; // Final static for 32768

            // Create data sink implementation object.
            XActiveDataSink dataSink = new MyActiveDataSink();
            arg.Sink = dataSink;

            // Execute command "open". The implementation of the command will
            // supply an XInputStream implementation to the data sink.
            m_helper.executeCommand( m_content, "open", arg );

            // Get input stream supplied by the open command implementation.
            data = dataSink.getInputStream();
        }
        return data;
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
     * Parse arguments
     *
     *@param      String[]   Arguments
     *@exception  java.lang.Exception
     */
    public void parseArguments( String[] args ) throws java.lang.Exception {

        for ( int i = 0; i < args.length; i++ ) {
            if ( args[i].startsWith( "-url=" )) {
                m_contenturl    = args[i].substring( 5 );
            } else if ( args[i].startsWith( "-help" ) ||
                        args[i].startsWith( "-?" )) {
                printCmdLineUsage();
                System.exit( 0 );
            }
        }

        if ( m_contenturl == null || m_contenturl.equals( "" )) {
            m_contenturl = Helper.prependCurrentDirAsAbsoluteFileURL( "data/data.txt" );
        }
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage   : DataStreamRetriever -url=..." );
        System.out.println(
            "Defaults: -url=<currentdir>/data/data.txt" );
        System.out.println(
            "\nExample : -url=file:///temp/my.txt" );
    }

    /**
     *  Print Stream content.
     *
     *@param    XInputStream
     *@exception  com.sun.star.uno.Exception
     */
    public void printStream( XInputStream data )
        throws com.sun.star.uno.Exception {

        /////////////////////////////////////////////////////////////////////
        // Read data from input stream...65536
        /////////////////////////////////////////////////////////////////////

        // Data buffer. Will be allocated by input stream implementation!
        byte[][] buffer = new byte[ 1 ][ 65536 ];
        int read = data.readSomeBytes( buffer, 65536 );
        System.out.println( "Read bytes : " + read );
        System.out.println( "Read data (only first 64K displayed): ");
        while ( read > 0 ) {
            byte[] bytes =  new byte[ read ];
            for( int i = 0; i < read; i++ ) {
                bytes[ i ] = buffer[ 0 ][ i ];
            }
            System.out.println( new String(bytes) );

            // Process data contained in buffer.
            read = data.readSomeBytes( buffer, 65536 );
        }
    }

    /**
     *  Create a new connection with the specific args to a running office and
     *  access (Load) the content data stream of a UCB document content.
     *
     *@param  String[]   Arguments
     */
    public static void main ( String args[] ) {
        System.out.println( "\n" );
        System.out.println(
            "-----------------------------------------------------------------------" );
        System.out.println(
            "DataStreamRetriever - obtains the data stream from a document resource." );
        System.out.println(
            "-----------------------------------------------------------------------" );

        try {

            DataStreamRetriever access = new DataStreamRetriever( args );
            XInputStream data = access.getDataStream();
            String url = access.getContentURL();
            if ( data != null )  {
                String tempPrint = "\nGetting data stream for resource " + url +
                " succeeded.";
                int size = tempPrint.length();
                System.out.println( tempPrint );
                tempPrint = "";
                for( int i = 0; i < size; i++ ) {
                    tempPrint += "-";
                }
                System.out.println( tempPrint );
                access.printStream( data );
            } else {
                System.out.println(
                    "Getting data stream for resource " + url + " failed." );
            }
        } catch ( com.sun.star.io.NotConnectedException e ) {
            System.out.println( "Error: " + e );
        } catch ( com.sun.star.io.BufferSizeExceededException e ) {
            System.out.println( "Error: " + e );
        } catch ( com.sun.star.io.IOException e ) {
            System.out.println( "Error: " + e );
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
