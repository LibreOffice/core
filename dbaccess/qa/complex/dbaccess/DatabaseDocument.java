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
package complex.dbaccess;

import com.sun.star.configuration.theDefaultProvider;
import com.sun.star.lang.NotInitializedException;
import com.sun.star.frame.DoubleInitializationException;
import com.sun.star.awt.XTopWindow;
import com.sun.star.beans.PropertyState;
import com.sun.star.document.DocumentEvent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.script.XStorageBasedLibraryContainer;
import com.sun.star.task.XInteractionRequest;

import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XStorable;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XSet;
import com.sun.star.document.XDocumentEventListener;
import com.sun.star.document.XEmbeddedScripts;
import com.sun.star.document.XEventsSupplier;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XGlobalEventBroadcaster;
import com.sun.star.frame.XLoadable;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XModel2;
import com.sun.star.frame.XTitle;
import com.sun.star.frame.theGlobalEventBroadcaster;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.script.provider.XScriptProviderSupplier;
import com.sun.star.sdb.XDocumentDataSource;

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
import java.io.IOException;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

// ---------- junit imports -----------------
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;


public class DatabaseDocument extends TestCase implements com.sun.star.document.XDocumentEventListener
{

    private static final String _BLANK = "_blank";
    private XComponent m_callbackFactory = null;
    private final ArrayList<String> m_documentEvents = new ArrayList<String>();
    private final ArrayList<String> m_globalEvents = new ArrayList<String>();
    // for those states, see testDocumentEvents
    private static short STATE_NOT_STARTED = 0;
    private static short STATE_LOADING_DOC = 1;
    private static short STATE_MACRO_EXEC_APPROVED = 2;
    private static short STATE_ON_LOAD_RECEIVED = 3;
    private short m_loadDocState = STATE_NOT_STARTED;


    /** a helper class which can be used by the Basic scripts in our test documents
     *  to notify us of events in this document
     */
    private class CallbackComponent implements XDocumentEventListener, XTypeProvider
    {

        public void documentEventOccured(DocumentEvent _event)
        {
            onDocumentEvent(_event);
        }

        public void disposing(com.sun.star.lang.EventObject _Event)
        {
            // not interested in
        }

        public Type[] getTypes()
        {
            final Class interfaces[] = getClass().getInterfaces();
            Type types[] = new Type[interfaces.length];
            for (int i = 0; i < interfaces.length; ++i)
            {
                types[i] = new Type(interfaces[i]);
            }
            return types;
        }

        public byte[] getImplementationId()
        {
            return new byte[0];
        }
    }


    private static String getCallbackComponentServiceName()
    {
        return "org.openoffice.complex.dbaccess.EventCallback";
    }


    /** a factory for a CallbackComponent
     */
    private class CallbackComponentFactory implements XSingleComponentFactory, XServiceInfo, XComponent
    {

        private final ArrayList<XEventListener> m_eventListeners = new ArrayList<XEventListener>();

        public Object createInstanceWithContext(XComponentContext _context) throws com.sun.star.uno.Exception
        {
            return new CallbackComponent();
        }

        public Object createInstanceWithArgumentsAndContext(Object[] arg0, XComponentContext _context) throws com.sun.star.uno.Exception
        {
            return createInstanceWithContext(_context);
        }

        public String getImplementationName()
        {
            return "org.openoffice.complex.dbaccess.CallbackComponent";
        }

        public boolean supportsService(String _service)
        {
            return _service.equals(getCallbackComponentServiceName());
        }

        public String[] getSupportedServiceNames()
        {
            return new String[]
                    {
                        getCallbackComponentServiceName()
                    };
        }

        public void dispose()
        {
            final EventObject event = new EventObject(this);

            final ArrayList<XEventListener> eventListenersCopy = (ArrayList<XEventListener>)m_eventListeners.clone();
            final Iterator<XEventListener> iter = eventListenersCopy.iterator();
            while (iter.hasNext())
            {
                iter.next().disposing(event);
            }
        }

        public void addEventListener(XEventListener _listener)
        {
            if (_listener != null)
            {
                m_eventListeners.add(_listener);
            }
        }

        public void removeEventListener(XEventListener _listener)
        {
            m_eventListeners.remove(_listener);
        }
    }


    private class MacroExecutionApprove implements XInteractionHandler
    {

        private XInteractionHandler m_defaultHandler = null;

        MacroExecutionApprove(XMultiServiceFactory _factory)
        {
            try
            {
                m_defaultHandler = UnoRuntime.queryInterface(XInteractionHandler.class, _factory.createInstance("com.sun.star.task.InteractionHandler"));
            }
            catch (Exception ex)
            {
                Logger.getLogger(DatabaseDocument.class.getName()).log(Level.SEVERE, null, ex);
            }
        }

        public void handle(XInteractionRequest _request)
        {
            final Object request = _request.getRequest();
            if (!(request instanceof DocumentMacroConfirmationRequest) && (m_defaultHandler != null))
            {
                m_defaultHandler.handle(_request);
                return;
            }

            assertEquals("interaction handler called in wrong state", STATE_LOADING_DOC, m_loadDocState);

            // auto-approve
            final XInteractionContinuation continuations[] = _request.getContinuations();
            for (int i = 0; i < continuations.length; ++i)
            {
                final XInteractionApprove approve = UnoRuntime.queryInterface(XInteractionApprove.class, continuations[i]);
                if (approve != null)
                {
                    approve.select();
                    m_loadDocState = STATE_MACRO_EXEC_APPROVED;
                    break;
                }
            }
        }
    }



    @Override
    @Before
    public void before() throws java.lang.Exception
    {
        super.before();

        try
        {
            // at our service factory, insert a new factory for our CallbackComponent
            // this will allow the Basic code in our test documents to call back into this test case
            // here, by just instantiating this service
            final XSet globalFactory = UnoRuntime.queryInterface(XSet.class, getMSF());
            m_callbackFactory = new CallbackComponentFactory();
            globalFactory.insert(m_callbackFactory);

            // register ourself as listener at the global event broadcaster
            final XGlobalEventBroadcaster broadcaster
                = theGlobalEventBroadcaster.get(getComponentContext());
            broadcaster.addDocumentEventListener(this);
        }
        catch (Exception e)
        {
            System.out.println("could not create the test case, error message:\n" + e.getMessage());
            e.printStackTrace(System.err);
            fail("failed to create the test case");
        }
    }


    @Override
    @After
    public void after() throws java.lang.Exception
    {
        try
        {
            // dispose our callback factory. This will automatically remove it from our service
            // factory
            m_callbackFactory.dispose();

            // revoke ourself as listener at the global event broadcaster
            final XGlobalEventBroadcaster broadcaster
                = theGlobalEventBroadcaster.get(getComponentContext());
            broadcaster.removeDocumentEventListener(this);
        }
        catch (Exception e)
        {
            System.out.println("could not create the test case, error message:\n" + e.getMessage());
            e.printStackTrace(System.err);
            fail("failed to close the test case");
        }

        super.after();
    }


    private static class UnoMethodDescriptor
    {

        public Class unoInterfaceClass = null;
        public String methodName = null;

        UnoMethodDescriptor(Class _class, String _method)
        {
            unoInterfaceClass = _class;
            methodName = _method;
        }
    }


    private void impl_checkDocumentInitState(Object _document, boolean _isInitialized)
    {
        // things you cannot do with an uninitialized document:
        final UnoMethodDescriptor[] unsupportedMethods = new UnoMethodDescriptor[]
        {
            new UnoMethodDescriptor(XStorable.class, "store"),
            new UnoMethodDescriptor(XFormDocumentsSupplier.class, "getFormDocuments"),
            new UnoMethodDescriptor(XReportDocumentsSupplier.class, "getReportDocuments"),
            new UnoMethodDescriptor(XScriptProviderSupplier.class, "getScriptProvider"),
            new UnoMethodDescriptor(XEventsSupplier.class, "getEvents"),
            new UnoMethodDescriptor(XTitle.class, "getTitle"),
            new UnoMethodDescriptor(XModel2.class, "getControllers")
        // (there's much more than this, but we cannot list all methods here, can we ...)
        };

        for (int i = 0; i < unsupportedMethods.length; ++i)
        {
            assureException( _document, unsupportedMethods[i].unoInterfaceClass,
                unsupportedMethods[i].methodName, new Object[]{}, _isInitialized ? null : NotInitializedException.class );
        }
    }


    private XModel impl_createDocument() throws Exception
    {
        final XModel databaseDoc = UnoRuntime.queryInterface(XModel.class, getMSF().createInstance("com.sun.star.sdb.OfficeDatabaseDocument"));

        // should not be initialized here - we did neither initNew nor load nor storeAsURL it
        impl_checkDocumentInitState(databaseDoc, false);

        return databaseDoc;
    }


    private void impl_closeDocument(XModel _databaseDoc) throws CloseVetoException, Exception
    {
        final XCloseable closeDoc = UnoRuntime.queryInterface(XCloseable.class, _databaseDoc);
        closeDoc.close(true);
    }


    private XModel impl_createEmptyEmbeddedHSQLDocument() throws Exception, IOException
    {
        final XModel databaseDoc = UnoRuntime.queryInterface(XModel.class, getMSF().createInstance("com.sun.star.sdb.OfficeDatabaseDocument"));
        final XStorable storeDoc = UnoRuntime.queryInterface(XStorable.class, databaseDoc);

        // verify the document rejects API calls which require it to be initialized
        impl_checkDocumentInitState(databaseDoc, false);

        // though the document is not initialized, you can ask for the location, the URL, and the args
        final String location = storeDoc.getLocation();
        final String url = databaseDoc.getURL();
        final PropertyValue[] args = databaseDoc.getArgs();
        // they should be all empty at this time
        assertEquals("location is expected to be empty here", "", location);
        assertEquals("URL is expected to be empty here", "", url);
        assertEquals("Args are expected to be empty here", 0, args.length);

        // and, you should be able to set properties at the data source
        final XOfficeDatabaseDocument dataSourceAccess = UnoRuntime.queryInterface(XOfficeDatabaseDocument.class, databaseDoc);
        final XPropertySet dsProperties = UnoRuntime.queryInterface(XPropertySet.class, dataSourceAccess.getDataSource());
        dsProperties.setPropertyValue("URL", "sdbc:embedded:hsqldb");

        final String documentURL = createTempFileURL();
        storeDoc.storeAsURL(documentURL, new PropertyValue[0]);

        // now that the document is stored, ...
        // ... its URL should be correct
        assertEquals("wrong URL after storing the document", documentURL, databaseDoc.getURL());
        // ... it should be initialized
        impl_checkDocumentInitState(databaseDoc, true);

        return databaseDoc;
    }


    @Test
    public void testLoadable() throws Exception, IOException
    {
        XModel databaseDoc = impl_createEmptyEmbeddedHSQLDocument();
        String documentURL = databaseDoc.getURL();

        // there's three methods how you can initialize a database document:


        // 1. XStorable::storeAsURL
        //      (this is for compatibility reasons, to not break existing code)
        // this test is already made in impl_createEmptyEmbeddedHSQLDocument


        // 2. XLoadable::load
        databaseDoc = UnoRuntime.queryInterface(XModel.class, getMSF().createInstance("com.sun.star.sdb.OfficeDatabaseDocument"));
        documentURL = copyToTempFile(documentURL);
        // load the doc, and verify it's initialized then, and has the proper URL
        XLoadable loadDoc = UnoRuntime.queryInterface(XLoadable.class, databaseDoc);
        loadDoc.load(new PropertyValue[]
                {
                    new PropertyValue("URL", 0, documentURL, PropertyState.DIRECT_VALUE)
                });
        databaseDoc.attachResource(documentURL, new PropertyValue[0]);

        assertEquals("wrong URL after loading the document", documentURL, databaseDoc.getURL());
        impl_checkDocumentInitState(databaseDoc, true);

        // and while we are here ... initializing the same document again should not be possible
        assureException( databaseDoc, XLoadable.class, "initNew", new Object[0],
            DoubleInitializationException.class );
        assureException( databaseDoc, XLoadable.class, "load", new Object[] { new PropertyValue[0] },
            DoubleInitializationException.class );


        // 3. XLoadable::initNew
        impl_closeDocument(databaseDoc);
        databaseDoc = impl_createDocument();
        loadDoc = UnoRuntime.queryInterface(XLoadable.class, databaseDoc);
        loadDoc.initNew();
        assertEquals("wrong URL after initializing the document", "", databaseDoc.getURL());
        impl_checkDocumentInitState(databaseDoc, true);

        // same as above - initializing the document a second time must fail
        assureException( databaseDoc, XLoadable.class, "initNew", new Object[0],
            DoubleInitializationException.class );
        assureException( databaseDoc, XLoadable.class, "load", new Object[] { new PropertyValue[0] },
            DoubleInitializationException.class );
    }


    private PropertyValue[] impl_getMarkerLoadArgs()
    {
        return new PropertyValue[]
                {
                    new PropertyValue( "PickListEntry", 0, false, PropertyState.DIRECT_VALUE ),
                    new PropertyValue( "TestCase_Marker", 0, "Yes", PropertyState.DIRECT_VALUE )
                };
    }


    private boolean impl_hasMarker( final PropertyValue[] _args )
    {
        for ( int i=0; i<_args.length; ++i )
        {
            if ( _args[i].Name.equals( "TestCase_Marker" ) && _args[i].Value.equals( "Yes" ) )
            {
                return true;
            }
        }
        return false;
    }


    private PropertyValue[] impl_getDefaultLoadArgs()
    {
        return new PropertyValue[]
                {
                    new PropertyValue("PickListEntry", 0, false, PropertyState.DIRECT_VALUE)
                };
    }


    private PropertyValue[] impl_getMacroExecLoadArgs()
    {
        return new PropertyValue[]
                {
                    new PropertyValue("PickListEntry", 0, false, PropertyState.DIRECT_VALUE),
                    new PropertyValue("MacroExecutionMode", 0, com.sun.star.document.MacroExecMode.USE_CONFIG, PropertyState.DIRECT_VALUE),
                    new PropertyValue("InteractionHandler", 0, new MacroExecutionApprove(getMSF()), PropertyState.DIRECT_VALUE)
                };
    }


    private int impl_setMacroSecurityLevel(int _level) throws Exception
    {
        final XMultiServiceFactory configProvider = theDefaultProvider.get(
            getComponentContext());

        final NamedValue[] args = new NamedValue[]
        {
            new NamedValue("nodepath", "/org.openoffice.Office.Common/Security/Scripting")
        };

        final XPropertySet securitySettings = UnoRuntime.queryInterface(XPropertySet.class, configProvider.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess", args));
        final int oldValue = ((Integer) securitySettings.getPropertyValue("MacroSecurityLevel")).intValue();
        securitySettings.setPropertyValue("MacroSecurityLevel", Integer.valueOf(_level));

        final XChangesBatch committer = UnoRuntime.queryInterface(XChangesBatch.class, securitySettings);
        committer.commitChanges();

        return oldValue;
    }


    private XModel impl_loadDocument( final String _documentURL, final PropertyValue[] _loadArgs ) throws Exception
    {
        final XComponentLoader loader = UnoRuntime.queryInterface(XComponentLoader.class, getMSF().createInstance("com.sun.star.frame.Desktop"));
        return UnoRuntime.queryInterface(XModel.class, loader.loadComponentFromURL(_documentURL, _BLANK, 0, _loadArgs));
    }


    private void impl_storeDocument( final XModel _document ) throws Exception
    {
        // store the document
        FileHelper.getOOoCompatibleFileURL( _document.getURL() );
        final XStorable storeDoc = UnoRuntime.queryInterface(XStorable.class, _document);
        storeDoc.store();

    }


    private XModel impl_createDocWithMacro( final String _libName, final String _moduleName, final String _code ) throws Exception, IOException
    {
        // create an empty document
        XModel databaseDoc = impl_createEmptyEmbeddedHSQLDocument();

        // create Basic library/module therein
        final XEmbeddedScripts embeddedScripts = UnoRuntime.queryInterface(XEmbeddedScripts.class, databaseDoc);
        final XStorageBasedLibraryContainer basicLibs = embeddedScripts.getBasicLibraries();
        final XNameContainer newLib = basicLibs.createLibrary( _libName );
        newLib.insertByName( _moduleName, _code );

        return databaseDoc;
    }


    /** Tests various aspects of database document "revenants"
     *
     *  Well, I do not really have a good term for this... The point is, database documents are in real
     *  only *one* aspect of a more complex thing. The second aspect is a data source. Both, in some sense,
     *  just represent different views on the same thing. For a given database, there's at each time at most
     *  one data source, and at most one database document. Both have a independent life time, and are
     *  created when needed.
     *  In particular, a document can be closed (this is what happens when the last UI window displaying
     *  this document is closed), and then dies. Now when the other "view", the data source, still exists,
     *  the underlying document data is not discarded, but kept alive (else the data source would die
     *  just because the document dies, which is not desired). If the document is loaded, again, then
     *  it is re-created, using the data of its previous "incarnation".
     *
     *  This method here tests some of those aspects of a document which should survive the death of one
     *  instance and re-creation as a revenant.
    */
    @Test
    public void testDocumentRevenants() throws Exception, IOException
    {
        // create an empty document
        XModel databaseDoc = impl_createDocWithMacro( "Lib", "Module",
            "Sub Hello\n" +
            "    MsgBox \"Hello\"\n" +
            "End Sub\n"
        );
        impl_storeDocument( databaseDoc );
        final String documentURL = databaseDoc.getURL();

        // at this stage, the marker should not yet be present in the doc's args, else some of the below
        // tests become meaningless
        assertTrue( "A newly created doc should not have the test case marker", !impl_hasMarker( databaseDoc.getArgs() ) );

        // obtain the DataSource associated with the document. Keeping this alive
        // ensures that the "impl data" of the document is kept alive, too, so when closing
        // and re-opening it, this "impl data" must be re-used.
        XDocumentDataSource dataSource = UnoRuntime.queryInterface(XDocumentDataSource.class, UnoRuntime.queryInterface(XOfficeDatabaseDocument.class, databaseDoc).getDataSource());

        // close and reload the doc
        impl_closeDocument(databaseDoc);
        databaseDoc = impl_loadDocument( documentURL, impl_getMarkerLoadArgs() );
        // since we just put the marker into the load-call, it should be present at the doc
        assertTrue( "The test case marker got lost.", impl_hasMarker( databaseDoc.getArgs() ) );

        // The basic library should have survived
        final XEmbeddedScripts embeddedScripts = UnoRuntime.queryInterface(XEmbeddedScripts.class, databaseDoc);
        final XStorageBasedLibraryContainer basicLibs = embeddedScripts.getBasicLibraries();
        assertTrue( "Basic lib did not survive reloading a closed document", basicLibs.hasByName( "Lib" ) );
        final XNameContainer lib = UnoRuntime.queryInterface(XNameContainer.class, basicLibs.getByName("Lib"));
        assertTrue( "Basic module did not survive reloading a closed document", lib.hasByName( "Module" ) );

        // now closing the doc, and obtaining it from the data source, should preserve the marker we put into the load
        // args
        impl_closeDocument( databaseDoc );
        databaseDoc = UnoRuntime.queryInterface(XModel.class, dataSource.getDatabaseDocument());
        assertTrue( "The test case marker did not survive re-retrieval of the doc from the data source.",
            impl_hasMarker( databaseDoc.getArgs() ) );

        // on the other hand, closing and regularly re-loading the doc *without* the marker should indeed
        // lose it
        impl_closeDocument( databaseDoc );
        databaseDoc = impl_loadDocument( documentURL, impl_getDefaultLoadArgs() );
        assertTrue( "Reloading the document kept the old args, instead of the newly supplied ones.",
            !impl_hasMarker( databaseDoc.getArgs() ) );

        // clean up
        impl_closeDocument( databaseDoc );
    }


    @Test
    public void testDocumentEvents() throws Exception, IOException
    {
        // create an empty document
        final String libName = "EventHandlers";
        final String moduleName = "all";
        final String eventHandlerCode =
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
                "  oCallback.documentEventOccurred( oEvent )\n" +
                "End Sub\n";
        XModel databaseDoc = impl_createDocWithMacro( libName, moduleName, eventHandlerCode );
        final String documentURL = databaseDoc.getURL();

        // bind the macro to the OnLoad event
        final String macroURI = "vnd.sun.star.script:" + libName + "." + moduleName + ".OnLoad?language=Basic&location=document";
        final XEventsSupplier eventsSupplier = UnoRuntime.queryInterface(XEventsSupplier.class, databaseDoc);
        eventsSupplier.getEvents().replaceByName("OnLoad", new PropertyValue[]
                {
                    new PropertyValue("EventType", 0, "Script", PropertyState.DIRECT_VALUE),
                    new PropertyValue("Script", 0, macroURI, PropertyState.DIRECT_VALUE)
                });

        // store the document, and close it
        impl_storeDocument( databaseDoc );
        impl_closeDocument( databaseDoc );

        // ensure the macro security configuration is "ask the user for document macro execution"
        final int oldSecurityLevel = impl_setMacroSecurityLevel(1);

        // load it, again
        m_loadDocState = STATE_LOADING_DOC;
        // expected order of states is:
        // STATE_LOADING_DOC - initialized here
        // STATE_MACRO_EXEC_APPROVED - done in our interaction handler, which auto-approves the execution of macros
        // STATE_ON_LOAD_RECEIVED - done in our callback for the document events
        //
        // In particular, it is important that the interaction handler (which plays the role of the user confirmation
        // here) is called before the OnLoad notification is received - since the latter happens from within
        // a Basic macro which is bound to the OnLoad event of the document.

        final String context = "OnLoad";
        impl_startObservingEvents(context);
        databaseDoc = impl_loadDocument( documentURL, impl_getMacroExecLoadArgs() );
        impl_stopObservingEvents(m_documentEvents, new String[]
                {
                    "OnLoad"
                }, context);

        assertEquals("our provided interaction handler was not called", STATE_ON_LOAD_RECEIVED, m_loadDocState);

        // restore macro security level
        impl_setMacroSecurityLevel(oldSecurityLevel);

        // close the document
        impl_closeDocument(databaseDoc);
    }


    @Test
    public void testGlobalEvents() throws Exception, IOException
    {
        XModel databaseDoc = impl_createEmptyEmbeddedHSQLDocument();
        final XStorable storeDoc = UnoRuntime.queryInterface(XStorable.class, databaseDoc);

        String context, newURL;

        // XStorable.store
        final String oldURL = databaseDoc.getURL();
        context = "store";
        impl_startObservingEvents(context);
        storeDoc.store();
        assertEquals("store is not expected to change the document URL", databaseDoc.getURL(), oldURL);
        impl_stopObservingEvents(m_globalEvents, new String[]
                {
                    "OnSave", "OnSaveDone"
                }, context);

        // XStorable.storeToURL
        context = "storeToURL";
        impl_startObservingEvents(context);
        storeDoc.storeToURL(createTempFileURL(), new PropertyValue[0]);
        assertEquals("storetoURL is not expected to change the document URL", databaseDoc.getURL(), oldURL);
        impl_stopObservingEvents(m_globalEvents, new String[]
                {
                    "OnSaveTo", "OnSaveToDone"
                }, context);

        // XStorable.storeAsURL
        newURL = createTempFileURL();
        context = "storeAsURL";
        impl_startObservingEvents(context);
        storeDoc.storeAsURL(newURL, new PropertyValue[0]);
        assertEquals("storeAsURL is expected to change the document URL", databaseDoc.getURL(), newURL);
        impl_stopObservingEvents(m_globalEvents, new String[]
                {
                    "OnSaveAs", "OnSaveAsDone"
                }, context);

        // XModifiable.setModified
        final XModifiable modifyDoc = UnoRuntime.queryInterface(XModifiable.class, databaseDoc);
        context = "setModified";
        impl_startObservingEvents(context);
        modifyDoc.setModified(true);
        assertEquals("setModified didn't work", modifyDoc.isModified(), true);
        impl_stopObservingEvents(m_globalEvents, new String[]
                {
                    "OnModifyChanged"
                }, context);

        // XStorable.store, with implicit reset of the "Modified" flag
        context = "store (2)";
        impl_startObservingEvents(context);
        storeDoc.store();
        assertEquals("'store' should implicitly reset the modified flag", modifyDoc.isModified(), false);
        impl_stopObservingEvents(m_globalEvents, new String[]
                {
                    "OnSave", "OnSaveDone", "OnModifyChanged"
                }, context);

        // XComponentLoader.loadComponentFromURL
        newURL = copyToTempFile(databaseDoc.getURL());
        final XComponentLoader loader = UnoRuntime.queryInterface(XComponentLoader.class, getMSF().createInstance("com.sun.star.frame.Desktop"));
        context = "loadComponentFromURL";
        impl_startObservingEvents(context);
        databaseDoc = UnoRuntime.queryInterface(XModel.class, loader.loadComponentFromURL(newURL, _BLANK, 0, impl_getDefaultLoadArgs()));
        impl_stopObservingEvents(m_globalEvents,
                new String[]
                {
                    "OnLoadFinished", "OnViewCreated", "OnFocus", "OnLoad"
                }, context);

        // closing a document by API
        final XCloseable closeDoc = UnoRuntime.queryInterface(XCloseable.class, databaseDoc);
        context = "close (API)";
        impl_startObservingEvents(context);
        closeDoc.close(true);
        impl_stopObservingEvents(m_globalEvents,
                new String[]
                {
                    "OnPrepareUnload", "OnViewClosed", "OnUnload"
                }, context);

        // closing a document via UI
        context = "close (UI)";
        impl_startObservingEvents("prepare for '" + context + "'");
        databaseDoc = UnoRuntime.queryInterface(XModel.class, loader.loadComponentFromURL(newURL, _BLANK, 0, impl_getDefaultLoadArgs()));
        impl_waitForEvent(m_globalEvents, "OnLoad", 5000);
        // wait for all events to arrive - OnLoad should be the last one

        final XDispatchProvider dispatchProvider = UnoRuntime.queryInterface(XDispatchProvider.class, databaseDoc.getCurrentController().getFrame());
        final URL url = impl_getURL(".uno:CloseDoc");
        final XDispatch dispatcher = dispatchProvider.queryDispatch(url, "", 0);
        impl_startObservingEvents(context);
        dispatcher.dispatch(url, new PropertyValue[0]);
        impl_stopObservingEvents(m_globalEvents,
                new String[]
                {
                    "OnPrepareViewClosing", "OnViewClosed", "OnPrepareUnload", "OnUnload"
                }, context);

        // creating a new document
        databaseDoc = impl_createDocument();
        final XLoadable loadDoc = UnoRuntime.queryInterface(XLoadable.class, databaseDoc);
        context = "initNew";
        impl_startObservingEvents(context);
        loadDoc.initNew();
        impl_stopObservingEvents(m_globalEvents, new String[]
                {
                    "OnCreate"
                }, context);

        impl_startObservingEvents(context + " (cleanup)");
        impl_closeDocument(databaseDoc);
        impl_waitForEvent(m_globalEvents, "OnUnload", 5000);

        // focus changes
        context = "activation";
        // for this, load a database document ...
        impl_startObservingEvents("prepare for '" + context + "'");
        databaseDoc = UnoRuntime.queryInterface(XModel.class, loader.loadComponentFromURL(newURL, _BLANK, 0, impl_getDefaultLoadArgs()));
        final int previousOnLoadEventPos = impl_waitForEvent(m_globalEvents, "OnLoad", 5000);
        // ... and another document ...
        final String otherURL = copyToTempFile(databaseDoc.getURL());
        final XModel otherDoc = UnoRuntime.queryInterface(XModel.class, loader.loadComponentFromURL(otherURL, _BLANK, 0, impl_getDefaultLoadArgs()));
        impl_raise(otherDoc);
        impl_waitForEvent(m_globalEvents, "OnLoad", 5000, previousOnLoadEventPos + 1);

        // ... and switch between the two
        impl_startObservingEvents(context);
        impl_raise(databaseDoc);
        impl_stopObservingEvents(m_globalEvents, new String[]
                {
                    "OnUnfocus", "OnFocus"
                }, context);

        // cleanup
        impl_startObservingEvents("cleanup after '" + context + "'");
        impl_closeDocument(databaseDoc);
        impl_closeDocument(otherDoc);
    }


    private URL impl_getURL(String _completeURL) throws Exception
    {
        final URL[] url =
        {
            new URL()
        };
        url[0].Complete = _completeURL;
        final XURLTransformer urlTransformer = UnoRuntime.queryInterface(XURLTransformer.class, getMSF().createInstance("com.sun.star.util.URLTransformer"));
        urlTransformer.parseStrict(url);
        return url[0];
    }


    private void impl_raise(XModel _document)
    {
        final XFrame frame = _document.getCurrentController().getFrame();
        final XTopWindow topWindow = UnoRuntime.queryInterface(XTopWindow.class, frame.getContainerWindow());
        topWindow.toFront();
    }


    private void impl_startObservingEvents(String _context)
    {
        System.out.println(" " + _context + " {");
        synchronized (m_documentEvents)
        {
            m_documentEvents.clear();
        }
        synchronized (m_globalEvents)
        {
            m_globalEvents.clear();
        }
    }


    private void impl_stopObservingEvents(ArrayList<String> _actualEvents, String[] _expectedEvents, String _context)
    {
        try
        {
            synchronized (_actualEvents)
            {
                int actualEventCount = _actualEvents.size();
                while (actualEventCount < _expectedEvents.length)
                {
                    // well, it's possible not all events already arrived, yet - finally, some of them
                    // are notified asynchronously
                    // So, wait a few seconds.
                    try
                    {
                        _actualEvents.wait(20000);
                    }
                    catch (InterruptedException ex)
                    {
                    }

                    if (actualEventCount == _actualEvents.size())
                    // the above wait was left because of the timeout, *not* because an event
                    // arrived. Okay, we won't wait any longer, this is a failure.
                    {
                        break;
                    }
                    actualEventCount = _actualEvents.size();
                }

                assertEquals("wrong event count for '" + _context + "'",
                        _expectedEvents.length, _actualEvents.size());

                for (int i = 0; i < _expectedEvents.length; ++i)
                {
                    assertEquals("wrong event at position " + (i + 1) + " for '" + _context + "'",
                            _expectedEvents[i], _actualEvents.get(i));
                }
            }
        }
        finally
        {
            System.out.println(" }");
        }
    }


    private int impl_waitForEvent(ArrayList<String> _eventQueue, String _expectedEvent, int _maxMilliseconds)
    {
        return impl_waitForEvent(_eventQueue, _expectedEvent, _maxMilliseconds, 0);
    }


    private int impl_waitForEvent(ArrayList<String> _eventQueue, String _expectedEvent, int _maxMilliseconds, int _firstQueueElementToCheck)
    {
        synchronized (_eventQueue)
        {
            int waitedMilliseconds = 0;

            while (waitedMilliseconds < _maxMilliseconds)
            {
                for (int i = _firstQueueElementToCheck; i < _eventQueue.size(); ++i)
                {
                    if (_expectedEvent.equals(_eventQueue.get(i)))
                    // found the event in the queue
                    {
                        return i;
                    }
                }

                // wait a little, perhaps the event will still arrive
                try
                {
                    _eventQueue.wait(500);
                    waitedMilliseconds += 500;
                }
                catch (InterruptedException e)
                {
                }
            }
        }

        fail("expected event '" + _expectedEvent + "' did not arrive after " + _maxMilliseconds + " milliseconds");
        return -1;
    }


    private void onDocumentEvent(DocumentEvent _Event)
    {
        if ("OnTitleChanged".equals(_Event.EventName))
        // OnTitleChanged events are notified too often. This is known, and accepted.
        // (the deeper reason is that it's difficult to determine, in the DatabaseDocument implementation,
        // when the title actually changed. In particular, when we do a saveAsURL, and then ask for a
        // title *before* the TitleHelper got the document's OnSaveAsDone event, then the wrong (old)
        // title is obtained.
        {
            return;
        }

        if ((_Event.EventName.equals("OnLoad")) && (m_loadDocState != STATE_NOT_STARTED))
        {
            assertEquals("OnLoad event must come *after* invocation of the interaction handler / user!",
                    m_loadDocState, STATE_MACRO_EXEC_APPROVED);
            m_loadDocState = STATE_ON_LOAD_RECEIVED;
        }

        synchronized (m_documentEvents)
        {
            m_documentEvents.add(_Event.EventName);
            m_documentEvents.notifyAll();
        }

        System.out.println("  document event: " + _Event.EventName);
    }


    public void documentEventOccured(DocumentEvent _Event)
    {
        if ("OnTitleChanged".equals(_Event.EventName))
        // ignore. See onDocumentEvent for a justification
        {
            return;
        }

        synchronized (m_globalEvents)
        {
            m_globalEvents.add(_Event.EventName);
            m_globalEvents.notifyAll();
        }

        System.out.println("  global event: " + _Event.EventName);
    }


    public void disposing(EventObject _Event)
    {
        // not interested in
    }
}
