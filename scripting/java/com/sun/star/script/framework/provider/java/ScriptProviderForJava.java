/*************************************************************************
*
*  $RCSfile: ScriptProviderForJava.java,v $
*
*  $Revision: 1.5 $
*
*  last change: $Author: hr $ $Date: 2004-07-23 14:04:02 $
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
package com.sun.star.script.framework.provider.java;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XDesktop;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XInitialization;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;

import com.sun.star.beans.Property;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.reflection.InvocationTargetException;
import com.sun.star.script.CannotConvertException;

import java.util.Properties;
import java.util.ArrayList;
import java.util.Map;
import java.util.Vector;
import java.net.URL;
import java.net.MalformedURLException;
import java.net.URLEncoder;

import drafts.com.sun.star.script.provider.XScriptContext;
import drafts.com.sun.star.script.provider.XScriptProvider;
import drafts.com.sun.star.script.provider.XScript;
import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;
import drafts.com.sun.star.script.provider.ScriptErrorRaisedException;
import drafts.com.sun.star.script.provider.ScriptExceptionRaisedException;
import drafts.com.sun.star.script.provider.ScriptFrameworkErrorException;
import drafts.com.sun.star.script.provider.ScriptFrameworkErrorType;

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
                script = new ScriptImpl( m_xContext, m_resolutionPolicy, scriptData, m_xModel );
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
                "drafts.com.sun.star.script.provider.ScriptProviderForJava",
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
        String impl = "com.sun.star.scripting.runtime.java." +
            "ScriptProviderForJava$_ScriptProviderForJava";

        String service = "drafts.com.sun.star.script.provider." +
            "ScriptProviderForJava";

        if (FactoryHelper.writeRegistryServiceInfo(impl, service, regKey)) {
              return true;
// code below is commented out because we want this to happen
// as part of the install, this will have to be done
// programatically during the install as registration of java components
// is not fully supported in setup. It should work somewhat like c++ somewhat like c++ registration in install
/*            try {
                XRegistryKey newKey = regKey.createKey(impl + "/UNO/SINGLETONS/drafts.com.sun.star.script.provider.theScriptProviderForJava");
                newKey.setStringValue(service);
                return true;
            }
            catch (Exception ex) {
                System.err.println(
                    "Error registering ScriptProviderForJava: " + ex);
            }*/
        }
        return false;
    }
}

class ScriptImpl implements XScript
{
    private ScriptMetaData metaData;
    private XComponentContext m_xContext;
    private XModel m_xModel;
    private XMultiComponentFactory m_xMultiComponentFactory;
    private Resolver m_resolutionPolicy;
    ScriptImpl( XComponentContext ctx, Resolver resolver, ScriptMetaData metaData, XModel xModel ) throws com.sun.star.uno.RuntimeException
    {
        this.metaData = metaData;
        this.m_xContext = ctx;
        this.m_xModel = xModel;
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
            XScriptContext xSc = ScriptContext.createContext(m_xModel,
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
