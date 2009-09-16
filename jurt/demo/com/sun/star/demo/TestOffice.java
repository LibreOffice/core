/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestOffice.java,v $
 * $Revision: 1.3 $
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

