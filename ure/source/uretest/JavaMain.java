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
