/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


import java.io.IOException;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyState;

import com.sun.star.bridge.XBridge;

//  import com.sun.star.comp.bootstrap.Bootstrap;

import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

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
import com.sun.star.container.*;
import com.sun.star.lang.*;

import com.sun.star.pgp.*;



public class TestPGP {


    static void doSomething(Object r) throws com.sun.star.uno.Exception, IOException, Exception {
        XNamingService rName = (XNamingService)UnoRuntime.queryInterface(XNamingService.class, r);

        if(rName != null) {
            System.err.println("got the remote naming service !");
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager");

            XMultiServiceFactory rSmgr = (XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, rXsmgr);
            if(rSmgr != null) {
                System.err.println("got the remote service manager !");
            }

            XSet set= (XSet)UnoRuntime.queryInterface(XSet.class, rSmgr);
            if( set == null) {
                System.err.println(" couldn't get XSet from ServiceFactory");
                return;
            }
//              try{
            SimplePGPMailerFactoryReg mailerReg= new SimplePGPMailerFactoryReg();
            XSingleServiceFactory factory= mailerReg.getServiceFactory(
                "com.sun.star.pgp.SimplePGPMailerImpl", rSmgr, null, null);

            if( factory == null) {
                System.err.println("couldn't create PGP factory !");
                return;
            }
            set.insert(factory );
            System.err.println("PGP factory inserted into service manager");

//              }catch(Exception e ){
//                  System.err.println( "exception caught in TestPGP");
//              }

        }
    }



    static String neededServices[] = new String[] {
        "com.sun.star.comp.servicemanager.ServiceManager",
        "com.sun.star.comp.loader.JavaLoader",
        "com.sun.star.comp.connections.Connector",
          "com.sun.star.comp.connections.Acceptor"
    };

    public static void main(String argv[]) throws Exception {
        if(argv.length != 1)    {
            System.err.println("usage : testoffice protocol:host:port");
            System.exit(-1);
        }

//          try {
            com.sun.star.comp.servicemanager.ServiceManager smgr = new com.sun.star.comp.servicemanager.ServiceManager();
            smgr.addFactories(neededServices, null);

              XConnector  xConnector  = (XConnector)smgr.createInstance("com.sun.star.connection.Connector");
              XConnection xConnection = xConnector.connect(argv[0]);

            String rootOid = "classic_uno";
              IBridge iBridge = UnoRuntime.getBridgeByName("java", null, "remote", null, new Object[]{"iiop", xConnection, null});

            Object rInitialObject = iBridge.mapInterfaceFrom(rootOid, XInterface.class);
//              Object rInitialObject = xBridge.getInstance("NamingService");

            if(rInitialObject != null) {
                System.err.println("got the remote object");
                doSomething(rInitialObject);
            }
//          }
//          catch (Exception exception) {
//              System.err.println("Exception thrown:" + exception);
//          }
    }
}

