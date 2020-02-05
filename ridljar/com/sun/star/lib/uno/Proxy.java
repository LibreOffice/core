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

package com.sun.star.lib.uno;

/**
 * Marker interface implemented by proxies for UNO objects.
 *
 * <p>Currently, this interface is used internally by
 * <code>com.sun.star.lib.uno.environments.java.java_environment</code> to
 * distinguish between proxies and local objects.  Any proxy object that shall
 * be registered at the <code>java_environment</code> must implement this marker
 * interface.</p>
 */
public interface Proxy {
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
