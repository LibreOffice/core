package complex.sfx2;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertArrayEquals;

import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.tools.OfficeDocument;

import com.sun.star.document.DocumentEvent;
import com.sun.star.document.XDocumentEventBroadcaster;
import com.sun.star.document.XDocumentEventListener;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XEventListener;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseListener;
import com.sun.star.util.XCloseable;

public class DocumentEvents extends JUnitBasedTest
{
    @Before
    public void beforeTest() throws Exception
    {
        m_document = OfficeDocument.blankTextDocument( this.getORB() );
    }

    @After
    public void afterTest()
    {
        if ( m_document != null )
        {
            assertTrue( "closing the test document failed", m_document.close() );
            m_document = null;
        }
    }

    /**
     * sets up the environment for a test which checks the behavior upon closing a doc
     */
    private void impl_setupDocCloseTest()
    {
        m_observedCloseEvents.clear();

        final XDocumentEventBroadcaster docEventBroadcaster = UnoRuntime.queryInterface(
            XDocumentEventBroadcaster.class, m_document.getDocument() );
        docEventBroadcaster.addDocumentEventListener( new DocumentEventListener() );

        final XCloseable docCloseable = UnoRuntime.queryInterface( XCloseable.class,
                m_document.getDocument() );
        docCloseable.addCloseListener( new CloseListener() );

        m_document.getDocument().addEventListener( new DocDisposeListener() );
    }

    /**
     * sets up the environment for a test which checks the behavior upon closing a doc
     */
    private void impl_tearDownDocCloseTest( final String i_docCloseMethod )
    {
        synchronized( m_document )
        {
            try
            {
                m_document.wait(10000);
            }
            catch (InterruptedException ex)
            {
                // don't continue the test if somebody interrupted us ...
                return;
            }
        }

        m_document = null;
        synchronized( m_observedCloseEvents )
        {
            assertArrayEquals(
                "wrong order of events when closing a doc " + i_docCloseMethod,
                new CloseEventType[] { CloseEventType.OnUnload, CloseEventType.NotifyClosing, CloseEventType.Disposing },
                m_observedCloseEvents.toArray( new CloseEventType[0] )
            );
        }
    }

    @Test
    public void testCloseWinEvents() throws Exception
    {
        impl_setupDocCloseTest();
        m_document.getCurrentView().dispatch( ".uno:CloseWin" );
        impl_tearDownDocCloseTest( "via .uno:CloseWin" );
    }

    //@Test
    public void testCloseDocEvents() throws Exception
    {
        impl_setupDocCloseTest();
        m_document.getCurrentView().dispatch( ".uno:CloseDoc" );
        impl_tearDownDocCloseTest( "via .uno:CloseDoc" );
    }

    //@Test
    public void testCloseByAPI() throws Exception
    {
        impl_setupDocCloseTest();
        // closing the doc by API is synchronous, so do this in a separate thread, else we will get a deadlock
        // when the document tries to call back our listener (well, I admit I didn't understand *why* we get this
        // deadlock ... :-\ )
        (new DocCloser()).start();
        impl_tearDownDocCloseTest( "by API" );
    }

    private class DocumentEventListener implements XDocumentEventListener
    {

        public void documentEventOccured( DocumentEvent i_documentEvent )
        {
            if ( i_documentEvent.EventName.equals( "OnUnload" ) )
            {
                synchronized( m_observedCloseEvents )
                {
                    m_observedCloseEvents.add( CloseEventType.OnUnload );
                }
            }
        }

        public void disposing(EventObject eo)
        {
            // not interested in
        }
    };

    private class CloseListener implements XCloseListener
    {

        public void queryClosing(EventObject eo, boolean bln) throws CloseVetoException
        {
            // not interested in
        }

        public void notifyClosing(EventObject eo)
        {
            synchronized( m_observedCloseEvents )
            {
                m_observedCloseEvents.add( CloseEventType.NotifyClosing );
            }
        }

        public void disposing(EventObject eo)
        {
            // not interested in
        }
    };

    private class DocDisposeListener implements XEventListener
    {
        public void disposing(EventObject eo)
        {
            synchronized( m_observedCloseEvents )
            {
                m_observedCloseEvents.add( CloseEventType.Disposing );
            }
            synchronized ( m_document )
            {
                m_document.notifyAll();
            }
        }
    };

    private class DocCloser extends Thread
    {
        @Override
        public void run()
        {
            try
            {
                final XCloseable docCloseable = UnoRuntime.queryInterface(XCloseable.class, m_document.getDocument());
                docCloseable.close(true);
            }
            catch (CloseVetoException ex)
            {
                Logger.getLogger(DocumentEvents.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    };

    private enum CloseEventType
    {
        OnUnload,
        NotifyClosing,
        Disposing
    };

    private OfficeDocument m_document = null;
    final private ArrayList< CloseEventType > m_observedCloseEvents = new ArrayList<DocumentEvents.CloseEventType>();
}
