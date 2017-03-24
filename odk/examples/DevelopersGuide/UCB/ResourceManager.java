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

import com.sun.star.ucb.NameClash;
import com.sun.star.ucb.TransferCommandOperation;
import com.sun.star.ucb.GlobalTransferCommandArgument;
import com.sun.star.uno.XInterface;

/**
 * Copying, Moving and Creating Links to a Resource
 */
public class ResourceManager {

    /**
     * Member properties
     */
    private  Helper      m_helper;
    private  XInterface  m_ucb;
    private  String      m_contenturl = "";
    private  String      m_targetFolderURL = "";
    private  String      m_newTitle = "";
    private  String      m_transOperation = "";

    /**
     * Constructor.
     *
     *@param      args   This constructor requires the arguments:
     *                          -url=...             (optional)
     *                          -targetFolderURL=... (optional)
     *                          -newTitle=...        (optional)
     *                          -transOper=...       (optional)
     *                          -workdir=...         (optional)
     *                       See Help (method printCmdLineUsage()).
     *                       Without the arguments a new connection to a
     *                       running office cannot created.
     */
    public ResourceManager( String args[] ) throws java.lang.Exception {

        // Parse arguments
        parseArguments( args );

        // Init
        m_helper       = new Helper( getContentURL() );

        // Get xUCB
        m_ucb          = m_helper.getUCB();
    }

    /**
     *  Copy, move or create a link for a resource.
     *  This method requires the main and the optional arguments to be set in order to work.
     *  See Constructor.
     *
     *@return true if resource successfully transferred, false otherwise
     */
    public boolean transferResource()
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {
        String sourceURL       = getContentURL();      // URL of the source object
        String targetFolderURL = getTargetFolderURL(); // URL of the target folder
        String newTitle        = getNewTitle();        // New name for the resource
        String transOperation  = getTransOperation();
        return transferResource( sourceURL, targetFolderURL, newTitle, transOperation );
    }

    /**
     *  Copy, move or create a link for a resource.
     *
     *@param  sourceURL   Source URL
     *@param  targetFolderURL   Target folder URL
     *@param  transOperation   Transferring operation (copy, move, link)
     *@return true if resource successfully transferred, false otherwise
     */
    public boolean transferResource(
            String sourceURL, String targetFolderURL,
            String newTitle, String transOperation )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception {

        boolean result = false;
        if ( m_ucb != null && sourceURL != null && !sourceURL.equals( "" ) &&
             targetFolderURL != null && !targetFolderURL.equals( "" ) &&
             newTitle != null && transOperation != null && !transOperation.equals( "" ) &&
             ( transOperation.equals( "copy" ) || transOperation.equals( "move" ) ||
               transOperation.equals( "link" ))) {


            // Copy, move or create a link for a resource to another location...

            GlobalTransferCommandArgument arg = new GlobalTransferCommandArgument();
            if ( transOperation.equals( "copy" )) {
                arg.Operation = TransferCommandOperation.COPY;
            } else if ( transOperation.equals( "move" )) {
                arg.Operation = TransferCommandOperation.MOVE;
            } else if ( transOperation.equals( "link" )) {
                arg.Operation = TransferCommandOperation.LINK;
            }
            arg.SourceURL = sourceURL;
            arg.TargetURL = targetFolderURL;

            // object get a new unique name
            arg.NewTitle  = newTitle;

            // fail, if object with same name exists in target folder
            arg.NameClash = NameClash.ERROR;

            // Let UCB execute the command "globalTransfer".
            m_helper.executeCommand( m_ucb, "globalTransfer", arg );
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
     * Get trasfering Operation.
     *
     *@return String    That contains the trasfering Operation
     */
    public String getTransOperation() {
        return m_transOperation;
    }

    /**
     * Get target folder URL.
     *
     *@return String    That contains the target folder URL
     */
    public String getTargetFolderURL() {
        return m_targetFolderURL;
    }

    /**
     * Get new title for the resource to be transferred.
     *
     *@return String    That contains a new title for the transferred
     *                  resource. Can be empty. In this case resource
     *                  will keep the title it has in the source folder.
     */
    public String getNewTitle() {
        return m_newTitle;
    }

    /**
     * Parse arguments
     */
    public void parseArguments( String[] args ) throws java.lang.Exception {

        String workdir = "";

        for ( int i = 0; i < args.length; i++ ) {
            if ( args[i].startsWith( "-url=" )) {
                m_contenturl    = args[i].substring( 5 );
            } else if ( args[i].startsWith( "-targetFolderURL=" )) {
                m_targetFolderURL = args[i].substring( 17 );
            } else if ( args[i].startsWith( "-newTitle=" )) {
                m_newTitle = args[i].substring( 10 );
            } else if ( args[i].startsWith( "-transOper=" )) {
                m_transOperation = args[i].substring( 11 );
            } else if ( args[i].startsWith( "-workdir=" )) {
                workdir = args[i].substring( 9 );
            } else if ( args[i].startsWith( "-help" ) ||
                        args[i].startsWith( "-?" )) {
                printCmdLineUsage();
                System.exit( 0 );
            }
        }

        if ( m_contenturl == null || m_contenturl.equals( "" )) {
            m_contenturl = Helper.prependCurrentDirAsAbsoluteFileURL( "data/data.txt" );
        }

        if ( m_targetFolderURL == null || m_targetFolderURL.equals( "" )) {
            m_targetFolderURL = Helper.getAbsoluteFileURLFromSystemPath( workdir );
        }

        if ( m_newTitle == null || m_newTitle.equals( "" )) {
            m_newTitle = "transferred-resource-" + System.currentTimeMillis();
        }

        if ( m_transOperation == null || m_transOperation.equals( "" )) {
            m_transOperation = "copy";
        }
    }

    /**
     * Print the commands options
     */
    public void printCmdLineUsage() {
        System.out.println(
            "Usage: ResourceManager -url=... -targetFolderURL=... -newTitle=... -transOper=... -workdir=..." );
        System.out.println(
            "Defaults: -url=<currentdir>/data/data.txt> -targetFolderURL=<workdir> -newTitle=transferred-resource-<uniquepostfix> -transOper=copy -workdir=<currentdir>");
        System.out.println(
            "\nExample : -url=file:///temp/MyFile.txt -targetFolderURL=file:///test/ -newTitle=RenamedFile.txt -transOper=copy " );
    }

    /**
     *  Create a new connection with the specific args to a running office and
     *  copy, move or create links a resource.
     */
    public static void main ( String args[] ) {

        System.out.println( "\n" );
        System.out.println(
            "-----------------------------------------------------------------" );
        System.out.println(
            "ResourceManager - copies/moves a resource." );
        System.out.println(
            "-----------------------------------------------------------------" );

        try {
            ResourceManager transResource = new ResourceManager( args );
            String sourceURL       = transResource.getContentURL();
            String targetFolderURL = transResource.getTargetFolderURL();
            String newTitle        = transResource.getNewTitle();
            String transOperation  = transResource.getTransOperation();
            boolean result = transResource.transferResource(
                                sourceURL, targetFolderURL, newTitle, transOperation );
            if ( result )
                System.out.println( "\nTransferring resource succeeded." );
            else
                System.out.println( "Transferring resource failed." );

            System.out.println( "   Source URL        : " + sourceURL );
            System.out.println( "   Target Folder URL : " + targetFolderURL );
            System.out.println( "   New name          : " + newTitle );
            System.out.println( "   Transfer Operation: " + transOperation );


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
