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

import java.io.File;
import java.io.FileOutputStream;

import com.sun.star.ucb.Command;
import com.sun.star.ucb.UniversalContentBroker;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XCommandProcessor;

import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


/**
 * Helper for creating a new connection with the specific args to a running office.
 */
public class Helper {

    /**
     * Member properties
     */
    private XInterface  m_ucb        = null;
    private String      m_contenturl = null;
    private static XComponentContext   m_xContext   = null;

    /**
     *  Constructor, create a new instance of the ucb. UNO is bootstrapped and
     *  the remote office service manager is used to create the ucb. If necessary
     *  a new office process is started.
     */
    public Helper(String url) throws java.lang.Exception {
        m_contenturl    = url;

        if (null == m_xContext ) {
            // get the remote office component context
            m_xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
        }

        m_ucb = UnoRuntime.queryInterface(
            XInterface.class, UniversalContentBroker.create(m_xContext));
    }

    /**
     * Returns created identifier object for given URL..
     *
     *@return     XContent       Created identifier object for given URL
     */
    public XContent createUCBContent() throws java.lang.Exception {
        return createUCBContent( getContentURL() );
    }

    /**
     * Returned created identifier object for given URL.
     *
     *@param      connectURL         Connect URL. Example : -url=file:///
     *@return  Created identifier object for given URL
     */
    private XContent createUCBContent( String connectURL ) throws java.lang.Exception {
        XContent content = null;
        if ( connectURL != null && !connectURL.equals( "" )) {

            // Obtain required UCB interfaces...
            XContentIdentifierFactory idFactory
                = UnoRuntime.queryInterface(
                XContentIdentifierFactory.class, m_ucb );
            XContentProvider provider
                = UnoRuntime.queryInterface(
                XContentProvider.class, m_ucb );

            // Create identifier object for given URL.
            XContentIdentifier id = idFactory.createContentIdentifier( connectURL );
            content = provider.queryContent( id );
        }
        return content;
    }

    /**
     *  Get ucb instance.
     *
     *@return   XInterface  That contains the ucb  instance
     */
    public XInterface getUCB() {
        return m_ucb;
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
     *  Executes a command.
     *
     *param       XInterface
     *param       String
     *param       Object
     *@return     Object     The result according to the specification of the command.
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    Object executeCommand( XInterface ifc, String commandName, Object argument )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception  {


        // Obtain command processor interface from given content.


        XCommandProcessor cmdProcessor
            = UnoRuntime.queryInterface(
            XCommandProcessor.class, ifc );


        // Assemble command to execute.


        Command command = new Command();
        command.Name     = commandName;
        command.Handle   = -1; // not available
        command.Argument = argument;

        // Note: throws CommandAbortedException, Exception
        return cmdProcessor.execute( command, 0, null );
    }

    public static String getAbsoluteFileURLFromSystemPath( String systemPath )
    {
        try
        {
            File file = new File( systemPath );
            String url = file.toURI().toURL().toString();
            if ( url.charAt( 6 ) != '/' ) { // file:/xxx vs. file:///xxxx
                StringBuffer buf1 = new StringBuffer( "file:///" );
                buf1.append( url.substring( 6 ) );
                url = buf1.toString();
            }
            return url;
        }
        catch ( java.net.MalformedURLException e )
        {
            e.printStackTrace();
        }
        return "";
    }

    public static String prependCurrentDirAsAbsoluteFileURL( String relativeURL )
    {
        // get url of current dir.
        String url = getAbsoluteFileURLFromSystemPath( "" );
        StringBuffer buf = new StringBuffer( url );
        if ( !url.endsWith( File.separator ) )
            buf.append( File.separator );
        buf.append( relativeURL );
        return buf.toString();
    }

    public static String createTargetDataFile( String workDir )
    {
        try
        {
            StringBuffer buf = new StringBuffer();
            if ( workDir != null && workDir.length() > 0 ) {
                buf.append( workDir );
                buf.append( File.separator );
            }
            buf.append( "resource-" );
            buf.append( System.currentTimeMillis() );
            File file = new File( buf.toString() );
            String url = file.toURI().toURL().toString();
            if ( url.charAt( 6 ) != '/' ) { // file:/xxx vs. file:///xxxx
                StringBuffer buf1 = new StringBuffer( "file:///" );
                buf1.append( url.substring( 6 ) );
                url = buf1.toString();
            }

            try
            {
                file.createNewFile();
                String content = "This is the content of a sample data file.";
                FileOutputStream stream = new FileOutputStream( file );
                stream.write( content.getBytes() );
                stream.close();
            }
            catch ( java.io.IOException e )
            {
                e.printStackTrace();
            }

            return url;
        }
        catch ( java.net.MalformedURLException e )
        {
            e.printStackTrace();
        }

        return "";
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
