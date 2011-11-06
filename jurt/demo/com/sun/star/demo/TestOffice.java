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



package com.sun.star.demo;


import java.io.IOException;


import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyState;

import com.sun.star.bridge.XUnoUrlResolver;

import com.sun.star.io.BufferSizeExceededException;
import com.sun.star.io.NotConnectedException;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;

import com.sun.star.frame.XComponentLoader;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;

import com.sun.star.text.XSimpleText;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;

import com.sun.star.uno.IBridge;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;


public class TestOffice {
    static void testPipe(XMultiServiceFactory rSmgr) throws com.sun.star.uno.Exception {
        XOutputStream rOut = (XOutputStream) rSmgr.createInstance("com.sun.star.io.Pipe");

        {
            byte bytes[] = new byte[10];
            bytes[0] = 42;
            rOut.writeBytes(bytes);
        }


        {
            byte bytes[][] = new byte[1][];

            XInputStream rIn = UnoRuntime.queryInterface(XInputStream.class, rOut);
            if(rIn.available() != 10)
                System.err.println("wrong bytes available\n");

            if(rIn.readBytes(bytes, 10) != 10)
                System.err.println("wrong bytes read\n");

            if(42 != bytes[0][0])
                System.err.println("wrong element in sequence\n");
        }
    }


    static void testWriter(XComponent rCmp) throws IOException {
        XTextDocument rTextDoc = UnoRuntime.queryInterface(XTextDocument.class, rCmp);

        XText rText = UnoRuntime.queryInterface(XText.class, rTextDoc.getText());
        XTextCursor rCursor = UnoRuntime.queryInterface(XTextCursor.class, rText.createTextCursor());
        XTextRange rRange = UnoRuntime.queryInterface(XTextRange.class, rCursor);

        byte pcText[] = new byte[1024];
        pcText[0] = 0;
        System.err.println("pleast type any text\n");
        while(true) {
            System.in.read(pcText);

            String string = new String(pcText);
            if(string.equals("end")) break;

            string += " ";

            rText.insertString(rRange , string, false);
        }
    }

    static void testDocument(XMultiServiceFactory rSmgr) throws com.sun.star.uno.Exception, IOException {
        XComponentLoader rLoader = UnoRuntime.queryInterface(XComponentLoader.class, rSmgr.createInstance("com.sun.star.frame.Desktop"));

        String urls[] = new String[] {
            "private:factory/swriter",
            "private:factory/scalc",
            "private:factory/sdraw",
            "http://www.heise.de",
        };

        String docu[] = new String[] {
            "a new writer document ...\n",
            "a new calc document ...\n",
            "a new draw document ...\n",
            "www.heise.de\n",
        };

        for(int i = 0; i < urls.length; ++ i)   {
            System.err.println("press any key to open "  + docu[i]);

            System.in.read();
            while(System.in.available() > 0)
                System.in.read();

            XComponent rComponent = rLoader.loadComponentFromURL(urls[i], "_blank", 0, new PropertyValue[0]);

//              testWriter(rComponent);
            System.err.println("press any key to close the document");
            System.in.read();
            while(System.in.available() > 0)
                System.in.read();

            rComponent.dispose();
        }
    }

    static void doSomething(Object r) throws com.sun.star.uno.Exception, IOException {
        XNamingService rName = UnoRuntime.queryInterface(XNamingService.class, r);

        if(rName != null) {
            System.err.println("got the remote naming service !");
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager");

            XMultiServiceFactory rSmgr = UnoRuntime.queryInterface(XMultiServiceFactory.class, rXsmgr);
            if(rSmgr != null) {
                System.err.println("got the remote service manager !");
//                  testPipe(rSmgr);
                testDocument(rSmgr);
            }
        }
    }



    static String neededServices[] = new String[] {
        "com.sun.star.comp.servicemanager.ServiceManager",
        "com.sun.star.comp.loader.JavaLoader",
        "com.sun.star.comp.connections.Connector",
        "com.sun.star.comp.bridgefactory.BridgeFactory",
        "com.sun.star.comp.urlresolver.UrlResolver"
    };

    public static void main(String argv[]) throws Exception {
        if(argv.length != 1)    {
            System.err.println("usage : testoffice uno:connection;protocol;objectName");
            System.exit(-1);
        }

        com.sun.star.comp.servicemanager.ServiceManager smgr = new com.sun.star.comp.servicemanager.ServiceManager();
        smgr.addFactories(neededServices);

        Object  resolver  = smgr.createInstance("com.sun.star.bridge.UnoUrlResolver" );
        XUnoUrlResolver resolver_xUnoUrlResolver = UnoRuntime.queryInterface(XUnoUrlResolver.class, resolver);

          Object rInitialObject = resolver_xUnoUrlResolver.resolve(argv[0]);

        if(rInitialObject != null) {
            System.err.println("got the remote object");
            doSomething(rInitialObject);
        }
    }
}

