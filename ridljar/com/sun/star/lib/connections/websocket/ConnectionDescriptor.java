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

package com.sun.star.lib.connections.websocket;

import java.util.Arrays;
import java.util.Iterator;

/**
 * Helper class for <code>websocketConnector</code>.
 */
final class ConnectionDescriptor {
    public ConnectionDescriptor(String description)
    throws com.sun.star.lang.IllegalArgumentException {
        Iterator<String> descriptionParts = Arrays.stream(description.split(",")).iterator();
        descriptionParts
            .next(); // skip over the first part as it's the protocol not a real parameter
        while (descriptionParts.hasNext())
        {
            String parameter = descriptionParts.next();
            String[] pair = parameter.split("=", 2);

            if (pair.length != 2)
            {
                throw new com.sun.star.lang.IllegalArgumentException(
                    String.format("parameter %s lacks '='", parameter));
            }

            String key = pair[0];
            String value = pair[1];
            if (key.equalsIgnoreCase("url")) {
                url = value;
            }
        }
    }

    public String getURL() {
        return url;
    }

    private String url = null;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
