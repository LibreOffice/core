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

package org.libreoffice.example.java_scripts;

import javax.swing.SwingUtilities;
import java.io.InputStream;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.ImporterTopLevel;
import org.mozilla.javascript.tools.debugger.Main;
import org.mozilla.javascript.tools.debugger.ScopeProvider;

import com.sun.star.script.provider.XScriptContext;

public class OORhinoDebugger implements OOScriptDebugger {

    public void go(final XScriptContext xsctxt, String filename) {
        Main sdb = initUI(xsctxt);

        // This is the method we've added to open a file when starting
        // the Rhino debugger
        sdb.openFile(filename);
    }

    public void go(final XScriptContext xsctxt, InputStream in) {
        Main sdb = initUI(xsctxt);

        // Open a stream in the debugger
        sdb.openStream(in);
    }

    // This code is based on the main method of the Rhino Debugger Main class
    // We pass in the XScriptContext in the global scope for script execution
    private Main initUI(final XScriptContext xsctxt) {
        try {
            final Main sdb = new Main("Rhino JavaScript Debugger");
            swingInvoke(new Runnable() {
                public void run() {
                    sdb.pack();
                    sdb.setSize(640, 640);
                    sdb.setVisible(true);
                }
            });
            sdb.setExitAction(new Runnable() {
                public void run() {
                    sdb.dispose();
                }
            });
            Context.addContextListener(sdb);
            sdb.setScopeProvider(new ScopeProvider() {
                public Scriptable getScope() {
                    Context ctxt = Context.enter();
                    ImporterTopLevel scope = new ImporterTopLevel(ctxt);
                    Scriptable jsArgs = Context.toObject(xsctxt, scope);
                    scope.put("XSCRIPTCONTEXT", scope, jsArgs);
                    Context.exit();
                    return scope;
                }
            });
            return sdb;
        } catch (Exception exc) {
            exc.printStackTrace();
        }

        return null;
    }

    static void swingInvoke(Runnable f) {
        if (SwingUtilities.isEventDispatchThread()) {
            f.run();
            return;
        }

        try {
            SwingUtilities.invokeAndWait(f);
        } catch (Exception exc) {
            exc.printStackTrace();
        }
    }
}
