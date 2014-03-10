/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        return new byte[0];
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
