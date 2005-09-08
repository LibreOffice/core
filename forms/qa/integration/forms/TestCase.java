/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestCase.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:31:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package integration.forms;

import com.sun.star.uno.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XModifiable;
import integration.forms.DocumentType;

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
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
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
                XModifiable docModify = (XModifiable)m_document.query( XModifiable.class );
                docModify.setModified( false );

                m_document.getCurrentView().dispatch( ".uno:CloseDoc" );

                // CloseDoc is asynchronous, so wait until it's done - or 1 second, at most
                synchronized ( this ) { wait( 1000 ); }
            }
        }
        catch ( java.lang.Exception e )
        {
            e.printStackTrace( System.out );
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
                XCloseable closeDoc = (XCloseable)m_document.query( XCloseable.class );
                closeDoc.close( true );
            }
        }
        catch ( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.out );
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
