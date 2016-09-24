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

import java.awt.Color;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;

public class TestApplet extends java.applet.Applet {

    /** Initialization method that will be called after the applet is loaded
     *  into the browser.
     */
    @Override
    public void init () {
        setBackground( Color.green);
        resize( 300, 300);

        // Security tests.
        File f= new File("d:\\temp\\javasecurity.txt");
        try {
            f.createNewFile();

        // local connection
        URL url= new URL("http://localhost:8080/index.html");
        url.openStream();
        // remote connection
        url= new URL("http://www.w3.org/index.html");
        url.openStream();
        }catch( MalformedURLException mue) {
        }catch( IOException e) {
            String s= e.getMessage();
            System.out.println(s);
        }

    }
}
