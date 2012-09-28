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


import java.io.IOException;

import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.container.XSet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.pgp.SimplePGPMailerFactoryReg;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
//  import com.sun.star.comp.bootstrap.Bootstrap;



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



    public static void main(String argv[]) throws Exception {
        if(argv.length != 1)    {
            System.err.println("usage : testoffice protocol:host:port");
            System.exit(-1);
        }

//          try {
              XUnoUrlResolver resolver = UnoUrlResolver.create(Bootstrap.createInitialComponentContext(null));

              XConnector  xConnector  = UnoRuntime.queryInterface(XConnector.class, resolver.resolve("com.sun.star.connection.Connector"));
              XConnection xConnection = xConnector.connect(argv[0]);

            String rootOid = "classic_uno";
              IBridge iBridge = UnoRuntime.getBridgeByName("java", null, "remote", null, new Object[]{"iiop", xConnection, null});

            Object rInitialObject = iBridge.mapInterfaceFrom(rootOid, new Type(XInterface.class));
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

