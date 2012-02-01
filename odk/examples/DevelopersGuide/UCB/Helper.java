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

import java.io.File;
import java.io.FileOutputStream;

import com.sun.star.lang.XMultiComponentFactory;

import com.sun.star.ucb.Command;
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
     *  the remote office service manger is used to create the ucb. If necessary
     *  a new office process is started.
     *
     *  @exception  java.lang.Exception
     */
    public Helper(String url) throws java.lang.Exception {
        m_contenturl    = url;

        if (null == m_xContext ) {
            // get the remote office component context
            m_xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
        }

        XMultiComponentFactory xMCF = m_xContext.getServiceManager();

        m_ucb = (XInterface)UnoRuntime.queryInterface(XInterface.class,
            xMCF.createInstanceWithContext(
                        "com.sun.star.ucb.UniversalContentBroker", m_xContext));
    }

    /**
     * Returns created identifier object for given URL..
     *
     *@return     XContent       Created identifier object for given URL
     *@exception  java.lang.Exception
     */
    public XContent createUCBContent() throws java.lang.Exception {
        return createUCBContent( getContentURL() );
    }

    /**
     * Returned created identifier object for given URL.
     *
     *@param      String         Connect URL. Example : -url=file:///
     *@return     XContent       Created identifier object for given URL
     *@exception  java.lang.Exception
     */
    public XContent createUCBContent( String connectURL ) throws java.lang.Exception {
        XContent content = null;
        if ( connectURL != null && !connectURL.equals( "" )) {

            // Obtain required UCB interfaces...
            XContentIdentifierFactory idFactory
                = ( XContentIdentifierFactory )UnoRuntime.queryInterface(
                    XContentIdentifierFactory.class, m_ucb );
            XContentProvider provider
                = ( XContentProvider )UnoRuntime.queryInterface(
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

        /////////////////////////////////////////////////////////////////////
        // Obtain command processor interface from given content.
        /////////////////////////////////////////////////////////////////////

        XCommandProcessor cmdProcessor
            = (XCommandProcessor)UnoRuntime.queryInterface(
                XCommandProcessor.class, ifc );

        /////////////////////////////////////////////////////////////////////
        // Assemble command to execute.
        /////////////////////////////////////////////////////////////////////

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
            String url = file.toURL().toString();
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
        return new String();
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
            String url = file.toURL().toString();
            if ( url.charAt( 6 ) != '/' ) { // file:/xxx vs. file:///xxxx
                StringBuffer buf1 = new StringBuffer( "file:///" );
                buf1.append( url.substring( 6 ) );
                url = buf1.toString();
            }

            try
            {
                file.createNewFile();
                String content = new String(
                    "This is the content of a sample data file." );
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

        return new String();
    }
}
