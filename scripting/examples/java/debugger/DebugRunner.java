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

import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;
import java.net.URLDecoder;

import com.sun.star.uno.XComponentContext;
import com.sun.star.script.framework.provider.PathUtils;
import com.sun.star.script.provider.XScriptContext;

public class DebugRunner {

    private static final String FILE_URL_PREFIX =
        System.getProperty("os.name").startsWith("Windows") == true ?
        "file:///" : "file://";

    public void go(final XScriptContext xsctxt, String language, String uri,
                   String filename) {

        OOScriptDebugger debugger;
        String path = "";

        if (language.equals("JavaScript")) {
            debugger = new OORhinoDebugger();
        } else if (language.equals("BeanShell")) {
            debugger = new OOBeanShellDebugger();
        } else {
            return;
        }

        if (uri.startsWith(FILE_URL_PREFIX)) {
            uri = URLDecoder.decode(uri);
            String s = uri.substring(FILE_URL_PREFIX.length());
            File f = new File(s);

            if (f.exists()) {
                if (f.isDirectory()) {
                    if (!filename.equals("")) {
                        path = new File(f, filename).getAbsolutePath();
                    }
                } else {
                    path = f.getAbsolutePath();
                }
            }

            debugger.go(xsctxt, path);
        } else {
            if (!uri.endsWith("/")) {
                uri += "/";
            }

            String script = uri + filename;
            InputStream is;

            try {
                is = PathUtils.getScriptFileStream(
                         script, xsctxt.getComponentContext());

                if (is != null) {
                    debugger.go(xsctxt, is);
                }
            } catch (IOException ioe) {
                System.out.println("Error loading script: " + script);
            }
        }
    }
}
