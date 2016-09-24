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
package com.sun.star.script.framework.provider.beanshell;

import bsh.Interpreter;

import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.document.XScriptInvocationContext;

import com.sun.star.frame.XModel;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.reflection.InvocationTargetException;

import com.sun.star.registry.XRegistryKey;

import com.sun.star.script.framework.container.ScriptMetaData;
import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.ClassLoaderFactory;
import com.sun.star.script.framework.provider.ScriptContext;
import com.sun.star.script.framework.provider.ScriptEditor;
import com.sun.star.script.framework.provider.ScriptProvider;
import com.sun.star.script.provider.ScriptErrorRaisedException;
import com.sun.star.script.provider.ScriptExceptionRaisedException;
import com.sun.star.script.provider.ScriptFrameworkErrorException;
import com.sun.star.script.provider.ScriptFrameworkErrorType;
import com.sun.star.script.provider.XScript;

import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;

import java.net.URL;

import java.util.StringTokenizer;

public class ScriptProviderForBeanShell {

    public static class ScriptProviderForBeanShell_2 extends ScriptProvider {

        public ScriptProviderForBeanShell_2(XComponentContext ctx) {
            super(ctx, "BeanShell");
        }

        @Override
        public XScript getScript(/*IN*/String scriptURI) throws
            com.sun.star.uno.RuntimeException, ScriptFrameworkErrorException {

            ScriptMetaData scriptData = getScriptData(scriptURI);

            try {

                ScriptImpl script =
                    new ScriptImpl(m_xContext, scriptData, m_xModel, m_xInvocContext);

                return script;
            } catch (com.sun.star.uno.RuntimeException re) {
                throw new ScriptFrameworkErrorException(
                    "Failed to create script object: " + re.getMessage(),
                    null, scriptData.getLanguageName(), language,
                    ScriptFrameworkErrorType.UNKNOWN);
            }
        }

        @Override
        public boolean hasScriptEditor() {
            return true;
        }

        @Override
        public ScriptEditor getScriptEditor() {
            return ScriptEditorForBeanShell.getEditor();
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
    public static XSingleServiceFactory __getServiceFactory(
        String implName, XMultiServiceFactory multiFactory, XRegistryKey regKey) {

        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(
                ScriptProviderForBeanShell.ScriptProviderForBeanShell_2.class.getName())) {

            xSingleServiceFactory =
                FactoryHelper.getServiceFactory(
                    ScriptProviderForBeanShell.ScriptProviderForBeanShell_2.class,
                    "com.sun.star.script.provider.ScriptProviderForBeanShell",
                    multiFactory, regKey);

        }

        return xSingleServiceFactory;
    }
}

class ScriptImpl implements XScript {

    private final ScriptMetaData metaData;
    private final XComponentContext m_xContext;
    private XMultiComponentFactory m_xMultiComponentFactory;
    private final XModel m_xModel;
    private final XScriptInvocationContext m_xInvocContext;

    ScriptImpl(XComponentContext ctx, ScriptMetaData metaData, XModel xModel,
               XScriptInvocationContext xContext) throws
        com.sun.star.uno.RuntimeException {

        this.metaData = metaData;
        this.m_xContext = ctx;
        this.m_xModel = xModel;
        this.m_xInvocContext = xContext;

        try {
            this.m_xMultiComponentFactory = m_xContext.getServiceManager();
        } catch (Exception e) {
            throw new com.sun.star.uno.RuntimeException(e);
        }

        LogUtils.DEBUG("ScriptImpl [beanshell] script data = " + metaData);
    }

    /**
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
     * @return                  The value returned from the function
     *                          being invoked
     *
     * @throws IllegalArgumentException If there is no matching script name
     *
     * @throws InvocationTargetException If the running script throws
     *                                   an exception this information
     *                                   is captured and rethrown as
     *                                   this exception type.
     */

    public Object invoke(/*IN*/Object[] aParams,
                               /*OUT*/short[][] aOutParamIndex,
                               /*OUT*/Object[][] aOutParam) throws
        ScriptFrameworkErrorException, InvocationTargetException {

        // Initialise the out parameters - not used at the moment
        aOutParamIndex[0] = new short[0];
        aOutParam[0] = new Object[0];

        ClassLoader cl = null;
        URL sourceUrl = null;

        try {
            cl = ClassLoaderFactory.getURLClassLoader(metaData);
            sourceUrl = metaData.getSourceURL();
        } catch (java.net.MalformedURLException mfu) {
            // Framework error
            throw new ScriptFrameworkErrorException(
                mfu.getMessage(), null,
                metaData.getLanguageName(), metaData.getLanguage(),
                ScriptFrameworkErrorType.MALFORMED_URL);
        }

        // Set class loader to be used for class files
        // and jar files
        Thread.currentThread().setContextClassLoader(cl);
        Interpreter interpreter = new Interpreter();

        interpreter.getNameSpace().clear();
        // Set class loader to be used by interpreter
        // to look for classes by source e.g. interpreter
        // will use this classloader to search classpath
        // for source file ( bla.java ) on import or reference
        interpreter.setClassLoader(cl);

        try {

            interpreter.set("XSCRIPTCONTEXT",
                            ScriptContext.createContext(m_xModel, m_xInvocContext,
                                    m_xContext, m_xMultiComponentFactory));

            interpreter.set("ARGUMENTS", aParams);
        } catch (bsh.EvalError e) {
            // Framework error setting up context
            throw new ScriptFrameworkErrorException(
                e.getMessage(), null,
                metaData.getLanguageName(), metaData.getLanguage(),
                ScriptFrameworkErrorType.UNKNOWN);
        }

        try {
            Object result;

            ScriptEditorForBeanShell editor =
                ScriptEditorForBeanShell.getEditor(sourceUrl);

            if (editor != null) {
                result = editor.execute();

                if (result == null) {
                    return new Any(new Type(), null);
                }

                return result;
            }

            metaData.loadSource();
            String source = metaData.getSource();

            if (source == null || source.length() == 0) {
                throw new ScriptFrameworkErrorException(
                    "Failed to read script", null,
                    metaData.getLanguageName(), metaData.getLanguage(),
                    ScriptFrameworkErrorType.NO_SUCH_SCRIPT);
            }

            result = interpreter.eval(source);

            if (result == null) {
                return new Any(new Type(), null);
            }

            return result;
        } catch (bsh.ParseException pe) {
            throw new InvocationTargetException(
                "Beanshell failed to parse " + metaData.getLanguageName(),
                null, processBshException(pe, metaData.getLanguageName()));
        } catch (bsh.TargetError te) {
            throw new InvocationTargetException(
                "Beanshell uncaught exception for " + metaData.getLanguageName(),
                null, processBshException(te, metaData.getLanguageName()));
        } catch (bsh.EvalError ex) {
            throw new InvocationTargetException(
                "Beanshell error for " + metaData.getLanguageName(),
                null, processBshException(ex, metaData.getLanguageName()));
        } catch (Exception e) {
            throw new ScriptFrameworkErrorException(
                "Failed to read script", null, metaData.getLanguageName(),
                metaData.getLanguage(), ScriptFrameworkErrorType.UNKNOWN);
        }
    }
    private void raiseEditor(int lineNum) {
        try {
            URL sourceUrl = metaData.getSourceURL();

            ScriptEditorForBeanShell editor =
                ScriptEditorForBeanShell.getEditor(sourceUrl);

            if (editor == null) {
                editor = ScriptEditorForBeanShell.getEditor();

                editor.edit(
                    ScriptContext.createContext(m_xModel, m_xInvocContext, m_xContext,
                                                m_xMultiComponentFactory), metaData);

                editor = ScriptEditorForBeanShell.getEditor(sourceUrl);
            }

            if (editor != null) {
                editor.indicateErrorLine(lineNum);
            }
        } catch (java.net.MalformedURLException ignore) {
        }
    }

    private ScriptErrorRaisedException processBshException(
        bsh.EvalError e, String script) {

        LogUtils.DEBUG("Beanshell error RAW message " + e.getMessage());
        String message = e.getMessage();
        int usefulInfoIndex = message.lastIndexOf("\' :");
        int lineNum = e.getErrorLineNumber();

        raiseEditor(lineNum);

        if (usefulInfoIndex > -1) {
            message = message.substring(usefulInfoIndex + 2);
        }

        if (e instanceof bsh.TargetError) {
            LogUtils.DEBUG("got instance of  TargetError");

            if (usefulInfoIndex == -1) {
                message = ((bsh.TargetError)e).getTarget().getMessage();
            }

            String wrappedException = "";
            String full = e.toString();
            int index = full.indexOf("Target exception:");

            if (index > -1) {
                String toParse = full.substring(index);
                LogUtils.DEBUG("About to parse " + toParse);

                StringTokenizer tokenizer =
                    new StringTokenizer(full.substring(index), ":");

                if (tokenizer.countTokens() > 2) {
                    LogUtils.DEBUG("First token = " + tokenizer.nextToken());
                    wrappedException = tokenizer.nextToken();
                    LogUtils.DEBUG("wrapped exception = = " + wrappedException);
                }
            }

            ScriptExceptionRaisedException se =
                new ScriptExceptionRaisedException(message);

            se.lineNum = lineNum;
            se.scriptName = script;
            se.exceptionType = wrappedException;
            se.language = "BeanShell";

            LogUtils.DEBUG("UnCaught Exception error: ");
            LogUtils.DEBUG("\tscript: " + script);
            LogUtils.DEBUG("\tline: " + lineNum);
            LogUtils.DEBUG("\twrapped exception: " + wrappedException);
            LogUtils.DEBUG("\tmessage: " + message);

            return se;

        } else {
            LogUtils.DEBUG("Error or ParseError Exception error: ");
            LogUtils.DEBUG("\tscript: " + script);
            LogUtils.DEBUG("\tline: " + lineNum);
            LogUtils.DEBUG("\tmessage: " + message);

            return new ScriptErrorRaisedException(message, null, script,
                                                  "BeanShell", lineNum);

        }
    }
}
