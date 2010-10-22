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
import com.sun.star.lang.XEventListener;
import java.lang.reflect.Constructor;
import org.openoffice.test.tools.OfficeDocument;
import com.sun.star.document.XUndoManagerSupplier;
import complex.sfx2.undo.CalcDocumentTest;
import com.sun.star.document.XUndoManager;
import com.sun.star.document.XUndoManagerListener;
import com.sun.star.lang.XComponent;
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
            m_wasCleared = true;
        }

        public void redoActionsCleared( EventObject eo )
        {
            m_redoWasCleared = true;
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
        boolean stackWasCleared() { return m_wasCleared; }
        boolean redoStackWasCleared() { return m_redoWasCleared; }

        void reset()
        {
            m_undoActionsAdded = m_undoCount = m_redoCount = 0;
            m_activeUndoContexts.clear();
            m_mostRecentlyAddedAction = m_mostRecentlyUndone = m_mostRecentlyRedone = null;
            // m_isDisposed is not cleared, intentionally
            m_leftContext = m_cancelledContext = m_wasCleared = m_redoWasCleared = false;
        }

        private int     m_undoActionsAdded = 0;
        private int     m_undoCount = 0;
        private int     m_redoCount = 0;
        private boolean m_isDisposed = false;
        private boolean m_leftContext = false;
        private boolean m_cancelledContext = false;
        private boolean m_wasCleared = false;
        private boolean m_redoWasCleared = false;
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

        final UndoListener listener = new UndoListener();
        undoManager.addUndoManagerListener( listener );

        impl_testSingleModification( test, undoManager, listener );
        impl_testMultipleModifications( test, undoManager, listener );
        impl_testCustomUndoActions( test, undoManager, listener );
        impl_testLocking( test, undoManager, listener );
        impl_testNestedContexts( undoManager, listener );
        impl_testErrorHandling( test, undoManager, listener );
        impl_testContextHandling( undoManager, listener );
        impl_testStackHandling( undoManager, listener );
        impl_testClearance( undoManager, listener );

        // close the document, ensure the Undo manager listener gets notified
        m_currentDocument.close();
        assertTrue( "document is closed, but the UndoManagerListener has not been notified of the disposal", listener.isDisposed() );
        m_currentDocument = null;
    }

    private void impl_testSingleModification( final DocumentTest i_test, final XUndoManager i_undoManager, final UndoListener i_listener ) throws com.sun.star.uno.Exception
    {
        i_test.doSingleModification();
        i_test.verifySingleModificationDocumentState();

        // undo the modification, ensure the listener got the proper notifications
        assertEquals( "We did not yet do a undo!", 0, i_listener.getUndoActionCount() );
        i_undoManager.undo();
        assertEquals( "A simple undo does not result in the proper Undo count.",
            1, i_listener.getUndoActionCount() );

        // verify the document is in its initial state, again
        i_test.verifyInitialDocumentState();

        // redo the modification, ensure the listener got the proper notifications
        assertEquals( "did not yet do a redo!", 0, i_listener.getRedoActionCount() );
        i_undoManager.redo();
        assertEquals( "did a redo, but got no notification of it!", 1, i_listener.getRedoActionCount() );
        // ensure the document is in the proper state, again
        i_test.verifySingleModificationDocumentState();

        // now do an Undo via the UI (aka the dispatch API), and see if this works, and notifies the listener as
        // expected
        i_test.getDocument().getCurrentView().dispatch( ".uno:Undo" );
        i_test.verifyInitialDocumentState();
        assertEquals( "UI-Undo does not notify the listener", 2, i_listener.getUndoActionCount() );
    }

    private void impl_testMultipleModifications( final DocumentTest i_test, final XUndoManager i_undoManager, final UndoListener i_listener ) throws com.sun.star.uno.Exception
    {
        i_listener.reset();
        assertEquals( "unexpected initial undo context depth", 0, i_listener.getUndoContextDepth() );
        i_undoManager.enterUndoContext( "Batch Changes" );
        assertEquals( "unexpected undo context depth after entering a context",
            1, i_listener.getUndoContextDepth() );
        assertEquals( "entering an Undo context has not been notified properly",
            "Batch Changes", i_listener.getCurrentUndoContextTitle() );

        final int modifications = i_test.doMultipleModifications();
        assertEquals( "unexpected number of undo actions while doing batch changes to the document",
            modifications, i_listener.getUndoActionsAdded() );
        assertEquals( "seems the document operations touched the undo context depth",
            1, i_listener.getUndoContextDepth() );

        i_undoManager.leaveUndoContext();
        assertEquals( "unexpected undo context depth after leaving the last context",
            0, i_listener.getUndoContextDepth() );
        assertEquals( "no Undo done, yet - still the listener has been notified of an Undo action",
            0, i_listener.getUndoActionCount() );

        i_undoManager.undo();
        assertEquals( "Just did an undo - the listener should have been notified", 1, i_listener.getUndoActionCount() );
        i_test.verifyInitialDocumentState();
    }

    private void impl_testCustomUndoActions( final DocumentTest i_test, final XUndoManager i_undoManager, final UndoListener i_listener ) throws com.sun.star.uno.Exception
    {
        i_undoManager.clear();
        i_listener.reset();
        assertFalse( "undo stack not empty after clearing the undo manager", i_undoManager.isUndoPossible() );
        assertFalse( "redo stack not empty after clearing the undo manager", i_undoManager.isRedoPossible() );
        assertArrayEquals( ">0 descriptions for an empty undo stack?",
            new String[0], i_undoManager.getAllUndoActionTitles() );
        assertArrayEquals( ">0 descriptions for an empty redo stack?",
            new String[0], i_undoManager.getAllRedoActionTitles() );

        // add two actions, one directly, one within a context
        final CustomUndoAction action1 = new CustomUndoAction( "UndoAction1" );
        i_undoManager.addUndoAction( action1 );
        assertEquals( "Adding an undo action not observed by the listener", 1, i_listener.getUndoActionsAdded() );
        assertEquals( "Adding an undo action did not notify the proper title",
            action1.getTitle(), i_listener.getMostRecentlyAddedActionTitle() );
        final String contextTitle = "Undo Context";
        i_undoManager.enterUndoContext( contextTitle );
        final CustomUndoAction action2 = new CustomUndoAction( "UndoAction2" );
        i_undoManager.addUndoAction( action2 );
        assertEquals( "Adding an undo action not observed by the listener",
            2, i_listener.getUndoActionsAdded() );
        assertEquals( "Adding an undo action did not notify the proper title",
            action2.getTitle(), i_listener.getMostRecentlyAddedActionTitle() );
        i_undoManager.leaveUndoContext();

        // see if the manager has proper descriptions
        assertArrayEquals( "unexpected Redo descriptions after adding two actions",
            new String[0], i_undoManager.getAllRedoActionTitles() );
        assertArrayEquals( "unexpected Undo descriptions after adding two actions",
            new String[]{contextTitle, action1.getTitle()}, i_undoManager.getAllUndoActionTitles() );

        // undo one action
        i_undoManager.undo();
        assertEquals( "improper action title notified during programmatic Undo",
            contextTitle, i_listener.getMostRecentlyUndoneTitle() );
        assertTrue( "nested custom undo action has not been undone as expected", action2.undoCalled() );
        assertFalse( "nested custom undo action has not been undone as expected", action1.undoCalled() );
        assertArrayEquals( "unexpected Redo descriptions after undoing a nested custom action",
            new String[]{contextTitle}, i_undoManager.getAllRedoActionTitles() );
        assertArrayEquals( "unexpected Undo descriptions after undoing a nested custom action",
            new String[]{action1.getTitle()}, i_undoManager.getAllUndoActionTitles() );

        // undo the second action, via UI dispatches
        i_test.getDocument().getCurrentView().dispatch( ".uno:Undo" );
        assertEquals( "improper action title notified during UI Undo", action1.getTitle(), i_listener.getMostRecentlyUndoneTitle() );
        assertTrue( "nested custom undo action has not been undone as expected", action1.undoCalled() );
        assertArrayEquals( "unexpected Redo descriptions after undoing the second custom action",
            new String[]{action1.getTitle(), contextTitle}, i_undoManager.getAllRedoActionTitles() );
        assertArrayEquals( "unexpected Undo descriptions after undoing the second custom action",
            new String[0], i_undoManager.getAllUndoActionTitles() );

        // check the actions are disposed when the stacks are cleared
        i_undoManager.clear();
        assertTrue( action1.disposed() && action2.disposed() );

    }

    private void impl_testLocking( final DocumentTest i_test, final XUndoManager i_undoManager, final UndoListener i_listener ) throws com.sun.star.uno.Exception
    {
        i_undoManager.clear();
        i_listener.reset();

        // implicit Undo actions, triggered by changes to the document
        assertFalse( "unexpected initial locking state", i_undoManager.isLocked() );
        i_undoManager.lock();
        assertTrue( "just locked the manager, why does it lie?", i_undoManager.isLocked() );
        i_test.doSingleModification();
        assertEquals( "when the Undo manager is locked, no implicit additions should happen",
            0, i_listener.getUndoActionsAdded() );
        i_undoManager.unlock();
        assertEquals( "unlock is not expected to add collected actions - they should be discarded",
            0, i_listener.getUndoActionsAdded() );
        assertFalse( "just unlocked the manager, why does it lie?", i_undoManager.isLocked() );

        // explicit Undo actions
        i_undoManager.lock();
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.unlock();
        assertEquals( "explicit Undo actions are expected to be ignored when the manager is locked",
            0, i_listener.getUndoActionsAdded() );

        // Undo contexts while being locked
        i_undoManager.lock();
        i_undoManager.enterUndoContext( "Dummy Context" );
        i_undoManager.enterHiddenUndoContext();
        assertEquals( "entering Undo contexts should be ignored when the manager is locked", 0, i_listener.getUndoContextDepth() );
        i_undoManager.leaveUndoContext();
        i_undoManager.leaveUndoContext();
        i_undoManager.unlock();

        // |unlock| error handling
        assertFalse( "internal error: manager should not be locked at this point in time", i_undoManager.isLocked() );
        boolean caughtExpected = false;
        try { i_undoManager.unlock(); } catch ( final InvalidStateException e ) { caughtExpected = true; }
        assertTrue( "unlocking the manager when it is not locked should throw", caughtExpected );
    }

    private void impl_testContextHandling( final XUndoManager i_undoManager, final UndoListener i_listener ) throws com.sun.star.uno.Exception
    {
        i_undoManager.clear();
        i_listener.reset();

        i_undoManager.enterUndoContext( "Undo Context" );
        assertEquals( "unexpected undo context depth after entering a context", 1, i_listener.getUndoContextDepth() );
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.leaveUndoContext();
        assertTrue( "leaving a non-empty context does not call leftUndoContext", i_listener.contextLeft() );
        assertFalse( "leaving a non-empty context should not call cancelledUndoContext", i_listener.contextCancelled() );
        assertEquals( "unexpected undo context depth leaving a non-empty context", 0, i_listener.getUndoContextDepth() );

        i_undoManager.clear();
        i_listener.reset();

        i_undoManager.enterUndoContext( "Undo Context" );
        i_undoManager.leaveUndoContext();
        assertFalse( "leaving an empty context should not call leftUndoContext", i_listener.contextLeft() );
        assertTrue( "leaving an empty context does not call cancelledUndoContext", i_listener.contextCancelled() );
        assertFalse( "leaving an empty context should silently remove it, and not contribute to the stack",
            i_undoManager.isUndoPossible() );
    }

    private void impl_testNestedContexts( final XUndoManager i_undoManager, final UndoListener i_listener ) throws com.sun.star.uno.Exception
    {
        i_undoManager.clear();
        i_listener.reset();
        i_undoManager.enterUndoContext( "context 1" );
        i_undoManager.enterUndoContext( "context 1.1" );
        final CustomUndoAction action1 = new CustomUndoAction( "action 1.1.1" );
        i_undoManager.addUndoAction( action1 );
        i_undoManager.enterUndoContext( "context 1.1.2" );
        final CustomUndoAction action2 = new CustomUndoAction( "action 1.1.2.1" );
        i_undoManager.addUndoAction( action2 );
        i_undoManager.leaveUndoContext();
        final CustomUndoAction action3 = new CustomUndoAction( "action 1.1.3" );
        i_undoManager.addUndoAction( action3 );
        i_undoManager.leaveUndoContext();
        i_undoManager.leaveUndoContext();
        final CustomUndoAction action4 = new CustomUndoAction( "action 1.2" );
        i_undoManager.addUndoAction( action4 );

        i_undoManager.undo();
        assertEquals( "undoing a single action notifies a wrong title", action4.getTitle(), i_listener.getMostRecentlyUndoneTitle() );
        assertTrue( "custom Undo not called", action4.undoCalled() );
        assertFalse( "too many custom Undos called", action1.undoCalled() || action2.undoCalled() || action3.undoCalled() );
        i_undoManager.undo();
        assertTrue( "nested actions not properly undone", action1.undoCalled() && action2.undoCalled() && action3.undoCalled() );
    }

    private void impl_testErrorHandling( final DocumentTest i_test, final XUndoManager i_undoManager, final UndoListener i_listener ) throws com.sun.star.uno.Exception
    {
        i_undoManager.clear();
        i_listener.reset();

        // try retrieving the comments for the current Undo/Redo - this should fail
        boolean caughtExpected = false;
        try { i_undoManager.getCurrentUndoActionTitle(); }
        catch( final InvalidStateException e ) { caughtExpected = true; }
        assertTrue( "trying the title of the current Undo action is expected to fail for an empty stack", caughtExpected );

        caughtExpected = false;
        try { i_undoManager.getCurrentRedoActionTitle(); }
        catch( final InvalidStateException e ) { caughtExpected = true; }
        assertTrue( "trying the title of the current Redo action is expected to fail for an empty stack", caughtExpected );

        caughtExpected = false;
        try { i_undoManager.undo(); } catch ( final InvalidStateException e ) { caughtExpected = true; }
        assertTrue( "undo should throw if no Undo action is on the stack", caughtExpected );

        caughtExpected = false;
        try { i_undoManager.redo(); } catch ( final InvalidStateException e ) { caughtExpected = true; }
        assertTrue( "redo should throw if no Redo action is on the stack", caughtExpected );

        caughtExpected = false;
        try { i_undoManager.leaveUndoContext(); } catch ( final InvalidStateException e ) { caughtExpected = true; }
        assertTrue( "leaveUndoContext should throw if no context is currently open", caughtExpected );

        caughtExpected = false;
        try { i_undoManager.addUndoAction( null ); } catch ( com.sun.star.lang.IllegalArgumentException e ) { caughtExpected = true; }
        assertTrue( "adding a NULL action should be rejected", caughtExpected );

        // an Undo action which throws when undone should propagate this to the API caller
        i_undoManager.clear();
        i_undoManager.addUndoAction( new FailingUndoAction() );
        caughtExpected = false;
        try { i_undoManager.undo(); } catch ( com.sun.star.uno.RuntimeException e ) { caughtExpected = true; }
        assertTrue( "RuntimeExceptions in XUndoAction.undo should be propagated at the API", caughtExpected );
        assertTrue( "an Undo action which fails should not be removed from the stack", i_undoManager.isUndoPossible() );

        // but on the other hand, doing the Undo via UI should not throw
        i_test.getDocument().getCurrentView().dispatch( ".uno:Undo" );
    }

    private void impl_testStackHandling( final XUndoManager i_undoManager, final UndoListener i_listener ) throws com.sun.star.uno.Exception
    {
        i_undoManager.clear();
        i_listener.reset();

        assertFalse( i_undoManager.isUndoPossible() );    // just for completeness, those two
        assertFalse( i_undoManager.isRedoPossible() );    // have been checked before already ...
        i_undoManager.addUndoAction( new CustomUndoAction() );
        assertTrue( i_undoManager.isUndoPossible() );
        assertFalse( i_undoManager.isRedoPossible() );
        i_undoManager.addUndoAction( new CustomUndoAction() );
        assertTrue( i_undoManager.isUndoPossible() );
        assertFalse( i_undoManager.isRedoPossible() );
        i_undoManager.undo();
        assertTrue( i_undoManager.isUndoPossible() );
        assertTrue( i_undoManager.isRedoPossible() );
        i_undoManager.undo();
        assertFalse( i_undoManager.isUndoPossible() );
        assertTrue( i_undoManager.isRedoPossible() );
        i_undoManager.addUndoAction( new CustomUndoAction() );
        assertTrue( i_undoManager.isUndoPossible() );
        assertFalse( "adding a new action should have cleared the Redo stack", i_undoManager.isRedoPossible() );
    }

    private void impl_testClearance( final XUndoManager i_undoManager, final UndoListener i_listener ) throws com.sun.star.uno.Exception
    {
        i_undoManager.clear();
        i_listener.reset();

        // add an action, clear the stack, verify the listener has been called
        i_undoManager.addUndoAction( new CustomUndoAction() );
        assertFalse( "clearance listener unexpectedly called", i_listener.stackWasCleared() );
        assertFalse( "redo-clearance listener unexpectedly called", i_listener.redoStackWasCleared() );
        i_undoManager.clear();
        assertTrue( "clearance listener not called as expected", i_listener.stackWasCleared() );
        assertFalse( "redo-clearance listener unexpectedly called (2)", i_listener.redoStackWasCleared() );

        // ensure the listener is also called if the stack is actually empty at the moment of the call
        i_listener.reset();
        assertFalse( i_undoManager.isUndoPossible() );
        i_undoManager.clear();
        assertTrue( "clearance listener is also expected to be called if the stack was empty before", i_listener.stackWasCleared() );

        // ensure the proper listeners are called for clearRedo
        i_listener.reset();
        i_undoManager.clearRedo();
        assertFalse( i_listener.stackWasCleared() );
        assertTrue( i_listener.redoStackWasCleared() );

        // ensure the redo listener is also called upon implicit redo stack clearance
        i_listener.reset();
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.undo();
        assertTrue( i_undoManager.isUndoPossible() );
        assertTrue( i_undoManager.isRedoPossible() );
        i_undoManager.addUndoAction( new CustomUndoAction() );
        assertFalse( i_undoManager.isRedoPossible() );
        assertTrue( "implicit clearance of the Redo stack does not notify listeners", i_listener.redoStackWasCleared() );
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

    private static class CustomUndoAction implements XUndoAction, XComponent
    {
        CustomUndoAction()
        {
            m_title = "Custom Undo Action";
        }

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

        public void dispose()
        {
            m_disposed = true;
        }

        public void addEventListener( XEventListener xl )
        {
            fail( "addEventListener is not expected to be called in the course of this test" );
        }

        public void removeEventListener( XEventListener xl )
        {
            fail( "removeEventListener is not expected to be called in the course of this test" );
        }

        boolean undoCalled() { return m_undoCalled; }
        boolean redoCalled() { return m_redoCalled; }
        boolean disposed() { return m_disposed; }

        private final String    m_title;
        private boolean         m_undoCalled = false;
        private boolean         m_redoCalled = false;
        private boolean         m_disposed = false;
    }

    private static class FailingUndoAction implements XUndoAction
    {
        FailingUndoAction()
        {
        }

        public String getTitle()
        {
            return "failing undo";
        }

        public void undo()
        {
            impl_throw();
        }

        public void redo()
        {
            impl_throw();
        }

        private void impl_throw()
        {
            throw new com.sun.star.uno.RuntimeException();
        }
    }

    private static final OfficeConnection m_connection = new OfficeConnection();
    private OfficeDocument m_currentDocument;
}
