/*************************************************************************
 *
 *  $RCSfile: TestOffice.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:27:54 $
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

            XInputStream rIn = (XInputStream)UnoRuntime.queryInterface(XInputStream.class, rOut);
            if(rIn.available() != 10)
                System.err.println("wrong bytes available\n");

            if(rIn.readBytes(bytes, 10) != 10)
                System.err.println("wrong bytes read\n");

            if(42 != bytes[0][0])
                System.err.println("wrong element in sequence\n");
        }
    }


    static void testWriter(XComponent rCmp) throws IOException {
        XTextDocument rTextDoc = (XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, rCmp);

        XText rText = (XText)UnoRuntime.queryInterface(XText.class, rTextDoc.getText());
        XTextCursor rCursor = (XTextCursor)UnoRuntime.queryInterface(XTextCursor.class, rText.createTextCursor());
        XTextRange rRange = (XTextRange)UnoRuntime.queryInterface(XTextRange.class, rCursor);

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
        XComponentLoader rLoader = (XComponentLoader)UnoRuntime.queryInterface(XComponentLoader.class, rSmgr.createInstance("com.sun.star.frame.Desktop"));

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
        XNamingService rName = (XNamingService)UnoRuntime.queryInterface(XNamingService.class, r);

        if(rName != null) {
            System.err.println("got the remote naming service !");
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager");

            XMultiServiceFactory rSmgr = (XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, rXsmgr);
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
        XUnoUrlResolver resolver_xUnoUrlResolver = (XUnoUrlResolver)UnoRuntime.queryInterface(XUnoUrlResolver.class, resolver);

          Object rInitialObject = resolver_xUnoUrlResolver.resolve(argv[0]);

        if(rInitialObject != null) {
            System.err.println("got the remote object");
            doSomething(rInitialObject);
        }
    }
}

