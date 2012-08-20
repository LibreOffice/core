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
package com.sun.star.script.framework.provider.java;

import com.sun.star.frame.XModel;
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
 */
public class ScriptProviderForJava
{
    /**
     *  Description of the Class
     *
     * @author     Noel Power
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
                ScriptFrameworkErrorException e2 =
                    new ScriptFrameworkErrorException(
                        "Failed to create script object: " + re,
                        null, scriptData.getLanguageName(), language,
                        ScriptFrameworkErrorType.UNKNOWN );
                e2.initCause( re );
                throw e2;
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
            com.sun.star.uno.RuntimeException e2 =
                new com.sun.star.uno.RuntimeException(
                    "Error constructing ScriptProvider: " + e );
            e2.initCause( e );
            throw e2;
        }

        LogUtils.DEBUG("ScriptImpl [java] script data = " + metaData );
    }
        /**
         *  Invoke
         *
         *
         * @param params            All parameters; pure, out params are undefined in
         *                      sequence, i.e., the value has to be ignored by the callee
         * @param aOutParamIndex    Out indices
         * @param aOutParam         Out parameters
         * @returns                 The value returned from the function being invoked
         * @throws IllegalArgumentException If there is no matching script name
         * @throws CannotConvertException   If args do not match or cannot be converted
         *                              the those of the invokee
         * @throws com.sun.star.reflection.InvocationTargetException If the running script throws an exception
         *              this information is captured and rethrown as this exception type.
         */

        public Object invoke(
                                     /*IN*/Object[]  params,
                                     /*OUT*/short[][]  aOutParamIndex,
                                     /*OUT*/Object[][]  aOutParam )

        throws  ScriptFrameworkErrorException, com.sun.star.reflection.InvocationTargetException
        {
            LogUtils.DEBUG( "** ScriptProviderForJava::invoke: Starting..." );

            // Initialise the out parameters - not used at the moment
            aOutParamIndex[0] = new short[0];
            aOutParam[0] = new Object[0];


            Map languageProps = metaData.getLanguageProperties();

            ScriptDescriptor scriptDesc =
                new ScriptDescriptor( metaData.getLanguageName() );

            ClassLoader scriptLoader = null;

            try {
                LogUtils.DEBUG( "Classloader starting..." );
                scriptLoader = ClassLoaderFactory.getURLClassLoader(
                    m_xContext, metaData );
                LogUtils.DEBUG( "Classloader finished..." );
            }
            catch (MalformedURLException mfe )
            {
                // Framework error
                ScriptFrameworkErrorException e2 =
                    new ScriptFrameworkErrorException(
                        mfe.toString(), null,
                        metaData.getLanguageName(), metaData.getLanguage(),
                        ScriptFrameworkErrorType.MALFORMED_URL );
                e2.initCause( mfe );
                throw e2;
            }
            catch (NoSuitableClassLoaderException ncl )
            {
                // Framework error
                ScriptFrameworkErrorException e2 =
                    new ScriptFrameworkErrorException(
                        ncl.toString(), null,
                        metaData.getLanguageName(), metaData.getLanguage(),
                        ScriptFrameworkErrorType.UNKNOWN );
                e2.initCause( ncl );
                throw e2;
            }
            catch (ArrayStoreException e )
            {
                // Framework error
                ScriptFrameworkErrorException e2 =
                    new ScriptFrameworkErrorException(
                        e.toString(), null,
                        metaData.getLanguageName(), metaData.getLanguage(),
                        ScriptFrameworkErrorType.UNKNOWN );
                e2.initCause( e );
                throw e2;
            }

            ArrayList<Object> invocationArgList = new ArrayList<Object>();
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
                    ScriptFrameworkErrorException e2 =
                        new ScriptFrameworkErrorException(
                            e.toString(), null,
                            metaData.getLanguageName(), metaData.getLanguage(),
                            ScriptFrameworkErrorType.NO_SUCH_SCRIPT );
                    e2.initCause( e );
                    throw e2;
                }
            }
            catch ( ClassNotFoundException e )
            {
                // Framework error
                ScriptFrameworkErrorException e2 =
                    new ScriptFrameworkErrorException(
                        e.toString(), null,
                        metaData.getLanguageName(), metaData.getLanguage(),
                        ScriptFrameworkErrorType.NO_SUCH_SCRIPT );
                e2.initCause( e );
                throw e2;
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
                ScriptFrameworkErrorException e2 =
                    new ScriptFrameworkErrorException(
                        iae.getMessage(), null,
                        metaData.getLanguageName(), metaData.getLanguage(),
                        ScriptFrameworkErrorType.UNKNOWN );
                e2.initCause( iae );
                throw e2;
            }
            catch ( java.lang.IllegalAccessException ia )
            {
                ScriptFrameworkErrorException e2 =
                    new ScriptFrameworkErrorException(
                        ia.toString(), null,
                        metaData.getLanguageName(), metaData.getLanguage(),
                        ScriptFrameworkErrorType.UNKNOWN );
                e2.initCause( ia );
                throw e2;
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
