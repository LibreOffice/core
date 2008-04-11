/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScriptProviderForJava.java,v $
 * $Revision: 1.10 $
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
package com.sun.star.script.framework.provider.java;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XDesktop;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.document.XScriptInvocationContext;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;

import java.util.ArrayList;
import java.util.Map;
import java.net.MalformedURLException;

import com.sun.star.script.provider.XScriptContext;
import com.sun.star.script.provider.XScript;
import com.sun.star.script.provider.ScriptExceptionRaisedException;
import com.sun.star.script.provider.ScriptFrameworkErrorException;
import com.sun.star.script.provider.ScriptFrameworkErrorType;

import com.sun.star.script.framework.container.ScriptMetaData;

import com.sun.star.script.framework.provider.*;
import com.sun.star.script.framework.log.LogUtils;
/**
 *  Description of the Class
 *
 * @author     Noel Power
 * @created    August 2, 2002
 */
public class ScriptProviderForJava
{
    /**
     *  Description of the Class
     *
     * @author     Noel Power
     * @created    August 2, 2002
     */
    public static class _ScriptProviderForJava extends ScriptProvider
    {
        private Resolver m_resolutionPolicy = new StrictResolver();

        public _ScriptProviderForJava( XComponentContext ctx )
        {
            super (ctx, "Java");
        }

        public XScript getScript( /*IN*/String scriptURI )
            throws com.sun.star.uno.RuntimeException,
                   ScriptFrameworkErrorException
        {
            ScriptMetaData scriptData = null;
            scriptData = getScriptData( scriptURI );
            ScriptImpl script = null;
            try
            {
                script = new ScriptImpl( m_xContext, m_resolutionPolicy, scriptData, m_xModel, m_xInvocContext );
                return script;
            }
            catch ( com.sun.star.uno.RuntimeException re )
            {
                throw new ScriptFrameworkErrorException( "Failed to create script object: " + re.getMessage(),
                    null, scriptData.getLanguageName(), language, ScriptFrameworkErrorType.UNKNOWN );
            }

        }

        public boolean hasScriptEditor()
        {
            return false;
        }

        public ScriptEditor getScriptEditor()
        {
            return null;
        }
    }

    /**
     * Returns a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     *
     * @param  implName      the name of the implementation for which a service is desired
     * @param  multiFactory  the service manager to be used if needed
     * @param  regKey        the registryKey
     * @return               returns a <code>XSingleServiceFactory</code> for creating
     *                          the component
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory( String implName,
            XMultiServiceFactory multiFactory,
            XRegistryKey regKey )
    {
        XSingleServiceFactory xSingleServiceFactory = null;

        if ( implName.equals( ScriptProviderForJava._ScriptProviderForJava.class.getName() ) )
        {
              xSingleServiceFactory = FactoryHelper.getServiceFactory(
                ScriptProviderForJava._ScriptProviderForJava.class,
                "com.sun.star.script.provider.ScriptProviderForJava",
                multiFactory,
                regKey );
        }

        return xSingleServiceFactory;
    }


    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     *
     * @param  regKey  the registryKey
     * @return         returns true if the operation succeeded
     * @see            com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo( XRegistryKey regKey )
    {
        String impl = "com.sun.star.script.framework.provider.java." +
            "ScriptProviderForJava$_ScriptProviderForJava";

        String service1 = "com.sun.star.script.provider." +
            "ScriptProvider";
        String service2 = "com.sun.star.script.provider." +
            "LanguageScriptProvider";
        String service3 = "com.sun.star.script.provider." +
            "ScriptProviderForJava";
        String service4 = "com.sun.star.script.browse." +
            "BrowseNode";

        if ( FactoryHelper.writeRegistryServiceInfo(impl, service1, regKey) &&
            FactoryHelper.writeRegistryServiceInfo(impl, service2, regKey) &&
            FactoryHelper.writeRegistryServiceInfo(impl, service3, regKey) &&
            FactoryHelper.writeRegistryServiceInfo(impl, service4, regKey) )
        {
              return true;
        }
        return false;
    }

}

class ScriptImpl implements XScript
{
    private ScriptMetaData metaData;
    private XComponentContext m_xContext;
    private XModel m_xModel;
    private XScriptInvocationContext m_xInvocContext;
    private XMultiComponentFactory m_xMultiComponentFactory;
    private Resolver m_resolutionPolicy;
    ScriptImpl( XComponentContext ctx, Resolver resolver, ScriptMetaData metaData, XModel xModel, XScriptInvocationContext xInvocContext ) throws com.sun.star.uno.RuntimeException
    {
        this.metaData = metaData;
        this.m_xContext = ctx;
        this.m_xModel = xModel;
        this.m_xInvocContext = xInvocContext;
        this.m_resolutionPolicy = resolver;
        try
        {
            this.m_xMultiComponentFactory = m_xContext.getServiceManager();
        }
        catch ( Exception e )
        {
            LogUtils.DEBUG( LogUtils.getTrace( e ) );
            throw new com.sun.star.uno.RuntimeException(
                "Error constructing  ScriptProvider: "
                + e.getMessage() );
        }

        LogUtils.DEBUG("ScriptImpl [java] script data = " + metaData );
    }
        /**
         *  Invoke
         *
         *
         * @param aParams           All parameters; pure, out params are undefined in
         *                      sequence, i.e., the value has to be ignored by the callee
         * @param aOutParamIndex    Out indices
         * @param aOutParam         Out parameters
         * @returns                 The value returned from the function being invoked
         * @throws IllegalArgumentException If there is no matching script name
         * @throws CannotConvertException   If args do not match or cannot be converted
         *                              the those of the invokee
         * @throws InvocationTargetException If the running script throws an exception
         *              this information is captured and rethrown as this exception type.
         */

        public Object invoke(
                                     /*IN*/Object[]  params,
                                     /*OUT*/short[][]  aOutParamIndex,
                                     /*OUT*/Object[][]  aOutParam )

        throws  ScriptFrameworkErrorException, com.sun.star.reflection.InvocationTargetException
        {
            LogUtils.DEBUG( "** ScriptProviderForJava::invoke: Starting..." );

            // Initialise the out paramters - not used at the moment
            aOutParamIndex[0] = new short[0];
            aOutParam[0] = new Object[0];


            Map languageProps = metaData.getLanguageProperties();

            ScriptDescriptor scriptDesc =
                new ScriptDescriptor( metaData.getLanguageName() );

            ClassLoader scriptLoader = null;

            try {
                LogUtils.DEBUG( "Classloader starting..." );
                scriptLoader = ClassLoaderFactory.getURLClassLoader(
                        metaData );
                LogUtils.DEBUG( "Classloader finished..." );
            }
            catch (MalformedURLException mfe )
            {
                // Framework error
                throw new ScriptFrameworkErrorException(
                    mfe.getMessage(), null,
                    metaData.getLanguageName(), metaData.getLanguage(),
                    ScriptFrameworkErrorType.UNKNOWN );
            }
            catch (NoSuitableClassLoaderException ncl )
            {
                // Framework error
                throw new ScriptFrameworkErrorException(
                    ncl.getMessage(), null,
                    metaData.getLanguageName(), metaData.getLanguage(),
                    ScriptFrameworkErrorType.UNKNOWN );
            }
            catch (ArrayStoreException e )
            {
                // Framework error
                throw new ScriptFrameworkErrorException(
                    e.getMessage(), null,
                    metaData.getLanguageName(), metaData.getLanguage(),
                    ScriptFrameworkErrorType.UNKNOWN );
            }

            ArrayList invocationArgList = new ArrayList();
            Object[] invocationArgs = null;

            LogUtils.DEBUG( "Parameter Mapping..." );

            // Setup Context Object
            XScriptContext xSc = ScriptContext.createContext(m_xModel, m_xInvocContext,
                m_xContext, m_xMultiComponentFactory);
            scriptDesc.addArgumentType( XScriptContext.class );
            invocationArgList.add( xSc );

            for ( int i = 0; i < params.length; i++ )
            {
                scriptDesc.addArgumentType( params[ i ].getClass() );
                invocationArgList.add( params[ i ] );
            }

            if ( !invocationArgList.isEmpty() )
            {
                invocationArgs = invocationArgList.toArray();
            }



            LogUtils.DEBUG( "ScriptProxy starting... " );
            ScriptProxy script = null;
            try
            {
                String className = metaData.getLanguageName().substring( 0,
                    metaData.getLanguageName().lastIndexOf( '.' ) );
                LogUtils.DEBUG( "About to load Class " + className + " starting... " );

                long start = new java.util.Date().getTime();
                Class c = scriptLoader.loadClass( className );
                long end = new java.util.Date().getTime();

                LogUtils.DEBUG("loadClass took: " + String.valueOf(end - start) +
                      "milliseconds");

                try
                {
                    LogUtils.DEBUG( "class loaded ... " );
                    script = m_resolutionPolicy.getProxy( scriptDesc, c );
                    LogUtils.DEBUG( "script resolved ... " );
                }
                catch( NoSuchMethodException e )
                {
                    // Framework error
                    throw new ScriptFrameworkErrorException(
                        e.getMessage(), null,
                        metaData.getLanguageName(), metaData.getLanguage(),
                        ScriptFrameworkErrorType.UNKNOWN );
                }
            }
            catch ( ClassNotFoundException e )
            {
                // Framework error
                throw new ScriptFrameworkErrorException(
                    e.getMessage(), null,
                    metaData.getLanguageName(), metaData.getLanguage(),
                    ScriptFrameworkErrorType.UNKNOWN );
            }

            LogUtils.DEBUG( "Starting Invoke on Proxy ..." );
            Object result = null;

            try
            {
                long start = new java.util.Date().getTime();
                result = script.invoke( invocationArgs );
                long end = new java.util.Date().getTime();

                LogUtils.DEBUG("invoke took: " +
                    String.valueOf(end - start) + "milliseconds");
            }
            catch ( java.lang.IllegalArgumentException iae )
            {
                throw new ScriptFrameworkErrorException(
                    iae.getMessage(), null,
                    metaData.getLanguageName(), metaData.getLanguage(),
                    ScriptFrameworkErrorType.UNKNOWN );

            }
            catch ( java.lang.IllegalAccessException ia )
            {
                throw new ScriptFrameworkErrorException(
                    ia.getMessage(), null,
                    metaData.getLanguageName(), metaData.getLanguage(),
                    ScriptFrameworkErrorType.UNKNOWN );
            }
            catch ( java.lang.reflect.InvocationTargetException ite )
            {
                Throwable targetException = ite.getTargetException();
                ScriptExceptionRaisedException se =
                    new ScriptExceptionRaisedException(
                        targetException.toString() );
                se.lineNum = -1;
                se.scriptName = metaData.getLanguageName();
                se.language = "Java";
                se.exceptionType = targetException.getClass().getName();
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Scripting Framework error executing script ", null, se );
            }
            catch ( Exception unknown )
            {
                ScriptExceptionRaisedException se =
                    new ScriptExceptionRaisedException(
                        unknown.toString() );
                se.lineNum = -1;
                se.scriptName = metaData.getLanguageName();
                se.language = "Java";
                se.exceptionType = unknown.getClass().getName();
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Scripting Framework error executing script ", null, se );
            }
            if ( result == null )
            {
                LogUtils.DEBUG( "Got Nothing Back" );
                // in the case where there is no return type
                Any voidAny = new Any(new Type(), null);
                result = voidAny;
            }
            else
            {
                LogUtils.DEBUG( "Got object " + result );
            }
            return result;
        }
}
