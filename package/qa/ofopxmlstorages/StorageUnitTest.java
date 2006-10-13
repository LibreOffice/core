/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StorageUnitTest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:43:15 $
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
package complex.ofopxmlstorages;

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

import complex.ofopxmlstorages.*;

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
                                "ExecuteTest08"
                            };
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
}

