/*************************************************************************
*
*  $RCSfile: ScriptProvider.java,v $
*
*  $Revision: 1.7 $
*
*  last change: $Author: hr $ $Date: 2004-07-23 14:02:29 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
*
************************************************************************/
package com.sun.star.script.framework.provider;

import com.sun.star.container.XNameContainer;

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
import com.sun.star.uno.Any;

import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.Property;

import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.script.XInvocation;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.reflection.InvocationTargetException;
import com.sun.star.script.CannotConvertException;

import drafts.com.sun.star.script.provider.XScriptContext;
import drafts.com.sun.star.script.provider.XScriptProvider;
import drafts.com.sun.star.script.provider.XScript;
import drafts.com.sun.star.script.provider.ScriptFrameworkErrorException;
import drafts.com.sun.star.script.provider.ScriptFrameworkErrorType;

import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;

import com.sun.star.script.framework.log.LogUtils;

import com.sun.star.script.framework.container.ScriptMetaData;
import com.sun.star.script.framework.container.XMLParserFactory;
import com.sun.star.script.framework.container.ParcelContainer;
import com.sun.star.script.framework.container.ParsedScriptUri;
import com.sun.star.script.framework.container.UnoPkgContainer;

import com.sun.star.ucb.Command;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XSimpleFileAccess;

import com.sun.star.sdbc.XRow;

import com.sun.star.script.framework.browse.ProviderBrowseNode;
import com.sun.star.script.framework.browse.DialogFactory;

import com.sun.star.deployment.XPackage;


import java.util.*;
public abstract class ScriptProvider
    implements XScriptProvider, XBrowseNode, XPropertySet, XInvocation,
               XInitialization, XTypeProvider, XServiceInfo, XNameContainer
{
    private final String[] __serviceNames = {
        "drafts.com.sun.star.script.provider.ScriptProviderFor",
        "drafts.com.sun.star.script.provider.LanguageScriptProvider"
    };

    public final static String CLASSPATH = "classpath";

    protected String language;
    protected String contextUrl;

    protected XComponentContext m_xContext;
    protected XMultiComponentFactory m_xMultiComponentFactory;
    protected XModel m_xModel;
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
                "${$SYSBINDIR/bootstraprc::BaseInstallation}/share/dtd/officedocument/1_0/"));

        }
        catch ( Exception e )
        {
            LogUtils.DEBUG( LogUtils.getTrace( e ) );
            throw new com.sun.star.uno.RuntimeException(
                "Error constructing  ScriptProvider: "
                + e.getMessage() );
        }

        LogUtils.DEBUG( "ScriptProvider: constructor - finished." );
    }

    synchronized public XScriptContext getScriptingContext()
    {
        if ( m_xScriptContext == null )
        {
            m_xScriptContext = ScriptContext.createContext( m_xModel, m_xContext, m_xMultiComponentFactory );
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
            if (AnyConverter.isString(aArguments[0]) == true)
            {
                String sCtxUrl = AnyConverter.toString(aArguments[0]);
                LogUtils.DEBUG("creating Application, path: " + sCtxUrl );
                contextUrl = sCtxUrl;
                // TODO no support for packages in documents yet
                if ( sCtxUrl.startsWith( "vnd.sun.star.tdoc" ) )
                {
                    m_container = new ParcelContainer( m_xContext, contextUrl, language );
                    m_xModel = getModelFromDocUrl( sCtxUrl );
                }
                else
                {

                    if ( sCtxUrl.startsWith( "share" ) )
                    {
                        contextUrl = "vnd.sun.star.expand:${$SYSBINDIR/" + PathUtils.BOOTSTRAP_NAME + "::BaseInstallation}/share";
                    }
                    else if ( sCtxUrl.startsWith( "user" ) )
                    {
                        contextUrl = "vnd.sun.star.expand:${$SYSBINDIR/" + PathUtils.BOOTSTRAP_NAME + "::UserInstallation}/user";
                    }

                    if ( sCtxUrl.endsWith( "uno_packages") )
                    {
                        isPkgProvider = true;
                    }
                    if ( sCtxUrl.endsWith( "uno_packages") &&  !sCtxUrl.equals( contextUrl ) )
                    {
                        contextUrl = PathUtils.make_url( contextUrl, "uno_packages" );
                    }
                    if ( isPkgProvider )
                    {
                        m_container = new UnoPkgContainer( m_xContext, contextUrl, language );
                    }
                    else
                    {
                        m_container = new ParcelContainer( m_xContext, contextUrl, language );;
                    }
                }
                LogUtils.DEBUG("Modified Application path is: " + contextUrl );
                LogUtils.DEBUG("isPkgProvider is: " + isPkgProvider );


                // TODO should all be done in this class instead of
                // deleagation????
                m_xBrowseNodeProxy = new ProviderBrowseNode( this,
                    m_container, m_xContext );

                m_xInvocationProxy = (XInvocation)UnoRuntime.queryInterface(XInvocation.class, m_xBrowseNodeProxy);
                m_xPropertySetProxy = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, m_xBrowseNodeProxy);


            }
            else
            {
                throw new com.sun.star.uno.RuntimeException(
                    "ScriptProvider created with invalid argument");
            }
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
    // How this is passed down or how it is accessable by BrowseNode
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
                // TODO specify the correct error Type
                throw new ScriptFrameworkErrorException( details.function + " does not exist",
                    null, details.function, language, ScriptFrameworkErrorType.UNKNOWN );
            }
            return scriptData;
        }
        catch (  com.sun.star.lang.IllegalArgumentException ila )
        {
            // TODO specify the correct error Type
            throw new ScriptFrameworkErrorException( ila.getMessage(),
                null, scriptURI, language, ScriptFrameworkErrorType.UNKNOWN );
        }
        catch ( com.sun.star.container.NoSuchElementException nse )
        {
            // TODO specify the correct error Type
            throw new ScriptFrameworkErrorException( nse.getMessage(),
                null, details.function, language, ScriptFrameworkErrorType.UNKNOWN );
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
            throw new ScriptFrameworkErrorException( message,
                null, details.function, language, ScriptFrameworkErrorType.UNKNOWN );
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
        if ( true )
        {
            throw new com.sun.star.uno.RuntimeException(
                "getByName not implemented" );
        }
        return new Object();
    }

    public String[] getElementNames()
    {
        // TODO needs implementing?
        String[] result = new String[0];
        if ( true )
        {
            throw new com.sun.star.uno.RuntimeException(
                "getElementNames not implemented" );

        }
        return result;
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
        boolean result = false;
        if ( true )
        {
            throw new com.sun.star.uno.RuntimeException(
                "hasElements not implemented" );

        }
        return result;
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
        XPackage newPackage = ( XPackage ) UnoRuntime.queryInterface( XPackage.class, aElement );
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
            String libName = Name.substring( Name.lastIndexOf( "/" ) + 1 );
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
private  XModel getModelFromDocUrl( String docUrl )
{
        LogUtils.DEBUG("getModelFromDocUrl - searching for match for ->" + docUrl + "<-" );
        XModel xModel = null;
        try
        {
            Object[] args = new String[] {"Local", "Office" };

            Object ucb = m_xMultiComponentFactory.createInstanceWithArgumentsAndContext( "com.sun.star.ucb.UniversalContentBroker", args, m_xContext );


            XContentIdentifierFactory xFac  =  ( XContentIdentifierFactory )
                UnoRuntime.queryInterface( XContentIdentifierFactory.class,
                    ucb );


            XContentIdentifier xCntId = xFac.createContentIdentifier( docUrl );


            XContentProvider xCntAccess = ( XContentProvider )
                UnoRuntime.queryInterface( XContentProvider.class,
                    ucb );


            XContent xCnt = xCntAccess.queryContent( xCntId );


            XCommandProcessor xCmd = ( XCommandProcessor )
                UnoRuntime.queryInterface( XCommandProcessor.class, xCnt );


            Property[] pArgs = new Property[ ] { new Property() };
            pArgs[ 0 ].Name = "DocumentModel";
            pArgs[ 0 ].Handle = -1;

            Command command = new Command();

            command.Handle = -1;
            command.Name = "getPropertyValues";
            command.Argument = pArgs;

            com.sun.star.ucb.XCommandEnvironment env = null ;
            Object result =  xCmd.execute( command, 0, env ) ;

            XRow values = ( XRow ) UnoRuntime.queryInterface( XRow.class,
                result );

            xModel = ( XModel )  UnoRuntime.queryInterface( XModel.class,
                values.getObject( 1, null ) );
        }
        catch ( Exception ignore )
        {
            LogUtils.DEBUG("Failed to get model exception " + ignore );

        }
        return xModel;
    }


}
