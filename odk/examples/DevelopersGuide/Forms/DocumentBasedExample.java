/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentBasedExample.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:07:31 $
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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.util.XCloseable;

public abstract class DocumentBasedExample implements com.sun.star.lang.XEventListener
{
    /// the intial remote context from the office
    protected XComponentContext       m_xCtx;
    /// our current test document
    protected DocumentHelper          m_document;
    protected FormLayer               m_formLayer;
    protected DocumentType            m_documentType;

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
            // get the remote office component context
            m_xCtx = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
        }
        catch (java.lang.Exception e)
        {
            e.printStackTrace();
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
            // collect whatever parameters where given
            collectParameters( argv );

            // prepare our sample document
            prepareDocument();

            // switch the document view's form layer to alive mode
            m_document.getCurrentView().toggleFormDesignMode();

            // grab the focus to the first control
            m_document.getCurrentView().grabControlFocus();

            // ----------------------------------------------
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
    protected void collectParameters(String argv[])
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
                XCloseable closeDoc = (XCloseable)
                    UnoRuntime.queryInterface( XCloseable.class,
                                               m_document.getDocument() );
                if (closeDoc != null)
                    closeDoc.close( true );
                else
                    m_document.getDocument().dispose();
            }
        }
        catch ( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.out );
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
