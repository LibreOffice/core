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

package com.sun.star.comp.connections;

import com.sun.star.connection.ConnectionSetupException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

/**
 * Helper class for <code>Acceptor</code> and <code>Connector</code>.
 */
final class Implementation {
    /**
     * Instantiate a service for a given connection type.
     *
     * @param factory the service factory used to instantiate the requested
     *     service.
     * @param description has the following format:
     *     <code><var>type</var></code><!--
     *         -->*(<code><var>key</var>=<var>value</var></code>).
     *     The specific service implementation is instantiated through the
     *     service factory as
     *     <code>com.sun.star.connection.<var>type</var>service<var></var><!--
     *         --></code>
     *     (with <code><var>type</var></code> in lower case, and
     *     <code><var>service</var></code> either <code>Acceptor</code> or
     *     <code>Connector</code>).
     * @param serviceClass the IDL interface type for which to query the
     *     requested service.
     * @param serviceType must be either <code>Acceptor</code> or
     *     <code>Connector</code>.
     * @return an instance of the requested service.  Never returns
     *     <code>null</code>.
     * @throws ConnectionSetupException if the requested service can not be
     *     found, or cannot be instantiated.
     */
    public static Object getConnectionService(XMultiServiceFactory factory,
                                              String description,
                                              Class<?> serviceClass,
                                              String serviceType)
        throws ConnectionSetupException
    {
        int i = description.indexOf(',');
        String type
            = (i < 0 ? description : description.substring(0, i)).toLowerCase();
        Object service = null;
        try {
            service = UnoRuntime.queryInterface(
                serviceClass,
                factory.createInstance("com.sun.star.connection." + type
                                       + serviceType));
        } catch (RuntimeException e) {
            throw e;
        } catch (com.sun.star.uno.Exception e) {
        }
        if (service == null) {
            // As a fallback, also try to instantiate the service from the
            // com.sun.star.lib.connections package structure:
            try {
                service
                    = Class.forName("com.sun.star.lib.connections." + type
                                    + "." + type + serviceType).newInstance();
            } catch (ClassNotFoundException e) {
            } catch (IllegalAccessException e) {
            } catch (InstantiationException e) {
            }
        }
        if (service == null) {
            throw new ConnectionSetupException("no " + serviceType + " for "
                                               + type);
        }
        return service;
    }

    private Implementation() {} // do not instantiate
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
