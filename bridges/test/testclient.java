/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: testclient.java,v $
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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.IBridge;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.bridge.XInstanceProvider;

import test.XCallMe;
import test.XTestFactory;


class MyInstanceProvider implements XInstanceProvider
{
    public Object getInstance( String sName )
        {
            System.out.println( "getInstance called" );
            return new MyTestFactory();
        }

}


class MyTestFactory implements XTestFactory
{
    public XCallMe createCallMe(  ) throws com.sun.star.uno.RuntimeException
        {
            return new MyCallMe();
        }

    public test.XInterfaceTest createInterfaceTest(  ) throws com.sun.star.uno.RuntimeException
        {
            return null;
        }

}
class MyCallMe implements XCallMe
{
    public String getsAttribute() throws com.sun.star.uno.RuntimeException
        {
            return "";
        }
    public void setsAttribute( String _sattribute ) throws com.sun.star.uno.RuntimeException
        {
        }

    // Methods
    public void call( /*IN*/String s, /*IN*/int nToDo ) throws test.TestBridgeException, com.sun.star.uno.RuntimeException
        {

        }
    public void callOneway( /*IN*/String s, /*IN*/int nToDo ) throws com.sun.star.uno.RuntimeException
        {
            System.out.println( "entering callOneway" );
//              this.wait( 5 );
            try {
                Thread.currentThread().sleep( 4000 );
            }
            catch ( java.lang.Exception e )
            {
                System.out.println( e );
            }
            System.out.println( "leaving callOneway" );
        }
    public void callAgain( /*IN*/XCallMe callAgain, /*IN*/int nToCall ) throws com.sun.star.uno.RuntimeException
        {

        }
    public test.TestTypes transport( /*IN*/test.TestTypes types ) throws com.sun.star.uno.RuntimeException
        {
            return new test.TestTypes();
        }

}

public class testclient
{
    static void main( String[] args )
        {
            try {

                com.sun.star.comp.servicemanager.ServiceManager smgr =
                    new com.sun.star.comp.servicemanager.ServiceManager();
                smgr.addFactories( new String[] { "com.sun.star.comp.connections.Connector" });

                Object  x  = smgr.createInstance("com.sun.star.connection.Connector");
                if( x == null )
                {
                    System.out.println( "couldn't create connector\n" );
                    return;
                }


                XConnector xConnector =
                    UnoRuntime.queryInterface( XConnector.class , x );

                XConnection xConnection = xConnector.connect(args[0]);

                if( null != xConnection )
                {
                    System.out.println( "after connect" );
                    String rootOid = "OfficeDaemon.Factory";
                    com.sun.star.uno.IBridge bridge = (IBridge ) UnoRuntime.getBridgeByName(
                        "java",
                        null,
                        "remote",
                        null,
                        new Object[]{"iiop", xConnection, new MyInstanceProvider()});

                    System.out.println( "after building bridge" );
//                  Object rInitialObject = m_bridge.mapInterfaceFrom(rootOid, XInterface.class);
//                  XTestFactory rFactory =
//                      UnoRuntime.queryInterface(XTestFactory.class,rInitialObject );

//                  XCallMe callMerFactory->
                    Thread.currentThread().sleep( 100000 );
                }
            }
            catch( com.sun.star.uno.Exception e)
            {
                System.out.println( "Exception thrown" );
            }
            catch( java.lang.Exception e)
            {
                System.out.println( "java.lang.Exception thrown" );
            }

            System.out.println( "exiting" );
        }
}
