/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScriptSourceModel.java,v $
 * $Revision: 1.9 $
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
package com.sun.star.script.framework.provider.beanshell;

import java.io.File;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import java.net.URL;

import java.lang.reflect.InvocationTargetException;
import com.sun.star.script.provider.XScriptContext;

public class ScriptSourceModel {

    private int currentPosition = -1;
    private URL file = null;
    private ScriptSourceView view = null;

    public ScriptSourceModel(URL file ) {
        this.file = file;
    }

    private String load() throws IOException {
        StringBuffer buf = new StringBuffer();
        InputStream in = file.openStream();

        byte[] contents = new byte[1024];
        int len = 0;

        while ((len = in.read(contents, 0, 1024)) != -1) {
            buf.append(new String(contents, 0, len));
        }

        try {
            in.close();
        }
        catch (IOException ignore) {
        }

        return buf.toString();
    }

    public String getText() {
        String result = "";

        try {
            result = load();
        }
        catch (IOException ioe) {
            // do nothing, empty string will be returned
        }

        return result;
    }

    public int getCurrentPosition() {
        return this.currentPosition;
    }

    public void setView(ScriptSourceView view) {
        this.view = view;
    }

    public Object execute(final XScriptContext context, ClassLoader cl )
        throws Exception
    {
        Object result = null;
        // Thread execThread = new Thread() {
            // public void run() {
                if ( cl != null )
                {
                    // sets this threads class loader
                    // hopefully any threads spawned by this
                    // will inherit this cl
                    // this enables any class files imported
                    // from the interpreter to be loaded
                    // note: setting the classloader on the
                    // interpreter has a slightly different
                    // meaning in that the classloader for
                    // the interpreter seems only to look for
                    // source files ( bla.java ) in the classpath
                    Thread.currentThread().setContextClassLoader(cl);
                }
                bsh.Interpreter interpreter = new bsh.Interpreter();
                if ( cl != null )
                {
                    // additionally set class loader on the interpreter
                    // to allow it to load java classes defined in source
                    // files e.g. bla.java
                    interpreter.getNameSpace().clear();
                }


                // reset position
                currentPosition = -1;
                view.update();

                interpreter.set("XSCRIPTCONTEXT", context);
                interpreter.set("ARGUMENTS", new Object[0]);

                if (view.isModified()) {
                    result = interpreter.eval(view.getText());
                }
                else {
                    result = interpreter.eval(getText());
                }
            // }
        // };
        // execThread.start();
        return result;
    }
    public void indicateErrorLine( int lineNum )
    {
        System.out.println("Beanshell indicateErrorLine " + lineNum );
        currentPosition = lineNum - 1;
        view.update();
    }
}
