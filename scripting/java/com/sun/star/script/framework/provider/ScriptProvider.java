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
package com.sun.star.script.framework.provider;

import com.sun.star.container.XNameContainer;

import com.sun.star.uno.Exception;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.frame.XModel;

import com.sun.star.util.XMacroExpander;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.Property;

import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.script.XInvocation;

import com.sun.star.script.provider.XScriptContext;
import com.sun.star.script.provider.XScriptProvider;
import com.sun.star.script.provider.XScript;
import com.sun.star.script.provider.ScriptFrameworkErrorException;
import com.sun.star.script.provider.ScriptFrameworkErrorType;

import com.sun.star.script.browse.XBrowseNode;
import com.sun.star.script.browse.BrowseNodeTypes;

import com.sun.star.script.framework.log.LogUtils;

import com.sun.star.script.framework.container.ScriptMetaData;
import com.sun.star.script.framework.container.XMLParserFactory;
import com.sun.star.script.framework.container.ParcelContainer;
import com.sun.star.script.framework.container.ParsedScriptUri;
import com.sun.star.script.framework.container.UnoPkgContainer;

import com.sun.star.ucb.Command;
import com.sun.star.ucb.UniversalContentBroker;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XUniversalContentBroker;

import com.sun.star.sdbc.XRow;

import com.sun.star.script.framework.browse.ProviderBrowseNode;
import com.sun.star.script.framework.browse.DialogFactory;

import com.sun.star.deployment.XPackage;


import com.sun.star.document.XScriptInvocationContext;
import com.sun.star.frame.XTransientDocumentsDocumentContentFactory;
import com.sun.star.uno.TypeClass;

public abstract class ScriptProvider
    implements XScriptProvider, XBrowseNode, XPropertySet, XInvocation,
               XInitialization, XTypeProvider, XServiceInfo, XNameContainer
{
    private final String[] __serviceNames = {
        "com.sun.star.script.provider.ScriptProviderFor",
        "com.sun.star.script.provider.LanguageScriptProvider"
    };

    public final static String CLASSPATH = "classpath";

    protected String language;

    protected XComponentContext m_xContext;
    protected XMultiComponentFactory m_xMultiComponentFactory;
    protected XModel m_xModel;
    protected XScriptInvocationContext m_xInvocContext;
    protected ParcelContainer m_container;

    // proxies to helper objects which implement interfaces
    private XPropertySet m_xPropertySetProxy;
    private XInvocation m_xInvocationProxy;
   // TODO should this be implemented in this class
    private XBrowseNode m_xBrowseNodeProxy;
    private XScriptContext m_xScriptContext;

    public ScriptProvider( XComponentContext ctx, String language )
    {
        this.language = language;
        __serviceNames[0] += language;

        LogUtils.DEBUG( "ScriptProvider: constructor - start. " + language );

        m_xContext = ctx;

        // Initialize DialogFactory class in case dialogs are required
        DialogFactory.createDialogFactory(m_xContext);

        try
        {
            m_xMultiComponentFactory = m_xContext.getServiceManager();

            if ( m_xMultiComponentFactory == null )
            {
                throw new Exception( "Error could not obtain a " +
                    "multicomponent factory - rethrowing Exception." );
            }

            Object serviceObj = m_xContext.getValueByName(
                "/singletons/com.sun.star.util.theMacroExpander");

            XMacroExpander me = (XMacroExpander) AnyConverter.toObject(
                new Type(XMacroExpander.class), serviceObj);

            XMLParserFactory.setOfficeDTDURL(me.expandMacros(
                "$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/dtd/officedocument/1_0/"));

        }
        catch ( Exception e )
        {
            LogUtils.DEBUG( LogUtils.getTrace( e ) );
            com.sun.star.uno.RuntimeException e2 =
                new com.sun.star.uno.RuntimeException(
                    "Error constructing  ScriptProvider: " + e );
            e2.initCause( e );
            throw e2;
        }

        LogUtils.DEBUG( "ScriptProvider: constructor - finished." );
    }

    synchronized public XScriptContext getScriptingContext()
    {
        if ( m_xScriptContext == null )
        {
            m_xScriptContext = ScriptContext.createContext( m_xModel, m_xInvocContext, m_xContext, m_xMultiComponentFactory );
        }
        return m_xScriptContext;
    }
    public void initialize( Object[] aArguments )
        throws com.sun.star.uno.Exception
    {
        LogUtils.DEBUG( "entering XInit for language " + language);
        boolean isPkgProvider = false;
        if( aArguments.length == 1 )
        {
            String contextUrl = null;
            if ( AnyConverter.getType(aArguments[0]).getTypeClass().equals(TypeClass.INTERFACE) )
            {
                // try whether it denotes a XScriptInvocationContext
                m_xInvocContext = UnoRuntime.queryInterface(
                        XScriptInvocationContext.class, aArguments[0]);
                if ( m_xInvocContext != null )
                {
                    // if so, obtain the document - by definition, this must be
                    // the ScriptContainer
                    m_xModel = UnoRuntime.queryInterface( XModel.class,
                            m_xInvocContext.getScriptContainer() );
                }
                else
                {
                    // otherwise, check whether it's an XModel
                    m_xModel = UnoRuntime.queryInterface( XModel.class,
                            m_xInvocContext.getScriptContainer() );
                }
                if ( m_xModel == null )
                {
                    throw new com.sun.star.uno.Exception(
                        "ScriptProvider argument must be either a string, a valid XScriptInvocationContext, " +
                        "or an XModel", this);
                }

                contextUrl = getDocUrlFromModel( m_xModel );
                m_container = new ParcelContainer( m_xContext, contextUrl, language  );
            }
            else if (AnyConverter.isString(aArguments[0]) == true)
            {
                String originalContextURL = AnyConverter.toString(aArguments[0]);
                LogUtils.DEBUG("creating Application, path: " + originalContextURL );
                contextUrl = originalContextURL;
                // TODO no support for packages in documents yet
                if ( originalContextURL.startsWith( "vnd.sun.star.tdoc" ) )
                {
                    m_container = new ParcelContainer( m_xContext, contextUrl, language  );
                    m_xModel = getModelFromDocUrl( originalContextURL );
                }
                else
                {
                    String extensionDb = null;
                    String extensionRepository = null;
                    if ( originalContextURL.startsWith( "bundled" ) )
                    {
                        contextUrl = "vnd.sun.star.expand:$BUNDLED_EXTENSIONS";
                        extensionDb = "vnd.sun.star.expand:${$BRAND_BASE_DIR/$BRAND_BIN_SUBDIR/" + PathUtils.BOOTSTRAP_NAME + "::UserInstallation}/user";
                        extensionRepository = "bundled";
                    }

                    if ( originalContextURL.startsWith( "share" ) )
                    {
                        contextUrl = "vnd.sun.star.expand:$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR";
                        extensionDb = "vnd.sun.star.expand:${$BRAND_BASE_DIR/$BRAND_BIN_SUBDIR/" + PathUtils.BOOTSTRAP_NAME + "::UserInstallation}/user";
                        extensionRepository = "shared";
                    }
                    else if ( originalContextURL.startsWith( "user" ) )
                    {
                        contextUrl = "vnd.sun.star.expand:${$BRAND_BASE_DIR/$BRAND_BIN_SUBDIR/" + PathUtils.BOOTSTRAP_NAME + "::UserInstallation}/user";
                        extensionDb = "vnd.sun.star.expand:${$BRAND_BASE_DIR/$BRAND_BIN_SUBDIR/" + PathUtils.BOOTSTRAP_NAME + "::UserInstallation}/user";
                        extensionRepository = "user";
                    }

                    if ( originalContextURL.endsWith( "uno_packages") )
                    {
                        isPkgProvider = true;
                    }
                    if ( originalContextURL.endsWith( "uno_packages") &&  !originalContextURL.equals( contextUrl  )
                            && !extensionRepository.equals("bundled"))
                    {
                        contextUrl = PathUtils.make_url( contextUrl, "uno_packages"  );
                    }
                    if ( isPkgProvider )
                    {
                        m_container = new UnoPkgContainer( m_xContext, contextUrl, extensionDb, extensionRepository, language  );
                    }
                    else
                    {
                        m_container = new ParcelContainer( m_xContext, contextUrl, language  );
                    }
                }
            }
            else
            {
                throw new com.sun.star.uno.RuntimeException(
                    "ScriptProvider created with invalid argument");
            }

            LogUtils.DEBUG("Modified Application path is: " + contextUrl );
            LogUtils.DEBUG("isPkgProvider is: " + isPkgProvider );

            // TODO should all be done in this class instead of
            // deleagation????
            m_xBrowseNodeProxy = new ProviderBrowseNode( this,
                m_container, m_xContext );

            m_xInvocationProxy = UnoRuntime.queryInterface(XInvocation.class, m_xBrowseNodeProxy);
            m_xPropertySetProxy = UnoRuntime.queryInterface(XPropertySet.class, m_xBrowseNodeProxy);
        }
        else
        {
            // this is ok, for example when executing a script from the
            // command line
            LogUtils.DEBUG( "ScriptProviderFor" + language +
                " initialized without a context");
        }
        LogUtils.DEBUG( "leaving XInit" );
    }

    /**
     *  Gets the types attribute of the ScriptProvider object
     *
     * @return    The types value
     */
    public com.sun.star.uno.Type[] getTypes()
    {
        Type[] retValue = new Type[ 8 ];
        retValue[ 0 ] = new Type( XScriptProvider.class );
        retValue[ 1 ] = new Type( XBrowseNode.class );
        retValue[ 2 ] = new Type( XInitialization.class );
        retValue[ 3 ] = new Type( XTypeProvider.class );
        retValue[ 4 ] = new Type( XServiceInfo.class );
        retValue[ 5 ] = new Type( XPropertySet.class );
        retValue[ 6 ] = new Type( XInvocation.class );
        retValue[ 7 ] = new Type( com.sun.star.container.XNameContainer.class );
        return retValue;
    }

    /**
     *  Gets the implementationId attribute of the ScriptProvider object
     *
     * @return    The implementationId value
     */
    public byte[] getImplementationId()
    {
        return this.getClass().getName().getBytes();
    }

    /**
     *  Gets the implementationName attribute of the ScriptProvider object
     *
     * @return    The implementationName value
     */
    public String getImplementationName()
    {
        return getClass().getName();
    }

    /**
     *  Description of the Method
     *
     * @param  serviceName  Description of the Parameter
     * @return              Description of the Return Value
     */
    public boolean supportsService( String serviceName )
    {
        for ( int index = __serviceNames.length; index-- > 0; )
        {
            if ( serviceName.equals( __serviceNames[ index ]  ) )
            {
                return true;
            }
        }
        return false;
    }

    /**
     *  Gets the supportedServiceNames attribute of the ScriptProvider object
     *
     * @return    The supportedServiceNames value
     */
    public String[] getSupportedServiceNames()
    {
        return __serviceNames;
    }



    public abstract XScript getScript( /*IN*/String scriptURI )
        throws com.sun.star.uno.RuntimeException,
               ScriptFrameworkErrorException;

    // TODO need to encapsulate this better,
    // Some factory concept for creating/accessing Editor
    // How this is passed down or how it is accessible by BrowseNode
    // implementations needs thinking about
    // This method is used to determine whether the ScriptProvider
    // has a ScriptEditor
    public abstract boolean hasScriptEditor();
    // TODO see above
    // This method is used to get the ScriptEditor for this ScriptProvider
    public abstract ScriptEditor getScriptEditor();

    public ScriptMetaData  getScriptData( /*IN*/String scriptURI ) throws ScriptFrameworkErrorException

    {
        ParsedScriptUri details = null;
        try
        {
            details = m_container.parseScriptUri( scriptURI );
            ScriptMetaData scriptData = m_container.findScript( details );
            if ( scriptData == null )
            {
                throw new ScriptFrameworkErrorException( details.function + " does not exist",
                    null, details.function, language, ScriptFrameworkErrorType.NO_SUCH_SCRIPT );
            }
            return scriptData;
        }
        catch (  com.sun.star.lang.IllegalArgumentException ila )
        {
            // TODO specify the correct error Type
            ScriptFrameworkErrorException e2 =
                new ScriptFrameworkErrorException(
                    ila.getMessage(), null, scriptURI, language,
                    ScriptFrameworkErrorType.UNKNOWN );
            e2.initCause( ila );
            throw e2;
        }
        catch ( com.sun.star.container.NoSuchElementException nse )
        {
            ScriptFrameworkErrorException e2 =
                new ScriptFrameworkErrorException(
                    nse.getMessage(), null, details.function, language,
                    ScriptFrameworkErrorType.NO_SUCH_SCRIPT );
            e2.initCause( nse );
            throw e2;
        }
        catch ( com.sun.star.lang.WrappedTargetException wta )
        {
            // TODO specify the correct error Type
            Exception wrapped = (Exception)wta.TargetException;
            String message = wta.getMessage();
            if ( wrapped != null )
            {
                message = wrapped.getMessage();
            }
            ScriptFrameworkErrorException e2 =
                new ScriptFrameworkErrorException(
                    message, null, details.function, language,
                    ScriptFrameworkErrorType.UNKNOWN );
            e2.initCause( wta );
            throw e2;
        }

    }


    // Implementation of XBrowseNode interface
    public String getName()
    {
        return language;
    }

    public XBrowseNode[] getChildNodes()
    {
        if ( m_xBrowseNodeProxy  == null )
        {
            LogUtils.DEBUG("No Nodes available ");
            return new XBrowseNode[0];
        }
        return m_xBrowseNodeProxy .getChildNodes();
    }

    public boolean hasChildNodes()
    {
        if (  m_xBrowseNodeProxy == null )
        {
            LogUtils.DEBUG("No Nodes available ");
            return false;
        }
        return  m_xBrowseNodeProxy.hasChildNodes();
    }

    public short getType()
    {
        return BrowseNodeTypes.CONTAINER;
    }

    public String toString()
    {
        return getName();
    }

    // implementation of XInvocation interface
    public XIntrospectionAccess getIntrospection() {
        return m_xInvocationProxy.getIntrospection();
    }

    public Object invoke(String aFunctionName, Object[] aParams,
                         short[][] aOutParamIndex, Object[][] aOutParam)
        throws com.sun.star.lang.IllegalArgumentException,
               com.sun.star.script.CannotConvertException,
               com.sun.star.reflection.InvocationTargetException
    {
        return m_xInvocationProxy.invoke(
            aFunctionName, aParams, aOutParamIndex, aOutParam);
    }

    public void setValue(String aPropertyName, Object aValue)
        throws com.sun.star.beans.UnknownPropertyException,
               com.sun.star.script.CannotConvertException,
               com.sun.star.reflection.InvocationTargetException
    {
        m_xInvocationProxy.setValue(aPropertyName, aValue);
    }

    public Object getValue(String aPropertyName)
        throws com.sun.star.beans.UnknownPropertyException
    {
        return m_xInvocationProxy.getValue(aPropertyName);
    }

    public boolean hasMethod(String aName) {
        return m_xInvocationProxy.hasMethod(aName);
    }

    public boolean hasProperty(String aName) {
        return m_xInvocationProxy.hasProperty(aName);
    }

    public XPropertySetInfo getPropertySetInfo()
    {
        return m_xPropertySetProxy.getPropertySetInfo();
    }

    public void setPropertyValue(String aPropertyName, Object aValue)
        throws com.sun.star.beans.UnknownPropertyException,
               com.sun.star.beans.PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException,
               com.sun.star.lang.WrappedTargetException
    {
        m_xPropertySetProxy.setPropertyValue(aPropertyName, aValue);
    }

    public Object getPropertyValue(String PropertyName)
        throws com.sun.star.beans.UnknownPropertyException,
               com.sun.star.lang.WrappedTargetException
    {
        return m_xPropertySetProxy.getPropertyValue(PropertyName);
    }

    public void addPropertyChangeListener(
        String aPropertyName, XPropertyChangeListener xListener)
        throws com.sun.star.beans.UnknownPropertyException,
               com.sun.star.lang.WrappedTargetException
    {
        m_xPropertySetProxy.addPropertyChangeListener(aPropertyName, xListener);
    }

    public void removePropertyChangeListener(
        String aPropertyName, XPropertyChangeListener aListener)
        throws com.sun.star.beans.UnknownPropertyException,
               com.sun.star.lang.WrappedTargetException
    {
        m_xPropertySetProxy.removePropertyChangeListener(
            aPropertyName, aListener);
    }

    public void addVetoableChangeListener(
        String PropertyName, XVetoableChangeListener aListener)
        throws com.sun.star.beans.UnknownPropertyException,
               com.sun.star.lang.WrappedTargetException
    {
        m_xPropertySetProxy.addVetoableChangeListener(PropertyName, aListener);
    }

    public void removeVetoableChangeListener(
        String PropertyName, XVetoableChangeListener aListener)
        throws com.sun.star.beans.UnknownPropertyException,
               com.sun.star.lang.WrappedTargetException
    {
        m_xPropertySetProxy.removeVetoableChangeListener(
            PropertyName, aListener);
    }
    public java.lang.Object getByName( String aName ) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
    {
        // TODO needs implementing?
        throw new com.sun.star.uno.RuntimeException(
            "getByName not implemented" );
    }

    public String[] getElementNames()
    {
        // TODO needs implementing?
        String[] result = new String[0];
        throw new com.sun.star.uno.RuntimeException(
            "getElementNames not implemented" );

    }


    // Performs the getRegStatus functionality for the PkgMgr
    public boolean hasByName( String aName )
    {
        boolean result = false;
        if ( ((UnoPkgContainer)m_container).hasRegisteredUnoPkgContainer( aName ) )
        {
            result = true;
        }
        return result;
    }

    public com.sun.star.uno.Type getElementType()
    {
        // TODO at the moment this returns void indicating
        // type is unknown should indicate XPackage ? do we implement XPackage
        return new Type();
    }

    public boolean hasElements()
    {
        // TODO needs implementing?
        throw new com.sun.star.uno.RuntimeException(
            "hasElements not implemented" );
    }
    public void replaceByName( String aName, java.lang.Object aElement ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
    {
        // TODO needs implementing
        if ( true )
        {
            throw new com.sun.star.uno.RuntimeException(
                "replaceByName not implemented" );

        }
    }

    public void insertByName( String aName, java.lang.Object aElement ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.container.ElementExistException, com.sun.star.lang.WrappedTargetException
    {
        LogUtils.DEBUG("Provider for " + language + " received register for package " + aName );
        XPackage newPackage = UnoRuntime.queryInterface( XPackage.class, aElement );
        if ( aName.length() == 0 )
        {
            throw new  com.sun.star.lang.IllegalArgumentException( "Empty name" );
        }
        if ( newPackage == null )
        {
            throw new com.sun.star.lang.IllegalArgumentException( "No package supplied" );
        }

        ((UnoPkgContainer)m_container).processUnoPackage( newPackage, language  );
    }

    // de-register for library only !!
    public void removeByName( String Name ) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
    {
        LogUtils.DEBUG("In ScriptProvider.removeByName() for " + Name + " this provider = " + language );
        ParcelContainer c = ((UnoPkgContainer)m_container).getRegisteredUnoPkgContainer( Name );
        if ( c != null )
        {
            String libName;
            if (Name.endsWith("/"))
            {
                String tmp = Name.substring( 0, Name.lastIndexOf( "/" ) );
                libName = tmp.substring( tmp.lastIndexOf( "/" ) + 1 );
            }
            else
            {
                libName = Name.substring( Name.lastIndexOf( "/" ) + 1 );
            }
            LogUtils.DEBUG("Deregistering library " + libName );
            if ( c.removeParcel( libName ) )
            {
                ((UnoPkgContainer)m_container).deRegisterPackageContainer( Name );
            }
            else
            {
                throw new com.sun.star.container.NoSuchElementException( libName + " cannot be removed from container." );
            }
        }
        else
        {
            throw new com.sun.star.container.NoSuchElementException( Name + " doesn't exist for " + language );
        }
        // TODO see if we want to remove the ParcelContainer is no Parcels/Libraries left
    }

    private String getDocUrlFromModel( XModel document )
    {
        XTransientDocumentsDocumentContentFactory factory = null;
        try
        {
            factory = UnoRuntime.queryInterface(
                    XTransientDocumentsDocumentContentFactory.class,
                        m_xMultiComponentFactory.createInstanceWithContext(
                            "com.sun.star.frame.TransientDocumentsDocumentContentFactory",
                            m_xContext
                        )
                    );
        }
        catch (Exception ex)
        {
        }

        if ( factory == null )
            throw new com.sun.star.uno.RuntimeException( "ScriptProvider: unable to create a TDOC context factory.", this );

        try
        {
            XContent content = factory.createDocumentContent( document );
            return content.getIdentifier().getContentIdentifier();
        }
        catch (com.sun.star.lang.IllegalArgumentException ex)
        {
        }

        LogUtils.DEBUG("unable to determine the model's TDOC URL");
        return "";
    }

    private  XModel getModelFromDocUrl( String docUrl )
    {
        LogUtils.DEBUG("getModelFromDocUrl - searching for match for ->" + docUrl + "<-" );
        XModel xModel = null;
        try
        {
            XUniversalContentBroker ucb = UniversalContentBroker.create( m_xContext );

            XContentIdentifier xCntId = ucb.createContentIdentifier( docUrl );


            XContent xCnt = ucb.queryContent( xCntId );


            XCommandProcessor xCmd = UnoRuntime.queryInterface( XCommandProcessor.class, xCnt );


            Property[] pArgs = new Property[ ] { new Property() };
            pArgs[ 0 ].Name = "DocumentModel";
            pArgs[ 0 ].Handle = -1;

            Command command = new Command();

            command.Handle = -1;
            command.Name = "getPropertyValues";
            command.Argument = pArgs;

            com.sun.star.ucb.XCommandEnvironment env = null ;
            Object result =  xCmd.execute( command, 0, env ) ;

            XRow values = UnoRuntime.queryInterface( XRow.class,
                result );

            xModel = UnoRuntime.queryInterface( XModel.class,
                values.getObject( 1, null ) );
        }
        catch ( Exception ignore )
        {
            LogUtils.DEBUG("Failed to get model exception " + ignore );

        }
        return xModel;
    }


}
