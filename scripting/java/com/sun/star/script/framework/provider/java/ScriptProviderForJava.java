/*************************************************************************
*
*  $RCSfile: ScriptProviderForJava.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: toconnor $ $Date: 2003-10-29 15:01:17 $
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
import java.net.MalformedURLException;

import drafts.com.sun.star.script.provider.XScriptContext;
import drafts.com.sun.star.script.framework.storage.XScriptInfo;
import drafts.com.sun.star.script.provider.XScriptProvider;
import drafts.com.sun.star.script.provider.XScript;
import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;

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

        /**
         *  Invoke
         *
         *
         * @param scriptName        The scriptName is the language specific name of the
         *                          script
         * @param invocationCtx     The invocation context contains the documentStorageID
         *                      and document reference for use in script name resolving
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

        private Object invoke(  /*IN*/String scriptURI,
                                     /*IN*/Object invocationCtx,
                                     /*IN*/Object[]  params,
                                     /*OUT*/short[][]  aOutParamIndex,
                                     /*OUT*/Object[][]  aOutParam )

        throws IllegalArgumentException, InvocationTargetException, CannotConvertException
        {
            LogUtils.DEBUG( "** ScriptProviderForJava::invoke: Starting..." );

            // Initialise the out paramters - not used at the moment
            aOutParamIndex[0] = new short[0];
            aOutParam[0] = new Object[0];


            String parcelURI = m_xScriptInfo.getParcelURI();
            XPropertySet languageProps = m_xScriptInfo.getLanguageProperties();
            String classpath = null;
            try
            {
                classpath = ( String ) languageProps.getPropertyValue( CLASSPATH );
            }
            catch ( Exception e )
            {
                LogUtils.DEBUG( "Can't get classpath!!" );
            }
            if ( classpath == null )
            {
                classpath = "";
            }
            LogUtils.DEBUG("Classpath from parcel is " + classpath );
            ScriptDescriptor scriptDesc =
                new ScriptDescriptor( m_xScriptInfo.getFunctionName() );

            try {
                scriptDesc.setClasspath(
                    PathUtils.buildClasspath( parcelURI, classpath ) );
            }
            catch (MalformedURLException mue) {
                throw new InvocationTargetException(mue.getMessage());
            }

            ArrayList invocationArgList = new ArrayList();
            Object[] invocationArgs = null;

            LogUtils.DEBUG( "Parameter Mapping..." );

            // Setup Context Object
            XScriptContext xSc = ScriptContext.createContext(invocationCtx,
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

            LogUtils.DEBUG( "Classloader starting..." );
            ClassLoader scriptLoader = null;

            try
            {
                scriptLoader = ClassLoaderFactory.getClassLoader(m_xContext,
                    scriptDesc.getClass().getClassLoader(),
                    scriptDesc.getClasspath() );
            }
            catch ( NoSuitableClassLoaderException e )
            {
                String trace = LogUtils.getTrace( e );
                LogUtils.DEBUG( trace );
                throw new InvocationTargetException( trace );
            }

            LogUtils.DEBUG( "ScriptProxy starting... " );
            ScriptProxy script = null;
            try
            {
                String className = m_xScriptInfo.getFunctionName().substring( 0,
                    m_xScriptInfo.getFunctionName().lastIndexOf( '.' ) );
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
                    String trace = LogUtils.getTrace( e );
                    LogUtils.DEBUG( trace );
                    throw new InvocationTargetException( trace );
                }
            }
            catch ( ClassNotFoundException e )
            {
                String trace = LogUtils.getTrace( e );
                LogUtils.DEBUG( trace );
                throw new InvocationTargetException( trace );
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
            catch ( IllegalArgumentException iae )
            {
                String trace = LogUtils.getTrace( iae );
                LogUtils.DEBUG( trace );
                throw new IllegalArgumentException( trace );
            }
            catch ( InvocationTargetException ite )
            {
                String trace = LogUtils.getTrace( ite );
                LogUtils.DEBUG( trace );
                throw new InvocationTargetException( trace );
            }
            catch ( IllegalAccessException illae )
            {
                String trace = LogUtils.getTrace( illae );
                LogUtils.DEBUG( trace );
                throw new IllegalArgumentException( trace );
            }
            catch ( Exception e )
            {
                String trace = LogUtils.getTrace( e );
                LogUtils.DEBUG( trace );
                throw new InvocationTargetException( trace );
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

        public Object invoke( /*IN*/Object[] aParams,
                            /*OUT*/short[][] aOutParamIndex,
                            /*OUT*/Object[][] aOutParam )
            throws IllegalArgumentException, CannotConvertException,
                InvocationTargetException
        {
            LogUtils.DEBUG( "ScriptProviderForJava: starting invoke" );
            return invoke(m_scriptURI, m_xInvocationContext, aParams,
                            aOutParamIndex, aOutParam);
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
