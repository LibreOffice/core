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
