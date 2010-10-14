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
import complex.sfx2.undo.DocumentTest;
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
    public void checkCalcUndo() throws Exception
    {
        impl_checkUndo( CalcDocumentTest.class );
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
        public void undoActionAdded( UndoManagerEvent ume )
        {
            ++m_undoActionsAdded;
        }

        public void actionUndone( UndoManagerEvent ume )
        {
            ++m_undoCount;
        }

        public void actionRedone( UndoManagerEvent ume )
        {
            ++m_redoCount;
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
        }

        public void enteredHiddenUndoContext( UndoManagerEvent ume )
        {
        }

        public void leftUndoContext( UndoManagerEvent i_event )
        {
            assertEquals( "undo context order is suspicious", m_activeUndoContexts.pop(), i_event.UndoActionTitle );
        }

        int     getUndoActionsAdded() { return m_undoActionsAdded; }
        int     getUndoActionCount() { return m_undoCount; }
        int     getRedoActionCount() { return m_redoCount; }
        String  getCurrentUndoContextTitle() { return m_activeUndoContexts.peek(); }
        int     getUndoContextDepth() { return m_activeUndoContexts.size(); }

        void reset()
        {
            m_undoActionsAdded = m_undoCount = m_redoCount = 0;
            m_activeUndoContexts.clear();
        }

        private int     m_undoActionsAdded = 0;
        private int     m_undoCount = 0;
        private int     m_redoCount = 0;
        private Stack< String >
                        m_activeUndoContexts = new Stack<String>();
    };

    private void impl_checkUndo( final Class i_testClass ) throws Exception
    {
        final Constructor ctor = i_testClass.getConstructor( XMultiServiceFactory.class );
        final DocumentTest test = (DocumentTest)ctor.newInstance( getORB() );
        System.out.println( "testing: " + test.getDocumentDescription() );
        m_currentDocument = test.getDocument();
        test.initializeDocument();
        test.verifyInitialDocumentState();

        final XUndoManager undoManager = getUndoManager( test.getDocument() );
        undoManager.clear();
        final UndoListener listener = new UndoListener();
        undoManager.addUndoManagerListener( listener );

        // do a single modification, undo it, and check the document state is as expected
        test.doSingleModification();
        undoManager.undo();
        test.verifyInitialDocumentState();
        // ensure the listener has been notified of the Undo operation
        assertEquals( "A simple undo does not result in the proper Undo count.", 1, listener.getUndoActionCount() );
        listener.reset();

        // do multiple changes in a row, after entering an Undo context
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
        listener.reset();
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

    private static final OfficeConnection m_connection = new OfficeConnection();
    private OfficeDocument m_currentDocument;
}
