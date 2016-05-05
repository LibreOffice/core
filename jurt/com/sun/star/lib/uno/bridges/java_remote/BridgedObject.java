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

package com.sun.star.lib.uno.bridges.java_remote;

import com.sun.star.bridge.XBridge;

/**
 * A back door to access the bridge associated with a bridged object.
 */
public final class BridgedObject {
    /**
     * Obtains the bridge associated with a bridged object.
     *
     * @param obj a reference to a (Java representation of a) UNO object;
     *     must not be null.
     * @return the bridge associated with the given object, if it is indeed
     *     bridged; otherwise, null is returned.
     */
    public static XBridge getBridge(Object obj) {
        return ProxyFactory.getBridge(obj);
    }

    private BridgedObject() {} // do not instantiate
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
