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

package com.sun.star.comp.test.deployment.active_java;

import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.registry.InvalidRegistryException;
import com.sun.star.registry.XRegistryKey;

public final class Services {
    private Services() {}

    public static XSingleComponentFactory __getComponentFactory(
        String implementation)
    {
        if (implementation.equals(Dispatch.implementationName)) {
            return Factory.createComponentFactory(
                Dispatch.class, Dispatch.implementationName,
                Dispatch.serviceNames);
        } else if (implementation.equals(Provider.implementationName)) {
            return Factory.createComponentFactory(
                Provider.class, Provider.implementationName,
                Provider.serviceNames);
        } else {
            return null;
        }
    }

    public static boolean __writeRegistryServiceInfo(XRegistryKey key) {
        if (!(Factory.writeRegistryServiceInfo(
                  Dispatch.implementationName, Dispatch.serviceNames, key) &&
              Factory.writeRegistryServiceInfo(
                  Provider.implementationName, Provider.serviceNames, key)))
        {
            return false;
        }
        try {
            key.
                createKey(
                    "/" + Dispatch.implementationName +
                    "/UNO/SINGLETONS/" +
                    "com.sun.star.test.deployment.active_java_singleton").
                setStringValue(Dispatch.implementationName);
        } catch (InvalidRegistryException e) {
            return false;
        }
        return true;
    }
}
