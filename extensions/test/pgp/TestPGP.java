/*************************************************************************
 *
 *  $RCSfile: TestPGP.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:56 $
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

