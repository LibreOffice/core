/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
