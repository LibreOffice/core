/*************************************************************************
 *
 *  $RCSfile: ScriptEditorForJavaScript.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-10-29 15:01:18 $
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

import javax.swing.SwingUtilities;
import java.io.InputStream;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.ImporterTopLevel;
import org.mozilla.javascript.tools.debugger.Main;
import org.mozilla.javascript.tools.debugger.ScopeProvider;

import drafts.com.sun.star.script.provider.XScriptContext;

public class ScriptEditorForJavaScript {

    public void edit(final XScriptContext xsctxt, String filename) {
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
