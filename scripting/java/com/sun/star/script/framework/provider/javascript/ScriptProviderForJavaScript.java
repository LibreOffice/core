/*************************************************************************
*
*  $RCSfile: ScriptProviderForJavaScript.java,v $
*
*  $Revision: 1.1 $
*
*  last change: $Author: toconnor $ $Date: 2003-09-10 10:44:35 $
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
package com.sun.star.script.framework.provider.javascript;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XInitialization;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;
import com.sun.star.beans.XPropertySet;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.reflection.InvocationTargetException;
import com.sun.star.script.CannotConvertException;

import java.io.*;
import java.util.Vector;
import java.net.MalformedURLException;

import drafts.com.sun.star.script.framework.provider.XScriptProvider;
import drafts.com.sun.star.script.framework.provider.XScript;
import drafts.com.sun.star.script.framework.runtime.XScriptContext;
import drafts.com.sun.star.script.framework.storage.XScriptInfo;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.PathUtils;
import com.sun.star.script.framework.provider.ScriptContext;
import com.sun.star.script.framework.provider.ClassLoaderFactory;
import com.sun.star.script.framework.provider.ScriptProvider;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.ImporterTopLevel;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.JavaScriptException;

public class ScriptProviderForJavaScript
{
    public static class _ScriptProviderForJavaScript extends ScriptProvider
    {
        public _ScriptProviderForJavaScript(XComponentContext ctx)
        {
            super(ctx, "JavaScript");
        }

        /**
         *  The invoke method of the ScriptProviderForJavaScript runs the
         *  JavaScript script specified in the URI
         *
         *
         * @param scriptName        The scriptName is the language specific
         *                          name of the script
         *
         * @param invocationCtx     The invocation context contains the
         *                          documentStorageID and document reference
         *                          for use in script name resolving
         *
         * @param aParams           All parameters; pure, out params are
         *                          undefined in sequence, i.e., the value
         *                          has to be ignored by the callee
         *
         * @param aOutParamIndex    Out indices
         *
         * @param aOutParam         Out parameters
         *
         * @returns                 The value returned from the function
         *                          being invoked
         *
         * @throws IllegalArgumentException If there is no matching script name
         *
         * @throws CannotConvertException   If args do not match or cannot
         *                                  be converted the those of the
         *                                  invokee
         *
         * @throws InvocationTargetException If the running script throws
         *                                   an exception this information
         *                                   is captured and rethrown as
         *                                   this exception type.
         */

        private Object invoke(  /*IN*/String scriptURI,
                               /*IN*/Object invocationCtx,
                               /*IN*/Object[]  params,
                               /*OUT*/short[][]  aOutParamIndex,
                               /*OUT*/Object[][]  aOutParam )

        throws IllegalArgumentException, InvocationTargetException,
               CannotConvertException
        {
            // Initialise the out paramters - not used at the moment
            aOutParamIndex[0] = new short[0];
            aOutParam[0] = new Object[0];

            XPropertySet languageProps = m_xScriptInfo.getLanguageProperties();
            String cp = null;

            // Set up ClassLoader based on property defined with script
            try {
                cp = (String)languageProps.getPropertyValue(CLASSPATH);
            }
            catch (Exception e) {
            }

            if (cp == null) {
                cp = "";
            }

            String parcelURI = m_xScriptInfo.getParcelURI();
            if (!parcelURI.endsWith("/")) {
                parcelURI += "/";
            }

            Vector classpath;
            try {
                classpath = PathUtils.buildClasspath(parcelURI, cp);
            }
            catch (MalformedURLException mue) {
                throw new InvocationTargetException(mue.getMessage());
            }

            ClassLoader cl = null;
            try {
                cl = ClassLoaderFactory.getClassLoader(m_xContext,
                    this.getClass().getClassLoader(), classpath);
            }
            catch (Exception e)
            {
                throw new InvocationTargetException(e.getMessage());
            }

            try {
                String script = parcelURI + m_xScriptInfo.getFunctionName();
                InputStream is;
                Object result = null;

                try {
                    is = PathUtils.getScriptFileStream( script, m_xContext );
                }
                catch (IOException ioe) {
                    throw new InvocationTargetException(ioe.getMessage());
                }

                if (is == null) {
                    throw new InvocationTargetException("Could not load script");
                }

                /* Set the context ClassLoader on the current thread to
                   be our custom ClassLoader. This is the suggested method
                   for setting up a ClassLoader to be used by the Rhino
                   interpreter
                 */
                if (cl != null) {
                    Thread.currentThread().setContextClassLoader(cl);
                }

                // Initialize a Rhino Context object
                Context ctxt = Context.enter();

                /* The ImporterTopLevel ensures that importClass and
                   importPackage statements work in Javascript scripts
                   Make the XScriptContext available as a global variable
                   to the script
                 */
                try {
                    ImporterTopLevel scope = new ImporterTopLevel(ctxt);
                    Scriptable jsArgs = Context.toObject(
                        ScriptContext.createContext(invocationCtx, m_xContext,
                            m_xMultiComponentFactory), scope);
                    scope.put("XSCRIPTCONTEXT", scope, jsArgs);

                    result = ctxt.evaluateReader(scope,
                        new InputStreamReader(is), "script", 1, null);

                    result = ctxt.toString(result);
                }
                catch (JavaScriptException jse) {
                    jse.printStackTrace();
                    throw new InvocationTargetException(jse.getMessage());
                }
                catch (IOException ioe) {
                    ioe.printStackTrace();
                    throw new InvocationTargetException(ioe.getMessage());
                }
                finally {
                    Context.exit();
                    try {is.close();} catch (IOException ignored) {}
                }

                return result;
            }
            catch (Exception ex) {
                ex.printStackTrace();
                throw new InvocationTargetException(ex.getMessage());
            }
        }

        public Object invoke( /*IN*/Object[] aParams,
                            /*OUT*/short[][] aOutParamIndex,
                            /*OUT*/Object[][] aOutParam )
            throws IllegalArgumentException, CannotConvertException,
                InvocationTargetException
        {
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

        if ( implName.equals( ScriptProviderForJavaScript._ScriptProviderForJavaScript.class.getName() ) )
        {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(
                ScriptProviderForJavaScript._ScriptProviderForJavaScript.class,
                "drafts.com.sun.star.script.framework.provider.ScriptProviderForJavaScript",
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
        String impl = "com.sun.star.scripting.runtime.javascript." +
            "ScriptProviderForJavaScript$_ScriptProviderForJavaScript";

        String service = "drafts.com.sun.star.script.framework.provider." +
            "ScriptProviderForJavaScript";

        if (FactoryHelper.writeRegistryServiceInfo(impl, service, regKey)) {
            try {
                // code below is commented out because we want this to happen
                // as part of the install, this will have to be done
                // programatically during the install as registration of java components
                // is not fully supported in setup. It should work somewhat like c++ somewhat like c++ registration in install
/*                XRegistryKey newKey = regKey.createKey(impl + "/UNO/SINGLETONS/drafts.com.sun.star.script.framework.provider.theScriptProviderForJavaScript");
                newKey.setStringValue(service); */
                return true;
            }
            catch (Exception ex) {
                System.err.println(
                    "Error registering ScriptProviderForJavaScript: " + ex);
            }
        }
        return false;
    }
}
