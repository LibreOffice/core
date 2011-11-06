/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

import java.awt.Graphics;
import java.awt.Color;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.io.InputStream;
import java.net.MalformedURLException;
/*
 * TestApplet.java
 *
 * Created on 21. November 2001, 09:37
 */

/**
 *
 * @author  jl97489
 * @version
 */
public class TestApplet extends java.applet.Applet {

    /** Initialization method that will be called after the applet is loaded
     *  into the browser.
     */
    public void init () {
        setBackground( Color.green);
        resize( 300, 300);

        // Security tests.
        File f= new File("d:\\temp\\javasecurity.txt");
        SecurityManager mgr= System.getSecurityManager();
        try {
            f.createNewFile();

        // local connection
        URL url= new URL("http://localhost:8080/index.html");
        InputStream is= url.openStream();
        // remote connection
        url= new URL("http://www.w3.org/index.html");
        is= url.openStream();
        }catch( MalformedURLException mue) {
        }catch( IOException e) {
            String s= e.getMessage();
            System.out.println(s);
        }catch( SandboxSecurityException sse) {
            String s= sse.getMessage();
            System.out.println("s");
        }
        //        catch( Exception ex) {
//            String s= ex.getMessage();
//            ex.printStackTrace();
//        }

    }

    public void paint( Graphics g) {
        super.paint( g);
    }
}
