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

package complex.sfx2;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XChild;
import com.sun.star.container.XIndexContainer;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNameReplace;
import com.sun.star.container.XSet;
import com.sun.star.document.EmptyUndoStackException;
import com.sun.star.document.UndoContextNotClosedException;
import com.sun.star.document.UndoFailedException;
import com.sun.star.document.UndoManagerEvent;
import com.sun.star.document.XEmbeddedScripts;
import com.sun.star.document.XEventsSupplier;
import com.sun.star.document.XUndoAction;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.XEventListener;
import java.lang.reflect.InvocationTargetException;
import org.openoffice.test.tools.OfficeDocument;
import com.sun.star.document.XUndoManagerSupplier;
import com.sun.star.document.XUndoManager;
import com.sun.star.document.XUndoManagerListener;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.script.ScriptEventDescriptor;
import com.sun.star.script.XEventAttacherManager;
import com.sun.star.script.XLibraryContainer;
import com.sun.star.task.XJob;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.InvalidStateException;
import com.sun.star.util.NotLockedException;
import com.sun.star.view.XControlAccess;
import complex.sfx2.undo.CalcDocumentTest;
import complex.sfx2.undo.ChartDocumentTest;
import complex.sfx2.undo.DocumentTest;
import complex.sfx2.undo.DrawDocumentTest;
import complex.sfx2.undo.ImpressDocumentTest;
import complex.sfx2.undo.WriterDocumentTest;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Stack;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;
import org.openoffice.test.OfficeConnection;
import org.openoffice.test.tools.DocumentType;
import org.openoffice.test.tools.SpreadsheetDocument;

/**
 * Unit test for the UndoManager API
 *
 * @author frank.schoenheit@oracle.com
 */
public class UndoManager
{
    // -----------------------------------------------------------------------------------------------------------------
    @Before
    public void beforeTest() throws com.sun.star.uno.Exception
    {
        m_currentTestCase = null;
        m_currentDocument = null;
        m_undoListener = null;

        // at our service factory, insert a new factory for our CallbackComponent
        // this will allow the Basic code in our test documents to call back into this test case
        // here, by just instantiating this service
        final XSet globalFactory = UnoRuntime.queryInterface( XSet.class, getORB() );
        m_callbackFactory = new CallbackComponentFactory();
        globalFactory.insert( m_callbackFactory );
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void checkWriterUndo() throws Exception
    {
        m_currentTestCase = new WriterDocumentTest( getORB() );
        impl_checkUndo();
    }

    // -----------------------------------------------------------------------------------------------------------------
//FIXME fails fdo#35663   @Test
    public void checkCalcUndo() throws Exception
    {
        m_currentTestCase = new CalcDocumentTest( getORB() );
        impl_checkUndo();
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void checkDrawUndo() throws Exception
    {
        m_currentTestCase = new DrawDocumentTest( getORB() );
        impl_checkUndo();
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void checkImpressUndo() throws Exception
    {
        m_currentTestCase = new ImpressDocumentTest( getORB() );
        impl_checkUndo();
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void checkChartUndo() throws Exception
    {
        m_currentTestCase = new ChartDocumentTest( getORB() );
        impl_checkUndo();
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void checkBrokenScripts() throws com.sun.star.uno.Exception, InterruptedException
    {
        System.out.println( "testing: broken scripts" );

        m_currentDocument = OfficeDocument.blankDocument( getORB(), DocumentType.CALC );
        m_undoListener = new UndoListener();
        getUndoManager().addUndoManagerListener( m_undoListener );

        impl_setupBrokenBasicScript();
        final String scriptURI = "vnd.sun.star.script:default.callbacks.brokenScript?language=Basic&location=document";

        // .............................................................................................................
        // scenario 1: Pressing a button which is bound to execute the script
        // (This is one of the many cases where SfxObjectShell::CallXScript is invoked)

        // set up the button
        final XPropertySet buttonModel = impl_setupButton();
        buttonModel.setPropertyValue( "Label", "exec broken script" );
        impl_assignScript( buttonModel, "XActionListener", "actionPerformed",
            scriptURI );

        // switch the doc's view to form alive mode (so the button will actually work)
        m_currentDocument.getCurrentView().dispatch( ".uno:SwitchControlDesignMode" );

        // click the button
        m_callbackCalled = false;
        impl_clickButton( buttonModel );
        // the macro is executed asynchronously by the button, so wait at most 2 seconds for the callback to be
        // triggered
        impl_waitFor( m_callbackCondition, 2000 );
        // check the callback has actually been called
        assertTrue( "clicking the test button did not work as expected - basic script not called", m_callbackCalled );

        // again, since the script is executed asynchronously, we might arrive here while its execution
        // is not completely finished. Give OOo another (at most) 2 seconds to finish it.
        m_undoListener.waitForAllContextsClosed( 20000 );
        // assure that the Undo Context Depth of the doc is still "0": The Basic script entered such a
        // context, and didn't close it (thus it is broken), but the application framework should have
        // auto-closed the context after the macro finished.
        assertEquals( "undo context was not auto-closed as expected", 0, m_undoListener.getCurrentUndoContextDepth() );

        // .............................................................................................................
        // scenario 2: dispatching the script URL. Technically, this is equivalent to configuring the
        // script into a menu or toolbar, and selecting the respective menu/toolbar item
        m_callbackCalled = false;
        m_currentDocument.getCurrentView().dispatch( scriptURI );
        assertTrue( "dispatching the Script URL did not work as expected - basic script not called", m_callbackCalled );
        // same as above: The script didn't close the context, but the OOo framework should have
        assertEquals( "undo context was not auto-closed as expected", 0, m_undoListener.getCurrentUndoContextDepth() );

        // .............................................................................................................
        // scenario 3: assigning the script to some document event, and triggering this event
        final XEventsSupplier eventSupplier = UnoRuntime.queryInterface( XEventsSupplier.class, m_currentDocument.getDocument() );
        final XNameReplace events = UnoRuntime.queryInterface( XNameReplace.class, eventSupplier.getEvents() );
        final NamedValue[] scriptDescriptor = new NamedValue[] {
            new NamedValue( "EventType", "Script" ),
            new NamedValue( "Script", scriptURI )
        };
        events.replaceByName( "OnViewCreated", scriptDescriptor );

        // The below doesn't work: event notification is broken in m96, see http://www.openoffice.org/issues/show_bug.cgi?id=116313
        m_callbackCalled = false;
        m_currentDocument.getCurrentView().dispatch( ".uno:NewWindow" );
        assertTrue( "triggering an event did not work as expected - basic script not called", m_callbackCalled );
        // same as above: The script didn't close the context, but the OOo framework should have
        assertEquals( "undo context was not auto-closed as expected", 0, m_undoListener.getCurrentUndoContextDepth() );

        // .............................................................................................................
        // scenario 4: let the script enter an Undo context, but not close it, as usual.
        // Additionally, let the script close the document - the OOo framework code which cares for
        // auto-closing of Undo contexts should survive this, ideally ...
        m_closeAfterCallback = true;
        m_callbackCalled = false;
        m_currentDocument.getCurrentView().dispatch( scriptURI );
        assertTrue( m_callbackCalled );
        assertTrue( "The Basic script should have closed the document.", m_undoListener.isDisposed() );
        m_currentDocument = null;
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void checkSerialization() throws com.sun.star.uno.Exception, InterruptedException
    {
        System.out.println( "testing: request serialization" );

        m_currentDocument = OfficeDocument.blankDocument( getORB(), DocumentType.CALC );
        final XUndoManager undoManager = getUndoManager();

        final int threadCount = 10;
        final int actionsPerThread = 10;
        final int actionCount = threadCount * actionsPerThread;

        // add some actions to the UndoManager, each knowing its position on the stack
        final Object lock = new Object();
        final Integer actionsUndone[] = new Integer[] { 0 };
        for ( int i=actionCount; i>0; )
            undoManager.addUndoAction( new CountingUndoAction( --i, lock, actionsUndone ) );

        // some concurrent threads which undo the actions
        Thread[] threads = new Thread[threadCount];
        for ( int i=0; i<threadCount; ++i )
        {
            threads[i] = new Thread()
            {
                @Override
                public void run()
                {
                    for ( int j=0; j<actionsPerThread; ++j )
                    {
                        try { undoManager.undo(); }
                        catch ( final Exception e )
                        {
                            fail( "Those dummy actions are not expected to fail." );
                            return;
                        }
                    }
                }
            };
        }

        // start the threads
        for ( int i=0; i<threadCount; ++i )
            threads[i].start();

        // wait for them to be finished
        for ( int i=0; i<threadCount; ++i )
            threads[i].join();

        // ensure all actions have been undone
        assertEquals( "not all actions have been undone", actionCount, actionsUndone[0].intValue() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    @After
    public void afterTest()
    {
        if ( m_currentTestCase != null )
            m_currentTestCase.closeDocument();
        else if ( m_currentDocument != null )
            m_currentDocument.close();
        m_currentTestCase = null;
        m_currentDocument = null;
        m_callbackFactory.dispose();
    }

    // -----------------------------------------------------------------------------------------------------------------
    /**
     * @return returns the undo manager belonging to a given document
     */
    private XUndoManager getUndoManager()
    {
        final XUndoManagerSupplier suppUndo = UnoRuntime.queryInterface( XUndoManagerSupplier.class, m_currentDocument.getDocument() );
        final XUndoManager undoManager = suppUndo.getUndoManager();
        assertTrue( UnoRuntime.areSame( undoManager.getParent(), m_currentDocument.getDocument() ) );
        return undoManager;
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_waitFor( final Object i_condition, final int i_milliSeconds ) throws InterruptedException
    {
        synchronized( i_condition )
        {
            i_condition.wait( i_milliSeconds );
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_setupBrokenBasicScript()
    {
        try
        {
            final XEmbeddedScripts embeddedScripts = UnoRuntime.queryInterface( XEmbeddedScripts.class, m_currentDocument.getDocument() );
            final XLibraryContainer basicLibs = embeddedScripts.getBasicLibraries();
            final XNameContainer basicLib = basicLibs.createLibrary( "default" );

            final String brokenScriptCode =
                "Option Explicit\n" +
                "\n" +
                "Sub brokenScript\n" +
                "    Dim callback as Object\n" +
                "    ThisComponent.UndoManager.enterUndoContext( \"" + getCallbackUndoContextTitle() + "\" )\n" +
                "\n" +
                "    callback = createUnoService( \"" + getCallbackComponentServiceName() + "\" )\n" +
                "    Dim emptyArgs() as new com.sun.star.beans.NamedValue\n" +
                "    Dim result as String\n" +
                "    result = callback.execute( emptyArgs() )\n" +
                "    If result = \"close\" Then\n" +
                "        ThisComponent.close( TRUE )\n" +
                "    End If\n" +
                "End Sub\n" +
                "\n";

            basicLib.insertByName( "callbacks", brokenScriptCode );
        }
        catch( com.sun.star.uno.Exception e )
        {
            fail( "caught an exception while setting up the script: " + e.toString() );
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    private XPropertySet impl_setupButton() throws com.sun.star.uno.Exception
    {
        // let the document create a shape
        final XMultiServiceFactory docAsFactory = UnoRuntime.queryInterface( XMultiServiceFactory.class,
            m_currentDocument.getDocument() );
        final XControlShape xShape = UnoRuntime.queryInterface( XControlShape.class,
            docAsFactory.createInstance( "com.sun.star.drawing.ControlShape" ) );

        // position and size of the shape
        xShape.setSize( new Size( 28 * 100, 10 * 100 ) );
        xShape.setPosition( new Point( 10 * 100, 10 * 100 ) );

        // create the form component (the model of a form control)
        final String sQualifiedComponentName = "com.sun.star.form.component.CommandButton";
        final XControlModel controlModel = UnoRuntime.queryInterface( XControlModel.class,
            getORB().createInstance( sQualifiedComponentName ) );

        // knitt both
        xShape.setControl( controlModel );

        // add the shape to the shapes collection of the document
        SpreadsheetDocument spreadsheetDoc = (SpreadsheetDocument)m_currentDocument;
        final XDrawPageSupplier suppDrawPage = UnoRuntime.queryInterface( XDrawPageSupplier.class,
            spreadsheetDoc.getSheet( 0 ) );
        final XDrawPage insertIntoPage = suppDrawPage.getDrawPage();

        final XShapes sheetShapes = UnoRuntime.queryInterface( XShapes.class, insertIntoPage );
        sheetShapes.add( xShape );

        return UnoRuntime.queryInterface( XPropertySet.class, controlModel );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_assignScript( final XPropertySet i_controlModel, final String i_interfaceName,
        final String i_interfaceMethod, final String i_scriptURI )
    {
        try
        {
            final XChild modelAsChild = UnoRuntime.queryInterface( XChild.class, i_controlModel );
            final XIndexContainer parentForm = UnoRuntime.queryInterface( XIndexContainer.class, modelAsChild.getParent() );

            final XEventAttacherManager manager = UnoRuntime.queryInterface( XEventAttacherManager.class, parentForm );

            int containerPosition = -1;
            for ( int i = 0; i < parentForm.getCount(); ++i )
            {
                final XPropertySet child = UnoRuntime.queryInterface( XPropertySet.class, parentForm.getByIndex( i ) );
                if ( UnoRuntime.areSame( child, i_controlModel ) )
                {
                    containerPosition = i;
                    break;
                }
            }
            assertFalse( "could not find the given control model within its parent", containerPosition == -1 );
            manager.registerScriptEvent( containerPosition, new ScriptEventDescriptor(
                i_interfaceName,
                i_interfaceMethod,
                "",
                "Script",
                i_scriptURI
            ) );
        }
        catch( com.sun.star.uno.Exception e )
        {
            fail( "caught an exception while assigning the script event to the button: " + e.toString() );
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_clickButton( final XPropertySet i_buttonModel ) throws NoSuchElementException, IndexOutOfBoundsException
    {
        final XControlAccess controlAccess = UnoRuntime.queryInterface( XControlAccess.class,
            m_currentDocument.getCurrentView().getController() );
        final XControl control = controlAccess.getControl( UnoRuntime.queryInterface( XControlModel.class, i_buttonModel ) );
        final XAccessible accessible = UnoRuntime.queryInterface( XAccessible.class, control );
        final XAccessibleAction controlActions = UnoRuntime.queryInterface( XAccessibleAction.class, accessible.getAccessibleContext() );
        for ( int i=0; i<controlActions.getAccessibleActionCount(); ++i )
        {
            if ( controlActions.getAccessibleActionDescription(i).equals( "click" ) )
            {
                controlActions.doAccessibleAction(i);
                return;
            }
        }
        fail( "did not find the accessible action named 'click'" );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private static class UndoListener implements XUndoManagerListener
    {
        public void undoActionAdded( UndoManagerEvent i_event )
        {
            assertFalse( "|undoActionAdded| called after document was disposed", m_isDisposed );

            ++m_undoActionsAdded;
            m_mostRecentlyAddedAction = i_event.UndoActionTitle;
        }

        public void actionUndone( UndoManagerEvent i_event )
        {
            assertFalse( "|actionUndone| called after document was disposed", m_isDisposed );

            ++m_undoCount;
            m_mostRecentlyUndone = i_event.UndoActionTitle;
        }

        public void actionRedone( UndoManagerEvent i_event )
        {
            assertFalse( "|actionRedone| called after document was disposed", m_isDisposed );

            ++m_redoCount;
        }

        public void allActionsCleared( EventObject eo )
        {
            assertFalse( "|allActionsCleared| called after document was disposed", m_isDisposed );

            m_wasCleared = true;
        }

        public void redoActionsCleared( EventObject eo )
        {
            assertFalse( "|redoActionsCleared| called after document was disposed", m_isDisposed );

            m_redoWasCleared = true;
        }

        public void resetAll( EventObject i_event )
        {
            assertFalse( "|resetAll| called after document was disposed", m_isDisposed );

            m_managerWasReset = true;
            m_activeUndoContexts.clear();
        }

        public void enteredContext( UndoManagerEvent i_event )
        {
            assertFalse( "|enteredContext| called after document was disposed", m_isDisposed );

            m_activeUndoContexts.push( i_event.UndoActionTitle );
            assertEquals( "different opinions on the context nesting level (after entering)",
                m_activeUndoContexts.size(), i_event.UndoContextDepth );
        }

        public void enteredHiddenContext( UndoManagerEvent i_event )
        {
            assertFalse( "|enteredHiddenContext| called after document was disposed", m_isDisposed );

            m_activeUndoContexts.push( i_event.UndoActionTitle );
            assertEquals( "different opinions on the context nesting level (after entering hidden)",
                m_activeUndoContexts.size(), i_event.UndoContextDepth );
        }

        public void leftContext( UndoManagerEvent i_event )
        {
            assertFalse( "|leftContext| called after document was disposed", m_isDisposed );

            assertEquals( "nested undo context descriptions do not match", m_activeUndoContexts.pop(), i_event.UndoActionTitle );
            assertEquals( "different opinions on the context nesting level (after leaving)",
                m_activeUndoContexts.size(), i_event.UndoContextDepth );
            m_leftContext = true;
            impl_notifyContextDepth();
        }

        public void leftHiddenContext( UndoManagerEvent i_event )
        {
            assertFalse( "|leftHiddenContext| called after document was disposed", m_isDisposed );
            assertEquals( "|leftHiddenContext| is not expected to notify an action title", 0, i_event.UndoActionTitle.length() );

            m_activeUndoContexts.pop();
            assertEquals( "different opinions on the context nesting level (after leaving)",
                m_activeUndoContexts.size(), i_event.UndoContextDepth );
            m_leftHiddenContext = true;
            impl_notifyContextDepth();
        }

        public void cancelledContext( UndoManagerEvent i_event )
        {
            assertFalse( "|cancelledContext| called after document was disposed", m_isDisposed );
            assertEquals( "|cancelledContext| is not expected to notify an action title", 0, i_event.UndoActionTitle.length() );

            m_activeUndoContexts.pop();
            assertEquals( "different opinions on the context nesting level (after cancelling)",
                m_activeUndoContexts.size(), i_event.UndoContextDepth );
            m_cancelledContext = true;
            impl_notifyContextDepth();
        }

        public void disposing( EventObject i_event )
        {
            m_isDisposed = true;
        }

        public void waitForAllContextsClosed( final int i_milliSeconds ) throws InterruptedException
        {
            synchronized ( m_allContextsClosedCondition )
            {
                if ( m_activeUndoContexts.empty() )
                    return;
                m_allContextsClosedCondition.wait( i_milliSeconds );
            }
        }

        private void impl_notifyContextDepth()
        {
            synchronized ( m_allContextsClosedCondition )
            {
                if ( m_activeUndoContexts.empty() )
                {
                    m_allContextsClosedCondition.notifyAll();
                }
            }
        }

        private int     getUndoActionsAdded() { return m_undoActionsAdded; }
        private int     getUndoActionCount() { return m_undoCount; }
        private int     getRedoActionCount() { return m_redoCount; }
        private String  getCurrentUndoContextTitle() { return m_activeUndoContexts.peek(); }
        private String  getMostRecentlyAddedActionTitle() { return m_mostRecentlyAddedAction; };
        private String  getMostRecentlyUndoneTitle() { return m_mostRecentlyUndone; }
        private int     getCurrentUndoContextDepth() { return m_activeUndoContexts.size(); }
        private boolean isDisposed() { return m_isDisposed; }
        private boolean wasContextLeft() { return m_leftContext; }
        private boolean wasHiddenContextLeft() { return m_leftHiddenContext; }
        private boolean hasContextBeenCancelled() { return m_cancelledContext; }
        private boolean wereStacksCleared() { return m_wasCleared; }
        private boolean wasRedoStackCleared() { return m_redoWasCleared; }
        private boolean wasManagerReset() { return m_managerWasReset; }

        void reset()
        {
            m_undoActionsAdded = m_undoCount = m_redoCount = 0;
            m_activeUndoContexts.clear();
            m_mostRecentlyAddedAction = m_mostRecentlyUndone = null;
            // m_isDisposed is not cleared, intentionally
            m_leftContext = m_leftHiddenContext = m_cancelledContext = m_wasCleared = m_redoWasCleared = m_managerWasReset = false;
        }

        private int     m_undoActionsAdded = 0;
        private int     m_undoCount = 0;
        private int     m_redoCount = 0;
        private boolean m_isDisposed = false;
        private boolean m_leftContext = false;
        private boolean m_leftHiddenContext = false;
        private boolean m_cancelledContext = false;
        private boolean m_wasCleared = false;
        private boolean m_redoWasCleared = false;
        private boolean m_managerWasReset = false;
        private Stack< String >
                        m_activeUndoContexts = new Stack<String>();
        private String  m_mostRecentlyAddedAction = null;
        private String  m_mostRecentlyUndone = null;
        private final Object    m_allContextsClosedCondition = new Object();
    };

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_checkUndo() throws Exception
    {
        System.out.println( "testing: " + m_currentTestCase.getDocumentDescription() );
        m_currentDocument = m_currentTestCase.getDocument();
        m_currentTestCase.initializeDocument();
        m_currentTestCase.verifyInitialDocumentState();

        final XUndoManager undoManager = getUndoManager();
        undoManager.clear();
        assertFalse( "clearing the Undo manager should result in the impossibility to undo anything", undoManager.isUndoPossible() );
        assertFalse( "clearing the Undo manager should result in the impossibility to redo anything", undoManager.isRedoPossible() );

        m_undoListener = new UndoListener();
        undoManager.addUndoManagerListener( m_undoListener );

        impl_testSingleModification( undoManager );
        impl_testMultipleModifications( undoManager );
        impl_testCustomUndoActions( undoManager );
        impl_testLocking( undoManager );
        impl_testNestedContexts( undoManager );
        impl_testErrorHandling( undoManager );
        impl_testContextHandling( undoManager );
        impl_testStackHandling( undoManager );
        impl_testClearance( undoManager );
        impl_testHiddenContexts( undoManager );

        // close the document, ensure the Undo manager listener gets notified
        m_currentTestCase.closeDocument();
        m_currentTestCase = null;
        m_currentDocument = null;
        assertTrue( "document is closed, but the UndoManagerListener has not been notified of the disposal", m_undoListener.isDisposed() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_testSingleModification( final XUndoManager i_undoManager ) throws com.sun.star.uno.Exception
    {
        m_currentTestCase.doSingleModification();
        m_currentTestCase.verifySingleModificationDocumentState();

        // undo the modification, ensure the listener got the proper notifications
        assertEquals( "We did not yet do a undo!", 0, m_undoListener.getUndoActionCount() );
        i_undoManager.undo();
        assertEquals( "A simple undo does not result in the proper Undo count.",
            1, m_undoListener.getUndoActionCount() );

        // verify the document is in its initial state, again
        m_currentTestCase.verifyInitialDocumentState();

        // redo the modification, ensure the listener got the proper notifications
        assertEquals( "did not yet do a redo!", 0, m_undoListener.getRedoActionCount() );
        i_undoManager.redo();
        assertEquals( "did a redo, but got no notification of it!", 1, m_undoListener.getRedoActionCount() );
        // ensure the document is in the proper state, again
        m_currentTestCase.verifySingleModificationDocumentState();

        // now do an Undo via the UI (aka the dispatch API), and see if this works, and notifies the listener as
        // expected
        m_currentTestCase.getDocument().getCurrentView().dispatch( ".uno:Undo" );
        m_currentTestCase.verifyInitialDocumentState();
        assertEquals( "UI-Undo does not notify the listener", 2, m_undoListener.getUndoActionCount() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_testMultipleModifications( final XUndoManager i_undoManager ) throws com.sun.star.uno.Exception
    {
        m_undoListener.reset();
        assertEquals( "unexpected initial undo context depth", 0, m_undoListener.getCurrentUndoContextDepth() );
        i_undoManager.enterUndoContext( "Batch Changes" );
        assertEquals( "unexpected undo context depth after entering a context",
            1, m_undoListener.getCurrentUndoContextDepth() );
        assertEquals( "entering an Undo context has not been notified properly",
            "Batch Changes", m_undoListener.getCurrentUndoContextTitle() );

        final int modifications = m_currentTestCase.doMultipleModifications();
        assertEquals( "unexpected number of undo actions while doing batch changes to the document",
            modifications, m_undoListener.getUndoActionsAdded() );
        assertEquals( "seems the document operations touched the undo context depth",
            1, m_undoListener.getCurrentUndoContextDepth() );

        i_undoManager.leaveUndoContext();
        assertEquals( "unexpected undo context depth after leaving the last context",
            0, m_undoListener.getCurrentUndoContextDepth() );
        assertEquals( "no Undo done, yet - still the listener has been notified of an Undo action",
            0, m_undoListener.getUndoActionCount() );

        i_undoManager.undo();
        assertEquals( "Just did an undo - the listener should have been notified", 1, m_undoListener.getUndoActionCount() );
        m_currentTestCase.verifyInitialDocumentState();
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_testCustomUndoActions( final XUndoManager i_undoManager ) throws com.sun.star.uno.Exception
    {
        i_undoManager.clear();
        m_undoListener.reset();
        assertFalse( "undo stack not empty after clearing the undo manager", i_undoManager.isUndoPossible() );
        assertFalse( "redo stack not empty after clearing the undo manager", i_undoManager.isRedoPossible() );
        assertArrayEquals( ">0 descriptions for an empty undo stack?",
            new String[0], i_undoManager.getAllUndoActionTitles() );
        assertArrayEquals( ">0 descriptions for an empty redo stack?",
            new String[0], i_undoManager.getAllRedoActionTitles() );

        // add two actions, one directly, one within a context
        final CustomUndoAction action1 = new CustomUndoAction( "UndoAction1" );
        i_undoManager.addUndoAction( action1 );
        assertEquals( "Adding an undo action not observed by the listener", 1, m_undoListener.getUndoActionsAdded() );
        assertEquals( "Adding an undo action did not notify the proper title",
            action1.getTitle(), m_undoListener.getMostRecentlyAddedActionTitle() );
        final String contextTitle = "Undo Context";
        i_undoManager.enterUndoContext( contextTitle );
        final CustomUndoAction action2 = new CustomUndoAction( "UndoAction2" );
        i_undoManager.addUndoAction( action2 );
        assertEquals( "Adding an undo action not observed by the listener",
            2, m_undoListener.getUndoActionsAdded() );
        assertEquals( "Adding an undo action did not notify the proper title",
            action2.getTitle(), m_undoListener.getMostRecentlyAddedActionTitle() );
        i_undoManager.leaveUndoContext();

        // see if the manager has proper descriptions
        assertArrayEquals( "unexpected Redo descriptions after adding two actions",
            new String[0], i_undoManager.getAllRedoActionTitles() );
        assertArrayEquals( "unexpected Undo descriptions after adding two actions",
            new String[]{contextTitle, action1.getTitle()}, i_undoManager.getAllUndoActionTitles() );

        // undo one action
        i_undoManager.undo();
        assertEquals( "improper action title notified during programmatic Undo",
            contextTitle, m_undoListener.getMostRecentlyUndoneTitle() );
        assertTrue( "nested custom undo action has not been undone as expected", action2.undoCalled() );
        assertFalse( "nested custom undo action has not been undone as expected", action1.undoCalled() );
        assertArrayEquals( "unexpected Redo descriptions after undoing a nested custom action",
            new String[]{contextTitle}, i_undoManager.getAllRedoActionTitles() );
        assertArrayEquals( "unexpected Undo descriptions after undoing a nested custom action",
            new String[]{action1.getTitle()}, i_undoManager.getAllUndoActionTitles() );

        // undo the second action, via UI dispatches
        m_currentTestCase.getDocument().getCurrentView().dispatch( ".uno:Undo" );
        assertEquals( "improper action title notified during UI Undo", action1.getTitle(), m_undoListener.getMostRecentlyUndoneTitle() );
        assertTrue( "nested custom undo action has not been undone as expected", action1.undoCalled() );
        assertArrayEquals( "unexpected Redo descriptions after undoing the second custom action",
            new String[]{action1.getTitle(), contextTitle}, i_undoManager.getAllRedoActionTitles() );
        assertArrayEquals( "unexpected Undo descriptions after undoing the second custom action",
            new String[0], i_undoManager.getAllUndoActionTitles() );

        // check the actions are disposed when the stacks are cleared
        i_undoManager.clear();
        assertTrue( action1.disposed() && action2.disposed() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_testLocking( final XUndoManager i_undoManager ) throws com.sun.star.uno.Exception
    {
        i_undoManager.reset();
        m_undoListener.reset();

        // implicit Undo actions, triggered by changes to the document
        assertFalse( "unexpected initial locking state", i_undoManager.isLocked() );
        i_undoManager.lock();
        assertTrue( "just locked the manager, why does it lie?", i_undoManager.isLocked() );
        m_currentTestCase.doSingleModification();
        assertEquals( "when the Undo manager is locked, no implicit additions should happen",
            0, m_undoListener.getUndoActionsAdded() );
        assertTrue( "Undo manager gets unlocked as a side effect of performing a simple operation", i_undoManager.isLocked() );
        i_undoManager.unlock();
        assertEquals( "unlock is not expected to add collected actions - they should be discarded",
            0, m_undoListener.getUndoActionsAdded() );
        assertFalse( "just unlocked the manager, why does it lie?", i_undoManager.isLocked() );

        // explicit Undo actions
        i_undoManager.lock();
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.unlock();
        assertEquals( "explicit Undo actions are expected to be ignored when the manager is locked",
            0, m_undoListener.getUndoActionsAdded() );

        // Undo contexts while being locked
        i_undoManager.lock();
        i_undoManager.enterUndoContext( "Dummy Context" );
        i_undoManager.enterHiddenUndoContext();
        assertEquals( "entering Undo contexts should be ignored when the manager is locked", 0, m_undoListener.getCurrentUndoContextDepth() );
        i_undoManager.leaveUndoContext();
        i_undoManager.leaveUndoContext();
        i_undoManager.unlock();

        // |unlock| error handling
        assertFalse( "internal error: manager should not be locked at this point in time", i_undoManager.isLocked() );
        boolean caughtExpected = false;
        try { i_undoManager.unlock(); } catch ( final NotLockedException e ) { caughtExpected = true; }
        assertTrue( "unlocking the manager when it is not locked should throw", caughtExpected );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_testContextHandling( final XUndoManager i_undoManager ) throws com.sun.star.uno.Exception
    {
        // .............................................................................................................
        // part I: non-empty contexts
        i_undoManager.reset();
        m_undoListener.reset();

        // put one action on the undo and one on the redo stack, as precondition for the following tests
        final XUndoAction undoAction1 = new CustomUndoAction( "Undo Action 1" );
        i_undoManager.addUndoAction( undoAction1 );
        final XUndoAction undoAction2 = new CustomUndoAction( "Undo Action 2" );
        i_undoManager.addUndoAction( undoAction2 );
        i_undoManager.undo();
        assertTrue( "precondition for context handling tests not met (1)", i_undoManager.isUndoPossible() );
        assertTrue( "precondition for context handling tests not met (2)", i_undoManager.isRedoPossible() );
        assertArrayEquals( new String[] { undoAction1.getTitle() }, i_undoManager.getAllUndoActionTitles() );
        assertArrayEquals( new String[] { undoAction2.getTitle() }, i_undoManager.getAllRedoActionTitles() );

        final String[] expectedRedoActionComments = new String[] { undoAction2.getTitle() };
        assertArrayEquals( expectedRedoActionComments, i_undoManager.getAllRedoActionTitles() );

        // enter a context
        i_undoManager.enterUndoContext( "Undo Context" );
        // this should not (yet) touch the redo stack
        assertArrayEquals( expectedRedoActionComments, i_undoManager.getAllRedoActionTitles() );
        assertEquals( "unexpected undo context depth after entering a context", 1, m_undoListener.getCurrentUndoContextDepth() );
        // add a single action
        XUndoAction undoAction3 = new CustomUndoAction( "Undo Action 3" );
        i_undoManager.addUndoAction( undoAction3 );
        // still, the redo stack should be untouched - added at a lower level does not affect it at all
        assertArrayEquals( expectedRedoActionComments, i_undoManager.getAllRedoActionTitles() );

        // while the context is open, its title should already contribute to the stack, ...
        assertEquals( "Undo Context", i_undoManager.getCurrentUndoActionTitle() );
        // ... getAllUndo/RedoActionTitles should operate on the top level, not on the level defined by the open
        // context,  ...
        assertArrayEquals( new String[] { "Undo Context", undoAction1.getTitle() },
            i_undoManager.getAllUndoActionTitles() );
        // ... but Undo and Redo should be impossible as long as the context is open
        assertFalse( i_undoManager.isUndoPossible() );
        assertFalse( i_undoManager.isRedoPossible() );

        // leave the context, check the listener has been notified properly, and the notified context depth is correct
        i_undoManager.leaveUndoContext();
        assertTrue( m_undoListener.wasContextLeft() );
        assertFalse( m_undoListener.wasHiddenContextLeft() );
        assertFalse( m_undoListener.hasContextBeenCancelled() );
        assertEquals( "unexpected undo context depth leaving a non-empty context", 0, m_undoListener.getCurrentUndoContextDepth() );
        // leaving a non-empty context should have cleare the redo stack
        assertArrayEquals( new String[0], i_undoManager.getAllRedoActionTitles() );
        assertTrue( m_undoListener.wasRedoStackCleared() );

        // .............................................................................................................
        // part II: empty contexts
        i_undoManager.reset();
        m_undoListener.reset();

        // enter a context, leave it immediately without adding an action to it
        i_undoManager.enterUndoContext( "Undo Context" );
        i_undoManager.leaveUndoContext();
        assertFalse( m_undoListener.wasContextLeft() );
        assertFalse( m_undoListener.wasHiddenContextLeft() );
        assertTrue( m_undoListener.hasContextBeenCancelled() );
        assertFalse( "leaving an empty context should silently remove it, and not contribute to the stack",
            i_undoManager.isUndoPossible() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_testNestedContexts( final XUndoManager i_undoManager ) throws com.sun.star.uno.Exception
    {
        i_undoManager.reset();
        m_undoListener.reset();
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
        assertEquals( "undoing a single action notifies a wrong title", action4.getTitle(), m_undoListener.getMostRecentlyUndoneTitle() );
        assertTrue( "custom Undo not called", action4.undoCalled() );
        assertFalse( "too many custom Undos called", action1.undoCalled() || action2.undoCalled() || action3.undoCalled() );
        i_undoManager.undo();
        assertTrue( "nested actions not properly undone", action1.undoCalled() && action2.undoCalled() && action3.undoCalled() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_testErrorHandling( final XUndoManager i_undoManager ) throws com.sun.star.uno.Exception
    {
        i_undoManager.reset();
        m_undoListener.reset();

        // try retrieving the comments for the current Undo/Redo - this should fail
        boolean caughtExpected = false;
        try { i_undoManager.getCurrentUndoActionTitle(); }
        catch( final EmptyUndoStackException e ) { caughtExpected = true; }
        assertTrue( "trying the title of the current Undo action is expected to fail for an empty stack", caughtExpected );

        caughtExpected = false;
        try { i_undoManager.getCurrentRedoActionTitle(); }
        catch( final EmptyUndoStackException e ) { caughtExpected = true; }
        assertTrue( "trying the title of the current Redo action is expected to fail for an empty stack", caughtExpected );

        caughtExpected = false;
        try { i_undoManager.undo(); } catch ( final EmptyUndoStackException e ) { caughtExpected = true; }
        assertTrue( "undo should throw if no Undo action is on the stack", caughtExpected );

        caughtExpected = false;
        try { i_undoManager.redo(); } catch ( final EmptyUndoStackException e ) { caughtExpected = true; }
        assertTrue( "redo should throw if no Redo action is on the stack", caughtExpected );

        caughtExpected = false;
        try { i_undoManager.leaveUndoContext(); } catch ( final InvalidStateException e ) { caughtExpected = true; }
        assertTrue( "leaveUndoContext should throw if no context is currently open", caughtExpected );

        caughtExpected = false;
        try { i_undoManager.addUndoAction( null ); } catch ( com.sun.star.lang.IllegalArgumentException e ) { caughtExpected = true; }
        assertTrue( "adding a NULL action should be rejected", caughtExpected );

        i_undoManager.reset();
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.undo();
        i_undoManager.enterUndoContext( "Undo Context" );
        // those methods should fail when a context is open:
        final String[] methodNames = new String[] { "undo", "redo", "clear", "clearRedo" };
        for ( int i=0; i<methodNames.length; ++i )
        {
            caughtExpected = false;
            try
            {
                Method method = i_undoManager.getClass().getMethod( methodNames[i], new Class[0] );
                method.invoke( i_undoManager, new Object[0] );
            }
            catch ( IllegalAccessException ex ) { }
            catch ( IllegalArgumentException ex ) { }
            catch ( InvocationTargetException ex )
            {
                Throwable targetException = ex.getTargetException();
                caughtExpected = ( targetException instanceof UndoContextNotClosedException );
            }
            catch ( NoSuchMethodException ex ) { }
            catch ( SecurityException ex ) { }

            assertTrue( methodNames[i] + " should be rejected when there is an open context", caughtExpected );
        }
        i_undoManager.leaveUndoContext();

        // try Undo actions which fail in their Undo/Redo
        for ( int i=0; i<4; ++i )
        {
            final boolean undo = ( i < 2 );
            final boolean doByAPI = ( i % 2 ) == 0;

            i_undoManager.reset();
            i_undoManager.addUndoAction( new CustomUndoAction() );
            i_undoManager.addUndoAction( new FailingUndoAction( undo ? FAIL_UNDO : FAIL_REDO ) );
            i_undoManager.addUndoAction( new CustomUndoAction() );
            i_undoManager.undo();
            if ( !undo )
                i_undoManager.undo();
            // assert preconditions for the below test
            assertTrue( i_undoManager.isUndoPossible() );
            assertTrue( i_undoManager.isRedoPossible() );

            boolean caughtUndoFailed = false;
            try
            {
                if ( undo )
                    if ( doByAPI )
                        i_undoManager.undo();
                    else
                        m_currentTestCase.getDocument().getCurrentView().dispatch( ".uno:Undo" );
                else
                    if ( doByAPI )
                        i_undoManager.redo();
                    else
                        m_currentTestCase.getDocument().getCurrentView().dispatch( ".uno:Redo" );
            }
            catch ( UndoFailedException e )
            {
                caughtUndoFailed = true;
            }
            if ( doByAPI )
                assertTrue( "Exceptions in XUndoAction.undo should be propagated at the API", caughtUndoFailed );
            else
                assertFalse( "Undo/Redo by UI should not let escape Exceptions", caughtUndoFailed );
            if ( undo )
            {
                assertFalse( "a failing Undo should clear the Undo stack", i_undoManager.isUndoPossible() );
                assertTrue( "a failing Undo should /not/ clear the Redo stack", i_undoManager.isRedoPossible() );
            }
            else
            {
                assertTrue( "a failing Redo should /not/ clear the Undo stack", i_undoManager.isUndoPossible() );
                assertFalse( "a failing Redo should clear the Redo stack", i_undoManager.isRedoPossible() );
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_testStackHandling( final XUndoManager i_undoManager ) throws com.sun.star.uno.Exception
    {
        i_undoManager.reset();
        m_undoListener.reset();

        assertFalse( i_undoManager.isUndoPossible() );
        assertFalse( i_undoManager.isRedoPossible() );

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

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_testClearance( final XUndoManager i_undoManager ) throws com.sun.star.uno.Exception
    {
        i_undoManager.reset();
        m_undoListener.reset();

        // add an action, clear the stack, verify the listener has been called
        i_undoManager.addUndoAction( new CustomUndoAction() );
        assertFalse( "clearance listener unexpectedly called", m_undoListener.wereStacksCleared() );
        assertFalse( "redo-clearance listener unexpectedly called", m_undoListener.wasRedoStackCleared() );
        i_undoManager.clear();
        assertTrue( "clearance listener not called as expected", m_undoListener.wereStacksCleared() );
        assertFalse( "redo-clearance listener unexpectedly called (2)", m_undoListener.wasRedoStackCleared() );

        // ensure the listener is also called if the stack is actually empty at the moment of the call
        m_undoListener.reset();
        assertFalse( i_undoManager.isUndoPossible() );
        i_undoManager.clear();
        assertTrue( "clearance listener is also expected to be called if the stack was empty before", m_undoListener.wereStacksCleared() );

        // ensure the proper listeners are called for clearRedo
        m_undoListener.reset();
        i_undoManager.clearRedo();
        assertFalse( m_undoListener.wereStacksCleared() );
        assertTrue( m_undoListener.wasRedoStackCleared() );

        // ensure the redo listener is also called upon implicit redo stack clearance
        m_undoListener.reset();
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.undo();
        assertTrue( i_undoManager.isUndoPossible() );
        assertTrue( i_undoManager.isRedoPossible() );
        i_undoManager.addUndoAction( new CustomUndoAction() );
        assertFalse( i_undoManager.isRedoPossible() );
        assertTrue( "implicit clearance of the Redo stack does not notify listeners", m_undoListener.wasRedoStackCleared() );

        // test resetting the manager
        m_undoListener.reset();
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.undo();
        assertTrue( i_undoManager.isUndoPossible() );
        assertTrue( i_undoManager.isRedoPossible() );
        i_undoManager.reset();
        assertFalse( i_undoManager.isUndoPossible() );
        assertFalse( i_undoManager.isRedoPossible() );
        assertTrue( "|reset| does not properly notify", m_undoListener.wasManagerReset() );

        // resetting the manager, with open undo contexts
        m_undoListener.reset();
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.enterUndoContext( "Undo Context" );
        i_undoManager.addUndoAction( new CustomUndoAction() );
        i_undoManager.enterHiddenUndoContext();
        i_undoManager.reset();
        assertTrue( "|reset| while contexts are open does not properly notify", m_undoListener.wasManagerReset() );
        // verify the manager really has the proper context depth now
        i_undoManager.enterUndoContext( "Undo Context" );
        assertEquals( "seems that |reset| did not really close the open contexts", 1, m_undoListener.getCurrentUndoContextDepth() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_testHiddenContexts( final XUndoManager i_undoManager ) throws com.sun.star.uno.Exception
    {
        i_undoManager.reset();
        m_undoListener.reset();
        assertFalse( "precondition for testing hidden undo contexts not met", i_undoManager.isUndoPossible() );

        // entering a hidden context should be rejected if the stack is empty
        boolean caughtExpected = false;
        try { i_undoManager.enterHiddenUndoContext(); }
        catch ( final EmptyUndoStackException e ) { caughtExpected = true; }
        assertTrue( "entering hidden contexts should be denied on an empty stack", caughtExpected );

        // but it should be allowed if the context is not empty
        final CustomUndoAction undoAction0 = new CustomUndoAction( "Step 0" );
        i_undoManager.addUndoAction( undoAction0 );
        final CustomUndoAction undoAction1 = new CustomUndoAction( "Step 1" );
        i_undoManager.addUndoAction( undoAction1 );
        i_undoManager.enterHiddenUndoContext();
        final CustomUndoAction hiddenUndoAction = new CustomUndoAction( "hidden context action" );
        i_undoManager.addUndoAction( hiddenUndoAction );
        i_undoManager.leaveUndoContext();
        assertFalse( "leaving a hidden should not call |leftUndocontext|", m_undoListener.wasContextLeft() );
        assertTrue( "leaving a hidden does not call |leftHiddenUndocontext|", m_undoListener.wasHiddenContextLeft() );
        assertFalse( "leaving a non-empty hidden context claims to have cancelled it", m_undoListener.hasContextBeenCancelled() );
        assertEquals( "leaving a hidden context is not properly notified", 0, m_undoListener.getCurrentUndoContextDepth() );
        assertArrayEquals( "unexpected Undo stack after leaving a hidden context",
            new String[] { undoAction1.getTitle(), undoAction0.getTitle() },
            i_undoManager.getAllUndoActionTitles() );

        // and then calling |undo| once should not only undo everything in the hidden context, but also
        // the previous action - but not more
        i_undoManager.undo();
        assertTrue( "Undo after leaving a hidden context does not actually undo the context actions",
            hiddenUndoAction.undoCalled() );
        assertTrue( "Undo after leaving a hidden context does not undo the predecessor action",
            undoAction1.undoCalled() );
        assertFalse( "Undo after leaving a hidden context undoes too much",
            undoAction0.undoCalled() );

        // leaving an empty hidden context should call the proper notification method
        m_undoListener.reset();
        i_undoManager.enterHiddenUndoContext();
        i_undoManager.leaveUndoContext();
        assertFalse( m_undoListener.wasContextLeft() );
        assertFalse( m_undoListener.wasHiddenContextLeft() );
        assertTrue( m_undoListener.hasContextBeenCancelled() );

        // nesting hidden and normal contexts
        m_undoListener.reset();
        i_undoManager.reset();
        final CustomUndoAction action0 = new CustomUndoAction( "action 0" );
        i_undoManager.addUndoAction( action0 );
        i_undoManager.enterUndoContext( "context 1" );
        final CustomUndoAction action1 = new CustomUndoAction( "action 1" );
        i_undoManager.addUndoAction( action1 );
        i_undoManager.enterHiddenUndoContext();
        final CustomUndoAction action2 = new CustomUndoAction( "action 2" );
        i_undoManager.addUndoAction( action2 );
        i_undoManager.enterUndoContext( "context 2" );
        // is entering a hidden context rejected even at the nesting level > 0 (the above test was for nesting level == 0)?
        caughtExpected = false;
        try { i_undoManager.enterHiddenUndoContext(); }
        catch( final EmptyUndoStackException e ) { caughtExpected = true; }
        assertTrue( "at a nesting level > 0, denied hidden contexts does not work as expected", caughtExpected );
        final CustomUndoAction action3 = new CustomUndoAction( "action 3" );
        i_undoManager.addUndoAction( action3 );
        i_undoManager.enterHiddenUndoContext();
        assertEquals( "mixed hidden/normal context do are not properly notified", 4, m_undoListener.getCurrentUndoContextDepth() );
        i_undoManager.leaveUndoContext();
        assertTrue( "the left context was empty - why wasn't 'cancelled' notified?", m_undoListener.hasContextBeenCancelled() );
        assertFalse( m_undoListener.wasContextLeft() );
        assertFalse( m_undoListener.wasHiddenContextLeft() );
        i_undoManager.leaveUndoContext();
        i_undoManager.leaveUndoContext();
        i_undoManager.leaveUndoContext();
        i_undoManager.undo();
        assertFalse( "one action too much has been undone", action0.undoCalled() );
        assertTrue( action1.undoCalled() );
        assertTrue( action2.undoCalled() );
        assertTrue( action3.undoCalled() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private XComponentContext getContext()
    {
        return m_connection.getComponentContext();
    }

    // -----------------------------------------------------------------------------------------------------------------
    private XMultiServiceFactory getORB()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(
            XMultiServiceFactory.class, getContext().getServiceManager() );
        return xMSF1;
    }

    // -----------------------------------------------------------------------------------------------------------------
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println( "--------------------------------------------------------------------------------" );
        System.out.println( "starting class: " + UndoManager.class.getName() );
        System.out.println( "connecting ..." );
        m_connection.setUp();
    }

    // -----------------------------------------------------------------------------------------------------------------
    @AfterClass
    public static void tearDownConnection() throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println();
        System.out.println( "tearing down connection" );
        m_connection.tearDown();
        System.out.println( "finished class: " + UndoManager.class.getName() );
        System.out.println( "--------------------------------------------------------------------------------" );
    }

    // -----------------------------------------------------------------------------------------------------------------
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

        public void undo() throws UndoFailedException
        {
            m_undoCalled = true;
        }

        public void redo() throws UndoFailedException
        {
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
        boolean disposed() { return m_disposed; }

        private final String    m_title;
        private boolean         m_undoCalled = false;
        private boolean         m_disposed = false;
    }

    private static short FAIL_UNDO = 1;
    private static short FAIL_REDO = 2;

    private static class FailingUndoAction implements XUndoAction
    {
        FailingUndoAction( final short i_failWhich )
        {
            m_failWhich = i_failWhich;
        }

        public String getTitle()
        {
            return "failing undo";
        }

        public void undo() throws UndoFailedException
        {
            if ( m_failWhich != FAIL_REDO )
                impl_throw();
        }

        public void redo() throws UndoFailedException
        {
            if ( m_failWhich != FAIL_UNDO )
                impl_throw();
        }

        private void impl_throw() throws UndoFailedException
        {
            throw new UndoFailedException();
        }

        private final short m_failWhich;
    }

    // -----------------------------------------------------------------------------------------------------------------
    private static class CountingUndoAction implements XUndoAction
    {
        CountingUndoAction( final int i_expectedOrder, final Object i_lock, final Integer[] i_actionsUndoneCounter )
        {
            m_expectedOrder = i_expectedOrder;
            m_lock = i_lock;
            m_actionsUndoneCounter = i_actionsUndoneCounter;
        }

        public String getTitle()
        {
            return "Counting Undo Action";
        }

        public void undo() throws UndoFailedException
        {
            synchronized( m_lock )
            {
                assertEquals( "Undo action called out of order", m_expectedOrder, m_actionsUndoneCounter[0].intValue() );
                ++m_actionsUndoneCounter[0];
            }
        }

        public void redo() throws UndoFailedException
        {
            fail( "CountingUndoAction.redo is not expected to be called in this test." );
        }
        private final int       m_expectedOrder;
        private final Object    m_lock;
        private Integer[]       m_actionsUndoneCounter;
    }

    // -----------------------------------------------------------------------------------------------------------------
    private static String getCallbackUndoContextTitle()
    {
        return "Some Unfinished Undo Context";
    }

    // -----------------------------------------------------------------------------------------------------------------
    private static String getCallbackComponentServiceName()
    {
        return "org.openoffice.complex.sfx2.Callback";
    }

    // -----------------------------------------------------------------------------------------------------------------
    /**
     * a factory for a callback component which, at OOo runtime, is inserted into OOo's "component repository"
     */
    private class CallbackComponentFactory implements XSingleComponentFactory, XServiceInfo, XComponent
    {
        public Object createInstanceWithContext( XComponentContext i_context ) throws com.sun.star.uno.Exception
        {
            return new CallbackComponent();
        }

        public Object createInstanceWithArgumentsAndContext( Object[] i_arguments, XComponentContext i_context ) throws com.sun.star.uno.Exception
        {
            return createInstanceWithContext( i_context );
        }

        public String getImplementationName()
        {
            return "org.openoffice.complex.sfx2.CallbackComponent";
        }

        public boolean supportsService( String i_serviceName )
        {
            return i_serviceName.equals( getCallbackComponentServiceName() );
        }

        public String[] getSupportedServiceNames()
        {
            return new String[] { getCallbackComponentServiceName() };
        }

        @SuppressWarnings("unchecked")
        public void dispose()
        {
            final EventObject event = new EventObject( this );

            final ArrayList<XEventListener> eventListenersCopy = (ArrayList<XEventListener>)m_eventListeners.clone();
            final Iterator<XEventListener> iter = eventListenersCopy.iterator();
            while ( iter.hasNext() )
            {
                iter.next().disposing( event );
            }
        }

        public void addEventListener( XEventListener i_listener )
        {
            if ( i_listener != null )
                m_eventListeners.add( i_listener );
        }

        public void removeEventListener( XEventListener i_listener )
        {
            m_eventListeners.remove( i_listener );
        }

        private final ArrayList<XEventListener> m_eventListeners = new ArrayList<XEventListener>();
    };

    // -----------------------------------------------------------------------------------------------------------------
    private class CallbackComponent implements XJob, XTypeProvider
    {
        CallbackComponent()
        {
        }

        public Object execute( NamedValue[] i_parameters ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.uno.Exception
        {
            // this method is called from within the Basic script which is to check whether the OOo framework
            // properly cleans up unfinished Undo contexts. It is called immediately after the context has been
            // entered, so verify the expected Undo manager state.
            assertEquals( getCallbackUndoContextTitle(), m_undoListener.getCurrentUndoContextTitle() );
            assertEquals( 1, m_undoListener.getCurrentUndoContextDepth() );

            synchronized( m_callbackCondition )
            {
                m_callbackCalled = true;
                m_callbackCondition.notifyAll();
            }
            return m_closeAfterCallback ? "close" : "";
        }

        public Type[] getTypes()
        {
            final Class<?> interfaces[] = getClass().getInterfaces();
            Type types[] = new Type[ interfaces.length ];
            for ( int i = 0; i < interfaces.length; ++i )
                types[i] = new Type(interfaces[i]);
            return types;
        }

        public byte[] getImplementationId()
        {
            return getClass().toString().getBytes();
        }
    }

    private static final OfficeConnection   m_connection = new OfficeConnection();
    private DocumentTest                    m_currentTestCase;
    private OfficeDocument                  m_currentDocument;
    private UndoListener                    m_undoListener;
    private CallbackComponentFactory        m_callbackFactory = null;
    private boolean                         m_callbackCalled = false;
    private boolean                         m_closeAfterCallback = false;
    private final Object                    m_callbackCondition = new Object();
}
