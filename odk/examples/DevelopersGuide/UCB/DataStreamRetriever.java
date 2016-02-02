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
     *@param      args   This constructor requires the arguments:
     *                          -url=... (optional)
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
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
     */
    public XInputStream getDataStream()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        XInputStream data = null;
        if ( m_content != null ) {

            // Fill argument structure...
            OpenCommandArgument2 arg = new OpenCommandArgument2();
            arg.Mode = OpenMode.DOCUMENT;
            arg.Priority = 32768; // static final for 32768

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
     *@param      args   Arguments
     */
    private void parseArguments( String[] args ) throws java.lang.Exception {

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
    private void printCmdLineUsage() {
        System.out.println(
            "Usage   : DataStreamRetriever -url=..." );
        System.out.println(
            "Defaults: -url=<currentdir>/data/data.txt" );
        System.out.println(
            "\nExample : -url=file:///temp/my.txt" );
    }

    /**
     *  Print Stream content.
     */
    private void printStream( XInputStream data )
        throws com.sun.star.uno.Exception {


        // Read data from input stream...65536


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
