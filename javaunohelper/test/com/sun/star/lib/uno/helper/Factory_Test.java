/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Factory_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:45:36 $
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
package com.sun.star.lib.uno.helper;

import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.Type;
import com.sun.star.uno.AnyConverter;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.beans.XPropertySet;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.registry.XSimpleRegistry;
import com.sun.star.registry.XImplementationRegistration;
import com.sun.star.container.XSet;

import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.comp.helper.RegistryServiceFactory;
import com.sun.star.uno.UnoRuntime;


//==================================================================================================
public class Factory_Test
    extends WeakBase
    implements XServiceInfo
{
    static final String m_impl_name = Factory_Test.class.getName();
    static final String m_supported_services [] = {
        "Factory_Test.Service0", "Factory_Test.Service1" };

    //______________________________________________________________________________________________
    public Factory_Test()
    {
    }
    //______________________________________________________________________________________________
    public Factory_Test( XComponentContext xContext )
        throws com.sun.star.uno.Exception
    {
        if (null == xContext.getValueByName( "/singletons/com.sun.star.lang.theServiceManager" ))
        {
            throw new com.sun.star.uno.RuntimeException(
                "bad component context given!", this );
        }
    }
    //______________________________________________________________________________________________
    public static Object __create( XComponentContext xContext )
        throws com.sun.star.uno.Exception
    {
        return new Factory_Test( xContext );
    }

    // XServiceInfo impl
    //______________________________________________________________________________________________
    public final String getImplementationName()
    {
        return m_impl_name;
    }
    //______________________________________________________________________________________________
    public final boolean supportsService( String service_name )
    {
        for ( int nPos = 0; nPos < m_supported_services.length; ++nPos )
        {
            if (m_supported_services[ nPos ].equals( service_name ))
                return true;
        }
        return false;
    }
    //______________________________________________________________________________________________
    public final String [] getSupportedServiceNames()
    {
        return m_supported_services;
    }

    //==============================================================================================
    public static XSingleComponentFactory __getComponentFactory( String implName )
    {
          if (implName.equals( m_impl_name ))
        {
            return Factory.createComponentFactory(
                Factory_Test.class, Factory_Test.m_supported_services );
        }
        return null;
    }
    //==============================================================================================
    public static boolean __writeRegistryServiceInfo( XRegistryKey xKey )
    {
        return Factory.writeRegistryServiceInfo(
            m_impl_name, Factory_Test.m_supported_services, xKey );
    }

    //==============================================================================================
    static void service_info_test( Object inst )
    {
        XServiceInfo xInfo = (XServiceInfo)UnoRuntime.queryInterface( XServiceInfo.class, inst );

        if (! xInfo.getImplementationName().equals( m_impl_name ))
        {
            System.err.println( "Factory_Test: err -- 1" );
            System.exit( 1 );
        }
        String supported_services [] = xInfo.getSupportedServiceNames();
        if (supported_services.length != m_supported_services.length)
        {
            System.err.println( "Factory_Test: err -- 2" );
            System.exit( 1 );
        }
        for ( int nPos = 0; nPos < supported_services.length; ++nPos )
        {
            if (! supported_services[ nPos ].equals( m_supported_services[ nPos ] ))
            {
                System.err.println( "Factory_Test: err -- 3" );
                System.exit( 1 );
            }
            if (! xInfo.supportsService( m_supported_services[ nPos ] ))
            {
                System.err.println( "Factory_Test: err -- 4" );
                System.exit( 1 );
            }
        }
    }
    //==============================================================================================
    public static void main( String args [] )
    {
        try
        {
            String jar = "file://" + new java.io.File( args[ 0 ] ).toURL().getPath();
            String rdb = "file://" + new java.io.File( args[ 1 ] ).toURL().getPath();
            System.out.println( "jar file = " + jar );
            System.out.println( "rdb file = " + rdb );

            // bootstrap service manager
            XMultiServiceFactory xMgr = RegistryServiceFactory.create( rdb );
            XPropertySet xProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, xMgr );
            XComponentContext xContext = (XComponentContext)AnyConverter.toObject(
                new Type( XComponentContext.class ), xProps.getPropertyValue( "DefaultContext" ) );
            // insert java loader
            XSet xSet = (XSet)AnyConverter.toObject(
                new Type( XSet.class ), xContext.getServiceManager() );
            xSet.insert( new com.sun.star.comp.loader.JavaLoaderFactory( xMgr ) );
            // get rdb of smgr
            XSimpleRegistry xRDB = (XSimpleRegistry)AnyConverter.toObject(
                new Type( XSimpleRegistry.class ), xProps.getPropertyValue( "Registry" ) );
            // register impl
            XImplementationRegistration xImpReg =
                (XImplementationRegistration)UnoRuntime.queryInterface(
                    XImplementationRegistration.class,
                    xContext.getServiceManager().createInstanceWithContext(
                        "com.sun.star.registry.ImplementationRegistration", xContext ) );
            xImpReg.registerImplementation( "com.sun.star.loader.Java2", jar, xRDB );

            // tests
            System.out.println( "testing instance" );
            service_info_test( new Factory_Test() );
            System.out.println( "testing instance" );
            service_info_test( new Factory_Test( xContext ) );
            System.out.println( "testing instance" );
            service_info_test( Factory_Test.__create( xContext ) );
            System.out.println( "testing factory __getComponentFactory()" );
            service_info_test( __getComponentFactory( m_impl_name ) );
            for ( int nPos = 0; nPos < m_supported_services.length; ++nPos )
            {
                System.out.println( "testing factory " + m_supported_services[ nPos ] );
                service_info_test(
                    // create Service
                    xContext.getServiceManager().createInstanceWithContext(
                        m_supported_services[ nPos ], xContext ) );
            }

            XComponent xComp = (XComponent)UnoRuntime.queryInterface( XComponent.class, xContext );
            xComp.dispose();
        }
        catch (Exception exc)
        {
            System.err.println( ">>>>>>>>>> exc occured: " + exc.toString() );
            exc.printStackTrace();
        }
        System.exit( 0 );
    }
}

