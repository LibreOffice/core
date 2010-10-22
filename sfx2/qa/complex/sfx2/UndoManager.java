/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package complex.sfx2;

import com.sun.star.document.UndoManagerEvent;
import com.sun.star.document.XUndoAction;
import com.sun.star.lang.EventObject;
import java.lang.reflect.Constructor;
import org.openoffice.test.tools.OfficeDocument;
import com.sun.star.document.XUndoManagerSupplier;
import complex.sfx2.undo.CalcDocumentTest;
import com.sun.star.document.XUndoManager;
import com.sun.star.document.XUndoManagerListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.InvalidStateException;
import complex.sfx2.undo.DocumentTest;
import complex.sfx2.undo.DrawDocumentTest;
import complex.sfx2.undo.ImpressDocumentTest;
import complex.sfx2.undo.WriterDocumentTest;
import java.util.Stack;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;
import org.openoffice.test.OfficeConnection;

/**
 * Unit test for the UndoManager API
 *
 * @author frank.schoenheit@oracle.com
 */
public class UndoManager
{
    @Before
    public void beforeTest()
    {
        m_currentDocument = null;
    }

    @Test
    public void checkWriterUndo() throws Exception
    {
        impl_checkUndo( WriterDocumentTest.class, true );
    }

    @Test
    public void checkCalcUndo() throws Exception
    {
        impl_checkUndo( CalcDocumentTest.class, false );
    }

    @Test
    public void checkDrawUndo() throws Exception
    {
        impl_checkUndo( DrawDocumentTest.class, false );
    }

    @Test
    public void checkImpressUndo() throws Exception
    {
        impl_checkUndo( ImpressDocumentTest.class, false );
    }

    @After
    public void afterTest()
    {
        if ( m_currentDocument != null )
            m_currentDocument.close();
    }

    /**
     * returns the undo manager belonging to a given document
     * @return
     */
    private XUndoManager getUndoManager( final OfficeDocument i_document )
    {
        XUndoManagerSupplier suppUndo = UnoRuntime.queryInterface( XUndoManagerSupplier.class, i_document.getDocument() );
        return suppUndo.getUndoManager();
    }

    private static class UndoListener implements XUndoManagerListener
    {
        public void undoActionAdded( UndoManagerEvent i_event )
        {
            ++m_undoActionsAdded;
            m_mostRecentlyAddedAction = i_event.UndoActionTitle;
        }

        public void actionUndone( UndoManagerEvent i_event )
        {
            ++m_undoCount;
            m_mostRecentlyUndone = i_event.UndoActionTitle;
        }

        public void actionRedone( UndoManagerEvent i_event )
        {
            ++m_redoCount;
            m_mostRecentlyRedone = i_event.UndoActionTitle;
        }

        public void allActionsCleared( EventObject eo )
        {
        }

        public void redoActionsCleared( EventObject eo )
        {
        }

        public void enteredUndoContext( UndoManagerEvent i_event )
        {
            m_activeUndoContexts.push( i_event.UndoActionTitle );
            assertEquals( "different opinions on the context nesting level (after entering)",
                m_activeUndoContexts.size(), i_event.UndoContextDepth );
        }

        public void enteredHiddenUndoContext( UndoManagerEvent i_event )
        {
            m_activeUndoContexts.push( i_event.UndoActionTitle );
            assertEquals( "different opinions on the context nesting level (after entering hidden)",
                m_activeUndoContexts.size(), i_event.UndoContextDepth );
        }

        public void leftUndoContext( UndoManagerEvent i_event )
        {
            assertEquals( "nested undo context descriptions do not match", m_activeUndoContexts.pop(), i_event.UndoActionTitle );
            assertEquals( "different opinions on the context nesting level (after leaving)",
                m_activeUndoContexts.size(), i_event.UndoContextDepth );
            m_leftContext = true;
        }

        public void cancelledUndoContext( UndoManagerEvent i_event )
        {
            m_activeUndoContexts.pop();
            assertEquals( "different opinions on the context nesting level (after cancelling)",
                m_activeUndoContexts.size(), i_event.UndoContextDepth );
            m_cancelledContext = true;
        }

        public void disposing( EventObject i_event )
        {
            m_isDisposed = true;
        }

        int     getUndoActionsAdded() { return m_undoActionsAdded; }
        int     getUndoActionCount() { return m_undoCount; }
        int     getRedoActionCount() { return m_redoCount; }
        String  getCurrentUndoContextTitle() { return m_activeUndoContexts.peek(); }
        String  getMostRecentlyAddedActionTitle() { return m_mostRecentlyAddedAction; };
        String  getMostRecentlyUndoneTitle() { return m_mostRecentlyUndone; }
        String  getMostRecentlyRedoneTitle() { return m_mostRecentlyRedone; }
        int     getUndoContextDepth() { return m_activeUndoContexts.size(); }
        boolean isDisposed() { return m_isDisposed; }
        boolean contextLeft() { return m_leftContext; }
        boolean contextCancelled() { return m_cancelledContext; }

        void reset()
        {
            m_undoActionsAdded = m_undoCount = m_redoCount = 0;
            m_activeUndoContexts.clear();
            m_mostRecentlyAddedAction = m_mostRecentlyUndone = m_mostRecentlyRedone = null;
            // m_isDisposed is not cleared, intentionally
            m_leftContext = m_cancelledContext = false;
        }

        private int     m_undoActionsAdded = 0;
        private int     m_undoCount = 0;
        private int     m_redoCount = 0;
        private boolean m_isDisposed = false;
        private boolean m_leftContext = false;
        private boolean m_cancelledContext = false;
        private Stack< String >
                        m_activeUndoContexts = new Stack<String>();
        private String  m_mostRecentlyAddedAction = null;
        private String  m_mostRecentlyUndone = null;
        private String  m_mostRecentlyRedone = null;
    };

    private void impl_checkUndo( final Class i_testClass, final boolean i_fakeTestForNow ) throws Exception
    {
        final Constructor ctor = i_testClass.getConstructor( XMultiServiceFactory.class );
        final DocumentTest test = (DocumentTest)ctor.newInstance( getORB() );
        System.out.println( "testing: " + test.getDocumentDescription() );
        m_currentDocument = test.getDocument();
        test.initializeDocument();
        test.verifyInitialDocumentState();

        if ( i_fakeTestForNow )
        {
            // Writer does not yet have an UndoManager in the current phase of the implementation. Once it has, we
            // this complete branch, which barely tests anything (except perhaps the DocumentTest implementation),
            // can vanish.
            test.doSingleModification();
            test.verifySingleModificationDocumentState();
            test.getDocument().getCurrentView().dispatch( ".uno:Undo" );
            test.verifyInitialDocumentState();
            final int expectedUndoSteps = test.doMultipleModifications();
            for ( int i=0; i<expectedUndoSteps; ++i )
                test.getDocument().getCurrentView().dispatch( ".uno:Undo" );
            test.verifyInitialDocumentState();
            test.getDocument().close();
            return;
        }

        final XUndoManager undoManager = getUndoManager( test.getDocument() );
        undoManager.clear();
        assertFalse( "clearing the Undo manager should result in the impossibility to undo anything", undoManager.isUndoPossible() );
        assertFalse( "clearing the Undo manager should result in the impossibility to redo anything", undoManager.isRedoPossible() );

        // try retrieving the comments for the current Undo/Redo - this should fail
        boolean caughtExpected = false;
        try { undoManager.getCurrentUndoActionTitle(); undoManager.getCurrentRedoActionTitle(); }
        catch( final InvalidStateException e ) { caughtExpected = true; }
        assertTrue( "trying the title of the current Undo/Redo action is expected to fail for an empty stack", caughtExpected );

        final UndoListener listener = new UndoListener();
        undoManager.addUndoManagerListener( listener );

        // do a single modification to the document
        {
            test.doSingleModification();
            test.verifySingleModificationDocumentState();

            // undo the modification, ensure the listener got the proper notifications
            assertEquals( "We did not yet do a undo!", 0, listener.getUndoActionCount() );
            undoManager.undo();
            assertEquals( "A simple undo does not result in the proper Undo count.", 1, listener.getUndoActionCount() );

            // verify the document is in its initial state, again
            test.verifyInitialDocumentState();

            // redo the modification, ensure the listener got the proper notifications
            assertEquals( "did not yet do a redo!", 0, listener.getRedoActionCount() );
            undoManager.redo();
            assertEquals( "did a redo, but got no notification of it!", 1, listener.getRedoActionCount() );

            // ensure the document is in the proper state, again
            test.verifySingleModificationDocumentState();

            // now do an Undo via the UI (aka the dispatch API), and see if this works, and notifies the listener as
            // expected
            test.getDocument().getCurrentView().dispatch( ".uno:Undo" );
            test.verifyInitialDocumentState();
            assertEquals( "UI-Undo does not notify the listener", 2, listener.getUndoActionCount() );
        }

        // do multiple changes in a row, after entering an Undo context
        {
            listener.reset();

            assertEquals( "unexpected initial undo context depth", 0, listener.getUndoContextDepth() );
            undoManager.enterUndoContext( "Batch Changes" );
            assertEquals( "unexpected undo context depth after entering a context", 1, listener.getUndoContextDepth() );

            assertEquals( "entering an Undo context has not been notified properly", "Batch Changes", listener.getCurrentUndoContextTitle() );
            final int modifications = test.doMultipleModifications();
            assertEquals( "unexpected number of undo actions while doing batch changes to the document",
                modifications, listener.getUndoActionsAdded() );

            assertEquals( "seems the document operations touched the undo context depth", 1, listener.getUndoContextDepth() );
            undoManager.leaveUndoContext();
            assertEquals( "unexpected undo context depth after leaving the last context", 0, listener.getUndoContextDepth() );

            assertEquals( "no Undo done, yet - still the listener has been notified of an Undo action", 0, listener.getUndoActionCount() );
            undoManager.undo();
            assertEquals( "Just did an undo - the listener should have been notified", 1, listener.getUndoActionCount() );

            test.verifyInitialDocumentState();
        }

        // custom Undo actions
        {
            undoManager.clear();
            listener.reset();
            assertFalse( "undo stack not empty after clearing the undo manager", undoManager.isUndoPossible() );
            assertFalse( "redo stack not empty after clearing the undo manager", undoManager.isRedoPossible() );
            assertArrayEquals( ">0 descriptions for an empty undo stack?", new String[0], undoManager.getAllUndoActionTitles() );
            assertArrayEquals( ">0 descriptions for an empty redo stack?", new String[0], undoManager.getAllRedoActionTitles() );

            // add two actions, one directly, one within a context
            final CustomUndoAction action1 = new CustomUndoAction( "UndoAction1" );
            undoManager.addUndoAction( action1 );
            assertEquals( "Adding an undo action not observed by the listener", 1, listener.getUndoActionsAdded() );
            assertEquals( "Adding an undo action did not notify the proper title", action1.getTitle(), listener.getMostRecentlyAddedActionTitle() );

            final String contextTitle = "Undo Context";
            undoManager.enterUndoContext( contextTitle );
            final CustomUndoAction action2 = new CustomUndoAction( "UndoAction2" );
            undoManager.addUndoAction( action2 );
            assertEquals( "Adding an undo action not observed by the listener", 2, listener.getUndoActionsAdded() );
            assertEquals( "Adding an undo action did not notify the proper title", action2.getTitle(), listener.getMostRecentlyAddedActionTitle() );
            undoManager.leaveUndoContext();

            // see if the manager has proper descriptions
            assertArrayEquals( "unexpected Redo descriptions after adding two actions",
                new String[0], undoManager.getAllRedoActionTitles() );
            assertArrayEquals( "unexpected Undo descriptions after adding two actions",
                new String[] { contextTitle, action1.getTitle() }, undoManager.getAllUndoActionTitles() );

            // undo one action
            undoManager.undo();
            assertEquals( "improper action title notified during programmatic Undo", contextTitle, listener.getMostRecentlyUndoneTitle() );
            assertTrue( "nested custom undo action has not been undone as expected", action2.undoCalled() );
            assertFalse( "nested custom undo action has not been undone as expected", action1.undoCalled() );
            assertArrayEquals( "unexpected Redo descriptions after undoing a nested custom action",
                new String[] { contextTitle }, undoManager.getAllRedoActionTitles() );
            assertArrayEquals( "unexpected Undo descriptions after undoing a nested custom action",
                new String[] { action1.getTitle() }, undoManager.getAllUndoActionTitles() );

            // undo the second action, via UI dispatches
            test.getDocument().getCurrentView().dispatch( ".uno:Undo" );
            assertEquals( "improper action title notified during UI Undo", action1.getTitle(), listener.getMostRecentlyUndoneTitle() );
            assertTrue( "nested custom undo action has not been undone as expected", action1.undoCalled() );
            assertArrayEquals( "unexpected Redo descriptions after undoing the second custom action",
                new String[] { action1.getTitle(), contextTitle }, undoManager.getAllRedoActionTitles() );
            assertArrayEquals( "unexpected Undo descriptions after undoing the second custom action",
                new String[0], undoManager.getAllUndoActionTitles() );
        }

        // nesting of contexts
        {
            undoManager.clear();
            listener.reset();
            undoManager.enterUndoContext( "context 1" );
            undoManager.enterUndoContext( "context 1.1" );
            final CustomUndoAction action1 = new CustomUndoAction( "action 1.1.1" );
            undoManager.addUndoAction( action1 );
            undoManager.enterUndoContext( "context 1.1.2" );
            final CustomUndoAction action2 = new CustomUndoAction( "action 1.1.2.1" );
            undoManager.addUndoAction( action2 );
            undoManager.leaveUndoContext();
            final CustomUndoAction action3 = new CustomUndoAction( "action 1.1.3" );
            undoManager.addUndoAction( action3 );
            undoManager.leaveUndoContext();
            undoManager.leaveUndoContext();
            final CustomUndoAction action4 = new CustomUndoAction( "action 1.2" );
            undoManager.addUndoAction( action4 );

            undoManager.undo();
            assertEquals( "undoing a single action notifies a wrong title", action4.getTitle(), listener.getMostRecentlyUndoneTitle() );
            assertTrue( "custom Undo not called", action4.undoCalled() );
            assertFalse( "too many custom Undos called", action1.undoCalled() || action2.undoCalled() || action3.undoCalled() );
            undoManager.undo();
            assertTrue( "nested actions not properly undone", action1.undoCalled() && action2.undoCalled() && action3.undoCalled() );
        }

        // some error handlings
        {
            undoManager.clear();
            listener.reset();
            caughtExpected = false;
            try { undoManager.undo(); } catch ( final InvalidStateException e ) { caughtExpected = true; }
            assertTrue( "undo should throw if no Undo action is on the stack", caughtExpected );
            caughtExpected = false;
            try { undoManager.redo(); } catch ( final InvalidStateException e ) { caughtExpected = true; }
            assertTrue( "redo should throw if no Redo action is on the stack", caughtExpected );
            caughtExpected = false;
            try { undoManager.leaveUndoContext(); } catch ( final InvalidStateException e ) { caughtExpected = true; }
            assertTrue( "leaveUndoContext should throw if no context is currently open", caughtExpected );
        }

        // more on undo contexts
        {
            undoManager.clear();
            listener.reset();

            undoManager.enterUndoContext( "Undo Context" );
            assertEquals( "unexpected undo context depth after entering a context", 1, listener.getUndoContextDepth() );
            undoManager.addUndoAction( new CustomUndoAction( "Undo Action" ) );
            undoManager.leaveUndoContext();
            assertTrue( "leaving a non-empty context does not call leftUndoContext", listener.contextLeft() );
            assertFalse( "leaving a non-empty context should not call cancelledUndoContext", listener.contextCancelled() );
            assertEquals( "unexpected undo context depth leaving a non-empty context", 0, listener.getUndoContextDepth() );

            undoManager.clear();
            listener.reset();

            undoManager.enterUndoContext( "Undo Context" );
            undoManager.leaveUndoContext();
            assertFalse( "leaving an empty context should not call leftUndoContext", listener.contextLeft() );
            assertTrue( "leaving an empty context does not call cancelledUndoContext", listener.contextCancelled() );
            assertFalse( "leaving an empty context should silently remove it, and not contribute to the stack",
                undoManager.isUndoPossible() );
        }

        // stack handling
        {
            undoManager.clear();
            listener.reset();

            assertFalse( undoManager.isUndoPossible() );    // just for completeness, those two
            assertFalse( undoManager.isRedoPossible() );    // have been checked above already ...
            undoManager.addUndoAction( new CustomUndoAction( "Undo Action 1" ) );
            assertTrue( undoManager.isUndoPossible() );
            assertFalse( undoManager.isRedoPossible() );
            undoManager.addUndoAction( new CustomUndoAction( "Undo Action 2" ) );
            assertTrue( undoManager.isUndoPossible() );
            assertFalse( undoManager.isRedoPossible() );
            undoManager.undo();
            assertTrue( undoManager.isUndoPossible() );
            assertTrue( undoManager.isRedoPossible() );
            undoManager.undo();
            assertFalse( undoManager.isUndoPossible() );
            assertTrue( undoManager.isRedoPossible() );
            undoManager.addUndoAction( new CustomUndoAction( "Undo Action 3" ) );
            assertTrue( undoManager.isUndoPossible() );
            assertFalse( "adding a new action should have cleared the Redo stack", undoManager.isRedoPossible() );
        }

        // locking the manager
        {
            undoManager.clear();
            listener.reset();

            // implicit Undo actions, triggered by changes to the document
            assertFalse( "unexpected initial locking state", undoManager.isLocked() );
            undoManager.lock();
            assertTrue( "just locked the manager, why does it lie?", undoManager.isLocked() );
            test.doSingleModification();
            assertEquals( "when the Undo manager is locked, no implicit additions should happen",
                0, listener.getUndoActionsAdded() );
            undoManager.unlock();
            assertEquals( "unlock is not expected to add collected actions - they should be discarded",
                0, listener.getUndoActionsAdded() );
            assertFalse( "just unlocked the manager, why does it lie?", undoManager.isLocked() );

            // explicit Undo actions
            undoManager.lock();
            undoManager.addUndoAction( new CustomUndoAction( "Dummy Undo Action" ) );
            undoManager.unlock();
            assertEquals( "explicit Undo actions are expected to be ignored when the manager is locked",
                0, listener.getUndoActionsAdded() );

            // Undo contexts while being locked
            undoManager.lock();
            undoManager.enterUndoContext( "Dummy Context" );
            undoManager.enterHiddenUndoContext();
            assertEquals( "entering Undo contexts should be ignored when the manager is locked", 0, listener.getUndoContextDepth() );
            undoManager.leaveUndoContext();
            undoManager.leaveUndoContext();
            undoManager.unlock();

            // |unlock| error handling
            assertFalse( "internal error: manager should not be locked at this point in time", undoManager.isLocked() );
            caughtExpected = false;
            try { undoManager.unlock(); } catch ( final InvalidStateException e ) { caughtExpected = true; }
            assertTrue( "unlocking the manager when it is not locked should throw", caughtExpected );
        }

        // close the document, ensure the Undo manager listener gets notified
        m_currentDocument.close();
        assertTrue( "document is closed, but the UndoManagerListener has not been notified of the disposal", listener.isDisposed() );
        m_currentDocument = null;
    }

    private XComponentContext getContext()
    {
        return m_connection.getComponentContext();
    }

    private XMultiServiceFactory getORB()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(
            XMultiServiceFactory.class, getContext().getServiceManager() );
        return xMSF1;
    }

    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println( "--------------------------------------------------------------------------------" );
        System.out.println( "starting class: " + UndoManager.class.getName() );
        System.out.println( "connecting ..." );
        m_connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection() throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println();
        System.out.println( "tearing down connection" );
        m_connection.tearDown();
        System.out.println( "finished class: " + UndoManager.class.getName() );
        System.out.println( "--------------------------------------------------------------------------------" );
    }

    private static class CustomUndoAction implements XUndoAction
    {
        CustomUndoAction( final String i_title )
        {
            m_title = i_title;
        }

        public String getTitle()
        {
            return m_title;
        }

        public void undo()
        {
            m_undoCalled = true;
        }

        public void redo()
        {
            m_redoCalled = true;
        }

        boolean undoCalled() { return m_undoCalled; }
        boolean redoCalled() { return m_redoCalled; }

        private final String    m_title;
        private boolean         m_undoCalled = false;
        private boolean         m_redoCalled = false;
    }

    private static final OfficeConnection m_connection = new OfficeConnection();
    private OfficeDocument m_currentDocument;
}
