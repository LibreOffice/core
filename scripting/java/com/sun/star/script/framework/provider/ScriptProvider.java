/*************************************************************************
*
*  $RCSfile: ScriptProvider.java,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: toconnor $ $Date: 2003-10-29 15:01:14 $
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
import com.sun.star.util.XMacroExpander;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;

import com.sun.star.beans.XPropertySet;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.reflection.InvocationTargetException;
import com.sun.star.script.CannotConvertException;

import drafts.com.sun.star.script.provider.XScriptContext;
import drafts.com.sun.star.script.framework.storage.XScriptInfo;
import drafts.com.sun.star.script.provider.XScriptProvider;
import drafts.com.sun.star.script.provider.XScript;
import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.browse.DirBrowseNode;
import com.sun.star.script.framework.browse.DocBrowseNode;
import com.sun.star.script.framework.browse.XMLParserFactory;

public abstract class ScriptProvider
    implements XScriptProvider, XScript, XBrowseNode,
               XInitialization, XTypeProvider, XServiceInfo
{
    private final String[] __serviceNames = {
        "drafts.com.sun.star.script.provider.ScriptProviderFor",
        "drafts.com.sun.star.script.provider.LanguageScriptProvider"
    };

    public final static String CLASSPATH = "classpath";

    private String language;

    protected XComponentContext m_xContext;
    protected XMultiComponentFactory m_xMultiComponentFactory;
    protected XPropertySet m_xInvocationContext;
    protected XScriptInfo m_xScriptInfo;
    protected String m_scriptURI;

    private XBrowseNode m_browseNodeProxy;

    public ScriptProvider( XComponentContext ctx, String language )
    {
        this.language = language;
        __serviceNames[0] += language;

        LogUtils.DEBUG( "ScriptProvider: constructor - start." );

        m_xContext = ctx;

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
            throw new com.sun.star.uno.RuntimeException(
                "Error constructing  ScriptProvider: "
                + e.getMessage() );
        }

        LogUtils.DEBUG( "ScriptProvider: constructor - finished." );
    }


    public void initialize( Object[] aArguments )
        throws com.sun.star.uno.Exception
    {
        LogUtils.DEBUG( "entering XInit" );
        if( aArguments.length == 1 )
        {
            if (AnyConverter.isObject(aArguments[0]) == true)
            {
                m_xInvocationContext =
                    (XPropertySet) AnyConverter.toObject(
                        new com.sun.star.uno.Type(XPropertySet.class),
                        aArguments[0]);

                XModel model =
                    (XModel) AnyConverter.toObject(
                        new com.sun.star.uno.Type(XModel.class),
                        m_xInvocationContext.getPropertyValue(
                            "SCRIPTING_DOC_REF"));

                LogUtils.DEBUG("creating DocBrowseNode, model: " + model.getURL());
                m_browseNodeProxy = new DocBrowseNode(m_xContext,model, language);
            }
            else if (AnyConverter.isString(aArguments[0]) == true)
            {
                String path = AnyConverter.toString(aArguments[0]);
                LogUtils.DEBUG("creating DirBrowseNode, path: " + path);
                m_browseNodeProxy = new DirBrowseNode(m_xContext, path, language);
            }
            else
            {
                throw new com.sun.star.uno.RuntimeException(
                    "ScriptProvider created with invalid argument");
            }
        }
        else
        {
            LogUtils.DEBUG( "throwing from  XInit" );
            throw new com.sun.star.uno.RuntimeException(
                "Incorrect number of args passed to " +
                "ScriptProvider.initialize" );
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
        Type[] retValue = new Type[ 6 ];
        retValue[ 0 ] = new Type( XScriptProvider.class );
        retValue[ 1 ] = new Type( XScript.class );
        retValue[ 2 ] = new Type( XBrowseNode.class );
        retValue[ 3 ] = new Type( XInitialization.class );
        retValue[ 4 ] = new Type( XTypeProvider.class );
        retValue[ 5 ] = new Type( XServiceInfo.class );
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

    public abstract Object invoke( /*IN*/Object[] aParams,
                        /*OUT*/short[][] aOutParamIndex,
                        /*OUT*/Object[][] aOutParam )
        throws IllegalArgumentException, CannotConvertException,
            InvocationTargetException;

    public XScript getScript ( /*IN*/String scriptURI )
        throws com.sun.star.uno.RuntimeException,
               com.sun.star.lang.IllegalArgumentException
    {
        LogUtils.DEBUG( "in ScriptProvider.getScript" );

        if( m_xInvocationContext == null )
        {
            LogUtils.DEBUG( "ScriptProvider.getScript: not init'd" );
            throw new com.sun.star.uno.RuntimeException(
                "ScriptProvider has not been initialised" );
        }

        // do the standard resolution stuff
        // and put something into m_ScriptContext
        XScriptInfo xScriptInfo = null;
        try
        {
            LogUtils.DEBUG( "ScriptProvider.getScript: about to resolve" );
            if( ( xScriptInfo = ScriptResolver.resolve( m_xContext,
                m_xInvocationContext, scriptURI ) ) == null )
            {
                LogUtils.DEBUG( "ScriptProvider.getScript: failed to resolve" );
                throw new com.sun.star.uno.RuntimeException( "ScriptProvider: failed to resolve script: " + scriptURI );
            }
        }
        catch (IllegalArgumentException iae)
        {
            String trace = LogUtils.getTrace( iae );
            LogUtils.DEBUG("ScriptProvider.getScript: failed to resolve:\n" + trace);
            throw new com.sun.star.lang.IllegalArgumentException(trace);
        }

        m_xScriptInfo = xScriptInfo;
        m_scriptURI = scriptURI;
        LogUtils.DEBUG( "leaving ScriptProvider.getScript" );
        return this;
    }

    // Implementation of XBrowseNode interface
    public String getName()
    {
        return language;
    }

    public XBrowseNode[] getChildNodes()
    {
        return m_browseNodeProxy.getChildNodes();
    }

    public boolean hasChildNodes()
    {
        return m_browseNodeProxy.hasChildNodes();
    }

    public short getType()
    {
        return BrowseNodeTypes.CONTAINER;
    }

    public String toString()
    {
        return getName();
    }
}
