/*************************************************************************
*
*  $RCSfile: ScriptProviderForJavaScript.java,v $
*
*  $Revision: 1.5 $
*
*  last change: $Author: hr $ $Date: 2004-07-23 14:05:08 $
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
import com.sun.star.frame.XModel;
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
import java.util.Map;
import java.net.MalformedURLException;
import java.net.URL;

import drafts.com.sun.star.script.provider.XScriptProvider;
import drafts.com.sun.star.script.provider.XScript;
import drafts.com.sun.star.script.provider.XScriptContext;

import drafts.com.sun.star.script.provider.ScriptErrorRaisedException;
import drafts.com.sun.star.script.provider.ScriptExceptionRaisedException;
import drafts.com.sun.star.script.provider.ScriptFrameworkErrorException;
import drafts.com.sun.star.script.provider.ScriptFrameworkErrorType;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.ScriptContext;
import com.sun.star.script.framework.provider.ClassLoaderFactory;
import com.sun.star.script.framework.provider.ScriptProvider;
import com.sun.star.script.framework.provider.ScriptEditor;
import com.sun.star.script.framework.container.ScriptMetaData;
import com.sun.star.script.framework.log.*;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.ImporterTopLevel;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.JavaScriptException;
import org.mozilla.javascript.EcmaError;

public class ScriptProviderForJavaScript
{
    public static class _ScriptProviderForJavaScript extends ScriptProvider
    {
        public _ScriptProviderForJavaScript(XComponentContext ctx)
        {
            super(ctx, "JavaScript");
        }

        public XScript getScript( /*IN*/String scriptURI )
            throws com.sun.star.uno.RuntimeException,
                   ScriptFrameworkErrorException
        {
            ScriptMetaData scriptData = null;
            try
            {
                scriptData = getScriptData( scriptURI );
                ScriptImpl script = new ScriptImpl( m_xContext, scriptData, m_xModel );
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
            return true;
        }

        public ScriptEditor getScriptEditor()
        {
            return ScriptEditorForJavaScript.getEditor();
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
                "drafts.com.sun.star.script.provider.ScriptProviderForJavaScript",
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

        String service = "drafts.com.sun.star.script.provider." +
            "ScriptProviderForJavaScript";

        if (FactoryHelper.writeRegistryServiceInfo(impl, service, regKey)) {
            try {
                // code below is commented out because we want this to happen
                // as part of the install, this will have to be done
                // programatically during the install as registration of java components
                // is not fully supported in setup. It should work somewhat like c++ somewhat like c++ registration in install
/*                XRegistryKey newKey = regKey.createKey(impl + "/UNO/SINGLETONS/drafts.com.sun.star.script.provider.theScriptProviderForJavaScript");
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
class ScriptImpl implements XScript
{
    private ScriptMetaData metaData;
    private XComponentContext m_xContext;
    private XMultiComponentFactory m_xMultiComponentFactory;
    private XModel m_xModel;

    ScriptImpl( XComponentContext ctx, ScriptMetaData metaData, XModel xModel ) throws com.sun.star.uno.RuntimeException
    {
        this.metaData = metaData;
        this.m_xContext = ctx;
        this.m_xModel = xModel;
        try
        {
            this.m_xMultiComponentFactory = m_xContext.getServiceManager();
        }
        catch ( Exception e )
        {
            LogUtils.DEBUG( LogUtils.getTrace( e ) );
            throw new com.sun.star.uno.RuntimeException(
                "Error constructing  ScriptImpl: [javascript]");
        }
        LogUtils.DEBUG("ScriptImpl [javascript] script data = " + metaData );
    }

        /**
         *  The invoke method of the ScriptProviderForJavaScript runs the
         *  JavaScript script specified in the URI
         *
         *
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
         * @throws ScriptFrameworkErrorException If there is no matching script name
         *
         *
         * @throws InvocationTargetException If the running script throws
         *                                   an exception this information
         *                                   is captured and rethrown as
         *                                   ScriptErrorRaisedException or
         *                                   ScriptExceptionRaisedException
         */

        public Object invoke(
                               /*IN*/Object[]  params,
                               /*OUT*/short[][]  aOutParamIndex,
                               /*OUT*/Object[][]  aOutParam )

        throws ScriptFrameworkErrorException, InvocationTargetException
        {
            // Initialise the out paramters - not used at the moment
            aOutParamIndex[0] = new short[0];
            aOutParam[0] = new Object[0];



            ClassLoader cl = null;
            URL sourceUrl = null;
            try {
                cl = ClassLoaderFactory.getURLClassLoader( metaData );
                sourceUrl = metaData.getSourceURL();
            }
            catch ( java.net.MalformedURLException mfu )
            {
                throw new ScriptFrameworkErrorException(
                    mfu.getMessage(), null,
                    metaData.getLanguageName(), metaData.getLanguage(),
                    ScriptFrameworkErrorType.UNKNOWN );
            }
            catch ( com.sun.star.script.framework.provider.NoSuitableClassLoaderException nsc )
            {
                // Framework error
                throw new ScriptFrameworkErrorException(
                    nsc.getMessage(), null,
                    metaData.getLanguageName(), metaData.getLanguage(),
                    ScriptFrameworkErrorType.UNKNOWN );
            }
            Context ctxt = null;

            try
            {
                String editorURL = sourceUrl.toString();
                Object result = null;
                String source = null;
                ScriptEditorForJavaScript editor =
                    ScriptEditorForJavaScript.getEditor(
                        metaData.getSourceURL() );

                if (editor != null)
                {
                    editorURL = editor.getURL();
                    result = editor.execute();
                    if ( result != null  &&
                         result.getClass().getName().equals( "org.mozilla.javascript.Undefined" ) )
                    {
                        // Always return a string
                        // TODO revisit
                        return Context.toString( result );
                    }

                }

                if (editor != null && editor.isModified() == true)
                {
                    LogUtils.DEBUG("GOT A MODIFIED SOURCE");
                    source = editor.getText();
                }
                else
                {
                    metaData.loadSource();
                    source =  metaData.getSource();

                }

                if ( source == null || source.length() == 0 ) {
                    throw new ScriptFrameworkErrorException(
                        "Failed to read source data for script", null,
                        metaData.getLanguageName(), metaData.getLanguage(),
                        ScriptFrameworkErrorType.UNKNOWN );
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
                ctxt = Context.enter();

                /* The ImporterTopLevel ensures that importClass and
                   importPackage statements work in Javascript scripts
                   Make the XScriptContext available as a global variable
                   to the script
                 */
                ImporterTopLevel scope = new ImporterTopLevel(ctxt);

                Scriptable jsCtxt = Context.toObject(
                   ScriptContext.createContext(
                       m_xModel, m_xContext,
                       m_xMultiComponentFactory), scope);
                scope.put("XSCRIPTCONTEXT", scope, jsCtxt);

                Scriptable jsArgs = Context.toObject(params, scope);
                scope.put("ARGUMENTS", scope, jsArgs);

                result = ctxt.evaluateString(scope,
                    source, editorURL, 1, null);

                result = ctxt.toString(result);
                return result;
            }
            catch (EcmaError ec) {
                LogUtils.DEBUG( "Caught EcmaError exception for JavaScript type = " + ec.getClass() );
                String message = ec.getMessage();
                int lineNo = ec.getLineNumber();
                LogUtils.DEBUG( "\t message  " + message );
                LogUtils.DEBUG( "\t lineNum  " + lineNo );
                ScriptErrorRaisedException se = new
                    ScriptErrorRaisedException( message );
                se.lineNum = lineNo;
                se.scriptName = metaData.getLanguageName();
                se.language = "JavaScript";
                LogUtils.DEBUG( "ErrorRaised exception  "  );
                LogUtils.DEBUG( "\t message  " + se.getMessage() );
                LogUtils.DEBUG( "\t lineNum  " + se.lineNum );
                LogUtils.DEBUG( "\t language  " + se.language );
                LogUtils.DEBUG( "\t scriptName  " + se.scriptName );
                raiseEditor( se.lineNum );
                throw new InvocationTargetException( "JavaScript error " + metaData.getLanguageName(), null, se );

            }
            catch (JavaScriptException jse) {
                LogUtils.DEBUG( "Caught JavaScriptException exception for JavaScript type = " + jse.getClass() );
                String message = jse.getMessage();
                //int lineNo = jse.getLineNumber();
                Object wrap = jse.getValue();
                LogUtils.DEBUG( "\t message  " + message );
                LogUtils.DEBUG( "\t wrapped type " + wrap.getClass() );
                LogUtils.DEBUG( "\t wrapped toString  " + wrap.toString() );
                ScriptExceptionRaisedException se = new
                    ScriptExceptionRaisedException( message );
                se.lineNum = -1;
                se.language = "JavaScript";
                se.scriptName = metaData.getLanguageName();
                se.exceptionType = wrap.getClass().getName();
                se.language = metaData.getLanguage();
                LogUtils.DEBUG( "ExceptionRaised exception  "  );
                LogUtils.DEBUG( "\t message  " + se.getMessage() );
                LogUtils.DEBUG( "\t lineNum  " + se.lineNum );
                LogUtils.DEBUG( "\t language  " + se.language );
                LogUtils.DEBUG( "\t scriptName  " + se.scriptName );
                raiseEditor( se.lineNum );
                throw new InvocationTargetException( "JavaScript uncaught exception" + metaData.getLanguageName(), null, se );
            }
            catch (Exception ex) {
                LogUtils.DEBUG("Caught Exception " + ex );
                LogUtils.DEBUG("rethrowing as ScriptFramework error"  );
                throw new ScriptFrameworkErrorException(
                    ex.getMessage(), null,
                    metaData.getLanguageName(), metaData.getLanguage(),
                    ScriptFrameworkErrorType.UNKNOWN );
            }
            finally {
                if ( ctxt != null )
                {
                    Context.exit();
                }
            }
        }

        private void raiseEditor( int lineNum )
        {
            ScriptEditorForJavaScript editor = null;
            try
            {
                URL sourceUrl = metaData.getSourceURL();
                editor = ScriptEditorForJavaScript.getEditor( sourceUrl );
                if ( editor == null )
                {
                    editor = ScriptEditorForJavaScript.getEditor();
                    editor.edit(
                        ScriptContext.createContext(m_xModel,
                            m_xContext, m_xMultiComponentFactory), metaData );
                    editor = ScriptEditorForJavaScript.getEditor( sourceUrl );
                }
                if ( editor != null )
                {
                    System.out.println("** Have raised IDE for JavaScript, calling indicateErrorLine for line " + lineNum );
                    editor.indicateErrorLine( lineNum );
                }
            }
            catch( Exception ignore )
            {
            }
        }
}

