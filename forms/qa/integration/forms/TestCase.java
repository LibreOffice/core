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

package integration.forms;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XModifiable;

public abstract class TestCase extends complexlib.ComplexTestCase implements com.sun.star.lang.XEventListener
{
    protected XMultiServiceFactory    m_orb;              /// our service factory
    protected DocumentType            m_documentType;     /// the type of our document
    protected DocumentHelper          m_document;         /// our current test document
    protected FormLayer               m_formLayer;

    /** Creates a new instance of TestCase */
    public TestCase( DocumentType docType )
    {
        m_documentType = docType;
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return this.getClass().getName();
    }

    /* ------------------------------------------------------------------ */
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
    }

    /* ------------------------------------------------------------------ */
    public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
    }

    /* ------------------------------------------------------------------ */
    /** closes our document, if we have an open one, via (simulated) user input
     */
    protected void closeDocumentByUI()
    {
        try
        {
            if ( m_document != null )
            {
                // first, set the document to "unmodified"
                XModifiable docModify = m_document.query( XModifiable.class );
                docModify.setModified( false );

                m_document.getCurrentView().dispatch( ".uno:CloseDoc" );

                // CloseDoc is asynchronous, so wait until it's done - or 1 second, at most
                synchronized ( this ) { wait( 1000 ); }
            }
        }
        catch ( java.lang.Exception e )
        {
            e.printStackTrace( System.err );
        }
    }

    /* ------------------------------------------------------------------ */
    /** closes our document, if we have an open one
     */
    protected void closeDocument()
    {
        try
        {
            // close our document
            if ( m_document != null )
            {
                XCloseable closeDoc = m_document.query( XCloseable.class );
                closeDoc.close( true );
            }
        }
        catch ( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.err );
        }
    }

    /* ------------------------------------------------------------------ */
    /** prepares a new document to work with
     */
    protected void prepareDocument() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_document = DocumentHelper.blankDocument( m_orb, m_documentType );
        m_document.getDocument( ).addEventListener( this );
        m_formLayer = new FormLayer( m_document );
    }

    /* ------------------------------------------------------------------ */
    /* internal methods                                                   */
    /* ------------------------------------------------------------------ */
    /** waits for the user to press a key (on the console where she started the java program)
        or the document to be closed by the user.
    @return
        <TRUE/> if the user pressed a key on the console, <FALSE/> if she closed the document
    */
    protected boolean waitForUserInput() throws java.lang.Exception
    {
        synchronized (this)
        {
            integration.forms.WaitForInput aWait = new integration.forms.WaitForInput( this );
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
