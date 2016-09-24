/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XCloseable;

public abstract class DocumentBasedExample implements com.sun.star.lang.XEventListener
{
    /// the initial remote context from the office
    protected XComponentContext       m_xCtx;
    /// our current test document
    protected DocumentHelper          m_document;
    protected FormLayer               m_formLayer;
    private DocumentType            m_documentType;

    /** Creates a new instance of DocumentBasedExample */
    public DocumentBasedExample( DocumentType documentType )
    {
        bootstrapUNO();
        m_documentType = documentType;
    }

    /* ------------------------------------------------------------------ */
    private void bootstrapUNO()
    {
        try
        {
            /*
            final XComponentContext componentContext = com.sun.star.comp.helper.Bootstrap.
                createInitialComponentContext( null );
            final XMultiComponentFactory localServiceManager = componentContext.getServiceManager();

            final XUnoUrlResolver urlResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
                XUnoUrlResolver.class, localServiceManager.createInstanceWithContext(
                    "com.sun.star.bridge.UnoUrlResolver", componentContext) );

            final String connectStr = "uno:pipe,name=<pipename>;urp;StarOffice.ComponentContext";
            final Object initialObject = urlResolver.resolve( connectStr );

            m_xCtx = (XComponentContext)UnoRuntime.queryInterface( XComponentContext.class,
                initialObject );
            */

            // get the remote office component context
            m_xCtx = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
        }
        catch (java.lang.Exception e)
        {
            e.printStackTrace( System.err );
            System.exit(1);
        }
    }

    /* ------------------------------------------------------------------ */
    /** main method for running the sample
     */
    public void run( String argv[] )
    {
        try
        {
            // collect whatever parameters were given
            collectParameters( argv );

            // prepare our sample document
            prepareDocument();

            // switch the document view's form layer to alive mode
            m_document.getCurrentView().toggleFormDesignMode();
            onFormsAlive();

            // grab the focus to the first control
            m_document.getCurrentView().grabControlFocus();


            // wait for the user to confirm that we can exit
            if ( waitForUserInput() )
            {
                // clean up
                cleanUp();
            }

            // if waitForUserInput returns false, the user closed the document manually - no need to do a clean up
            // then
        }
        catch(com.sun.star.uno.Exception e)
        {
            System.out.println(e);
            e.printStackTrace();
        }
        catch(java.lang.Exception e)
        {
            System.out.println(e);
            e.printStackTrace();
        }

        System.exit(0);
    }

    /* ------------------------------------------------------------------ */
    /** collect the RuntimeArguments
    */
    private void collectParameters(String argv[])
    {
        // not interested in. Derived classes may want to use it.
    }

    /* ------------------------------------------------------------------ */
    /** prepares a new document to work with
     */
    protected void prepareDocument() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_document = DocumentHelper.blankDocument(m_xCtx, m_documentType);
        m_document.getDocument( ).addEventListener( this );
        m_formLayer = new FormLayer( m_document );
    }

    /* ------------------------------------------------------------------ */
    /** called when the form layer has been switched to alive mode
    */
    protected void onFormsAlive()
    {
    }

    /* ------------------------------------------------------------------ */
    /** performs any cleanup before exiting the program
    */
    protected void cleanUp( ) throws java.lang.Exception
    {
        // do not listen at the document any longer
        m_document.getDocument().removeEventListener( this );

        // close the document
        closeDocument();
    }

    /* ------------------------------------------------------------------ */
    /** closes our document, if we have an open one
     */
    private void closeDocument()
    {
        try
        {
            // close our document
            if ( m_document != null )
            {
                XCloseable closeDoc = UnoRuntime.queryInterface( XCloseable.class,
                                           m_document.getDocument() );
                if (closeDoc != null)
                    closeDoc.close( true );
                else
                    m_document.getDocument().dispose();
            }
        }
        catch ( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.err );
        }
    }

    /* ------------------------------------------------------------------ */
    /* internal methods                                                   */
    /* ------------------------------------------------------------------ */
    /** waits for the user to press a key (on the console where she started
        the java program) or the document to be closed by the user.

        @return <TRUE/> if the user pressed a key on the console,
                <FALSE/> if she closed the document
    */
    protected boolean waitForUserInput() throws java.lang.Exception
    {
        synchronized (this)
        {
            WaitForInput aWait = new WaitForInput( this );
            aWait.start();
            wait();

            // if the waiter thread is done, the user pressed enter
            boolean bKeyPressed = aWait.isDone();
            if ( !bKeyPressed )
                aWait.interrupt();

            return bKeyPressed;
        }
    }

    /* ------------------------------------------------------------------ */
    /* XEventListener overridables                                        */
    /* ------------------------------------------------------------------ */
    public void disposing( com.sun.star.lang.EventObject eventObject )
    {
        if ( m_document.getDocument().equals( eventObject.Source ) )
    {
            // notify ourself that we can stop waiting for user input
            synchronized (this)
            {
                notify();
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
