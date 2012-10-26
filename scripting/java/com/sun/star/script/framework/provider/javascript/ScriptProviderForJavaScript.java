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
package com.sun.star.script.framework.provider.javascript;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.frame.XModel;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.document.XScriptInvocationContext;
import com.sun.star.reflection.InvocationTargetException;

import java.net.URL;

import com.sun.star.script.provider.XScript;

import com.sun.star.script.provider.ScriptExceptionRaisedException;
import com.sun.star.script.provider.ScriptFrameworkErrorException;
import com.sun.star.script.provider.ScriptFrameworkErrorType;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.ScriptContext;
import com.sun.star.script.framework.provider.ClassLoaderFactory;
import com.sun.star.script.framework.provider.ScriptProvider;
import com.sun.star.script.framework.provider.ScriptEditor;
import com.sun.star.script.framework.container.ScriptMetaData;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.ImporterTopLevel;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.JavaScriptException;

public class ScriptProviderForJavaScript
{
    public static class ScriptProviderForJavaScript_2 extends ScriptProvider
    {
        public ScriptProviderForJavaScript_2(XComponentContext ctx)
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
                ScriptImpl script = new ScriptImpl( m_xContext, scriptData, m_xModel, m_xInvocContext );
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

        if ( implName.equals( ScriptProviderForJavaScript.ScriptProviderForJavaScript_2.class.getName() ) )
        {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(
                ScriptProviderForJavaScript.ScriptProviderForJavaScript_2.class,
                "com.sun.star.script.provider.ScriptProviderForJavaScript",
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
    private XMultiComponentFactory m_xMultiComponentFactory;
    private XModel m_xModel;
    private XScriptInvocationContext m_xInvocContext;

    ScriptImpl( XComponentContext ctx, ScriptMetaData metaData, XModel xModel, XScriptInvocationContext xInvocContext ) throws com.sun.star.uno.RuntimeException
    {
        this.metaData = metaData;
        this.m_xContext = ctx;
        this.m_xModel = xModel;
        this.m_xInvocContext = xInvocContext;
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
         * @param params            All parameters; pure, out params are
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
            // Initialise the out parameters - not used at the moment
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
                    ScriptFrameworkErrorType.MALFORMED_URL );
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
                       m_xModel, m_xInvocContext, m_xContext,
                       m_xMultiComponentFactory), scope);
                scope.put("XSCRIPTCONTEXT", scope, jsCtxt);

                Scriptable jsArgs = Context.toObject(params, scope);
                scope.put("ARGUMENTS", scope, jsArgs);

                result = ctxt.evaluateString(scope,
                        source, "<stdin>", 1, null);
                result = ctxt.toString(result);
                return result;
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
                        ScriptContext.createContext(m_xModel, m_xInvocContext,
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

