/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testclient.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:51:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
                    ( XConnector ) UnoRuntime.queryInterface( XConnector.class , x );

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
//                  XTestFactory rFactory = (XTestFactory )
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
