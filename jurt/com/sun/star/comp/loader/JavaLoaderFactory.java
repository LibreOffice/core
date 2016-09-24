/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.comp.loader;

import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;


public class JavaLoaderFactory implements XSingleServiceFactory, XServiceInfo {

    private static String[] supportedServices = {
        "com.sun.star.loader.Java",
        "com.sun.star.loader.Java2"
    };

    protected XMultiServiceFactory multiServiceFactory = null;

    public JavaLoaderFactory(XMultiServiceFactory factory) {
        multiServiceFactory = factory;
    }

    public java.lang.Object createInstance()
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        return new JavaLoader(multiServiceFactory);
    }

    public java.lang.Object createInstanceWithArguments( java.lang.Object[] args )
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        JavaLoader loader = new JavaLoader();
        loader.initialize(args);

        return loader;
    }

    /**
     * Implements the XServiceInfo interface.
     */
    public String getImplementationName()
            throws com.sun.star.uno.RuntimeException
    {
        return JavaLoader.class.getName();
    }

    /**
     * Implements the XServiceInfo interface.
     */
    public boolean supportsService(String serviceName)
            throws com.sun.star.uno.RuntimeException
    {
        for (String supportedService : supportedServices) {
            if (supportedService.equals(serviceName)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Implements the XServiceInfo interface.
     */
    public String[] getSupportedServiceNames()
            throws com.sun.star.uno.RuntimeException
    {
        return supportedServices;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
