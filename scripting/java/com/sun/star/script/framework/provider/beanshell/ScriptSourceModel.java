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

import com.sun.star.script.provider.XScriptContext;

import java.io.IOException;
import java.io.InputStream;

import java.net.URL;

public class ScriptSourceModel {

    private int currentPosition = -1;
    private final URL file;
    private ScriptSourceView view = null;

    public ScriptSourceModel(URL file) {
        this.file = file;
    }

    private String load() throws IOException {
        StringBuilder buf = new StringBuilder();
        InputStream in = file.openStream();

        byte[] contents = new byte[1024];
        int len;

        while ((len = in.read(contents, 0, 1024)) != -1) {
            buf.append(new String(contents, 0, len));
        }

        try {
            in.close();
        } catch (IOException ignore) {
        }

        return buf.toString();
    }

    public String getText() {
        String result = "";

        try {
            result = load();
        } catch (IOException ioe) {
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

    public Object execute(final XScriptContext context, ClassLoader cl)
    throws Exception {
        if (cl != null) {
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

        if (cl != null) {
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

        Object result;

        if (view.isModified()) {
            String s = view.getText();
            result = interpreter.eval(s != null ? s : "");
        } else {
            result = interpreter.eval(getText());
        }

        return result;
    }

    public void indicateErrorLine(int lineNum) {
        System.out.println("Beanshell indicateErrorLine " + lineNum);
        currentPosition = lineNum - 1;
        view.update();
    }
}
