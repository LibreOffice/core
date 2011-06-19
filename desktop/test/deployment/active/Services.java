/*************************************************************************
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
************************************************************************/

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
