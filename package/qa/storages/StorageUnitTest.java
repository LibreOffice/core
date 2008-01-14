/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StorageUnitTest.java,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 17:33:00 $
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
package complex.storages;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;

import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.lang.*;

import complexlib.ComplexTestCase;

import complex.storages.*;

import util.utils;
import java.util.*;
import java.io.*;

/* This unit test for storage objects is designed to
 * test most important statements from storage service
 * specification.
 *
 * Regression tests are added to extend the tested
 * functionalities.
 */
public class StorageUnitTest  extends ComplexTestCase
{
    private XMultiServiceFactory m_xMSF = null;
    private XSingleServiceFactory m_xStorageFactory = null;

    public String[] getTestMethodNames()
    {
        return new String[] {
                                "ExecuteTest01",
                                "ExecuteTest02",
                                "ExecuteTest03",
                                "ExecuteTest04",
                                "ExecuteTest05",
                                "ExecuteTest06",
                                "ExecuteTest07",
                                "ExecuteTest08",
                                "ExecuteTest09",
                                "ExecuteTest10",
                                "ExecuteTest11",
                                "ExecuteTest12",
                                "ExecuteTest13",
                                "ExecuteTest14",
                                "ExecuteTest15",
                                "ExecuteRegressionTest_114358",
                                "ExecuteRegressionTest_i29169",
                                "ExecuteRegressionTest_i30400",
                                "ExecuteRegressionTest_i29321",
                                "ExecuteRegressionTest_i30677",
                                "ExecuteRegressionTest_i27773",
                                "ExecuteRegressionTest_i46848",
                                "ExecuteRegressionTest_i55821",
                                "ExecuteRegressionTest_i35095",
                                "ExecuteRegressionTest_i49755",
                                "ExecuteRegressionTest_i59886",
                                "ExecuteRegressionTest_i61909",
                                "ExecuteRegressionTest_i84234",
                                "ExecuteRegressionTest_125919"};
    }

    public String getTestObjectName()
    {
        return "StorageUnitTest";
    }

    public void before()
    {
        m_xMSF = (XMultiServiceFactory)param.getMSF();
        if ( m_xMSF == null )
        {
            failed( "Can't create service factory!" );
            return;
        }

        try {
            Object oStorageFactory = m_xMSF.createInstance( "com.sun.star.embed.StorageFactory" );
            m_xStorageFactory = (XSingleServiceFactory)UnoRuntime.queryInterface( XSingleServiceFactory.class,
                                                                                oStorageFactory );
        }
        catch( Exception e )
        {
            failed( "Can't create storage factory!" );
            return;
        }

        if ( m_xStorageFactory == null )
        {
            failed( "Can't create service factory!" );
            return;
        }
    }

    public void ExecuteTest01()
    {
        StorageTest aTest = new Test01( m_xMSF, m_xStorageFactory, log );
        assure( "Test01 failed!", aTest.test() );
    }

    public void ExecuteTest02()
    {
        StorageTest aTest = new Test02( m_xMSF, m_xStorageFactory, log );
        assure( "Test02 failed!", aTest.test() );
    }

    public void ExecuteTest03()
    {
        StorageTest aTest = new Test03( m_xMSF, m_xStorageFactory, log );
        assure( "Test03 failed!", aTest.test() );
    }

    public void ExecuteTest04()
    {
        StorageTest aTest = new Test04( m_xMSF, m_xStorageFactory, log );
        assure( "Test04 failed!", aTest.test() );
    }

    public void ExecuteTest05()
    {
        StorageTest aTest = new Test05( m_xMSF, m_xStorageFactory, log );
        assure( "Test05 failed!", aTest.test() );
    }

    public void ExecuteTest06()
    {
        StorageTest aTest = new Test06( m_xMSF, m_xStorageFactory, log );
        assure( "Test06 failed!", aTest.test() );
    }

    public void ExecuteTest07()
    {
        StorageTest aTest = new Test07( m_xMSF, m_xStorageFactory, log );
        assure( "Test07 failed!", aTest.test() );
    }

    public void ExecuteTest08()
    {
        StorageTest aTest = new Test08( m_xMSF, m_xStorageFactory, log );
        assure( "Test08 failed!", aTest.test() );
    }

    public void ExecuteTest09()
    {
        StorageTest aTest = new Test09( m_xMSF, m_xStorageFactory, log );
        assure( "Test09 failed!", aTest.test() );
    }

    public void ExecuteTest10()
    {
        StorageTest aTest = new Test10( m_xMSF, m_xStorageFactory, log );
        assure( "Test10 failed!", aTest.test() );
    }

    public void ExecuteTest11()
    {
        StorageTest aTest = new Test11( m_xMSF, m_xStorageFactory, log );
        assure( "Test11 failed!", aTest.test() );
    }

    public void ExecuteTest12()
    {
        StorageTest aTest = new Test12( m_xMSF, m_xStorageFactory, log );
        assure( "Test12 failed!", aTest.test() );
    }

    public void ExecuteTest13()
    {
        StorageTest aTest = new Test13( m_xMSF, m_xStorageFactory, log );
        assure( "Test13 failed!", aTest.test() );
    }

    public void ExecuteTest14()
    {
        StorageTest aTest = new Test14( m_xMSF, m_xStorageFactory, log );
        assure( "Test14 failed!", aTest.test() );
    }

    public void ExecuteTest15()
    {
        StorageTest aTest = new Test15( m_xMSF, m_xStorageFactory, log );
        assure( "Test15 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_114358()
    {
        StorageTest aTest = new RegressionTest_114358( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_114358 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i29169()
    {
        StorageTest aTest = new RegressionTest_i29169( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i29169 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i30400()
    {
        StorageTest aTest = new RegressionTest_i30400( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i30400 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i29321()
    {
        StorageTest aTest = new RegressionTest_i29321( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i29321 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i30677()
    {
        StorageTest aTest = new RegressionTest_i30677( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i30677 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i27773()
    {
        StorageTest aTest = new RegressionTest_i27773( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i27773 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i46848()
    {
        StorageTest aTest = new RegressionTest_i46848( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i46848 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i55821()
    {
        StorageTest aTest = new RegressionTest_i55821( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i55821 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i35095()
    {
        StorageTest aTest = new RegressionTest_i35095( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i35095 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i49755()
    {
        StorageTest aTest = new RegressionTest_i49755( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i49755 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i59886()
    {
        StorageTest aTest = new RegressionTest_i59886( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i59886 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i61909()
    {
        StorageTest aTest = new RegressionTest_i61909( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i61909 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_i84234()
    {
        StorageTest aTest = new RegressionTest_i84234( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_i84234 failed!", aTest.test() );
    }

    public void ExecuteRegressionTest_125919()
    {
        StorageTest aTest = new RegressionTest_125919( m_xMSF, m_xStorageFactory, log );
        assure( "RegressionTest_125919 failed!", aTest.test() );
    }
}

