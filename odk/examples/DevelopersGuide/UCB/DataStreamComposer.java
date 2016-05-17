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

import com.sun.star.ucb.InsertCommandArgument;
import com.sun.star.ucb.XContent;
import com.sun.star.io.XInputStream;

/**
 * Setting (Storing) the Content Data Stream of a UCB Document Content.
 */
public class DataStreamComposer {

    /**
     * Member properties
     */
    private  Helper    m_helper;
    private  XContent  m_content;
    private  String    m_contenturl    = "";
    private  String    m_srcURL        = "";


    /**
     * Constructor.
     *
     *@param      args   This constructor requires the arguments:
     *                          -url=...               (optional)
     *                          -srcURL=...            (optional)
     *                          -workdir=...           (optional)
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     */
    public DataStreamComposer( String args[] ) throws java.lang.Exception {

        // Parse arguments
        parseArguments( args );

        // Init
        m_helper       = new Helper( getContentURL() );

        // Create UCB content
        m_content      = m_helper.createUCBContent();
    }

    /**
     * Write the document data stream of a document content.
     * This method requires the main and the optional arguments to be set in order to work.
     * See Constructor.
     *
     *@return boolean   Result
     */
    public boolean setDataStream()
        throws com.sun.star.ucb.CommandAbortedException,
               com.sun.star.uno.Exception,
               java.lang.Exception {

         String sourceURL         = getSourceURL();
         return ( setDataStream( sourceURL ));
    }

    /**
     * Write the document data stream of a document content.
     *
     *@param  sourceURL    Source URL
     *@return true if data stream successfully seted, false otherwise
     */
    public boolean setDataStream( String sourceURL )
        throws com.sun.star.ucb.CommandAbortedException,
               com.sun.star.uno.Exception,
               java.lang.Exception {

         XInputStream stream;
         if ( sourceURL == null || sourceURL.equals("") )  {
            stream = new MyInputStream();
         } else {
            String[] args =  new String[ 1 ];
            args[ 0 ] = "-url=" + sourceURL;
            DataStreamRetriever access = new DataStreamRetriever( args );
            stream = access.getDataStream();
         }
         return ( setDataStream( stream ));
    }

    /**
     * Write the document data stream of a document content...
     *
     *@return boolean        Returns true if data stream successfully seted, false otherwise
     */
    public boolean setDataStream( XInputStream stream )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        boolean result = false;
        XInputStream data = stream;
        if ( data != null && m_content != null )  {

            // Fill argument structure...
            InsertCommandArgument arg = new InsertCommandArgument();
            arg.Data = data;
            arg.ReplaceExisting = true;

            // Execute command "insert".
            m_helper.executeCommand( m_content, "insert", arg );
            result = true;
        }
        return result;
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
     */
    public void parseArguments( String[] args ) throws java.lang.Exception {

        String workdir = "";

        for ( int i = 0; i < args.length; i++ ) {
            if ( args[i].startsWith( "-url=" )) {
                m_contenturl    = args[i].substring( 5 );
            } else if ( args[i].startsWith( "-srcURL=" )) {
                m_srcURL = args[i].substring( 9 );
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

        if ( m_srcURL == null || m_srcURL.equals( "" )) {
            m_srcURL = Helper.prependCurrentDirAsAbsoluteFileURL( "data/data.txt" );
        }
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage   : DataStreamComposer -url=... -srcURL=... -workdir=..." );
        System.out.println(
            "Defaults: -url=<workdir>/resource-<uniquepostfix> -srcURL=<currentdir>/data/data.txt -workdir=<currentdir>" );
        System.out.println(
            "\nExample : -url=file:///temp/my.txt -srcURL=file:///temp/src.txt " );
    }


    /**
     *  Create a new connection with the specific args to a running office and
     *  set the Content Data Stream of a UCB Document Content.
     */
    public static void main ( String args[] ) {
        System.out.println( "\n" );
        System.out.println(
            "-----------------------------------------------------------------" );
        System.out.println(
            "DataStreamComposer - sets the data stream of a document resource." );
        System.out.println(
            " The data stream is obtained from another (the source) document " );
        System.out.println(
            " resource before." );
        System.out.println(
            "-----------------------------------------------------------------" );
        try {

            DataStreamComposer dataStream = new DataStreamComposer( args );
            String sourceURL         = dataStream.getSourceURL();
            boolean result = dataStream.setDataStream( sourceURL );
            if ( result ) {
                System.out.println(
                        "\nSetting data stream succeeded.\n   Source URL: " +
                        dataStream.getSourceURL() +
                        "\n   Target URL: " +
                        dataStream.getContentURL() );
            } else {
                System.out.println(
                        "\nSetting data stream failed. \n   Source URL: " +
                        dataStream.getSourceURL() +
                        "\n   Target URL: " +
                        dataStream.getContentURL() );
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
