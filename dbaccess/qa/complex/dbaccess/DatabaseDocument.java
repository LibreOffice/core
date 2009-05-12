/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DatabaseDocument.java,v $
 * $Revision: 1.1.2.9 $
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
package complex.dbaccess;

import com.sun.star.awt.XTopWindow;
import com.sun.star.beans.PropertyState;
import com.sun.star.document.DocumentEvent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.script.XStorageBasedLibraryContainer;
import com.sun.star.task.XInteractionRequest;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XStorable;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XSet;
import com.sun.star.document.XDocumentEventBroadcaster;
import com.sun.star.document.XDocumentEventListener;
import com.sun.star.document.XEmbeddedScripts;
import com.sun.star.document.XEventsSupplier;
import com.sun.star.document.XScriptInvocationContext;
import com.sun.star.frame.DoubleInitializationException;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XLoadable;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XModel2;
import com.sun.star.frame.XTitle;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.NotInitializedException;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.script.provider.XScriptProviderSupplier;
import com.sun.star.sdb.XFormDocumentsSupplier;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.XReportDocumentsSupplier;
import com.sun.star.task.DocumentMacroConfirmationRequest;
import com.sun.star.task.XInteractionApprove;
import com.sun.star.task.XInteractionContinuation;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.URL;
import com.sun.star.util.XChangesBatch;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XModifiable;
import com.sun.star.util.XURLTransformer;
import connectivity.tools.*;
import java.io.IOException;
import java.util.Iterator;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;

public class DatabaseDocument extends TestCase implements com.sun.star.document.XDocumentEventListener
{
    private XComponent  m_callbackFactory = null;
    private Vector      m_documentEvents = new Vector();
    private Vector      m_globalEvents = new Vector();

    // for those states, see testDocumentEvents
    private static short STATE_NOT_STARTED          = 0;
    private static short STATE_LOADING_DOC          = 1;
    private static short STATE_MACRO_EXEC_APPROVED  = 2;
    private static short STATE_ON_LOAD_RECEIVED     = 3;
    private short m_loadDocState = STATE_NOT_STARTED;

    // ========================================================================================================
    /** a helper class which can be used by the Basic scripts in our test documents
     *  to notify us of events in this document
     */
    private class CallbackComponent implements XDocumentEventListener, XTypeProvider
    {
        public void documentEventOccured( DocumentEvent _event )
        {
            onDocumentEvent( _event );
        }

        public void disposing( com.sun.star.lang.EventObject _Event )
        {
            // not interested in
        }

        public Type[] getTypes()
        {
            Class interfaces[] = getClass().getInterfaces();
            Type types[] = new Type[interfaces.length];
            for(int i = 0; i < interfaces.length; ++ i)
            {
                types[i] = new Type(interfaces[i]);
            }
            return types;
        }

        public byte[] getImplementationId()
        {
            return getClass().toString().getBytes();
        }
    };

    // ========================================================================================================
    private static String getCallbackComponentServiceName()
    {
        return "org.openoffice.complex.dbaccess.EventCallback";
    }

    // ========================================================================================================
    /** a factory for a CallbackComponent
     */
    private class CallbackComponentFactory implements XSingleComponentFactory, XServiceInfo, XComponent
    {
        private Vector  m_eventListeners = new Vector();

        public Object createInstanceWithContext( XComponentContext _context ) throws Exception
        {
            return new CallbackComponent();
        }

        public Object createInstanceWithArgumentsAndContext( Object[] arg0, XComponentContext _context ) throws Exception
        {
            return createInstanceWithContext( _context );
        }

        public String getImplementationName()
        {
            return "org.openoffice.complex.dbaccess.CallbackComponent";
        }

        public boolean supportsService( String _service )
        {
            return _service.equals( getCallbackComponentServiceName() );
        }

        public String[] getSupportedServiceNames()
        {
            return new String[] { getCallbackComponentServiceName() };
        }

        public void dispose()
        {
            EventObject event = new EventObject( this );

            Vector eventListenersCopy = (Vector)m_eventListeners.clone();
            Iterator iter = eventListenersCopy.iterator();
            while ( iter.hasNext() )
            {
                ((XEventListener)iter.next()).disposing( event );
            }
        }

        public void addEventListener( XEventListener _listener )
        {
            if ( _listener != null )
                m_eventListeners.add( _listener );
        }

        public void removeEventListener( XEventListener _listener )
        {
            m_eventListeners.remove( _listener );
        }
    };

    // ========================================================================================================
    private class MacroExecutionApprove implements XInteractionHandler
    {
        private XInteractionHandler m_defaultHandler = null;

        MacroExecutionApprove( XMultiServiceFactory _factory )
        {
            try
            {
                m_defaultHandler = (XInteractionHandler)UnoRuntime.queryInterface( XInteractionHandler.class,
                    _factory.createInstance( "com.sun.star.sdb.InteractionHandler" ) );
            }
            catch ( Exception ex )
            {
                Logger.getLogger( DatabaseDocument.class.getName() ).log( Level.SEVERE, null, ex );
            }
        }

        public void handle( XInteractionRequest _request )
        {
            Object request = _request.getRequest();
            if ( !( request instanceof DocumentMacroConfirmationRequest ) && ( m_defaultHandler != null ) )
            {
                m_defaultHandler.handle( _request );
                return;
            }

            assureEquals( "interaction handleer called in wrong state", STATE_LOADING_DOC, m_loadDocState );

            // auto-approve
            XInteractionContinuation continuations[] = _request.getContinuations();
            for ( int i=0; i<continuations.length; ++i )
            {
                XInteractionApprove approve = (XInteractionApprove)UnoRuntime.queryInterface( XInteractionApprove.class,
                    continuations[i] );
                if ( approve != null )
                {
                    approve.select();
                    m_loadDocState = STATE_MACRO_EXEC_APPROVED;
                    break;
                }
            }
        }
    };

    // ========================================================================================================
    // --------------------------------------------------------------------------------------------------------
    public String[] getTestMethodNames()
    {
        return new String[]
        {
            "testLoadable",
            "testDocumentEvents",
            "testGlobalEvents"
        };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return "DatabaseDocument";
    }

    public void before()
    {
        super.before();

        try
        {
            // at our service factory, insert a new factory for our CallbackComponent
            // this will allow the Basic code in our test documents to call back into this test case
            // here, by just instantiating this service
            XSet globalFactory = (XSet)UnoRuntime.queryInterface(
                XSet.class, getORB() );
            m_callbackFactory = new CallbackComponentFactory();
            globalFactory.insert( m_callbackFactory );

            // register ourself as listener at the global event broadcaster
            XDocumentEventBroadcaster broadcaster = (XDocumentEventBroadcaster)UnoRuntime.queryInterface(
                XDocumentEventBroadcaster.class, getORB().createInstance( "com.sun.star.frame.GlobalEventBroadcaster" ) );
            broadcaster.addDocumentEventListener( this );
        }
        catch( Exception e )
        {
            System.err.println( "could not create the test case, error message:\n" + e.getMessage() );
            e.printStackTrace( System.err );
            failed( "failed to create the test case" );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    public void after()
    {
        super.after();

        try
        {
            // dispose our callback factory. This will automatically remove it from our service
            // factory
            m_callbackFactory.dispose();

            // revoke ourself as listener at the global event broadcaster
            XDocumentEventBroadcaster broadcaster = (XDocumentEventBroadcaster) UnoRuntime.queryInterface(
                XDocumentEventBroadcaster.class, getORB().createInstance( "com.sun.star.frame.GlobalEventBroadcaster" ) );
            broadcaster.removeDocumentEventListener( this );
        }
        catch ( Exception e )
        {
            System.err.println( "could not close the test case, error message:\n" + e.getMessage() );
            e.printStackTrace( System.err );
            failed( "failed to close the test case" );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private class UnoMethodDescriptor
    {
        public Class    unoInterfaceClass = null;
        public String   methodName = null;

        public UnoMethodDescriptor( Class _class, String _method )
        {
            unoInterfaceClass = _class;
            methodName = _method;
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private void impl_checkDocumentInitState( Object _document, boolean _isInitialized )
    {
        // things you cannot do with an uninitialized document:
        UnoMethodDescriptor[] unsupportedMethods = new UnoMethodDescriptor[] {
            new UnoMethodDescriptor( XStorable.class, "store" ),
            new UnoMethodDescriptor( XFormDocumentsSupplier.class, "getFormDocuments" ),
            new UnoMethodDescriptor( XReportDocumentsSupplier.class, "getReportDocuments" ),
            new UnoMethodDescriptor( XScriptProviderSupplier.class, "getScriptProvider" ),
            new UnoMethodDescriptor( XEventsSupplier.class, "getEvents" ),
            new UnoMethodDescriptor( XTitle.class, "getTitle" ),
            new UnoMethodDescriptor( XModel2.class, "getControllers" )
            // (there's much more than this, but we cannot list all methods here, can we ...)
        };

        for ( int i=0; i<unsupportedMethods.length; ++i)
        {
            verifyExpectedException( _document, unsupportedMethods[i].unoInterfaceClass,
                unsupportedMethods[i].methodName, new Object[]{}, _isInitialized ? null : NotInitializedException.class );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private XModel impl_createDocument( ) throws Exception
    {
        XModel databaseDoc = (XModel)UnoRuntime.queryInterface( XModel.class,
            getORB().createInstance( "com.sun.star.sdb.OfficeDatabaseDocument" ) );

        // should not be initialized here - we did neither initNew nor load nor storeAsURL it
        impl_checkDocumentInitState( databaseDoc, false );

        return databaseDoc;
    }

    // --------------------------------------------------------------------------------------------------------
    private void impl_closeDocument( XModel _databaseDoc ) throws CloseVetoException, IOException, Exception
    {
        XCloseable closeDoc = (XCloseable)UnoRuntime.queryInterface( XCloseable.class,
            _databaseDoc );
        closeDoc.close( true );
    }

    // --------------------------------------------------------------------------------------------------------
    private XModel impl_createEmptyEmbeddedHSQLDocument() throws Exception, IOException
    {
        XModel databaseDoc = (XModel)UnoRuntime.queryInterface( XModel.class,
            getORB().createInstance( "com.sun.star.sdb.OfficeDatabaseDocument" ) );
        XStorable storeDoc = (XStorable)UnoRuntime.queryInterface( XStorable.class, databaseDoc );

        // verify the document rejects API calls which require it to be initialized
        impl_checkDocumentInitState( databaseDoc, false );

        // though the document is not initialized, you can ask for the location, the URL, and the args
        String location = storeDoc.getLocation();
        String url = databaseDoc.getURL();
        PropertyValue[] args = databaseDoc.getArgs();
        // they should be all empty at this time
        assureEquals( "location is expected to be empty here", "", location );
        assureEquals( "URL is expected to be empty here", "", url );
        assureEquals( "Args are expected to be empty here", 0, args.length );

        // and, you should be able to set properties at the data source
        XOfficeDatabaseDocument dataSourceAccess = (XOfficeDatabaseDocument)UnoRuntime.queryInterface(
            XOfficeDatabaseDocument.class, databaseDoc );
        XPropertySet dsProperties = (XPropertySet)UnoRuntime.queryInterface(
            XPropertySet.class, dataSourceAccess.getDataSource() );
        dsProperties.setPropertyValue( "URL", "sdbc:embedded:hsqldb" );

        String documentURL = createTempFileURL();
        storeDoc.storeAsURL( documentURL, new PropertyValue[0] );

        // now that the document is stored, ...
        // ... its URL should be correct
        assureEquals( "wrong URL after storing the document", documentURL, databaseDoc.getURL() );
        // ... it should be initialized
        impl_checkDocumentInitState( databaseDoc, true );

        return databaseDoc;
    }

    // --------------------------------------------------------------------------------------------------------
    public void testLoadable() throws Exception, IOException
    {
        XModel databaseDoc = impl_createEmptyEmbeddedHSQLDocument();
        String documentURL = databaseDoc.getURL();

        // there's three methods how you can initialize a database document:

        // ....................................................................
        // 1. XStorable::storeAsURL
        //      (this is for compatibility reasons, to not break existing code)
        // this test is already made in impl_createEmptyEmbeddedHSQLDocument

        // ....................................................................
        // 2. XLoadable::load
        databaseDoc = (XModel)UnoRuntime.queryInterface( XModel.class,
            getORB().createInstance( "com.sun.star.sdb.OfficeDatabaseDocument" ) );
        documentURL = copyToTempFile( documentURL );
        // load the doc, and verify it's initialized then, and has the proper URL
        XLoadable loadDoc = (XLoadable)UnoRuntime.queryInterface( XLoadable.class, databaseDoc );
        loadDoc.load( new PropertyValue[] { new PropertyValue( "URL", 0, documentURL, PropertyState.DIRECT_VALUE ) } );
        databaseDoc.attachResource( documentURL, new PropertyValue[0] );

        assureEquals( "wrong URL after loading the document", documentURL, databaseDoc.getURL() );
        impl_checkDocumentInitState( databaseDoc, true );

        // and while we are here ... initilizing the same document again should not be possible
        verifyExpectedException( databaseDoc, XLoadable.class, "initNew", new Object[0],
            DoubleInitializationException.class );
        verifyExpectedException( databaseDoc, XLoadable.class, "load", new Object[] { new PropertyValue[0] },
            DoubleInitializationException.class );

        // ....................................................................
        // 3. XLoadable::initNew
        impl_closeDocument( databaseDoc );
        databaseDoc = impl_createDocument();
        loadDoc = (XLoadable)UnoRuntime.queryInterface( XLoadable.class, databaseDoc );
        loadDoc.initNew();
        assureEquals( "wrong URL after initializing the document", "", databaseDoc.getURL() );
        impl_checkDocumentInitState( databaseDoc, true );

        // same as above - initializing the document a second time must fail
        verifyExpectedException( databaseDoc, XLoadable.class, "initNew", new Object[0],
            DoubleInitializationException.class );
        verifyExpectedException( databaseDoc, XLoadable.class, "load", new Object[] { new PropertyValue[0] },
            DoubleInitializationException.class );
    }

    // --------------------------------------------------------------------------------------------------------
    private PropertyValue[] impl_getDefaultLoadArgs()
    {
        return new PropertyValue[] {
            new PropertyValue( "PickListEntry", 0, false, PropertyState.DIRECT_VALUE )
        };
    }

    // --------------------------------------------------------------------------------------------------------
    private PropertyValue[] impl_getMacroExecLoadArgs()
    {
        return new PropertyValue[] {
            new PropertyValue( "PickListEntry", 0, false, PropertyState.DIRECT_VALUE ),
            new PropertyValue( "MacroExecutionMode", 0, com.sun.star.document.MacroExecMode.USE_CONFIG, PropertyState.DIRECT_VALUE ),
            new PropertyValue( "InteractionHandler", 0, new MacroExecutionApprove(( getORB() )), PropertyState.DIRECT_VALUE )
        };
    }

    // --------------------------------------------------------------------------------------------------------
    private int impl_setMacroSecurityLevel( int _level ) throws Exception
    {
        XMultiServiceFactory configProvider = (XMultiServiceFactory)UnoRuntime.queryInterface( XMultiServiceFactory.class,
            getORB().createInstance( "com.sun.star.configuration.ConfigurationProvider" ) );

        PropertyValue[] args = new PropertyValue[] {
            new PropertyValue( "nodepath", 0, "/org.openoffice.Office.Common/Security/Scripting", PropertyState.DIRECT_VALUE )
        };

        XPropertySet securitySettings = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class,
            configProvider.createInstanceWithArguments( "com.sun.star.configuration.ConfigurationUpdateAccess", args ) );
        int oldValue = ((Integer)securitySettings.getPropertyValue( "MacroSecurityLevel" )).intValue();
        securitySettings.setPropertyValue( "MacroSecurityLevel", new Integer( _level ) );

        XChangesBatch committer = (XChangesBatch)UnoRuntime.queryInterface( XChangesBatch.class,
            securitySettings );
        committer.commitChanges();

        return oldValue;
    }

    // --------------------------------------------------------------------------------------------------------
    public void testDocumentEvents() throws Exception, IOException
    {
        // create an empty document
        XModel databaseDoc = impl_createEmptyEmbeddedHSQLDocument();

        // create Basic library/module therein
        XEmbeddedScripts embeddedScripts = (XEmbeddedScripts) UnoRuntime.queryInterface( XEmbeddedScripts.class,
            databaseDoc );
        XStorageBasedLibraryContainer basicLibs = embeddedScripts.getBasicLibraries();
        XNameContainer newLib = basicLibs.createLibrary( "EventHandlers" );
        String eventHandlerCode =
            "Option Explicit\n" +
            "\n" +
            "Sub OnLoad\n" +
            "  Dim oCallback as Object\n" +
            "  oCallback = createUnoService( \"" + getCallbackComponentServiceName() + "\" )\n" +
            "\n" +
            "  ' as long as the Document is not passed to the Basic callbacks, we need to create\n" +
            "  ' one ourself\n" +
            "  Dim oEvent as new com.sun.star.document.DocumentEvent\n" +
            "  oEvent.EventName = \"OnLoad\"\n" +
            "  oEvent.Source = ThisComponent\n" +
            "\n" +
            "  oCallback.documentEventOccured( oEvent )\n" +
            "End Sub\n";
        newLib.insertByName( "all", eventHandlerCode );

        // bind the macro to the OnLoad event
        String macroURI = "vnd.sun.star.script:EventHandlers.all.OnLoad?language=Basic&location=document";
        XEventsSupplier eventsSupplier = (XEventsSupplier)UnoRuntime.queryInterface( XEventsSupplier.class,
            databaseDoc );
        eventsSupplier.getEvents().replaceByName( "OnLoad", new PropertyValue[] {
            new PropertyValue( "EventType", 0, "Script", PropertyState.DIRECT_VALUE ),
            new PropertyValue( "Script", 0, macroURI, PropertyState.DIRECT_VALUE )
        } );

        // store the document, and close it
        String documentURL = databaseDoc.getURL();
        documentURL = FileHelper.getOOoCompatibleFileURL( documentURL );
        XStorable storeDoc = (XStorable) UnoRuntime.queryInterface( XStorable.class,
            databaseDoc );
        storeDoc.store();
        impl_closeDocument( databaseDoc );

        // ensure the macro security configuration is "ask the user for document macro execution"
        int oldSecurityLevel = impl_setMacroSecurityLevel( 1 );

        // load it, again
        XComponentLoader loader = (XComponentLoader)UnoRuntime.queryInterface( XComponentLoader.class,
            getORB().createInstance( "com.sun.star.frame.Desktop" ) );

        m_loadDocState = STATE_LOADING_DOC;
        // expected order of states is:
        // STATE_LOADING_DOC - initialized here
        // STATE_MACRO_EXEC_APPROVED - done in our interaction handler, which auto-approves the execution of macros
        // STATE_ON_LOAD_RECEIVED - done in our callback for the document events
        //
        // In particular, it is important that the interaction handler (which plays the role of the user confirmation
        // here) is called before the OnLoad notification is received - since the latter happens from within
        // a Basic macro which is bound to the OnLoad event of the document.

        String context = "OnLoad";
        impl_startObservingEvents( context );
        databaseDoc = (XModel)UnoRuntime.queryInterface( XModel.class,
            loader.loadComponentFromURL( documentURL, "_blank", 0, impl_getMacroExecLoadArgs() ) );
        impl_stopObservingEvents( m_documentEvents, new String[] { "OnLoad" }, context );

        assureEquals( "our provided interaction handler was not called", STATE_ON_LOAD_RECEIVED, m_loadDocState );

        // restore macro security level
        impl_setMacroSecurityLevel( oldSecurityLevel );

        // close the document
        impl_closeDocument( databaseDoc );
    }

    // --------------------------------------------------------------------------------------------------------
    public void testGlobalEvents() throws Exception, IOException
    {
        XModel databaseDoc = impl_createEmptyEmbeddedHSQLDocument();
        XStorable storeDoc = (XStorable) UnoRuntime.queryInterface( XStorable.class,
            databaseDoc );

        String oldURL = null, newURL = null, context = null;

        // XStorable.store
        oldURL = databaseDoc.getURL();
        context = "store";
        impl_startObservingEvents( context );
        storeDoc.store();
        assureEquals( "store is not expected to change the document URL", databaseDoc.getURL(), oldURL );
        impl_stopObservingEvents( m_globalEvents, new String[] { "OnSave", "OnSaveDone" }, context );

        // XStorable.storeToURL
        context = "storeToURL";
        impl_startObservingEvents( context );
        storeDoc.storeToURL( createTempFileURL(), new PropertyValue[0] );
        assureEquals( "storetoURL is not expected to change the document URL", databaseDoc.getURL(), oldURL );
        impl_stopObservingEvents( m_globalEvents, new String[] { "OnSaveTo", "OnSaveToDone" }, context );

        // XStorable.storeAsURL
        newURL = createTempFileURL();
        context = "storeAsURL";
        impl_startObservingEvents( context );
        storeDoc.storeAsURL( newURL, new PropertyValue[0] );
        assureEquals( "storeAsURL is expected to change the document URL", databaseDoc.getURL(), newURL );
        impl_stopObservingEvents( m_globalEvents, new String[] { "OnSaveAs", "OnSaveAsDone" }, context );

        // XModifiable.setModified
        XModifiable modifyDoc = (XModifiable) UnoRuntime.queryInterface( XModifiable.class,
            databaseDoc );
        context = "setModified";
        impl_startObservingEvents( context );
        modifyDoc.setModified( true );
        assureEquals( "setModified didn't work", modifyDoc.isModified(), true );
        impl_stopObservingEvents( m_globalEvents, new String[] { "OnModifyChanged" }, context );

        // XStorable.store, with implicit reset of the "Modified" flag
        context = "store (2)";
        impl_startObservingEvents( context );
        storeDoc.store();
        assureEquals( "'store' should implicitly reset the modified flag", modifyDoc.isModified(), false );
        impl_stopObservingEvents( m_globalEvents, new String[] { "OnSave", "OnSaveDone", "OnModifyChanged" }, context );

        // XComponentLoader.loadComponentFromURL
        newURL = copyToTempFile( databaseDoc.getURL() );
        XComponentLoader loader = (XComponentLoader)UnoRuntime.queryInterface( XComponentLoader.class,
            getORB().createInstance( "com.sun.star.frame.Desktop" ) );
        context = "loadComponentFromURL";
        impl_startObservingEvents( context );
        databaseDoc = (XModel) UnoRuntime.queryInterface( XModel.class,
            loader.loadComponentFromURL( newURL, "_blank", 0, impl_getDefaultLoadArgs() ) );
        impl_stopObservingEvents( m_globalEvents,
            new String[] { "OnLoadFinished", "OnViewCreated", "OnFocus", "OnLoad" }, context );

        // closing a document by API
        XCloseable closeDoc = (XCloseable) UnoRuntime.queryInterface( XCloseable.class,
            databaseDoc );
        context = "close (API)";
        impl_startObservingEvents( context );
        closeDoc.close( true );
        impl_stopObservingEvents( m_globalEvents,
            new String[] { "OnPrepareUnload", "OnViewClosed", "OnUnload" }, context );

        // closing a document via UI
        context = "close (UI)";
        impl_startObservingEvents( "prepare for '" + context + "'" );
        databaseDoc = (XModel)UnoRuntime.queryInterface( XModel.class,
            loader.loadComponentFromURL( newURL, "_blank", 0, impl_getDefaultLoadArgs() ) );
        impl_waitForEvent( m_globalEvents, "OnLoad", 5000 );
            // wait for all events to arrive - OnLoad should be the last one

        XDispatchProvider dispatchProvider = (XDispatchProvider) UnoRuntime.queryInterface( XDispatchProvider.class,
            databaseDoc.getCurrentController().getFrame() );
        URL url = impl_getURL( ".uno:CloseDoc" );
        XDispatch dispatcher = dispatchProvider.queryDispatch( url, "", 0 );
        impl_startObservingEvents( context );
        dispatcher.dispatch( url, new PropertyValue[0] );
        impl_stopObservingEvents( m_globalEvents,
            new String[] { "OnPrepareViewClosing", "OnViewClosed", "OnPrepareUnload", "OnUnload" }, context );

        // creating a new document
        databaseDoc = impl_createDocument();
        XLoadable loadDoc = (XLoadable) UnoRuntime.queryInterface( XLoadable.class,
            databaseDoc );
        context = "initNew";
        impl_startObservingEvents( context );
        loadDoc.initNew();
        impl_stopObservingEvents( m_globalEvents, new String[] { "OnCreate" }, context );

        impl_startObservingEvents( context + " (cleanup)" );
        impl_closeDocument( databaseDoc );
        impl_waitForEvent( m_globalEvents, "OnUnload", 5000 );

        // focus changes
        context = "activation";
        // for this, load a database document ...
        impl_startObservingEvents( "prepare for '" + context + "'" );
        databaseDoc = (XModel)UnoRuntime.queryInterface( XModel.class,
            loader.loadComponentFromURL( newURL, "_blank", 0, impl_getDefaultLoadArgs() ) );
        int previousOnLoadEventPos = impl_waitForEvent( m_globalEvents, "OnLoad", 5000 );
        // ... and another document ...
        String otherURL = copyToTempFile( databaseDoc.getURL() );
        XModel otherDoc = (XModel)UnoRuntime.queryInterface( XModel.class,
            loader.loadComponentFromURL( otherURL, "_blank", 0, impl_getDefaultLoadArgs() ) );
        impl_waitForEvent( m_globalEvents, "OnLoad", 5000, previousOnLoadEventPos + 1 );
        impl_raise( otherDoc );

        // ... and switch between the two
        impl_startObservingEvents( context );
        impl_raise( databaseDoc );
        impl_stopObservingEvents( m_globalEvents, new String[] { "OnUnfocus", "OnFocus" }, context );

        // cleanup
        impl_startObservingEvents( "cleanup after '" + context + "'" );
        impl_closeDocument( databaseDoc );
        impl_closeDocument( otherDoc );
    }

    // --------------------------------------------------------------------------------------------------------
    private URL impl_getURL( String _completeURL ) throws Exception
    {
        URL[] url = { new URL() };
        url[0].Complete = _completeURL;
        XURLTransformer urlTransformer = (XURLTransformer) UnoRuntime.queryInterface( XURLTransformer.class,
            getORB().createInstance( "com.sun.star.util.URLTransformer" ) );
        urlTransformer.parseStrict( url );
        return url[0];
    }

    // --------------------------------------------------------------------------------------------------------
    private void impl_raise( XModel _document )
    {
        XFrame frame = _document.getCurrentController().getFrame();
        XTopWindow topWindow = (XTopWindow) UnoRuntime.queryInterface( XTopWindow.class,
            frame.getContainerWindow() );
        topWindow.toFront();
    }

    // --------------------------------------------------------------------------------------------------------
    private void impl_startObservingEvents( String _context )
    {
        log.println( " " + _context );
        synchronized ( m_documentEvents )
        {
            m_documentEvents.clear();
        }
        synchronized ( m_globalEvents )
        {
            m_globalEvents.clear();
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private void impl_stopObservingEvents( Vector _actualEvents, String[] _expectedEvents, String _context )
    {
        synchronized ( _actualEvents )
        {
            int actualEventCount = _actualEvents.size();
            while ( actualEventCount < _expectedEvents.length )
            {
                // well, it's possible not all events already arrived, yet - finally, some of them
                // are notified asynchronously
                // So, wait a few seconds.
                try
                {
                    _actualEvents.wait( 5000 );
                }
                catch ( InterruptedException ex ) {  }

                if ( actualEventCount == _actualEvents.size() )
                    // the above wait was left because of the timeout, *not* because an event
                    // arrived. Okay, we won't wait any longer, this is a failure.
                    break;
                actualEventCount = _actualEvents.size();
            }

            assureEquals( "wrong event count for '" + _context + "'",
                _expectedEvents.length, _actualEvents.size() );

            for ( int i=0; i<_expectedEvents.length; ++i )
            {
                assureEquals( "wrong event at positon " + ( i + 1 ) + " for '" + _context + "'",
                    _expectedEvents[i], _actualEvents.get(i) );
            }
        }
    }

    // --------------------------------------------------------------------------------------------------------
    int impl_waitForEvent( Vector _eventQueue, String _expectedEvent, int _maxMilliseconds )
    {
        return impl_waitForEvent( _eventQueue, _expectedEvent, _maxMilliseconds, 0 );
    }

    // --------------------------------------------------------------------------------------------------------
    int impl_waitForEvent( Vector _eventQueue, String _expectedEvent, int _maxMilliseconds, int _firstQueueElementToCheck )
    {
        synchronized ( _eventQueue )
        {
            int waitedMilliseconds = 0;

            while ( waitedMilliseconds < _maxMilliseconds )
            {
                for ( int i=_firstQueueElementToCheck; i<_eventQueue.size(); ++i )
                {
                    if ( _expectedEvent.equals( _eventQueue.get(i) ) )
                        // found the event in the queue
                        return i;
                }

                // wait a little, perhaps the event will still arrive
                try
                {
                    _eventQueue.wait( 500 );
                    waitedMilliseconds += 500;
                }
                catch ( InterruptedException e ) { }
            }
        }

        failed( "expected event '" + _expectedEvent + "' did not arrive after " + _maxMilliseconds + " milliseconds" );
        return -1;
    }

    // --------------------------------------------------------------------------------------------------------
    void onDocumentEvent( DocumentEvent _Event )
    {
        if ( _Event.EventName.equals( "OnTitleChanged" ) )
            // OnTitleChanged events are notified too often. This is known, and accepted.
            // (the deeper reason is that it's diffult to determine, in the DatabaseDocument implementatin,
            // when the title actually changed. In particular, when we do a saveAsURL, and then ask for a
            // title *before* the TitleHelper got the document's OnSaveAsDone event, then the wrong (old)
            // title is obtained.
            return;

        if  (   ( _Event.EventName.equals( "OnLoad" ) )
            &&  (   m_loadDocState != STATE_NOT_STARTED )
            )
        {
            assureEquals( "OnLoad event must come *after* invocation of the interaction handler / user!",
                m_loadDocState, STATE_MACRO_EXEC_APPROVED );
            m_loadDocState = STATE_ON_LOAD_RECEIVED;
        }

        synchronized ( m_documentEvents )
        {
            m_documentEvents.add( _Event.EventName );
            m_documentEvents.notifyAll();
        }

        log.println( "  document event: " + _Event.EventName );
    }

    // --------------------------------------------------------------------------------------------------------
    public void documentEventOccured( DocumentEvent _Event )
    {
        if ( _Event.EventName.equals( "OnTitleChanged" ) )
            // ignore. See onDocumentEvent for a justification
            return;

        synchronized ( m_globalEvents )
        {
            m_globalEvents.add( _Event.EventName );
            m_globalEvents.notifyAll();
        }

        log.println( "  global event: " + _Event.EventName );
    }

    // --------------------------------------------------------------------------------------------------------
    public void disposing( EventObject _Event )
    {
        // not interested in
    }
}
