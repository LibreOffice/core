/*************************************************************************
*
*  $RCSfile: ScriptProvider.java,v $
*
*  $Revision: 1.6 $
*
*  last change: $Author: rt $ $Date: 2004-05-19 08:23:43 $
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

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.frame.XModel;

import com.sun.star.ucb.XFileIdentifierConverter;

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
import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;

import com.sun.star.script.framework.log.LogUtils;

import com.sun.star.script.framework.container.ScriptMetaData;
import com.sun.star.script.framework.container.XMLParserFactory;
import com.sun.star.script.framework.container.ContainerCache;
import com.sun.star.script.framework.container.ParcelContainer;

import com.sun.star.script.framework.io.XStorageHelper;


import com.sun.star.script.framework.browse.ProviderBrowseNode;
import com.sun.star.script.framework.browse.DialogFactory;


import java.util.*;
public abstract class ScriptProvider
    implements XScriptProvider, XBrowseNode, XPropertySet, XInvocation,
               XInitialization, XTypeProvider, XServiceInfo
{
    private final String[] __serviceNames = {
        "drafts.com.sun.star.script.provider.ScriptProviderFor",
        "drafts.com.sun.star.script.provider.LanguageScriptProvider"
    };

    public final static String CLASSPATH = "classpath";

    private String language;
    private XScriptContext m_xScriptContext;

    protected XComponentContext m_xContext;
    protected XMultiComponentFactory m_xMultiComponentFactory;
    protected XPropertySet m_xInvocationContext;

    // proxies to helper objects which implement interfaces
    private XPropertySet m_xPropertySetProxy;
    private XInvocation m_xInvocationProxy;
   // TODO should this be implemented in this class
    private XBrowseNode m_xBrowseNodeProxy;

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

    public XScriptContext getScriptingContext()
    {
        return m_xScriptContext;
    }

    public void initialize( Object[] aArguments )
        throws com.sun.star.uno.Exception
    {
        LogUtils.DEBUG( "entering XInit for language " + language);
        if( aArguments.length == 1 )
        {
            if (AnyConverter.isObject(aArguments[0]) == true)
            {
                m_xInvocationContext =
                    (XPropertySet) AnyConverter.toObject(
                        new com.sun.star.uno.Type(XPropertySet.class),
                        aArguments[0]);

                 XModel xModel =
                    (XModel) AnyConverter.toObject(
                        new com.sun.star.uno.Type(XModel.class),
                        m_xInvocationContext.getPropertyValue(
                            "SCRIPTING_DOC_REF"));
                XStorageHelper.addNewModel( xModel );
                ContainerCache cache = new ContainerCache( m_xContext, xModel );
                ParcelContainer parcelContainer = cache.getContainer( language, xModel );

                // TODO should be done for provider, no need to proxy this anymore
                m_xBrowseNodeProxy = new ProviderBrowseNode( this, parcelContainer );

                m_xInvocationProxy = (XInvocation)UnoRuntime.queryInterface(XInvocation.class, m_xBrowseNodeProxy);
                m_xPropertySetProxy = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, m_xBrowseNodeProxy);

            }
            else if (AnyConverter.isString(aArguments[0]) == true)
            {
                String sPath = AnyConverter.toString(aArguments[0]);
                LogUtils.DEBUG("creating Application, path: " + sPath);


                // TODO m_xInvocationProxy, m_xPropertySetProxy in ctor, otherwise bridge will
                // have problems as it does some introspection on the XPropertySet
                // interface ( which is null if initialise is not called


                ContainerCache cache = new ContainerCache( m_xContext );
                // TODO fix up where paths are fixed up
                // currently path is also fixed up by macro expander
                ParcelContainer parcelContainer = cache.getContainer( language, sPath );
                LogUtils.DEBUG("******* CONTAINER retrieved is " + parcelContainer );


                // TODO should all be done in this class instead of
                // deleagation????
                m_xBrowseNodeProxy = new ProviderBrowseNode( this, parcelContainer );

                m_xInvocationProxy = (XInvocation)UnoRuntime.queryInterface(XInvocation.class, m_xBrowseNodeProxy);
                m_xPropertySetProxy = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, m_xBrowseNodeProxy);
            }
            else if ( AnyConverter.getType(aArguments[0]).equals( Type.VOID ) )
            {
                // Default type, implies this provider to be used for
                // execution of application ( user/share ) scripts only
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
        m_xScriptContext = ScriptContext.createContext( m_xInvocationContext, m_xContext, m_xMultiComponentFactory );
        LogUtils.DEBUG( "leaving XInit" );
    }

    /**
     *  Gets the types attribute of the ScriptProvider object
     *
     * @return    The types value
     */
    public com.sun.star.uno.Type[] getTypes()
    {
        Type[] retValue = new Type[ 7 ];
        retValue[ 0 ] = new Type( XScriptProvider.class );
        retValue[ 1 ] = new Type( XBrowseNode.class );
        retValue[ 2 ] = new Type( XInitialization.class );
        retValue[ 3 ] = new Type( XTypeProvider.class );
        retValue[ 4 ] = new Type( XServiceInfo.class );
        retValue[ 5 ] = new Type( XPropertySet.class );
        retValue[ 6 ] = new Type( XInvocation.class );
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
               com.sun.star.lang.IllegalArgumentException;

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

    // TODO should throw appropriate exception
    public ScriptMetaData  getScriptData( /*IN*/String scriptURI )
    {

        ScriptMetaData scriptData = null;

        XModel xModel = null;
        if ( m_xInvocationContext != null ) // not application script
        {
            try
            {
                xModel = (XModel) AnyConverter.toObject(
                   new com.sun.star.uno.Type(XModel.class),
                       m_xInvocationContext.getPropertyValue(
                           "SCRIPTING_DOC_REF"));
            }
            catch ( com.sun.star.lang.WrappedTargetException ignore )
            {
            }
            catch ( com.sun.star.beans.UnknownPropertyException ignore )
            {
            }
            catch ( com.sun.star.lang.IllegalArgumentException ignore )
            {
            }
        }
        ContainerCache cache = new ContainerCache( m_xContext, xModel  );

        scriptData = cache.findScript( scriptURI );
        return scriptData;
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

}
