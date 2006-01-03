/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JavaMain.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-01-03 12:39:56 $
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

package test.java.javamain;

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XMain;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.XComponentContext;
import test.java.tester.Tester;

public final class JavaMain implements XMain {
    public JavaMain(XComponentContext context) {
        this.context = context;
    }

    public int run(String[] arguments) {
        Tester.test(context);
        return 0;
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
                JavaMain.class, SERVICE_NAME, factory, key);
        } else {
            return null;
        }
    }

    private static final String IMPLEMENTATION_NAME
    = "test.java.javamain.Component";
    private static final String SERVICE_NAME = "test.dummy.JavaMain";

    private final XComponentContext context;
}
