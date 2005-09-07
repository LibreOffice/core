/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestComponent.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:25:04 $
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

package com.sun.star.comp.jvmaccess.workbench;

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XMain;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.Type;

/* Deploy this component with pkgchk, and call it with the Basic program

   Sub Main
     dim args$()
     o = createunoservice("com.sun.star.comp.jvmaccess.workbench.TestComponent")
     o.run args$()
   End Sub

   The name of the context class loader should appear on the console.
 */

public final class TestComponent implements XTypeProvider, XServiceInfo, XMain {
    public Type[] getTypes() {
        return new Type[] { new Type(XTypeProvider.class),
                            new Type(XServiceInfo.class),
                            new Type(XMain.class) };
    }

    public byte[] getImplementationId() {
        byte[] id = new byte[16];
        int n = hashCode();
        id[0] = (byte) (n & 0xFF);
        id[1] = (byte) ((n >> 8) & 0xFF);
        id[2] = (byte) ((n >> 16) & 0xFF);
        id[3] = (byte) ((n >> 24) & 0xFF);
        return id;
    }

    public String getImplementationName() {
        return getClass().getName();
    }

    public boolean supportsService(String serviceName) {
        return serviceName.equals(serviceName);
    }

    public String[] getSupportedServiceNames() {
        return new String[] { serviceName };
    }

    public int run(String[] arguments) {
        System.out.println("context class loader: "
                           + Thread.currentThread().getContextClassLoader());
        return 0;
    }

    public static XSingleServiceFactory __getServiceFactory(
        String implName, XMultiServiceFactory multiFactory, XRegistryKey regKey)
    {
        if (implName.equals(TestComponent.class.getName())) {
            return FactoryHelper.getServiceFactory(TestComponent.class,
                                                   serviceName, multiFactory,
                                                   regKey);
        } else {
            return null;
        }
    }

    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(
            TestComponent.class.getName(), serviceName, regKey);
    }

    private static final String serviceName
    = "com.sun.star.comp.jvmaccess.workbench.TestComponent";
}
