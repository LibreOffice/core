/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package com.sun.star.script.framework.provider.javascript;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.ImporterTopLevel;
import org.mozilla.javascript.tools.debugger.Main;
import org.mozilla.javascript.tools.debugger.ScopeProvider;

import com.sun.star.script.provider.XScriptContext;
import com.sun.star.script.framework.container.ScriptMetaData;
import com.sun.star.script.framework.provider.ScriptEditor;
import com.sun.star.script.framework.provider.SwingInvocation;
import com.sun.star.script.framework.log.LogUtils;

import java.io.InputStream;
import java.io.IOException;
import java.net.URL;

import java.util.Map;
import java.util.HashMap;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

public class ScriptEditorForJavaScript implements ScriptEditor
{
    // global ScriptEditorForJavaScript instance
    private static ScriptEditorForJavaScript theScriptEditorForJavaScript;

    // template for JavaScript scripts
    private static String JSTEMPLATE;

    static private Main rhinoWindow;
    private URL scriptURL;
    // global list of ScriptEditors, key is URL of file being edited
    private static Map BEING_EDITED = new HashMap();

    static {
        try {
            URL url =
                ScriptEditorForJavaScript.class.getResource("template.js");

            InputStream in = url.openStream();
            StringBuffer buf = new StringBuffer();
            byte[] b = new byte[1024];
            int len = 0;

            while ((len = in.read(b)) != -1) {
                buf.append(new String(b, 0, len));
            }

            in.close();

            JSTEMPLATE = buf.toString();
        }
        catch (IOException ioe) {
            JSTEMPLATE = "// JavaScript script";
        }
        catch (Exception e) {
            JSTEMPLATE = "// JavaScript script";
        }
    }

    /**
     *  Returns the global ScriptEditorForJavaScript instance.
     */
    public static ScriptEditorForJavaScript getEditor()
    {
        if (theScriptEditorForJavaScript == null)
        {
            synchronized(ScriptEditorForJavaScript.class)
            {
                if (theScriptEditorForJavaScript == null)
                {
                    theScriptEditorForJavaScript =
                        new ScriptEditorForJavaScript();
                }
            }
        }
        return theScriptEditorForJavaScript;
    }

    /**
     *  Get the ScriptEditorForJavaScript instance for this URL
     *
     * @param  url         The URL of the script source file
     *
     * @return             The ScriptEditorForJavaScript associated with
     *                     the given URL if one exists, otherwise null.
     */
    public static ScriptEditorForJavaScript getEditor(URL url)
    {
        synchronized (BEING_EDITED) {
            return (ScriptEditorForJavaScript)BEING_EDITED.get(url);
        }
    }

    /**
     *  Returns whether or not the script source being edited in this
     *  ScriptEditorForJavaScript has been modified
     */
    public boolean isModified()
    {
        return rhinoWindow.isModified( scriptURL );
    }

    /**
     *  Returns the text being displayed in this ScriptEditorForJavaScript
     *
     *  @return            The text displayed in this ScriptEditorForJavaScript
     */
    public String getText()
    {
        return rhinoWindow.getText( scriptURL );
    }

    /**
     *  Returns the Rhino Debugger url of this ScriptEditorForJavaScript
     *
     *  @return            The url of this ScriptEditorForJavaScript
     */
    public String getURL()
    {
        return scriptURL.toString();
    }

    /**
     *  Returns the template text for JavaScript scripts
     *
     *  @return            The template text for JavaScript scripts
     */
    public String getTemplate()
    {
        return JSTEMPLATE;
    }

    /**
     *  Returns the default extension for JavaScript scripts
     *
     *  @return            The default extension for JavaScript scripts
     */
    public String getExtension()
    {
        return "js";
    }

    /**
     *  Opens an editor window for the specified ScriptMetaData.
     *  If an editor window is already open for that data it will be
     *  moved to the front.
     *
     * @param  metadata    The metadata describing the script
     * @param  context     The context in which to execute the script
     *
     */
    public void edit(final XScriptContext context, ScriptMetaData entry)
    {
        try {
            String sUrl = entry.getParcelLocation();
            if ( !sUrl.endsWith( "/" ) )
            {
                sUrl += "/";
            }
            sUrl +=  entry.getLanguageName();
            final URL url = entry.getSourceURL();
            SwingInvocation.invoke(
                new Runnable() {
                    public void run() {
                        synchronized (BEING_EDITED) {
                            ScriptEditorForJavaScript editor =
                                (ScriptEditorForJavaScript) BEING_EDITED.get(
                                    url);
                            if (editor == null) {
                                editor = new ScriptEditorForJavaScript(
                                    context, url);
                                BEING_EDITED.put(url, editor);
                            }
                        }
                        assert rhinoWindow != null;
                        rhinoWindow.showScriptWindow(url);
                        rhinoWindow.toFront();
                    }
                });
        }
        catch ( IOException e )
        {
            LogUtils.DEBUG("Caught exception: " + e);
            LogUtils.DEBUG(LogUtils.getTrace(e));
        }
    }

    // Ensures that new instances of this class can only be created using
    // the factory methods
    private ScriptEditorForJavaScript()
    {
    }

    private ScriptEditorForJavaScript(XScriptContext context, URL url)
    {
        initUI();
        Scriptable scope = getScope( context );
        this.rhinoWindow.openFile(url, scope, new closeHandler( url ) );


        this.scriptURL = url;
    }

    /**
     *  Executes the script edited by the editor
     *
     */

    public Object execute() throws Exception
    {
        rhinoWindow.toFront();

        return this.rhinoWindow.runScriptWindow( scriptURL );
    }

    /**
     *  Indicates the line where error occurred
     *
     */
    public void indicateErrorLine( int lineNum )
    {
        this.rhinoWindow.toFront();
        this.rhinoWindow.highlighLineInScriptWindow( scriptURL, lineNum );
    }
    // This code is based on the main method of the Rhino Debugger Main class
    // We pass in the XScriptContext in the global scope for script execution
    private void initUI() {
        try {
            synchronized ( ScriptEditorForJavaScript.class )
            {
                if ( this.rhinoWindow != null )
                {
                    return;
                }

                final Main sdb = new Main("Rhino JavaScript Debugger");
                sdb.pack();
                sdb.setSize(640, 640);
                sdb.setVisible(true);
                sdb.setExitAction(new Runnable() {
                    public void run() {
                        sdb.clearAllBreakpoints();
                        sdb.dispose();
                        shutdown();
                    }
                });
                Context.addContextListener(sdb);
                sdb.setScopeProvider(new ScopeProvider() {
                    public Scriptable getScope() {
                        return org.mozilla.javascript.tools.shell.Main.getScope();
                    }
                });
                sdb.addWindowListener( new WindowAdapter() {
                    public void windowClosing(WindowEvent e) {
                        shutdown();
                    }
                });
                this.rhinoWindow = sdb;
            }
        } catch (Exception exc) {
            LogUtils.DEBUG( LogUtils.getTrace( exc ) );
        }
    }

    private void shutdown()
    {
        // dereference Rhino Debugger window
        this.rhinoWindow = null;
        this.scriptURL = null;
        // remove all scripts from BEING_EDITED
        synchronized( BEING_EDITED )
        {
            java.util.Iterator iter = BEING_EDITED.keySet().iterator();
            java.util.Vector keysToRemove = new java.util.Vector();
            while ( iter.hasNext() )
            {

                URL key = (URL)iter.next();
                keysToRemove.add( key );
            }
            for ( int i=0; i<keysToRemove.size(); i++ )
            {
                BEING_EDITED.remove( keysToRemove.elementAt( i ) );
            }
            keysToRemove = null;
        }

    }
    private Scriptable getScope(XScriptContext xsctxt )
    {
        Context ctxt = Context.enter();
        ImporterTopLevel scope = new ImporterTopLevel(ctxt);

        Scriptable jsCtxt = Context.toObject(xsctxt, scope);
        scope.put("XSCRIPTCONTEXT", scope, jsCtxt);

        Scriptable jsArgs = Context.toObject(
           new Object[0], scope);
        scope.put("ARGUMENTS", scope, jsArgs);

        Context.exit();
        return scope;
    }

    class closeHandler implements Runnable
    {
        URL url;
        closeHandler( URL url )
        {
            this.url = url;
        }
        public void run()
        {
            synchronized( BEING_EDITED )
            {
                Object o = BEING_EDITED.remove( this.url );
            }
        }
    }
}
