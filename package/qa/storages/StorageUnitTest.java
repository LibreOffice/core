/*************************************************************************
 *
 *  $RCSfile: StorageUnitTest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-30 09:18:10 $
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
 */
public class StorageUnitTest  extends ComplexTestCase
{
    private XMultiServiceFactory m_xMSF = null;
    private XSingleServiceFactory m_xStorageFactory = null;

    public String[] getTestMethodNames()
    {
        return new String[] {   "ExecuteTest01",
                                "ExecuteTest02",
                                "ExecuteTest03",
                                "ExecuteTest04",
                                "ExecuteTest05",
                                "ExecuteTest06",
                                "ExecuteTest07",
                                "ExecuteTest08",
                                "ExecuteTest09" };
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

}

