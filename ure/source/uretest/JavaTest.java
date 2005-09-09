/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JavaTest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:12:53 $
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

package test.java.javatest;

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import test.types.TestException;
import test.types.XTest;

public final class JavaTest implements XTest {
    public JavaTest() {}

    public void throwException() throws TestException {
        throw new TestException("test", this);
    }

    public static boolean __writeRegistryServiceInfo(XRegistryKey key) {
        return
            FactoryHelper.writeRegistryServiceInfo(
                IMPLEMENTATION_NAME, SERVICE_NAME, key);
    }

    public static XSingleServiceFactory __getServiceFactory(
        String name, XMultiServiceFactory factory, XRegistryKey key)
    {
        if (name.equals(IMPLEMENTATION_NAME)) {
            return FactoryHelper.getServiceFactory(
                JavaTest.class, SERVICE_NAME, factory, key);
        } else {
            return null;
        }
    }

    private static final String IMPLEMENTATION_NAME
    = "test.java.javatest.Component";
    private static final String SERVICE_NAME = "test.types.JavaTest";
}
